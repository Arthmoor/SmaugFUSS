/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.8 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops, Fireblade, Edmond, Conran                         |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *                            Ban module by Shaddai                         *
 ****************************************************************************/

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include "mud.h"

/* Local functions */
void fread_ban args( ( FILE * fp, int type ) );
bool check_expire args( ( BAN_DATA * ban ) );
void dispose_ban args( ( BAN_DATA * ban, int type ) );
void free_ban args( ( BAN_DATA * pban ) );

/* Global Variables */

BAN_DATA *first_ban;
BAN_DATA *last_ban;
BAN_DATA *first_ban_class;
BAN_DATA *last_ban_class;
BAN_DATA *first_ban_race;
BAN_DATA *last_ban_race;


/*
 * Load all those nasty bans up :)
 * 	Shaddai
 */
void load_banlist( void )
{
   const char *word;
   FILE *fp;
   bool fMatch = FALSE;

   if( !( fp = fopen( SYSTEM_DIR BAN_LIST, "r" ) ) )
   {
      bug( "%s: Cannot open %s", __FUNCTION__, BAN_LIST );
      perror( BAN_LIST );
      return;
   }
   for( ;; )
   {
      word = feof( fp ) ? "END" : fread_word( fp );
      fMatch = FALSE;
      switch ( UPPER( word[0] ) )
      {
         case 'C':
            if( !str_cmp( word, "CLASS" ) )
            {
               fread_ban( fp, BAN_CLASS );
               fMatch = TRUE;
            }
            break;
         case 'E':
            if( !str_cmp( word, "END" ) ) /*File should always contain END */
            {
               fclose( fp );
               log_string( "Done." );
               return;
            }
         case 'R':
            if( !str_cmp( word, "RACE" ) )
            {
               fread_ban( fp, BAN_RACE );
               fMatch = TRUE;
            }
            break;
         case 'S':
            if( !str_cmp( word, "SITE" ) )
            {
               fread_ban( fp, BAN_SITE );
               fMatch = TRUE;
            }
            break;
      }
      if( !fMatch )
      {
         bug( "Load_banlist: no match: %s", word );
         fread_to_eol( fp );
      }  /* End of switch statement */
   }  /* End of for loop */
}

/*
 * Load up one class or one race ban structure.
 */

void fread_ban( FILE * fp, int type )
{
   BAN_DATA *pban;
   unsigned int i = 0;
   bool fMatch = FALSE;

   CREATE( pban, BAN_DATA, 1 );

   pban->name = fread_string_nohash( fp );
   pban->level = fread_number( fp );
   pban->duration = fread_number( fp );
   pban->unban_date = fread_number( fp );
   if( type == BAN_SITE )
   {  /* Sites have 2 extra numbers written out */
      pban->prefix = fread_number( fp );
      pban->suffix = fread_number( fp );
   }
   pban->warn = fread_number( fp );
   pban->ban_by = fread_string_nohash( fp );
   pban->ban_time = fread_string_nohash( fp );
   pban->note = fread_string( fp );

   /*
    * Need to lookup the class or race number if it is of that type 
    */

   if( type == BAN_CLASS )
      for( i = 0; i < MAX_CLASS; i++ )
      {
         if( !str_cmp( class_table[i]->who_name, pban->name ) )
         {
            fMatch = TRUE;
            break;
         }
      }
   else if( type == BAN_RACE )
      for( i = 0; i < MAX_RACE; i++ )
      {
         if( !str_cmp( race_table[i]->race_name, pban->name ) )
         {
            fMatch = TRUE;
            break;
         }
      }
   else if( type == BAN_SITE )
      for( i = 0; i < strlen( pban->name ); i++ )
      {
         if( pban->name[i] == '@' )
         {
            char *temp;
            const char *temp2;

            temp = str_dup( pban->name );
            temp[i] = '\0';
            temp2 = &pban->name[i + 1];
            DISPOSE( pban->name );
            pban->name = str_dup( temp2 );
            DISPOSE( temp );
            break;
         }
      }

   if( type == BAN_RACE || type == BAN_CLASS )
   {
      if( fMatch )
         pban->flag = i;
      else  /* The file is corupted throw out this ban structure */
      {
         bug( "Bad class structure %d.\r\n", i );
         free_ban( pban );
         return;
      }
   }
   if( type == BAN_CLASS )
      LINK( pban, first_ban_class, last_ban_class, next, prev );
   else if( type == BAN_RACE )
      LINK( pban, first_ban_race, last_ban_race, next, prev );
   else if( type == BAN_SITE )
      LINK( pban, first_ban, last_ban, next, prev );
   else  /* Bad type throw out the ban structure */
   {
      bug( "Fread_ban: Bad type %d", type );
      free_ban( pban );
   }
   return;
}

