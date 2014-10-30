/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			        Wizard/god command module                         *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include "mud.h"
#include "sha256.h"

#define RESTORE_INTERVAL 21600

const char *const save_flag[] = { "death", "kill", "passwd", "drop", "put", "give", "auto", "zap",
   "auction", "get", "receive", "idle", "backup", "quitbackup", "fill",
   "empty", "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24",
   "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};

void save_sysdata( SYSTEM_DATA sys );

/* from reset.c */
int generate_itemlevel( AREA_DATA * pArea, OBJ_INDEX_DATA * pObjIndex );

/* from comm.c */
bool write_to_descriptor( int desc, const char *txt, int length );
bool check_parse_name( const char *name, bool newchar );

/* from boards.c */
void note_attach( CHAR_DATA * ch );

/* from tables.c */
void write_race_file( int ra );

/*
 * Local functions.
 */
void save_watchlist( void );
void close_area( AREA_DATA * pArea );
int get_color( const char *argument ); /* function proto */
void sort_reserved( RESERVE_DATA * pRes );
PROJECT_DATA *get_project_by_number( int pnum );
NOTE_DATA *get_log_by_number( PROJECT_DATA * pproject, int pnum );

/*
 * Global variables.
 */
char reboot_time[50];
time_t new_boot_time_t;
extern struct tm new_boot_struct;
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

int get_saveflag( const char *name )
{
   size_t x;

   for( x = 0; x < sizeof( save_flag ) / sizeof( save_flag[0] ); x++ )
      if( !str_cmp( name, save_flag[x] ) )
         return x;
   return -1;
}

/*
 * Toggle "Do Not Disturb" flag. Used to prevent lower level imms from
 * using commands like "trans" and "goto" on higher level imms.
 */
void do_dnd( CHAR_DATA* ch, const char* argument)
{
   if( !IS_NPC( ch ) && ch->pcdata )
      if( IS_SET( ch->pcdata->flags, PCFLAG_DND ) )
      {
         REMOVE_BIT( ch->pcdata->flags, PCFLAG_DND );
         send_to_char( "Your 'do not disturb' flag is now off.\r\n", ch );
      }
      else
      {
         SET_BIT( ch->pcdata->flags, PCFLAG_DND );
         send_to_char( "Your 'do not disturb' flag is now on.\r\n", ch );
      }
   else
      send_to_char( "huh?\r\n", ch );
}

/*
 * The "watch" facility allows imms to specify the name of a player or
 * the name of a site to be watched. It is like "logging" a player except
 * the results are written to a file in the "watch" directory named with
 * the same name as the imm. The idea is to allow lower level imms to 
 * watch players or sites without having to have access to the log files.
 */
void do_watch( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
   WATCH_DATA *pw;

   if( IS_NPC( ch ) )
      return;

   argument = one_argument( argument, arg );
   set_pager_color( AT_IMMORT, ch );

   if( arg[0] == '\0' || !str_cmp( arg, "help" ) )
   {
      send_to_pager( "Syntax Examples:\r\n", ch );
      /*
       * Only IMP+ can see all the watches. The rest can just see their own.
       */
      if( ch->level >= LEVEL_IMPLEMENTOR )
         send_to_pager( "   watch show all          show all watches\r\n", ch );
      send_to_pager( "   watch show              show all my watches\r\n", ch );
      send_to_pager( "   watch size              show the size of my watch file\r\n", ch );
      send_to_pager( "   watch player joe        add a new player watch\r\n", ch );
      send_to_pager( "   watch site 2.3.123      add a new site watch\r\n", ch );
      send_to_pager( "   watch command make      add a new command watch\r\n", ch );
      send_to_pager( "   watch site 2.3.12       matches 2.3.12x\r\n", ch );
      send_to_pager( "   watch site 2.3.12.      matches 2.3.12.x\r\n", ch );
      send_to_pager( "   watch delete n          delete my nth watch\r\n", ch );
      send_to_pager( "   watch print 500         print watch file starting at line 500\r\n", ch );
      send_to_pager( "   watch print 500 1000    print 1000 lines starting at line 500\r\n", ch );
      send_to_pager( "   watch clear             clear my watch file\r\n", ch );
      return;
   }

   set_pager_color( AT_PLAIN, ch );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   /*
    * Clear watch file
    */
   if( !str_cmp( arg, "clear" ) )
   {
      char fname[256];

      snprintf( fname, 256, "%s%s", WATCH_DIR, strlower( ch->name ) );
      if( 0 == remove( fname ) )
      {
         send_to_pager( "Ok. Your watch file has been cleared.\r\n", ch );
         return;
      }
      send_to_pager( "You have no valid watch file to clear.\r\n", ch );
      return;
   }

   /*
    * Display size of watch file
    */
   if( !str_cmp( arg, "size" ) )
   {
      FILE *fp;
      char fname[256], s[MAX_STRING_LENGTH];
      int rec_count = 0;

      snprintf( fname, 256, "%s%s", WATCH_DIR, strlower( ch->name ) );

      if( !( fp = fopen( fname, "r" ) ) )
      {
         send_to_pager( "You have no watch file. Perhaps you cleared it?\r\n", ch );
         return;
      }

      fgets( s, MAX_STRING_LENGTH, fp );
      while( !feof( fp ) )
      {
         rec_count++;
         fgets( s, MAX_STRING_LENGTH, fp );
      }
      pager_printf( ch, "You have %d lines in your watch file.\r\n", rec_count );
      fclose( fp );
      return;
   }

   /*
    * Print watch file
    */
   if( !str_cmp( arg, "print" ) )
   {
      FILE *fp;
      char fname[256], s[MAX_STRING_LENGTH];
      const int MAX_DISPLAY_LINES = 1000;
      int start, limit, disp_count = 0, rec_count = 0;

      if( arg2[0] == '\0' )
      {
         send_to_pager( "Sorry. You must specify a starting line number.\r\n", ch );
         return;
      }

      start = atoi( arg2 );
      limit = ( arg3[0] == '\0' ) ? MAX_DISPLAY_LINES : atoi( arg3 );
      limit = UMIN( limit, MAX_DISPLAY_LINES );

      snprintf( fname, 256, "%s%s", WATCH_DIR, strlower( ch->name ) );
      if( !( fp = fopen( fname, "r" ) ) )
         return;
      fgets( s, MAX_STRING_LENGTH, fp );

      while( ( disp_count < limit ) && ( !feof( fp ) ) )
      {
         if( ++rec_count >= start )
         {
            send_to_pager( s, ch );
            disp_count++;
         }
         fgets( s, MAX_STRING_LENGTH, fp );
      }
      send_to_pager( "\r\n", ch );
      if( disp_count >= MAX_DISPLAY_LINES )
         send_to_pager( "Maximum display lines exceeded. List is terminated.\r\n"
                        "Type 'help watch' to see how to print the rest of the list.\r\n\r\n"
                        "Your watch file is large. Perhaps you should clear it?\r\n", ch );

      fclose( fp );
      return;
   }

   /*
    * Display all watches
    * Only IMP+ can see all the watches. The rest can just see their own.
    */
   if( ch->level >= LEVEL_IMPLEMENTOR && !str_cmp( arg, "show" ) && !str_cmp( arg2, "all" ) )
   {
      pager_printf( ch, "%-12s %-14s %-15s\r\n", "Imm Name", "Player/Command", "Player Site" );
      if( first_watch )
         for( pw = first_watch; pw; pw = pw->next )
            if( get_trust( ch ) >= pw->imm_level )
               pager_printf( ch, "%-14s %-12s %-15s\r\n", pw->imm_name, pw->target_name ? pw->target_name : " ",
                             pw->player_site ? pw->player_site : " " );
      return;
   }

   /*
    * Display only those watches belonging to the requesting imm 
    */
   if( !str_cmp( arg, "show" ) && arg2[0] == '\0' )
   {
      int cou = 0;
      pager_printf( ch, "%-3s %-12s %-14s %-15s\r\n", " ", "Imm Name", "Player/Command", "Player Site" );
      if( first_watch )
         for( pw = first_watch; pw; pw = pw->next )
            if( !str_cmp( ch->name, pw->imm_name ) )
               pager_printf( ch, "%3d %-12s %-14s %-15s\r\n", ++cou, pw->imm_name, pw->target_name ? pw->target_name : " ",
                             pw->player_site ? pw->player_site : " " );
      return;
   }

   /*
    * Delete a watch belonging to the requesting imm
    */
   if( !str_cmp( arg, "delete" ) && isdigit( *arg2 ) )
   {
      int cou = 0;
      int num;

      num = atoi( arg2 );
      if( first_watch )
      {
         for( pw = first_watch; pw; pw = pw->next )
            if( !str_cmp( ch->name, pw->imm_name ) )
               if( num == ++cou )
               {
                  /*
                   * Oops someone forgot to clear up the memory --Shaddai 
                   */
                  DISPOSE( pw->imm_name );
                  DISPOSE( pw->player_site );
                  DISPOSE( pw->target_name );
                  /*
                   * Now we can unlink and then clear up that final
                   * * pointer -- Shaddai 
                   */
                  UNLINK( pw, first_watch, last_watch, next, prev );
                  DISPOSE( pw );
                  save_watchlist(  );
                  send_to_pager( "Deleted.\r\n", ch );
                  return;
               }
      }
      send_to_pager( "Sorry. I found nothing to delete.\r\n", ch );
      return;
   }

   /*
    * Watch a specific player
    */
   if( !str_cmp( arg, "player" ) && *arg2 )
   {
      WATCH_DATA *pinsert;
      CHAR_DATA *vic;
      char buf[MAX_INPUT_LENGTH];

      if( first_watch ) /* check for dups */
      {
         for( pw = first_watch; pw; pw = pw->next )
            if( !str_cmp( ch->name, pw->imm_name ) && pw->target_name && !str_cmp( arg2, pw->target_name ) )
            {
               send_to_pager( "You are already watching that player.\r\n", ch );
               return;
            }
      }

      CREATE( pinsert, WATCH_DATA, 1 );   /* create new watch */
      pinsert->imm_level = get_trust( ch );
      pinsert->imm_name = str_dup( strlower( ch->name ) );
      pinsert->target_name = str_dup( strlower( arg2 ) );
      pinsert->player_site = NULL;

      /*
       * stupid get_char_world returns ptr to "samantha" when given "sam" 
       */
      /*
       * so I do a str_cmp to make sure it finds the right player --Gorog 
       */

      snprintf( buf, MAX_INPUT_LENGTH, "0.%s", arg2 );
      if( ( vic = get_char_world( ch, buf ) ) ) /* if vic is in game now */
         if( ( !IS_NPC( vic ) ) && !str_cmp( arg2, vic->name ) )
            SET_BIT( vic->pcdata->flags, PCFLAG_WATCH );

      if( first_watch ) /* ins new watch if app */
      {
         for( pw = first_watch; pw; pw = pw->next )
            if( str_cmp( pinsert->imm_name, pw->imm_name ) )
            {
               INSERT( pinsert, pw, first_watch, next, prev );
               save_watchlist(  );
               send_to_pager( "Ok. That player will be watched.\r\n", ch );
               return;
            }
      }

      LINK( pinsert, first_watch, last_watch, next, prev ); /* link new watch */
      save_watchlist(  );
      send_to_pager( "Ok. That player will be watched.\r\n", ch );
      return;
   }

   /*
    * Watch a specific site
    */
   if( !str_cmp( arg, "site" ) && *arg2 )
   {
      WATCH_DATA *pinsert;
      CHAR_DATA *vic;

      if( first_watch ) /* check for dups */
      {
         for( pw = first_watch; pw; pw = pw->next )
            if( !str_cmp( ch->name, pw->imm_name ) && pw->player_site && !str_cmp( arg2, pw->player_site ) )
            {
               send_to_pager( "You are already watching that site.\r\n", ch );
               return;
            }
      }
      CREATE( pinsert, WATCH_DATA, 1 );   /* create new watch */
      pinsert->imm_level = get_trust( ch );
      pinsert->imm_name = str_dup( strlower( ch->name ) );
      pinsert->player_site = str_dup( strlower( arg2 ) );
      pinsert->target_name = NULL;

      for( vic = first_char; vic; vic = vic->next )
         if( !IS_NPC( vic ) && vic->desc && *pinsert->player_site && !str_prefix( pinsert->player_site, vic->desc->host )
             && get_trust( vic ) < pinsert->imm_level )
            SET_BIT( vic->pcdata->flags, PCFLAG_WATCH );

      if( first_watch ) /* ins new watch if app */
      {
         for( pw = first_watch; pw; pw = pw->next )
            if( str_cmp( pinsert->imm_name, pw->imm_name ) )
            {
               INSERT( pinsert, pw, first_watch, next, prev );
               save_watchlist(  );
               send_to_pager( "Ok. That site will be watched.\r\n", ch );
               return;
            }
      }
      LINK( pinsert, first_watch, last_watch, next, prev );
      save_watchlist(  );
      send_to_pager( "Ok. That site will be watched.\r\n", ch );
      return;
   }

   /*
    * Watch a specific command - FB
    */
   if( !str_cmp( arg, "command" ) && *arg2 )
   {
      WATCH_DATA *pinsert;
      CMDTYPE *cmd;
      bool found = FALSE;

      for( pw = first_watch; pw; pw = pw->next )
      {
         if( !str_cmp( ch->name, pw->imm_name ) && pw->target_name && !str_cmp( arg2, pw->target_name ) )
         {
            send_to_pager( "You are already watching that command.\r\n", ch );
            return;
         }
      }

      for( cmd = command_hash[LOWER( arg2[0] ) % 126]; cmd; cmd = cmd->next )
      {
         if( !str_cmp( arg2, cmd->name ) )
         {
            found = TRUE;
            break;
         }
      }

      if( !found )
      {
         send_to_pager( "No such command exists.\r\n", ch );
         return;
      }
      else
         SET_BIT( cmd->flags, CMD_WATCH );

      CREATE( pinsert, WATCH_DATA, 1 );
      pinsert->imm_level = get_trust( ch );
      pinsert->imm_name = str_dup( strlower( ch->name ) );
      pinsert->player_site = NULL;
      pinsert->target_name = str_dup( arg2 );

      for( pw = first_watch; pw; pw = pw->next )
      {
         if( !str_cmp( pinsert->imm_name, pw->imm_name ) )
         {
            INSERT( pinsert, pw, first_watch, next, prev );
            save_watchlist(  );
            send_to_pager( "Ok, That command will be watched.\r\n", ch );
            return;
         }
      }

      LINK( pinsert, first_watch, last_watch, next, prev );
      save_watchlist(  );
      send_to_pager( "Ok. That site will be watched.\r\n", ch );
      return;
   }

   send_to_pager( "Sorry. I can't do anything with that. Please read the help file.\r\n", ch );
   return;
}

void do_wizhelp( CHAR_DATA* ch, const char* argument)
{
   CMDTYPE *cmd;
   int col, hash;

   col = 0;
   set_pager_color( AT_PLAIN, ch );
   for( hash = 0; hash < 126; hash++ )
      for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
         if( cmd->level >= LEVEL_HERO && cmd->level <= get_trust( ch ) )
         {
            pager_printf( ch, "%-12s", cmd->name );
            if( ++col % 6 == 0 )
               send_to_pager( "\r\n", ch );
         }

   if( col % 6 != 0 )
      send_to_pager( "\r\n", ch );
   return;
}

void do_restrict( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   short level, hash;
   CMDTYPE *cmd;
   bool found;

   found = FALSE;
   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Restrict which command?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg2 );
   if( arg2[0] == '\0' )
      level = get_trust( ch );
   else
      level = atoi( arg2 );

   level = UMAX( UMIN( get_trust( ch ), level ), 0 );

   hash = arg[0] % 126;
   for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
   {
      if( !str_prefix( arg, cmd->name ) && cmd->level <= get_trust( ch ) )
      {
         found = TRUE;
         break;
      }
   }

   if( found )
   {
      if( !str_prefix( arg2, "show" ) )
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s show", cmd->name );
         do_cedit( ch, buf );
/*    		ch_printf( ch, "%s is at level %d.\r\n", cmd->name, cmd->level );*/
         return;
      }
      cmd->level = level;
      ch_printf( ch, "You restrict %s to level %d\r\n", cmd->name, level );
      log_printf( "%s restricting %s to level %d", ch->name, cmd->name, level );
   }
   else
      send_to_char( "You may not restrict that command.\r\n", ch );

   return;
}

/* 
 * Check if the name prefix uniquely identifies a char descriptor
 */
CHAR_DATA *get_waiting_desc( CHAR_DATA * ch, char *name )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *ret_char = NULL;
   static unsigned int number_of_hits;

   number_of_hits = 0;
   for( d = first_descriptor; d; d = d->next )
   {
      if( d->character && ( !str_prefix( name, d->character->name ) ) && IS_WAITING_FOR_AUTH( d->character ) )
      {
         if( ++number_of_hits > 1 )
         {
            ch_printf( ch, "%s does not uniquely identify a char.\r\n", name );
            return NULL;
         }
         ret_char = d->character;   /* return current char on exit */
      }
   }
   if( number_of_hits == 1 )
      return ret_char;
   else
   {
      send_to_char( "No one like that waiting for authorization.\r\n", ch );
      return NULL;
   }
}

/* 02-07-99  New auth messages --Mystaric */
void do_authorize( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;

   set_char_color( AT_LOG, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Usage:  authorize <player> <yes|name|immsim|mobsim|swear|plain|unpronu|no/deny>\r\n", ch );
      send_to_char( "Pending authorizations:\r\n", ch );
      send_to_char( " Chosen Character Name\r\n", ch );
      send_to_char( "---------------------------------------------\r\n", ch );
      for( d = first_descriptor; d; d = d->next )
         if( ( victim = d->character ) != NULL && IS_WAITING_FOR_AUTH( victim ) )
            ch_printf( ch, " %s@%s new %s %s (%s)...\r\n",
                       victim->name,
                       victim->desc->host,
                       race_table[victim->race]->race_name,
                       class_table[victim->Class]->who_name, IS_PKILL( victim ) ? "Deadly" : "Peaceful" );
      return;
   }

   victim = get_waiting_desc( ch, arg1 );
   if( victim == NULL )
      return;

   set_char_color( AT_IMMORT, victim );
   if( arg2[0] == '\0' || !str_cmp( arg2, "accept" ) || !str_cmp( arg2, "yes" ) )
   {
      victim->pcdata->auth_state = 3;
      if( victim->pcdata->authed_by )
         STRFREE( victim->pcdata->authed_by );
      victim->pcdata->authed_by = QUICKLINK( ch->name );
      snprintf( buf, MAX_STRING_LENGTH, "%s: authorized", victim->name );
      to_channel( buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE );
      ch_printf( ch, "You have authorized %s.\r\n", victim->name );

      /*
       * Below sends a message to player when name is accepted - Brittany 
       */
      ch_printf_color( victim,
                       "\r\n&GThe MUD Administrators have accepted the name %s.\r\n"
                       "You are authorized to enter the Realms at the end of " "this area.\r\n", victim->name );
      return;
   }

   else if( !str_cmp( arg2, "immsim" ) || !str_cmp( arg2, "i" ) )
   {
      victim->pcdata->auth_state = 2;
      snprintf( buf, MAX_STRING_LENGTH, "%s: name denied - similar to Imm name", victim->name );
      to_channel( buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE );
      send_to_char_color( "&RThe name you have chosen is too similar to that of a current immortal. \r\n"
                          "We ask you to please choose another name using the name command.\r\n", victim );
      ch_printf( ch, "You requested %s change names.\r\n", victim->name );
      return;
   }

   else if( !str_cmp( arg2, "mobsim" ) || !str_cmp( arg2, "m" ) )
   {
      victim->pcdata->auth_state = 2;
      snprintf( buf, MAX_STRING_LENGTH, "%s: name denied - similar to mob name", victim->name );
      to_channel( buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE );
      send_to_char_color( "&RThe name you have chosen is too similar to that of certain\r\n"
                          "monsters in the game, in the long run this could cause problems\r\n"
                          "and therefore we are unable to authorize them.  Please choose\r\n"
                          "another name using the name command.\r\n", victim );
      ch_printf( ch, "You requested %s change names.\r\n", victim->name );
      return;
   }

   else if( !str_cmp( arg2, "swear" ) || !str_cmp( arg2, "s" ) )
   {
      victim->pcdata->auth_state = 2;
      snprintf( buf, MAX_STRING_LENGTH, "%s: name denied - swear word", victim->name );
      to_channel( buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE );
      send_to_char_color( "&RWe will not authorize names containing swear words, in any language.\r\n"
                          "Please choose another name using the name command.\r\n", victim );
      ch_printf( ch, "You requested %s change names.\r\n", victim->name );
      return;
   }

   else if( !str_cmp( arg2, "plain" ) || !str_cmp( arg2, "p" ) )
   {
      victim->pcdata->auth_state = 2;
      snprintf( buf, MAX_STRING_LENGTH, "%s: name denied", victim->name );
      to_channel( buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE );
      send_to_char_color( "&RWe would ask you to please attempt to choose a name that is more\r\n"
                          "medieval in nature.  Please choose another name using the name\r\n" "command.\r\n", victim );
      ch_printf( ch, "You requested %s change names.\r\n", victim->name );
      return;
   }

   else if( !str_cmp( arg2, "unprou" ) || !str_cmp( arg2, "u" ) )
   {
      victim->pcdata->auth_state = 2;
      snprintf( buf, MAX_STRING_LENGTH, "%s: name denied - unpronouncable", victim->name );
      to_channel( buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE );
      send_to_char_color( "&RThe name you have chosen is unpronouncable.\r\n"
                          "Please choose another name using the name\r\n" "command.\r\n", victim );
      ch_printf( ch, "You requested %s change names.\r\n", victim->name );
      return;
   }

   else if( !str_cmp( arg2, "no" ) || !str_cmp( arg2, "deny" ) )
   {
      send_to_char_color( "&RThe name you have chosen and/or the actions you have taken have\r\n"
                          "been deemed grossly unacceptable to the administration of this mud.\r\n"
                          "We ask you to discontinue such behaviour, or suffer possible banishmemt\r\n"
                          "from this mud.\r\n", victim );
      snprintf( buf, MAX_STRING_LENGTH, "%s: denied authorization", victim->name );
      to_channel( buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE );
      ch_printf( ch, "You have denied %s.\r\n", victim->name );
      do_quit( victim, "" );
   }

   else if( !str_cmp( arg2, "name" ) || !str_cmp( arg2, "n" ) )
   {
      victim->pcdata->auth_state = 2;
      snprintf( buf, MAX_STRING_LENGTH, "%s: name denied", victim->name );
      to_channel( buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE );
      ch_printf_color( victim,
                       "&R\r\nThe MUD Administrators have found the name %s "
                       "to be unacceptable.\r\n"
                       "You may choose a new name when you reach "
                       "the end of this area.\r\n"
                       "The name you choose must be medieval and original.\r\n"
                       "No titles, descriptive words, or names close to any existing "
                       "Immortal's name.\r\n", victim->name );
      ch_printf( ch, "You requested %s change names.\r\n", victim->name );
      return;
   }

   else
   {
      send_to_char( "Invalid argument.\r\n", ch );
      return;
   }
}

void do_bamfin( CHAR_DATA* ch, const char* argument)
{
   if( !IS_NPC( ch ) )
   {
      char* newbamf = str_dup(argument);
      smash_tilde(newbamf);
      DISPOSE( ch->pcdata->bamfin );
      ch->pcdata->bamfin = newbamf;
      send_to_char_color( "&YBamfin set.\r\n", ch );
   }
   return;
}

void do_bamfout( CHAR_DATA* ch, const char* argument)
{
   if( !IS_NPC( ch ) )
   {
      char* newbamf = str_dup(argument);
      smash_tilde(newbamf);
      DISPOSE( ch->pcdata->bamfout );
      ch->pcdata->bamfout = newbamf;
      send_to_char_color( "&YBamfout set.\r\n", ch );
   }
   return;
}

void do_rank( CHAR_DATA* ch, const char* argument)
{

   set_char_color( AT_IMMORT, ch );

   if( IS_NPC( ch ) )
      return;
   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage:  rank <string>.\r\n", ch );
      send_to_char( "   or:  rank none.\r\n", ch );
      return;
   }

   // clear the old rank
   DISPOSE( ch->pcdata->rank );

   if( !str_cmp( argument, "none" ) )
      ch->pcdata->rank = str_dup( "" );
   else
   {
      char* newrank = str_dup(argument);
      smash_tilde( newrank );
      ch->pcdata->rank = newrank;
   }
   send_to_char( "Ok.\r\n", ch );
   return;
}

void do_retire( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Retire whom?\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }
   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }
   if( victim->level < LEVEL_SAVIOR )
   {
      send_to_char( "The minimum level for retirement is savior.\r\n", ch );
      return;
   }
   if( IS_RETIRED( victim ) )
   {
      REMOVE_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
      ch_printf( ch, "%s returns from retirement.\r\n", victim->name );
      ch_printf( victim, "%s brings you back from retirement.\r\n", ch->name );
   }
   else
   {
      SET_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
      ch_printf( ch, "%s is now a retired immortal.\r\n", victim->name );
      ch_printf( victim, "Courtesy of %s, you are now a retired immortal.\r\n", ch->name );
   }
   return;
}

void do_delay( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   int delay;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "Syntax:  delay <victim> <# of rounds>\r\n", ch );
      return;
   }
   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "No such character online.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Mobiles are unaffected by lag.\r\n", ch );
      return;
   }
   if( !IS_NPC( victim ) && get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You haven't the power to succeed against them.\r\n", ch );
      return;
   }
   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "For how long do you wish to delay them?\r\n", ch );
      return;
   }
   if( !str_cmp( arg, "none" ) )
   {
      send_to_char( "All character delay removed.\r\n", ch );
      victim->wait = 0;
      return;
   }
   delay = atoi( arg );
   if( delay < 1 )
   {
      send_to_char( "Pointless.  Try a positive number.\r\n", ch );
      return;
   }
   if( delay > 999 )
   {
      send_to_char( "You cruel bastard.  Just kill them.\r\n", ch );
      return;
   }
   WAIT_STATE( victim, delay * PULSE_VIOLENCE );
   ch_printf( ch, "You've delayed %s for %d rounds.\r\n", victim->name, delay );
   return;
}

void do_deny( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Deny whom?\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }
   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }
   xSET_BIT( victim->act, PLR_DENY );
   set_char_color( AT_IMMORT, victim );
   send_to_char( "You are denied access!\r\n", victim );
   ch_printf( ch, "You have denied access to %s.\r\n", victim->name );
   if( victim->fighting )
      stop_fighting( victim, TRUE );   /* Blodkai, 97 */
   do_quit( victim, "" );
   return;
}

void do_disconnect( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Disconnect whom?\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( victim->desc == NULL )
   {
      act( AT_PLAIN, "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
      return;
   }
   if( get_trust( ch ) <= get_trust( victim ) )
   {
      send_to_char( "They might not like that...\r\n", ch );
      return;
   }

   for( d = first_descriptor; d; d = d->next )
   {
      if( d == victim->desc )
      {
         close_socket( d, FALSE );
         send_to_char( "Ok.\r\n", ch );
         return;
      }
   }
   bug( "%s", "Do_disconnect: *** desc not found ***." );
   send_to_char( "Descriptor not found!\r\n", ch );
   return;
}


/*
 * Force a level one player to quit.             Gorog
 */
void do_fquit( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Force whom to quit?\r\n", ch );
      return;
   }
   if( !( victim = get_char_world( ch, arg1 ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( victim->level != 1 )
   {
      send_to_char( "They are not level one!\r\n", ch );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   send_to_char( "The MUD administrators force you to quit...\r\n", victim );
   if( victim->fighting )
      stop_fighting( victim, TRUE );
   do_quit( victim, "" );
   ch_printf( ch, "You have forced %s to quit.\r\n", victim->name );
   return;
}

void do_forceclose( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   int desc;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Usage: forceclose <descriptor#>\r\n", ch );
      return;
   }

   desc = atoi( arg );
   for( d = first_descriptor; d; d = d->next )
   {
      if( d->descriptor == desc )
      {
         if( d->character && get_trust( d->character ) >= get_trust( ch ) )
         {
            send_to_char( "They might not like that...\r\n", ch );
            return;
         }
         close_socket( d, FALSE );
         send_to_char( "Ok.\r\n", ch );
         return;
      }
   }
   send_to_char( "Not found!\r\n", ch );
   return;
}

void do_pardon( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Syntax: pardon <character> <killer|thief|attacker>.\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "attacker" ) )
   {
      if( xIS_SET( victim->act, PLR_ATTACKER ) )
      {
         xREMOVE_BIT( victim->act, PLR_ATTACKER );
         ch_printf( ch, "Attacker flag removed from %s.\r\n", victim->name );
         set_char_color( AT_IMMORT, victim );
         send_to_char( "You are no longer an ATTACKER.\r\n", victim );
      }
      return;
   }
   if( !str_cmp( arg2, "killer" ) )
   {
      if( xIS_SET( victim->act, PLR_KILLER ) )
      {
         xREMOVE_BIT( victim->act, PLR_KILLER );
         ch_printf( ch, "Killer flag removed from %s.\r\n", victim->name );
         set_char_color( AT_IMMORT, victim );
         send_to_char( "You are no longer a KILLER.\r\n", victim );
      }
      return;
   }
   if( !str_cmp( arg2, "thief" ) )
   {
      if( xIS_SET( victim->act, PLR_THIEF ) )
      {
         xREMOVE_BIT( victim->act, PLR_THIEF );
         ch_printf( ch, "Thief flag removed from %s.\r\n", victim->name );
         set_char_color( AT_IMMORT, victim );
         send_to_char( "You are no longer a THIEF.\r\n", victim );
      }
      return;
   }
   send_to_char( "Syntax: pardon <character> <killer|thief>.\r\n", ch );
   return;
}

void echo_to_all( short AT_COLOR, const char *argument, short tar )
{
   DESCRIPTOR_DATA *d;

   if( !argument || argument[0] == '\0' )
      return;

   for( d = first_descriptor; d; d = d->next )
   {
      /*
       * Added showing echoes to players who are editing, so they won't
       * miss out on important info like upcoming reboots. --Narn 
       */
      if( d->connected == CON_PLAYING || d->connected == CON_EDITING )
      {
         /*
          * This one is kinda useless except for switched.. 
          */
         if( tar == ECHOTAR_PC && IS_NPC( d->character ) )
            continue;
         else if( tar == ECHOTAR_IMM && !IS_IMMORTAL( d->character ) )
            continue;
         set_char_color( AT_COLOR, d->character );
         send_to_char( argument, d->character );
         send_to_char( "\r\n", d->character );
      }
   }
   return;
}

void do_ech( CHAR_DATA* ch, const char* argument)
{
   send_to_char_color( "&YIf you want to echo something, use 'echo'.\r\n", ch );
   return;
}

void do_echo( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   short color;
   int target;
   const char *parg;

   set_char_color( AT_IMMORT, ch );

   if( xIS_SET( ch->act, PLR_NO_EMOTE ) )
   {
      send_to_char( "You can't do that right now.\r\n", ch );
      return;
   }
   if( argument[0] == '\0' )
   {
      send_to_char( "Echo what?\r\n", ch );
      return;
   }

   if( ( color = get_color( argument ) ) )
      argument = one_argument( argument, arg );
   parg = argument;
   argument = one_argument( argument, arg );
   if( !str_cmp( arg, "PC" ) || !str_cmp( arg, "player" ) )
      target = ECHOTAR_PC;
   else if( !str_cmp( arg, "imm" ) )
      target = ECHOTAR_IMM;
   else
   {
      target = ECHOTAR_ALL;
      argument = parg;
   }
   if( !color && ( color = get_color( argument ) ) )
      argument = one_argument( argument, arg );
   if( !color )
      color = AT_IMMORT;
   one_argument( argument, arg );
   echo_to_all( color, argument, target );
}

void echo_to_room( short AT_COLOR, ROOM_INDEX_DATA * room, const char *argument )
{
   CHAR_DATA *vic;

   for( vic = room->first_person; vic; vic = vic->next_in_room )
   {
      set_char_color( AT_COLOR, vic );
      send_to_char( argument, vic );
      send_to_char( "\r\n", vic );
   }
}

void do_recho( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   short color;

   set_char_color( AT_IMMORT, ch );

   if( xIS_SET( ch->act, PLR_NO_EMOTE ) )
   {
      send_to_char( "You can't do that right now.\r\n", ch );
      return;
   }
   if( argument[0] == '\0' )
   {
      send_to_char( "Recho what?\r\n", ch );
      return;
   }

   one_argument( argument, arg );
   if( ( color = get_color( argument ) ) )
   {
      argument = one_argument( argument, arg );
      echo_to_room( color, ch->in_room, argument );
   }
   else
      echo_to_room( AT_IMMORT, ch->in_room, argument );
}

ROOM_INDEX_DATA *find_location( CHAR_DATA * ch, const char *arg )
{
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   if( is_number( arg ) )
      return get_room_index( atoi( arg ) );

   if( !str_cmp( arg, "pk" ) )   /* "Goto pk", "at pk", etc */
      return get_room_index( last_pkroom );

   if( ( victim = get_char_world( ch, arg ) ) != NULL )
      return victim->in_room;

   if( ( obj = get_obj_world( ch, arg ) ) != NULL )
      return obj->in_room;

   return NULL;
}

/* This function shared by do_transfer and do_mptransfer
 *
 * Immortals bypass most restrictions on where to transfer victims.
 * NPCs cannot transfer victims who are:
 * 1. Not authorized yet.
 * 2. Outside of the level range for the target room's area.
 * 3. Being sent to private rooms.
 */
void transfer_char( CHAR_DATA * ch, CHAR_DATA * victim, ROOM_INDEX_DATA * location )
{
   if( !victim->in_room )
   {
      bug( "%s: victim in NULL room: %s", __FUNCTION__, victim->name );
      return;
   }

   if( IS_NPC( ch ) && room_is_private( location ) )
   {
      progbug( "Mptransfer - Private room", ch );
      return;
   }

   if( !can_see( ch, victim ) )
      return;

   if( IS_NPC( ch ) && NOT_AUTHED( victim ) && location->area != victim->in_room->area )
   {
      char buf[MAX_STRING_LENGTH];

      snprintf( buf, MAX_STRING_LENGTH, "Mptransfer - unauthed char (%s)", victim->name );
      progbug( buf, ch );
      return;
   }

   /*
    * If victim not in area's level range, do not transfer 
    */
   if( IS_NPC( ch ) && !in_hard_range( victim, location->area ) && !xIS_SET( location->room_flags, ROOM_PROTOTYPE ) )
      return;

   if( victim->fighting )
      stop_fighting( victim, TRUE );

   if( !IS_NPC( ch ) )
   {
      act( AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim, NULL, NULL, TO_ROOM );
      victim->retran = victim->in_room->vnum;
   }
   char_from_room( victim );
   char_to_room( victim, location );
   if( !IS_NPC( ch ) )
   {
      act( AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
      if( ch != victim )
         act( AT_IMMORT, "$n has transferred you.", ch, NULL, victim, TO_VICT );
      do_look( victim, "auto" );
      if( !IS_IMMORTAL( victim ) && !IS_NPC( victim ) && !in_hard_range( victim, location->area ) )
         act( AT_DANGER, "Warning:  this player's level is not within the area's level range.", ch, NULL, NULL, TO_CHAR );
   }
}

void do_transfer( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Transfer whom (and where)?\r\n", ch );
      return;
   }

   if( arg2[0] != '\0' )
   {
      if( !( location = find_location( ch, arg2 ) ) )
      {
         send_to_char( "That location does not exist.\r\n", ch );
         return;
      }
   }
   else
      location = ch->in_room;

   if( !str_cmp( arg1, "all" ) && get_trust( ch ) >= LEVEL_GREATER )
   {
      for( d = first_descriptor; d; d = d->next )
      {
         if( d->connected == CON_PLAYING && d->character && d->character != ch && d->character->in_room )
            transfer_char( ch, d->character, location );
      }
      return;
   }

   if( !( victim = get_char_world( ch, arg1 ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   /*
    * modification to prevent a low level imm from transferring a 
    * higher level imm with the DND flag on.  - Gorog             
    */
   if( !IS_NPC( victim ) && get_trust( ch ) < get_trust( victim )
       && victim->desc
       && ( victim->desc->connected == CON_PLAYING
            || victim->desc->connected == CON_EDITING ) && IS_SET( victim->pcdata->flags, PCFLAG_DND ) )
   {
      pager_printf( ch, "Sorry. %s does not wish to be disturbed.\r\n", victim->name );
      pager_printf( victim, "Your DND flag just foiled %s's transfer command.\r\n", ch->name );
      return;
   }
   /*
    * end of modification
    */

   transfer_char( ch, victim, location );
}

void do_retran( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Retransfer whom?\r\n", ch );
      return;
   }
   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   snprintf( buf, MAX_STRING_LENGTH, "'%s' %d", victim->name, victim->retran );
   do_transfer( ch, buf );
   return;
}

void do_regoto( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];

   snprintf( buf, MAX_STRING_LENGTH, "%d", ch->regoto );
   do_goto( ch, buf );
   return;
}

/*  Added do_at and do_atobj to reduce lag associated with at
 *  --Shaddai
 */
void do_at( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location = NULL;
   ROOM_INDEX_DATA *original;
   CHAR_DATA *wch = NULL, *victim;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "At where what?\r\n", ch );
      return;
   }
   if( is_number( arg ) )
      location = get_room_index( atoi( arg ) );
   else if( !str_cmp( arg, "pk" ) )
      location = get_room_index( last_pkroom );
   else if( ( wch = get_char_world( ch, arg ) ) == NULL || wch->in_room == NULL )
   {
      send_to_char( "No such mobile or player in existance.\r\n", ch );
      return;
   }
   if( !location && wch )
      location = wch->in_room;

   if( !location )
   {
      send_to_char( "No such location exists.\r\n", ch );
      return;
   }

   /*
    * The following mod is used to prevent players from using the 
    */
   /*
    * at command on a higher level immortal who has a DND flag    
    */
   if( wch && !IS_NPC( wch ) && IS_SET( wch->pcdata->flags, PCFLAG_DND ) && get_trust( ch ) < get_trust( wch ) )
   {
      pager_printf( ch, "Sorry. %s does not wish to be disturbed.\r\n", wch->name );
      pager_printf( wch, "Your DND flag just foiled %s's at command.\r\n", ch->name );
      return;
   }
   /*
    * End of modification  -- Gorog 
    */


   if( room_is_private( location ) )
   {
      if( get_trust( ch ) < LEVEL_GREATER )
      {
         send_to_char( "That room is private right now.\r\n", ch );
         return;
      }
      else
         send_to_char( "Overriding private flag!\r\n", ch );
   }

   if( ( victim = room_is_dnd( ch, location ) ) )
   {
      pager_printf( ch, "That room is \"do not disturb\" right now.\r\n" );
      pager_printf( victim, "Your DND flag just foiled %s's atmob command\r\n", ch->name );
      return;
   }

   set_char_color( AT_PLAIN, ch );
   original = ch->in_room;
   char_from_room( ch );
   char_to_room( ch, location );
   interpret( ch, argument );

   if( !char_died( ch ) )
   {
      char_from_room( ch );
      char_to_room( ch, original );
   }
   return;
}

