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
 *                          Special clan module                             *
 ****************************************************************************/

#include <stdio.h>
#include <time.h>
#include "mud.h"

static OBJ_DATA *rgObjNest[MAX_NEST];

CLAN_DATA *first_clan;
CLAN_DATA *last_clan;
COUNCIL_DATA *first_council;
COUNCIL_DATA *last_council;
VAULT_DATA *first_vault;
VAULT_DATA *last_vault; 

/* local routines */
void fread_clan( CLAN_DATA * clan, FILE * fp );
bool load_clan_file( const char *clanfile );
void write_clan_list( void );
void fread_council( COUNCIL_DATA * council, FILE * fp );
bool load_council_file( const char *councilfile );
void write_council_list( void );
bool fread_storage( int rnum, const char *filename );

void add_roster( CLAN_DATA * clan, const char *name, int Class, int level, int kills, int deaths )
{
   ROSTER_DATA *roster;

   CREATE( roster, ROSTER_DATA, 1 );
   roster->name = STRALLOC( name );
   roster->Class = Class;
   roster->level = level;
   roster->kills = kills;
   roster->deaths = deaths;
   roster->joined = current_time;
   LINK( roster, clan->first_member, clan->last_member, next, prev );
   return;
}

void remove_roster( CLAN_DATA * clan, const char *name )
{
   ROSTER_DATA *roster;

   for( roster = clan->first_member; roster; roster = roster->next )
   {
      if( !str_cmp( name, roster->name ) )
      {
         STRFREE( roster->name );
         UNLINK( roster, clan->first_member, clan->last_member, next, prev );
         DISPOSE( roster );
         return;
      }
   }
}

void update_roster( CHAR_DATA * ch )
{
   ROSTER_DATA *roster;

   if( IS_NPC( ch ) || !ch->pcdata->clan )
      return;

   for( roster = ch->pcdata->clan->first_member; roster; roster = roster->next )
   {
      if( !str_cmp( ch->name, roster->name ) )
      {
         roster->level = ch->level;
         roster->kills = ch->pcdata->mkills;
         roster->deaths = ch->pcdata->mdeaths;
         save_clan( ch->pcdata->clan );
         return;
      }
   }

   /*
    * If we make it here, assume they haven't been added previously 
    */
   add_roster( ch->pcdata->clan, ch->name, ch->Class, ch->level, ch->pcdata->mkills, ch->pcdata->mdeaths );
   save_clan( ch->pcdata->clan );
   return;
}

/* For use during clan removal and memory cleanup */
void remove_all_rosters( CLAN_DATA * clan )
{
   ROSTER_DATA *roster, *roster_next;

   for( roster = clan->first_member; roster; roster = roster_next )
   {
      roster_next = roster->next;

      STRFREE( roster->name );
      UNLINK( roster, clan->first_member, clan->last_member, next, prev );
      DISPOSE( roster );
   }
}

void do_roster( CHAR_DATA* ch, const char* argument)
{
   CLAN_DATA *clan;
   ROSTER_DATA *roster;
   char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
   int total = 0;

   if( IS_NPC( ch ) )
   {
      send_to_char( "NPCs can't use this command.\r\n", ch );
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: roster <clanname>\r\n", ch );
      send_to_char( "Usage: roster <clanname> remove <name>\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );
   if( !( clan = get_clan( arg ) ) )
   {
      ch_printf( ch, "No such guild or clan known as %s\r\n", arg );
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      ch_printf( ch, "Membership roster for the %s %s\r\n\r\n", clan->name,
                 clan->clan_type == CLAN_ORDER ? "Guild" : "Clan" );
      ch_printf( ch, "%-15.15s  %-15.15s %-6.6s %-6.6s %-6.6s %s\r\n", "Name", "Class", "Level", "Kills", "Deaths",
                 "Joined on" );
      send_to_char( "-------------------------------------------------------------------------------------\r\n", ch );
      for( roster = clan->first_member; roster; roster = roster->next )
      {
         ch_printf( ch, "%-15.15s  %-15.15s %-6d %-6d %-6d %s",
                    roster->name, capitalize( npc_class[roster->Class] ), roster->level, roster->kills, roster->deaths,
                    ctime( &roster->joined ) );
         total++;
      }
      ch_printf( ch, "\r\nThere are %d member%s in %s\r\n", total, total == 1 ? "" : "s", clan->name );
      return;
   }

   argument = one_argument( argument, arg2 );
   if( !str_cmp( arg2, "remove" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Remove who from the roster?\r\n", ch );
         return;
      }
      remove_roster( clan, argument );
      save_clan( clan );
      ch_printf( ch, "%s has been removed from the roster for %s\r\n", argument, clan->name );
      return;
   }
   do_roster( ch, "" );
   return;
}

void fwrite_memberlist( FILE * fp, CLAN_DATA * clan )
{
   ROSTER_DATA *roster;

   for( roster = clan->first_member; roster; roster = roster->next )
   {
      fprintf( fp, "%s", "#ROSTER\n" );
      fprintf( fp, "Name      %s~\n", roster->name );
      fprintf( fp, "Joined    %ld\n", ( time_t ) roster->joined );
      fprintf( fp, "Class     %s~\n", npc_class[roster->Class] );
      fprintf( fp, "Level     %d\n", roster->level );
      fprintf( fp, "Kills     %d\n", roster->kills );
      fprintf( fp, "Deaths    %d\n", roster->deaths );
      fprintf( fp, "%s", "End\n\n" );
   }
   return;
}

void fread_memberlist( CLAN_DATA * clan, FILE * fp )
{
   ROSTER_DATA *roster;
   const char *word;
   bool fMatch;

   CREATE( roster, ROSTER_DATA, 1 );

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'C':
            if( !str_cmp( word, "Class" ) )
            {
               const char *temp = fread_string( fp );
               int Class = get_npc_class( temp );

               if( Class < 0 || Class >= MAX_NPC_CLASS )
               {
                  bug( "%s: Invalid class in clan roster", __FUNCTION__ );
                  Class = get_npc_class( "warrior" );
               }
               STRFREE( temp );
               roster->Class = Class;
               fMatch = TRUE;
               break;
            }
            break;

         case 'D':
            KEY( "Deaths", roster->deaths, fread_number( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               LINK( roster, clan->first_member, clan->last_member, next, prev );
               return;
            }
            break;

         case 'J':
            KEY( "Joined", roster->joined, fread_number( fp ) );
            break;

         case 'K':
            KEY( "Kills", roster->kills, fread_number( fp ) );
            break;

         case 'L':
            KEY( "Level", roster->level, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", roster->name, fread_string( fp ) );
            break;
      }
      if( !fMatch )
         bug( "%s: no match: %s", __FUNCTION__, word );
   }
}

void free_one_clan( CLAN_DATA * clan )
{
   UNLINK( clan, first_clan, last_clan, next, prev );
   DISPOSE( clan->filename );
   STRFREE( clan->name );
   STRFREE( clan->motto );
   STRFREE( clan->description );
   STRFREE( clan->deity );
   STRFREE( clan->leader );
   STRFREE( clan->number1 );
   STRFREE( clan->number2 );
   STRFREE( clan->leadrank );
   STRFREE( clan->onerank );
   STRFREE( clan->tworank );
   STRFREE( clan->badge );
   DISPOSE( clan );

   return;
}

void free_clans( void )
{
   CLAN_DATA *clan, *clan_next;

   for( clan = first_clan; clan; clan = clan_next )
   {
      clan_next = clan->next;
      free_one_clan( clan );
   }
   return;
}

void free_one_council( COUNCIL_DATA * council )
{
   UNLINK( council, first_council, last_council, next, prev );
   STRFREE( council->description );
   DISPOSE( council->filename );
   STRFREE( council->head );
   STRFREE( council->head2 );
   STRFREE( council->name );
   STRFREE( council->powers );
   DISPOSE( council );
   return;
}

void free_councils( void )
{
   COUNCIL_DATA *council, *council_next;

   for( council = first_council; council; council = council_next )
   {
      council_next = council->next;
      free_one_council( council );
   }
   return;
}

/*
 * Get pointer to clan structure from clan name.
 */
CLAN_DATA *get_clan( const char *name )
{
   CLAN_DATA *clan;

   for( clan = first_clan; clan; clan = clan->next )
      if( !str_cmp( name, clan->name ) || ( ( clan->abbrev != '\0' ) && !str_cmp( name, clan->abbrev ) ) )
         return clan;
   return NULL;
}

COUNCIL_DATA *get_council( const char *name )
{
   COUNCIL_DATA *council;

   for( council = first_council; council; council = council->next )
      if( !str_cmp( name, council->name ) || ( ( council->abbrev != '\0' ) && !str_cmp( name, council->abbrev ) ) )
         return council;
   return NULL;
}

void write_clan_list(  )
{
   CLAN_DATA *tclan;
   FILE *fpout;
   char filename[256];

   snprintf( filename, 256, "%s%s", CLAN_DIR, CLAN_LIST );
   fpout = fopen( filename, "w" );
   if( !fpout )
   {
      bug( "FATAL: cannot open %s for writing!\r\n", filename );
      return;
   }
   for( tclan = first_clan; tclan; tclan = tclan->next )
      fprintf( fpout, "%s\n", tclan->filename );
   fprintf( fpout, "$\n" );
   fclose( fpout );
}

void write_council_list(  )
{
   COUNCIL_DATA *tcouncil;
   FILE *fpout;
   char filename[256];

   snprintf( filename, 256, "%s%s", COUNCIL_DIR, COUNCIL_LIST );
   fpout = fopen( filename, "w" );
   if( !fpout )
   {
      bug( "FATAL: cannot open %s for writing!\r\n", filename );
      return;
   }
   for( tcouncil = first_council; tcouncil; tcouncil = tcouncil->next )
      fprintf( fpout, "%s\n", tcouncil->filename );
   fprintf( fpout, "$\n" );
   fclose( fpout );
}

/*
 * Save a clan's data to its data file
 */
void save_clan( CLAN_DATA * clan )
{
   FILE *fp;
   char filename[256];

   if( !clan )
   {
      bug( "%s", "save_clan: null clan pointer!" );
      return;
   }

   if( !clan->filename || clan->filename[0] == '\0' )
   {
      bug( "%s: %s has no filename", __FUNCTION__, clan->name );
      return;
   }

   snprintf( filename, 256, "%s%s", CLAN_DIR, clan->filename );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "save_clan: cant open %s", filename );
      perror( filename );
   }
   else
   {
      fprintf( fp, "#CLAN\n" );
      fprintf( fp, "Name         %s~\n", clan->name );
      fprintf( fp, "Abbrev       %s~\n", clan->abbrev );
      fprintf( fp, "Filename     %s~\n", clan->filename );
      fprintf( fp, "Motto        %s~\n", clan->motto );
      fprintf( fp, "Description  %s~\n", clan->description );
      fprintf( fp, "Deity        %s~\n", clan->deity );
      fprintf( fp, "Leader       %s~\n", clan->leader );
      fprintf( fp, "NumberOne    %s~\n", clan->number1 );
      fprintf( fp, "NumberTwo    %s~\n", clan->number2 );
      fprintf( fp, "Badge        %s~\n", clan->badge );
      fprintf( fp, "Leadrank     %s~\n", clan->leadrank );
      fprintf( fp, "Onerank      %s~\n", clan->onerank );
      fprintf( fp, "Tworank      %s~\n", clan->tworank );
      fprintf( fp, "PKillRangeNew   %d %d %d %d %d %d %d\n",
               clan->pkills[0], clan->pkills[1], clan->pkills[2],
               clan->pkills[3], clan->pkills[4], clan->pkills[5], clan->pkills[6] );
      fprintf( fp, "PDeathRangeNew  %d %d %d %d %d %d %d\n",
               clan->pdeaths[0], clan->pdeaths[1], clan->pdeaths[2],
               clan->pdeaths[3], clan->pdeaths[4], clan->pdeaths[5], clan->pdeaths[6] );
      fprintf( fp, "MKills       %d\n", clan->mkills );
      fprintf( fp, "MDeaths      %d\n", clan->mdeaths );
      fprintf( fp, "IllegalPK    %d\n", clan->illegal_pk );
      fprintf( fp, "Score        %d\n", clan->score );
      fprintf( fp, "Type         %d\n", clan->clan_type );
      fprintf( fp, "Class        %d\n", clan->Class );
      fprintf( fp, "Favour       %d\n", clan->favour );
      fprintf( fp, "Strikes      %d\n", clan->strikes );
      fprintf( fp, "Members      %d\n", clan->members );
      fprintf( fp, "MemLimit     %d\n", clan->mem_limit );
      fprintf( fp, "Alignment    %d\n", clan->alignment );
      fprintf( fp, "Board        %d\n", clan->board );
      fprintf( fp, "ClanObjOne   %d\n", clan->clanobj1 );
      fprintf( fp, "ClanObjTwo   %d\n", clan->clanobj2 );
      fprintf( fp, "ClanObjThree %d\n", clan->clanobj3 );
      fprintf( fp, "ClanObjFour  %d\n", clan->clanobj4 );
      fprintf( fp, "ClanObjFive  %d\n", clan->clanobj5 );
      fprintf( fp, "Recall       %d\n", clan->recall );
      fprintf( fp, "Storeroom    %d\n", clan->storeroom );
      fprintf( fp, "GuardOne     %d\n", clan->guard1 );
      fprintf( fp, "GuardTwo     %d\n", clan->guard2 );
      fprintf( fp, "%s", "End\n\n" );

      fwrite_memberlist( fp, clan );
      fprintf( fp, "%s", "#END\n" );

      fclose( fp );
      fp = NULL;
   }
   return;
}