/*
 * Saves all bans, for sites, classes and races.
 * 	Shaddai
 */

void save_banlist( void )
{
   BAN_DATA *pban;
   FILE *fp;

   if( !( fp = fopen( SYSTEM_DIR BAN_LIST, "w" ) ) )
   {
      bug( "Save_banlist: Cannot open %s", BAN_LIST );
      perror( BAN_LIST );
      return;
   }

   /*
    * Print out all the site bans 
    */

   for( pban = first_ban; pban; pban = pban->next )
   {
      fprintf( fp, "SITE\n" );
      fprintf( fp, "%s~\n", pban->name );
      fprintf( fp, "%d %d %d %d %d %d\n", pban->level, pban->duration,
               pban->unban_date, pban->prefix, pban->suffix, pban->warn );
      fprintf( fp, "%s~\n%s~\n%s~\n", pban->ban_by, pban->ban_time, pban->note );
   }

   /*
    * Print out all the race bans 
    */

   for( pban = first_ban_race; pban; pban = pban->next )
   {
      fprintf( fp, "RACE\n" );
      fprintf( fp, "%s~\n", pban->name );
      fprintf( fp, "%d %d %d %d\n", pban->level, pban->duration, pban->unban_date, pban->warn );
      fprintf( fp, "%s~\n%s~\n%s~\n", pban->ban_by, pban->ban_time, pban->note );
   }

   /*
    * Print out all the class bans 
    */

   for( pban = first_ban_class; pban; pban = pban->next )
   {
      fprintf( fp, "CLASS\n" );
      fprintf( fp, "%s~\n", pban->name );
      fprintf( fp, "%d %d %d %d\n", pban->level, pban->duration, pban->unban_date, pban->warn );
      fprintf( fp, "%s~\n%s~\n%s~\n", pban->ban_by, pban->ban_time, pban->note );
   }
   fprintf( fp, "END\n" ); /* File must have an END even if empty */
   fclose( fp );
   return;
}

/*
 * The main command for ban, lots of arguments so be carefull what you
 * change here.		Shaddai
 */