void do_atobj( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   OBJ_DATA *obj;
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "At where what?\r\n", ch );
      return;
   }

   if( ( obj = get_obj_world( ch, arg ) ) == NULL || !obj->in_room )
   {
      send_to_char( "No such object in existance.\r\n", ch );
      return;
   }
   location = obj->in_room;
   if( room_is_private( location ) )
   {
      if( get_trust( ch ) < LEVEL_GREATER )
      {
         send_to_char( "That room is private right now.\r\n", ch );
         return;
      }
      else
         send_to_char( "Overriding private flag!\r\n", ch );
   }

   if( ( victim = room_is_dnd( ch, location ) ) )
   {
      pager_printf( ch, "That room is \"do not disturb\" right now.\r\n" );
      pager_printf( victim, "Your DND flag just foiled %s's atobj command\r\n", ch->name );
      return;
   }

   set_char_color( AT_PLAIN, ch );
   original = ch->in_room;
   char_from_room( ch );
   char_to_room( ch, location );
   interpret( ch, argument );

   if( !char_died( ch ) )
   {
      char_from_room( ch );
      char_to_room( ch, original );
   }
   return;
}

void do_rat( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   int Start, End, vnum;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Syntax: rat <start> <end> <command>\r\n", ch );
      return;
   }

   Start = atoi( arg1 );
   End = atoi( arg2 );
   if( Start < 1 || End < Start || Start > End || Start == End || End > MAX_VNUM )
   {
      send_to_char( "Invalid range.\r\n", ch );
      return;
   }
   if( !str_cmp( argument, "quit" ) )
   {
      send_to_char( "I don't think so!\r\n", ch );
      return;
   }

   original = ch->in_room;
   for( vnum = Start; vnum <= End; vnum++ )
   {
      if( ( location = get_room_index( vnum ) ) == NULL )
         continue;
      char_from_room( ch );
      char_to_room( ch, location );
      interpret( ch, argument );
   }

   char_from_room( ch );
   char_to_room( ch, original );
   send_to_char( "Done.\r\n", ch );
   return;
}

void do_rstat( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   const char *sect;
   ROOM_INDEX_DATA *location;
   OBJ_DATA *obj;
   CHAR_DATA *rch;
   EXIT_DATA *pexit;
   AFFECT_DATA *paf;
   int cnt;
   static const char *dir_text[] = { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };

   one_argument( argument, arg );
   if( !str_cmp( arg, "ex" ) || !str_cmp( arg, "exits" ) )
   {
      location = ch->in_room;

      ch_printf_color( ch, "&cExits for room '&W%s&c'  Vnum &W%d\r\n", location->name, location->vnum );
      for( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
         ch_printf_color( ch,
                          "&W%2d) &w%2s to %-5d  &cKey: &w%d  &cFlags: &w%d  &cKeywords: '&w%s&c'\r\n     Exdesc: &w%s     &cBack link: &w%d  &cVnum: &w%d  &cDistance: &w%d  &cPulltype: &w%s  &cPull: &w%d\r\n",
                          ++cnt,
                          dir_text[pexit->vdir],
                          pexit->to_room ? pexit->to_room->vnum : 0,
                          pexit->key,
                          pexit->exit_info,
                          pexit->keyword,
                          pexit->description[0] != '\0'
                          ? pexit->description : "(none).\r\n",
                          pexit->rexit ? pexit->rexit->vnum : 0,
                          pexit->rvnum, pexit->distance, pull_type_name( pexit->pulltype ), pexit->pull );
      return;
   }
   location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
   if( !location )
   {
      send_to_char( "No such location.\r\n", ch );
      return;
   }

   if( ch->in_room != location && room_is_private( location ) )
   {
      if( get_trust( ch ) < LEVEL_GREATER )
      {
         send_to_char( "That room is private right now.\r\n", ch );
         return;
      }
      else
         send_to_char( "Overriding private flag!\r\n", ch );
   }

   ch_printf_color( ch, "&cName: &w%s\r\n&cArea: &w%s  &cFilename: &w%s\r\n",
                    location->name,
                    location->area ? location->area->name : "None????",
                    location->area ? location->area->filename : "None????" );

   switch ( ch->in_room->sector_type )
   {
      default:
         sect = "?!";
         break;
      case SECT_INSIDE:
         sect = "Inside";
         break;
      case SECT_CITY:
         sect = "City";
         break;
      case SECT_FIELD:
         sect = "Field";
         break;
      case SECT_FOREST:
         sect = "Forest";
         break;
      case SECT_HILLS:
         sect = "Hills";
         break;
      case SECT_MOUNTAIN:
         sect = "Mountains";
         break;
      case SECT_WATER_SWIM:
         sect = "Swim";
         break;
      case SECT_WATER_NOSWIM:
         sect = "Noswim";
         break;
      case SECT_UNDERWATER:
         sect = "Underwater";
         break;
      case SECT_AIR:
         sect = "Air";
         break;
      case SECT_DESERT:
         sect = "Desert";
         break;
      case SECT_OCEANFLOOR:
         sect = "Oceanfloor";
         break;
      case SECT_UNDERGROUND:
         sect = "Underground";
         break;
      case SECT_LAVA:
         sect = "Lava";
         break;
      case SECT_SWAMP:
         sect = "Swamp";
         break;
   }

   ch_printf_color( ch, "&cVnum: &w%d   &cSector: &w%d (%s)   &cLight: &w%d",
                    location->vnum, location->sector_type, sect, location->light );
   if( location->tunnel > 0 )
      ch_printf_color( ch, "   &cTunnel: &W%d", location->tunnel );
   send_to_char( "\r\n", ch );
   if( location->tele_delay > 0 || location->tele_vnum > 0 )
      ch_printf_color( ch, "&cTeleDelay: &R%d   &cTeleVnum: &R%d\r\n", location->tele_delay, location->tele_vnum );
   ch_printf_color( ch, "&cRoom flags: &w%s\r\n", ext_flag_string( &location->room_flags, r_flags ) );
   ch_printf_color( ch, "&cDescription:\r\n&w%s", location->description );
   if( location->first_extradesc )
   {
      EXTRA_DESCR_DATA *ed;

      send_to_char_color( "&cExtra description keywords: &w'", ch );
      for( ed = location->first_extradesc; ed; ed = ed->next )
      {
         send_to_char( ed->keyword, ch );
         if( ed->next )
            send_to_char( " ", ch );
      }
      send_to_char( "'\r\n", ch );
   }
   for( paf = location->first_affect; paf; paf = paf->next )
      ch_printf_color( ch, "&cAffect: &w%s &cby &w%d.\r\n", affect_loc_name( paf->location ), paf->modifier );

   send_to_char( "&cPermanent affects: &w", ch );
   if( !ch->in_room->first_permaffect )
      send_to_char( "None\r\n", ch );
   else
   {
      send_to_char( "\r\n", ch );

      for( paf = location->first_permaffect; paf; paf = paf->next )
         showaffect( ch, paf );
   }

   send_to_char_color( "&cCharacters: &w", ch );
   for( rch = location->first_person; rch; rch = rch->next_in_room )
   {
      if( can_see( ch, rch ) )
      {
         send_to_char( " ", ch );
         one_argument( rch->name, buf );
         send_to_char( buf, ch );
      }
   }

   send_to_char_color( "\r\n&cObjects:    &w", ch );
   for( obj = location->first_content; obj; obj = obj->next_content )
   {
      send_to_char( " ", ch );
      one_argument( obj->name, buf );
      send_to_char( buf, ch );
   }
   send_to_char( "\r\n", ch );

   if( location->first_exit )
      send_to_char_color( "&c------------------- &wEXITS &c-------------------\r\n", ch );
   for( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
      ch_printf( ch,
                 "%2d) %-2s to %-5d.  Key: %d  Flags: %d  Keywords: %s.\r\n",
                 ++cnt,
                 dir_text[pexit->vdir],
                 pexit->to_room ? pexit->to_room->vnum : 0,
                 pexit->key, pexit->exit_info, pexit->keyword[0] != '\0' ? pexit->keyword : "(none)" );
   return;
}

/* Face-lift by Demora */
void do_ostat( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   AFFECT_DATA *paf;
   OBJ_DATA *obj;

   set_char_color( AT_CYAN, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Ostat what?\r\n", ch );
      return;
   }
   if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
      mudstrlcpy( arg, argument, MAX_INPUT_LENGTH );

   if( ( obj = get_obj_world( ch, arg ) ) == NULL )
   {
      send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );
      return;
   }
   ch_printf_color( ch, "&cName: &C%s\r\n", obj->name );
   ch_printf_color( ch, "&cVnum: &w%d  ", obj->pIndexData->vnum );
   ch_printf_color( ch, "&cType: &w%s  ", item_type_name( obj ) );
   ch_printf_color( ch, "&cCount:  &w%d  ", obj->pIndexData->count );
   ch_printf_color( ch, "&cGcount: &w%d\r\n", obj->count );
   ch_printf_color( ch, "&cSerial#: &w%d  ", obj->serial );
   ch_printf_color( ch, "&cTopIdxSerial#: &w%d  ", obj->pIndexData->serial );
   ch_printf_color( ch, "&cTopSerial#: &w%d\r\n", cur_obj_serial );
   ch_printf_color( ch, "&cShort description: &C%s\r\n", obj->short_descr );
   ch_printf_color( ch, "&cLong description : &C%s\r\n", obj->description );
   if( obj->action_desc && obj->action_desc[0] != '\0' )
      ch_printf_color( ch, "&cAction description: &w%s\r\n", obj->action_desc );
   if( IS_OBJ_STAT( obj, ITEM_PERSONAL ) && obj->owner && obj->owner[0] != '\0' )
      ch_printf_color( ch, "&cOwner: &Y%s\r\n", obj->owner );
   ch_printf_color( ch, "&cWear flags : &w%s\r\n", flag_string( obj->wear_flags, w_flags ) );
   ch_printf_color( ch, "&cExtra flags: &w%s\r\n", ext_flag_string( &obj->extra_flags, o_flags ) );
   ch_printf_color( ch, "&cMagic flags: &w%s\r\n", magic_bit_name( obj->magic_flags ) );
   ch_printf_color( ch, "&cNumber: &w%d/%d   ", 1, get_obj_number( obj ) );
   ch_printf_color( ch, "&cWeight: &w%d/%d   ", obj->weight, get_obj_weight( obj ) );
   ch_printf_color( ch, "&cLayers: &w%d   ", obj->pIndexData->layers );
   ch_printf_color( ch, "&cWear_loc: &w%d\r\n", obj->wear_loc );
   ch_printf_color( ch, "&cCost: &Y%d  ", obj->cost );
   ch_printf_color( ch, "&cRent: &w%d  ", obj->pIndexData->rent );
   send_to_char_color( "&cTimer: ", ch );
   if( obj->timer > 0 )
      ch_printf_color( ch, "&R%d  ", obj->timer );
   else
      ch_printf_color( ch, "&w%d  ", obj->timer );
   ch_printf_color( ch, "&cLevel: &P%d\r\n", obj->level );
   ch_printf_color( ch, "&cIn room: &w%d  ", obj->in_room == NULL ? 0 : obj->in_room->vnum );
   ch_printf_color( ch, "&cIn object: &w%s  ", obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr );
   ch_printf_color( ch, "&cCarried by: &C%s\r\n", obj->carried_by == NULL ? "(none)" : obj->carried_by->name );
   ch_printf_color( ch, "&cIndex Values : &w%d %d %d %d %d %d.\r\n",
                    obj->pIndexData->value[0], obj->pIndexData->value[1],
                    obj->pIndexData->value[2], obj->pIndexData->value[3],
                    obj->pIndexData->value[4], obj->pIndexData->value[5] );
   ch_printf_color( ch, "&cObject Values: &w%d %d %d %d %d %d.\r\n",
                    obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4], obj->value[5] );
   if( obj->pIndexData->first_extradesc )
   {
      EXTRA_DESCR_DATA *ed;
      send_to_char( "Primary description keywords:   '", ch );
      for( ed = obj->pIndexData->first_extradesc; ed; ed = ed->next )
      {
         send_to_char( ed->keyword, ch );
         if( ed->next )
            send_to_char( " ", ch );
      }
      send_to_char( "'.\r\n", ch );
   }
   if( obj->first_extradesc )
   {
      EXTRA_DESCR_DATA *ed;
      send_to_char( "Secondary description keywords: '", ch );
      for( ed = obj->first_extradesc; ed; ed = ed->next )
      {
         send_to_char( ed->keyword, ch );
         if( ed->next )
            send_to_char( " ", ch );
      }
      send_to_char( "'.\r\n", ch );
   }
   for( paf = obj->first_affect; paf; paf = paf->next )
      ch_printf_color( ch, "&cAffects &w%s &cby &w%d. (extra)\r\n", affect_loc_name( paf->location ), paf->modifier );
   for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
      ch_printf_color( ch, "&cAffects &w%s &cby &w%d.\r\n", affect_loc_name( paf->location ), paf->modifier );
   return;
}