/*
 * Save a council's data to its data file
 */
void save_council( COUNCIL_DATA * council )
{
   FILE *fp;
   char filename[256];

   if( !council )
   {
      bug( "%s", "save_council: null council pointer!" );
      return;
   }

   if( !council->filename || council->filename[0] == '\0' )
   {
      bug( "save_council: %s has no filename", council->name );
      return;
   }

   snprintf( filename, 256, "%s%s", COUNCIL_DIR, council->filename );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "save_council: cant open %s", filename );
      perror( filename );
   }
   else
   {

      fprintf( fp, "#COUNCIL\n" );
      if( council->name )
         fprintf( fp, "Name         %s~\n", council->name );
      fprintf( fp, "Abbrev       %s~\n", council->abbrev );
      if( council->filename )
         fprintf( fp, "Filename     %s~\n", council->filename );
      if( council->description )
         fprintf( fp, "Description  %s~\n", council->description );
      if( council->head )
         fprintf( fp, "Head         %s~\n", council->head );
      if( council->head2 != NULL )
         fprintf( fp, "Head2        %s~\n", council->head2 );
      fprintf( fp, "Members      %d\n", council->members );
      fprintf( fp, "Board        %d\n", council->board );
      fprintf( fp, "Meeting      %d\n", council->meeting );
      fprintf( fp, "Storeroom    %d\n", council->storeroom );
      if( council->powers )
         fprintf( fp, "Powers       %s~\n", council->powers );
      fprintf( fp, "End\n\n" );
      fprintf( fp, "#END\n" );
      fclose( fp );
      fp = NULL;
   }
   return;
}

void sort_vaults( VAULT_DATA *pVault )
{
   VAULT_DATA *vault = NULL;

   if( !pVault )
   {
      bug( "%s: NULL pVault", __FUNCTION__ );
      return;
   }

   pVault->next = NULL;
   pVault->prev = NULL;
   for( vault = first_vault; vault; vault = vault->next )
   {
      if( vault->vnum == pVault->vnum )
         break;
      if( vault->vnum > pVault->vnum )
      {
         INSERT( pVault, vault, first_vault, next, prev );
         break;
      }
   }

   if( !vault )
      LINK( pVault, first_vault, last_vault, next, prev );
   return;
}                                                                                                                                               

void save_vault_list( )
{
   VAULT_DATA *vault;
   FILE *fpout;
   char filename[256];

   snprintf( filename, 256, "%s%s", VAULT_DIR, VAULT_LIST );
   fpout = fopen( filename, "w" );
   if( !fpout )
   {
      bug( "%s: FATAL: cannot open vault.lst for writing!", __FUNCTION__ );
      return;
   }

   for( vault = first_vault; vault; vault = vault->next )
      fprintf( fpout, "%d\n", vault->vnum );
   fprintf( fpout, "$\n" );
   fclose( fpout );
   fpout = NULL;
}                                                                                                                                               

void load_vaults( )
{
   FILE *fpList;
   char filename[256];
   const char *vnum;
   char vaultlist[256];
   int rnum;

   snprintf( vaultlist, 256, "%s%s", VAULT_DIR, VAULT_LIST );
   if( ( fpList = fopen( vaultlist, "r" ) ) == NULL )
   {
      perror( vaultlist );
      exit( 1 );
   }

   first_vault = NULL;
   last_vault = NULL;

   for( ; ; )
   {
      vnum = feof( fpList ) ? "$" : fread_word( fpList );
      log_string( vnum );

      if( vnum[0] == '$' )
         break;

      if( ( rnum = atoi(vnum) ) == 0 )
      {
         bug( "%s: Invalid vault number: %d", __FUNCTION__, rnum );
         break;
      }

      snprintf( filename, 256, "%s%s.vault", VAULT_DIR, vnum );

      if( !fread_storage( rnum, filename ) )
      {
         bug( "%s: Cannot load vault file: %s", __FUNCTION__, filename );
      }
   }

   fclose( fpList );
   fpList = NULL;

   log_string(" Done vaults " );
   return;
}                                                                                                                                               

bool fread_storage( int rnum, const char *filename )
{
   FILE *fp;
   bool found = FALSE;
   VAULT_DATA *vault;
   ROOM_INDEX_DATA *storeroom;

   CREATE( vault, VAULT_DATA, 1 );

   if( ( vault->vnum = rnum ) == 0 || ( storeroom = get_room_index( rnum ) ) == NULL )
   {
      log_string( "Storeroom not found" );
      DISPOSE( vault ); // Memory leak, fixed by SmaugFUSS
      return found;
   }

   sort_vaults( vault );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {
      int iNest;
      OBJ_DATA *tobj, *tobj_next;

      log_printf( "Loading vault file - %s", filename );

      rset_supermob( storeroom );

      for( iNest = 0; iNest < MAX_NEST; iNest++ )
         rgObjNest[iNest] = NULL;

      found = TRUE;
      for( ; ; )
      {
         char letter;
         const char *word;

         letter = fread_letter( fp );
         if ( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "%s: %s: # not found.", __FUNCTION__, filename );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "OBJECT" ) )  /* Objects */
         {
            fread_obj( supermob, fp, OS_VAULT );
         }
         else if( !str_cmp( word, "END" ) )  /* Done */
            break;
         else
         {
            bug( "%s: %s: bad section.", __FUNCTION__, filename );
            break;
         }
      }
      fclose( fp );
      fp = NULL;

      for( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
      {
         tobj_next = tobj->next_content;

         obj_from_char( tobj );
         obj_to_room( tobj, storeroom );
      }
      release_supermob();
   }
   else
   {
      log_printf( "Cannot open vault - %s", filename );
   }
   return found;
}

/*
 * Read in actual clan data.
 * Reads in PKill and PDeath still for backward compatibility but now it
 * should be written to PKillRange and PDeathRange for multiple level pkill
 * tracking support. --Shaddai
 * Added a hardcoded limit memlimit to the amount of members a clan can 
 * have set using setclan.  --Shaddai
 */