void do_ban( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   char *temp;
   BAN_DATA *pban;
   int value = 0, btime;

   if( IS_NPC( ch ) )   /* Don't want mobs banning sites ;) */
   {
      send_to_char( "Monsters are too dumb to do that!\r\n", ch );
      return;
   }

   if( !ch->desc )   /* No desc means no go :) */
   {
      bug( "%s", "do_ban: no descriptor" );
      return;
   }

   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   argument = one_argument( argument, arg4 );

   /*
    * Do we have a time duration for the ban? 
    */

   if( arg4[0] != '\0' && is_number( arg4 ) )
      btime = atoi( arg4 );
   else
      btime = -1;


   /*
    * -1 is default, but no reason the time should be greater than 1000
    * * or less than 1, after all if it is greater than 1000 you are talking
    * * around 3 years.
    */

   if( btime != -1 && ( btime < 1 || btime > 1000 ) )
   {
      send_to_char( "Time value is -1 (forever) or from 1 to 1000.\r\n", ch );
      return;
   }

   /*
    * Need to be carefull with sub-states or everything will get messed up.
    */

   switch ( ch->substate )
   {
      default:
         bug( "%s", "do_ban: illegal substate" );
         return;
      case SUB_RESTRICTED:
         send_to_char( "You cannot use this command from within another command.\r\n", ch );
         return;
      case SUB_NONE:
         ch->tempnum = SUB_NONE;
         break;

         /*
          * Returning to end the editing of the note 
          */

      case SUB_BAN_DESC:
         add_ban( ch, "", "", 0, 0 );
         return;
   }
   if( arg1[0] == '\0' )
      goto syntax_message;

   /*
    * If no args are sent after the class/site/race, show the current banned
    * * items.  Shaddai
    */

   if( !str_cmp( arg1, "site" ) )
   {
      if( arg2[0] == '\0' )
      {
         show_bans( ch, BAN_SITE );
         return;
      }

      /*
       * Are they high enough to ban sites? 
       */

      if( get_trust( ch ) < sysdata.ban_site_level )
      {
         ch_printf( ch, "You must be %d level to add bans.\r\n", sysdata.ban_site_level );
         return;
      }
      if( arg3[0] == '\0' )
         goto syntax_message;
      if( !add_ban( ch, arg2, arg3, btime, BAN_SITE ) )
         return;
   }
   else if( !str_cmp( arg1, "race" ) )
   {
      if( arg2[0] == '\0' )
      {
         show_bans( ch, BAN_RACE );
         return;
      }

      /*
       * Are they high enough level to ban races? 
       */

      if( get_trust( ch ) < sysdata.ban_race_level )
      {
         ch_printf( ch, "You must be %d level to add bans.\r\n", sysdata.ban_race_level );
         return;
      }
      if( arg3[0] == '\0' )
         goto syntax_message;
      if( !add_ban( ch, arg2, arg3, btime, BAN_RACE ) )
         return;
   }
   else if( !str_cmp( arg1, "class" ) )
   {
      if( arg2[0] == '\0' )
      {
         show_bans( ch, BAN_CLASS );
         return;
      }

      /*
       * Are they high enough to ban classes? 
       */

      if( get_trust( ch ) < sysdata.ban_class_level )
      {
         ch_printf( ch, "You must be %d level to add bans.\r\n", sysdata.ban_class_level );
         return;
      }
      if( arg3[0] == '\0' )
         goto syntax_message;
      if( !add_ban( ch, arg2, arg3, btime, BAN_CLASS ) )
         return;
   }
   else if( !str_cmp( arg1, "show" ) )
   {

      /*
       * This will show the note attached to a ban 
       */

      if( arg2[0] == '\0' || arg3[0] == '\0' )
         goto syntax_message;
      temp = arg3;
      if( arg3[0] == '#' ) /* Use #1 to show the first ban */
      {
         temp = arg3;
         temp++;
         if( !is_number( temp ) )
         {
            send_to_char( "Which ban # to show?\r\n", ch );
            return;
         }
         value = atoi( temp );
         if( value < 1 )
         {
            send_to_char( "You must specify a number greater than 0.\r\n", ch );
            return;
         }
      }
      if( !str_cmp( arg2, "site" ) )
      {
         pban = first_ban;
         if( temp[0] == '*' )
            temp++;
         if( temp[strlen( temp ) - 1] == '*' )
            temp[strlen( temp ) - 1] = '\0';
      }
      else if( !str_cmp( arg2, "class" ) )
         pban = first_ban_class;
      else if( !str_cmp( arg2, "race" ) )
         pban = first_ban_race;
      else
         goto syntax_message;
      for( ; pban; pban = pban->next )
         if( value == 1 || !str_cmp( pban->name, temp ) )
            break;
         else if( value > 1 )
            value--;

      if( !pban )
      {
         send_to_char( "No such ban.\r\n", ch );
         return;
      }
      ch_printf( ch, "Banned by: %s\r\n", pban->ban_by );
      send_to_char( pban->note, ch );
      return;
   }
   else
      goto syntax_message;
   return;

/* Catch all syntax message, make sure that return stays above this or you
 * will get the syntax message everytime you issue the command even if it
 * is a valid one.  Shaddai
 */

 syntax_message:
   send_to_char( "Syntax: ban site  <address> <type> <duration>\r\n", ch );
   send_to_char( "Syntax: ban race  <race>    <type> <duration>\r\n", ch );
   send_to_char( "Syntax: ban class <class>   <type> <duration>\r\n", ch );
   send_to_char( "Syntax: ban show  <field>   <number>\r\n", ch );
   send_to_char( "Ban site lists current bans.\r\n", ch );
   send_to_char( "Duration is the length of the ban in days.\r\n", ch );
   send_to_char( "Type can be:  newbie, mortal, all, warn or level.\r\n", ch );
   send_to_char( "In ban show, the <field> is site, race or class,", ch );
   send_to_char( "  and the <number> is the ban number.\r\n", ch );
   return;
}


/*
 * Allow a already banned site/class or race.  Shaddai
 */