void do_vstat( CHAR_DATA* ch, const char* argument)
{
   VARIABLE_DATA *vd;
   CHAR_DATA *victim;

   if( argument[0] == '\0' )
   {
      send_to_char( "Vstat whom?\r\n", ch );
      return;
   }

   if( !( victim = get_char_world( ch, argument ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( get_trust( ch ) < get_trust( victim ) )
   {
      send_to_char( "Their godly glow prevents you from getting a good look.\r\n", ch );
      return;
   }

   if( !victim->variables )
   {
      send_to_char( "They have no variables currently assigned to them.\r\n", ch );
      return;
   }

   pager_printf( ch, "\r\n&cName: &C%-20s &cRoom : &w%-10d", victim->name,
                 victim->in_room == NULL ? 0 : victim->in_room->vnum );
   pager_printf( ch, "\r\n&cVariables:\r\n" );

   /*
    * Variables:
    * Vnum:           Tag:                 Type:     Timer:
    * Flags:
    * Data:
    */
   for( vd = victim->variables; vd; vd = vd->next )
   {
      pager_printf( ch, "  &cVnum: &W%-10d &cTag: &W%-15s &cTimer: &W%d\r\n", vd->vnum, vd->tag, vd->timer );
      pager_printf( ch, "  &cType: " );
      if( vd->data )
      {
         switch ( vd->type )
         {
            case vtSTR:
               if( vd->data )
                  pager_printf( ch, "&CString     &cData: &W%s", ( char * )vd->data );
               break;

            case vtINT:
               if( vd->data )
                  pager_printf( ch, "&CInteger    &cData: &W%ld", ( long )vd->data );
               break;

            case vtXBIT:
               if( vd->data )
               {
                  char buf[MAX_STRING_LENGTH];
                  int started = 0;
                  int x;

                  buf[0] = '\0';
                  for( x = MAX_BITS; x > 0; --x )
                  {
                     if( !started && xIS_SET( *( EXT_BV * ) vd->data, x ) )
                        started = x;
                  }

                  for( x = 1; x <= started; x++ )
                     strcat( buf, xIS_SET( *( EXT_BV * ) vd->data, x ) ? "1 " : "0 " );

                  if( buf[0] != '\0' )
                     buf[strlen( buf ) - 1] = '\0';
                  pager_printf( ch, "&CXBIT       &cData: &w[&W%s&w]", buf );
               }
               break;
         }
      }
      else
         send_to_pager( "&CNo Data", ch );

      send_to_pager( "\r\n\r\n", ch );
   }
   return;
}

void do_mstat( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char hpbuf[MAX_STRING_LENGTH];
   char mnbuf[MAX_STRING_LENGTH];
   char mvbuf[MAX_STRING_LENGTH];
   char bdbuf[MAX_STRING_LENGTH];
   AFFECT_DATA *paf;
   CHAR_DATA *victim;
   SKILLTYPE *skill;
   VARIABLE_DATA *vd;
   int x;

   set_pager_color( AT_CYAN, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_pager( "Mstat whom?\r\n", ch );
      return;
   }
   if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
      mudstrlcpy( arg, argument, MAX_INPUT_LENGTH );

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_pager( "They aren't here.\r\n", ch );
      return;
   }
   if( get_trust( ch ) < get_trust( victim ) && !IS_NPC( victim ) )
   {
      set_pager_color( AT_IMMORT, ch );
      send_to_pager( "Their godly glow prevents you from getting a good look.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) && get_trust( ch ) < LEVEL_GREATER && xIS_SET( victim->act, ACT_STATSHIELD ) )
   {
      set_pager_color( AT_IMMORT, ch );
      send_to_pager( "Their godly glow prevents you from getting a good look.\r\n", ch );
      return;
   }

   pager_printf_color( ch, "\r\n&c%s: &C%-20s", IS_NPC( victim ) ? "Mobile name" : "Name", victim->name );
   if( !IS_NPC( victim ) )
      pager_printf_color( ch, "&cStatus : &w%-10s", CAN_PKILL( victim ) ? "Deadly" :
                          IS_PKILL( victim ) ? "Pre-Deadly" : "Non-Deadly" );
   if( !IS_NPC( victim ) && victim->pcdata->clan )
      pager_printf_color( ch, "   &c%s: &w%s",
                          victim->pcdata->clan->clan_type == CLAN_ORDER ? "Order" :
                          victim->pcdata->clan->clan_type == CLAN_GUILD ? "Guild" : "Clan", victim->pcdata->clan->name );
   send_to_pager( "\r\n", ch );
   if( get_trust( ch ) >= LEVEL_GOD && !IS_NPC( victim ) && victim->desc )
      pager_printf_color( ch, "&cHost: &w%s   Descriptor: %d  &cTrust: &w%d  &cAuthBy: &w%s\r\n",
                          victim->desc->host, victim->desc->descriptor,
                          victim->trust, victim->pcdata->authed_by[0] != '\0' ? victim->pcdata->authed_by : "(unknown)" );
   if( !IS_NPC( victim ) && victim->pcdata->release_date != 0 )
      pager_printf_color( ch, "&cHelled until %24.24s by %s.\r\n",
                          ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );
   pager_printf_color( ch, "&cVnum: &w%-5d    &cSex: &w%-6s    &cRoom: &w%-5d    &cCount: &w%d   &cKilled: &w%d\r\n",
                       IS_NPC( victim ) ? victim->pIndexData->vnum : 0,
                       victim->sex == SEX_MALE ? "male" :
                       victim->sex == SEX_FEMALE ? "female" : "neutral",
                       victim->in_room == NULL ? 0 : victim->in_room->vnum,
                       IS_NPC( victim ) ? victim->pIndexData->count : 1,
                       IS_NPC( victim ) ? victim->pIndexData->killed : victim->pcdata->mdeaths + victim->pcdata->pdeaths );
   pager_printf_color( ch,
                       "&cStr: &C%2d&c )( Int: &C%2d&c )( Wis: &C%2d&c )( Dex: &C%2d&c )( Con: &C%2d&c )( Cha: &C%2d&c )( Lck: &C%2d&c\r\n",
                       get_curr_str( victim ), get_curr_int( victim ), get_curr_wis( victim ), get_curr_dex( victim ),
                       get_curr_con( victim ), get_curr_cha( victim ), get_curr_lck( victim ) );
   pager_printf_color( ch, "&cLevel   : &P%-2d              ", victim->level );
   pager_printf_color( ch, "&cclass  : &w%-2.2d/%-10s   &cRace      : &w%-2.2d/%-10s\r\n",
                       victim->Class,
                       IS_NPC( victim ) ? victim->Class < MAX_NPC_CLASS && victim->Class >= 0 ?
                       npc_class[victim->Class] : "unknown" : victim->Class < MAX_PC_CLASS &&
                       class_table[victim->Class]->who_name &&
                       class_table[victim->Class]->who_name[0] != '\0' ?
                       class_table[victim->Class]->who_name : "unknown",
                       victim->race,
                       IS_NPC( victim ) ? victim->race < MAX_NPC_RACE && victim->race >= 0 ?
                       npc_race[victim->race] : "unknown" : victim->race < MAX_PC_RACE &&
                       race_table[victim->race]->race_name &&
                       race_table[victim->race]->race_name[0] != '\0' ? race_table[victim->race]->race_name : "unknown" );
   snprintf( hpbuf, MAX_STRING_LENGTH, "%d/%d", victim->hit, victim->max_hit );
   snprintf( mnbuf, MAX_STRING_LENGTH, "%d/%d", victim->mana, victim->max_mana );
   snprintf( mvbuf, MAX_STRING_LENGTH, "%d/%d", victim->move, victim->max_move );
   if( IS_VAMPIRE( victim ) && !IS_NPC( victim ) )
   {
      snprintf( bdbuf, MAX_STRING_LENGTH, "%d/%d", victim->pcdata->condition[COND_BLOODTHIRST], 10 + victim->level );
      pager_printf_color( ch, "&cHps     : &w%-12s    &cBlood  : &w%-12s    &cMove      : &w%-12s\r\n",
                          hpbuf, bdbuf, mvbuf );
   }
   else
      pager_printf_color( ch, "&cHps     : &w%-12s    &cMana   : &w%-12s    &cMove      : &w%-12s\r\n",
                          hpbuf, mnbuf, mvbuf );
   pager_printf_color( ch, "&cHitroll : &C%-5d           &cAlign  : &w%-5d           &cArmorclass: &w%d\r\n",
                       GET_HITROLL( victim ), victim->alignment, GET_AC( victim ) );
   pager_printf_color( ch, "&cDamroll : &C%-5d           &cWimpy  : &w%-5d           &cPosition  : &w%d\r\n",
                       GET_DAMROLL( victim ), victim->wimpy, victim->position );
   pager_printf_color( ch, "&cFighting: &w%-13s   &cMaster : &w%-13s   &cLeader    : &w%s\r\n",
                       victim->fighting ? victim->fighting->who->name : "(none)",
                       victim->master ? victim->master->name : "(none)", victim->leader ? victim->leader->name : "(none)" );
   if( IS_NPC( victim ) )
      pager_printf_color( ch, "&cHating  : &w%-13s   &cHunting: &w%-13s   &cFearing   : &w%s\r\n",
                          victim->hating ? victim->hating->name : "(none)",
                          victim->hunting ? victim->hunting->name : "(none)",
                          victim->fearing ? victim->fearing->name : "(none)" );
   else
      pager_printf_color( ch, "&cDeity   : &w%-13s&w   &cFavor  : &w%-5d           &cGlory     : &w%-d (%d)\r\n",
                          victim->pcdata->deity ? victim->pcdata->deity->name : "(none)",
                          victim->pcdata->favor, victim->pcdata->quest_curr, victim->pcdata->quest_accum );
   if( IS_NPC( victim ) )
      pager_printf_color( ch,
                          "&cMob hitdie : &C%dd%d+%d    &cMob damdie : &C%dd%d+%3d    &cIndex damdie : &C%dd%d+%3d\r\n&cNumAttacks : &C%d\r\n",
                          victim->pIndexData->hitnodice, victim->pIndexData->hitsizedice, victim->pIndexData->hitplus,
                          victim->barenumdie, victim->baresizedie, victim->damplus, victim->pIndexData->damnodice,
                          victim->pIndexData->damsizedice, victim->pIndexData->damplus, victim->numattacks );
   pager_printf_color( ch, "&cMentalState: &w%-3d   &cEmotionalState: &w%-3d   ", victim->mental_state,
                       victim->emotional_state );
   if( !IS_NPC( victim ) )
      pager_printf_color( ch, "&cThirst: &w%d   &cFull: &w%d   &cDrunk: &w%d\r\n",
                          victim->pcdata->condition[COND_THIRST],
                          victim->pcdata->condition[COND_FULL], victim->pcdata->condition[COND_DRUNK] );
   else
      send_to_pager( "\r\n", ch );
   pager_printf_color( ch, "&cSave versus: &w%d %d %d %d %d       &cItems: &w(%d/%d)  &cWeight &w(%d/%d)\r\n",
                       victim->saving_poison_death,
                       victim->saving_wand,
                       victim->saving_para_petri,
                       victim->saving_breath,
                       victim->saving_spell_staff,
                       victim->carry_number, can_carry_n( victim ), victim->carry_weight, can_carry_w( victim ) );
   pager_printf_color( ch, "&cYear: &w%-5d  &cSecs: &w%d  &cTimer: &w%d  &cGold: &Y%d\r\n",
                       calculate_age( victim ), ( int )victim->played, victim->timer, victim->gold );
   if( get_timer( victim, TIMER_PKILLED ) )
      pager_printf_color( ch, "&cTimerPkilled:  &R%d\r\n", get_timer( victim, TIMER_PKILLED ) );
   if( get_timer( victim, TIMER_RECENTFIGHT ) )
      pager_printf_color( ch, "&cTimerRecentfight:  &R%d\r\n", get_timer( victim, TIMER_RECENTFIGHT ) );
   if( get_timer( victim, TIMER_ASUPRESSED ) )
      pager_printf_color( ch, "&cTimerAsupressed:  &R%d\r\n", get_timer( victim, TIMER_ASUPRESSED ) );
   if( IS_NPC( victim ) )
      pager_printf_color( ch, "&cAct Flags  : &w%s\r\n", ext_flag_string( &victim->act, act_flags ) );
   else
   {
      pager_printf_color( ch, "&cPlayerFlags: &w%s\r\n", ext_flag_string( &victim->act, plr_flags ) );
      pager_printf_color( ch, "&cPcflags    : &w%s\r\n", flag_string( victim->pcdata->flags, pc_flags ) );
      if( victim->pcdata->nuisance )
      {
         pager_printf_color( ch, "&RNuisance   &cStage: (&R%d&c/%d)  Power:  &w%d  &cTime:  &w%s.\r\n",
                             victim->pcdata->nuisance->flags, MAX_NUISANCE_STAGE, victim->pcdata->nuisance->power,
                             ctime( &victim->pcdata->nuisance->set_time ) );
      }
   }
   if( victim->morph )
   {
      if( victim->morph->morph )
         pager_printf_color( ch, "&cMorphed as : (&C%d&c) &C%s    &cTimer: &C%d\r\n",
                             victim->morph->morph->vnum, victim->morph->morph->short_desc, victim->morph->timer );
      else
         pager_printf_color( ch, "&cMorphed as: Morph was deleted.\r\n" );
   }
   pager_printf_color( ch, "&cAffected by: &C%s\r\n", affect_bit_name( &victim->affected_by ) );
   pager_printf_color( ch, "&cSpeaks: &w%d   &cSpeaking: &w%d   &cExperience: &w%d",
                       victim->speaks, victim->speaking, victim->exp );
   if( !IS_NPC( victim ) && victim->wait )
      pager_printf_color( ch, "   &cWaitState: &R%d\r\n", victim->wait / 12 );
   else
      send_to_pager( "\r\n", ch );
   send_to_pager_color( "&cLanguages  : &w", ch );
   for( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
      if( knows_language( victim, lang_array[x], victim ) || ( IS_NPC( victim ) && victim->speaks == 0 ) )
      {
         if( IS_SET( lang_array[x], victim->speaking ) || ( IS_NPC( victim ) && !victim->speaking ) )
            set_pager_color( AT_RED, ch );
         send_to_pager( lang_names[x], ch );
         send_to_pager( " ", ch );
         set_pager_color( AT_PLAIN, ch );
      }
      else if( IS_SET( lang_array[x], victim->speaking ) || ( IS_NPC( victim ) && !victim->speaking ) )
      {
         set_pager_color( AT_PINK, ch );
         send_to_pager( lang_names[x], ch );
         send_to_pager( " ", ch );
         set_pager_color( AT_PLAIN, ch );
      }
   send_to_pager( "\r\n", ch );
   if( victim->pcdata && victim->pcdata->bestowments && victim->pcdata->bestowments[0] != '\0' )
      pager_printf_color( ch, "&cBestowments: &w%s\r\n", victim->pcdata->bestowments );
   if( IS_NPC( victim ) )
      pager_printf_color( ch, "&cShortdesc  : &w%s\r\n&cLongdesc   : &w%s",
                          victim->short_descr[0] != '\0' ? victim->short_descr : "(none set)",
                          victim->long_descr[0] != '\0' ? victim->long_descr : "(none set)\r\n" );
   else
   {
      if( victim->short_descr[0] != '\0' )
         pager_printf_color( ch, "&cShortdesc  : &w%s\r\n", victim->short_descr );
      if( victim->long_descr[0] != '\0' )
         pager_printf_color( ch, "&cLongdesc   : &w%s\r\n", victim->long_descr );
   }
   if( IS_NPC( victim ) && victim->spec_fun )
      pager_printf_color( ch, "&cMobile has spec fun: &w%s\r\n", victim->spec_funname );
   if( IS_NPC( victim ) )
      pager_printf_color( ch, "&cBody Parts : &w%s\r\n", flag_string( victim->xflags, part_flags ) );
   if( victim->resistant > 0 )
      pager_printf_color( ch, "&cResistant  : &w%s\r\n", flag_string( victim->resistant, ris_flags ) );
   if( victim->immune > 0 )
      pager_printf_color( ch, "&cImmune     : &w%s\r\n", flag_string( victim->immune, ris_flags ) );
   if( victim->susceptible > 0 )
      pager_printf_color( ch, "&cSusceptible: &w%s\r\n", flag_string( victim->susceptible, ris_flags ) );
   if( IS_NPC( victim ) )
   {
      pager_printf_color( ch, "&cAttacks    : &w%s\r\n", ext_flag_string( &victim->attacks, attack_flags ) );
      pager_printf_color( ch, "&cDefenses   : &w%s\r\n", ext_flag_string( &victim->defenses, defense_flags ) );
   }

   for( paf = victim->first_affect; paf; paf = paf->next )
   {
      if( ( skill = get_skilltype( paf->type ) ) != NULL )
         pager_printf_color( ch,
                             "&c%s: &w'%s' mods %s by %d for %d rnds with bits %s.",
                             skill_tname[skill->type],
                             skill->name,
                             affect_loc_name( paf->location ),
                             paf->modifier, paf->duration, affect_bit_name( &paf->bitvector ) );
      send_to_pager( "\r\n", ch );
   }

   if( victim->variables )
   {
      send_to_pager( "&cVariables  : &w", ch );
      for( vd = victim->variables; vd; vd = vd->next )
      {
         pager_printf( ch, "%s:%d", vd->tag, vd->vnum );
         switch ( vd->type )
         {
            case vtSTR:
               if( vd->data )
                  pager_printf( ch, "=%s", ( char * )vd->data );
               break;

            case vtINT:
               if( vd->data )
                  pager_printf( ch, "=%ld", ( long )vd->data );
               break;

            case vtXBIT:
               if( vd->data )
               {
                  char buf[MAX_STRING_LENGTH];
                  int started = 0;

                  buf[0] = '\0';
                  for( x = MAX_BITS; x > 0; --x )
                  {
                     if( !started && xIS_SET( *( EXT_BV * ) vd->data, x ) )
                        started = x;
                  }

                  for( x = 1; x <= started; x++ )
                     strcat( buf, xIS_SET( *( EXT_BV * ) vd->data, x ) ? "1 " : "0 " );

                  if( buf[0] != '\0' )
                     buf[strlen( buf ) - 1] = '\0';
                  pager_printf( ch, "=[%s]", buf );
               }
         }
         if( vd->next )
            send_to_pager( "  ", ch );
      }
      send_to_pager( "\r\n", ch );
   }
   return;
}

void do_mfind( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   MOB_INDEX_DATA *pMobIndex;
   int hash;
   int nMatch;
   bool fAll;

   set_pager_color( AT_PLAIN, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Mfind whom?\r\n", ch );
      return;
   }

   fAll = !str_cmp( arg, "all" );
   nMatch = 0;

   /*
    * This goes through all the hash entry points (1024), and is therefore
    * much faster, though you won't get your vnums in order... oh well. :)
    *
    * Tests show that Furey's method will usually loop 32,000 times, calling
    * get_mob_index()... which loops itself, an average of 1-2 times...
    * So theoretically, the above routine may loop well over 40,000 times,
    * and my routine bellow will loop for as many index_mobiles are on
    * your mud... likely under 3000 times.
    * -Thoric
    */
   for( hash = 0; hash < MAX_KEY_HASH; hash++ )
      for( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
         if( fAll || nifty_is_name( arg, pMobIndex->player_name ) )
         {
            nMatch++;
            pager_printf( ch, "[%5d] %s\r\n", pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
         }

   if( nMatch )
      pager_printf( ch, "Number of matches: %d\n", nMatch );
   else
      send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );
   return;
}

void do_ofind( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_INDEX_DATA *pObjIndex;
   int hash;
   int nMatch;
   bool fAll;

   set_pager_color( AT_PLAIN, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Ofind what?\r\n", ch );
      return;
   }

   fAll = !str_cmp( arg, "all" );
   nMatch = 0;

   /*
    * This goes through all the hash entry points (1024), and is therefore
    * much faster, though you won't get your vnums in order... oh well. :)
    *
    * Tests show that Furey's method will usually loop 32,000 times, calling
    * get_obj_index()... which loops itself, an average of 2-3 times...
    * So theoretically, the above routine may loop well over 50,000 times,
    * and my routine bellow will loop for as many index_objects are on
    * your mud... likely under 3000 times.
    * -Thoric
    */
   for( hash = 0; hash < MAX_KEY_HASH; hash++ )
      for( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
         if( fAll || nifty_is_name( arg, pObjIndex->name ) )
         {
            nMatch++;
            pager_printf( ch, "[%5d] %s\r\n", pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
         }

   if( nMatch )
      pager_printf( ch, "Number of matches: %d\n", nMatch );
   else
      send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );
   return;
}

void do_mwhere( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   bool found;

   set_pager_color( AT_PLAIN, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Mwhere whom?\r\n", ch );
      return;
   }

   found = FALSE;
   for( victim = first_char; victim; victim = victim->next )
   {
      if( IS_NPC( victim ) && victim->in_room && nifty_is_name( arg, victim->name ) )
      {
         found = TRUE;
         pager_printf( ch, "[%5d] %-28s [%5d] %s\r\n",
                       victim->pIndexData->vnum, victim->short_descr, victim->in_room->vnum, victim->in_room->name );
      }
   }

   if( !found )
      act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
   return;
}

void do_gwhere( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   bool pmobs = FALSE;
   int low = 1, high = MAX_LEVEL, count = 0;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] != '\0' )
   {
      if( arg1[0] == '\0' || arg2[0] == '\0' )
      {
         send_to_pager_color( "\r\n&wSyntax:  gwhere | gwhere <low> <high> | gwhere <low> <high> mobs\r\n", ch );
         return;
      }
      low = atoi( arg1 );
      high = atoi( arg2 );
   }
   if( low < 1 || high < low || low > high || high > MAX_LEVEL )
   {
      send_to_pager_color( "&wInvalid level range.\r\n", ch );
      return;
   }
   argument = one_argument( argument, arg3 );
   if( !str_cmp( arg3, "mobs" ) )
      pmobs = TRUE;

   pager_printf_color( ch, "\r\n&cGlobal %s locations:&w\r\n", pmobs ? "mob" : "player" );
   if( !pmobs )
   {
      for( d = first_descriptor; d; d = d->next )
         if( ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
             && ( victim = d->character ) != NULL && !IS_NPC( victim ) && victim->in_room
             && can_see( ch, victim ) && victim->level >= low && victim->level <= high )
         {
            pager_printf_color( ch, "&c(&C%2d&c) &w%-12.12s   [%-5d - %-19.19s]   &c%-25.25s\r\n",
                                victim->level, victim->name, victim->in_room->vnum, victim->in_room->area->name,
                                victim->in_room->name );
            count++;
         }
   }
   else
   {
      for( victim = first_char; victim; victim = victim->next )
         if( IS_NPC( victim ) && victim->in_room && can_see( ch, victim ) && victim->level >= low && victim->level <= high )
         {
            pager_printf_color( ch, "&c(&C%2d&c) &w%-12.12s   [%-5d - %-19.19s]   &c%-25.25s\r\n",
                                victim->level, victim->name, victim->in_room->vnum, victim->in_room->area->name,
                                victim->in_room->name );
            count++;
         }
   }
   pager_printf_color( ch, "&c%d %s found.\r\n", count, pmobs ? "mobs" : "characters" );
   return;
}

void do_gfighting( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   bool pmobs = FALSE, phating = FALSE, phunting = FALSE;
   int low = 1, high = MAX_LEVEL, count = 0;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] != '\0' )
   {
      if( arg1[0] == '\0' || arg2[0] == '\0' )
      {
         send_to_pager_color( "\r\n&wSyntax:  gfighting | gfighting <low> <high> | gfighting <low> <high> mobs\r\n", ch );
         return;
      }
      low = atoi( arg1 );
      high = atoi( arg2 );
   }
   if( low < 1 || high < low || low > high || high > MAX_LEVEL )
   {
      send_to_pager_color( "&wInvalid level range.\r\n", ch );
      return;
   }
   argument = one_argument( argument, arg3 );
   if( !str_cmp( arg3, "mobs" ) )
      pmobs = TRUE;
   else if( !str_cmp( arg3, "hating" ) )
      phating = TRUE;
   else if( !str_cmp( arg3, "hunting" ) )
      phunting = TRUE;

   pager_printf_color( ch, "\r\n&cGlobal %s conflict:\r\n", pmobs ? "mob" : "character" );
   if( !pmobs && !phating && !phunting )
   {
      for( d = first_descriptor; d; d = d->next )
         if( ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
             && ( victim = d->character ) != NULL && !IS_NPC( victim ) && victim->in_room
             && can_see( ch, victim ) && victim->fighting && victim->level >= low && victim->level <= high )
         {
            pager_printf_color( ch, "&w%-12.12s &C|%2d &wvs &C%2d| &w%-16.16s [%5d]  &c%-20.20s [%5d]\r\n",
                                victim->name, victim->level, victim->fighting->who->level,
                                IS_NPC( victim->fighting->who ) ? victim->fighting->who->short_descr : victim->fighting->
                                who->name, IS_NPC( victim->fighting->who ) ? victim->fighting->who->pIndexData->vnum : 0,
                                victim->in_room->area->name, victim->in_room == NULL ? 0 : victim->in_room->vnum );
            count++;
         }
   }
   else if( !phating && !phunting )
   {
      for( victim = first_char; victim; victim = victim->next )
         if( IS_NPC( victim )
             && victim->in_room && can_see( ch, victim )
             && victim->fighting && victim->level >= low && victim->level <= high )
         {
            pager_printf_color( ch, "&w%-12.12s &C|%2d &wvs &C%2d| &w%-16.16s [%5d]  &c%-20.20s [%5d]\r\n",
                                victim->name, victim->level, victim->fighting->who->level,
                                IS_NPC( victim->fighting->who ) ? victim->fighting->who->short_descr : victim->fighting->
                                who->name, IS_NPC( victim->fighting->who ) ? victim->fighting->who->pIndexData->vnum : 0,
                                victim->in_room->area->name, victim->in_room == NULL ? 0 : victim->in_room->vnum );
            count++;
         }
   }
   else if( !phunting && phating )
   {
      for( victim = first_char; victim; victim = victim->next )
         if( IS_NPC( victim )
             && victim->in_room && can_see( ch, victim ) && victim->hating && victim->level >= low && victim->level <= high )
         {
            pager_printf_color( ch, "&w%-12.12s &C|%2d &wvs &C%2d| &w%-16.16s [%5d]  &c%-20.20s [%5d]\r\n",
                                victim->name, victim->level, victim->hating->who->level, IS_NPC( victim->hating->who ) ?
                                victim->hating->who->short_descr : victim->hating->who->name, IS_NPC( victim->hating->who ) ?
                                victim->hating->who->pIndexData->vnum : 0, victim->in_room->area->name,
                                victim->in_room == NULL ? 0 : victim->in_room->vnum );
            count++;
         }
   }
   else if( phunting )
   {
      for( victim = first_char; victim; victim = victim->next )
         if( IS_NPC( victim )
             && victim->in_room && can_see( ch, victim )
             && victim->hunting && victim->level >= low && victim->level <= high )
         {
            pager_printf_color( ch, "&w%-12.12s &C|%2d &wvs &C%2d| &w%-16.16s [%5d]  &c%-20.20s [%5d]\r\n",
                                victim->name, victim->level, victim->hunting->who->level, IS_NPC( victim->hunting->who ) ?
                                victim->hunting->who->short_descr : victim->hunting->who->name,
                                IS_NPC( victim->hunting->who ) ? victim->hunting->who->pIndexData->vnum : 0,
                                victim->in_room->area->name, victim->in_room == NULL ? 0 : victim->in_room->vnum );
            count++;
         }
   }
   pager_printf_color( ch, "&c%d %s conflicts located.\r\n", count, pmobs ? "mob" : "character" );
   return;
}

/* Added 'show' argument for lowbie imms without ostat -- Blodkai */
/* Made show the default action :) Shaddai */
/* Trimmed size, added vict info, put lipstick on the pig -- Blod */
void do_bodybag( CHAR_DATA* ch, const char* argument)
{
   char buf2[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *owner;
   OBJ_DATA *obj;
   bool found = FALSE, bag = FALSE;

   argument = one_argument( argument, arg1 );
   if( arg1[0] == '\0' )
   {
      send_to_char_color( "&PSyntax:  bodybag <character> | bodybag <character> yes/bag/now\r\n", ch );
      return;
   }

   snprintf( buf2, MAX_STRING_LENGTH, "the corpse of %s", arg1 );
   argument = one_argument( argument, arg2 );

   if( arg2[0] != '\0' && ( str_cmp( arg2, "yes" ) && str_cmp( arg2, "bag" ) && str_cmp( arg2, "now" ) ) )
   {
      send_to_char_color( "\r\n&PSyntax:  bodybag <character> | bodybag <character> yes/bag/now\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "yes" ) || !str_cmp( arg2, "bag" ) || !str_cmp( arg2, "now" ) )
      bag = TRUE;

   pager_printf_color( ch, "\r\n&P%s remains of %s ... ", bag ? "Retrieving" : "Searching for", capitalize( arg1 ) );
   for( obj = first_object; obj; obj = obj->next )
   {
      if( obj->in_room && obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC && !str_cmp( buf2, obj->short_descr ) )
      {
         send_to_pager( "\r\n", ch );
         found = TRUE;
         pager_printf_color( ch, "&P%s:  %s%-12.12s %s  &PIn:  &w%-22.22s  &P[&w%5d&P]   &PTimer:  %s%2d",
                             bag ? "Bagging" : "Corpse",
                             bag ? "&Y" : "&w",
                             capitalize( arg1 ),
                             IS_OBJ_STAT( obj, ITEM_CLANCORPSE ) ? "&RPK" : "&R  ",
                             obj->in_room->area->name,
                             obj->in_room->vnum,
                             obj->timer < 1 ? "&w" : obj->timer < 5 ? "&R" : obj->timer < 10 ? "&Y" : "&w", obj->timer );
         if( bag )
         {
            obj_from_room( obj );
            obj = obj_to_char( obj, ch );
            obj->timer = -1;
            save_char_obj( ch );
         }
      }
   }
   if( !found )
   {
      send_to_pager_color( "&Pno corpse was found.\r\n", ch );
      return;
   }
   send_to_pager( "\r\n", ch );
   for( owner = first_char; owner; owner = owner->next )
   {
      if( IS_NPC( owner ) )
         continue;
      if( can_see( ch, owner ) && !str_cmp( arg1, owner->name ) )
         break;
   }
   if( owner == NULL )
   {
      pager_printf_color( ch, "&P%s is not currently online.\r\n", capitalize( arg1 ) );
      return;
   }
   if( owner->pcdata->deity )
      pager_printf_color( ch, "&P%s (%d) has %d favor with %s (needed to supplicate: %d)\r\n",
                          owner->name,
                          owner->level, owner->pcdata->favor, owner->pcdata->deity->name, owner->pcdata->deity->scorpse );
   else
      pager_printf_color( ch, "&P%s (%d) has no deity.\r\n", owner->name, owner->level );
   return;
}

/* New owhere by Altrag, 03/14/96 */
void do_owhere( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   bool found;
   int icnt = 0;

   set_pager_color( AT_PLAIN, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Owhere what?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   if( arg1[0] != '\0' && !str_prefix( arg1, "nesthunt" ) )
   {
      if( !( obj = get_obj_world( ch, arg ) ) )
      {
         send_to_char( "Nesthunt for what object?\r\n", ch );
         return;
      }
      for( ; obj->in_obj; obj = obj->in_obj )
      {
         pager_printf( ch, "[%5d] %-28s in object [%5d] %s\r\n",
                       obj->pIndexData->vnum, obj_short( obj ), obj->in_obj->pIndexData->vnum, obj->in_obj->short_descr );
         ++icnt;
      }
      snprintf( buf, MAX_STRING_LENGTH, "[%5d] %-28s in ", obj->pIndexData->vnum, obj_short( obj ) );
      if( obj->carried_by )
         snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "invent [%5d] %s\r\n",
                   ( IS_NPC( obj->carried_by ) ? obj->carried_by->pIndexData->vnum : 0 ), PERS( obj->carried_by, ch ) );
      else if( obj->in_room )
         snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "room   [%5d] %s\r\n", obj->in_room->vnum,
                   obj->in_room->name );
      else if( obj->in_obj )
      {
         bug( "%s", "do_owhere: obj->in_obj after NULL!" );
         mudstrlcat( buf, "object??\r\n", MAX_STRING_LENGTH );
      }
      else
      {
         bug( "%s", "do_owhere: object doesnt have location!" );
         mudstrlcat( buf, "nowhere??\r\n", MAX_STRING_LENGTH );
      }
      send_to_pager( buf, ch );
      ++icnt;
      pager_printf( ch, "Nested %d levels deep.\r\n", icnt );
      return;
   }

   found = FALSE;
   for( obj = first_object; obj; obj = obj->next )
   {
      if( !nifty_is_name( arg, obj->name ) )
         continue;
      found = TRUE;

      snprintf( buf, MAX_STRING_LENGTH, "(%3d) [%5d] %-28s in ", ++icnt, obj->pIndexData->vnum, obj_short( obj ) );
      if( obj->carried_by )
         snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "invent [%5d] %s\r\n",
                   ( IS_NPC( obj->carried_by ) ? obj->carried_by->pIndexData->vnum : 0 ), PERS( obj->carried_by, ch ) );
      else if( obj->in_room )
         snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "room   [%5d] %s\r\n", obj->in_room->vnum,
                   obj->in_room->name );
      else if( obj->in_obj )
         snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "object [%5d] %s\r\n",
                   obj->in_obj->pIndexData->vnum, obj_short( obj->in_obj ) );
      else
      {
         bug( "%s", "do_owhere: object doesnt have location!" );
         mudstrlcat( buf, "nowhere??\r\n", MAX_STRING_LENGTH );
      }
      send_to_pager( buf, ch );
   }

   if( !found )
      act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
   else
      pager_printf( ch, "%d matches.\r\n", icnt );
   return;
}

/*
 * "Claim" an object.  Will allow an immortal to "grab" an object no matter
 * where it is hiding.  ie: from a player's inventory, from deep inside
 * a container, from a mobile, from anywhere.			-Thoric
 */
void do_oclaim( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   char arg3[MAX_INPUT_LENGTH];
   char *who = NULL;
   CHAR_DATA *vch = NULL;
   OBJ_DATA *obj;
   bool silently = FALSE, found = FALSE;
   int number, count, vnum;

   number = number_argument( argument, arg );
   argument = arg;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: oclaim <object> [from who] [+silent]\r\n", ch );
      return;
   }
   if( arg3[0] == '\0' )
   {
      if( arg2[0] != '\0' )
      {
         if( !str_cmp( arg2, "+silent" ) )
            silently = TRUE;
         else
            who = arg2;
      }
   }
   else
   {
      who = arg2;
      if( !str_cmp( arg3, "+silent" ) )
         silently = TRUE;
   }

   if( who )
   {
      if( ( vch = get_char_world( ch, who ) ) == NULL )
      {
         send_to_pager( "They aren't here.\n\r", ch );
         return;
      }
      if( get_trust( ch ) < get_trust( vch ) && !IS_NPC( vch ) )
      {
         act( AT_TELL, "$n tells you, 'Keep your hands to yourself!'", vch, NULL, ch, TO_VICT );
         return;
      }
   }

   if( is_number( arg1 ) )
      vnum = atoi( arg1 );
   else
      vnum = -1;

   count = 0;
   for( obj = first_object; obj; obj = obj->next )
   {
      if( can_see_obj( ch, obj )
          && ( obj->pIndexData->vnum == vnum || nifty_is_name( arg1, obj->name ) ) && ( !vch || vch == carried_by( obj ) ) )
         if( ( count += obj->count ) >= number )
         {
            found = TRUE;
            break;
         }
   }
   if( !found && vnum != -1 )
   {
      send_to_char( "You can't find that.\r\n", ch );
      return;
   }

   count = 0;
   for( obj = first_object; obj; obj = obj->next )
   {
      if( can_see_obj( ch, obj )
          && ( obj->pIndexData->vnum == vnum || nifty_is_name_prefix( arg1, obj->name ) )
          && ( !vch || vch == carried_by( obj ) ) )
         if( ( count += obj->count ) >= number )
         {
            found = TRUE;
            break;
         }
   }

   if( !found )
   {
      send_to_char( "You can't find that.\r\n", ch );
      return;
   }

   if( !vch && ( vch = carried_by( obj ) ) != NULL )
   {
      if( get_trust( ch ) < get_trust( vch ) && !IS_NPC( vch ) )
      {
         act( AT_TELL, "$n tells you, 'Keep your hands off $p!  It's mine.'", vch, obj, ch, TO_VICT );
         act( AT_IMMORT, "$n tried to lay claim to $p from your possession!", vch, obj, ch, TO_CHAR );
         return;
      }
   }

   separate_obj( obj );
   if( obj->item_type == ITEM_PORTAL )
      remove_portal( obj );

   if( obj->carried_by )
      obj_from_char( obj );
   else if( obj->in_room )
      obj_from_room( obj );
   else if( obj->in_obj )
      obj_from_obj( obj );

   obj_to_char( obj, ch );
   if( vch )
   {
      if( !silently )
      {
         act( AT_IMMORT, "$n claims $p from you!", ch, obj, vch, TO_VICT );
         act( AT_IMMORT, "$n claims $p from $N!", ch, obj, vch, TO_NOTVICT );
         act( AT_IMMORT, "You claim $p from $N!", ch, obj, vch, TO_CHAR );
      }
      else
         act( AT_IMMORT, "You silently claim $p from $N.", ch, obj, vch, TO_CHAR );
   }
   else
   {
      if( !silently )
      {
         /*
          * notify people in the room... (not done yet) 
          */
         act( AT_IMMORT, "You claim $p!", ch, obj, NULL, TO_CHAR );
      }
      else
         act( AT_IMMORT, "You silently claim $p.", ch, obj, NULL, TO_CHAR );
   }
}

void do_reboo( CHAR_DATA* ch, const char* argument)
{
   send_to_char_color( "&YIf you want to REBOOT, spell it out.\r\n", ch );
   return;
}

void do_reboot( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *vch;

   set_char_color( AT_IMMORT, ch );

   if( str_cmp( argument, "mud now" ) && str_cmp( argument, "nosave" ) && str_cmp( argument, "and sort skill table" ) )
   {
      send_to_char( "Syntax:  'reboot mud now' or 'reboot nosave'\r\n", ch );
      return;
   }

   if( auction->item )
      do_auction( ch, "stop" );
   snprintf( buf, MAX_STRING_LENGTH, "Reboot by %s.", ch->name );
   do_echo( ch, buf );

   if( !str_cmp( argument, "and sort skill table" ) )
   {
      sort_skill_table(  );
      save_skill_table(  );
   }

   /*
    * Save all characters before booting. 
    */
   if( str_cmp( argument, "nosave" ) )
      for( vch = first_char; vch; vch = vch->next )
         if( !IS_NPC( vch ) )
            save_char_obj( vch );

   mud_down = TRUE;
   return;
}

void do_shutdow( CHAR_DATA* ch, const char* argument)
{
   send_to_char_color( "&YIf you want to SHUTDOWN, spell it out.\r\n", ch );
   return;
}

void do_shutdown( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *vch;

   set_char_color( AT_IMMORT, ch );

   if( str_cmp( argument, "mud now" ) && str_cmp( argument, "nosave" ) )
   {
      send_to_char( "Syntax:  'shutdown mud now' or 'shutdown nosave'\r\n", ch );
      return;
   }

   if( auction->item )
      do_auction( ch, "stop" );
   snprintf( buf, MAX_STRING_LENGTH, "Shutdown by %s.", ch->name );
   append_file( ch, SHUTDOWN_FILE, buf );
   mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
   do_echo( ch, buf );

   /*
    * Save all characters before booting. 
    */
   if( str_cmp( argument, "nosave" ) )
      for( vch = first_char; vch; vch = vch->next )
         if( !IS_NPC( vch ) )
            save_char_obj( vch );
   mud_down = TRUE;
   return;
}

void do_snoop( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Snoop whom?\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( !victim->desc )
   {
      send_to_char( "No descriptor to snoop.\r\n", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "Cancelling all snoops.\r\n", ch );
      for( d = first_descriptor; d; d = d->next )
         if( d->snoop_by == ch->desc )
            d->snoop_by = NULL;
      return;
   }
   if( victim->desc->snoop_by )
   {
      send_to_char( "Busy already.\r\n", ch );
      return;
   }

   /*
    * Minimum snoop level... a secret mset value
    * makes the snooper think that the victim is already being snooped
    */
   if( get_trust( victim ) >= get_trust( ch ) || ( victim->pcdata && victim->pcdata->min_snoop > get_trust( ch ) ) )
   {
      send_to_char( "Busy already.\r\n", ch );
      return;
   }

   if( ch->desc )
   {
      for( d = ch->desc->snoop_by; d; d = d->snoop_by )
         if( d->character == victim || d->original == victim )
         {
            send_to_char( "No snoop loops.\r\n", ch );
            return;
         }
   }

/*  Snoop notification for higher imms, if desired, uncomment this */
#ifdef TOOSNOOPY
   if( get_trust( victim ) > LEVEL_GOD && get_trust( ch ) < LEVEL_SUPREME )
      write_to_descriptor( victim->desc->descriptor, "\r\nYou feel like someone is watching your every move...\r\n", 0 );
#endif
   victim->desc->snoop_by = ch->desc;
   send_to_char( "Ok.\r\n", ch );
   return;
}

void do_statshield( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( IS_NPC( ch ) || get_trust( ch ) < LEVEL_GREATER )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
   if( arg[0] == '\0' )
   {
      send_to_char( "Statshield which mobile?\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "No such mobile.\r\n", ch );
      return;
   }
   if( !IS_NPC( victim ) )
   {
      send_to_char( "You can only statshield mobiles.\r\n", ch );
      return;
   }
   if( xIS_SET( victim->act, ACT_STATSHIELD ) )
   {
      xREMOVE_BIT( victim->act, ACT_STATSHIELD );
      ch_printf( ch, "You have lifted the statshield on %s.\r\n", victim->short_descr );
   }
   else
   {
      xSET_BIT( victim->act, ACT_STATSHIELD );
      ch_printf( ch, "You have applied a statshield to %s.\r\n", victim->short_descr );
   }
   return;
}


void do_switch( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Switch into whom?\r\n", ch );
      return;
   }
   if( !ch->desc )
      return;
   if( ch->desc->original )
   {
      send_to_char( "You are already switched.\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "Ok.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_STATSHIELD ) && get_trust( ch ) < LEVEL_GREATER )
   {
      set_pager_color( AT_IMMORT, ch );
      send_to_pager( "Their godly glow prevents you from getting close enough.\r\n", ch );
      return;
   }
   if( victim->desc )
   {
      send_to_char( "Character in use.\r\n", ch );
      return;
   }
   if( !IS_NPC( victim ) && ch->level < LEVEL_GREATER )
   {
      send_to_char( "You cannot switch into a player!\r\n", ch );
      return;
   }
   if( victim->switched )
   {
      send_to_char( "You can't switch into a player that is switched!\r\n", ch );
      return;
   }
   if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_FREEZE ) )
   {
      send_to_char( "You shouldn't switch into a player that is frozen!\r\n", ch );
      return;
   }
   ch->desc->character = victim;
   ch->desc->original = ch;
   victim->desc = ch->desc;
   ch->desc = NULL;
   ch->switched = victim;
   send_to_char( "Ok.\r\n", victim );
   return;
}

void do_return( CHAR_DATA* ch, const char* argument)
{

   if( !IS_NPC( ch ) && get_trust( ch ) < LEVEL_IMMORTAL )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
   set_char_color( AT_IMMORT, ch );

   if( !ch->desc )
      return;
   if( !ch->desc->original )
   {
      send_to_char( "You aren't switched.\r\n", ch );
      return;
   }

   send_to_char( "You return to your original body.\r\n", ch );

   if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_POSSESS ) )
   {
      affect_strip( ch, gsn_possess );
      xREMOVE_BIT( ch->affected_by, AFF_POSSESS );
   }

   ch->desc->character = ch->desc->original;
   ch->desc->original = NULL;
   ch->desc->character->desc = ch->desc;
   ch->desc->character->switched = NULL;
   ch->desc = NULL;
   return;
}

void do_minvoke( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   MOB_INDEX_DATA *pMobIndex;
   CHAR_DATA *victim;
   int vnum;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax:  minvoke <vnum>\r\n", ch );
      return;
   }
   if( !is_number( arg ) )
   {
      char arg2[MAX_INPUT_LENGTH];
      int hash, cnt;
      int count = number_argument( arg, arg2 );

      vnum = -1;
      for( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
         for( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
            if( nifty_is_name( arg2, pMobIndex->player_name ) && ++cnt == count )
            {
               vnum = pMobIndex->vnum;
               break;
            }
      if( vnum == -1 )
      {
         send_to_char( "No such mobile exists.\r\n", ch );
         return;
      }
   }
   else
      vnum = atoi( arg );

   if( get_trust( ch ) < LEVEL_DEMI )
   {
      AREA_DATA *pArea;

      if( IS_NPC( ch ) )
      {
         send_to_char( "Huh?\r\n", ch );
         return;
      }
      if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
      {
         send_to_char( "You must have an assigned area to invoke this mobile.\r\n", ch );
         return;
      }
      if( vnum < pArea->low_m_vnum || vnum > pArea->hi_m_vnum )
      {
         send_to_char( "That number is not in your allocated range.\r\n", ch );
         return;
      }
   }
   if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
   {
      send_to_char( "No mobile has that vnum.\r\n", ch );
      return;
   }

   victim = create_mobile( pMobIndex );
   char_to_room( victim, ch->in_room );
   act( AT_IMMORT, "$n invokes $N!", ch, NULL, victim, TO_ROOM );
   /*
    * How about seeing what we're invoking for a change. -Blodkai
    */
   ch_printf_color( ch, "&YYou invoke %s (&W#%d &Y- &W%s &Y- &Wlvl %d&Y)\r\n",
                    pMobIndex->short_descr, pMobIndex->vnum, pMobIndex->player_name, victim->level );
   return;
}

void do_oinvoke( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   int vnum, level, quantity = 1;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: oinvoke <vnum> <level> <quantity>\r\n", ch );
      return;
   }

   if( arg2[0] == '\0' )
      level = get_trust( ch );
   else
   {
      if( !is_number( arg2 ) )
      {
         send_to_char( "Syntax: oinvoke <vnum> <level>\r\n", ch );
         return;
      }
      level = atoi( arg2 );
      if( level < 0 || level > get_trust( ch ) )
      {
         send_to_char( "Limited to your trust level.\r\n", ch );
         return;
      }
   }

   if( arg3[0] != '\0' )
   {
      if( !is_number( arg3 ) )
      {
         send_to_char( "Syntax: oinvoke <vnum> <level> <quantity>\r\n", ch );
         return;
      }

      quantity = atoi( arg3 );

      if( quantity < 1 || quantity > MAX_OINVOKE_QUANTITY )
      {
         ch_printf( ch, "You must oinvoke between 1 and %d items.\r\n", MAX_OINVOKE_QUANTITY );
         return;
      }
   }

   if( !is_number( arg1 ) )
   {
      char arg[MAX_INPUT_LENGTH];
      int hash, cnt;
      int count = number_argument( arg1, arg );

      vnum = -1;
      for( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
         for( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
            if( nifty_is_name( arg, pObjIndex->name ) && ++cnt == count )
            {
               vnum = pObjIndex->vnum;
               break;
            }
      if( vnum == -1 )
      {
         send_to_char( "No such object exists.\r\n", ch );
         return;
      }
   }
   else
      vnum = atoi( arg1 );

   if( get_trust( ch ) < LEVEL_DEMI )
   {
      AREA_DATA *pArea;

      if( IS_NPC( ch ) )
      {
         send_to_char( "Huh?\r\n", ch );
         return;
      }
      if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
      {
         send_to_char( "You must have an assigned area to invoke this object.\r\n", ch );
         return;
      }
      if( vnum < pArea->low_o_vnum || vnum > pArea->hi_o_vnum )
      {
         send_to_char( "That number is not in your allocated range.\r\n", ch );
         return;
      }
   }

   if( !( pObjIndex = get_obj_index( vnum ) ) )
   {
      send_to_char( "No object has that vnum.\r\n", ch );
      return;
   }

   if( level == 0 )
   {
      AREA_DATA *temp_area;

      if( ( temp_area = get_area_obj( pObjIndex ) ) == NULL )
         level = ch->level;
      else
      {
         level = generate_itemlevel( temp_area, pObjIndex );
         level = URANGE( 0, level, LEVEL_AVATAR );
      }
   }

   obj = create_object( pObjIndex, level );

   if( quantity > 1 && ( !IS_OBJ_STAT( obj, ITEM_MULTI_INVOKE ) ) )
   {
      send_to_char( "This item can not be invoked in quantities greater than 1.\r\n", ch );
      return;
   }
   else
      obj->count = quantity;

   if( CAN_WEAR( obj, ITEM_TAKE ) )
      obj = obj_to_char( obj, ch );
   else
   {
      obj = obj_to_room( obj, ch->in_room );
      act( AT_IMMORT, "$n fashions $p from ether!", ch, obj, NULL, TO_ROOM );
   }

   /*
    * I invoked what? --Blodkai 
    */
   ch_printf_color( ch, "&YYou invoke %s (&W#%d &Y- &W%s &Y- &Wlvl %d &Y- &Wqty %d&Y)\r\n",
                    pObjIndex->short_descr, pObjIndex->vnum, pObjIndex->name, obj->level, quantity );
   return;
}

void do_purge( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   CLAN_DATA *clan;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      /*
       * 'purge' 
       */
      CHAR_DATA *vnext;
      OBJ_DATA *obj_next;

      for( victim = ch->in_room->first_person; victim; victim = vnext )
      {
         vnext = victim->next_in_room;
         if( IS_NPC( victim ) && victim != ch )
            extract_char( victim, TRUE );
      }

      for( obj = ch->in_room->first_content; obj; obj = obj_next )
      {
         obj_next = obj->next_content;
         extract_obj( obj );
      }

      act( AT_IMMORT, "$n purges the room!", ch, NULL, NULL, TO_ROOM );
      act( AT_IMMORT, "You have purged the room!", ch, NULL, NULL, TO_CHAR );

      /*
       * Clan storeroom check 
       */
      if( xIS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) )
      {
         for( clan = first_clan; clan; clan = clan->next )
            if( clan->storeroom == ch->in_room->vnum )
               save_clan_storeroom( ch, clan );
      }
      return;
   }
   victim = NULL;
   obj = NULL;

   /*
    * fixed to get things in room first -- i.e., purge portal (obj),
    * * no more purging mobs with that keyword in another room first
    * * -- Tri 
    */
   if( ( victim = get_char_room( ch, arg ) ) == NULL && ( obj = get_obj_here( ch, arg ) ) == NULL )
   {
      if( ( victim = get_char_world( ch, arg ) ) == NULL && ( obj = get_obj_world( ch, arg ) ) == NULL ) /* no get_obj_room */
      {
         send_to_char( "They aren't here.\r\n", ch );
         return;
      }
   }

/* Single object purge in room for high level purge - Scryn 8/12*/
   if( obj )
   {
      separate_obj( obj );
      act( AT_IMMORT, "$n purges $p.", ch, obj, NULL, TO_ROOM );
      act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR );
      extract_obj( obj );

      /*
       * Clan storeroom check 
       */
      if( xIS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) )
      {
         for( clan = first_clan; clan; clan = clan->next )
            if( clan->storeroom == ch->in_room->vnum )
               save_clan_storeroom( ch, clan );
      }
      return;
   }

   if( !IS_NPC( victim ) )
   {
      send_to_char( "Not on PC's.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You cannot purge yourself!\r\n", ch );
      return;
   }

   act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
   act( AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL, victim, TO_CHAR );
   extract_char( victim, TRUE );
   return;
}

void do_low_purge( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   CLAN_DATA *clan;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Purge what?\r\n", ch );
      return;
   }

   victim = NULL;
   obj = NULL;
   if( ( victim = get_char_room( ch, arg ) ) == NULL && ( obj = get_obj_here( ch, arg ) ) == NULL )
   {
      send_to_char( "You can't find that here.\r\n", ch );
      return;
   }

   if( obj )
   {
      separate_obj( obj );
      act( AT_IMMORT, "$n purges $p!", ch, obj, NULL, TO_ROOM );
      act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR );
      extract_obj( obj );

      /*
       * Clan storeroom check 
       */
      if( xIS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) )
      {
         for( clan = first_clan; clan; clan = clan->next )
            if( clan->storeroom == ch->in_room->vnum )
               save_clan_storeroom( ch, clan );
      }
      return;
   }

   if( !IS_NPC( victim ) )
   {
      send_to_char( "Not on PC's.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You cannot purge yourself!\r\n", ch );
      return;
   }

   act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
   act( AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL, victim, TO_CHAR );
   extract_char( victim, TRUE );
   return;
}

void do_balzhur( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char *name;
   CHAR_DATA *victim;
   AREA_DATA *pArea;
   int sn;

   set_char_color( AT_BLOOD, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Who is deserving of such a fate?\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't currently playing.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "This will do little good on mobiles.\r\n", ch );
      return;
   }
   if( victim->level >= get_trust( ch ) )
   {
      send_to_char( "I wouldn't even think of that if I were you...\r\n", ch );
      return;
   }

   victim->level = 2;
   victim->trust = 0;
   check_switch( victim, TRUE );
   set_char_color( AT_WHITE, ch );
   send_to_char( "You summon the demon Balzhur to wreak your wrath!\r\n", ch );
   send_to_char( "Balzhur sneers at you evilly, then vanishes in a puff of smoke.\r\n", ch );
   set_char_color( AT_IMMORT, victim );
   send_to_char( "You hear an ungodly sound in the distance that makes your blood run cold!\r\n", victim );
   snprintf( buf, MAX_STRING_LENGTH, "Balzhur screams, 'You are MINE %s!!!'", victim->name );
   echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
   victim->exp = 2000;
   victim->max_hit = 10;
   victim->max_mana = 100;
   victim->max_move = 100;
   for( sn = 0; sn < num_skills; ++sn )
      victim->pcdata->learned[sn] = 0;
   victim->practice = 0;
   victim->hit = victim->max_hit;
   victim->mana = victim->max_mana;
   victim->move = victim->max_move;
   name = capitalize( victim->name );
   snprintf( buf, MAX_STRING_LENGTH, "%s%s", GOD_DIR, name );

   set_char_color( AT_RED, ch );
   if( !remove( buf ) )
      send_to_char( "Player's immortal data destroyed.\r\n", ch );
   else if( errno != ENOENT )
   {
      ch_printf( ch, "Unknown error #%d - %s (immortal data). Report to www.smaugmuds.org\r\n", errno, strerror( errno ) );
      snprintf( buf2, MAX_STRING_LENGTH, "%s balzhuring %s", ch->name, buf );
      perror( buf2 );
   }
   snprintf( buf2, MAX_STRING_LENGTH, "%s.are", name );
   for( pArea = first_build; pArea; pArea = pArea->next )
      if( !str_cmp( pArea->filename, buf2 ) )
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s%s", BUILD_DIR, buf2 );
         if( IS_SET( pArea->status, AREA_LOADED ) )
            fold_area( pArea, buf, FALSE );
         close_area( pArea );
         snprintf( buf2, MAX_STRING_LENGTH, "%s.bak", buf );
         set_char_color( AT_RED, ch ); /* Log message changes colors */
         if( !rename( buf, buf2 ) )
            send_to_char( "Player's area data destroyed.  Area saved as backup.\r\n", ch );
         else if( errno != ENOENT )
         {
            ch_printf( ch, "Unknown error #%d - %s (area data). Report to www.smaugmuds.org\r\n", errno, strerror( errno ) );
            snprintf( buf2, MAX_STRING_LENGTH, "%s destroying %s", ch->name, buf );
            perror( buf2 );
         }
         break;
      }

   make_wizlist(  );
   advance_level( victim );
   do_help( victim, "M_BALZHUR_" );
   set_char_color( AT_WHITE, victim );
   send_to_char( "You awake after a long period of time...\r\n", victim );
   while( victim->first_carrying )
      extract_obj( victim->first_carrying );
   return;
}