void fread_clan( CLAN_DATA * clan, FILE * fp )
{
   const char *word;
   bool fMatch;

   clan->mem_limit = 0; /* Set up defaults */
   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            KEY( "Abbrev", clan->abbrev, fread_string( fp ) );
            KEY( "Alignment", clan->alignment, fread_number( fp ) );
            break;

         case 'B':
            KEY( "Badge", clan->badge, fread_string( fp ) );
            KEY( "Board", clan->board, fread_number( fp ) );
            break;

         case 'C':
            KEY( "ClanObjOne", clan->clanobj1, fread_number( fp ) );
            KEY( "ClanObjTwo", clan->clanobj2, fread_number( fp ) );
            KEY( "ClanObjThree", clan->clanobj3, fread_number( fp ) );
            KEY( "ClanObjFour", clan->clanobj4, fread_number( fp ) );
            KEY( "ClanObjFive", clan->clanobj5, fread_number( fp ) );
            KEY( "Class", clan->Class, fread_number( fp ) );
            break;

         case 'D':
            KEY( "Deity", clan->deity, fread_string( fp ) );
            KEY( "Description", clan->description, fread_string( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !clan->name )
                  clan->name = STRALLOC( "" );
               if( !clan->leader )
                  clan->leader = STRALLOC( "" );
               if( !clan->description )
                  clan->description = STRALLOC( "" );
               if( !clan->motto )
                  clan->motto = STRALLOC( "" );
               if( !clan->number1 )
                  clan->number1 = STRALLOC( "" );
               if( !clan->number2 )
                  clan->number2 = STRALLOC( "" );
               if( !clan->deity )
                  clan->deity = STRALLOC( "" );
               if( !clan->badge )
                  clan->badge = STRALLOC( "" );
               if( !clan->leadrank )
                  clan->leadrank = STRALLOC( "" );
               if( !clan->onerank )
                  clan->onerank = STRALLOC( "" );
               if( !clan->tworank )
                  clan->tworank = STRALLOC( "" );
               return;
            }
            break;

         case 'F':
            KEY( "Favour", clan->favour, fread_number( fp ) );
            KEY( "Filename", clan->filename, fread_string_nohash( fp ) );

         case 'G':
            KEY( "GuardOne", clan->guard1, fread_number( fp ) );
            KEY( "GuardTwo", clan->guard2, fread_number( fp ) );
            break;

         case 'I':
            KEY( "IllegalPK", clan->illegal_pk, fread_number( fp ) );
            break;

         case 'L':
            KEY( "Leader", clan->leader, fread_string( fp ) );
            KEY( "Leadrank", clan->leadrank, fread_string( fp ) );
            break;

         case 'M':
            KEY( "MDeaths", clan->mdeaths, fread_number( fp ) );
            KEY( "Members", clan->members, fread_number( fp ) );
            KEY( "MemLimit", clan->mem_limit, fread_number( fp ) );
            KEY( "MKills", clan->mkills, fread_number( fp ) );
            KEY( "Motto", clan->motto, fread_string( fp ) );
            break;

         case 'N':
            KEY( "Name", clan->name, fread_string( fp ) );
            KEY( "NumberOne", clan->number1, fread_string( fp ) );
            KEY( "NumberTwo", clan->number2, fread_string( fp ) );
            break;

         case 'O':
            KEY( "Onerank", clan->onerank, fread_string( fp ) );
            break;

         case 'P':
            KEY( "PDeaths", clan->pdeaths[6], fread_number( fp ) );
            KEY( "PKills", clan->pkills[6], fread_number( fp ) );
            /*
             * Addition of New Ranges 
             */
            if( !str_cmp( word, "PDeathRange" ) )
            {
               fMatch = TRUE;
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
            }
            if( !str_cmp( word, "PDeathRangeNew" ) )
            {
               fMatch = TRUE;
               clan->pdeaths[0] = fread_number( fp );
               clan->pdeaths[1] = fread_number( fp );
               clan->pdeaths[2] = fread_number( fp );
               clan->pdeaths[3] = fread_number( fp );
               clan->pdeaths[4] = fread_number( fp );
               clan->pdeaths[5] = fread_number( fp );
               clan->pdeaths[6] = fread_number( fp );
            }
            if( !str_cmp( word, "PKillRangeNew" ) )
            {
               fMatch = TRUE;
               clan->pkills[0] = fread_number( fp );
               clan->pkills[1] = fread_number( fp );
               clan->pkills[2] = fread_number( fp );
               clan->pkills[3] = fread_number( fp );
               clan->pkills[4] = fread_number( fp );
               clan->pkills[5] = fread_number( fp );
               clan->pkills[6] = fread_number( fp );
            }
            if( !str_cmp( word, "PKillRange" ) )
            {
               fMatch = TRUE;
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
               fread_number( fp );
            }
            break;

         case 'R':
            KEY( "Recall", clan->recall, fread_number( fp ) );
            break;

         case 'S':
            KEY( "Score", clan->score, fread_number( fp ) );
            KEY( "Strikes", clan->strikes, fread_number( fp ) );
            KEY( "Storeroom", clan->storeroom, fread_number( fp ) );
            break;

         case 'T':
            KEY( "Tworank", clan->tworank, fread_string( fp ) );
            KEY( "Type", clan->clan_type, fread_number( fp ) );
            break;
      }

      if( !fMatch )
      {
         bug( "Fread_clan: no match: %s", word );
         fread_to_eol( fp );
      }
   }
}

/*
 * Read in actual council data.
 */
void fread_council( COUNCIL_DATA * council, FILE * fp )
{
   const char *word;
   bool fMatch;

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            KEY( "Abbrev", council->abbrev, fread_string( fp ) );
            break;

         case 'B':
            KEY( "Board", council->board, fread_number( fp ) );
            break;

         case 'D':
            KEY( "Description", council->description, fread_string( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !council->name )
                  council->name = STRALLOC( "" );
               if( !council->description )
                  council->description = STRALLOC( "" );
               if( !council->powers )
                  council->powers = STRALLOC( "" );
               return;
            }
            break;

         case 'F':
            KEY( "Filename", council->filename, fread_string_nohash( fp ) );
            break;

         case 'H':
            KEY( "Head", council->head, fread_string( fp ) );
            KEY( "Head2", council->head2, fread_string( fp ) );
            break;

         case 'M':
            KEY( "Members", council->members, fread_number( fp ) );
            KEY( "Meeting", council->meeting, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", council->name, fread_string( fp ) );
            break;

         case 'P':
            KEY( "Powers", council->powers, fread_string( fp ) );
            break;

         case 'S':
            KEY( "Storeroom", council->storeroom, fread_number( fp ) );
            break;
      }

      if( !fMatch )
      {
         bug( "Fread_council: no match: %s", word );
         fread_to_eol( fp );
      }
   }
}

/*
 * Load a clan file
 */
bool load_clan_file( const char *clanfile )
{
   char filename[256];
   CLAN_DATA *clan;
   FILE *fp;
   bool found;

   CREATE( clan, CLAN_DATA, 1 );

   /*
    * Make sure we default these to 0 --Shaddai 
    */
   clan->pkills[0] = 0;
   clan->pkills[1] = 0;
   clan->pkills[2] = 0;
   clan->pkills[3] = 0;
   clan->pkills[4] = 0;
   clan->pkills[5] = 0;
   clan->pkills[6] = 0;
   clan->pdeaths[0] = 0;
   clan->pdeaths[1] = 0;
   clan->pdeaths[2] = 0;
   clan->pdeaths[3] = 0;
   clan->pdeaths[4] = 0;
   clan->pdeaths[5] = 0;
   clan->pdeaths[6] = 0;

   found = FALSE;
   snprintf( filename, 256, "%s%s", CLAN_DIR, clanfile );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {
      found = TRUE;
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "%s: # not found.", __FUNCTION__ );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "CLAN" ) )
            fread_clan( clan, fp );
         else if( !str_cmp( word, "ROSTER" ) )
            fread_memberlist( clan, fp );
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section: %s.", __FUNCTION__, word );
            break;
         }
      }
      fclose( fp );
      fp = NULL;
   }

   if( found )
   {
      char fname[256];
      VAULT_DATA *vault;

      LINK( clan, first_clan, last_clan, next, prev );

      for( vault = first_vault; vault; vault = vault->next )
         if( clan->storeroom == vault->vnum )
            return found;

      snprintf( fname, 256, "%s%s.vault", CLAN_DIR, clan->filename );
      fread_storage( clan->storeroom, fname );
   }
   else
      DISPOSE( clan );

   return found;
}

/*
 * Load a council file
 */
bool load_council_file( const char *councilfile )
{
   char filename[256];
   COUNCIL_DATA *council;
   FILE *fp;
   bool found;

   CREATE( council, COUNCIL_DATA, 1 );

   found = FALSE;
   snprintf( filename, 256, "%s%s", COUNCIL_DIR, councilfile );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {

      found = TRUE;
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "%s: # not found.", __FUNCTION__ );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "COUNCIL" ) )
         {
            fread_council( council, fp );
            break;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section: %s", __FUNCTION__, word );
            break;
         }
      }
      fclose( fp );
   }

   if( found )
   {
      char fname[256];
      VAULT_DATA *vault;

      LINK( council, first_council, last_council, next, prev );

      for( vault = first_vault; vault; vault = vault->next )
         if( council->storeroom == vault->vnum )
            return found;

      snprintf( fname, 256, "%s%s.vault", COUNCIL_DIR, council->filename );
      fread_storage( council->storeroom, fname );
   }
   else
      DISPOSE( council );

   return found;
}

/*
 * Load in all the clan files.
 */
void load_clans(  )
{
   FILE *fpList;
   const char *filename;
   char clanlist[256];

   first_clan = NULL;
   last_clan = NULL;

   log_string( "Loading clans..." );

   snprintf( clanlist, 256, "%s%s", CLAN_DIR, CLAN_LIST );
   if( ( fpList = fopen( clanlist, "r" ) ) == NULL )
   {
      perror( clanlist );
      exit( 1 );
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      log_string( filename );
      if( filename[0] == '$' )
         break;

      if( !load_clan_file( filename ) )
      {
         bug( "%s: Cannot load clan file: %s", __FUNCTION__, filename );
      }
   }
   fclose( fpList );
   log_string( " Done clans " );
   return;
}

/*
 * Load in all the council files.
 */
void load_councils(  )
{
   FILE *fpList;
   const char *filename;
   char councillist[256];

   first_council = NULL;
   last_council = NULL;

   log_string( "Loading councils..." );

   snprintf( councillist, 256, "%s%s", COUNCIL_DIR, COUNCIL_LIST );
   if( ( fpList = fopen( councillist, "r" ) ) == NULL )
   {
      perror( councillist );
      exit( 1 );
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      log_string( filename );
      if( filename[0] == '$' )
         break;

      if( !load_council_file( filename ) )
      {
         bug( "Cannot load council file: %s", filename );
      }
   }
   fclose( fpList );
   log_string( " Done councils " );
   return;
}

/*
 * Save items in a clan storage room -Rewritten by Edmond
 */
void save_storeroom( CHAR_DATA *ch, int vnum )
{
   FILE *fp;
   char filename[256];
   short templvl;
   OBJ_DATA *contents;

   if( !vnum )
   {
      bug( "%s: Null vnum pointer!", __FUNCTION__ );
      return;
   }                                                                                                                                           

   if( !ch )
   {
      bug( "%s: Null ch pointer!", __FUNCTION__ );
      return;
   }                                                                                                                                           

   snprintf( filename, 256, "%s%d.vault", VAULT_DIR, vnum );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "%s: fopen", __FUNCTION__ );
      perror( filename );
   }
   else
   {
      templvl = ch->level;
      ch->level = LEVEL_HERO;  /* make sure EQ doesn't get lost */
      contents = ch->in_room->last_content;

      if( contents )
         fwrite_obj( ch, contents, fp, 0, OS_VAULT, FALSE );
      fprintf( fp, "#END\n" );

      ch->level = templvl;
      fclose( fp );
      fp = NULL;
      return;
   }
   return;
}