void do_allow( CHAR_DATA* ch, const char* argument)
{
   BAN_DATA *pban;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char *temp = NULL;
   bool fMatch = FALSE;
   int value = 0;

   if( IS_NPC( ch ) )   /* No mobs allowing sites */
   {
      send_to_char( "Monsters are too dumb to do that!\r\n", ch );
      return;
   }

   if( !ch->desc )   /* No desc is a bad thing */
   {
      bug( "%s", "do_allow: no descriptor" );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   set_char_color( AT_IMMORT, ch );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
      goto syntax_message;
   if( arg2[0] == '#' ) /* Use #1 to ban the first ban in the list specified */
   {
      temp = arg2;
      temp++;
      if( !is_number( temp ) )
      {
         send_to_char( "Which ban # to allow?\r\n", ch );
         return;
      }
      value = atoi( temp );
   }
   if( !str_cmp( arg1, "site" ) )
   {
      if( !value )
      {
         if( strlen( arg2 ) < 2 )
         {
            send_to_char( "You have to have at least 2 chars for a ban\r\n", ch );
            send_to_char( "If you are trying to allow by number use #\r\n", ch );
            return;
         }

         temp = arg2;
         if( arg2[0] == '*' )
            temp++;
         if( temp[strlen( temp ) - 1] == '*' )
            temp[strlen( temp ) - 1] = '\0';
      }

      for( pban = first_ban; pban; pban = pban->next )
      {
         /*
          * Need to make sure we dispose properly of the ban_data 
          * * Or memory problems will be created.
          * * Shaddai
          */

         if( value == 1 || !str_cmp( pban->name, temp ) )
         {
            fMatch = TRUE;
            dispose_ban( pban, BAN_SITE );
            break;
         }
         if( value > 1 )
            value--;
      }
   }
   else if( !str_cmp( arg1, "race" ) )
   {

      arg2[0] = toupper( arg2[0] );
      for( pban = first_ban_race; pban; pban = pban->next )
      {
         /*
          * Need to make sure we dispose properly of the ban_data 
          * * Or memory problems will be created.
          * * Shaddai
          */

         if( value == 1 || !str_cmp( pban->name, arg2 ) )
         {
            fMatch = TRUE;
            dispose_ban( pban, BAN_RACE );
            break;
         }
         if( value > 1 )
            value--;
      }
   }
   else if( !str_cmp( arg1, "class" ) )
   {

      arg2[0] = toupper( arg2[0] );
      for( pban = first_ban_class; pban; pban = pban->next )
      {
         /*
          * Need to make sure we dispose properly of the ban_data 
          * * Or memory problems will be created.
          * * Shaddai
          */

         if( value == 1 || !str_cmp( pban->name, arg2 ) )
         {
            fMatch = TRUE;
            dispose_ban( pban, BAN_CLASS );
            break;
         }
         if( value > 1 )
            value--;
      }
   }
   else
      goto syntax_message;

   if( fMatch )
   {
      save_banlist(  );
      ch_printf( ch, "%s is now allowed.\r\n", arg2 );
   }
   else
      ch_printf( ch, "%s was not banned.\r\n", arg2 );
   return;

/*
 *  Make sure that return above stays in!
 */

 syntax_message:
   send_to_char( "Syntax: allow site  <address>\r\n", ch );
   send_to_char( "Syntax: allow race  <race>\r\n", ch );
   send_to_char( "Syntax: allow class <class>\r\n", ch );
   return;
}

/* 
 *  Sets the warn flag on bans.
 */
void do_warn( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_STRING_LENGTH];
   char arg2[MAX_STRING_LENGTH];
   char *name;
   int count = -1, type;
   BAN_DATA *pban;

   /*
    * Don't want mobs or link-deads doing this.
    */

   if( IS_NPC( ch ) )
   {
      send_to_char( "Monsters are too dumb to do that!\r\n", ch );
      return;
   }

   if( !ch->desc )
   {
      bug( "%s", "do_warn: no descriptor" );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
      goto syntax_message;

   if( arg2[0] == '#' )
   {
      name = arg2;
      name++;
      if( !is_number( name ) )
         goto syntax_message;
      count = atoi( name );
      if( count < 1 )
      {
         send_to_char( "The number has to be above 0.\r\n", ch );
         return;
      }
   }

   /*
    *  We simply set up which ban list we will be looking at here.
    */
   if( !str_cmp( arg1, "class" ) )
      type = BAN_CLASS;
   else if( !str_cmp( arg1, "race" ) )
      type = BAN_RACE;
   else if( !str_cmp( arg1, "site" ) )
      type = BAN_SITE;
   else
      type = -1;


   if( type == BAN_CLASS )
   {
      pban = first_ban_class;
      arg2[0] = toupper( arg2[0] );
   }
   else if( type == BAN_RACE )
   {
      pban = first_ban_race;
      arg2[0] = toupper( arg2[0] );
   }
   else if( type == BAN_SITE )
   {
      pban = first_ban;
   }
   else
      goto syntax_message;
   for( ; pban && count != 0; count--, pban = pban->next )
      if( count == -1 && !str_cmp( pban->name, arg2 ) )
         break;
   if( pban )
   {
      /*
       * If it is just a warn delete it, otherwise remove the warn flag. 
       */

      if( pban->warn )
      {
         if( pban->level == BAN_WARN )
         {
            dispose_ban( pban, type );
            send_to_char( "Warn has been deleted.\r\n", ch );
         }
         else
         {
            pban->warn = FALSE;
            send_to_char( "Warn turned off.\r\n", ch );
         }
      }
      else
      {
         pban->warn = TRUE;
         send_to_char( "Warn turned on.\r\n", ch );
      }
      save_banlist(  );
   }
   else
   {
      ch_printf( ch, "%s was not found in the ban list.\r\n", arg2 );
      return;
   }
   return;
   /*
    * The above return has to stay in! 
    */
 syntax_message:
   send_to_char( "Syntax: warn class <field>\r\n", ch );
   send_to_char( "Syntax: warn race  <field>\r\n", ch );
   send_to_char( "Syntax: warn site  <field>\r\n", ch );
   send_to_char( "Field is either #(ban_number) or the site/class/race.\r\n", ch );
   send_to_char( "Example:  warn class #1\r\n", ch );
   return;
}

/*
 *  This actually puts the new ban into the proper linked list and
 *  initializes its data.  Shaddai
 */
int add_ban( CHAR_DATA * ch, const char *arg1, const char *arg2, int btime, int type )
{
   char arg[MAX_STRING_LENGTH];
   char buf[MAX_STRING_LENGTH];
   BAN_DATA *pban, *temp;
   struct tm *tms;
   char *name;
   int level, i, value;

   /*
    * Should we check to see if they have dropped link sometime in between 
    * * writing the note and now?  Not sure but for right now we won't since
    * * do_ban checks for that.  Shaddai
    */

   switch ( ch->substate )
   {
      default:
         bug( "%s", "add_ban: illegal substate" );
         return 0;

      case SUB_RESTRICTED:
         send_to_char( "You cannot use this command from within another command.\r\n", ch );
         return 0;

      case SUB_NONE:
      {
         one_argument( arg1, arg );
         smash_tilde( arg );  /* Make sure the immortals don't put a ~ in it. */

         if( arg[0] == '\0' || arg2[0] == '\0' )
            return 0;

         if( is_number( arg2 ) )
         {
            level = atoi( arg2 );
            if( level < 0 || level > LEVEL_SUPREME )
            {
               ch_printf( ch, "Level range is from 0 to %d.\r\n", LEVEL_SUPREME );
               return 0;
            }
         }
         else if( !str_cmp( arg2, "all" ) )
            level = LEVEL_SUPREME;
         else if( !str_cmp( arg2, "newbie" ) )
            level = 1;
         else if( !str_cmp( arg2, "mortal" ) )
            level = LEVEL_AVATAR;
         else if( !str_cmp( arg2, "warn" ) )
            level = BAN_WARN;
         else
         {
            bug( "%s", "Bad string for flag in add_ban." );
            return 0;
         }

         switch ( type )
         {
            case BAN_CLASS:
               if( arg[0] == '\0' )
                  return 0;
               if( is_number( arg ) )
                  value = atoi( arg );
               else
               {
                  for( i = 0; i < MAX_CLASS; i++ )
                     if( !str_cmp( class_table[i]->who_name, arg ) )
                        break;
                  value = i;
               }
               if( value < 0 || value >= MAX_CLASS )
               {
                  send_to_char( "Unknown class.\r\n", ch );
                  return 0;
               }
               for( temp = first_ban_class; temp; temp = temp->next )
               {
                  if( temp->flag == value )
                  {
                     if( temp->level == level )
                     {
                        send_to_char( "That entry already exists.\r\n", ch );
                        return 0;
                     }
                     else
                     {
                        temp->level = level;
                        if( temp->level == BAN_WARN )
                           temp->warn = TRUE;
                        snprintf( buf, MAX_STRING_LENGTH, "%24.24s", ctime( &current_time ) );
                        temp->ban_time = str_dup( buf );
                        if( btime > 0 )
                        {
                           temp->duration = btime;
                           tms = localtime( &current_time );
                           tms->tm_mday += btime;
                           temp->unban_date = mktime( tms );
                        }
                        else
                        {
                           temp->duration = -1;
                           temp->unban_date = -1;
                        }
                        if( temp->ban_by )
                           DISPOSE( temp->ban_by );
                        temp->ban_by = str_dup( ch->name );
                        send_to_char( "Updated entry.\r\n", ch );
                        return 1;
                     }
                  }
               }
               CREATE( pban, BAN_DATA, 1 );
               pban->name = str_dup( class_table[value]->who_name );
               pban->flag = value;
               pban->level = level;
               pban->ban_by = str_dup( ch->name );
               LINK( pban, first_ban_class, last_ban_class, next, prev );
               break;

            case BAN_RACE:
               if( is_number( arg ) )
                  value = atoi( arg );
               else
               {
                  for( i = 0; i < MAX_RACE; i++ )
                     if( !str_cmp( race_table[i]->race_name, arg ) )
                        break;
                  value = i;
               }
               if( value < 0 || value >= MAX_RACE )
               {
                  send_to_char( "Unknown race.\r\n", ch );
                  return 0;
               }
               for( temp = first_ban_race; temp; temp = temp->next )
               {
                  if( temp->flag == value )
                  {
                     if( temp->level == level )
                     {
                        send_to_char( "That entry already exists.\r\n", ch );
                        return 0;
                     }
                     else
                     {
                        temp->level = level;
                        if( temp->level == BAN_WARN )
                           temp->warn = TRUE;
                        snprintf( buf, MAX_STRING_LENGTH, "%24.24s", ctime( &current_time ) );
                        temp->ban_time = str_dup( buf );
                        if( btime > 0 )
                        {
                           temp->duration = btime;
                           tms = localtime( &current_time );
                           tms->tm_mday += btime;
                           temp->unban_date = mktime( tms );
                        }
                        else
                        {
                           temp->duration = -1;
                           temp->unban_date = -1;
                        }
                        if( temp->ban_by )
                           DISPOSE( temp->ban_by );
                        temp->ban_by = str_dup( ch->name );
                        send_to_char( "Updated entry.\r\n", ch );
                        return 1;
                     }
                  }
               }
               CREATE( pban, BAN_DATA, 1 );
               pban->name = str_dup( race_table[value]->race_name );
               pban->flag = value;
               pban->level = level;
               pban->ban_by = str_dup( ch->name );
               LINK( pban, first_ban_race, last_ban_race, next, prev );
               break;

            case BAN_SITE:
            {
               bool prefix = FALSE, suffix = FALSE, user_name = FALSE;
               char *temp_host = NULL, *temp_user = NULL;
               size_t x;

               for( x = 0; x < strlen( arg ); x++ )
               {
                  if( arg[x] == '@' )
                  {
                     user_name = TRUE;
                     temp_host = str_dup( &arg[x + 1] );
                     arg[x] = '\0';
                     temp_user = str_dup( arg );
                     break;
                  }
               }
               if( !user_name )
                  name = arg;
               else
                  name = temp_host;

               if( !name ) /* Double check to make sure name isnt null */
               {
                  /*
                   * Free this stuff if its there 
                   */
                  if( user_name )
                  {
                     DISPOSE( temp_host );
                     DISPOSE( temp_user );
                  }
                  send_to_char( "Name was null.\r\n", ch );
                  return 0;
               }

               if( name[0] == '*' )
               {
                  prefix = TRUE;
                  name++;
               }

               if( name[strlen( name ) - 1] == '*' )
               {
                  suffix = TRUE;
                  name[strlen( name ) - 1] = '\0';
               }
               for( temp = first_ban; temp; temp = temp->next )
               {
                  if( !str_cmp( temp->name, name ) )
                  {
                     if( temp->level == level && ( prefix && temp->prefix ) && ( suffix && temp->suffix ) )
                     {
                        /*
                         * Free this stuff if its there 
                         */
                        if( user_name )
                        {
                           DISPOSE( temp_host );
                           DISPOSE( temp_user );
                        }
                        send_to_char( "That entry already exists.\r\n", ch );
                        return 0;
                     }
                     else
                     {
                        temp->suffix = suffix;
                        temp->prefix = prefix;
                        if( temp->level == BAN_WARN )
                           temp->warn = TRUE;
                        temp->level = level;
                        snprintf( buf, MAX_STRING_LENGTH, "%24.24s", ctime( &current_time ) );
                        temp->ban_time = str_dup( buf );
                        if( btime > 0 )
                        {
                           temp->duration = btime;
                           tms = localtime( &current_time );
                           tms->tm_mday += btime;
                           temp->unban_date = mktime( tms );
                        }
                        else
                        {
                           temp->duration = -1;
                           temp->unban_date = -1;
                        }
                        if( temp->ban_by )
                           DISPOSE( temp->ban_by );
                        if( user_name )
                        {
                           DISPOSE( temp_host );
                           DISPOSE( temp_user );
                        }
                        temp->ban_by = str_dup( ch->name );
                        send_to_char( "Updated entry.\r\n", ch );
                        return 1;
                     }
                  }
               }
               CREATE( pban, BAN_DATA, 1 );
               pban->ban_by = str_dup( ch->name );
               pban->suffix = suffix;
               pban->prefix = prefix;
               pban->name = str_dup( name );
               pban->level = level;
               LINK( pban, first_ban, last_ban, next, prev );
               if( user_name )
               {
                  DISPOSE( temp_host );
                  DISPOSE( temp_user );
               }
               break;
            }
            default:
               bug( "Bad type in add_ban: %d.", type );
               return 0;
         }
         snprintf( buf, MAX_STRING_LENGTH, "%24.24s", ctime( &current_time ) );
         pban->ban_time = str_dup( buf );
         if( btime > 0 )
         {
            pban->duration = btime;
            tms = localtime( &current_time );
            tms->tm_mday += btime;
            pban->unban_date = mktime( tms );
         }
         else
         {
            pban->duration = -1;
            pban->unban_date = -1;
         }
         if( pban->level == BAN_WARN )
            pban->warn = TRUE;
         ch->substate = SUB_BAN_DESC;
         ch->dest_buf = pban;
         if( !pban->note )
            pban->note = STRALLOC( "" );;
         start_editing( ch, pban->note );
         return 1;
      }
      case SUB_BAN_DESC:
         pban = ( BAN_DATA * ) ch->dest_buf;
         if( !pban )
         {
            bug( "%s", "do_ban: sub_ban_desc: NULL ch->dest_buf" );
            ch->substate = SUB_NONE;
            return 0;
         }
         if( pban->note )
            STRFREE( pban->note );
         pban->note = copy_buffer( ch );
         stop_editing( ch );
         ch->substate = ch->tempnum;
         save_banlist(  );
         if( pban->duration > 0 )
         {
            ch_printf( ch, "%s banned for %d days.\r\n", pban->name, pban->duration );
         }
         else
         {
            ch_printf( ch, "%s banned forever.\r\n", pban->name );
         }
         return 1;
   }
}

/*
 * Print the bans out to the screen.  Shaddai
 */
void show_bans( CHAR_DATA * ch, int type )
{
   BAN_DATA *pban;
   int bnum;

   set_pager_color( AT_IMMORT, ch );

   switch ( type )
   {
      case BAN_SITE:
         send_to_pager( "Banned sites:\r\n", ch );
         send_to_pager( "[ #] Warn (Lv) Time                     By              For   Site\r\n", ch );
         send_to_pager( "---- ---- ---- ------------------------ --------------- ----  ---------------\r\n", ch );
         pban = first_ban;
         set_pager_color( AT_PLAIN, ch );
         for( bnum = 1; pban; pban = pban->next, bnum++ )
         {
            pager_printf( ch, "[%2d] %-4s (%2d) %-24s %-15s %4d  %c%s%c\r\n",
                          bnum, ( pban->warn ) ? "YES" : "no", pban->level,
                          pban->ban_time, pban->ban_by, pban->duration,
                          ( pban->prefix ) ? '*' : ' ', pban->name, ( pban->suffix ) ? '*' : ' ' );
         }
         return;
      case BAN_RACE:
         send_to_pager( "Banned races:\r\n", ch );
         send_to_pager( "[ #] Warn (Lv) Time                     By              For   Race\r\n", ch );
         pban = first_ban_race;
         break;
      case BAN_CLASS:
         send_to_pager( "Banned classes:\r\n", ch );
         send_to_pager( "[ #] Warn (Lv) Time                     By              For   Class\r\n", ch );
         pban = first_ban_class;
         break;
      default:
         bug( "Bad type in show_bans: %d", type );
         return;
   }
   send_to_pager( "---- ---- ---- ------------------------ --------------- ----  ---------------\r\n", ch );
   set_pager_color( AT_PLAIN, ch );
   for( bnum = 1; pban; pban = pban->next, bnum++ )
      pager_printf( ch, "[%2d] %-4s (%2d) %-24s %-15s %4d  %s\r\n", bnum,
                    ( pban->warn ) ? "YES" : "no", pban->level, pban->ban_time, pban->ban_by, pban->duration, pban->name );
   return;
}

/*
 * Check for totally banned sites.  Need this because we don't have a
 * char struct yet.  Shaddai
 */

bool check_total_bans( DESCRIPTOR_DATA * d )
{
   BAN_DATA *pban;
   char new_host[MAX_STRING_LENGTH];
   int i;

   for( i = 0; i < ( int )strlen( d->host ); i++ )
      new_host[i] = LOWER( d->host[i] );
   new_host[i] = '\0';

   for( pban = first_ban; pban; pban = pban->next )
   {
      if( pban->level != LEVEL_SUPREME )
         continue;
      if( pban->prefix && pban->suffix && strstr( new_host, pban->name ) )
      {
         if( check_expire( pban ) )
         {
            dispose_ban( pban, BAN_SITE );
            save_banlist(  );
            return FALSE;
         }
         else
            return TRUE;
      }
      /*
       *   Bug of switched checks noticed by Cronel
       */
      if( pban->suffix && !str_prefix( pban->name, new_host ) )
      {
         if( check_expire( pban ) )
         {
            dispose_ban( pban, BAN_SITE );
            save_banlist(  );
            return FALSE;
         }
         else
            return TRUE;
      }
      if( pban->prefix && !str_suffix( pban->name, new_host ) )
      {
         if( check_expire( pban ) )
         {
            dispose_ban( pban, BAN_SITE );
            save_banlist(  );
            return FALSE;
         }
         else
            return TRUE;
      }
      if( !str_cmp( pban->name, new_host ) )
      {
         if( check_expire( pban ) )
         {
            dispose_ban( pban, BAN_SITE );
            save_banlist(  );
            return FALSE;
         }
         else
            return TRUE;
      }
   }
   return FALSE;
}

/*
 * The workhose, checks for bans on sites/classes and races. Shaddai
 */

bool check_bans( CHAR_DATA * ch, int type )
{
   BAN_DATA *pban;
   char new_host[MAX_STRING_LENGTH];
   int i;
   bool fMatch = FALSE;

   switch ( type )
   {
      case BAN_RACE:
         pban = first_ban_race;
         break;
      case BAN_CLASS:
         pban = first_ban_class;
         break;
      case BAN_SITE:
         pban = first_ban;
         for( i = 0; i < ( int )( strlen( ch->desc->host ) ); i++ )
            new_host[i] = LOWER( ch->desc->host[i] );
         new_host[i] = '\0';
         break;
      default:
         bug( "Ban type in check_bans: %d.", type );
         return FALSE;
   }
   for( ; pban; pban = pban->next )
   {
      if( type == BAN_CLASS && pban->flag == ch->Class )
      {
         if( check_expire( pban ) )
         {
            dispose_ban( pban, BAN_CLASS );
            save_banlist(  );
            return FALSE;
         }
         if( ch->level > pban->level )
         {
            if( pban->warn )
            {
               log_printf_plus( LOG_WARN, sysdata.log_level, "%s class logging in from %s.", pban->name, ch->desc->host );
            }
            return FALSE;
         }
         else
            return TRUE;
      }
      if( type == BAN_RACE && pban->flag == ch->race )
      {
         if( check_expire( pban ) )
         {
            dispose_ban( pban, BAN_RACE );
            save_banlist(  );
            return FALSE;
         }
         if( ch->level > pban->level )
         {
            if( pban->warn )
            {
               log_printf_plus( LOG_WARN, sysdata.log_level, "%s race logging in from %s.", pban->name, ch->desc->host );
            }
            return FALSE;
         }
         else
            return TRUE;
      }
      if( type == BAN_SITE )
      {
         if( pban->prefix && pban->suffix && strstr( new_host, pban->name ) )
            fMatch = TRUE;
         else if( pban->prefix && !str_suffix( pban->name, new_host ) )
            fMatch = TRUE;
         else if( pban->suffix && !str_prefix( pban->name, new_host ) )
            fMatch = TRUE;
         else if( !str_cmp( pban->name, new_host ) )
            fMatch = TRUE;
         if( fMatch )
         {
            if( check_expire( pban ) )
            {
               dispose_ban( pban, BAN_SITE );
               save_banlist(  );
               return FALSE;
            }
            if( ch->level > pban->level )
            {
               if( pban->warn )
               {
                  log_printf_plus( LOG_WARN, sysdata.log_level, "%s logging in from site %s.", ch->name, ch->desc->host );
               }
               return FALSE;
            }
            else
               return TRUE;
         }
      }
   }
   return FALSE;
}

bool check_expire( BAN_DATA * pban )
{
   if( pban->unban_date < 0 )
      return FALSE;
   if( pban->unban_date <= current_time )
   {
      log_printf_plus( LOG_WARN, sysdata.log_level, "%s ban has expired.", pban->name );
      return TRUE;
   }
   return FALSE;
}

void dispose_ban( BAN_DATA * pban, int type )
{
   if( !pban )
      return;

   if( type != BAN_SITE && type != BAN_CLASS && type != BAN_RACE )
   {
      bug( "%s: Unknown Ban Type %d.", __FUNCTION__, type );
      return;
   }

   switch ( type )
   {
      case BAN_SITE:
         UNLINK( pban, first_ban, last_ban, next, prev );
         break;
      case BAN_CLASS:
         UNLINK( pban, first_ban_class, last_ban_class, next, prev );
         break;
      case BAN_RACE:
         UNLINK( pban, first_ban_race, last_ban_race, next, prev );
         break;
   }
   free_ban( pban );
}

void free_ban( BAN_DATA * pban )
{
   DISPOSE( pban->name );
   DISPOSE( pban->ban_time );
   STRFREE( pban->note );
   DISPOSE( pban->ban_by );
   DISPOSE( pban );
}

void free_bans( void )
{
   BAN_DATA *ban, *ban_next;

   for( ban = first_ban; ban; ban = ban_next )
   {
      ban_next = ban->next;
      dispose_ban( ban, BAN_SITE );
   }
   for( ban = first_ban_race; ban; ban = ban_next )
   {
      ban_next = ban->next;
      dispose_ban( ban, BAN_RACE );
   }
   for( ban = first_ban_class; ban; ban = ban_next )
   {
      ban_next = ban->next;
      dispose_ban( ban, BAN_CLASS );
   }
   return;
}