void do_advance( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   int level;
   int iLevel;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
   {
      send_to_char( "Syntax:  advance <character> <level>\r\n", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "That character is not in the room.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "You cannot advance a mobile.\r\n", ch );
      return;
   }
   if( get_trust( ch ) <= get_trust( victim ) || ch == victim )
   {
      send_to_char( "You can't do that.\r\n", ch );
      return;
   }
   if( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL )
   {
      ch_printf( ch, "Level range is 1 to %d.\r\n", MAX_LEVEL );
      return;
   }
   if( level > get_trust( ch ) )
   {
      send_to_char( "Level limited to your trust level.\r\n", ch );
      return;
   }
   /*
    * Lower level:
    * *   Reset to level 1.
    * *   Then raise again.
    * *   Currently, an imp can lower another imp.
    * *   -- Swiftest
    * *   Can't lower imms >= your trust (other than self) per Narn's change.
    * *   Few minor text changes as well.  -- Blod
    */
   if( level <= victim->level )
   {
      int sn;

      set_char_color( AT_IMMORT, victim );

      if( victim->level >= LEVEL_AVATAR && IS_IMMORTAL( victim ) )
      {
         if( victim->pcdata->bestowments )
            DISPOSE( victim->pcdata->bestowments );
         victim->pcdata->bestowments = str_dup( "" );
         xREMOVE_BIT( victim->act, PLR_HOLYLIGHT );
         if( !IS_RETIRED( victim ) )
         {
            /*
             * Fixed bug here, was removing the immortal data of the person
             * who used advance - Orion Elder
             */
            snprintf( buf, MAX_INPUT_LENGTH, "%s%s", GOD_DIR, capitalize( victim->name ) );

            /*
             * Added to notify of removal of Immortal data. - Orion Elder 
             */
            if( !remove( buf ) )
               send_to_char( "Player's immortal data destroyed.\r\n", ch );
         }
      }

      if( level < victim->level )
      {
         int tmp = victim->level;

         victim->level = level;
         check_switch( victim, FALSE );
         victim->level = tmp;

         ch_printf( ch, "Demoting %s from level %d to level %d!\r\n", victim->name, victim->level, level );
         send_to_char( "Cursed and forsaken!  The gods have lowered your level...\r\n", victim );
      }
      else
      {
         ch_printf( ch, "%s is already level %d.  Re-advancing...\r\n", victim->name, level );
         send_to_char( "Deja vu!  Your mind reels as you re-live your past levels!\r\n", victim );
      }
      victim->level = 1;
      victim->exp = exp_level( victim, 1 );
      victim->max_hit = 20;
      victim->max_mana = 100;
      victim->max_move = 100;
      for( sn = 0; sn < num_skills; ++sn )
         victim->pcdata->learned[sn] = 0;
      victim->practice = 0;
      victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      victim->move = victim->max_move;
      advance_level( victim );
      /*
       * Rank fix added by Narn. 
       */
      DISPOSE( victim->pcdata->rank );
      victim->pcdata->rank = str_dup( "" );
      /*
       * Stuff added to make sure character's wizinvis level doesn't stay
       * higher than actual level, take wizinvis away from advance < 50 
       */
      victim->pcdata->wizinvis = victim->trust;
      if( victim->level <= LEVEL_AVATAR )
      {
         xREMOVE_BIT( victim->act, PLR_WIZINVIS );
         victim->pcdata->wizinvis = 0;
      }
   }
   else
   {
      ch_printf( ch, "Raising %s from level %d to level %d!\r\n", victim->name, victim->level, level );
      if( victim->level >= LEVEL_AVATAR )
      {
         set_char_color( AT_IMMORT, victim );
         act( AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s finger at you!",
              ch, NULL, victim, TO_VICT );
         act( AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s finger at $N!",
              ch, NULL, victim, TO_NOTVICT );
         set_char_color( AT_WHITE, victim );
         send_to_char( "You suddenly feel very strange...\r\n\r\n", victim );
         set_char_color( AT_LBLUE, victim );
      }
      switch ( level )
      {
         default:
            send_to_char( "The gods feel fit to raise your level!\r\n", victim );
            break;
         case LEVEL_IMMORTAL:
            do_help( victim, "M_GODLVL1_" );
            set_char_color( AT_WHITE, victim );
            send_to_char( "You awake... all your possessions are gone.\r\n", victim );
            while( victim->first_carrying )
               extract_obj( victim->first_carrying );
            break;
         case LEVEL_ACOLYTE:
            do_help( victim, "M_GODLVL2_" );
            break;
         case LEVEL_CREATOR:
            do_help( victim, "M_GODLVL3_" );
            break;
         case LEVEL_SAVIOR:
            do_help( victim, "M_GODLVL4_" );
            break;
         case LEVEL_DEMI:
            do_help( victim, "M_GODLVL5_" );
            break;
         case LEVEL_TRUEIMM:
            do_help( victim, "M_GODLVL6_" );
            break;
         case LEVEL_LESSER:
            do_help( victim, "M_GODLVL7_" );
            break;
         case LEVEL_GOD:
            do_help( victim, "M_GODLVL8_" );
            break;
         case LEVEL_GREATER:
            do_help( victim, "M_GODLVL9_" );
            break;
         case LEVEL_ASCENDANT:
            do_help( victim, "M_GODLVL10_" );
            break;
         case LEVEL_SUB_IMPLEM:
            do_help( victim, "M_GODLVL11_" );
            break;
         case LEVEL_IMPLEMENTOR:
            do_help( victim, "M_GODLVL12_" );
            break;
         case LEVEL_ETERNAL:
            do_help( victim, "M_GODLVL13_" );
            break;
         case LEVEL_INFINITE:
            do_help( victim, "M_GODLVL14_" );
            break;
         case LEVEL_SUPREME:
            do_help( victim, "M_GODLVL15_" );
      }
   }
   for( iLevel = victim->level; iLevel < level; iLevel++ )
   {
      if( level < LEVEL_IMMORTAL )
         send_to_char( "You raise a level!!\r\n", victim );
      victim->level += 1;
      advance_level( victim );
   }
   victim->exp = exp_level( victim, victim->level );
   victim->trust = 0;
   return;
}

void do_elevate( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax: elevate <char>\r\n", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }
   if( victim->level == LEVEL_IMMORTAL )
   {
      send_to_char( "Elevating a player...\r\n", ch );
      set_char_color( AT_IMMORT, victim );
      act( AT_IMMORT, "$n begins to chant softly... then makes some arcane gestures...", ch, NULL, NULL, TO_ROOM );
      set_char_color( AT_WHITE, victim );
      send_to_char( "You suddenly feel very strange...\r\n\r\n", victim );
      set_char_color( AT_LBLUE, victim );
      do_help( victim, "M_GODLVL2_" );
      victim->level = LEVEL_ACOLYTE;
      set_char_color( AT_WHITE, victim );
      advance_level( victim );
      victim->exp = exp_level( victim, victim->level );
      victim->trust = 0;
      return;
   }
   if( victim->level == LEVEL_ACOLYTE )
   {
      send_to_char( "Elevating a player...\r\n", ch );
      set_char_color( AT_IMMORT, victim );
      act( AT_IMMORT, "$n begins to chant softly... then makes some arcane gestures...", ch, NULL, NULL, TO_ROOM );
      set_char_color( AT_WHITE, victim );
      send_to_char( "You suddenly feel very strange...\r\n\r\n", victim );
      set_char_color( AT_LBLUE, victim );
      do_help( victim, "M_GODLVL3_" );
      victim->level = LEVEL_CREATOR;
      set_char_color( AT_WHITE, victim );
      advance_level( victim );
      victim->exp = exp_level( victim, victim->level );
      victim->trust = 0;
      return;
   }
   else
      send_to_char( "You cannot elevate this character.\r\n", ch );
   return;
}

void do_immortalize( CHAR_DATA* ch, const char* argument)
{
   int i;
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax:  immortalize <char>\r\n", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }
   /*
    * Added this check, not sure why the code didn't already have it. Samson 1-18-98 
    */
   if( victim->level >= LEVEL_IMMORTAL )
   {
      ch_printf( ch, "Don't be silly, %s is already immortal.\r\n", victim->name );
      return;
   }
   if( victim->level != LEVEL_AVATAR )
   {
      send_to_char( "This player is not yet worthy of immortality.\r\n", ch );
      return;
   }

   send_to_char( "Immortalizing a player...\r\n", ch );
   set_char_color( AT_IMMORT, victim );
   act( AT_IMMORT, "$n begins to chant softly... then raises $s arms to the sky...", ch, NULL, NULL, TO_ROOM );
   set_char_color( AT_WHITE, victim );
   send_to_char( "You suddenly feel very strange...\r\n\r\n", victim );
   set_char_color( AT_LBLUE, victim );
   do_help( victim, "M_GODLVL1_" );
   set_char_color( AT_WHITE, victim );
   send_to_char( "You awake... all your possessions are gone.\r\n", victim );
   while( victim->first_carrying )
      extract_obj( victim->first_carrying );
   victim->level = LEVEL_IMMORTAL;
   advance_level( victim );

   /*
    * Remove clan/guild/order and update accordingly 
    */
   if( victim->pcdata->clan )
   {
      if( victim->pcdata->clan->clan_type == CLAN_GUILD )
      {
         int sn;

         for( sn = 0; sn < num_skills; ++sn )
            if( skill_table[sn]->guild == victim->pcdata->clan->Class && skill_table[sn]->name != NULL )
               victim->pcdata->learned[sn] = 0;
      }
      if( victim->speaking & LANG_CLAN )
         victim->speaking = LANG_COMMON;
      REMOVE_BIT( victim->speaks, LANG_CLAN );
      --victim->pcdata->clan->members;
      if( !str_cmp( victim->name, victim->pcdata->clan->leader ) )
      {
         STRFREE( victim->pcdata->clan->leader );
         victim->pcdata->clan->leader = STRALLOC( "" );
      }
      if( !str_cmp( victim->name, victim->pcdata->clan->number1 ) )
      {
         STRFREE( victim->pcdata->clan->number1 );
         victim->pcdata->clan->number1 = STRALLOC( "" );
      }
      if( !str_cmp( victim->name, victim->pcdata->clan->number2 ) )
      {
         STRFREE( victim->pcdata->clan->number2 );
         victim->pcdata->clan->number2 = STRALLOC( "" );
      }
      victim->pcdata->clan = NULL;
      STRFREE( victim->pcdata->clan_name );
   }

   /*
    * create immortal only data for tell history 
    */
   CREATE( victim->pcdata->tell_history, const char *, 26 );
   for( i = 0; i < 26; i++ )
      victim->pcdata->tell_history[i] = NULL;
   victim->exp = exp_level( victim, victim->level );
   victim->trust = 0;
   save_char_obj( victim );
   return;
}

void do_trust( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   int level;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
   {
      send_to_char( "Syntax:  trust <char> <level>.\r\n", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }
   if( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
   {
      send_to_char( "Level must be 0 (reset) or 1 to 65.\r\n", ch );
      return;
   }
   if( level > get_trust( ch ) )
   {
      send_to_char( "Limited to your own trust.\r\n", ch );
      return;
   }
   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You can't do that.\r\n", ch );
      return;
   }

   victim->trust = level;
   send_to_char( "Ok.\r\n", ch );
   return;
}

/* Summer 1997 --Blod */
void do_scatter( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *pRoomIndex;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Scatter whom?\r\n", ch );
      return;
   }
   if( !( victim = get_char_room( ch, arg ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "It's called teleport.  Try it.\r\n", ch );
      return;
   }
   if( !IS_NPC( victim ) && get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You haven't the power to succeed against them.\r\n", ch );
      return;
   }
   for( ;; )
   {
      pRoomIndex = get_room_index( number_range( 0, MAX_VNUM ) );
      if( pRoomIndex )
         if( !xIS_SET( pRoomIndex->room_flags, ROOM_PRIVATE )
             && !xIS_SET( pRoomIndex->room_flags, ROOM_SOLITARY )
             && !xIS_SET( pRoomIndex->room_flags, ROOM_NO_ASTRAL ) && !xIS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE ) )
            break;
   }
   if( victim->fighting )
      stop_fighting( victim, TRUE );
   act( AT_MAGIC, "With the sweep of an arm, $n flings $N to the winds.", ch, NULL, victim, TO_NOTVICT );
   act( AT_MAGIC, "With the sweep of an arm, $n flings you to the astral winds.", ch, NULL, victim, TO_VICT );
   act( AT_MAGIC, "With the sweep of an arm, you fling $N to the astral winds.", ch, NULL, victim, TO_CHAR );
   char_from_room( victim );
   char_to_room( victim, pRoomIndex );
   victim->position = POS_RESTING;
   act( AT_MAGIC, "$n staggers forth from a sudden gust of wind, and collapses.", victim, NULL, NULL, TO_ROOM );
   do_look( victim, "auto" );
   return;
}

void do_strew( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *obj_next;
   OBJ_DATA *obj_lose;
   ROOM_INDEX_DATA *pRoomIndex;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Strew who, what?\r\n", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "It would work better if they were here.\r\n", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "Try taking it out on someone else first.\r\n", ch );
      return;
   }
   if( !IS_NPC( victim ) && get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You haven't the power to succeed against them.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "coins" ) )
   {
      if( victim->gold < 1 )
      {
         send_to_char( "Drat, this one's got no gold to start with.\r\n", ch );
         return;
      }
      victim->gold = 0;
      act( AT_MAGIC, "$n gestures and an unearthly gale sends $N's coins flying!", ch, NULL, victim, TO_NOTVICT );
      act( AT_MAGIC, "You gesture and an unearthly gale sends $N's coins flying!", ch, NULL, victim, TO_CHAR );
      act( AT_MAGIC, "As $n gestures, an unearthly gale sends your currency flying!", ch, NULL, victim, TO_VICT );
      return;
   }
   for( ;; )
   {
      pRoomIndex = get_room_index( number_range( 0, MAX_VNUM ) );
      if( pRoomIndex )
         if( !xIS_SET( pRoomIndex->room_flags, ROOM_PRIVATE )
             && !xIS_SET( pRoomIndex->room_flags, ROOM_SOLITARY )
             && !xIS_SET( pRoomIndex->room_flags, ROOM_NO_ASTRAL ) && !xIS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE ) )
            break;
   }
   if( !str_cmp( arg2, "inventory" ) )
   {
      act( AT_MAGIC, "$n speaks a single word, sending $N's possessions flying!", ch, NULL, victim, TO_NOTVICT );
      act( AT_MAGIC, "You speak a single word, sending $N's possessions flying!", ch, NULL, victim, TO_CHAR );
      act( AT_MAGIC, "$n speaks a single word, sending your possessions flying!", ch, NULL, victim, TO_VICT );
      for( obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next )
      {
         obj_next = obj_lose->next_content;
         obj_from_char( obj_lose );
         obj_to_room( obj_lose, pRoomIndex );
         pager_printf_color( ch, "\t&w%s sent to %d\r\n", capitalize( obj_lose->short_descr ), pRoomIndex->vnum );
      }
      return;
   }
   send_to_char( "Strew their coins or inventory?\r\n", ch );
   return;
}

void do_strip( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   OBJ_DATA *obj_next;
   OBJ_DATA *obj_lose;
   int count = 0;

   set_char_color( AT_OBJECT, ch );
   if( !argument )
   {
      send_to_char( "Strip who?\r\n", ch );
      return;
   }
   if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      send_to_char( "They're not here.\r\n", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "Kinky.\r\n", ch );
      return;
   }
   if( !IS_NPC( victim ) && get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You haven't the power to succeed against them.\r\n", ch );
      return;
   }
   act( AT_OBJECT, "Searching $N ...", ch, NULL, victim, TO_CHAR );
   for( obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next )
   {
      obj_next = obj_lose->next_content;
      obj_from_char( obj_lose );
      obj_to_char( obj_lose, ch );
      pager_printf_color( ch, "  &G... %s (&g%s) &Gtaken.\r\n", capitalize( obj_lose->short_descr ), obj_lose->name );
      count++;
   }
   if( !count )
      send_to_pager( "&GNothing found to take.\r\n", ch );
   return;
}

void do_restore( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   bool boost = FALSE;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Restore whom?\r\n", ch );
      return;
   }
   argument = one_argument( argument, arg2 );
   if( !str_cmp( arg2, "boost" ) && get_trust( ch ) >= LEVEL_SUB_IMPLEM )
   {
      send_to_char( "Boosting!\r\n", ch );
      boost = TRUE;
   }
   if( !str_cmp( arg, "all" ) )
   {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if( !ch->pcdata )
         return;

      if( get_trust( ch ) < LEVEL_SUB_IMPLEM )
      {
         if( IS_NPC( ch ) )
         {
            send_to_char( "You can't do that.\r\n", ch );
            return;
         }
         else
         {
            /*
             * Check if the player did a restore all within the last 18 hours. 
             */
            if( current_time - last_restore_all_time < RESTORE_INTERVAL )
            {
               send_to_char( "Sorry, you can't do a restore all yet.\r\n", ch );
               do_restoretime( ch, "" );
               return;
            }
         }
      }
      last_restore_all_time = current_time;
      ch->pcdata->restore_time = current_time;
      save_char_obj( ch );
      send_to_char( "Beginning 'restore all' ...\r\n", ch );
      for( vch = first_char; vch; vch = vch_next )
      {
         vch_next = vch->next;

         if( !IS_NPC( vch ) && !IS_IMMORTAL( vch ) && !CAN_PKILL( vch ) && !in_arena( vch ) )
         {
            if( boost )
               vch->hit = ( short )( vch->max_hit * 1.5 );
            else
               vch->hit = vch->max_hit;
            vch->mana = vch->max_mana;
            vch->move = vch->max_move;
            vch->pcdata->condition[COND_BLOODTHIRST] = ( 10 + vch->level );
            update_pos( vch );
            act( AT_IMMORT, "$n has restored you.", ch, NULL, vch, TO_VICT );
         }
      }
      send_to_char( "Restored.\r\n", ch );
   }
   else
   {

      CHAR_DATA *victim;

      if( ( victim = get_char_world( ch, arg ) ) == NULL )
      {
         send_to_char( "They aren't here.\r\n", ch );
         return;
      }

      if( get_trust( ch ) < LEVEL_LESSER && victim != ch && !( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) ) )
      {
         send_to_char( "You can't do that.\r\n", ch );
         return;
      }

      if( boost )
         victim->hit = ( short )( victim->max_hit * 1.5 );
      else
         victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      victim->move = victim->max_move;
      if( victim->pcdata )
         victim->pcdata->condition[COND_BLOODTHIRST] = ( 10 + victim->level );
      update_pos( victim );
      if( ch != victim )
         act( AT_IMMORT, "$n has restored you.", ch, NULL, victim, TO_VICT );
      send_to_char( "Restored.\r\n", ch );
      return;
   }
}

void do_restoretime( CHAR_DATA* ch, const char* argument)
{
   long int time_passed;
   int hour, minute;

   set_char_color( AT_IMMORT, ch );

   if( !last_restore_all_time )
      ch_printf( ch, "There has been no restore all since reboot.\r\n" );
   else
   {
      time_passed = current_time - last_restore_all_time;
      hour = ( int )( time_passed / 3600 );
      minute = ( int )( ( time_passed - ( hour * 3600 ) ) / 60 );
      ch_printf( ch, "The  last restore all was %d hours and %d minutes ago.\r\n", hour, minute );
   }

   if( !ch->pcdata )
      return;

   if( !ch->pcdata->restore_time )
   {
      send_to_char( "You have never done a restore all.\r\n", ch );
      return;
   }

   time_passed = current_time - ch->pcdata->restore_time;
   hour = ( int )( time_passed / 3600 );
   minute = ( int )( ( time_passed - ( hour * 3600 ) ) / 60 );
   ch_printf( ch, "Your last restore all was %d hours and %d minutes ago.\r\n", hour, minute );
   return;
}

void do_freeze( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_LBLUE, ch );

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Freeze whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   set_char_color( AT_LBLUE, victim );
   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed, and they saw...\r\n", ch );
      ch_printf( victim, "%s is attempting to freeze you.\r\n", ch->name );
      return;
   }

   if( victim->desc && victim->desc->original && get_trust( victim->desc->original ) >= get_trust( ch ) )
   {
      send_to_char( "For some inexplicable reason, you failed.\r\n", ch );
      return;
   }

   if( xIS_SET( victim->act, PLR_FREEZE ) )
   {
      xREMOVE_BIT( victim->act, PLR_FREEZE );
      send_to_char( "Your frozen form suddenly thaws.\r\n", victim );
      ch_printf( ch, "%s is now unfrozen.\r\n", victim->name );
   }
   else
   {
      if( victim->switched )
      {
         do_return( victim->switched, "" );
         set_char_color( AT_LBLUE, victim );
      }
      xSET_BIT( victim->act, PLR_FREEZE );
      send_to_char( "A godly force turns your body to ice!\r\n", victim );
      ch_printf( ch, "You have frozen %s.\r\n", victim->name );
   }
   save_char_obj( victim );
   return;
}

void do_log( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Log whom?\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "all" ) )
   {
      if( fLogAll )
      {
         fLogAll = FALSE;
         send_to_char( "Log ALL off.\r\n", ch );
      }
      else
      {
         fLogAll = TRUE;
         send_to_char( "Log ALL on.\r\n", ch );
      }
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   /*
    * No level check, gods can log anyone.
    */

   if( xIS_SET( victim->act, PLR_LOG ) )
   {
      xREMOVE_BIT( victim->act, PLR_LOG );
      ch_printf( ch, "LOG removed from %s.\r\n", victim->name );
   }
   else
   {
      xSET_BIT( victim->act, PLR_LOG );
      ch_printf( ch, "LOG applied to %s.\r\n", victim->name );
   }
   return;
}

void do_litterbug( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Set litterbug flag on whom?\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }
   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   if( xIS_SET( victim->act, PLR_LITTERBUG ) )
   {
      xREMOVE_BIT( victim->act, PLR_LITTERBUG );
      send_to_char( "You can drop items again.\r\n", victim );
      ch_printf( ch, "LITTERBUG removed from %s.\r\n", victim->name );
   }
   else
   {
      xSET_BIT( victim->act, PLR_LITTERBUG );
      send_to_char( "A strange force prevents you from dropping any more items!\r\n", victim );
      ch_printf( ch, "LITTERBUG set on %s.\r\n", victim->name );
   }
   return;
}

void do_noemote( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Noemote whom?\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }
   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   if( xIS_SET( victim->act, PLR_NO_EMOTE ) )
   {
      xREMOVE_BIT( victim->act, PLR_NO_EMOTE );
      send_to_char( "You can emote again.\r\n", victim );
      ch_printf( ch, "NOEMOTE removed from %s.\r\n", victim->name );
   }
   else
   {
      xSET_BIT( victim->act, PLR_NO_EMOTE );
      send_to_char( "You can't emote!\r\n", victim );
      ch_printf( ch, "NOEMOTE applied to %s.\r\n", victim->name );
   }
   return;
}

void do_notell( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Notell whom?", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }
   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   if( xIS_SET( victim->act, PLR_NO_TELL ) )
   {
      xREMOVE_BIT( victim->act, PLR_NO_TELL );
      send_to_char( "You can use tells again.\r\n", victim );
      ch_printf( ch, "NOTELL removed from %s.\r\n", victim->name );
   }
   else
   {
      xSET_BIT( victim->act, PLR_NO_TELL );
      send_to_char( "You can't use tells!\r\n", victim );
      ch_printf( ch, "NOTELL applied to %s.\r\n", victim->name );
   }
   return;
}

void do_notitle( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Notitle whom?\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }
   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   if( IS_SET( victim->pcdata->flags, PCFLAG_NOTITLE ) )
   {
      REMOVE_BIT( victim->pcdata->flags, PCFLAG_NOTITLE );
      send_to_char( "You can set your own title again.\r\n", victim );
      ch_printf( ch, "NOTITLE removed from %s.\r\n", victim->name );
   }
   else
   {
      SET_BIT( victim->pcdata->flags, PCFLAG_NOTITLE );
      snprintf( buf, MAX_STRING_LENGTH, "the %s",
                title_table[victim->Class][victim->level][victim->sex == SEX_FEMALE ? 1 : 0] );
      set_title( victim, buf );
      send_to_char( "You can't set your own title!\r\n", victim );
      ch_printf( ch, "NOTITLE set on %s.\r\n", victim->name );
   }
   return;
}

void do_silence( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Silence whom?", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }
   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   if( xIS_SET( victim->act, PLR_SILENCE ) )
   {
      send_to_char( "Player already silenced, use unsilence to remove.\r\n", ch );
   }
   else
   {
      xSET_BIT( victim->act, PLR_SILENCE );
      send_to_char( "You can't use channels!\r\n", victim );
      ch_printf( ch, "You SILENCE %s.\r\n", victim->name );
   }
   return;
}

/* Much better than toggling this with do_silence, yech --Blodkai */
void do_unsilence( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Unsilence whom?\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }
   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   if( xIS_SET( victim->act, PLR_SILENCE ) )
   {
      xREMOVE_BIT( victim->act, PLR_SILENCE );
      send_to_char( "You can use channels again.\r\n", victim );
      ch_printf( ch, "SILENCE removed from %s.\r\n", victim->name );
   }
   else
   {
      send_to_char( "That player is not silenced.\r\n", ch );
   }
   return;
}

void do_peace( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *rch;

   act( AT_IMMORT, "$n booms, 'PEACE!'", ch, NULL, NULL, TO_ROOM );
   act( AT_IMMORT, "You boom, 'PEACE!'", ch, NULL, NULL, TO_CHAR );
   for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
   {
      if( rch->fighting )
      {
         stop_fighting( rch, TRUE );
         do_sit( rch, "" );
      }

      /*
       * Added by Narn, Nov 28/95 
       */
      stop_hating( rch );
      stop_hunting( rch );
      stop_fearing( rch );
   }

   send_to_char_color( "&YOk.\r\n", ch );
   return;
}

WATCH_DATA *first_watch;
WATCH_DATA *last_watch;

void free_watchlist( void )
{
   WATCH_DATA *pw, *pw_next;

   for( pw = first_watch; pw; pw = pw_next )
   {
      pw_next = pw->next;
      UNLINK( pw, first_watch, last_watch, next, prev );
      DISPOSE( pw->imm_name );
      DISPOSE( pw->player_site );
      DISPOSE( pw->target_name );
      DISPOSE( pw );
   }
   return;
}

void save_watchlist( void )
{
   WATCH_DATA *pwatch;
   FILE *fp;

   if( !( fp = fopen( SYSTEM_DIR WATCH_LIST, "w" ) ) )
   {
      bug( "Save_watchlist: Cannot open %s", WATCH_LIST );
      perror( WATCH_LIST );
      return;
   }

   for( pwatch = first_watch; pwatch; pwatch = pwatch->next )
      fprintf( fp, "%d %s~%s~%s~\n", pwatch->imm_level, pwatch->imm_name,
               pwatch->target_name ? pwatch->target_name : " ", pwatch->player_site ? pwatch->player_site : " " );
   fprintf( fp, "-1\n" );
   fclose( fp );
   return;
}

void do_wizlock( CHAR_DATA* ch, const char* argument)
{
   sysdata.wizlock = !sysdata.wizlock;

   set_char_color( AT_DANGER, ch );

   if( sysdata.wizlock )
      send_to_char( "Game wizlocked.\r\n", ch );
   else
      send_to_char( "Game un-wizlocked.\r\n", ch );
   save_sysdata( sysdata );
   return;
}

void do_noresolve( CHAR_DATA* ch, const char* argument)
{
   sysdata.NO_NAME_RESOLVING = !sysdata.NO_NAME_RESOLVING;

   if( sysdata.NO_NAME_RESOLVING )
      send_to_char_color( "&YName resolving disabled.\r\n", ch );
   else
      send_to_char_color( "&YName resolving enabled.\r\n", ch );
   return;
}

/* Output of command reformmated by Samson 2-8-98, and again on 4-7-98 */
void do_users( CHAR_DATA* ch, const char* argument)
{
   DESCRIPTOR_DATA *d;
   int count;
   const char *st;

   set_pager_color( AT_PLAIN, ch );

   count = 0;
   send_to_pager( "Desc|     Constate      |Idle|    Player    | HostIP                   \r\n", ch );
   send_to_pager( "----+-------------------+----+--------------+--------------------------\r\n", ch );
   for( d = first_descriptor; d; d = d->next )
   {
      switch ( d->connected )
      {
         case CON_PLAYING:
            st = "Playing";
            break;
         case CON_GET_NAME:
            st = "Get name";
            break;
         case CON_GET_OLD_PASSWORD:
            st = "Get password";
            break;
         case CON_CONFIRM_NEW_NAME:
            st = "Confirm name";
            break;
         case CON_GET_NEW_PASSWORD:
            st = "New password";
            break;
         case CON_CONFIRM_NEW_PASSWORD:
            st = "Confirm password";
            break;
         case CON_GET_NEW_SEX:
            st = "Get sex";
            break;
         case CON_READ_MOTD:
            st = "Reading MOTD";
            break;
         case CON_EDITING:
            st = "In line editor";
            break;
         case CON_PRESS_ENTER:
            st = "Press enter";
            break;
         default:
            st = "Invalid!!!!";
            break;
      }

      if( !argument || argument[0] == '\0' )
      {
         if( get_trust( ch ) >= LEVEL_ASCENDANT || ( d->character && can_see( ch, d->character ) ) )
         {
            count++;
            pager_printf( ch, " %3d| %-17s |%4d| %-12s | %s \r\n", d->descriptor, st, d->idle / 4,
                          d->original ? d->original->name : d->character ? d->character->name : "(None!)", d->host );
         }
      }
      else
      {
         if( ( get_trust( ch ) >= LEVEL_SUPREME || ( d->character && can_see( ch, d->character ) ) )
             && ( !str_prefix( argument, d->host ) || ( d->character && !str_prefix( argument, d->character->name ) ) ) )
         {
            count++;
            pager_printf( ch, " %3d| %2d|%4d| %-12s | %s \r\n", d->descriptor, d->connected, d->idle / 4,
                          d->original ? d->original->name : d->character ? d->character->name : "(None!)", d->host );
         }
      }
   }
   pager_printf( ch, "%d user%s.\r\n", count, count == 1 ? "" : "s" );
   return;
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   bool mobsonly;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Force whom to do what?\r\n", ch );
      return;
   }

   mobsonly = get_trust( ch ) < sysdata.level_forcepc;

   if( !str_cmp( arg, "all" ) )
   {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if( mobsonly )
      {
         send_to_char( "Force whom to do what?\r\n", ch );
         return;
      }

      for( vch = first_char; vch; vch = vch_next )
      {
         vch_next = vch->next;

         if( !IS_NPC( vch ) && get_trust( vch ) < get_trust( ch ) )
         {
            act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, vch, TO_VICT );
            interpret( vch, argument );
         }
      }
   }
   else
   {
      CHAR_DATA *victim;

      if( ( victim = get_char_world( ch, arg ) ) == NULL )
      {
         send_to_char( "They aren't here.\r\n", ch );
         return;
      }

      if( victim == ch )
      {
         send_to_char( "Aye aye, right away!\r\n", ch );
         return;
      }

      if( ( get_trust( victim ) >= get_trust( ch ) ) || ( mobsonly && !IS_NPC( victim ) ) )
      {
         send_to_char( "Do it yourself!\r\n", ch );
         return;
      }

      if( get_trust( ch ) < LEVEL_GOD && IS_NPC( victim ) && !str_prefix( "mp", argument ) )
      {
         send_to_char( "You can't force a mob to do that!\r\n", ch );
         return;
      }
      act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, victim, TO_VICT );
      interpret( victim, argument );
   }

   send_to_char( "Ok.\r\n", ch );
   return;
}

void do_invis( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   short level;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] != '\0' )
   {
      if( !is_number( arg ) )
      {
         send_to_char( "Usage: invis | invis <level>\r\n", ch );
         return;
      }
      level = atoi( arg );
      if( level < 2 || level > get_trust( ch ) )
      {
         send_to_char( "Invalid level.\r\n", ch );
         return;
      }

      if( !IS_NPC( ch ) )
      {
         ch->pcdata->wizinvis = level;
         ch_printf( ch, "Wizinvis level set to %d.\r\n", level );
      }
      else
      {
         ch->mobinvis = level;
         ch_printf( ch, "Mobinvis level set to %d.\r\n", level );
      }
      return;
   }

   if( IS_NPC( ch ) )
   {
      if( ch->mobinvis < 2 )
         ch->mobinvis = ch->level;
      return;
   }

   if( ch->pcdata->wizinvis < 2 )
      ch->pcdata->wizinvis = ch->level;

   if( xIS_SET( ch->act, PLR_WIZINVIS ) )
   {
      xREMOVE_BIT( ch->act, PLR_WIZINVIS );
      act( AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly fade back into existence.\r\n", ch );
   }
   else
   {
      act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly vanish into thin air.\r\n", ch );
      xSET_BIT( ch->act, PLR_WIZINVIS );
   }
   return;
}

void do_holylight( CHAR_DATA* ch, const char* argument)
{

   set_char_color( AT_IMMORT, ch );

   if( IS_NPC( ch ) )
      return;

   if( xIS_SET( ch->act, PLR_HOLYLIGHT ) )
   {
      xREMOVE_BIT( ch->act, PLR_HOLYLIGHT );
      send_to_char( "Holy light mode off.\r\n", ch );
   }
   else
   {
      xSET_BIT( ch->act, PLR_HOLYLIGHT );
      send_to_char( "Holy light mode on.\r\n", ch );
   }
   return;
}

void do_cmdtable( CHAR_DATA* ch, const char* argument)
{
   int hash, cnt;
   CMDTYPE *cmd;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( strcmp( arg, "lag" ) ) /* display normal command table */
   {
      set_pager_color( AT_IMMORT, ch );
      send_to_pager( "Commands and Number of Uses This Run\r\n", ch );
      set_pager_color( AT_PLAIN, ch );
      for( cnt = hash = 0; hash < 126; hash++ )
         for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
         {
            if( ( ++cnt ) % 4 )
               pager_printf( ch, "%-6.6s %4d\t", cmd->name, cmd->userec.num_uses );
            else
               pager_printf( ch, "%-6.6s %4d\r\n", cmd->name, cmd->userec.num_uses );
         }
      send_to_char( "\r\n", ch );
   }
   else  /* display commands causing lag */
   {
      set_pager_color( AT_IMMORT, ch );
      send_to_pager( "Commands that have caused lag this run\r\n", ch );
      set_pager_color( AT_PLAIN, ch );
      for( cnt = hash = 0; hash < 126; hash++ )
         for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
         {
            if( !cmd->lag_count )
               continue;
            else if( ( ++cnt ) % 4 )
               pager_printf( ch, "%-6.6s %4d\t", cmd->name, cmd->lag_count );
            else
               pager_printf( ch, "%-6.6s %4d\r\n", cmd->name, cmd->lag_count );
         }
      send_to_char( "\r\n", ch );
   }

   return;
}

void do_mortalize( CHAR_DATA* ch, const char* argument)
{
   char fname[1024];
   char name[256];
   struct stat fst;
   bool loaded;
   DESCRIPTOR_DATA *d;
   int old_room_vnum;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   AREA_DATA *pArea;
   int sn;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, name );
   if( name[0] == '\0' )
   {
      send_to_char( "Usage: mortalize <playername>\r\n", ch );
      return;
   }

   name[0] = UPPER( name[0] );
   snprintf( fname, 1024, "%s%c/%s", PLAYER_DIR, tolower( name[0] ), capitalize( name ) );
   if( stat( fname, &fst ) != -1 )
   {
      CREATE( d, DESCRIPTOR_DATA, 1 );
      d->next = NULL;
      d->prev = NULL;
      d->connected = CON_GET_NAME;
      d->outsize = 2000;
      CREATE( d->outbuf, char, d->outsize );

      loaded = load_char_obj( d, name, FALSE, FALSE );
      if( !loaded )
         bug( "%s: failed to load_char_obj for %s.", __FUNCTION__, name );
      add_char( d->character );
      old_room_vnum = d->character->in_room->vnum;
      char_to_room( d->character, ch->in_room );
      if( get_trust( d->character ) >= get_trust( ch ) )
      {
         do_say( d->character, "Do *NOT* disturb me again!" );
         send_to_char( "I think you'd better leave that player alone!\r\n", ch );
         d->character->desc = NULL;
         do_quit( d->character, "" );
         return;
      }
      d->character->desc = NULL;
      victim = d->character;
      d->character = NULL;
      DISPOSE( d->outbuf );
      DISPOSE( d );
      victim->level = LEVEL_AVATAR;
      victim->exp = exp_level( victim, LEVEL_AVATAR );
      victim->max_hit = 800;
      victim->max_mana = 800;
      victim->max_move = 800;
      for( sn = 0; sn < num_skills; ++sn )
         victim->pcdata->learned[sn] = 0;
      victim->practice = 0;
      victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      victim->move = victim->max_move;
      advance_level( victim );
      DISPOSE( victim->pcdata->rank );
      victim->pcdata->rank = str_dup( "" );
      if( xIS_SET( victim->act, PLR_WIZINVIS ) )
         victim->pcdata->wizinvis = victim->trust;
      if( xIS_SET( victim->act, PLR_WIZINVIS ) && ( victim->level <= LEVEL_AVATAR ) )
      {
         xREMOVE_BIT( victim->act, PLR_WIZINVIS );
         victim->pcdata->wizinvis = victim->trust;
      }
      snprintf( buf, MAX_STRING_LENGTH, "%s%s", GOD_DIR, capitalize( victim->name ) );

      if( !remove( buf ) )
         send_to_char( "Player's immortal data destroyed.\r\n", ch );
      else if( errno != ENOENT )
      {
         ch_printf( ch, "Unknown error #%d - %s (immortal data). Report to www.smaugmuds.org\r\n", errno,
                    strerror( errno ) );
         snprintf( buf2, MAX_STRING_LENGTH, "%s mortalizing %s", ch->name, buf );
         perror( buf2 );
      }
      snprintf( buf2, MAX_STRING_LENGTH, "%s.are", capitalize( argument ) );
      for( pArea = first_build; pArea; pArea = pArea->next )
         if( !strcmp( pArea->filename, buf2 ) )
         {
            snprintf( buf, MAX_STRING_LENGTH, "%s%s", BUILD_DIR, buf2 );
            if( IS_SET( pArea->status, AREA_LOADED ) )
               fold_area( pArea, buf, FALSE );
            close_area( pArea );
            snprintf( buf2, MAX_STRING_LENGTH, "%s.bak", buf );
            set_char_color( AT_RED, ch );
            if( !rename( buf, buf2 ) )
               send_to_char( "Player's area data destroyed. Area saved as backup.\r\n", ch );
            else if( errno != ENOENT )
            {
               ch_printf( ch, "Unknown error #%d - %s (area data). Report to www.smaugmuds.org\r\n", errno,
                          strerror( errno ) );
               snprintf( buf2, MAX_STRING_LENGTH, "%s mortalizing %s", ch->name, buf );
               perror( buf2 );
            }
         }
      make_wizlist(  );
      while( victim->first_carrying )
         extract_obj( victim->first_carrying );
      char_from_room( victim );
      char_to_room( victim, get_room_index( old_room_vnum ) );
      do_quit( victim, "" );
      return;
   }
   send_to_char( "No such player.\r\n", ch );
   return;
}