void do_make( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   CLAN_DATA *clan;
   int level;

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( str_cmp( ch->name, clan->leader )
       && str_cmp( ch->name, clan->deity ) && ( clan->clan_type != CLAN_GUILD || str_cmp( ch->name, clan->number1 ) ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Make what?\r\n", ch );
      return;
   }

   pObjIndex = get_obj_index( clan->clanobj1 );
   level = 40;

   if( !pObjIndex || !is_name( arg, pObjIndex->name ) )
   {
      pObjIndex = get_obj_index( clan->clanobj2 );
      level = 45;
   }
   if( !pObjIndex || !is_name( arg, pObjIndex->name ) )
   {
      pObjIndex = get_obj_index( clan->clanobj3 );
      level = 50;
   }
   if( !pObjIndex || !is_name( arg, pObjIndex->name ) )
   {
      pObjIndex = get_obj_index( clan->clanobj4 );
      level = 35;
   }
   if( !pObjIndex || !is_name( arg, pObjIndex->name ) )
   {
      pObjIndex = get_obj_index( clan->clanobj5 );
      level = 1;
   }

   if( !pObjIndex || !is_name( arg, pObjIndex->name ) )
   {
      send_to_char( "You don't know how to make that.\r\n", ch );
      return;
   }

   obj = create_object( pObjIndex, level );
   xSET_BIT( obj->extra_flags, ITEM_CLANOBJECT );
   if( CAN_WEAR( obj, ITEM_TAKE ) )
      obj = obj_to_char( obj, ch );
   else
      obj = obj_to_room( obj, ch->in_room );
   act( AT_MAGIC, "$n makes $p!", ch, obj, NULL, TO_ROOM );
   act( AT_MAGIC, "You make $p!", ch, obj, NULL, TO_CHAR );
   return;
}

void do_induct( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   CLAN_DATA *clan;

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( ( ch->pcdata && ch->pcdata->bestowments
         && is_name( "caninduct", ch->pcdata->bestowments ) )
       || !str_cmp( ch->name, clan->deity )
       || !str_cmp( ch->name, clan->leader ) || !str_cmp( ch->name, clan->number1 ) || !str_cmp( ch->name, clan->number2 ) )
      ;
   else
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Induct whom?\r\n", ch );
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

   if( IS_IMMORTAL( victim ) )
   {
      send_to_char( "You can't induct such a godly presence.\r\n", ch );
      return;
   }

   if( !IS_PKILL( victim ) && clan->clan_type != CLAN_GUILD &&
       clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_NOKILL )
   {
      send_to_char( "You cannot induct a peaceful character.\r\n", ch );
      return;
   }

   if( clan->clan_type == CLAN_GUILD )
   {
      if( victim->Class != clan->Class )
      {
         send_to_char( "This player's will is not in accordance with your guild.\r\n", ch );
         return;
      }
   }
   else
   {
      if( victim->level < 10 )
      {
         send_to_char( "This player is not worthy of joining yet.\r\n", ch );
         return;
      }

      if( victim->level > ch->level )
      {
         send_to_char( "This player is too powerful for you to induct.\r\n", ch );
         return;
      }
   }

   if( victim->pcdata->clan )
   {
      if( victim->pcdata->clan->clan_type == CLAN_ORDER )
      {
         if( victim->pcdata->clan == clan )
            send_to_char( "This player already belongs to your order!\r\n", ch );
         else
            send_to_char( "This player already belongs to an order!\r\n", ch );
         return;
      }
      else if( victim->pcdata->clan->clan_type == CLAN_GUILD )
      {
         if( victim->pcdata->clan == clan )
            send_to_char( "This player already belongs to your guild!\r\n", ch );
         else
            send_to_char( "This player already belongs to an guild!\r\n", ch );
         return;
      }
      else
      {
         if( victim->pcdata->clan == clan )
            send_to_char( "This player already belongs to your clan!\r\n", ch );
         else
            send_to_char( "This player already belongs to a clan!\r\n", ch );
         return;
      }
   }

   if( clan->mem_limit && clan->members >= clan->mem_limit )
   {
      send_to_char( "Your clan is too big to induct anymore players.\r\n", ch );
      return;
   }

   clan->members++;
   if( clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_GUILD )
      SET_BIT( victim->speaks, LANG_CLAN );

   if( clan->clan_type != CLAN_NOKILL && clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_GUILD )
   {
      xREMOVE_BIT( victim->act, PLR_NICE );
      SET_BIT( victim->pcdata->flags, PCFLAG_DEADLY );
   }

   if( clan->clan_type != CLAN_GUILD && clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_NOKILL )
   {
      int sn;

      for( sn = 0; sn < num_skills; ++sn )
      {
         if( skill_table[sn]->guild == clan->Class && skill_table[sn]->name != NULL )
         {
            victim->pcdata->learned[sn] = GET_ADEPT( victim, sn );
            ch_printf( victim, "%s instructs you in the ways of %s.\r\n", ch->name, skill_table[sn]->name );
         }
      }
   }

   victim->pcdata->clan = clan;
   STRFREE( victim->pcdata->clan_name );
   victim->pcdata->clan_name = QUICKLINK( clan->name );
   act( AT_MAGIC, "You induct $N into $t", ch, clan->name, victim, TO_CHAR );
   act( AT_MAGIC, "$n inducts $N into $t", ch, clan->name, victim, TO_NOTVICT );
   act( AT_MAGIC, "$n inducts you into $t", ch, clan->name, victim, TO_VICT );
   add_roster( clan, victim->name, victim->Class, victim->level, victim->pcdata->mkills, victim->pcdata->mdeaths );
   save_char_obj( victim );
   save_clan( clan );
   return;
}

void do_council_induct( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   COUNCIL_DATA *council;

   if( IS_NPC( ch ) || !ch->pcdata->council )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   council = ch->pcdata->council;

   if( ( council->head == NULL || str_cmp( ch->name, council->head ) )
       && ( council->head2 == NULL || str_cmp( ch->name, council->head2 ) ) && str_cmp( council->name, "mortal council" ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Induct whom into your council?\r\n", ch );
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

   if( victim->pcdata->council )
   {
      send_to_char( "This player already belongs to a council!\r\n", ch );
      return;
   }

   council->members++;
   victim->pcdata->council = council;
   STRFREE( victim->pcdata->council_name );
   victim->pcdata->council_name = QUICKLINK( council->name );
   act( AT_MAGIC, "You induct $N into $t", ch, council->name, victim, TO_CHAR );
   act( AT_MAGIC, "$n inducts $N into $t", ch, council->name, victim, TO_ROOM );
   act( AT_MAGIC, "$n inducts you into $t", ch, council->name, victim, TO_VICT );
   save_char_obj( victim );
   save_council( council );
   return;
}

/* Can the character outcast the victim? */
bool can_outcast( CLAN_DATA * clan, CHAR_DATA * ch, CHAR_DATA * victim )
{
   if( !clan || !ch || !victim )
      return FALSE;
   if( !str_cmp( ch->name, clan->deity ) )
      return TRUE;
   if( !str_cmp( victim->name, clan->deity ) )
      return FALSE;
   if( !str_cmp( ch->name, clan->leader ) )
      return TRUE;
   if( !str_cmp( victim->name, clan->leader ) )
      return FALSE;
   if( !str_cmp( ch->name, clan->number1 ) )
      return TRUE;
   if( !str_cmp( victim->name, clan->number1 ) )
      return FALSE;
   if( !str_cmp( ch->name, clan->number2 ) )
      return TRUE;
   if( !str_cmp( victim->name, clan->number2 ) )
      return FALSE;
   return TRUE;
}

void do_outcast( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   CLAN_DATA *clan;
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   clan = ch->pcdata->clan;

   if( ( ch->pcdata && ch->pcdata->bestowments
         && is_name( "canoutcast", ch->pcdata->bestowments ) )
       || !str_cmp( ch->name, clan->deity )
       || !str_cmp( ch->name, clan->leader ) || !str_cmp( ch->name, clan->number1 ) || !str_cmp( ch->name, clan->number2 ) )
      ;
   else
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Outcast whom?\r\n", ch );
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

   if( victim == ch )
   {
      if( ch->pcdata->clan->clan_type == CLAN_ORDER )
      {
         send_to_char( "Kick yourself out of your own order?\r\n", ch );
         return;
      }
      else if( ch->pcdata->clan->clan_type == CLAN_GUILD )
      {
         send_to_char( "Kick yourself out of your own guild?\r\n", ch );
         return;
      }
      else
      {
         send_to_char( "Kick yourself out of your own clan?\r\n", ch );
         return;
      }
   }

   if( victim->pcdata->clan != ch->pcdata->clan )
   {
      if( ch->pcdata->clan->clan_type == CLAN_ORDER )
      {
         send_to_char( "This player does not belong to your order!\r\n", ch );
         return;
      }
      else if( ch->pcdata->clan->clan_type == CLAN_GUILD )
      {
         send_to_char( "This player does not belong to your guild!\r\n", ch );
         return;
      }
      else
      {
         send_to_char( "This player does not belong to your clan!\r\n", ch );
         return;
      }
   }

   if( !can_outcast( clan, ch, victim ) )
   {
      send_to_char( "You are not able to outcast them.\r\n", ch );
      return;
   }

   if( clan->clan_type != CLAN_GUILD && clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_NOKILL )
   {
      int sn;

      for( sn = 0; sn < num_skills; ++sn )
         if( skill_table[sn]->guild == victim->pcdata->clan->Class && skill_table[sn]->name != NULL )
         {
            victim->pcdata->learned[sn] = 0;
            ch_printf( victim, "You forget the ways of %s.\r\n", skill_table[sn]->name );
         }
   }

   if( victim->speaking & LANG_CLAN )
      victim->speaking = LANG_COMMON;
   REMOVE_BIT( victim->speaks, LANG_CLAN );
   --clan->members;
   if( clan->members < 0 )
      clan->members = 0;
   if( !str_cmp( victim->name, ch->pcdata->clan->number1 ) )
   {
      STRFREE( ch->pcdata->clan->number1 );
      ch->pcdata->clan->number1 = STRALLOC( "" );
   }
   if( !str_cmp( victim->name, ch->pcdata->clan->number2 ) )
   {
      STRFREE( ch->pcdata->clan->number2 );
      ch->pcdata->clan->number2 = STRALLOC( "" );
   }
   if( !str_cmp( victim->name, ch->pcdata->clan->deity ) )
   {
      STRFREE( ch->pcdata->clan->deity );
      ch->pcdata->clan->deity = STRALLOC( "" );
   }
   victim->pcdata->clan = NULL;
   STRFREE( victim->pcdata->clan_name );
   victim->pcdata->clan_name = STRALLOC( "" );
   act( AT_MAGIC, "You outcast $N from $t", ch, clan->name, victim, TO_CHAR );
   act( AT_MAGIC, "$n outcasts $N from $t", ch, clan->name, victim, TO_ROOM );
   if( victim->desc && victim->desc->host )
      act( AT_MAGIC, "$n outcasts you from $t", ch, clan->name, victim, TO_VICT );
   else
      add_loginmsg( victim->name, 6, NULL );
   if( clan->clan_type != CLAN_GUILD && clan->clan_type != CLAN_ORDER )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s has been outcast from %s!", victim->name, clan->name );
      echo_to_all( AT_MAGIC, buf, ECHOTAR_PK );
   }
   remove_roster( clan, victim->name );
   save_char_obj( victim );
   save_clan( clan );
   return;
}

void do_council_outcast( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   COUNCIL_DATA *council;

   if( IS_NPC( ch ) || !ch->pcdata->council )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   council = ch->pcdata->council;

   if( ( council->head == NULL || str_cmp( ch->name, council->head ) )
       && ( council->head2 == NULL || str_cmp( ch->name, council->head2 ) ) && str_cmp( council->name, "mortal council" ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Outcast whom from your council?\r\n", ch );
      return;
   }

   if( !( victim = get_char_room( ch, arg ) ) )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Kick yourself out of your own council?\r\n", ch );
      return;
   }

   if( victim->pcdata->council != ch->pcdata->council )
   {
      send_to_char( "This player does not belong to your council!\r\n", ch );
      return;
   }

   if( council->head2 && !str_cmp( victim->name, ch->pcdata->council->head2 ) )
   {
      STRFREE( ch->pcdata->council->head2 );
      ch->pcdata->council->head2 = NULL;
   }

   --council->members;
   if( council->members < 0 )
      council->members = 0;
   victim->pcdata->council = NULL;
   STRFREE( victim->pcdata->council_name );
   victim->pcdata->council_name = STRALLOC( "" );
   check_switch( ch, FALSE );
   act( AT_MAGIC, "You outcast $N from $t", ch, council->name, victim, TO_CHAR );
   act( AT_MAGIC, "$n outcasts $N from $t", ch, council->name, victim, TO_ROOM );
   act( AT_MAGIC, "$n outcasts you from $t", ch, council->name, victim, TO_VICT );
   save_char_obj( victim );
   save_council( council );
   return;
}

void do_setvault( CHAR_DATA *ch, const char *argument )
{
   VAULT_DATA *vault;
   int rnum;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( !str_cmp( arg1, "show" ) )
   {
      ROOM_INDEX_DATA *room;

      pager_printf_color( ch, "&W%6s | %-40.40s | Area name\r\n", "VNUM", "Room Name" );
      for( vault= first_vault; vault; vault = vault->next )
      {
         if( ( room = get_room_index( vault->vnum ) ) == NULL )
            continue;
         pager_printf_color( ch, "&c%6d | &C%-40.40s | %s\r\n", vault->vnum, room->name, room->area->filename );
      }
      return;
   }                                                                                                                                       

   if ( !str_cmp( arg1, "save" )	&& get_trust( ch ) > LEVEL_GREATER )
   {
      save_vault_list( );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( is_number(arg1) && arg2[0] != '\0' )
   {
      rnum = atoi(arg1);

      if( get_room_index( rnum ) != NULL )
      {
         if( !str_cmp(arg2, "create" ) )
         {
            CREATE( vault, VAULT_DATA, 1 );
            vault->vnum = rnum;
            sort_vaults( vault );
            send_to_char( "Donation room created.\r\n", ch );
            return;
         }

         if( !str_cmp( arg2, "delete" ) )
         {
            for( vault = first_vault; vault; vault = vault->next )
            {
               if( vault->vnum == rnum )
               {
                  UNLINK( vault, first_vault, last_vault, next, prev );
                  DISPOSE( vault );
                  send_to_char( "Deleting that vnum...\r\n", ch );
                  return;
               }
            }
         }
         else
            send_to_char( "Not currently a donation vnum.\r\n", ch );
      }
      else
      {
         send_to_char( "Invalid vnum argument.\r\n", ch );
         return;
      }
   }

   set_char_color( AT_IMMORT, ch );

   send_to_char( "Syntax:\r\n", ch );
   send_to_char( "  setvault show - lists the rooms currently set to save donations.\r\n", ch );
   send_to_char( "  setvault <vnum> create - adds a vnum to the list of rooms to save.\r\n", ch );
   send_to_char( "  setvault <vnum> delete - removes a vnum from the list of rooms to save,\r\n\r\n", ch );
   if( get_trust( ch ) > LEVEL_GREATER )
      send_to_char( "  setvault save - saves the vault list.\r\n", ch );
   send_to_char( "    Remember, rooms set as storage on clans or councils will need to be.\r\n", ch );
   send_to_char( "    removed in both clan/council file and the vault list.\r\n", ch );

   return;
}                               

void do_setclan( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CLAN_DATA *clan;

   set_char_color( AT_PLAIN, ch );
   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Usage: setclan <clan> <field> <deity|leader|number1|number2> <player>\r\n", ch );
      send_to_char( "\r\nField being one of:\r\n", ch );
      send_to_char( " deity leader number1 number2\r\n", ch );
      send_to_char( " members board recall storage guard1 guard2\r\n", ch );
      send_to_char( " align (not functional) memlimit\r\n", ch );
      send_to_char( " leadrank onerank tworank\r\n", ch );
      send_to_char( " obj1 obj2 obj3 obj4 obj5\r\n", ch );
      send_to_char( " badge abbrev\r\n", ch );
      if( get_trust( ch ) >= LEVEL_GOD )
      {
         send_to_char( " name filename motto desc\r\n", ch );
         send_to_char( " favour strikes type class\r\n", ch );
      }
      if( get_trust( ch ) >= LEVEL_IMPLEMENTOR )
         send_to_char( " pkill1-7 pdeath1-7\r\n", ch );
      return;
   }

   clan = get_clan( arg1 );
   if( !clan )
   {
      send_to_char( "No such clan.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "deity" ) )
   {
      STRFREE( clan->deity );
      clan->deity = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "leader" ) )
   {
      STRFREE( clan->leader );
      clan->leader = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "number1" ) )
   {
      STRFREE( clan->number1 );
      clan->number1 = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "number2" ) )
   {
      STRFREE( clan->number2 );
      clan->number2 = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "leadrank" ) )
   {
      STRFREE( clan->leadrank );
      clan->leadrank = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "onerank" ) )
   {
      STRFREE( clan->onerank );
      clan->onerank = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "tworank" ) )
   {
      STRFREE( clan->tworank );
      clan->tworank = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "badge" ) )
   {
      STRFREE( clan->badge );
      clan->badge = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "abbrev" ) )
   {
      STRFREE( clan->abbrev );
      clan->abbrev = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "board" ) )
   {
      clan->board = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "memlimit" ) )
   {
      clan->mem_limit = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "members" ) )
   {
      clan->members = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "recall" ) )
   {
      clan->recall = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "storage" ) )
   {
      VAULT_DATA *vault;

      if( clan->storeroom )
      {
         for( vault = first_vault; vault; vault = vault->next )
            if( vault->vnum == clan->storeroom )
               UNLINK( vault, first_vault, last_vault, next, prev );
      }

      clan->storeroom = atoi( argument );
      CREATE( vault, VAULT_DATA, 1 );
      vault->vnum = atoi( argument );
      sort_vaults( vault );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "obj1" ) )
   {
      clan->clanobj1 = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "obj2" ) )
   {
      clan->clanobj2 = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "obj3" ) )
   {
      clan->clanobj3 = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "obj4" ) )
   {
      clan->clanobj4 = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "obj5" ) )
   {
      clan->clanobj5 = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "guard1" ) )
   {
      clan->guard1 = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "guard2" ) )
   {
      clan->guard2 = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( get_trust( ch ) < LEVEL_GOD )
   {
      do_setclan( ch, "" );
      return;
   }

   if( !str_cmp( arg2, "align" ) )
   {
      clan->alignment = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "type" ) )
   {
      if( !str_cmp( argument, "order" ) )
         clan->clan_type = CLAN_ORDER;
      else if( !str_cmp( argument, "guild" ) )
         clan->clan_type = CLAN_GUILD;
      else
         clan->clan_type = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "class" ) )
   {
      clan->Class = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      CLAN_DATA *uclan = NULL;

      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "You can't name a clan nothing.\r\n", ch );
         return;
      }
      if( ( uclan = get_clan( argument ) ) )
      {
         send_to_char( "There is already another clan with that name.\r\n", ch );
         return;
      }
      STRFREE( clan->name );
      clan->name = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "filename" ) )
   {
      char filename[256];

      if( !is_valid_filename( ch, CLAN_DIR, argument ) )
         return;

      snprintf( filename, sizeof( filename ), "%s%s", CLAN_DIR, clan->filename );
      if( !remove( filename ) )
         send_to_char( "Old clan file deleted.\r\n", ch );

      DISPOSE( clan->filename );
      clan->filename = str_dup( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      write_clan_list(  );
      return;
   }

   if( !str_cmp( arg2, "motto" ) )
   {
      STRFREE( clan->motto );
      clan->motto = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( !str_cmp( arg2, "desc" ) )
   {
      STRFREE( clan->description );
      clan->description = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_clan( clan );
      return;
   }

   if( get_trust( ch ) < LEVEL_IMPLEMENTOR )
   {
      do_setclan( ch, "" );
      return;
   }

   if( !str_prefix( "pkill", arg2 ) )
   {
      int temp_value;
      if( !str_cmp( arg2, "pkill1" ) )
         temp_value = 0;
      else if( !str_cmp( arg2, "pkill2" ) )
         temp_value = 1;
      else if( !str_cmp( arg2, "pkill3" ) )
         temp_value = 2;
      else if( !str_cmp( arg2, "pkill4" ) )
         temp_value = 3;
      else if( !str_cmp( arg2, "pkill5" ) )
         temp_value = 4;
      else if( !str_cmp( arg2, "pkill6" ) )
         temp_value = 5;
      else if( !str_cmp( arg2, "pkill7" ) )
         temp_value = 6;
      else
      {
         do_setclan( ch, "" );
         return;
      }
      clan->pkills[temp_value] = atoi( argument );
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_prefix( "pdeath", arg2 ) )
   {
      int temp_value;
      if( !str_cmp( arg2, "pdeath1" ) )
         temp_value = 0;
      else if( !str_cmp( arg2, "pdeath2" ) )
         temp_value = 1;
      else if( !str_cmp( arg2, "pdeath3" ) )
         temp_value = 2;
      else if( !str_cmp( arg2, "pdeath4" ) )
         temp_value = 3;
      else if( !str_cmp( arg2, "pdeath5" ) )
         temp_value = 4;
      else if( !str_cmp( arg2, "pdeath6" ) )
         temp_value = 5;
      else if( !str_cmp( arg2, "pdeath7" ) )
         temp_value = 6;
      else
      {
         do_setclan( ch, "" );
         return;
      }
      clan->pdeaths[temp_value] = atoi( argument );
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   do_setclan( ch, "" );
   return;
}

void do_setcouncil( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   COUNCIL_DATA *council;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Usage: setcouncil <council> <field> <value>\r\n", ch );
      send_to_char( "\r\nField being one of:\r\n", ch );
      send_to_char( " head head2 members board meeting\r\n", ch );
      send_to_char( " abbrev storage\r\n", ch );
      if( get_trust( ch ) >= LEVEL_GOD )
         send_to_char( " name filename desc\r\n", ch );
      if( get_trust( ch ) >= LEVEL_SUB_IMPLEM )
         send_to_char( " powers\r\n", ch );
      return;
   }

   council = get_council( arg1 );
   if( !council )
   {
      send_to_char( "No such council.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "head" ) )
   {
      STRFREE( council->head );
      council->head = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }

   if( !str_cmp( arg2, "head2" ) )
   {
      if( council->head2 != NULL )
         STRFREE( council->head2 );
      if( !str_cmp( argument, "none" ) || !str_cmp( argument, "clear" ) )
         council->head2 = NULL;
      else
         council->head2 = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }

   if( !str_cmp( arg2, "board" ) )
   {
      council->board = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }

   if( !str_cmp( arg2, "members" ) )
   {
      council->members = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }

   if( !str_cmp( arg2, "meeting" ) )
   {
      council->meeting = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }

   if( !str_cmp( arg2, "storage" ) )
   {
      VAULT_DATA *vault;

      if( council->storeroom )
      {
         for( vault = first_vault; vault; vault = vault->next )
            if( vault->vnum == council->storeroom )
               UNLINK( vault, first_vault, last_vault, next, prev );
      }

      council->storeroom = atoi( argument );
      CREATE( vault, VAULT_DATA, 1 );
      vault->vnum = atoi( argument );
      sort_vaults( vault );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }

   if( get_trust( ch ) < LEVEL_GOD )
   {
      do_setcouncil( ch, "" );
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      COUNCIL_DATA *ucouncil;

      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Can't set a council name to nothing.\r\n", ch );
         return;
      }
      if( ( ucouncil = get_council( argument ) ) )
      {
         send_to_char( "A council is already using that name.\r\n", ch );
         return;
      }
      STRFREE( council->name );
      council->name = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }

   if( !str_cmp( arg2, "filename" ) )
   {
      char filename[256];

      if( !is_valid_filename( ch, COUNCIL_DIR, argument ) )
         return;

      snprintf( filename, sizeof( filename ), "%s%s", COUNCIL_DIR, council->filename );
      if( !remove( filename ) )
         send_to_char( "Old council file deleted.\r\n", ch );

      DISPOSE( council->filename );
      council->filename = str_dup( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      write_council_list(  );
      return;
   }

   if( !str_cmp( arg2, "abbrev" ) )
   {
      STRFREE( council->abbrev );
      council->abbrev = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }

   if( !str_cmp( arg2, "desc" ) )
   {
      STRFREE( council->description );
      council->description = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }

   if( get_trust( ch ) < LEVEL_SUB_IMPLEM )
   {
      do_setcouncil( ch, "" );
      return;
   }

   if( !str_cmp( arg2, "powers" ) )
   {
      STRFREE( council->powers );
      council->powers = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      save_council( council );
      return;
   }

   do_setcouncil( ch, "" );
   return;
}

/*
 * Added multiple levels on pkills and pdeaths. -- Shaddai
 */

void do_showclan( CHAR_DATA* ch, const char* argument)
{
   CLAN_DATA *clan;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
   if( argument[0] == '\0' )
   {
      send_to_char( "Usage: showclan <clan>\r\n", ch );
      return;
   }

   clan = get_clan( argument );
   if( !clan )
   {
      send_to_char( "No such clan, guild or order.\r\n", ch );
      return;
   }

   ch_printf_color( ch, "\r\n&w%s    : &W%s\t\t&wBadge: %s\r\n&wFilename : &W%s\r\n&wMotto    : &W%s\r\n",
                    clan->clan_type == CLAN_ORDER ? "Order" :
                    ( clan->clan_type == CLAN_GUILD ? "Guild" : "Clan " ),
                    clan->name, clan->badge ? clan->badge : "(not set)", clan->filename, clan->motto );
   ch_printf_color( ch, "&wAbbrev   : &W%s\r\n", clan->abbrev ? clan->abbrev : "(not set)" );
   ch_printf_color( ch, "&wDesc     : &W%s\r\n&wDeity    : &W%s\r\n", clan->description, clan->deity );
   ch_printf_color( ch, "&wLeader   : &W%-19.19s\t&wRank: &W%s\r\n", clan->leader, clan->leadrank );
   ch_printf_color( ch, "&wNumber1  : &W%-19.19s\t&wRank: &W%s\r\n", clan->number1, clan->onerank );
   ch_printf_color( ch, "&wNumber2  : &W%-19.19s\t&wRank: &W%s\r\n", clan->number2, clan->tworank );
   ch_printf_color( ch,
                    "&wPKills   : &w1-9:&W%-3d &w10-14:&W%-3d &w15-19:&W%-3d &w20-29:&W%-3d &w30-39:&W%-3d &w40-49:&W%-3d &w50:&W%-3d\r\n",
                    clan->pkills[0], clan->pkills[1], clan->pkills[2], clan->pkills[3], clan->pkills[4], clan->pkills[5],
                    clan->pkills[6] );
   ch_printf_color( ch,
                    "&wPDeaths  : &w1-9:&W%-3d &w10-14:&W%-3d &w15-19:&W%-3d &w20-29:&W%-3d &w30-39:&W%-3d &w40-49:&W%-3d &w50:&W%-3d\r\n",
                    clan->pdeaths[0], clan->pdeaths[1], clan->pdeaths[2], clan->pdeaths[3], clan->pdeaths[4],
                    clan->pdeaths[5], clan->pdeaths[6] );
   ch_printf_color( ch, "&wIllegalPK: &W%-6d\r\n", clan->illegal_pk );
   ch_printf_color( ch, "&wMKills   : &W%-6d   &wMDeaths: &W%-6d\r\n", clan->mkills, clan->mdeaths );
   ch_printf_color( ch, "&wScore    : &W%-6d   &wFavor  : &W%-6d   &wStrikes: &W%d\r\n",
                    clan->score, clan->favour, clan->strikes );
   ch_printf_color( ch, "&wMembers  : &W%-6d  &wMemLimit: &W%-6d   &wAlign  : &W%-6d",
                    clan->members, clan->mem_limit, clan->alignment );
   if( clan->clan_type == CLAN_GUILD )
      ch_printf_color( ch, "   &wClass  : &W%d &w(&W%s&w)",
                       clan->Class, clan->Class < MAX_PC_CLASS ? class_table[clan->Class]->who_name : "unknown" );
   send_to_char( "\r\n", ch );
   ch_printf_color( ch, "&wBoard    : &W%-5d    &wRecall : &W%-5d    &wStorage: &W%-5d\r\n",
                    clan->board, clan->recall, clan->storeroom );
   ch_printf_color( ch, "&wGuard1   : &W%-5d    &wGuard2 : &W%-5d\r\n", clan->guard1, clan->guard2 );
   ch_printf_color( ch, "&wObj1( &W%d &w)  Obj2( &W%d &w)  Obj3( &W%d &w)  Obj4( &W%d &w)  Obj5( &W%d &w)\r\n",
                    clan->clanobj1, clan->clanobj2, clan->clanobj3, clan->clanobj4, clan->clanobj5 );
   return;
}

void do_showcouncil( CHAR_DATA* ch, const char* argument)
{
   COUNCIL_DATA *council;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
   if( argument[0] == '\0' )
   {
      send_to_char( "Usage: showcouncil <council>\r\n", ch );
      return;
   }

   council = get_council( argument );
   if( !council )
   {
      send_to_char( "No such council.\r\n", ch );
      return;
   }

   ch_printf_color( ch, "\r\n&wCouncil :  &W%s\r\n&wFilename:  &W%s\r\n", council->name, council->filename );
   ch_printf_color( ch, "&wAbbreviation :  &W%s\r\n", council->abbrev );
   ch_printf_color( ch, "&wHead:      &W%s\r\n", council->head );
   ch_printf_color( ch, "&wHead2:     &W%s\r\n", council->head2 );
   ch_printf_color( ch, "&wMembers:   &W%-d\r\n", council->members );
   ch_printf_color( ch, "&wBoard:     &W%-5d\r\n&wMeeting:   &W%-5d\r\n&wPowers:    &W%s\r\n",
                    council->board, council->meeting, council->powers );
   ch_printf_color( ch, "&wStoreroom: &W%-5d\r\n", council->storeroom );
   ch_printf_color( ch, "&wDescription:\r\n&W%s\r\n", council->description );
   return;
}

void do_makeclan( CHAR_DATA* ch, const char* argument)
{
   CLAN_DATA *clan;

   set_char_color( AT_IMMORT, ch );

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: makeclan <clan name>\r\n", ch );
      return;
   }

   set_char_color( AT_PLAIN, ch );
   clan = get_clan( argument );
   if( clan )
   {
      send_to_char( "There is already a clan with that name.\r\n", ch );
      return;
   }

   CREATE( clan, CLAN_DATA, 1 );
   LINK( clan, first_clan, last_clan, next, prev );

   clan->name = STRALLOC( argument );
   clan->abbrev = STRALLOC( "" );
   /*
    * Let's refix this, STRALLOC shouldn't be used for the 'filename'
    * member without changing load_clan() and do_setclan() to employ hashstrings too... 
    */
   clan->filename = str_dup( "" );
   clan->motto = STRALLOC( "" );
   clan->description = STRALLOC( "" );
   clan->deity = STRALLOC( "" );
   clan->leader = STRALLOC( "" );
   clan->number1 = STRALLOC( "" );
   clan->number2 = STRALLOC( "" );
   clan->leadrank = STRALLOC( "" );
   clan->onerank = STRALLOC( "" );
   clan->tworank = STRALLOC( "" );
   clan->badge = STRALLOC( "" );
}

void do_makecouncil( CHAR_DATA* ch, const char* argument)
{
   char filename[256];
   COUNCIL_DATA *council;

   set_char_color( AT_IMMORT, ch );

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: makecouncil <council name>\r\n", ch );
      return;
   }

   snprintf( filename, 256, "%s%s", COUNCIL_DIR, strlower( argument ) );

   CREATE( council, COUNCIL_DATA, 1 );
   LINK( council, first_council, last_council, next, prev );
   council->name = STRALLOC( argument );
   council->abbrev = STRALLOC( "" );
   council->head = STRALLOC( "" );
   council->head2 = NULL;
   council->powers = STRALLOC( "" );
}

/*
 * Added multiple level pkill and pdeath support. --Shaddai
 */
void do_clans( CHAR_DATA* ch, const char* argument)
{
   CLAN_DATA *clan;
   int count = 0;

   if( argument[0] == '\0' )
   {
      set_char_color( AT_BLOOD, ch );
      send_to_char
         ( "\r\nClan          Deity         Leader           Pkills:    Avatar      Other\r\n_________________________________________________________________________\r\n\r\n",
           ch );
      for( clan = first_clan; clan; clan = clan->next )
      {
         if( clan->clan_type == CLAN_ORDER || clan->clan_type == CLAN_GUILD )
            continue;
         set_char_color( AT_GREY, ch );
         ch_printf( ch, "%-13s %-13s %-13s", clan->name, clan->deity, clan->leader );
         set_char_color( AT_BLOOD, ch );
         ch_printf( ch, "                %5d      %5d\r\n",
                    clan->pkills[6], ( clan->pkills[2] + clan->pkills[3] + clan->pkills[4] + clan->pkills[5] ) );
         count++;
      }
      set_char_color( AT_BLOOD, ch );
      if( !count )
         send_to_char( "There are no Clans currently formed.\r\n", ch );
      else
         send_to_char
            ( "_________________________________________________________________________\r\n\r\nUse 'clans <clan>' for detailed information and a breakdown of victories.\r\n",
              ch );
      return;
   }

   clan = get_clan( argument );
   if( !clan || clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_ORDER )
   {
      set_char_color( AT_BLOOD, ch );
      send_to_char( "No such clan.\r\n", ch );
      return;
   }
   set_char_color( AT_BLOOD, ch );
   ch_printf( ch, "\r\n%s, '%s'\r\n\r\n", clan->name, clan->motto );
   set_char_color( AT_GREY, ch );
   send_to_char_color( "Victories:&w\r\n", ch );
   ch_printf_color( ch,
                    "    &w15-19...  &r%-4d\r\n    &w20-29...  &r%-4d\r\n    &w30-39...  &r%-4d\r\n    &w40-49...  &r%-4d\r\n",
                    clan->pkills[2], clan->pkills[3], clan->pkills[4], clan->pkills[5] );
   ch_printf_color( ch, "   &wAvatar...  &r%-4d\r\n", clan->pkills[6] );
   set_char_color( AT_GREY, ch );
   ch_printf( ch, "Clan Leader:  %s\r\nNumber One :  %s\r\nNumber Two :  %s\r\nClan Deity :  %s\r\n",
              clan->leader, clan->number1, clan->number2, clan->deity );
   if( !str_cmp( ch->name, clan->deity )
       || !str_cmp( ch->name, clan->leader )
       || !str_cmp( ch->name, clan->number1 ) || !str_cmp( ch->name, clan->number2 ) || get_trust( ch ) >= LEVEL_GREATER )
      ch_printf( ch, "Members    :  %d\r\n", clan->members );
   ch_printf( ch, "Abbrev     :  %s\n\r", clan->abbrev ? clan->abbrev : "" );
   set_char_color( AT_BLOOD, ch );
   ch_printf( ch, "\r\nDescription:  %s\r\n", clan->description );
   return;
}

void do_orders( CHAR_DATA* ch, const char* argument)
{
   CLAN_DATA *order;
   int count = 0;

   if( argument[0] == '\0' )
   {
      set_char_color( AT_DGREEN, ch );
      send_to_char
         ( "\r\nOrder            Deity          Leader           Mkills      Mdeaths\r\n____________________________________________________________________\r\n\r\n",
           ch );
      set_char_color( AT_GREEN, ch );
      for( order = first_clan; order; order = order->next )
         if( order->clan_type == CLAN_ORDER )
         {
            ch_printf( ch, "%-16s %-14s %-14s   %-7d       %5d\r\n",
                       order->name, order->deity, order->leader, order->mkills, order->mdeaths );
            count++;
         }
      set_char_color( AT_DGREEN, ch );
      if( !count )
         send_to_char( "There are no Orders currently formed.\r\n", ch );
      else
         send_to_char
            ( "____________________________________________________________________\r\n\r\nUse 'orders <order>' for more detailed information.\r\n",
              ch );
      return;
   }

   order = get_clan( argument );
   if( !order || order->clan_type != CLAN_ORDER )
   {
      set_char_color( AT_DGREEN, ch );
      send_to_char( "No such Order.\r\n", ch );
      return;
   }

   set_char_color( AT_DGREEN, ch );
   ch_printf( ch, "\r\nOrder of %s\r\n'%s'\r\n\r\n", order->name, order->motto );
   set_char_color( AT_GREEN, ch );
   ch_printf( ch, "Deity      :  %s\r\nLeader     :  %s\r\nNumber One :  %s\r\nNumber Two :  %s\r\n",
              order->deity, order->leader, order->number1, order->number2 );
   if( !str_cmp( ch->name, order->deity )
       || !str_cmp( ch->name, order->leader )
       || !str_cmp( ch->name, order->number1 ) || !str_cmp( ch->name, order->number2 ) || get_trust( ch ) >= LEVEL_GREATER )
      ch_printf( ch, "Members    :  %d\r\n", order->members );
   ch_printf( ch, "Abbrev     :  %s\n\r", order->abbrev ? order->abbrev : "" );
   set_char_color( AT_DGREEN, ch );
   ch_printf( ch, "\r\nDescription:\r\n%s\r\n", order->description );
   return;
}

void do_councils( CHAR_DATA* ch, const char* argument)
{
   COUNCIL_DATA *council;

   set_char_color( AT_CYAN, ch );
   if( !first_council )
   {
      send_to_char( "There are no councils currently formed.\r\n", ch );
      return;
   }
   if( argument[0] == '\0' )
   {
      send_to_char_color( "\r\n&cTitle                    Head\r\n", ch );
      for( council = first_council; council; council = council->next )
      {
         if( council->head2 != NULL )
            ch_printf_color( ch, "&w%-24s %s and %s\r\n", council->name, council->head, council->head2 );
         else
            ch_printf_color( ch, "&w%-24s %-14s\r\n", council->name, council->head );
      }
      send_to_char_color( "&cUse 'councils <name of council>' for more detailed information.\r\n", ch );
      return;
   }
   council = get_council( argument );
   if( !council )
   {
      send_to_char_color( "&cNo such council exists...\r\n", ch );
      return;
   }
   ch_printf_color( ch, "&c\r\n%s\r\n", council->name );
   if( council->head2 == NULL )
      ch_printf_color( ch, "&cHead:     &w%s\r\n&cMembers:  &w%d\r\n", council->head, council->members );
   else
      ch_printf_color( ch, "&cCo-Heads:     &w%s &cand &w%s\r\n&cMembers:  &w%d\r\n",
                       council->head, council->head2, council->members );
   ch_printf_color( ch, "&cDescription:\r\n&w%s\r\n", council->description );
   return;
}

void do_guilds( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   CLAN_DATA *guild;
   int count = 0;

   if( argument[0] == '\0' )
   {
      set_char_color( AT_HUNGRY, ch );
      send_to_char
         ( "\r\nGuild                  Leader             Mkills      Mdeaths\r\n_____________________________________________________________\r\n\r\n",
           ch );
      set_char_color( AT_YELLOW, ch );
      for( guild = first_clan; guild; guild = guild->next )
         if( guild->clan_type == CLAN_GUILD )
         {
            ++count;
            ch_printf( ch, "%-20s   %-14s     %-6d       %6d\r\n", guild->name, guild->leader, guild->mkills,
                       guild->mdeaths );
         }
      set_char_color( AT_HUNGRY, ch );
      if( !count )
         send_to_char( "There are no Guilds currently formed.\r\n", ch );
      else
         send_to_char
            ( "_____________________________________________________________\r\n\r\nUse guilds <class>' for specifics. (ex:  guilds thieves)\r\n",
              ch );
      return;
   }

   snprintf( buf, MAX_STRING_LENGTH, "guild of %s", argument );
   guild = get_clan( buf );
      guild = get_clan( argument );
   if( !guild || guild->clan_type != CLAN_GUILD )
   {
      set_char_color( AT_HUNGRY, ch );
      send_to_char( "No such Guild.\r\n", ch );
      return;
   }
   set_char_color( AT_HUNGRY, ch );
   ch_printf( ch, "\r\n%s\r\n", guild->name );
   set_char_color( AT_YELLOW, ch );
   ch_printf( ch, "Leader:    %s\r\nNumber 1:  %s\r\nNumber 2:  %s\r\nMotto:     %s\r\n",
              guild->leader, guild->number1, guild->number2, guild->motto );
   if( !str_cmp( ch->name, guild->deity )
       || !str_cmp( ch->name, guild->leader )
       || !str_cmp( ch->name, guild->number1 ) || !str_cmp( ch->name, guild->number2 ) || get_trust( ch ) >= LEVEL_GREATER )
      ch_printf( ch, "Members:   %d\r\n", guild->members );
   ch_printf( ch, "Abbrev:    %s\n\r", guild->abbrev ? guild->abbrev : "" );
   set_char_color( AT_HUNGRY, ch );
   ch_printf( ch, "Guild Description:\r\n%s\r\n", guild->description );
   return;
}

void do_defeats( CHAR_DATA * ch, const char *argument )
{
   char filename[256];

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   if( ch->pcdata->clan->clan_type != CLAN_ORDER && ch->pcdata->clan->clan_type != CLAN_GUILD )
   {
      sprintf( filename, "%s%s.defeats", CLAN_DIR, ch->pcdata->clan->name );
      set_pager_color( AT_PURPLE, ch );
      if( !str_cmp( ch->name, ch->pcdata->clan->leader ) && !str_cmp( argument, "clean" ) )
      {
         FILE *fp = fopen( filename, "w" );
         if( fp )
            fclose( fp );
         send_to_pager( "\r\nDefeats ledger has been cleared.\r\n", ch );
         return;
      }
      else
      {
         send_to_pager( "\r\nLVL  Character                LVL  Character\r\n", ch );
         show_file( ch, filename );
         return;
      }
   }
   else
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
}

void do_victories( CHAR_DATA* ch, const char* argument)
{
   char filename[256];

   if( IS_NPC( ch ) || !ch->pcdata->clan )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
   if( ch->pcdata->clan->clan_type != CLAN_ORDER && ch->pcdata->clan->clan_type != CLAN_GUILD )
   {
      snprintf( filename, 256, "%s%s.record", CLAN_DIR, ch->pcdata->clan->name );
      set_pager_color( AT_PURPLE, ch );
      if( !str_cmp( ch->name, ch->pcdata->clan->leader ) && !str_cmp( argument, "clean" ) )
      {
         FILE *fp = fopen( filename, "w" );
         if( fp )
            fclose( fp );
         send_to_pager( "\r\nVictories ledger has been cleared.\r\n", ch );
         return;
      }
      else
      {
         send_to_pager( "\r\nLVL  Character       LVL  Character\r\n", ch );
         show_file( ch, filename );
         return;
      }
   }
   else
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }
}

void do_shove( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int exit_dir;
   EXIT_DATA *pexit;
   CHAR_DATA *victim;
   bool nogo;
   ROOM_INDEX_DATA *to_room;
   int schance = 0;
   int race_bonus = 0;
   short temp;

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );

   if( IS_NPC( ch ) || !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
   {
      send_to_char( "Only deadly characters can shove.\r\n", ch );
      return;
   }

   if( get_timer( ch, TIMER_PKILLED ) > 0 )
   {
      send_to_char( "You can't shove a player right now.\r\n", ch );
      return;
   }

   if( arg[0] == '\0' )
   {
      send_to_char( "Shove whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You shove yourself around, to no avail.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) || !IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) )
   {
      send_to_char( "You can only shove deadly characters.\r\n", ch );
      return;
   }

   if( ch->level - victim->level > 5 || victim->level - ch->level > 5 )
   {
      send_to_char( "There is too great an experience difference for you to even bother.\r\n", ch );
      return;
   }

   if( get_timer( victim, TIMER_PKILLED ) > 0 )
   {
      send_to_char( "You can't shove that player right now.\r\n", ch );
      return;
   }

   if( ( victim->position ) != POS_STANDING )
   {
      act( AT_PLAIN, "$N isn't standing up.", ch, NULL, victim, TO_CHAR );
      return;
   }

   if( arg2[0] == '\0' )
   {
      send_to_char( "Shove them in which direction?\r\n", ch );
      return;
   }

   exit_dir = get_dir( arg2 );
   if( xIS_SET( victim->in_room->room_flags, ROOM_SAFE ) && get_timer( victim, TIMER_SHOVEDRAG ) <= 0 )
   {
      send_to_char( "That character cannot be shoved right now.\r\n", ch );
      return;
   }

   nogo = FALSE;
   if( ( pexit = get_exit( ch->in_room, exit_dir ) ) == NULL )
      nogo = TRUE;
   else
      if( IS_SET( pexit->exit_info, EX_CLOSED )
          && ( !IS_AFFECTED( victim, AFF_PASS_DOOR ) || IS_SET( pexit->exit_info, EX_NOPASSDOOR ) ) )
      nogo = TRUE;

   if( nogo )
   {
      send_to_char( "There's no exit in that direction.\r\n", ch );
      return;
   }

   to_room = pexit->to_room;
   if( xIS_SET( to_room->room_flags, ROOM_DEATH ) )
   {
      send_to_char( "You cannot shove someone into a death trap.\r\n", ch );
      return;
   }

   if( ch->in_room->area != to_room->area && !in_hard_range( victim, to_room->area ) )
   {
      send_to_char( "That character cannot enter that area.\r\n", ch );
      return;
   }

/* Check for class, assign percentage based on that. */
   if( ch->Class == CLASS_WARRIOR )
      schance = 70;
   if( ch->Class == CLASS_VAMPIRE )
      schance = 65;
   if( ch->Class == CLASS_RANGER )
      schance = 60;
   if( ch->Class == CLASS_DRUID )
      schance = 45;
   if( ch->Class == CLASS_CLERIC )
      schance = 35;
   if( ch->Class == CLASS_THIEF )
      schance = 30;
   if( ch->Class == CLASS_MAGE )
      schance = 15;
   if( ch->Class == CLASS_AUGURER )
      schance = 20;
   if( ch->Class == CLASS_PALADIN )
      schance = 55;
   if( ch->Class == CLASS_NEPHANDI )
      schance = 20;
   if( ch->Class == CLASS_SAVAGE )
      schance = 70;

/* Add 3 points to chance for every str point above 15, subtract for below 15 */

   schance += ( ( get_curr_str( ch ) - 15 ) * 3 );

   schance += ( ch->level - victim->level );

/* Check for race, adjust percentage based on that. */
   if( ch->race == RACE_ELF )
      race_bonus = -3;

   if( ch->race == RACE_DWARF )
      race_bonus = 3;

   if( ch->race == RACE_HALFLING )
      race_bonus = -5;

   if( ch->race == RACE_PIXIE )
      race_bonus = -7;

   if( ch->race == RACE_HALF_OGRE )
      race_bonus = 5;

   if( ch->race == RACE_HALF_ORC )
      race_bonus = 7;

   if( ch->race == RACE_HALF_TROLL )
      race_bonus = 10;

   if( ch->race == RACE_HALF_ELF )
      race_bonus = -2;

   if( ch->race == RACE_GITH )
      race_bonus = -2;

   if( ch->race == RACE_PIXIE )
      race_bonus = -7;

   if( ch->race == RACE_DROW )
      race_bonus = 1;

   if( ch->race == RACE_SEA_ELF )
      race_bonus = -1;

   if( ch->race == RACE_LIZARDMAN )
      race_bonus = 4;

   if( ch->race == RACE_GNOME )
      race_bonus = -2;

   schance += race_bonus;

   if( schance < number_percent(  ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   temp = victim->position;
   victim->position = POS_SHOVE;
   act( AT_ACTION, "You shove $M.", ch, NULL, victim, TO_CHAR );
   act( AT_ACTION, "$n shoves you.", ch, NULL, victim, TO_VICT );
   move_char( victim, get_exit( ch->in_room, exit_dir ), 0 );
   if( !char_died( victim ) )
      victim->position = temp;
   WAIT_STATE( ch, 12 );
   /*
    * Remove protection from shove/drag if char shoves -- Blodkai 
    */
   if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) && get_timer( ch, TIMER_SHOVEDRAG ) <= 0 )
      add_timer( ch, TIMER_SHOVEDRAG, 10, NULL, 0 );
}

void do_drag( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int exit_dir;
   CHAR_DATA *victim;
   EXIT_DATA *pexit;
   ROOM_INDEX_DATA *to_room;
   bool nogo;
   int schance = 0;
   int race_bonus = 0;

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Only characters can drag.\r\n", ch );
      return;
   }

   if( get_timer( ch, TIMER_PKILLED ) > 0 )
   {
      send_to_char( "You can't drag a player right now.\r\n", ch );
      return;
   }

   if( arg[0] == '\0' )
   {
      send_to_char( "Drag whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You take yourself by the scruff of your neck, but go nowhere.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can only drag characters.\r\n", ch );
      return;
   }

   if( !xIS_SET( victim->act, PLR_SHOVEDRAG ) && !IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) )
   {
      send_to_char( "That character doesn't seem to appreciate your attentions.\r\n", ch );
      return;
   }

   if( get_timer( victim, TIMER_PKILLED ) > 0 )
   {
      send_to_char( "You can't drag that player right now.\r\n", ch );
      return;
   }

   if( victim->fighting )
   {
      send_to_char( "You try, but can't get close enough.\r\n", ch );
      return;
   }

   if( !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) && IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) )
   {
      send_to_char( "You cannot drag a deadly character.\r\n", ch );
      return;
   }

   if( !IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) && victim->position > 3 )
   {
      send_to_char( "They don't seem to need your assistance.\r\n", ch );
      return;
   }

   if( arg2[0] == '\0' )
   {
      send_to_char( "Drag them in which direction?\r\n", ch );
      return;
   }

   if( ch->level - victim->level > 5 || victim->level - ch->level > 5 )
   {
      if( IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) && IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
      {
         send_to_char( "There is too great an experience difference for you to even bother.\r\n", ch );
         return;
      }
   }

   exit_dir = get_dir( arg2 );

   if( xIS_SET( victim->in_room->room_flags, ROOM_SAFE ) && get_timer( victim, TIMER_SHOVEDRAG ) <= 0 )
   {
      send_to_char( "That character cannot be dragged right now.\r\n", ch );
      return;
   }

   nogo = FALSE;
   if( ( pexit = get_exit( ch->in_room, exit_dir ) ) == NULL )
      nogo = TRUE;
   else
      if( IS_SET( pexit->exit_info, EX_CLOSED )
          && ( !IS_AFFECTED( victim, AFF_PASS_DOOR ) || IS_SET( pexit->exit_info, EX_NOPASSDOOR ) ) )
      nogo = TRUE;
   if( nogo )
   {
      send_to_char( "There's no exit in that direction.\r\n", ch );
      return;
   }

   to_room = pexit->to_room;
   if( xIS_SET( to_room->room_flags, ROOM_DEATH ) )
   {
      send_to_char( "You cannot drag someone into a death trap.\r\n", ch );
      return;
   }

   if( ch->in_room->area != to_room->area && !in_hard_range( victim, to_room->area ) )
   {
      send_to_char( "That character cannot enter that area.\r\n", ch );
      return;
   }