/*
 * Load up a player file
 */
void do_loadup( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *temp;
   char fname[1024];
   char name[256];
   struct stat fst;
   bool loaded;
   DESCRIPTOR_DATA *d;
   int old_room_vnum;
   char buf[MAX_STRING_LENGTH];

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, name );
   if( name[0] == '\0' )
   {
      send_to_char( "Usage: loadup <playername>\r\n", ch );
      return;
   }
   for( temp = first_char; temp; temp = temp->next )
   {
      if( IS_NPC( temp ) )
         continue;
      if( can_see( ch, temp ) && !str_cmp( name, temp->name ) )
         break;
   }
   if( temp != NULL )
   {
      send_to_char( "They are already playing.\r\n", ch );
      return;
   }
   name[0] = UPPER( name[0] );
   snprintf( fname, 1024, "%s%c/%s", PLAYER_DIR, tolower( name[0] ), capitalize( name ) );

   /*
    * Bug fix here provided by Senir to stop /dev/null crash 
    */
   if( stat( fname, &fst ) == -1 || !check_parse_name( capitalize( name ), FALSE ) )
   {
      send_to_char( "&YNo such player exists.\r\n", ch );
      return;
   }

   if( stat( fname, &fst ) != -1 )
   {
      CREATE( d, DESCRIPTOR_DATA, 1 );
      d->next = NULL;
      d->prev = NULL;
      d->connected = CON_GET_NAME;
      d->outsize = 2000;
      CREATE( d->outbuf, char, d->outsize );

      loaded = load_char_obj( d, name, FALSE, FALSE );
      if( !loaded )
         bug( "%s: Failed to load_char_object for %s.", __FUNCTION__, name );

      add_char( d->character );
      old_room_vnum = d->character->in_room->vnum;
      char_to_room( d->character, ch->in_room );
      if( get_trust( d->character ) >= get_trust( ch ) )
      {
         do_say( d->character, "Do *NOT* disturb me again!" );
         send_to_char( "I think you'd better leave that player alone!\r\n", ch );
         d->character->desc = NULL;
         do_quit( d->character, "" );
         return;
      }
      d->character->desc = NULL;
      d->character->retran = old_room_vnum;
      d->character = NULL;
      DISPOSE( d->outbuf );
      DISPOSE( d );
      ch_printf( ch, "Player %s loaded from room %d.\r\n", capitalize( name ), old_room_vnum );
      snprintf( buf, MAX_STRING_LENGTH, "%s appears from nowhere, eyes glazed over.\r\n", capitalize( name ) );
      act( AT_IMMORT, buf, ch, NULL, NULL, TO_ROOM );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   /*
    * else no player file 
    */
   send_to_char( "No such player.\r\n", ch );
   return;
}

void do_fixchar( CHAR_DATA* ch, const char* argument)
{
   char name[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, name );
   if( name[0] == '\0' )
   {
      send_to_char( "Usage: fixchar <playername>\r\n", ch );
      return;
   }

   victim = get_char_room( ch, name );
   if( !victim )
   {
      send_to_char( "They're not here.\r\n", ch );
      return;
   }
   fix_char( victim );
/*  victim->armor	= 100;
    victim->mod_str	= 0;
    victim->mod_dex	= 0;
    victim->mod_wis	= 0;
    victim->mod_int	= 0;
    victim->mod_con	= 0;
    victim->mod_cha	= 0;
    victim->mod_lck	= 0;
    victim->damroll	= 0;
    victim->hitroll	= 0;
    victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
    victim->saving_spell_staff = 0; */
   send_to_char( "Done.\r\n", ch );
}

void do_newbieset( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: newbieset <char>.\r\n", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }
   if( ( victim->level < 1 ) || ( victim->level > 5 ) )
   {
      send_to_char( "Level of victim must be between 1 and 5.\r\n", ch );
      return;
   }

   obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_VEST ), 1 );
   obj_to_char( obj, victim );
   obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_SHIELD ), 1 );
   obj_to_char( obj, victim );
   obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_BANNER ), 1 );
   obj_to_char( obj, victim );

   if( ( victim->Class == CLASS_MAGE ) || ( victim->Class == CLASS_THIEF )
       || ( victim->Class == CLASS_VAMPIRE ) || ( victim->Class == CLASS_AUGURER ) )
   {
      obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_DAGGER ), 1 );
      obj_to_char( obj, victim );
   }
   else if( ( victim->Class == CLASS_CLERIC ) || ( victim->Class == CLASS_DRUID ) )
   {
      obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_MACE ), 1 );
      obj_to_char( obj, victim );
   }
   else if( ( victim->Class == CLASS_WARRIOR ) || ( victim->Class == CLASS_RANGER ) || ( victim->Class == CLASS_PALADIN ) )
   {
      obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_SWORD ), 1 );
      obj_to_char( obj, victim );
   }

   /*
    * Added by Brittany, on Nov. 24, 1996. The object is the adventurer's 
    * guide to the realms of despair, part of academy.are. 
    */
   {
      OBJ_INDEX_DATA *obj_ind = get_obj_index( 10333 );
      if( obj_ind != NULL )
      {
         obj = create_object( obj_ind, 1 );
         obj_to_char( obj, victim );
      }
   }

/* Added the burlap sack to the newbieset.  The sack is part of sgate.are
   called Spectral Gate.  Brittany */

   {

      OBJ_INDEX_DATA *obj_ind = get_obj_index( 123 );
      if( obj_ind != NULL )
      {
         obj = create_object( obj_ind, 1 );
         obj_to_char( obj, victim );
      }
   }

   act( AT_IMMORT, "$n has equipped you with a newbieset.", ch, NULL, victim, TO_VICT );
   ch_printf( ch, "You have re-equipped %s.\r\n", victim->name );
   return;
}

/*
 * Extract area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "joe.are susan.are"
 * - Gorog
 */
void extract_area_names( const char *inp, char *out )
{
   char buf[MAX_INPUT_LENGTH], *pbuf = buf;
   int len;

   *out = '\0';
   while( inp && *inp )
   {
      inp = one_argument( inp, buf );
      if( ( len = strlen( buf ) ) >= 5 && !strcmp( ".are", pbuf + len - 4 ) )
      {
         if( *out )
            mudstrlcat( out, " ", MAX_INPUT_LENGTH );
         mudstrlcat( out, buf, MAX_INPUT_LENGTH );
      }
   }
}

/*
 * Remove area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "aset sedit cset"
 * - Gorog
 */
void remove_area_names( const char *inp, char *out )
{
   char buf[MAX_INPUT_LENGTH], *pbuf = buf;
   int len;

   *out = '\0';
   while( inp && *inp )
   {
      inp = one_argument( inp, buf );
      if( ( len = strlen( buf ) ) < 5 || strcmp( ".are", pbuf + len - 4 ) )
      {
         if( *out )
            mudstrlcat( out, " ", MAX_INPUT_LENGTH );
         mudstrlcat( out, buf, MAX_INPUT_LENGTH );
      }
   }
}

/*
 * Allows members of the Area Council to add Area names to the bestow field.
 * Area names mus end with ".are" so that no commands can be bestowed.
 */
void do_bestowarea( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   int arg_len;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );

   if( !*arg )
   {
      send_to_char( "Syntax:\r\n"
                    "bestowarea <victim> <filename>.are\r\n"
                    "bestowarea <victim> none             removes bestowed areas\r\n"
                    "bestowarea <victim> list             lists bestowed areas\r\n"
                    "bestowarea <victim>                  lists bestowed areas\r\n", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't give special abilities to a mob!\r\n", ch );
      return;
   }

   if( get_trust( victim ) < LEVEL_IMMORTAL )
   {
      send_to_char( "They aren't an immortal.\r\n", ch );
      return;
   }

   if( !victim->pcdata->bestowments )
      victim->pcdata->bestowments = str_dup( "" );

   if( !*argument || !str_cmp( argument, "list" ) )
   {
      extract_area_names( victim->pcdata->bestowments, buf );
      ch_printf( ch, "Bestowed areas: %s\r\n", buf );
      return;
   }

   if( !str_cmp( argument, "none" ) )
   {
      remove_area_names( victim->pcdata->bestowments, buf );
      smash_tilde( buf );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   arg_len = strlen( argument );
   if( arg_len < 5
       || argument[arg_len - 4] != '.' || argument[arg_len - 3] != 'a'
       || argument[arg_len - 2] != 'r' || argument[arg_len - 1] != 'e' )
   {
      send_to_char( "You can only bestow an area name\r\n", ch );
      send_to_char( "E.G. bestow joe sam.are\r\n", ch );
      return;
   }

   snprintf( buf, MAX_STRING_LENGTH, "%s %s", victim->pcdata->bestowments, argument );
   DISPOSE( victim->pcdata->bestowments );
   victim->pcdata->bestowments = str_dup( buf );
   set_char_color( AT_IMMORT, victim );
   ch_printf( victim, "%s has bestowed on you the area: %s\r\n", ch->name, argument );
   send_to_char( "Done.\r\n", ch );
}

void do_bestow( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], arg_buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   CMDTYPE *cmd;
   bool fComm = FALSE;

   const char *const bestow_exceptions[] = {
      "protoflag", "caninduct", "canoutcast"
   };

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Bestow whom with what?\r\n", ch );
      return;
   }
   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't give special abilities to a mob!\r\n", ch );
      return;
   }
   if( victim == ch || get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You aren't powerful enough...\r\n", ch );
      return;
   }

   if( !victim->pcdata->bestowments )
      victim->pcdata->bestowments = str_dup( "" );

   if( argument[0] == '\0' || !str_cmp( argument, "show list" ) )
   {
      ch_printf( ch, "Current bestowed commands on %s: %s.\r\n", victim->name, victim->pcdata->bestowments );
      return;
   }

   if( !str_cmp( argument, "none" ) )
   {
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( "" );
      ch_printf( ch, "Bestowments removed from %s.\r\n", victim->name );
      ch_printf( victim, "%s has removed your bestowed commands.\r\n", ch->name );
      check_switch( victim, FALSE );
      return;
   }

   arg_buf[0] = '\0';

   argument = one_argument( argument, arg );

   while( arg[0] != '\0' )
   {
      const char *cmd_buf;
      char cmd_tmp[MAX_INPUT_LENGTH];
      short numExceptions = ( sizeof( bestow_exceptions ) / sizeof( bestow_exceptions[0] ) );
      bool cFound = FALSE, isException = FALSE;

      if( get_trust( ch ) < LEVEL_GREATER )
      {
         for( int count = 0; count < numExceptions; count++ )
         {
            if( !str_cmp( arg, bestow_exceptions[count] ) )
            {
               isException = TRUE;
               break;
            }
         }  
      }

      if( !isException && !( cmd = find_command( arg ) ) )
      {
         ch_printf( ch, "No such command as %s!\r\n", arg );
         argument = one_argument( argument, arg );
         continue;
      }
      else if( !isException && cmd->level > get_trust( ch ) )
      {
         ch_printf( ch, "You can't bestow the %s command!\r\n", arg );
         argument = one_argument( argument, arg );
         continue;
      }

      cmd_buf = victim->pcdata->bestowments;
      cmd_buf = one_argument( cmd_buf, cmd_tmp );
      while( cmd_tmp[0] != '\0' )
      {
         if( !str_cmp( cmd_tmp, arg ) )
         {
            cFound = TRUE;
            break;
         }
         cmd_buf = one_argument( cmd_buf, cmd_tmp );
      }

      if( cFound == TRUE )
      {
         argument = one_argument( argument, arg );
         continue;
      }

      mudstrlcat( arg_buf, " ", MAX_STRING_LENGTH );
      mudstrlcat( arg_buf, arg, MAX_STRING_LENGTH );
      argument = one_argument( argument, arg );
      fComm = TRUE;
   }

   if( !fComm )
   {
      send_to_char( "Good job, knucklehead... you just bestowed them with that master command called 'NOTHING!'\r\n", ch );
      return;
   }

   if( arg_buf[strlen( arg_buf ) - 1] == ' ' )
      arg_buf[strlen( arg_buf ) - 1] = '\0';

   snprintf( buf, MAX_STRING_LENGTH, "%s %s", victim->pcdata->bestowments, arg_buf );
   DISPOSE( victim->pcdata->bestowments );
   smash_tilde( buf );
   victim->pcdata->bestowments = str_dup( buf );
   set_char_color( AT_IMMORT, victim );
   ch_printf( victim, "%s has bestowed on you the command(s): %s\r\n", ch->name, arg_buf );
   send_to_char( "Done.\r\n", ch );
}

struct tm *update_time( struct tm *old_time )
{
   time_t sttime;

   sttime = mktime( old_time );
   return localtime( &sttime );
}

void do_set_boot_time( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   bool check;

   check = FALSE;
   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax: setboot time {hour minute <day> <month> <year>}\r\n", ch );
      send_to_char( "        setboot manual {0/1}\r\n", ch );
      send_to_char( "        setboot default\r\n", ch );
      ch_printf( ch, "Boot time is currently set to %s, manual bit is set to %d\r\n", reboot_time, set_boot_time->manual );
      return;
   }

   if( !str_cmp( arg, "time" ) )
   {
      struct tm *now_time;

      argument = one_argument( argument, arg );
      argument = one_argument( argument, arg1 );
      if( !*arg || !*arg1 || !is_number( arg ) || !is_number( arg1 ) )
      {
         send_to_char( "You must input a value for hour and minute.\r\n", ch );
         return;
      }

      now_time = localtime( &current_time );
      if( ( now_time->tm_hour = atoi( arg ) ) < 0 || now_time->tm_hour > 23 )
      {
         send_to_char( "Valid range for hour is 0 to 23.\r\n", ch );
         return;
      }
      if( ( now_time->tm_min = atoi( arg1 ) ) < 0 || now_time->tm_min > 59 )
      {
         send_to_char( "Valid range for minute is 0 to 59.\r\n", ch );
         return;
      }

      argument = one_argument( argument, arg );
      if( *arg != '\0' && is_number( arg ) )
      {
         if( ( now_time->tm_mday = atoi( arg ) ) < 1 || now_time->tm_mday > 31 )
         {
            send_to_char( "Valid range for day is 1 to 31.\r\n", ch );
            return;
         }
         argument = one_argument( argument, arg );
         if( *arg != '\0' && is_number( arg ) )
         {
            if( ( now_time->tm_mon = atoi( arg ) ) < 1 || now_time->tm_mon > 12 )
            {
               send_to_char( "Valid range for month is 1 to 12.\r\n", ch );
               return;
            }
            now_time->tm_mon--;
            argument = one_argument( argument, arg );
            if( ( now_time->tm_year = atoi( arg ) - 1900 ) < 0 || now_time->tm_year > 199 )
            {
               send_to_char( "Valid range for year is 1900 to 2099.\r\n", ch );
               return;
            }
         }
      }

      now_time->tm_sec = 0;
      if( mktime( now_time ) < current_time )
      {
         send_to_char( "You can't set a time previous to today!\r\n", ch );
         return;
      }
      if( set_boot_time->manual == 0 )
         set_boot_time->manual = 1;
      new_boot_time = update_time( now_time );
      new_boot_struct = *new_boot_time;
      new_boot_time = &new_boot_struct;
      reboot_check( mktime( new_boot_time ) );
      get_reboot_string(  );

      ch_printf( ch, "Boot time set to %s\r\n", reboot_time );
      check = TRUE;
   }
   else if( !str_cmp( arg, "manual" ) )
   {
      argument = one_argument( argument, arg1 );
      if( arg1[0] == '\0' )
      {
         send_to_char( "Please enter a value for manual boot on/off\r\n", ch );
         return;
      }
      if( !is_number( arg1 ) )
      {
         send_to_char( "Value for manual must be 0 (off) or 1 (on)\r\n", ch );
         return;
      }
      if( atoi( arg1 ) < 0 || atoi( arg1 ) > 1 )
      {
         send_to_char( "Value for manual must be 0 (off) or 1 (on)\r\n", ch );
         return;
      }

      set_boot_time->manual = atoi( arg1 );
      ch_printf( ch, "Manual bit set to %s\r\n", arg1 );
      check = TRUE;
      get_reboot_string(  );
      return;
   }

   else if( !str_cmp( arg, "default" ) )
   {
      set_boot_time->manual = 0;
      /*
       * Reinitialize new_boot_time 
       */
      new_boot_time = localtime( &current_time );
      new_boot_time->tm_mday += 1;
      if( new_boot_time->tm_hour > 12 )
         new_boot_time->tm_mday += 1;
      new_boot_time->tm_hour = 6;
      new_boot_time->tm_min = 0;
      new_boot_time->tm_sec = 0;
      new_boot_time = update_time( new_boot_time );

      sysdata.DENY_NEW_PLAYERS = FALSE;

      send_to_char( "Reboot time set back to normal.\r\n", ch );
      check = TRUE;
   }

   if( !check )
   {
      send_to_char( "Invalid argument for setboot.\r\n", ch );
      return;
   }
   else
   {
      get_reboot_string(  );
      new_boot_time_t = mktime( new_boot_time );
   }
}

void do_form_password( CHAR_DATA* ch, const char* argument)
{
   char *pwcheck;

   set_char_color( AT_IMMORT, ch );

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: formpass <password>\r\n", ch );
      return;
   }

   /*
    * This is arbitrary to discourage weak passwords 
    */
   if( strlen( argument ) < 5 )
   {
      send_to_char( "Usage: formpass <password>\r\n", ch );
      send_to_char( "New password must be at least 5 characters in length.\r\n", ch );
      return;
   }

   if( argument[0] == '!' )
   {
      send_to_char( "Usage: formpass <password>\r\n", ch );
      send_to_char( "New password cannot begin with the '!' character.\r\n", ch );
      return;
   }

   pwcheck = sha256_crypt( argument );
   ch_printf( ch, "%s results in the encrypted string: %s\r\n", argument, pwcheck );
   return;
}

/*
 * Purge a player file.  No more player.  -- Altrag
 */
void do_destro( CHAR_DATA* ch, const char* argument)
{
   set_char_color( AT_RED, ch );
   send_to_char( "If you want to destroy a character, spell it out!\r\n", ch );
   return;
}

/*
 * This could have other applications too.. move if needed. -- Altrag
 */
void close_area( AREA_DATA * pArea )
{
   CHAR_DATA *ech, *ech_next;
   OBJ_DATA *eobj, *eobj_next;
   ROOM_INDEX_DATA *rid, *rid_next;
   OBJ_INDEX_DATA *oid, *oid_next;
   MOB_INDEX_DATA *mid, *mid_next;
   int icnt;

   for( ech = first_char; ech; ech = ech_next )
   {
      ech_next = ech->next;

      if( ech->fighting )
         stop_fighting( ech, TRUE );
      if( IS_NPC( ech ) )
      {
         /*
          * if mob is in area, or part of area. 
          */
         if( URANGE( pArea->low_m_vnum, ech->pIndexData->vnum, pArea->hi_m_vnum ) == ech->pIndexData->vnum
             || ( ech->in_room && ech->in_room->area == pArea ) )
            extract_char( ech, TRUE );
         continue;
      }
      if( ech->in_room && ech->in_room->area == pArea )
         do_recall( ech, "" );
   }

   for( eobj = first_object; eobj; eobj = eobj_next )
   {
      eobj_next = eobj->next;
      /*
       * if obj is in area, or part of area. 
       */
      if( URANGE( pArea->low_o_vnum, eobj->pIndexData->vnum, pArea->hi_o_vnum ) == eobj->pIndexData->vnum
          || ( eobj->in_room && eobj->in_room->area == pArea ) )
         extract_obj( eobj );
   }

   for( icnt = 0; icnt < MAX_KEY_HASH; icnt++ )
   {
      for( rid = room_index_hash[icnt]; rid; rid = rid_next )
      {
         rid_next = rid->next;

         if( rid->area != pArea )
            continue;
         delete_room( rid );
      }
      pArea->first_room = pArea->last_room = NULL;

      for( mid = mob_index_hash[icnt]; mid; mid = mid_next )
      {
         mid_next = mid->next;

         if( mid->vnum < pArea->low_m_vnum || mid->vnum > pArea->hi_m_vnum )
            continue;
         delete_mob( mid );
      }

      for( oid = obj_index_hash[icnt]; oid; oid = oid_next )
      {
         oid_next = oid->next;

         if( oid->vnum < pArea->low_o_vnum || oid->vnum > pArea->hi_o_vnum )
            continue;
         delete_obj( oid );
      }
   }

   DISPOSE( pArea->name );
   DISPOSE( pArea->filename );
   DISPOSE( pArea->resetmsg );
   STRFREE( pArea->author );
   if( IS_SET( pArea->flags, AFLAG_PROTOTYPE ) )
   {
      UNLINK( pArea, first_build, last_build, next, prev );
      UNLINK( pArea, first_bsort, last_bsort, next_sort, prev_sort );
   }
   else
   {
      UNLINK( pArea, first_area, last_area, next, prev );
      UNLINK( pArea, first_asort, last_asort, next_sort, prev_sort );
      UNLINK( pArea, first_area_name, last_area_name, next_sort_name, prev_sort_name );
   }
   DISPOSE( pArea );
}

void close_all_areas( void )
{
   AREA_DATA *area, *area_next;

   for( area = first_area; area; area = area_next )
   {
      area_next = area->next;
      close_area( area );
   }
   for( area = first_build; area; area = area_next )
   {
      area_next = area->next;
      close_area( area );
   }
   return;
}

void do_destroy( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char *name;
   struct stat fst;

   set_char_color( AT_RED, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Destroy what player file?\r\n", ch );
      return;
   }

   /*
    * Set the file points.
    */
   name = capitalize( arg );
   snprintf( buf, MAX_STRING_LENGTH, "%s%c/%s", PLAYER_DIR, tolower( arg[0] ), name );
   snprintf( buf2, MAX_STRING_LENGTH, "%s%c/%s", BACKUP_DIR, tolower( arg[0] ), name );

   /*
    * This check makes sure the name is valid and that the file is there, else there
    * is no need to go on. -Orion
    */
   if( !check_parse_name( name, TRUE ) || lstat( buf, &fst ) == -1 )
   {
      ch_printf( ch, "No player exists by the name %s.\r\n", name );
      return;
   }

   for( victim = first_char; victim; victim = victim->next )
      if( !IS_NPC( victim ) && !str_cmp( victim->name, arg ) )
         break;

   if( !victim )
   {
      DESCRIPTOR_DATA *d;

      /*
       * Make sure they aren't halfway logged in. 
       */
      for( d = first_descriptor; d; d = d->next )
         if( ( victim = d->character ) && !IS_NPC( victim ) && !str_cmp( victim->name, arg ) )
            break;
      if( d )
         close_socket( d, TRUE );
   }
   else
   {
      int x, y;

      quitting_char = victim;
      save_char_obj( victim );
      saving_char = NULL;
      extract_char( victim, TRUE );
      for( x = 0; x < MAX_WEAR; x++ )
         for( y = 0; y < MAX_LAYERS; y++ )
            save_equipment[x][y] = NULL;
   }

   if( !rename( buf, buf2 ) )
   {
      AREA_DATA *pArea;

      set_char_color( AT_RED, ch );
      ch_printf( ch, "Player %s destroyed.  Pfile saved in backup directory.\r\n", name );
      snprintf( buf, MAX_STRING_LENGTH, "%s%s", GOD_DIR, name );
      if( !remove( buf ) )
         send_to_char( "Player's immortal data destroyed.\r\n", ch );
      else if( errno != ENOENT )
      {
         ch_printf( ch, "Unknown error #%d - %s (immortal data). Report to www.smaugmuds.org\r\n", errno,
                    strerror( errno ) );
         snprintf( buf2, MAX_STRING_LENGTH, "%s destroying %s", ch->name, buf );
         perror( buf2 );
      }

      snprintf( buf2, MAX_STRING_LENGTH, "%s.are", name );
      for( pArea = first_build; pArea; pArea = pArea->next )
         if( !str_cmp( pArea->filename, buf2 ) )
         {
            snprintf( buf, MAX_STRING_LENGTH, "%s%s", BUILD_DIR, buf2 );
            if( IS_SET( pArea->status, AREA_LOADED ) )
               fold_area( pArea, buf, FALSE );
            close_area( pArea );
            snprintf( buf2, MAX_STRING_LENGTH, "%s.bak", buf );
            set_char_color( AT_RED, ch ); /* Log message changes colors */
            if( !rename( buf, buf2 ) )
               send_to_char( "Player's area data destroyed.  Area saved as backup.\r\n", ch );
            else if( errno != ENOENT )
            {
               ch_printf( ch, "Unknown error #%d - %s (area data). Report to www.smaugmuds.org\r\n", errno,
                          strerror( errno ) );
               snprintf( buf2, MAX_STRING_LENGTH, "%s destroying %s", ch->name, buf );
               perror( buf2 );
            }
            break;
         }
   }
   else if( errno == ENOENT )
   {
      set_char_color( AT_PLAIN, ch );
      send_to_char( "Player does not exist.\r\n", ch );
   }
   else
   {
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "Unknown error #%d - %s. Report to www.smaugmuds.org\r\n", errno, strerror( errno ) );
      snprintf( buf, MAX_STRING_LENGTH, "%s destroying %s", ch->name, arg );
      perror( buf );
   }
   return;
}

/* Super-AT command:
FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>

Executes action several times, either on ALL players (not including yourself),
MORTALS (including trusted characters), GODS (characters with level higher than
L_HERO), MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containg
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example: 

FOR ALL SMAX_INPUT_LENGTHE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with 
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/
const char *name_expand( CHAR_DATA * ch )
{
   int count = 1;
   CHAR_DATA *rch;
   char name[MAX_INPUT_LENGTH];  /*  HOPEFULLY no mob has a name longer than THAT */

   static char outbuf[MAX_INPUT_LENGTH];

   if( !IS_NPC( ch ) )
      return ch->name;

   one_argument( ch->name, name );  /* copy the first word into name */

   if( !name[0] ) /* weird mob .. no keywords */
   {
      mudstrlcpy( outbuf, "", MAX_INPUT_LENGTH );  /* Do not return NULL, just an empty buffer */
      return outbuf;
   }

   /*
    * ->people changed to ->first_person -- TRI 
    */
   for( rch = ch->in_room->first_person; rch && ( rch != ch ); rch = rch->next_in_room )
      if( is_name( name, rch->name ) )
         count++;


   snprintf( outbuf, MAX_INPUT_LENGTH, "%d.%s", count, name );
   return outbuf;
}

void do_for( CHAR_DATA* ch, const char* argument)
{
   char range[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   bool fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fEverywhere = FALSE, found;
   ROOM_INDEX_DATA *room, *old_room;
   CHAR_DATA *p, *p_prev;  /* p_next to p_prev -- TRI */
   int i;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, range );
   if( !range[0] || !argument[0] )  /* invalid usage? */
   {
      do_help( ch, "for" );
      return;
   }

   if( !str_prefix( "quit", argument ) )
   {
      send_to_char( "Are you trying to crash the MUD or something?\r\n", ch );
      return;
   }


   if( !str_cmp( range, "all" ) )
   {
      fMortals = TRUE;
      fGods = TRUE;
   }
   else if( !str_cmp( range, "gods" ) )
      fGods = TRUE;
   else if( !str_cmp( range, "mortals" ) )
      fMortals = TRUE;
   else if( !str_cmp( range, "mobs" ) )
      fMobs = TRUE;
   else if( !str_cmp( range, "everywhere" ) )
      fEverywhere = TRUE;
   else
      do_help( ch, "for" );   /* show syntax */

   /*
    * do not allow # to make it easier 
    */
   if( fEverywhere && strchr( argument, '#' ) )
   {
      send_to_char( "Cannot use FOR EVERYWHERE with the # thingie.\r\n", ch );
      return;
   }

   set_char_color( AT_PLAIN, ch );
   if( strchr( argument, '#' ) ) /* replace # ? */
   {
      /*
       * char_list - last_char, p_next - gch_prev -- TRI 
       */
      for( p = last_char; p; p = p_prev )
      {
         p_prev = p->prev; /* TRI */
         /*
          * p_next = p->next; 
          *//*
          * In case someone DOES try to AT MOBS SLAY # 
          */
         found = FALSE;

         if( !( p->in_room ) || room_is_private( p->in_room ) || ( p == ch ) )
            continue;

         if( IS_NPC( p ) && fMobs )
            found = TRUE;
         else if( !IS_NPC( p ) && p->level >= LEVEL_IMMORTAL && fGods )
            found = TRUE;
         else if( !IS_NPC( p ) && p->level < LEVEL_IMMORTAL && fMortals )
            found = TRUE;

         /*
          * It looks ugly to me.. but it works :) 
          */
         if( found ) /* p is 'appropriate' */
         {
            const char *pSource = argument;  /* head of buffer to be parsed */
            char *pDest = buf;   /* parse into this */

            while( *pSource )
            {
               if( *pSource == '#' )   /* Replace # with name of target */
               {
                  const char *namebuf = name_expand( p );

                  if( namebuf )  /* in case there is no mob name ?? */
                     while( *namebuf ) /* copy name over */
                        *( pDest++ ) = *( namebuf++ );

                  pSource++;
               }
               else
                  *( pDest++ ) = *( pSource++ );
            }  /* while */
            *pDest = '\0'; /* Terminate */

            /*
             * Execute 
             */
            old_room = ch->in_room;
            char_from_room( ch );
            char_to_room( ch, p->in_room );
            interpret( ch, buf );
            char_from_room( ch );
            char_to_room( ch, old_room );

         }  /* if found */
      }  /* for every char */
   }
   else  /* just for every room with the appropriate people in it */
   {
      for( i = 0; i < MAX_KEY_HASH; i++ ) /* run through all the buckets */
         for( room = room_index_hash[i]; room; room = room->next )
         {
            found = FALSE;

            /*
             * Anyone in here at all? 
             */
            if( fEverywhere ) /* Everywhere executes always */
               found = TRUE;
            else if( !room->first_person )   /* Skip it if room is empty */
               continue;
            /*
             * ->people changed to first_person -- TRI 
             */

            /*
             * Check if there is anyone here of the requried type 
             */
            /*
             * Stop as soon as a match is found or there are no more ppl in room 
             */
            /*
             * ->people to ->first_person -- TRI 
             */
            for( p = room->first_person; p && !found; p = p->next_in_room )
            {

               if( p == ch )  /* do not execute on oneself */
                  continue;

               if( IS_NPC( p ) && fMobs )
                  found = TRUE;
               else if( !IS_NPC( p ) && ( p->level >= LEVEL_IMMORTAL ) && fGods )
                  found = TRUE;
               else if( !IS_NPC( p ) && ( p->level <= LEVEL_IMMORTAL ) && fMortals )
                  found = TRUE;
            }  /* for everyone inside the room */

            if( found && !room_is_private( room ) )   /* Any of the required type here AND room not private? */
            {
               /*
                * This may be ineffective. Consider moving character out of old_room
                * once at beginning of command then moving back at the end.
                * This however, is more safe?
                */

               old_room = ch->in_room;
               char_from_room( ch );
               char_to_room( ch, room );
               interpret( ch, argument );
               char_from_room( ch );
               char_to_room( ch, old_room );
            }  /* if found */
         }  /* for every room in a bucket */
   }  /* if strchr */
}  /* do_for */

void update_calendar( void )
{
   sysdata.daysperyear = sysdata.dayspermonth * sysdata.monthsperyear;
   sysdata.hoursunrise = sysdata.hoursperday / 4;
   sysdata.hourdaybegin = sysdata.hoursunrise + 1;
   sysdata.hournoon = sysdata.hoursperday / 2;
   sysdata.hoursunset = ( ( sysdata.hoursperday / 4 ) * 3 );
   sysdata.hournightbegin = sysdata.hoursunset + 1;
   sysdata.hourmidnight = sysdata.hoursperday;
   calc_season(  );
}

void update_timers( void )
{
   sysdata.pulsetick = sysdata.secpertick * sysdata.pulsepersec;
   sysdata.pulseviolence = 3 * sysdata.pulsepersec;
   sysdata.pulsemobile = 4 * sysdata.pulsepersec;
   sysdata.pulsecalendar = 4 * sysdata.pulsetick;
}

void do_cset( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_STRING_LENGTH];
   short level, value;

   set_pager_color( AT_PLAIN, ch );

   if( argument && argument[0] == '\0' )
   {
      pager_printf_color( ch, "\r\n&WMud_name: %s", sysdata.mud_name );
      pager_printf_color( ch,
                          "\r\n&WMail:\r\n  &wRead all mail: &W%d  &wRead mail for free: &W%d  &wWrite mail for free: &W%d\r\n",
                          sysdata.read_all_mail, sysdata.read_mail_free, sysdata.write_mail_free );
      pager_printf_color( ch, "  &wTake all mail: &W%d\r\n", sysdata.take_others_mail );
      pager_printf_color( ch, "&WChannels:\r\n  &wMuse: &W%d   &wThink: &W%d   &wLog: &W%d   &wBuild: &W%d\r\n",
                          sysdata.muse_level, sysdata.think_level, sysdata.log_level, sysdata.build_level );
      pager_printf_color( ch, "&WBuilding:\r\n  &wPrototype modification: &W%d  &wPlayer msetting: &W%d\r\n",
                          sysdata.level_modify_proto, sysdata.level_mset_player );
      pager_printf_color( ch, "&WGuilds:\r\n  &wOverseer: &W%s   &wAdvisor: &W%s\r\n",
                          sysdata.guild_overseer, sysdata.guild_advisor );
      pager_printf_color( ch, "&WBan Data:\r\n  &wBan Site Level: &W%d   &wBan class Level: &W%d   ",
                          sysdata.ban_site_level, sysdata.ban_class_level );
      pager_printf_color( ch, "&wBan Race Level: &W%d\r\n", sysdata.ban_race_level );
      pager_printf_color( ch, "&WDefenses:\r\n  &wDodge_mod: &W%d    &wParry_mod: &W%d    &wTumble_mod: &W%d\r\n",
                          sysdata.dodge_mod, sysdata.parry_mod, sysdata.tumble_mod );
      pager_printf_color( ch, "&WOther:\r\n  &wForce on players:             &W%-2d     ", sysdata.level_forcepc );
      pager_printf_color( ch, "&wPrivate room override:         &W%-2d\r\n", sysdata.level_override_private );
      pager_printf_color( ch, "  &wPenalty to bash plr vs. plr:  &W%-7d", sysdata.bash_plr_vs_plr );
      pager_printf_color( ch, "&wPenalty to non-tank bash:      &W%-3d\r\n", sysdata.bash_nontank );
      pager_printf_color( ch, "  &wPenalty to gouge plr vs. plr: &W%-7d", sysdata.gouge_plr_vs_plr );
      pager_printf_color( ch, "&wPenalty to non-tank gouge:     &W%-3d\r\n", sysdata.gouge_nontank );
      pager_printf_color( ch, "  &wPenalty regular stun chance:  &W%-7d", sysdata.stun_regular );
      pager_printf_color( ch, "&wPenalty to stun plr vs. plr:   &W%-3d\r\n", sysdata.stun_plr_vs_plr );
      pager_printf_color( ch, "  &wPercent damage plr vs. plr:   &W%-7d", sysdata.dam_plr_vs_plr );
      pager_printf_color( ch, "&wPercent damage plr vs. mob:    &W%-3d \r\n", sysdata.dam_plr_vs_mob );
      pager_printf_color( ch, "  &wPercent damage mob vs. plr:   &W%-7d", sysdata.dam_mob_vs_plr );
      pager_printf_color( ch, "&wPercent damage mob vs. mob:    &W%-3d\r\n", sysdata.dam_mob_vs_mob );
      pager_printf_color( ch, "  &wGet object without take flag: &W%-7d", sysdata.level_getobjnotake );
      pager_printf_color( ch, "&wAutosave frequency (minutes):  &W%d\r\n", sysdata.save_frequency );
      pager_printf_color( ch, "  &wMax level difference bestow:  &W%-7d", sysdata.bestow_dif );
      pager_printf_color( ch, "&wChecking Imm_host is:          &W%s\r\n", ( sysdata.check_imm_host ) ? "ON" : "off" );
      pager_printf_color( ch, "  &wMorph Optimization is:        &W%-7s", ( sysdata.morph_opt ) ? "ON" : "off" );
      pager_printf_color( ch, "&wSaving Pets is:                &W%s\r\n", ( sysdata.save_pets ) ? "ON" : "off" );
      pager_printf_color( ch, "  &wPkill looting is:             &W%s\r\n", ( sysdata.pk_loot ) ? "ON" : "off" );
      pager_printf_color( ch, "  &wSave flags: &W%s\r\n", flag_string( sysdata.save_flags, save_flag ) );
      pager_printf_color( ch, "&WCalendar:\r\n" );
      pager_printf_color( ch, "  &wSeconds per tick: &W%d   &wPulse per second: &W%d\n\r", sysdata.secpertick, sysdata.pulsepersec );
      pager_printf_color( ch, "  &wHours per day: &W%d &wDays per week: &W%d &wDays per month: &W%d &wMonths per year: &W%d &wDays per year: &W%d\r\n",
         sysdata.hoursperday, sysdata.daysperweek, sysdata.dayspermonth, sysdata.monthsperyear, sysdata.daysperyear );
      pager_printf_color( ch, "  &wPULSE_TICK: &W%d &wPULSE_VIOLENCE: &W%d &wPULSE_MOBILE: &W%d &wPULSE_CALENDAR: &W%d\r\n",
         sysdata.pulsetick, sysdata.pulseviolence, sysdata.pulsemobile, sysdata.pulsecalendar );
      return;
   }

   argument = one_argument( argument, arg );
   argument = smash_tilde( argument );

   if( !str_cmp( arg, "help" ) )
   {
      do_help( ch, "controls" );
      return;
   }

   if( !str_cmp( arg, "save" ) )
   {
      save_sysdata( sysdata );
      send_to_char( "Cset functions saved.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "mudname" ) )
   {
      if( sysdata.mud_name )
         DISPOSE( sysdata.mud_name );
      sysdata.mud_name = str_dup( argument );
      send_to_char( "Name set.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "saveflag" ) )
   {
      int x = get_saveflag( argument );

      if( x == -1 )
         send_to_char( "Not a save flag.\r\n", ch );
      else
      {
         TOGGLE_BIT( sysdata.save_flags, 1 << x );
         send_to_char( "Ok.\r\n", ch );
      }
      return;
   }

   if( !str_prefix( arg, "guild_overseer" ) )
   {
      STRFREE( sysdata.guild_overseer );
      sysdata.guild_overseer = STRALLOC( argument );
      send_to_char( "Ok.\r\n", ch );
      return;
   }
   if( !str_prefix( arg, "guild_advisor" ) )
   {
      STRFREE( sysdata.guild_advisor );
      sysdata.guild_advisor = STRALLOC( argument );
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   value = ( short )atoi( argument );

   if( !str_cmp( arg, "max-holidays" ) )
   {
      sysdata.maxholiday = value;
      ch_printf( ch, "Max Holiday set to %d.\r\n", value );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "hours-per-day" ) )
   {
      sysdata.hoursperday = value;
      ch_printf( ch, "Hours per day set to %d.\r\n", value );
      update_calendar(  );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "days-per-week" ) )
   {
      sysdata.daysperweek = value;
      ch_printf( ch, "Days per week set to %d.\r\n", value );
      update_calendar(  );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "days-per-month" ) )
   {
      sysdata.dayspermonth = value;
      ch_printf( ch, "Days per month set to %d.\r\n", value );
      update_calendar(  );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "months-per-year" ) )
   {
      sysdata.monthsperyear = value;
      ch_printf( ch, "Months per year set to %d.\r\n", value );
      update_calendar(  );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "seconds-per-tick" ) )
   {
      sysdata.secpertick = value;
      ch_printf( ch, "Seconds per tick set to %d.\r\n", value );
      update_timers(  );
      save_sysdata( sysdata );
      return;
   }

   if( !str_cmp( arg, "pulse-per-second" ) )
   {
      sysdata.pulsepersec = value;
      ch_printf( ch, "Pulse per second set to %d.\r\n", value );
      update_timers(  );
      save_sysdata( sysdata );
      return;
   }

   level = ( short )atoi( argument );

   if( !str_prefix( arg, "savefrequency" ) )
   {
      sysdata.save_frequency = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_prefix( arg, "checkimmhost" ) )
   {
      if( level != 0 && level != 1 )
      {
         send_to_char( "Use 1 to turn it on, 0 to turn in off.\r\n", ch );
         return;
      }
      sysdata.check_imm_host = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "bash_pvp" ) )
   {
      sysdata.bash_plr_vs_plr = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "bash_nontank" ) )
   {
      sysdata.bash_nontank = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "gouge_pvp" ) )
   {
      sysdata.gouge_plr_vs_plr = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "gouge_nontank" ) )
   {
      sysdata.gouge_nontank = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "dodge_mod" ) )
   {
      sysdata.dodge_mod = level > 0 ? level : 1;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "parry_mod" ) )
   {
      sysdata.parry_mod = level > 0 ? level : 1;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "tumble_mod" ) )
   {
      sysdata.tumble_mod = level > 0 ? level : 1;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "stun" ) )
   {
      sysdata.stun_regular = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "stun_pvp" ) )
   {
      sysdata.stun_plr_vs_plr = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "dam_pvp" ) )
   {
      sysdata.dam_plr_vs_plr = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "get_notake" ) )
   {
      sysdata.level_getobjnotake = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "dam_pvm" ) )
   {
      sysdata.dam_plr_vs_mob = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "dam_mvp" ) )
   {
      sysdata.dam_mob_vs_plr = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "dam_mvm" ) )
   {
      sysdata.dam_mob_vs_mob = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( level < 0 || level > MAX_LEVEL )
   {
      send_to_char( "Invalid value for new control.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "read_all" ) )
      sysdata.read_all_mail = level;
   else if( !str_cmp( arg, "read_free" ) )
      sysdata.read_mail_free = level;
   else if( !str_cmp( arg, "write_free" ) )
      sysdata.write_mail_free = level;
   else if( !str_cmp( arg, "take_all" ) )
      sysdata.take_others_mail = level;
   else if( !str_cmp( arg, "muse" ) )
      sysdata.muse_level = level;
   else if( !str_cmp( arg, "think" ) )
      sysdata.think_level = level;
   else if( !str_cmp( arg, "log" ) )
      sysdata.log_level = level;
   else if( !str_cmp( arg, "build" ) )
      sysdata.build_level = level;
   else if( !str_cmp( arg, "proto_modify" ) )
      sysdata.level_modify_proto = level;
   else if( !str_cmp( arg, "override_private" ) )
      sysdata.level_override_private = level;
   else if( !str_cmp( arg, "bestow_dif" ) )
      sysdata.bestow_dif = level > 0 ? level : 1;
   else if( !str_cmp( arg, "forcepc" ) )
      sysdata.level_forcepc = level;
   else if( !str_cmp( arg, "ban_site_level" ) )
      sysdata.ban_site_level = level;
   else if( !str_cmp( arg, "ban_race_level" ) )
      sysdata.ban_race_level = level;
   else if( !str_cmp( arg, "ban_class_level" ) )
      sysdata.ban_class_level = level;
   else if( !str_cmp( arg, "petsave" ) )
   {
      if( level )
         sysdata.save_pets = TRUE;
      else
         sysdata.save_pets = FALSE;
   }
   else if( !str_cmp( arg, "pk_loot" ) )
   {
      if( level )
      {
         send_to_char( "Pkill looting is enabled.\r\n", ch );
         sysdata.pk_loot = TRUE;
      }
      else
      {
         send_to_char( "Pkill looting is disabled.  (use cset pkloot 1 to enable)\r\n", ch );
         sysdata.pk_loot = FALSE;
      }
   }
   else if( !str_cmp( arg, "morph_opt" ) )
   {
      if( level )
         sysdata.morph_opt = TRUE;
      else
         sysdata.morph_opt = FALSE;
   }
   else if( !str_cmp( arg, "mset_player" ) )
      sysdata.level_mset_player = level;
   else
   {
      send_to_char( "Invalid argument.\r\n", ch );
      return;
   }
   send_to_char( "Ok.\r\n", ch );
   return;
}

void get_reboot_string( void )
{
   snprintf( reboot_time, 50, "%s", asctime( new_boot_time ) );
}

void do_hell( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   short htime;
   bool h_d = FALSE;
   struct tm *tms;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "Hell who, and for how long?\r\n", ch );
      return;
   }
   if( !( victim = get_char_world( ch, arg ) ) || IS_NPC( victim ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( IS_IMMORTAL( victim ) )
   {
      send_to_char( "There is no point in helling an immortal.\r\n", ch );
      return;
   }
   if( victim->pcdata->release_date != 0 )
   {
      ch_printf( ch, "They are already in hell until %24.24s, by %s.\r\n",
                 ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );
      return;
   }

   argument = one_argument( argument, arg );
   if( !*arg || !is_number( arg ) )
   {
      send_to_char( "Hell them for how long?\r\n", ch );
      return;
   }

   htime = atoi( arg );
   if( htime <= 0 )
   {
      send_to_char( "You cannot hell for zero or negative time.\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );
   if( !*arg || !str_cmp( arg, "hours" ) )
      h_d = TRUE;
   else if( str_cmp( arg, "days" ) )
   {
      send_to_char( "Is that value in hours or days?\r\n", ch );
      return;
   }
   else if( htime > 30 )
   {
      send_to_char( "You may not hell a person for more than 30 days at a time.\r\n", ch );
      return;
   }
   tms = localtime( &current_time );

   if( h_d )
      tms->tm_hour += htime;
   else
      tms->tm_mday += htime;
   victim->pcdata->release_date = mktime( tms );
   victim->pcdata->helled_by = STRALLOC( ch->name );
   ch_printf( ch, "%s will be released from hell at %24.24s.\r\n", victim->name, ctime( &victim->pcdata->release_date ) );
   act( AT_MAGIC, "$n disappears in a cloud of hellish light.", victim, NULL, ch, TO_NOTVICT );
   char_from_room( victim );
   char_to_room( victim, get_room_index( ROOM_VNUM_HELL ) );
   act( AT_MAGIC, "$n appears in a could of hellish light.", victim, NULL, ch, TO_NOTVICT );
   do_look( victim, "auto" );
   ch_printf( victim, "The immortals are not pleased with your actions.\r\n"
              "You shall remain in hell for %d %s%s.\r\n", htime, ( h_d ? "hour" : "day" ), ( htime == 1 ? "" : "s" ) );
   save_char_obj( victim );   /* used to save ch, fixed by Thoric 09/17/96 */
   return;
}

void do_unhell( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "Unhell whom..?\r\n", ch );
      return;
   }
   location = ch->in_room;
/*ch->in_room = get_room_index(ROOM_VNUM_HELL);*/
   victim = get_char_world( ch, arg );
/*ch->in_room = location;          The case of unhell self, etc.*/
   if( !victim || IS_NPC( victim ) )
   {
      send_to_char( "No such player character present.\r\n", ch );
      return;
   }
   if( victim->in_room->vnum != ROOM_VNUM_HELL )
   {
      send_to_char( "No one like that is in hell.\r\n", ch );
      return;
   }

   if( victim->pcdata->clan )
      location = get_room_index( victim->pcdata->clan->recall );
   else
      location = get_room_index( ROOM_VNUM_TEMPLE );
   if( !location )
      location = ch->in_room;
   MOBtrigger = FALSE;
   act( AT_MAGIC, "$n disappears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
   char_from_room( victim );
   char_to_room( victim, location );
   send_to_char( "The gods have smiled on you and released you from hell early!\r\n", victim );
   do_look( victim, "auto" );
   if( victim != ch )
      send_to_char( "They have been released.\r\n", ch );
   if( victim->pcdata->helled_by )
   {
      if( str_cmp( ch->name, victim->pcdata->helled_by ) )
         ch_printf( ch, "(You should probably write a note to %s, explaining the early release.)\r\n",
                    victim->pcdata->helled_by );
      STRFREE( victim->pcdata->helled_by );
      victim->pcdata->helled_by = NULL;
   }

   MOBtrigger = FALSE;
   act( AT_MAGIC, "$n appears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
   victim->pcdata->release_date = 0;
   save_char_obj( victim );
   return;
}

/* Vnum search command by Swordbearer */
void do_vsearch( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   bool found = FALSE;
   OBJ_DATA *obj;
   OBJ_DATA *in_obj;
   int obj_counter = 1;
   int argi;

   set_pager_color( AT_PLAIN, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax:  vsearch <vnum>.\r\n", ch );
      return;
   }

   argi = atoi( arg );
   if( argi < 1 || argi > MAX_VNUM )
   {
      send_to_char( "Vnum out of range.\r\n", ch );
      return;
   }
   for( obj = first_object; obj != NULL; obj = obj->next )
   {
      if( !can_see_obj( ch, obj ) || !( argi == obj->pIndexData->vnum ) )
         continue;

      found = TRUE;
      for( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );

      if( in_obj->carried_by != NULL )
         pager_printf( ch, "[%2d] Level %d %s carried by %s.\r\n",
                       obj_counter, obj->level, obj_short( obj ), PERS( in_obj->carried_by, ch ) );
      else
         pager_printf( ch, "[%2d] [%-5d] %s in %s.\r\n", obj_counter,
                       ( ( in_obj->in_room ) ? in_obj->in_room->vnum : 0 ),
                       obj_short( obj ), ( in_obj->in_room == NULL ) ? "somewhere" : in_obj->in_room->name );

      obj_counter++;
   }

   if( !found )
      send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );
   return;
}

/* 
 * Simple function to let any imm make any player instantly sober.
 * Saw no need for level restrictions on this.
 * Written by Narn, Apr/96 
 */
void do_sober( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];

   set_char_color( AT_IMMORT, ch );

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on mobs.\r\n", ch );
      return;
   }

   if( victim->pcdata )
      victim->pcdata->condition[COND_DRUNK] = 0;
   send_to_char( "Ok.\r\n", ch );
   set_char_color( AT_IMMORT, victim );
   send_to_char( "You feel sober again.\r\n", victim );
   return;
}

/*
 * Free a social structure					-Thoric
 */
void free_social( SOCIALTYPE * social )
{
   if( social->name )
      DISPOSE( social->name );
   if( social->char_no_arg )
      DISPOSE( social->char_no_arg );
   if( social->others_no_arg )
      DISPOSE( social->others_no_arg );
   if( social->char_found )
      DISPOSE( social->char_found );
   if( social->others_found )
      DISPOSE( social->others_found );
   if( social->vict_found )
      DISPOSE( social->vict_found );
   if( social->char_auto )
      DISPOSE( social->char_auto );
   if( social->others_auto )
      DISPOSE( social->others_auto );
   DISPOSE( social );
}

void free_socials( void )
{
   SOCIALTYPE *social, *social_next;
   int hash;

   for( hash = 0; hash < 27; hash++ )
   {
      for( social = social_index[hash]; social; social = social_next )
      {
         social_next = social->next;
         free_social( social );
      }
   }
   return;
}

/*
 * Remove a social from it's hash index				-Thoric
 */
void unlink_social( SOCIALTYPE * social )
{
   SOCIALTYPE *tmp, *tmp_next;
   int hash;

   if( !social )
   {
      bug( "%s", "Unlink_social: NULL social" );
      return;
   }

   if( social->name[0] < 'a' || social->name[0] > 'z' )
      hash = 0;
   else
      hash = ( social->name[0] - 'a' ) + 1;

   if( social == ( tmp = social_index[hash] ) )
   {
      social_index[hash] = tmp->next;
      return;
   }
   for( ; tmp; tmp = tmp_next )
   {
      tmp_next = tmp->next;
      if( social == tmp_next )
      {
         tmp->next = tmp_next->next;
         return;
      }
   }
}

/*
 * Add a social to the social index table			-Thoric
 * Hashed and insert sorted
 */
void add_social( SOCIALTYPE * social )
{
   int hash, x;
   SOCIALTYPE *tmp, *prev;

   if( !social )
   {
      bug( "%s", "Add_social: NULL social" );
      return;
   }

   if( !social->name )
   {
      bug( "%s", "Add_social: NULL social->name" );
      return;
   }

   if( !social->char_no_arg )
   {
      bug( "%s", "Add_social: NULL social->char_no_arg" );
      return;
   }

   /*
    * make sure the name is all lowercase 
    * evil hack to cast to non-const...
    */
   for( x = 0; social->name[x] != '\0'; x++ )
      ((char*)social->name)[x] = LOWER( social->name[x] );

   if( social->name[0] < 'a' || social->name[0] > 'z' )
      hash = 0;
   else
      hash = ( social->name[0] - 'a' ) + 1;

   if( ( prev = tmp = social_index[hash] ) == NULL )
   {
      social->next = social_index[hash];
      social_index[hash] = social;
      return;
   }

   for( ; tmp; tmp = tmp->next )
   {
      if( ( x = strcmp( social->name, tmp->name ) ) == 0 )
      {
         bug( "Add_social: trying to add duplicate name to bucket %d", hash );
         free_social( social );
         return;
      }
      else if( x < 0 )
      {
         if( tmp == social_index[hash] )
         {
            social->next = social_index[hash];
            social_index[hash] = social;
            return;
         }
         prev->next = social;
         social->next = tmp;
         return;
      }
      prev = tmp;
   }

   /*
    * add to end 
    */
   prev->next = social;
   social->next = NULL;
   return;
}

/*
 * Social editor/displayer/save/delete				-Thoric
 */
void do_sedit( CHAR_DATA* ch, const char* argument)
{
   SOCIALTYPE *social;
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

   set_char_color( AT_SOCIAL, ch );

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: sedit <social> [field]\r\n", ch );
      send_to_char( "Syntax: sedit <social> create\r\n", ch );
      if( get_trust( ch ) > LEVEL_GOD )
         send_to_char( "Syntax: sedit <social> delete\r\n", ch );
      if( get_trust( ch ) > LEVEL_LESSER )
         send_to_char( "Syntax: sedit <save>\r\n", ch );
      send_to_char( "\r\nField being one of:\r\n", ch );
      send_to_char( "  cnoarg onoarg cfound ofound vfound cauto oauto\r\n", ch );
      return;
   }

   if( get_trust( ch ) > LEVEL_LESSER && !str_cmp( arg1, "save" ) )
   {
      save_socials(  );
      send_to_char( "Saved.\r\n", ch );
      return;
   }

   social = find_social( arg1 );
   if( !str_cmp( arg2, "create" ) )
   {
      if( social )
      {
         send_to_char( "That social already exists!\r\n", ch );
         return;
      }
      CREATE( social, SOCIALTYPE, 1 );
      social->name = str_dup( arg1 );
      snprintf( arg2, MAX_INPUT_LENGTH, "You %s.", arg1 );
      social->char_no_arg = str_dup( arg2 );
      add_social( social );
      send_to_char( "Social added.\r\n", ch );
      return;
   }

   if( !social )
   {
      send_to_char( "Social not found.\r\n", ch );
      return;
   }

   if( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
   {
      ch_printf( ch, "Social: %s\r\n\r\nCNoArg: %s\r\n", social->name, social->char_no_arg );
      ch_printf( ch, "ONoArg: %s\r\nCFound: %s\r\nOFound: %s\r\n",
                 social->others_no_arg ? social->others_no_arg : "(not set)",
                 social->char_found ? social->char_found : "(not set)",
                 social->others_found ? social->others_found : "(not set)" );
      ch_printf( ch, "VFound: %s\r\nCAuto : %s\r\nOAuto : %s\r\n",
                 social->vict_found ? social->vict_found : "(not set)",
                 social->char_auto ? social->char_auto : "(not set)",
                 social->others_auto ? social->others_auto : "(not set)" );
      return;
   }

   if( get_trust( ch ) > LEVEL_GOD && !str_cmp( arg2, "delete" ) )
   {
      unlink_social( social );
      free_social( social );
      send_to_char( "Deleted.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "cnoarg" ) )
   {
      if( argument[0] == '\0' || !str_cmp( argument, "clear" ) )
      {
         send_to_char( "You cannot clear this field.  It must have a message.\r\n", ch );
         return;
      }
      if( social->char_no_arg )
         DISPOSE( social->char_no_arg );
      social->char_no_arg = str_dup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "onoarg" ) )
   {
      if( social->others_no_arg )
         DISPOSE( social->others_no_arg );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->others_no_arg = str_dup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "cfound" ) )
   {
      if( social->char_found )
         DISPOSE( social->char_found );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->char_found = str_dup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "ofound" ) )
   {
      if( social->others_found )
         DISPOSE( social->others_found );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->others_found = str_dup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "vfound" ) )
   {
      if( social->vict_found )
         DISPOSE( social->vict_found );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->vict_found = str_dup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "cauto" ) )
   {
      if( social->char_auto )
         DISPOSE( social->char_auto );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->char_auto = str_dup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "oauto" ) )
   {
      if( social->others_auto )
         DISPOSE( social->others_auto );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->others_auto = str_dup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( get_trust( ch ) > LEVEL_GREATER && !str_cmp( arg2, "name" ) )
   {
      bool relocate;
      SOCIALTYPE *checksocial;

      one_argument( argument, arg1 );
      if( arg1[0] == '\0' )
      {
         send_to_char( "Cannot clear name field!\r\n", ch );
         return;
      }
      if( ( checksocial = find_social( arg1 ) ) != NULL )
      {
         ch_printf( ch, "There is already a social named %s.\r\n", arg1 );
         return;
      }
      if( arg1[0] != social->name[0] )
      {
         unlink_social( social );
         relocate = TRUE;
      }
      else
         relocate = FALSE;
      if( social->name )
         DISPOSE( social->name );
      social->name = str_dup( arg1 );
      if( relocate )
         add_social( social );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   /*
    * display usage message 
    */
   do_sedit( ch, "" );
}

/*
 * Free a command structure					-Thoric
 */
void free_command( CMDTYPE * command )
{
   if( command->name )
      DISPOSE( command->name );
   if( command->fun_name )
      DISPOSE( command->fun_name );
   DISPOSE( command );
}

void free_commands( void )
{
   CMDTYPE *command, *cmd_next;
   int hash;

   for( hash = 0; hash < 126; hash++ )
   {
      for( command = command_hash[hash]; command; command = cmd_next )
      {
         cmd_next = command->next;
         command->next = NULL;
         command->do_fun = NULL;
         free_command( command );
      }
   }
   return;
}

/*
 * Remove a command from it's hash index			-Thoric
 */
void unlink_command( CMDTYPE * command )
{
   CMDTYPE *tmp, *tmp_next;
   int hash;

   if( !command )
   {
      bug( "%s", "Unlink_command NULL command" );
      return;
   }

   hash = command->name[0] % 126;

   if( command == ( tmp = command_hash[hash] ) )
   {
      command_hash[hash] = tmp->next;
      return;
   }
   for( ; tmp; tmp = tmp_next )
   {
      tmp_next = tmp->next;
      if( command == tmp_next )
      {
         tmp->next = tmp_next->next;
         return;
      }
   }
}

/*
 * Add a command to the command hash table			-Thoric
 */
void add_command( CMDTYPE * command )
{
   int hash, x;
   CMDTYPE *tmp, *prev;

   if( !command )
   {
      bug( "%s", "Add_command: NULL command" );
      return;
   }

   if( !command->name )
   {
      bug( "%s", "Add_command: NULL command->name" );
      return;
   }

   if( !command->do_fun )
   {
      bug( "%s", "Add_command: NULL command->do_fun" );
      return;
   }

   /*
    * make sure the name is all lowercase 
    * evil hack to cast to non-const...
    */
   for( x = 0; command->name[x] != '\0'; x++ )
      ((char*)command->name)[x] = LOWER( command->name[x] );

   hash = command->name[0] % 126;

   if( ( prev = tmp = command_hash[hash] ) == NULL )
   {
      command->next = command_hash[hash];
      command_hash[hash] = command;
      return;
   }

   /*
    * add to the END of the list 
    */
   for( ; tmp; tmp = tmp->next )
      if( !tmp->next )
      {
         tmp->next = command;
         command->next = NULL;
      }
   return;
}

/*
 * Command editor/displayer/save/delete				-Thoric
 * Added support for interpret flags                            -Shaddai
 */
void do_cedit( CHAR_DATA* ch, const char* argument)
{
   CMDTYPE *command;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   set_char_color( AT_IMMORT, ch );

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: cedit save cmdtable\r\n", ch );
      if( get_trust( ch ) > LEVEL_SUB_IMPLEM )
      {
         send_to_char( "Syntax: cedit <command> create [code]\r\n", ch );
         send_to_char( "Syntax: cedit <command> delete\r\n", ch );
         send_to_char( "Syntax: cedit <command> show\r\n", ch );
         send_to_char( "Syntax: cedit <command> raise\r\n", ch );
         send_to_char( "Syntax: cedit <command> lower\r\n", ch );
         send_to_char( "Syntax: cedit <command> list\r\n", ch );
         send_to_char( "Syntax: cedit <command> [field]\r\n", ch );
         send_to_char( "\r\nField being one of:\r\n", ch );
         send_to_char( "  level position log code flags\r\n", ch );
      }
      return;
   }

   if( get_trust( ch ) > LEVEL_GREATER && !str_cmp( arg1, "save" ) && !str_cmp( arg2, "cmdtable" ) )
   {
      save_commands(  );
      send_to_char( "Saved.\r\n", ch );
      return;
   }

   command = find_command( arg1 );
   if( get_trust( ch ) > LEVEL_SUB_IMPLEM && !str_cmp( arg2, "create" ) )
   {
      if( command )
      {
         send_to_char( "That command already exists!\r\n", ch );
         return;
      }
      CREATE( command, CMDTYPE, 1 );
      command->lag_count = 0; /* FB */
      command->name = str_dup( arg1 );
      command->level = get_trust( ch );
      if( *argument )
         one_argument( argument, arg2 );
      else
         snprintf( arg2, MAX_INPUT_LENGTH, "do_%s", arg1 );
      command->do_fun = skill_function( arg2 );
      command->fun_name = str_dup( arg2 );
      add_command( command );
      send_to_char( "Command added.\r\n", ch );
      if( command->do_fun == skill_notfound )
         ch_printf( ch, "Code %s not found.  Set to no code.\r\n", arg2 );
      return;
   }

   if( !command )
   {
      send_to_char( "Command not found.\r\n", ch );
      return;
   }
   else if( command->level > get_trust( ch ) )
   {
      send_to_char( "You cannot touch this command.\r\n", ch );
      return;
   }

   if( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
   {
      ch_printf( ch, "Command:  %s\r\nLevel:    %d\r\nPosition: %d\r\nLog:      %d\r\nCode:     %s\r\nFlags:  %s\r\n",
                 command->name, command->level, command->position, command->log,
                 command->fun_name, flag_string( command->flags, cmd_flags ) );
      if( command->userec.num_uses )
         send_timer( &command->userec, ch );
      return;
   }

   if( get_trust( ch ) <= LEVEL_SUB_IMPLEM )
   {
      do_cedit( ch, "" );
      return;
   }

   if( !str_cmp( arg2, "raise" ) )
   {
      CMDTYPE *tmp, *tmp_next;
      int hash = command->name[0] % 126;

      if( ( tmp = command_hash[hash] ) == command )
      {
         send_to_char( "That command is already at the top.\r\n", ch );
         return;
      }
      if( tmp->next == command )
      {
         command_hash[hash] = command;
         tmp_next = tmp->next;
         tmp->next = command->next;
         command->next = tmp;
         ch_printf( ch, "Moved %s above %s.\r\n", command->name, command->next->name );
         return;
      }
      for( ; tmp; tmp = tmp->next )
      {
         tmp_next = tmp->next;
         if( tmp_next->next == command )
         {
            tmp->next = command;
            tmp_next->next = command->next;
            command->next = tmp_next;
            ch_printf( ch, "Moved %s above %s.\r\n", command->name, command->next->name );
            return;
         }
      }
      send_to_char( "ERROR -- Not Found!\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "lower" ) )
   {
      CMDTYPE *tmp, *tmp_next;
      int hash = command->name[0] % 126;

      if( command->next == NULL )
      {
         send_to_char( "That command is already at the bottom.\r\n", ch );
         return;
      }
      tmp = command_hash[hash];
      if( tmp == command )
      {
         tmp_next = tmp->next;
         command_hash[hash] = command->next;
         command->next = tmp_next->next;
         tmp_next->next = command;

         ch_printf( ch, "Moved %s below %s.\r\n", command->name, tmp_next->name );
         return;
      }
      for( ; tmp; tmp = tmp->next )
      {
         if( tmp->next == command )
         {
            tmp_next = command->next;
            tmp->next = tmp_next;
            command->next = tmp_next->next;
            tmp_next->next = command;

            ch_printf( ch, "Moved %s below %s.\r\n", command->name, tmp_next->name );
            return;
         }
      }
      send_to_char( "ERROR -- Not Found!\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "list" ) )
   {
      CMDTYPE *tmp;
      int hash = command->name[0] % 126;

      pager_printf( ch, "Priority placement for [%s]:\r\n", command->name );
      for( tmp = command_hash[hash]; tmp; tmp = tmp->next )
      {
         if( tmp == command )
            set_pager_color( AT_GREEN, ch );
         else
            set_pager_color( AT_PLAIN, ch );
         pager_printf( ch, "  %s\r\n", tmp->name );
      }
      return;
   }

   if( !str_cmp( arg2, "delete" ) )
   {
      unlink_command( command );
      free_command( command );
      send_to_char( "Deleted.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "code" ) )
   {
      DO_FUN *fun = skill_function( argument );

      if( fun == skill_notfound )
      {
         send_to_char( "Code not found.\r\n", ch );
         return;
      }
      command->do_fun = fun;
      DISPOSE( command->fun_name );
      command->fun_name = str_dup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "level" ) )
   {
      int level = atoi( argument );

      if( ( level < 0 || level > get_trust( ch ) ) )
      {
         send_to_char( "Level out of range.\r\n", ch );
         return;
      }

      if( level > command->level && command->do_fun == do_switch )
      {
         command->level = level;
         check_switches( FALSE );
      }
      else
         command->level = level;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "log" ) )
   {
      int clog = atoi( argument );

      if( clog < 0 || clog > LOG_COMM )
      {
         send_to_char( "Log out of range.\r\n", ch );
         return;
      }
      command->log = clog;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "position" ) )
   {
      int position = atoi( argument );

      if( position < 0 || position > POS_DRAG )
      {
         send_to_char( "Position out of range.\r\n", ch );
         return;
      }
      command->position = position;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "flags" ) )
   {
      int flag;
      if( is_number( argument ) )
         flag = atoi( argument );
      else
         flag = get_cmdflag( argument );
      if( flag < 0 || flag >= 32 )
      {
         if( is_number( argument ) )
            ch_printf( ch, "Invalid flag: range is from 0 to 31.\n" );
         else
            ch_printf( ch, "Unknown flag %s.\n", argument );
         return;
      }

      TOGGLE_BIT( command->flags, 1 << flag );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      bool relocate;
      CMDTYPE *checkcmd;

      one_argument( argument, arg1 );
      if( arg1[0] == '\0' )
      {
         send_to_char( "Cannot clear name field!\r\n", ch );
         return;
      }
      if( ( checkcmd = find_command( arg1 ) ) != NULL )
      {
         ch_printf( ch, "There is already a command named %s.\r\n", arg1 );
         return;
      }
      if( arg1[0] != command->name[0] )
      {
         unlink_command( command );
         relocate = TRUE;
      }
      else
         relocate = FALSE;
      if( command->name )
         DISPOSE( command->name );
      command->name = str_dup( arg1 );
      if( relocate )
         add_command( command );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   /*
    * display usage message 
    */
   do_cedit( ch, "" );
}

/*
 * Display class information					-Thoric
 */
void do_showclass( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   struct class_type *Class;
   int cl, low, hi;

   set_pager_color( AT_PLAIN, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: showclass <class> [level range]\r\n", ch );
      return;
   }
   if( is_number( arg1 ) && ( cl = atoi( arg1 ) ) >= 0 && cl < MAX_CLASS )
      Class = class_table[cl];
   else
   {
      Class = NULL;
      for( cl = 0; cl < MAX_CLASS && class_table[cl]; cl++ )
         if( !str_cmp( class_table[cl]->who_name, arg1 ) )
         {
            Class = class_table[cl];
            break;
         }
   }
   if( !Class )
   {
      send_to_char( "No such class.\r\n", ch );
      return;
   }
   pager_printf_color( ch, "&wCLASS: &W%s\r\n&wPrime Attribute: &W%-14s  &wWeapon: &W%-5d      &wGuild: &W%-5d\r\n",
                       Class->who_name, affect_loc_name( Class->attr_prime ), Class->weapon, Class->guild );
   pager_printf_color( ch, "&wSecond Attribute:  &W%-14s  &wDeficient Attribute:  &W%-14s\r\n",
                       affect_loc_name( Class->attr_second ), affect_loc_name( Class->attr_deficient ) );
   pager_printf_color( ch, "&wMax Skill Adept: &W%-3d             &wThac0 : &W%-5d     &wThac32: &W%d\r\n",
                       Class->skill_adept, Class->thac0_00, Class->thac0_32 );
   pager_printf_color( ch, "&wHp Min/Hp Max  : &W%-2d/%-2d           &wMana  : &W%-3s      &wExpBase: &W%d\r\n",
                       Class->hp_min, Class->hp_max, Class->fMana ? "yes" : "no ", Class->exp_base );
   pager_printf_color( ch, "&wAffected by:  &W%s\r\n", affect_bit_name( &Class->affected ) );
   pager_printf_color( ch, "&wResistant to: &W%s\r\n", flag_string( Class->resist, ris_flags ) );
   pager_printf_color( ch, "&wSusceptible to: &W%s\r\n", flag_string( Class->suscept, ris_flags ) );
   if( arg2[0] != '\0' )
   {
      int x, y, cnt;

      low = UMAX( 0, atoi( arg2 ) );
      hi = URANGE( low, atoi( argument ), MAX_LEVEL );
      for( x = low; x <= hi; ++x )
      {
         set_pager_color( AT_LBLUE, ch );
         pager_printf( ch, "Male: %-30s Female: %s\r\n", title_table[cl][x][0], title_table[cl][x][1] );
         cnt = 0;
         set_pager_color( AT_BLUE, ch );
         for( y = 0; y < num_skills; ++y )
            if( skill_table[y]->skill_level[cl] == x )
            {
               pager_printf( ch, "  %-7s %-19s%3d     ",
                             skill_tname[skill_table[y]->type], skill_table[y]->name, skill_table[y]->skill_adept[cl] );
               if( ++cnt % 2 == 0 )
                  send_to_pager( "\r\n", ch );
            }
         if( cnt % 2 != 0 )
            send_to_pager( "\r\n", ch );
         send_to_pager( "\r\n", ch );
      }
   }
}

/*
 * Create a new class online.				    	-Shaddai
 */
bool create_new_class( int rcindex, const char *argument )
{
   int i;

   if( rcindex >= MAX_CLASS || class_table[rcindex] == NULL )
      return FALSE;
   if( class_table[rcindex]->who_name )
      STRFREE( class_table[rcindex]->who_name );

   char* tmp = strdup(argument);
   if( tmp[0] != '\0' )
      tmp[0] = UPPER( tmp[0] );

   class_table[rcindex]->who_name = STRALLOC( tmp );

   DISPOSE( tmp );

   xCLEAR_BITS( class_table[rcindex]->affected );
   class_table[rcindex]->attr_prime = 0;
   class_table[rcindex]->attr_second = 0;
   class_table[rcindex]->attr_deficient = 0;
   class_table[rcindex]->resist = 0;
   class_table[rcindex]->suscept = 0;
   class_table[rcindex]->weapon = 0;
   class_table[rcindex]->guild = 0;
   class_table[rcindex]->skill_adept = 0;
   class_table[rcindex]->thac0_00 = 0;
   class_table[rcindex]->thac0_32 = 0;
   class_table[rcindex]->hp_min = 0;
   class_table[rcindex]->hp_max = 0;
   class_table[rcindex]->fMana = FALSE;
   class_table[rcindex]->exp_base = 1000;
   for( i = 0; i < MAX_LEVEL; i++ )
   {
      if( title_table[rcindex][i][0] )
         DISPOSE( title_table[rcindex][i][0] );
      title_table[rcindex][i][0] = str_dup( "Not set." );
      if( title_table[rcindex][i][1] )
         DISPOSE( title_table[rcindex][i][1] );
      title_table[rcindex][i][1] = str_dup( "Not set." );
   }
   return TRUE;
}

/*
 * Edit class information					-Thoric
 */
void do_setclass( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
   FILE *fpList;
   char classlist[256];
   struct class_type *Class;
   int cl, value, i;

   set_char_color( AT_PLAIN, ch );

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: setclass <class> <field> <value>\r\n", ch );
      send_to_char( "Syntax: setclass <class> create\r\n", ch );
      send_to_char( "\r\nField being one of:\r\n", ch );
      send_to_char( "  name prime weapon guild thac0 thac32\r\n", ch );
      send_to_char( "  hpmin hpmax mana expbase mtitle ftitle\r\n", ch );
      send_to_char( "  second, deficient affected resist suscept skill\r\n", ch );
      return;
   }
   if( is_number( arg1 ) && ( cl = atoi( arg1 ) ) >= 0 && cl < MAX_CLASS )
      Class = class_table[cl];
   else
   {
      Class = NULL;
      for( cl = 0; cl < MAX_CLASS && class_table[cl]; cl++ )
      {
         if( !class_table[cl]->who_name )
            continue;
         if( !str_cmp( class_table[cl]->who_name, arg1 ) )
         {
            Class = class_table[cl];
            break;
         }
      }
   }
   if( !str_cmp( arg2, "create" ) && Class )
   {
      send_to_char( "That class already exists!\r\n", ch );
      return;
   }

   if( !Class && str_cmp( arg2, "create" ) )
   {
      send_to_char( "No such class.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "create" ) )
   {
      char filename[256];

      if( MAX_PC_CLASS >= MAX_CLASS )
      {
         send_to_char( "You need to up MAX_CLASS in mud and make clean.\r\n", ch );
         return;
      }

      snprintf( filename, sizeof( filename ), "%s.class", arg1 );
      if( !is_valid_filename( ch, CLASS_DIR, filename ) )
         return;

      if( !( create_new_class( MAX_PC_CLASS, arg1 ) ) )
      {
         send_to_char( "Couldn't create a new class.\r\n", ch );
         return;
      }
      write_class_file( MAX_PC_CLASS );
      MAX_PC_CLASS++;

      snprintf( classlist, 256, "%s%s", CLASS_DIR, CLASS_LIST );
      if( !( fpList = fopen( classlist, "w" ) ) )
      {
         bug( "%s", "Can't open class list for writing." );
         return;
      }

      for( i = 0; i < MAX_PC_CLASS; ++i )
         fprintf( fpList, "%s.class\n", class_table[i]->who_name );

      fprintf( fpList, "%s", "$\n" );
      fclose( fpList );
      fpList = NULL;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !argument )
   {
      send_to_char( "You must specify an argument.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "skill" ) )
   {
      SKILLTYPE *skill;
      int sn, level, adept;

      argument = one_argument( argument, arg2 );
      if( ( sn = skill_lookup( arg2 ) ) > 0 )
      {
         skill = get_skilltype( sn );
         argument = one_argument( argument, arg2 );
         level = atoi( arg2 );
         argument = one_argument( argument, arg2 );
         adept = atoi( arg2 );
         skill->skill_level[cl] = level;
         skill->skill_adept[cl] = adept;
         write_class_file( cl );
         ch_printf( ch, "Skill \"%s\" added at level %d and %d%%.\r\n", skill->name, level, adept );
      }
      else
         ch_printf( ch, "No such skill as %s.\r\n", arg2 );
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      char buf[256];
      struct class_type *ccheck = NULL;

      one_argument( argument, arg1 );
      if( arg1[0] == '\0' )
      {
         send_to_char( "You can't set a class name to nothing.\r\n", ch );
         return;
      }

      snprintf( buf, sizeof( buf ), "%s.class", arg1 );
      if( !is_valid_filename( ch, CLASS_DIR, buf ) )
         return;

      for( i = 0; i < MAX_PC_CLASS && class_table[i]; ++i )
      {
         if( !class_table[i]->who_name )
            continue;

         if( !str_cmp( class_table[i]->who_name, arg1 ) )
         {
            ccheck = class_table[i];
            break;
         }
      }
      if( ccheck != NULL )
      {
         ch_printf( ch, "Already a class called %s.\r\n", arg1 );
         return;
      }

      snprintf( buf, sizeof( buf ), "%s%s.class", CLASS_DIR, Class->who_name );
      unlink( buf );
      if( Class->who_name )
         STRFREE( Class->who_name );
      Class->who_name = STRALLOC( capitalize( argument ) );
      ch_printf( ch, "class renamed to %s.\r\n", arg1 );
      write_class_file( cl );

      snprintf( classlist, 256, "%s%s", CLASS_DIR, CLASS_LIST );
      if( !( fpList = fopen( classlist, "w" ) ) )
      {
         bug( "%s", "Can't open class list for writing." );
         return;
      }

      for( i = 0; i < MAX_PC_CLASS; ++i )
         fprintf( fpList, "%s%s.class\n", CLASS_DIR, class_table[i]->who_name );

      fprintf( fpList, "%s", "$\n" );
      fclose( fpList );
      fpList = NULL;
      return;
   }

   if( !str_cmp( arg2, "affected" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: setclass <class> affected <flag> [flag]...\r\n", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg2 );
         value = get_aflag( arg2 );
         if( value < 0 || value >= MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\r\n", arg2 );
         else
            xTOGGLE_BIT( Class->affected, value );
      }
      send_to_char( "Done.\r\n", ch );
      write_class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "resist" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: setclass <class> resist <flag> [flag]...\r\n", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg2 );
         value = get_risflag( arg2 );
         if( value < 0 || value > 31 )
            ch_printf( ch, "Unknown flag: %s\r\n", arg2 );
         else
            TOGGLE_BIT( Class->resist, 1 << value );
      }
      send_to_char( "Done.\r\n", ch );
      write_class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "suscept" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: setclass <class> suscept <flag> [flag]...\r\n", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg2 );
         value = get_risflag( arg2 );
         if( value < 0 || value > 31 )
            ch_printf( ch, "Unknown flag: %s\r\n", arg2 );
         else
            TOGGLE_BIT( Class->suscept, 1 << value );
      }
      send_to_char( "Done.\r\n", ch );
      write_class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "second" ) )
   {
      int x = get_atype( argument );

      if( x < APPLY_NONE || ( x > APPLY_CON && x != APPLY_LCK && x != APPLY_CHA ) )
         send_to_char( "Invalid second attribute!\r\n", ch );
      else
      {
         Class->attr_second = x;
         send_to_char( "Second attribute set.\r\n", ch );
         write_class_file( cl );
      }
      return;
   }

   if( !str_cmp( arg2, "deficient" ) )
   {
      int x = get_atype( argument );

      if( x < APPLY_NONE || ( x > APPLY_CON && x != APPLY_LCK && x != APPLY_CHA ) )
         send_to_char( "Invalid deficient attribute!\r\n", ch );
      else
      {
         Class->attr_deficient = x;
         send_to_char( "Deficient attribute set.\r\n", ch );
         write_class_file( cl );
      }
      return;
   }

   if( !str_cmp( arg2, "prime" ) )
   {
      int x = get_atype( argument );

      if( x < APPLY_NONE || ( x > APPLY_CON && x != APPLY_LCK && x != APPLY_CHA ) )
         send_to_char( "Invalid prime attribute!\r\n", ch );
      else
      {
         Class->attr_prime = x;
         send_to_char( "Prime attribute set.\r\n", ch );
         write_class_file( cl );
      }
      return;
   }

   if( !str_cmp( arg2, "weapon" ) )
   {
      Class->weapon = atoi( argument );
      send_to_char( "Starting weapon set.\r\n", ch );
      write_class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "guild" ) )
   {
      Class->guild = atoi( argument );
      send_to_char( "Guild set.\r\n", ch );
      write_class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "thac0" ) )
   {
      Class->thac0_00 = atoi( argument );
      send_to_char( "thac0 set.\r\n", ch );
      write_class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "thac32" ) )
   {
      Class->thac0_32 = atoi( argument );
      send_to_char( "thac32 set.\r\n", ch );
      write_class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "hpmin" ) )
   {
      Class->hp_min = atoi( argument );
      send_to_char( "Min HP gain set.\r\n", ch );
      write_class_file( cl );
      return;
   }
   if( !str_cmp( arg2, "hpmax" ) )
   {
      Class->hp_max = atoi( argument );
      send_to_char( "Max HP gain set.\r\n", ch );
      write_class_file( cl );
      return;
   }
   if( !str_cmp( arg2, "mana" ) )
   {
      if( UPPER( argument[0] ) == 'Y' )
         Class->fMana = TRUE;
      else
         Class->fMana = FALSE;
      send_to_char( "Mana flag toggled.\r\n", ch );
      write_class_file( cl );
      return;
   }
   if( !str_cmp( arg2, "expbase" ) )
   {
      Class->exp_base = atoi( argument );
      send_to_char( "Base EXP set.\r\n", ch );
      write_class_file( cl );
      return;
   }
   if( !str_cmp( arg2, "mtitle" ) )
   {
      char arg3[MAX_INPUT_LENGTH];
      int x;

      argument = one_argument( argument, arg3 );
      if( arg3[0] == '\0' || argument[0] == '\0' )
      {
         send_to_char( "Syntax: setclass <class> mtitle <level> <title>\r\n", ch );
         return;
      }
      if( ( x = atoi( arg3 ) ) < 0 || x > MAX_LEVEL )
      {
         send_to_char( "Invalid level.\r\n", ch );
         return;
      }
      STRFREE( title_table[cl][x][SEX_MALE] );
      title_table[cl][x][SEX_MALE] = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      write_class_file( cl );
      return;
   }
   if( !str_cmp( arg2, "ftitle" ) )
   {
      char arg3[MAX_INPUT_LENGTH], arg4[MAX_INPUT_LENGTH];
      int x;

      argument = one_argument( argument, arg3 );
      argument = one_argument( argument, arg4 );
      if( arg3[0] == '\0' || argument[0] == '\0' )
      {
         send_to_char( "Syntax: setclass <class> ftitle <level> <title>\r\n", ch );
         return;
      }
      if( ( x = atoi( arg4 ) ) < 0 || x > MAX_LEVEL )
      {
         send_to_char( "Invalid level.\r\n", ch );
         return;
      }
      STRFREE( title_table[cl][x][SEX_FEMALE] );
      /*
       * Bug fix below -Shaddai
       */
      title_table[cl][x][SEX_FEMALE] = STRALLOC( argument );
      send_to_char( "Done\r\n", ch );
      write_class_file( cl );
      return;
   }
   do_setclass( ch, "" );
}

/*
 * Create an instance of a new race.			-Shaddai
 */

bool create_new_race( int rcindex, char *argument )
{
   int i = 0;
   if( rcindex >= MAX_RACE || race_table[rcindex] == NULL )
      return FALSE;
   for( i = 0; i < MAX_WHERE_NAME; i++ )
      race_table[rcindex]->where_name[i] = str_dup( where_name[i] );
   if( argument[0] != '\0' )
      argument[0] = UPPER( argument[0] );

   snprintf( race_table[rcindex]->race_name, 16, "%-.16s", argument );
   race_table[rcindex]->class_restriction = 0;
   race_table[rcindex]->str_plus = 0;
   race_table[rcindex]->dex_plus = 0;
   race_table[rcindex]->wis_plus = 0;
   race_table[rcindex]->int_plus = 0;
   race_table[rcindex]->con_plus = 0;
   race_table[rcindex]->cha_plus = 0;
   race_table[rcindex]->lck_plus = 0;
   race_table[rcindex]->hit = 0;
   race_table[rcindex]->mana = 0;
   xCLEAR_BITS( race_table[rcindex]->affected );
   race_table[rcindex]->resist = 0;
   race_table[rcindex]->suscept = 0;
   race_table[rcindex]->language = 0;
   race_table[rcindex]->alignment = 0;
   race_table[rcindex]->minalign = 0;
   race_table[rcindex]->maxalign = 0;
   race_table[rcindex]->ac_plus = 0;
   race_table[rcindex]->exp_multiplier = 100;
   xCLEAR_BITS( race_table[rcindex]->attacks );
   xCLEAR_BITS( race_table[rcindex]->defenses );
   race_table[rcindex]->height = 0;
   race_table[rcindex]->weight = 0;
   race_table[rcindex]->hunger_mod = 0;
   race_table[rcindex]->thirst_mod = 0;
   race_table[rcindex]->mana_regen = 0;
   race_table[rcindex]->hp_regen = 0;
   race_table[rcindex]->race_recall = 0;
   return TRUE;
}

/* Modified by Samson to allow setting language by name - 8-6-98 */
void do_setrace( CHAR_DATA* ch, const char* argument)
{
   RACE_TYPE *race;
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
   FILE *fpList = NULL;
   char racelist[256];
   int value, v2, ra, i;

   set_char_color( AT_PLAIN, ch );

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Syntax: setrace <race> <field> <value>\r\n", ch );
      send_to_char( "Syntax: setrace <race> create	     \r\n", ch );
      send_to_char( "\r\nField being one of:\r\n", ch );
      send_to_char( "  name classes strplus dexplus wisplus\r\n", ch );
      send_to_char( "  intplus conplus chaplus lckplus hit\r\n", ch );
      send_to_char( "  mana affected resist suscept language\r\n", ch );
      send_to_char( "  attack defense alignment acplus \r\n", ch );
      send_to_char( "  minalign maxalign height weight      \r\n", ch );
      send_to_char( "  hungermod thirstmod expmultiplier    \r\n", ch );
      send_to_char( "  saving_poison_death saving_wand      \r\n", ch );
      send_to_char( "  saving_para_petri saving_breath      \r\n", ch );
      send_to_char( "  saving_spell_staff race_recall       \r\n", ch );
      send_to_char( "  mana_regen hp_regen                  \r\n", ch );
      return;
   }
   if( is_number( arg1 ) && ( ra = atoi( arg1 ) ) >= 0 && ra < MAX_RACE )
      race = race_table[ra];
   else
   {
      race = NULL;
      for( ra = 0; ra < MAX_RACE && race_table[ra]; ra++ )
      {
         if( !race_table[ra]->race_name )
            continue;

         if( !str_cmp( race_table[ra]->race_name, arg1 ) )
         {
            race = race_table[ra];
            break;
         }
      }
   }

   if( !str_cmp( arg2, "create" ) && race )
   {
      send_to_char( "That race already exists!\r\n", ch );
      return;
   }
   else if( !race && str_cmp( arg2, "create" ) )
   {
      send_to_char( "No such race.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "create" ) )
   {
      if( MAX_PC_RACE >= MAX_RACE )
      {
         send_to_char( "You need to up MAX_RACE in mud.h and make clean.\r\n", ch );
         return;
      }
      if( ( create_new_race( MAX_PC_RACE, arg1 ) ) == FALSE )
      {
         send_to_char( "Couldn't create a new race.\r\n", ch );
         return;
      }
      write_race_file( MAX_PC_RACE );
      MAX_PC_RACE++;
      snprintf( racelist, 256, "%s%s", RACE_DIR, RACE_LIST );
      if( !( fpList = fopen( racelist, "w" ) ) )
      {
         bug( "%s", "Error opening racelist." );
         return;
      }
      for( i = 0; i < MAX_PC_RACE; ++i )
         fprintf( fpList, "%s.race\n", race_table[i]->race_name );
      fprintf( fpList, "%s", "$\n" );
      fclose( fpList );
      fpList = NULL;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !argument )
   {
      send_to_char( "You must specify an argument.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      char buf[256];
      RACE_TYPE *rcheck = NULL;

      one_argument( argument, arg1 );

      if( arg1[0] == '\0' )
      {
         send_to_char( "You can't set a race name to nothing.\r\n", ch );
         return;
      }

      snprintf( buf, sizeof( buf ), "%s.race", arg1 );
      if( !is_valid_filename( ch, RACE_DIR, buf ) )
         return;

      for( i = 0; i < MAX_PC_RACE && race_table[i]; ++i )
      {
         if( !race_table[i]->race_name )
            continue;

         if( !str_cmp( race_table[i]->race_name, arg1 ) )
         {
            rcheck = race_table[i];
            break;
         }
      }
      if( rcheck != NULL )
      {
         ch_printf( ch, "Already a race called %s.\r\n", arg1 );
         return;
      }

      snprintf( buf, sizeof( buf ), "%s%s.race", RACE_DIR, race->race_name );
      unlink( buf );

      snprintf( race->race_name, 16, "%s", capitalize( argument ) );
      write_race_file( ra );
      for( i = 0; i < MAX_PC_RACE; ++i )
         fprintf( fpList, "%s.race\n", race_table[i]->race_name );
      fprintf( fpList, "%s", "$\n" );
      fclose( fpList );
      fpList = NULL;
      send_to_char( "Race name set.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "strplus" ) )
   {
      race->str_plus = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "dexplus" ) )
   {
      race->dex_plus = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "wisplus" ) )
   {
      race->wis_plus = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "intplus" ) )
   {
      race->int_plus = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "conplus" ) )
   {
      race->con_plus = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "chaplus" ) )
   {
      race->cha_plus = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "lckplus" ) )
   {
      race->lck_plus = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "hit" ) )
   {
      race->hit = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "mana" ) )
   {
      race->mana = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "affected" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: setrace <race> affected <flag> [flag]...\r\n", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_aflag( arg3 );
         if( value < 0 || value >= MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
         else
            xTOGGLE_BIT( race->affected, value );
      }
      write_race_file( ra );
      send_to_char( "Racial affects set.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "resist" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: setrace <race> resist <flag> [flag]...\r\n", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_risflag( arg3 );
         if( value < 0 || value > 31 )
            ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
         else
            TOGGLE_BIT( race->resist, 1 << value );
      }
      write_race_file( ra );
      send_to_char( "Racial resistances set.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "suscept" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: setrace <race> suscept <flag> [flag]...\r\n", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_risflag( arg3 );
         if( value < 0 || value > 31 )
            ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
         else
            TOGGLE_BIT( race->suscept, 1 << value );
      }
      write_race_file( ra );
      send_to_char( "Racial susceptabilities set.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "language" ) )
   {
      argument = one_argument( argument, arg3 );
      value = get_langflag( arg3 );
      if( value == LANG_UNKNOWN )
         ch_printf( ch, "Unknown language: %s\r\n", arg3 );
      else
      {
         if( !( value &= VALID_LANGS ) )
            ch_printf( ch, "Player races may not speak %s.\r\n", arg3 );
      }

      v2 = get_langnum( arg3 );
      if( v2 == -1 )
         ch_printf( ch, "Unknown language: %s\r\n", arg3 );
      else
         TOGGLE_BIT( race->language, 1 << v2 );

      write_race_file( ra );
      send_to_char( "Racial language set.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "classes" ) )
   {
      for( i = 0; i < MAX_CLASS; ++i )
      {
         if( !str_cmp( argument, class_table[i]->who_name ) )
         {
            TOGGLE_BIT( race->class_restriction, 1 << i );  /* k, that's boggling */
            write_race_file( ra );
            send_to_char( "classes set.\r\n", ch );
            return;
         }
      }
      send_to_char( "No such class.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "acplus" ) )
   {
      race->ac_plus = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "alignment" ) )
   {
      race->alignment = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   /*
    * not implemented 
    */
   if( !str_cmp( arg2, "defense" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: setrace <race> defense <flag> [flag]...\r\n", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_defenseflag( arg3 );
         if( value < 0 || value >= MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
         else
            xTOGGLE_BIT( race->defenses, value );
      }
      write_race_file( ra );
      return;
   }

   /*
    * not implemented 
    */
   if( !str_cmp( arg2, "attack" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: setrace <race> attack <flag> [flag]...\r\n", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_attackflag( arg3 );
         if( value < 0 || value >= MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
         else
            xTOGGLE_BIT( race->attacks, value );
      }
      write_race_file( ra );
      return;
   }

   if( !str_cmp( arg2, "minalign" ) )
   {
      race->minalign = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "maxalign" ) )
   {
      race->maxalign = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "height" ) )
   {
      race->height = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "weight" ) )
   {
      race->weight = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "thirstmod" ) )
   {
      race->thirst_mod = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "hungermod" ) )
   {
      race->hunger_mod = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "expmultiplier" ) )
   {
      race->exp_multiplier = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "saving_poison_death" ) )
   {
      race->saving_poison_death = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "saving_wand" ) )
   {
      race->saving_wand = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "saving_para_petri" ) )
   {
      race->saving_para_petri = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "saving_breath" ) )
   {
      race->saving_breath = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "saving_spell_staff" ) )
   {
      race->saving_spell_staff = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   /*
    * unimplemented stuff follows 
    */
   if( !str_cmp( arg2, "mana_regen" ) )
   {
      race->mana_regen = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "hp_regen" ) )
   {
      race->hp_regen = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "race_recall" ) )
   {
      race->race_recall = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
#ifdef NEW_RACE_STUFF
   if( !str_cmp( arg2, "carry_weight" ) )
   {
      race->acplus = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg2, "carry_number" ) )
   {
      race->acplus = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
#endif
   do_setrace( ch, "" );
}