/* Check for class, assign percentage based on that. */
   if( ch->Class == CLASS_WARRIOR )
      schance = 70;
   if( ch->Class == CLASS_VAMPIRE )
      schance = 65;
   if( ch->Class == CLASS_RANGER )
      schance = 60;
   if( ch->Class == CLASS_DRUID )
      schance = 45;
   if( ch->Class == CLASS_CLERIC )
      schance = 35;
   if( ch->Class == CLASS_THIEF )
      schance = 30;
   if( ch->Class == CLASS_MAGE )
      schance = 15;

/* Add 3 points to chance for every str point above 15, subtract for 
below 15 */

   schance += ( ( get_curr_str( ch ) - 15 ) * 3 );

   schance += ( ch->level - victim->level );

   if( ch->race == 1 )
      race_bonus = -3;

   if( ch->race == 2 )
      race_bonus = 3;

   if( ch->race == 3 )
      race_bonus = -5;

   if( ch->race == 4 )
      race_bonus = -7;

   if( ch->race == 6 )
      race_bonus = 5;

   if( ch->race == 7 )
      race_bonus = 7;

   if( ch->race == 8 )
      race_bonus = 10;

   if( ch->race == 9 )
      race_bonus = -2;

   schance += race_bonus;

   if( schance < number_percent(  ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   if( victim->position < POS_STANDING )
   {
      short temp;

      temp = victim->position;
      victim->position = POS_DRAG;
      act( AT_ACTION, "You drag $M into the next room.", ch, NULL, victim, TO_CHAR );
      act( AT_ACTION, "$n grabs your hair and drags you.", ch, NULL, victim, TO_VICT );
      move_char( victim, get_exit( ch->in_room, exit_dir ), 0 );
      if( !char_died( victim ) )
         victim->position = temp;
/* Move ch to the room too.. they are doing dragging - Scryn */
      move_char( ch, get_exit( ch->in_room, exit_dir ), 0 );
      WAIT_STATE( ch, 12 );
      return;
   }
   send_to_char( "You cannot do that to someone who is standing.\r\n", ch );
   return;
}