void do_showrace( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   struct race_type *race;
   int ra, i, ct;

   set_pager_color( AT_PLAIN, ch );

   argument = one_argument( argument, arg1 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: showrace  \r\n", ch );
      /*
       * Show the races code addition by Blackmane 
       */
      /*
       * fixed printout by Miki 
       */
      ct = 0;
      for( i = 0; i < MAX_RACE; i++ )
      {
         ++ct;
         pager_printf( ch, "%2d> %-11s", i, race_table[i]->race_name );
         if( ct % 5 == 0 )
            send_to_pager( "\r\n", ch );
      }

      send_to_pager( "\r\n", ch );
      return;
   }

   if( is_number( arg1 ) && ( ra = atoi( arg1 ) ) >= 0 && ra < MAX_RACE )
      race = race_table[ra];
   else
   {
      race = NULL;
      for( ra = 0; ra < MAX_RACE && race_table[ra]; ra++ )
         if( !str_cmp( race_table[ra]->race_name, arg1 ) )
         {
            race = race_table[ra];
            break;
         }
   }
   if( !race )
   {
      send_to_char( "No such race.\r\n", ch );
      return;
   }

   ch_printf( ch, "RACE: %s\r\n", race->race_name );
   ct = 0;
   send_to_char( "Disallowed classes: ", ch );
   for( i = 0; i < MAX_CLASS; i++ )
   {
      if( IS_SET( race->class_restriction, 1 << i ) )
      {
         ct++;
         ch_printf( ch, "%s ", class_table[i]->who_name );
         if( ct % 6 == 0 )
            send_to_char( "\r\n", ch );
      }
   }
   if( ( ct % 6 != 0 ) || ( ct == 0 ) )
      send_to_char( "\r\n", ch );

   ct = 0;
   send_to_char( "Allowed classes: ", ch );
   for( i = 0; i < MAX_CLASS; i++ )
   {
      if( !IS_SET( race->class_restriction, 1 << i ) )
      {
         ct++;
         ch_printf( ch, "%s ", class_table[i]->who_name );
         if( ct % 6 == 0 )
            send_to_char( "\r\n", ch );
      }
   }
   if( ( ct % 6 != 0 ) || ( ct == 0 ) )
      send_to_char( "\r\n", ch );

   ch_printf( ch, "Str Plus: %-3d\tDex Plus: %-3d\tWis Plus: %-3d\tInt Plus: %-3d\t\r\n",
              race->str_plus, race->dex_plus, race->wis_plus, race->int_plus );
   ch_printf( ch, "Con Plus: %-3d\tCha Plus: %-3d\tLck Plus: %-3d\r\n", race->con_plus, race->cha_plus, race->lck_plus );
   ch_printf( ch, "Hit Pts:  %-3d\tMana: %-3d\tAlign: %-4d\tAC: %-d\r\n",
              race->hit, race->mana, race->alignment, race->ac_plus );
   ch_printf( ch, "Min Align: %d\tMax Align: %-d\t\tXP Mult: %-d%%\r\n",
              race->minalign, race->maxalign, race->exp_multiplier );
   ch_printf( ch, "Height: %3d in.\t\tWeight: %4d lbs.\tHungerMod: %d\tThirstMod: %d\r\n",
              race->height, race->weight, race->hunger_mod, race->thirst_mod );

   ch_printf( ch, "Spoken Languages: %s", flag_string( race->language, lang_names ) );
   send_to_char( "\r\n", ch );

   send_to_char( "Affected by: ", ch );
   send_to_char( affect_bit_name( &race->affected ), ch );
   send_to_char( "\r\n", ch );

   send_to_char( "Resistant to: ", ch );
   send_to_char( flag_string( race->resist, ris_flags ), ch );
   send_to_char( "\r\n", ch );

   send_to_char( "Susceptible to: ", ch );
   send_to_char( flag_string( race->suscept, ris_flags ), ch );
   send_to_char( "\r\n", ch );

   ch_printf( ch, "Saves: (P/D) %d (W) %d (P/P) %d (B) %d (S/S) %d\r\n",
              race->saving_poison_death,
              race->saving_wand, race->saving_para_petri, race->saving_breath, race->saving_spell_staff );

   send_to_char( "Innate Attacks: ", ch );
   send_to_char( ext_flag_string( &race->attacks, attack_flags ), ch );
   send_to_char( "\r\n", ch );

   send_to_char( "Innate Defenses: ", ch );
   send_to_char( ext_flag_string( &race->defenses, defense_flags ), ch );
   send_to_char( "\r\n", ch );
}

/*
 * quest point set - TRI
 * syntax is: qpset char give/take amount
 */

void do_qpset( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   int amount;
   bool give = TRUE;

   set_char_color( AT_IMMORT, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Cannot qpset as an NPC.\r\n", ch );
      return;
   }
   if( get_trust( ch ) < LEVEL_IMMORTAL )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   amount = atoi( arg3 );
   if( arg[0] == '\0' || arg2[0] == '\0' || amount <= 0 )
   {
      send_to_char( "Syntax: qpset <character> <give/take> <amount>\r\n", ch );
      send_to_char( "Amount must be a positive number greater than 0.\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "There is no such player currently playing.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Glory cannot be given to or taken from a mob.\r\n", ch );
      return;
   }

   set_char_color( AT_IMMORT, victim );
   if( nifty_is_name_prefix( arg2, "give" ) )
   {
      give = TRUE;
      if( str_cmp( ch->pcdata->council_name, "Quest Council" ) && ( get_trust( ch ) < LEVEL_DEMI ) )
      {
         send_to_char( "You must be a member of the Quest Council to give qp to a character.\r\n", ch );
         return;
      }
   }
   else if( nifty_is_name_prefix( arg2, "take" ) )
      give = FALSE;
   else
   {
      do_qpset( ch, "" );
      return;
   }

   if( give )
   {
      victim->pcdata->quest_curr += amount;
      victim->pcdata->quest_accum += amount;
      ch_printf( victim, "Your glory has been increased by %d.\r\n", amount );
      ch_printf( ch, "You have increased the glory of %s by %d.\r\n", victim->name, amount );
   }
   else
   {
      if( victim->pcdata->quest_curr - amount < 0 )
      {
         ch_printf( ch, "%s does not have %d glory to take.\r\n", victim->name, amount );
         return;
      }
      else
      {
         victim->pcdata->quest_curr -= amount;
         ch_printf( victim, "Your glory has been decreased by %d.\r\n", amount );
         ch_printf( ch, "You have decreased the glory of %s by %d.\r\n", victim->name, amount );
      }
   }
   return;
}

/* Easy way to check a player's glory -- Blodkai, June 97 */
void do_qpstat( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   if( IS_NPC( ch ) )
      return;

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax:  qpstat <character>\r\n", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "No one by that name currently in the Realms.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Mobs don't have glory.\r\n", ch );
      return;
   }
   ch_printf( ch, "%s has %d glory, out of a lifetime total of %d.\r\n",
              victim->name, victim->pcdata->quest_curr, victim->pcdata->quest_accum );
   return;
}

/* Simple, small way to make keeping track of small mods easier - Blod */
void do_fixed( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   struct tm *t = localtime( &current_time );

   set_char_color( AT_OBJECT, ch );
   if( argument[0] == '\0' )
   {
      send_to_char( "\r\nUsage:  'fixed list' or 'fixed <message>'", ch );
      if( get_trust( ch ) >= LEVEL_ASCENDANT )
         send_to_char( " or 'fixed clear now'\r\n", ch );
      else
         send_to_char( "\r\n", ch );
      return;
   }
   if( !str_cmp( argument, "clear now" ) && get_trust( ch ) >= LEVEL_ASCENDANT )
   {
      FILE *fp = fopen( FIXED_FILE, "w" );
      if( fp )
         fclose( fp );
      send_to_char( "Fixed file cleared.\r\n", ch );
      return;
   }
   if( !str_cmp( argument, "list" ) )
   {
      send_to_char_color( "\r\n&g[&GDate  &g|  &GVnum&g]\r\n", ch );
      show_file( ch, FIXED_FILE );
   }
   else
   {
      snprintf( buf, MAX_STRING_LENGTH, "&g|&G%-2.2d/%-2.2d &g| &G%5d&g|  %s:  &G%s",
                t->tm_mon + 1, t->tm_mday, ch->in_room ? ch->in_room->vnum : 0,
                IS_NPC( ch ) ? ch->short_descr : ch->name, argument );
      append_to_file( FIXED_FILE, buf );
      send_to_char( "Thanks, your modification has been logged.\r\n", ch );
   }
   return;
}

void do_fshow( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax:  fshow <moblog | plevel>\r\n", ch );
      return;
   }
   if( !str_cmp( arg, "moblog" ) )
   {
      set_char_color( AT_LOG, ch );
      send_to_char( "\r\n[Date_|_Time]  Current moblog:\r\n", ch );
      show_file( ch, MOBLOG_FILE );
      return;
   }
   send_to_char( "No such file.\r\n", ch );
   return;
}

RESERVE_DATA *first_reserved;
RESERVE_DATA *last_reserved;

void free_reserve( RESERVE_DATA * res )
{
   UNLINK( res, first_reserved, last_reserved, next, prev );
   DISPOSE( res->name );
   DISPOSE( res );
}

void free_all_reserved( void )
{
   RESERVE_DATA *res, *res_next;

   for( res = first_reserved; res; res = res_next )
   {
      res_next = res->next;

      free_reserve( res );
   }
}

void save_reserved( void )
{
   RESERVE_DATA *res;
   FILE *fp;

   if( !( fp = fopen( SYSTEM_DIR RESERVED_LIST, "w" ) ) )
   {
      bug( "%s: cannot open %s", __FUNCTION__, RESERVED_LIST );
      perror( RESERVED_LIST );
      return;
   }
   for( res = first_reserved; res; res = res->next )
      fprintf( fp, "%s~\n", res->name );
   fprintf( fp, "$~\n" );
   fclose( fp );
   fp = NULL;
   return;
}

void do_reserve( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   RESERVE_DATA *res;

   set_char_color( AT_PLAIN, ch );

   argument = one_argument( argument, arg );
   if( !*arg )
   {
      int wid = 0;

      send_to_char( "-- Reserved Names --\r\n", ch );
      for( res = first_reserved; res; res = res->next )
      {
         ch_printf( ch, "%c%-17s ", ( *res->name == '*' ? '*' : ' ' ), ( *res->name == '*' ? res->name + 1 : res->name ) );
         if( ++wid % 4 == 0 )
            send_to_char( "\r\n", ch );
      }
      if( wid % 4 != 0 )
         send_to_char( "\r\n", ch );
      return;
   }
   for( res = first_reserved; res; res = res->next )
   {
      if( !str_cmp( arg, res->name ) )
      {
         free_reserve( res );
         save_reserved(  );
         send_to_char( "Name no longer reserved.\r\n", ch );
         return;
      }
   }
   CREATE( res, RESERVE_DATA, 1 );
   res->name = str_dup( arg );
   sort_reserved( res );
   save_reserved(  );
   send_to_char( "Name reserved.\r\n", ch );
   return;
}

void do_khistory( CHAR_DATA* ch, const char* argument)
{
   MOB_INDEX_DATA *tmob;
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *vch;
   int track;

   if( IS_NPC( ch ) || !IS_IMMORTAL( ch ) )
   {
      ch_printf( ch, "Huh?\r\n" );
      return;
   }

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      ch_printf( ch, "syntax: khistory <player>\r\n" );
      return;
   }

   vch = get_char_world( ch, arg );

   if( !vch || IS_NPC( vch ) )
   {
      ch_printf( ch, "They are not here.\r\n" );
      return;
   }

   set_char_color( AT_BLOOD, ch );
   ch_printf( ch, "Kill history for %s:\r\n", vch->name );

   for( track = 0; track < MAX_KILLTRACK && vch->pcdata->killed[track].vnum; track++ )
   {
      tmob = get_mob_index( vch->pcdata->killed[track].vnum );

      if( !tmob )
      {
         bug( "killhistory: unknown mob vnum" );
         continue;
      }

      set_char_color( AT_RED, ch );
      ch_printf( ch, "   %-30s", capitalize( tmob->short_descr ) );
      set_char_color( AT_BLOOD, ch );
      ch_printf( ch, "(" );
      set_char_color( AT_RED, ch );
      ch_printf( ch, "%-5d", tmob->vnum );
      set_char_color( AT_BLOOD, ch );
      ch_printf( ch, ")" );
      set_char_color( AT_RED, ch );
      ch_printf( ch, "    - killed %d times.\r\n", vch->pcdata->killed[track].count );
   }

   return;
}

void do_project( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int pcount;
   int pnum;
   PROJECT_DATA *pproject;

   if( IS_NPC( ch ) )
      return;

   if( !ch->desc )
   {
      bug( "%s", "do_project: no descriptor" );
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_WRITING_NOTE:
         if( !ch->pnote )
         {
            bug( "%s", "do_project: log got lost?" );
            send_to_char( "Your log was lost!\r\n", ch );
            stop_editing( ch );
            return;
         }
         if( ch->dest_buf != ch->pnote )
            bug( "%s", "do_project: sub_writing_note: ch->dest_buf != ch->pnote" );
         STRFREE( ch->pnote->text );
         ch->pnote->text = copy_buffer( ch );
         stop_editing( ch );
         return;
      case SUB_PROJ_DESC:
         if( !ch->dest_buf )
         {
            send_to_char( "Your description was lost!", ch );
            bug( "%s", "do_project: sub_project_desc: NULL ch->dest_buf" );
            ch->substate = SUB_NONE;
            return;
         }
         pproject = ( PROJECT_DATA * ) ch->dest_buf;
         STRFREE( pproject->description );
         pproject->description = copy_buffer( ch );
         stop_editing( ch );
         ch->substate = ch->tempnum;
         write_projects(  );
         return;
   }

   set_char_color( AT_NOTE, ch );
   argument = one_argument( argument, arg );
   smash_tilde( argument );

   if( !str_cmp( arg, "save" ) )
   {
      write_projects(  );
      ch_printf( ch, "Projects saved.\r\n" );
      return;
   }

   if( !str_cmp( arg, "code" ) )
   {
      pcount = 0;
      pager_printf( ch, " # | Owner       | Project              |\r\n" );
      pager_printf( ch, "---|-------------|----------------------|--------------------------|-----------\r\n" );
      for( pproject = first_project; pproject; pproject = pproject->next )
      {
         pcount++;
         if( ( pproject->status && str_cmp( pproject->status, "approved" ) ) || pproject->coder != NULL )
            continue;
         pager_printf( ch, "%2d | %-11s | %-20s |\r\n",
                       pcount, pproject->owner ? pproject->owner : "(None)", pproject->name );
      }
      return;
   }
   if( !str_cmp( arg, "more" ) || !str_cmp( arg, "mine" ) )
   {
      NOTE_DATA *nlog;
      bool MINE = FALSE;
      int num_logs = 0;
      pcount = 0;

      if( !str_cmp( arg, "mine" ) )
         MINE = TRUE;

      pager_printf( ch, "\r\n" );
      pager_printf( ch, " # | Owner       | Project              | Coder         | Status     | # of Logs\r\n" );
      pager_printf( ch, "---|-------------|----------------------|---------------|------------|----------\r\n" );
      for( pproject = first_project; pproject; pproject = pproject->next )
      {
         pcount++;
         if( MINE && ( !pproject->owner || str_cmp( ch->name, pproject->owner ) )
             && ( !pproject->coder || str_cmp( ch->name, pproject->coder ) ) )
            continue;
         else if( !MINE && pproject->status && !str_cmp( "Done", pproject->status ) )
            continue;
         num_logs = 0;
         for( nlog = pproject->first_log; nlog; nlog = nlog->next )
            num_logs++;
         pager_printf( ch, "%2d | %-11s | %-20s | %-13s | %-10s | %3d\r\n",
                       pcount,
                       pproject->owner ? pproject->owner : "(None)",
                       pproject->name,
                       pproject->coder ? pproject->coder : "(None)",
                       pproject->status ? pproject->status : "(None)", num_logs );
      }
      return;
   }
   if( arg[0] == '\0' || !str_cmp( arg, "list" ) )
   {
      bool aflag, projects_available;
      aflag = FALSE;
      projects_available = FALSE;
      if( !str_cmp( argument, "available" ) )
         aflag = TRUE;

      pager_printf( ch, "\r\n" );
      if( !aflag )
      {
         pager_printf( ch, " # | Owner       | Project              | Date                     | Status\r\n" );
         pager_printf( ch, "---|-------------|----------------------|--------------------------|-----------\r\n" );
      }
      else
      {
         pager_printf( ch, " # | Project              | Date\r\n" );
         pager_printf( ch, "---|----------------------|--------------------------\r\n" );
      }
      pcount = 0;
      for( pproject = first_project; pproject; pproject = pproject->next )
      {
         pcount++;
         if( pproject->status && !str_cmp( "Done", pproject->status ) )
            continue;
         if( !aflag )
            pager_printf( ch, "%2d | %-11s | %-20s | %-24s | %-10s\r\n",
                          pcount,
                          pproject->owner ? pproject->owner : "(None)",
                          pproject->name, pproject->date, pproject->status ? pproject->status : "(None)" );
         else if( !pproject->taken )
         {
            if( !projects_available )
               projects_available = TRUE;
            pager_printf( ch, "%2d | %-20s | %s\r\n", pcount, pproject->name, pproject->date );
         }
      }
      if( pcount == 0 )
         pager_printf( ch, "No projects exist.\r\n" );
      else if( aflag && !projects_available )
         pager_printf( ch, "No projects available.\r\n" );
      return;
   }

   if( !str_cmp( arg, "add" ) )
   {
      char *strtime;
      PROJECT_DATA *new_project; /* Just to be safe */

      if( get_trust( ch ) < LEVEL_GOD && str_cmp( ch->pcdata->council_name, "Code Council" ) )
      {
         send_to_char( "You are not powerfull enough to add a new project.\r\n", ch );
         return;
      }

      CREATE( new_project, PROJECT_DATA, 1 );
      LINK( new_project, first_project, last_project, next, prev );
      new_project->name = str_dup( argument );
      new_project->coder = NULL;
      new_project->taken = FALSE;
      new_project->description = STRALLOC( "" );
      strtime = ctime( &current_time );
      strtime[strlen( strtime ) - 1] = '\0';
      new_project->date = STRALLOC( strtime );
      write_projects(  );
      ch_printf( ch, "Ok.\r\n" );
      return;
   }

   if( !is_number( arg ) )
   {
      ch_printf( ch, "Invalid project.\r\n" );
      return;
   }

   pnum = atoi( arg );
   pproject = get_project_by_number( pnum );
   if( !pproject )
   {
      ch_printf( ch, "No such project.\r\n" );
      return;
   }

   argument = one_argument( argument, arg1 );

   if( !str_cmp( arg1, "description" ) )
   {
      if( get_trust( ch ) < LEVEL_GOD && str_cmp( ch->pcdata->council_name, "Code Council" ) )
         CHECK_SUBRESTRICTED( ch );
      ch->tempnum = SUB_NONE;
      ch->substate = SUB_PROJ_DESC;
      ch->dest_buf = pproject;
      if( pproject->description == NULL )
         pproject->description = STRALLOC( "" );
      start_editing( ch, pproject->description );
      return;
   }
   if( !str_cmp( arg1, "delete" ) )
   {
      NOTE_DATA *nlog, *tlog;
      if( str_cmp( ch->pcdata->council_name, "Code Council" ) && get_trust( ch ) < LEVEL_ASCENDANT )
      {
         send_to_char( "You are not high enough level to delete a project.\r\n", ch );
         return;
      }

      nlog = pproject->last_log;
      while( nlog )
      {
         UNLINK( nlog, pproject->first_log, pproject->last_log, next, prev );
         tlog = nlog->prev;
         free_note( nlog );
         nlog = tlog;
      }
      UNLINK( pproject, first_project, last_project, next, prev );

      DISPOSE( pproject->name );
      if( pproject->coder )
         DISPOSE( pproject->coder );
      if( pproject->owner )
         STRFREE( pproject->owner );
      if( pproject->description )
         STRFREE( pproject->description );
      if( pproject->date )
         STRFREE( pproject->date );
      if( pproject->status )
         STRFREE( pproject->status );

      DISPOSE( pproject );
      write_projects(  );
      ch_printf( ch, "Ok.\r\n" );
      return;
   }

   if( !str_cmp( arg1, "take" ) )
   {
      if( pproject->taken && pproject->owner && !str_cmp( pproject->owner, ch->name ) )
      {
         pproject->taken = FALSE;
         STRFREE( pproject->owner );
         pproject->owner = NULL;
         send_to_char( "You removed yourself as the owner.\r\n", ch );
         write_projects(  );
         return;
      }
      else if( pproject->taken )
      {
         ch_printf( ch, "This project is already taken.\r\n" );
         return;
      }


      if( pproject->owner )
         STRFREE( pproject->owner );
      pproject->owner = STRALLOC( ch->name );
      pproject->taken = TRUE;
      write_projects(  );
      ch_printf( ch, "Ok.\r\n" );
      return;
   }
   if( !str_cmp( arg1, "coder" ) )
   {
      if( pproject->coder && !str_cmp( ch->name, pproject->coder ) )
      {
         DISPOSE( pproject->coder );
         pproject->coder = NULL;
         send_to_char( "You removed yourself as the coder.\r\n", ch );
         write_projects(  );
         return;
      }
      else if( pproject->coder )
      {
         ch_printf( ch, "This project already has a coder.\r\n" );
         return;
      }
      pproject->coder = str_dup( ch->name );
      write_projects(  );
      ch_printf( ch, "Ok.\r\n" );
      return;
   }
   if( !str_cmp( arg1, "status" ) )
   {
      if( pproject->owner && str_cmp( pproject->owner, ch->name ) &&
          get_trust( ch ) < LEVEL_GREATER
          && pproject->coder && str_cmp( pproject->coder, ch->name ) && str_cmp( ch->pcdata->council_name, "Code Council" ) )
      {
         ch_printf( ch, "This is not your project!\r\n" );
         return;
      }
      if( pproject->status )
         STRFREE( pproject->status );
      pproject->status = STRALLOC( argument );
      write_projects(  );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   if( !str_cmp( arg1, "show" ) )
   {
      if( pproject->description )
         send_to_char( pproject->description, ch );
      else
         send_to_char( "That project does not have a description.\r\n", ch );
      return;
   }
   if( !str_cmp( arg1, "log" ) )
   {
      NOTE_DATA *plog;
      if( !str_cmp( argument, "write" ) )
      {
         note_attach( ch );
         ch->substate = SUB_WRITING_NOTE;
         ch->dest_buf = ch->pnote;
         start_editing( ch, ch->pnote->text );
         return;
      }

      argument = one_argument( argument, arg2 );

      if( !str_cmp( arg2, "subject" ) )
      {
         note_attach( ch );
         STRFREE( ch->pnote->subject );
         ch->pnote->subject = STRALLOC( argument );
         ch_printf( ch, "Ok.\r\n" );
         return;
      }

      if( !str_cmp( arg2, "post" ) )
      {
         char *strtime;

         if( pproject->owner && str_cmp( ch->name, pproject->owner ) &&
             pproject->coder && str_cmp( ch->name, pproject->coder ) &&
             get_trust( ch ) < LEVEL_GREATER && str_cmp( ch->pcdata->council_name, "Code Council" ) )
         {
            ch_printf( ch, "This is not your project!\r\n" );
            return;
         }

         if( !ch->pnote )
         {
            ch_printf( ch, "You have no log in progress.\r\n" );
            return;
         }

         if( !ch->pnote->subject )
         {
            ch_printf( ch, "Your log has no subject.\r\n" );
            return;
         }

         strtime = ctime( &current_time );
         strtime[strlen( strtime ) - 1] = '\0';
         ch->pnote->date = STRALLOC( strtime );
         ch->pnote->sender = ch->name;

         plog = ch->pnote;
         ch->pnote = NULL;
         LINK( plog, pproject->first_log, pproject->last_log, next, prev );
         write_projects(  );
         ch_printf( ch, "Ok.\r\n" );
         return;
      }

      if( !str_cmp( arg2, "list" ) )
      {
         if( pproject->owner && pproject->coder &&
             str_cmp( ch->name, pproject->owner ) && get_trust( ch ) < LEVEL_SAVIOR
             && str_cmp( ch->name, pproject->coder ) && str_cmp( ch->pcdata->council_name, "Code Council" ) )
         {
            ch_printf( ch, "This is not your project!\r\n" );
            return;
         }

         pcount = 0;
         pager_printf( ch, "Project: %-12s: %s\r\n", pproject->owner ? pproject->owner : "(None)", pproject->name );

         for( plog = pproject->first_log; plog; plog = plog->next )
         {
            pcount++;
            pager_printf( ch, "%2d) %-12s: %s\r\n", pcount, plog->sender, plog->subject );
         }
         if( pcount == 0 )
            ch_printf( ch, "No logs available.\r\n" );
         return;
      }

      if( !is_number( arg2 ) )
      {
         ch_printf( ch, "Invalid log.\r\n" );
         return;
      }

      pnum = atoi( arg2 );

      plog = get_log_by_number( pproject, pnum );
      if( !plog )
      {
         ch_printf( ch, "Invalid log.\r\n" );
         return;
      }


      if( !str_cmp( argument, "delete" ) )
      {
         if( pproject->owner && str_cmp( ch->name, pproject->owner ) &&
             get_trust( ch ) < LEVEL_ASCENDANT &&
             pproject->coder && str_cmp( ch->name, pproject->coder ) && str_cmp( ch->pcdata->council_name, "Code Council" ) )
         {
            ch_printf( ch, "This is not your project!\r\n" );
            return;
         }

         UNLINK( plog, pproject->first_log, pproject->last_log, next, prev );
         free_note( plog );
         write_projects(  );
         ch_printf( ch, "Ok.\r\n" );
         return;
      }

      if( !str_cmp( argument, "read" ) )
      {
         if( pproject->owner && pproject->coder &&
             str_cmp( ch->name, pproject->owner ) && get_trust( ch ) < LEVEL_SAVIOR
             && str_cmp( ch->name, pproject->coder ) && str_cmp( ch->pcdata->council_name, "Code Council" ) )
         {
            ch_printf( ch, "This is not your project!\r\n" );
            return;
         }

         pager_printf( ch, "[%3d] %s: %s\r\n%s\r\n%s", pnum, plog->sender, plog->subject, plog->date, plog->text );
         return;
      }
   }
   send_to_char( "Unknown syntax see help 'PROJECT'.\r\n", ch );
   return;
}

PROJECT_DATA *get_project_by_number( int pnum )
{
   int pcount;
   PROJECT_DATA *pproject;
   pcount = 0;
   for( pproject = first_project; pproject; pproject = pproject->next )
   {
      pcount++;
      if( pcount == pnum )
         return pproject;
   }
   return NULL;
}

NOTE_DATA *get_log_by_number( PROJECT_DATA * pproject, int pnum )
{
   int pcount;
   NOTE_DATA *plog;
   pcount = 0;
   for( plog = pproject->first_log; plog; plog = plog->next )
   {
      pcount++;
      if( pcount == pnum )
         return plog;
   }
   return NULL;
}

/*
 * Command to check for multiple ip addresses in the mud.
 * --Shaddai
 */

 /*
  * Added this new struct to do matching
  * If ya think of a better way do it, easiest way I could think of at
  * 2 in the morning :) --Shaddai
  */

typedef struct ipcompare_data IPCOMPARE_DATA;
struct ipcompare_data
{
   struct ipcompare_data *prev;
   struct ipcompare_data *next;
   char *host;
   char *name;
   int connected;
   int count;
   int descriptor;
   int idle;
   int port;
   bool printed;
};

void do_ipcompare( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   char *addie = NULL;
   bool prefix = FALSE, suffix = FALSE, inarea = FALSE, inroom = FALSE;
   int count = 0, times = -1;
   bool fMatch;
   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   set_pager_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   if( arg[0] == '\0' )
   {
      send_to_char( "ipcompare pkill\r\n", ch );
      send_to_char( "ipcompare total\r\n", ch );
      send_to_char( "ipcompare <person> [room|area|world] [#]\r\n", ch );
      send_to_char( "ipcompare <site>   [room|area|world] [#]\r\n", ch );
      return;
   }
   if( !str_cmp( arg, "total" ) )
   {
      IPCOMPARE_DATA *first_ip = NULL, *last_ip = NULL, *hmm, *hmm_next;
      for( d = first_descriptor; d; d = d->next )
      {
         fMatch = FALSE;
         for( hmm = first_ip; hmm; hmm = hmm->next )
            if( !str_cmp( hmm->host, d->host ) )
               fMatch = TRUE;
         if( !fMatch )
         {
            IPCOMPARE_DATA *temp;
            CREATE( temp, IPCOMPARE_DATA, 1 );
            temp->host = str_dup( d->host );
            LINK( temp, first_ip, last_ip, next, prev );
            count++;
         }
      }
      for( hmm = first_ip; hmm; hmm = hmm_next )
      {
         hmm_next = hmm->next;
         UNLINK( hmm, first_ip, last_ip, next, prev );
         if( hmm->host )
            DISPOSE( hmm->host );
         DISPOSE( hmm );
      }
      ch_printf( ch, "There were %d unique ip addresses found.\r\n", count );
      return;
   }
   else if( !str_cmp( arg, "pkill" ) )
   {
      IPCOMPARE_DATA *first_ip = NULL, *last_ip = NULL, *hmm, *hmm_next;
      snprintf( buf, MAX_STRING_LENGTH, "%s", "\r\nDesc|Con|Idle| Port | Player      " );
      if( get_trust( ch ) >= LEVEL_SAVIOR )
         mudstrlcat( buf, "@HostIP           ", MAX_STRING_LENGTH );
      if( get_trust( ch ) >= LEVEL_GOD )
         mudstrlcat( buf, "| Username", MAX_STRING_LENGTH );
      mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
      mudstrlcat( buf, "----+---+----+------+-------------", MAX_STRING_LENGTH );
      if( get_trust( ch ) >= LEVEL_SAVIOR )
         mudstrlcat( buf, "------------------", MAX_STRING_LENGTH );
      if( get_trust( ch ) >= LEVEL_GOD )
         mudstrlcat( buf, "+---------", MAX_STRING_LENGTH );
      mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
      send_to_pager( buf, ch );

      for( d = first_descriptor; d; d = d->next )
      {
         IPCOMPARE_DATA *temp;

         if( ( d->connected != CON_PLAYING && d->connected != CON_EDITING )
             || d->character == NULL || !CAN_PKILL( d->character ) || !can_see( ch, d->character ) )
            continue;
         CREATE( temp, IPCOMPARE_DATA, 1 );
         temp->host = str_dup( d->host );
         temp->descriptor = d->descriptor;
         temp->connected = d->connected;
         temp->idle = d->idle;
         temp->port = d->port;
         temp->name = ( d->original ? str_dup( d->original->name ) :
                        d->character ? str_dup( d->character->name ) : str_dup( "(none)" ) );
         temp->count = 0;
         temp->printed = FALSE;
         LINK( temp, first_ip, last_ip, next, prev );
      }

      for( d = first_descriptor; d; d = d->next )
      {
         fMatch = FALSE;
         if( ( d->connected != CON_PLAYING && d->connected != CON_EDITING )
             || d->character == NULL || !can_see( ch, d->character ) )
            continue;
         for( hmm = first_ip; hmm; hmm = hmm->next )
         {
            if( !str_cmp( hmm->host, d->host ) &&
                str_cmp( hmm->name, ( d->original ? d->original->name : d->character ? d->character->name : "(none)" ) ) )
            {
               fMatch = TRUE;
               break;
            }
         }
         if( fMatch && hmm )
         {
            hmm->count++;
            if( !hmm->printed && hmm->count > 0 )
            {
               snprintf( buf, MAX_STRING_LENGTH,
                         " %3d| %2d|%4d|%6d| %-12s", hmm->descriptor, hmm->connected, hmm->idle / 4, hmm->port, hmm->name );
               if( get_trust( ch ) >= LEVEL_SAVIOR )
                  snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "@%-16s ", hmm->host );
               mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
               send_to_pager( buf, ch );
               hmm->printed = TRUE;
            }
            snprintf( buf, MAX_STRING_LENGTH,
                      " %3d| %2d|%4d|%6d| %-12s",
                      d->descriptor,
                      d->connected,
                      d->idle / 4, d->port, d->original ? d->original->name : d->character ? d->character->name : "(none)" );
            if( get_trust( ch ) >= LEVEL_SAVIOR )
               snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "@%-16s ", d->host );
            mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
            send_to_pager( buf, ch );
         }
      }
      for( hmm = first_ip; hmm; hmm = hmm_next )
      {
         hmm_next = hmm->next;
         UNLINK( hmm, first_ip, last_ip, next, prev );
         if( hmm->name )
            DISPOSE( hmm->name );
         if( hmm->host )
            DISPOSE( hmm->host );
         DISPOSE( hmm );
      }
      return;
   }
   if( arg1[0] != '\0' )
   {
      if( is_number( arg1 ) )
         times = atoi( arg1 );
      else
      {
         if( !str_cmp( arg1, "room" ) )
            inroom = TRUE;
         else if( !str_cmp( arg1, "area" ) )
            inarea = TRUE;
      }
      if( arg2[0] != '\0' )
      {
         if( is_number( arg2 ) )
            times = atoi( arg2 );
         else
         {
            send_to_char( "Please see help ipcompare for more info.\r\n", ch );
            return;
         }
      }
   }
   if( ( victim = get_char_world( ch, arg ) ) != NULL && victim->desc )
   {
      if( IS_NPC( victim ) )
      {
         send_to_char( "Not on NPC's.\r\n", ch );
         return;
      }
      // we need to treat addie as non-const in the else block below,
      // so we can't make it a const char*. the "right" way to do this
      // would be to create a temporary string but then we need to manage
      // memory or copy things around. so we cast instead...
      addie = (char*) victim->desc->host;
   }
   else
   {
      addie = arg;
      if( arg[0] == '*' )
      {
         prefix = TRUE;
         addie++;
      }
      if( addie[strlen( addie ) - 1] == '*' )
      {
         suffix = TRUE;
         addie[strlen( addie ) - 1] = '\0';
      }
   }
   snprintf( buf, MAX_STRING_LENGTH, "%s", "\r\nDesc|Con|Idle| Port | Player      " );
   if( get_trust( ch ) >= LEVEL_SAVIOR )
      mudstrlcat( buf, "@HostIP           ", MAX_STRING_LENGTH );
   if( get_trust( ch ) >= LEVEL_GOD )
      mudstrlcat( buf, "| Username", MAX_STRING_LENGTH );
   mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
   mudstrlcat( buf, "----+---+----+------+-------------", MAX_STRING_LENGTH );
   if( get_trust( ch ) >= LEVEL_SAVIOR )
      mudstrlcat( buf, "------------------", MAX_STRING_LENGTH );
   if( get_trust( ch ) >= LEVEL_GOD )
      mudstrlcat( buf, "+---------", MAX_STRING_LENGTH );
   mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
   send_to_pager( buf, ch );
   for( d = first_descriptor; d; d = d->next )
   {
      if( !d->character || ( d->connected != CON_PLAYING && d->connected != CON_EDITING ) || !can_see( ch, d->character ) )
         continue;
      if( inroom && ch->in_room != d->character->in_room )
         continue;
      if( inarea && ch->in_room->area != d->character->in_room->area )
         continue;
      if( times > 0 && count == ( times - 1 ) )
         break;
      if( prefix && suffix && strstr( d->host, addie ) )
         fMatch = TRUE;
      else if( prefix && !str_suffix( addie, d->host ) )
         fMatch = TRUE;
      else if( suffix && !str_prefix( addie, d->host ) )
         fMatch = TRUE;
      else if( !str_cmp( d->host, addie ) )
         fMatch = TRUE;
      else
         fMatch = FALSE;
      if( fMatch )
      {
         count++;
         snprintf( buf, MAX_STRING_LENGTH,
                   " %3d| %2d|%4d|%6d| %-12s",
                   d->descriptor,
                   d->connected,
                   d->idle / 4, d->port, d->original ? d->original->name : d->character ? d->character->name : "(none)" );
         if( get_trust( ch ) >= LEVEL_SAVIOR )
            snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "@%-16s ", d->host );
         mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
         send_to_pager( buf, ch );
      }
   }
   pager_printf( ch, "%d user%s.\r\n", count, count == 1 ? "" : "s" );
   return;
}


/*
 * New nuisance flag to annoy people that deserve it :) --Shaddai
 */
void do_nuisance( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   struct tm *now_time;
   int sttime = 0, max_time = 0, power = 1;
   bool minute = FALSE, day = FALSE, hour = FALSE;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax: nuisance <victim> [Options]\r\n", ch );
      send_to_char( "Options:\r\n", ch );
      send_to_char( "  power <level 1-10>\r\n", ch );
      send_to_char( "  time  <days>\r\n", ch );
      send_to_char( "  maxtime <#> <minutes/hours/days>\r\n", ch );
      send_to_char( "Defaults: Time -- forever, power -- 1, maxtime 8 days.\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "There is no one on with that name.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't set a nuisance flag on a mob.\r\n", ch );
      return;
   }

   if( get_trust( ch ) <= get_trust( victim ) )
   {
      send_to_char( "I don't think they would like that.\r\n", ch );
      return;
   }

   if( victim->pcdata->nuisance )
   {
      send_to_char( "That flag has already been set.\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg1 );

   while( argument[0] != '\0' )
   {
      if( !str_cmp( arg1, "power" ) )
      {
         argument = one_argument( argument, arg1 );
         if( arg1[0] == '\0' || !is_number( arg1 ) )
         {
            send_to_char( "Power option syntax: power <number>\r\n", ch );
            return;
         }
         if( ( power = atoi( arg1 ) ) < 1 || power > 10 )
         {
            send_to_char( "Power must be 1 - 10.\r\n", ch );
            return;
         }
      }
      else if( !str_cmp( arg1, "time" ) )
      {
         argument = one_argument( argument, arg1 );
         if( arg1[0] == '\0' || !is_number( arg1 ) )
         {
            send_to_char( "Time option syntax: time <number> (In days)\r\n", ch );
            return;
         }
         if( ( sttime = atoi( arg1 ) ) < 1 )
         {
            send_to_char( "Time must be a positive number.\r\n", ch );
            return;
         }
      }
      else if( !str_cmp( arg1, "maxtime" ) )
      {
         argument = one_argument( argument, arg1 );
         argument = one_argument( argument, arg2 );
         if( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg1 ) )
         {
            send_to_char( "Maxtime option syntax: maxtime <number> <minute|day|hour>\r\n", ch );
            return;
         }
         if( ( max_time = atoi( arg1 ) ) < 1 )
         {
            send_to_char( "Maxtime must be a positive number.\r\n", ch );
            return;
         }
         if( !str_cmp( arg2, "minutes" ) )
            minute = TRUE;
         else if( !str_cmp( arg2, "hours" ) )
            hour = TRUE;
         else if( !str_cmp( arg2, "days" ) )
            day = TRUE;
      }
      else
      {
         ch_printf( ch, "Unknown option %s.\r\n", arg1 );
         return;
      }
      argument = one_argument( argument, arg1 );
   }

   if( minute && ( max_time < 1 || max_time > 59 ) )
   {
      send_to_char( "Minutes must be 1 to 59.\r\n", ch );
      return;
   }
   else if( hour && ( max_time < 1 || max_time > 23 ) )
   {
      send_to_char( "Hours must be 1 - 23.\r\n", ch );
      return;
   }
   else if( day && ( max_time < 1 || max_time > 999 ) )
   {
      send_to_char( "Days must be 1 - 999.\r\n", ch );
      return;
   }
   else if( !max_time )
   {
      day = TRUE;
      max_time = 7;
   }
   CREATE( victim->pcdata->nuisance, NUISANCE_DATA, 1 );
   victim->pcdata->nuisance->set_time = current_time;
   victim->pcdata->nuisance->flags = 1;
   victim->pcdata->nuisance->power = power;
   now_time = localtime( &current_time );

   if( minute )
      now_time->tm_min += max_time;
   else if( hour )
      now_time->tm_hour += max_time;
   else
      now_time->tm_mday += max_time;

   victim->pcdata->nuisance->max_time = mktime( now_time );
   if( sttime )
   {
      add_timer( victim, TIMER_NUISANCE, ( 28800 * sttime ), NULL, 0 );
      ch_printf( ch, "Nuisance flag set for %d days.\r\n", sttime );
   }
   else
      send_to_char( "Nuisance flag set forever\r\n", ch );
   return;
}

void do_unnuisance( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   TIMER *timer, *timer_next;
   char arg[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
   one_argument( argument, arg );

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "There is no one on with that name.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't remove a nuisance flag from a mob.\r\n", ch );
      return;
   }
   if( get_trust( ch ) <= get_trust( victim ) )
   {
      send_to_char( "You can't do that.\r\n", ch );
      return;
   }
   if( !victim->pcdata->nuisance )
   {
      send_to_char( "They do not have that flag set.\r\n", ch );
      return;
   }
   for( timer = victim->first_timer; timer; timer = timer_next )
   {
      timer_next = timer->next;
      if( timer->type == TIMER_NUISANCE )
         extract_timer( victim, timer );
   }
   DISPOSE( victim->pcdata->nuisance );
   send_to_char( "Nuisance flag removed.\r\n", ch );
   return;
}

void do_pcrename( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char newname[MAX_STRING_LENGTH];
   char oldname[MAX_STRING_LENGTH];
   char backname[MAX_STRING_LENGTH];

   argument = one_argument( argument, arg1 );
   one_argument( argument, arg2 );
   smash_tilde( arg2 );


   if( IS_NPC( ch ) )
      return;

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Syntax: rename <victim> <new name>\r\n", ch );
      return;
   }

   if( !check_parse_name( arg2, 1 ) )
   {
      send_to_char( "Illegal name.\r\n", ch );
      return;
   }

   /*
    * Just a security precaution so you don't rename someone you don't mean 
    * * too --Shaddai
    */
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "That person is not in the room.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't rename NPC's.\r\n", ch );
      return;
   }

   if( get_trust( ch ) < get_trust( victim ) )
   {
      send_to_char( "I don't think they would like that!\r\n", ch );
      return;
   }
   snprintf( newname, MAX_STRING_LENGTH, "%s%c/%s", PLAYER_DIR, tolower( arg2[0] ), capitalize( arg2 ) );
   snprintf( oldname, MAX_STRING_LENGTH, "%s%c/%s", PLAYER_DIR, tolower( victim->pcdata->filename[0] ),
             capitalize( victim->pcdata->filename ) );
   snprintf( backname, MAX_STRING_LENGTH, "%s%c/%s", BACKUP_DIR, tolower( victim->pcdata->filename[0] ),
             capitalize( victim->pcdata->filename ) );
   if( access( newname, F_OK ) == 0 )
   {
      send_to_char( "That name already exists.\r\n", ch );
      return;
   }

   /*
    * Have to remove the old god entry in the directories 
    */
   if( IS_IMMORTAL( victim ) )
   {
      char godname[MAX_STRING_LENGTH];
      snprintf( godname, MAX_STRING_LENGTH, "%s%s", GOD_DIR, capitalize( victim->pcdata->filename ) );
      remove( godname );
   }

   /*
    * Remember to change the names of the areas 
    */
   if( victim->pcdata->area )
   {
      char filename[MAX_STRING_LENGTH];
      char newfilename[MAX_STRING_LENGTH];

      snprintf( filename, MAX_STRING_LENGTH, "%s%s.are", BUILD_DIR, victim->name );
      snprintf( newfilename, MAX_STRING_LENGTH, "%s%s.are", BUILD_DIR, capitalize( arg2 ) );
      rename( filename, newfilename );
      snprintf( filename, MAX_STRING_LENGTH, "%s%s.are.bak", BUILD_DIR, victim->name );
      snprintf( newfilename, MAX_STRING_LENGTH, "%s%s.are.bak", BUILD_DIR, capitalize( arg2 ) );
      rename( filename, newfilename );
   }

   STRFREE( victim->name );
   victim->name = STRALLOC( capitalize( arg2 ) );
   STRFREE( victim->pcdata->filename );
   victim->pcdata->filename = STRALLOC( capitalize( arg2 ) );
   remove( backname );
   if( remove( oldname ) )
   {
      log_printf( "Error: Couldn't delete file %s in do_rename.", oldname );
      send_to_char( "Couldn't delete the old file!\r\n", ch );
   }
   /*
    * Time to save to force the affects to take place 
    */
   save_char_obj( victim );

   /*
    * Now lets update the wizlist 
    */
   if( IS_IMMORTAL( victim ) )
      make_wizlist(  );
   send_to_char( "Character was renamed.\r\n", ch );
   return;
}

bool check_area_conflict( AREA_DATA * area, int low_range, int hi_range )
{
   if( low_range < area->low_r_vnum && area->low_r_vnum < hi_range )
      return TRUE;
   if( low_range < area->low_m_vnum && area->low_m_vnum < hi_range )
      return TRUE;
   if( low_range < area->low_o_vnum && area->low_o_vnum < hi_range )
      return TRUE;

   if( low_range < area->hi_r_vnum && area->hi_r_vnum < hi_range )
      return TRUE;
   if( low_range < area->hi_m_vnum && area->hi_m_vnum < hi_range )
      return TRUE;
   if( low_range < area->hi_o_vnum && area->hi_o_vnum < hi_range )
      return TRUE;

   if( ( low_range >= area->low_r_vnum ) && ( low_range <= area->hi_r_vnum ) )
      return TRUE;
   if( ( low_range >= area->low_m_vnum ) && ( low_range <= area->hi_m_vnum ) )
      return TRUE;
   if( ( low_range >= area->low_o_vnum ) && ( low_range <= area->hi_o_vnum ) )
      return TRUE;

   if( ( hi_range <= area->hi_r_vnum ) && ( hi_range >= area->low_r_vnum ) )
      return TRUE;
   if( ( hi_range <= area->hi_m_vnum ) && ( hi_range >= area->low_m_vnum ) )
      return TRUE;
   if( ( hi_range <= area->hi_o_vnum ) && ( hi_range >= area->low_o_vnum ) )
      return TRUE;

   return FALSE;
}

/* Runs the entire list, easier to call in places that have to check them all */
bool check_area_conflicts( int lo, int hi )
{
   AREA_DATA *area;

   for( area = first_area; area; area = area->next )
      if( check_area_conflict( area, lo, hi ) )
         return TRUE;

   for( area = first_build; area; area = area->next )
      if( check_area_conflict( area, lo, hi ) )
         return TRUE;

   return FALSE;
}

/* Consolidated *assign function. 
 * Assigns room/obj/mob ranges and initializes new zone - Samson 2-12-99 
 */
/* Bugfix: Vnum range would not be saved properly without placeholders at both ends - Samson 1-6-00 */
void do_vassign( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
   int lo = -1, hi = -1;
   CHAR_DATA *victim, *mob;
   ROOM_INDEX_DATA *room;
   MOB_INDEX_DATA *pMobIndex;
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   AREA_DATA *tarea;
   char filename[256];

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   lo = atoi( arg2 );
   hi = atoi( arg3 );

   if( arg1[0] == '\0' || lo < 0 || hi < 0 )
   {
      send_to_char( "Syntax: vassign <who> <low> <high>\r\n", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg1 ) ) )
   {
      send_to_char( "They don't seem to be around.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) || get_trust( victim ) < LEVEL_CREATOR )
   {
      send_to_char( "They wouldn't know what to do with a vnum range.\r\n", ch );
      return;
   }

   if( lo == 0 && hi == 0 )
   {
      if( victim->pcdata->area )
         close_area( victim->pcdata->area );
      victim->pcdata->area = NULL;
      victim->pcdata->r_range_lo = 0;
      victim->pcdata->r_range_hi = 0;
      victim->pcdata->o_range_lo = 0;
      victim->pcdata->o_range_hi = 0;
      victim->pcdata->m_range_lo = 0;
      victim->pcdata->m_range_hi = 0;
      ch_printf( victim, "%s has removed your vnum range.\r\n", ch->name );
      save_char_obj( victim );
      return;
   }

   if( victim->pcdata->area && lo != 0 )
   {
      send_to_char( "You cannot assign them a range, they already have one!\r\n", ch );
      return;
   }

   if( lo == 0 && hi != 0 )
   {
      send_to_char( "Unacceptable vnum range, low vnum cannot be 0 when hi vnum is not.\r\n", ch );
      return;
   }

   if( lo > hi )
   {
      send_to_char( "Unacceptable vnum range, low vnum must be smaller than high vnum.\r\n", ch );
      return;
   }

   if( check_area_conflicts( lo, hi ) )
   {
      send_to_char( "That vnum range conflicts with another area. Check the zones or vnums command.\r\n", ch );
      return;
   }

   victim->pcdata->r_range_lo = lo;
   victim->pcdata->r_range_hi = hi;
   victim->pcdata->o_range_lo = lo;
   victim->pcdata->o_range_hi = hi;
   victim->pcdata->m_range_lo = lo;
   victim->pcdata->m_range_hi = hi;
   assign_area( victim );
   send_to_char( "Done.\r\n", ch );
   ch_printf( victim, "%s has assigned you the vnum range %d - %d.\r\n", ch->name, lo, hi );
   assign_area( victim );  /* Put back by Thoric on 02/07/96 */

   if( !victim->pcdata->area )
   {
      bug( "%s: assign_area failed", __FUNCTION__ );
      return;
   }

   tarea = victim->pcdata->area;

   /*
    * Initialize first and last rooms in range 
    */
   if( !( room = make_room( lo, tarea ) ) )
   {
      bug( "%s: make_room failed to initialize first room.", __FUNCTION__ );
      return;
   }

   if( !( room = make_room( hi, tarea ) ) )
   {
      bug( "%s: make_room failed to initialize last room.", __FUNCTION__ );
      return;
   }

   /*
    * Initialize first mob in range 
    */
   if( !( pMobIndex = make_mobile( lo, 0, "first mob" ) ) )
   {
      bug( "%s: make_mobile failed to initialize first mob.", __FUNCTION__ );
      return;
   }
   mob = create_mobile( pMobIndex );
   char_to_room( mob, room );

   /*
    * Initialize last mob in range 
    */
   if( !( pMobIndex = make_mobile( hi, 0, "last mob" ) ) )
   {
      bug( "%s: make_mobile failed to initialize last mob.", __FUNCTION__ );
      return;
   }
   mob = create_mobile( pMobIndex );
   char_to_room( mob, room );

   /*
    * Initialize first obj in range 
    */
   if( !( pObjIndex = make_object( lo, 0, "first obj" ) ) )
   {
      bug( "%s: make_object failed to initialize first obj.", __FUNCTION__ );
      return;
   }
   obj = create_object( pObjIndex, 0 );
   obj_to_room( obj, room );

   /*
    * Initialize last obj in range 
    */
   if( !( pObjIndex = make_object( hi, 0, "last obj" ) ) )
   {
      bug( "%s: make_object failed to initialize last obj.", __FUNCTION__ );
      return;
   }
   obj = create_object( pObjIndex, 0 );
   obj_to_room( obj, room );

   /*
    * Save character and newly created zone 
    */
   save_char_obj( victim );

   if( !IS_SET( tarea->status, AREA_DELETED ) )
   {
      SET_BIT( tarea->flags, AFLAG_PROTOTYPE );
      SET_BIT( tarea->status, AREA_LOADED );
      snprintf( filename, 256, "%s%s", BUILD_DIR, tarea->filename );
      fold_area( tarea, filename, FALSE );
   }

   set_char_color( AT_IMMORT, ch );
   ch_printf( ch, "Vnum range set for %s and initialized.\r\n", victim->name );
}

/*
 * oowner will make an item owned by a player so only that player can use it.
 * Shaddai
 */
void do_oowner( CHAR_DATA* ch, const char* argument)
{
   OBJ_DATA *obj;
   CHAR_DATA *victim = NULL;
   char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Syntax: oowner <object> <player|none>\r\n", ch );
      return;
   }

   if( str_cmp( arg2, "none" ) && ( victim = get_char_world( ch, arg2 ) ) == NULL )
   {
      send_to_char( "No such player is in the game.\r\n", ch );
      return;
   }

   if( ( obj = get_obj_here( ch, arg1 ) ) == NULL )
   {
      send_to_char( "No such object exists.\r\n", ch );
      return;
   }

   separate_obj( obj );

   if( !str_cmp( "none", arg2 ) )
   {
      STRFREE( obj->owner );
      obj->owner = STRALLOC( "" );
      xREMOVE_BIT( obj->extra_flags, ITEM_PERSONAL );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "A mob can't be an owner of an item.\r\n", ch );
      return;
   }
   STRFREE( obj->owner );
   obj->owner = STRALLOC( victim->name );
   xSET_BIT( obj->extra_flags, ITEM_PERSONAL );
   send_to_char( "Done.\r\n", ch );
   return;
}
