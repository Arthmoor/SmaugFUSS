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
 *                          MSSP Plaintext Module                           *
 ****************************************************************************/

/******************************************************************
* Program writen by:                                              *
*  Greg (Keberus Maou'San) Mosley                                 *
*  Co-Owner/Coder SW: TGA                                         *
*  www.t-n-k-games.com                                            *
*                                                                 *
* Description:                                                    *
*  This program will allow admin to view and set thier MSSP       *
*  variables in game, and allows thier game to respond to a MSSP  *
*  Server with the MSSP-Plaintext protocol                        *
*******************************************************************
* What it does:                                                   *
*  Allows admin to set/view MSSP variables and transmits the MSSP *
*  information to anyone who does an MSSP-REQUEST at the login    *
*  screen                                                         *
*******************************************************************
* Special Thanks:                                                 *
*  A special thanks to Scandum for coming up with the MSSP        *
*  protocol, Cratylus for the MSSP-Plaintext idea, and Elanthis   *
*  for the GNUC_FORMAT idea ( which I like to use now ).          *
******************************************************************/

/*TERMS OF USE
         I only really have 2 terms...
 1. Give credit where it is due, keep the above header in your code 
    (you don't have to give me credit in mud) and if someone asks 
	don't lie and say you did it.
 2. If you have any comments or questions feel free to email me
    at keberus@gmail.com

  Thats All....
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "mud.h"
#include "mssp.h"

struct mssp_info *mssp_info;
void fread_mssp_info( FILE * fp );

void free_mssp_info( void )
{
   DISPOSE( mssp_info->hostname );
   DISPOSE( mssp_info->ip );
   DISPOSE( mssp_info->ipv6 );
   DISPOSE( mssp_info->contact );
   DISPOSE( mssp_info->icon );
   DISPOSE( mssp_info->language );
   DISPOSE( mssp_info->location );
   DISPOSE( mssp_info->website );
   DISPOSE( mssp_info->family );
   DISPOSE( mssp_info->genre );
   DISPOSE( mssp_info->subgenre );
   DISPOSE( mssp_info->gamePlay );
   DISPOSE( mssp_info->gameSystem );
   DISPOSE( mssp_info->intermud );
   DISPOSE( mssp_info->status );
   DISPOSE( mssp_info );
}

void save_mssp_info( void )
{
   FILE *fp;
   char filename[256];

   snprintf( filename, 256, "%s", MSSP_FILE );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "%s: can't open file", __func__ );
      perror( filename );
   }
   else
   {
      fprintf( fp, "%s", "#MSSP_INFO\n" );
      fprintf( fp, "Hostname          %s~\n", mssp_info->hostname );
      fprintf( fp, "IP                %s~\n", mssp_info->ip );
      fprintf( fp, "IPv6              %s~\n", mssp_info->ipv6 );
      fprintf( fp, "CrawlDelay        %d~\n", mssp_info->crawldelay );
      fprintf( fp, "Contact           %s~\n", mssp_info->contact );
      fprintf( fp, "Icon              %s~\n", mssp_info->icon );
      fprintf( fp, "Language          %s~\n", mssp_info->language );
      fprintf( fp, "Location          %s~\n", mssp_info->location );
      fprintf( fp, "Website           %s~\n", mssp_info->website );
      fprintf( fp, "Family            %s~\n", mssp_info->family );
      fprintf( fp, "Genre             %s~\n", mssp_info->genre );
      fprintf( fp, "SubGenre          %s~\n", mssp_info->subgenre );
      fprintf( fp, "GamePlay          %s~\n", mssp_info->gamePlay );
      fprintf( fp, "GameSystem        %s~\n", mssp_info->gameSystem );
      fprintf( fp, "Intermud          %s~\n", mssp_info->intermud );
      fprintf( fp, "Status            %s~\n", mssp_info->status );
      fprintf( fp, "Created           %d\n", mssp_info->created );
      fprintf( fp, "MinAge            %d\n", mssp_info->minAge );
      fprintf( fp, "Ansi              %d\n", mssp_info->ansi );
      fprintf( fp, "MCCP              %d\n", mssp_info->mccp );
      fprintf( fp, "MCP               %d\n", mssp_info->mcp );
      fprintf( fp, "MSP               %d\n", mssp_info->msp );
      fprintf( fp, "SSL               %d\n", mssp_info->ssl );
      fprintf( fp, "MXP               %d\n", mssp_info->mxp );
      fprintf( fp, "Vt100             %d\n", mssp_info->vt100 );
      fprintf( fp, "Xterm256          %d\n", mssp_info->xterm256 );
      fprintf( fp, "Pay2Play          %d\n", mssp_info->pay2play );
      fprintf( fp, "Pay4Perks         %d\n", mssp_info->pay4perks );
      fprintf( fp, "HiringBuilders    %d\n", mssp_info->hiringBuilders );
      fprintf( fp, "HiringCoders      %d\n", mssp_info->hiringCoders );
      fprintf( fp, "%s", "End\n\n" );
      fprintf( fp, "%s", "#END\n" );

      FCLOSE( fp );
   }
}

/*
 * Load the MSSP data file
 */
bool load_mssp_data( void )
{
   char filename[256];
   FILE *fp;
   bool found;

   CREATE( mssp_info, struct mssp_info, 1 );

   found = FALSE;
   snprintf( filename, 256, "%s", MSSP_FILE );

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
            bug( "%s: # not found.", __func__ );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "MSSP_INFO" ) )
         {
            fread_mssp_info( fp );
            break;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section.", __func__ );
            break;
         }
      }
      FCLOSE( fp );
   }
   return found;
}

void fread_mssp_info( FILE * fp )
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
            KEY( "Ansi", mssp_info->ansi, fread_number( fp ) );
            break;

         case 'C':
            KEY( "Contact", mssp_info->contact, fread_string_nohash( fp ) );
            KEY( "Created", mssp_info->created, fread_number( fp ) );
            KEY( "CrawlDelay", mssp_info->crawldelay, fread_number( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
               return;
            break;

         case 'F':
            KEY( "Family", mssp_info->family, fread_string_nohash( fp ) );
            break;

         case 'G':
            KEY( "Genre", mssp_info->genre, fread_string_nohash( fp ) );
            KEY( "GamePlay", mssp_info->gamePlay, fread_string_nohash( fp ) );
            KEY( "GameSystem", mssp_info->gameSystem, fread_string_nohash( fp ) );
            break;

         case 'H':
            KEY( "Hostname", mssp_info->hostname, fread_string_nohash( fp ) );
            KEY( "HiringBuilders", mssp_info->hiringBuilders, fread_number( fp ) );
            KEY( "HiringCoders", mssp_info->hiringCoders, fread_number( fp ) );
            break;

         case 'I':
            KEY( "Icon", mssp_info->icon, fread_string_nohash( fp ) );
            KEY( "Intermud", mssp_info->intermud, fread_string_nohash( fp ) );
            KEY( "IP", mssp_info->ip, fread_string_nohash( fp ) );
            KEY( "IPv6", mssp_info->ipv6, fread_string_nohash( fp ) );
            break;

         case 'L':
            KEY( "Language", mssp_info->language, fread_string_nohash( fp ) );
            KEY( "Location", mssp_info->location, fread_string_nohash( fp ) );
            break;

         case 'M':
            KEY( "MCCP", mssp_info->mccp, fread_number( fp ) );
            KEY( "MCP", mssp_info->mcp, fread_number( fp ) );
            KEY( "MinAge", mssp_info->minAge, fread_number( fp ) );
            KEY( "MSP", mssp_info->msp, fread_number( fp ) );
            KEY( "MXP", mssp_info->mxp, fread_number( fp ) );
            break;

         case 'P':
            KEY( "Pay2Play", mssp_info->pay2play, fread_number( fp ) );
            KEY( "Pay4Perks", mssp_info->pay4perks, fread_number( fp ) );
            break;

         case 'S':
            KEY( "SSL", mssp_info->ssl, fread_number( fp ) );
            KEY( "Status", mssp_info->status, fread_string_nohash( fp ) );
            KEY( "SubGenre", mssp_info->subgenre, fread_string_nohash( fp ) );
            break;

         case 'V':
            KEY( "Vt100", mssp_info->vt100, fread_number( fp ) );
            break;

         case 'W':
            KEY( "Website", mssp_info->website, fread_string_nohash( fp ) );
            break;

         case 'X':
            KEY( "Xterm256", mssp_info->xterm256, fread_number( fp ) );
            break;
      }
      if( !fMatch )
         bug( "%s: no match: %s", __func__, word );
   }
}

#define MSSP_YN( value )  ( (value) == 0 ? "No" : "Yes" )

void show_mssp( CHAR_DATA * ch )
{
   if( !ch )
   {
      bug( "%s: NULL ch", __func__ );
      return;
   }

   ch_printf( ch, "&zMudname           &W%s\r\n", sysdata.mud_name );
   ch_printf( ch, "&zHostname          &W%s\r\n", mssp_info->hostname );
   ch_printf( ch, "&zIP                &W%s\r\n", mssp_info->hostname );
   ch_printf( ch, "&zIPv6              &W%s\r\n", mssp_info->hostname );
   ch_printf( ch, "&Crawl Delay        &W%d\r\n", mssp_info->crawldelay );
   ch_printf( ch, "&zContact           &W%s\r\n", mssp_info->contact );
   ch_printf( ch, "&zIcon              &W%s\r\n", mssp_info->icon );
   ch_printf( ch, "&zLanguage          &W%s\r\n", mssp_info->language );
   ch_printf( ch, "&zLocation          &W%s\r\n", mssp_info->location );
   ch_printf( ch, "&zWebsite           &W%s\r\n", mssp_info->website );
   ch_printf( ch, "&zFamily            &W%s\r\n", mssp_info->family );
   ch_printf( ch, "&zCodebase          &W%s %s\r\n", CODENAME, CODEVERSION );
   ch_printf( ch, "&zGenre             &W%s\r\n", mssp_info->genre );
   ch_printf( ch, "&zSubGenre          &W%s\r\n", mssp_info->subgenre );
   ch_printf( ch, "&zGamePlay          &W%s\r\n", mssp_info->gamePlay );
   ch_printf( ch, "&zGameSystem        &W%s\r\n", mssp_info->gameSystem );
   ch_printf( ch, "&zIntermud          &W%s\r\n", mssp_info->intermud );
   ch_printf( ch, "&zStatus            &W%s\r\n", mssp_info->status );
   ch_printf( ch, "&zCreated           &W%d\r\n", mssp_info->created );
   ch_printf( ch, "&zMinAge            &W%d\r\n", mssp_info->minAge );
   ch_printf( ch, "&zAnsi              &W%s\r\n", MSSP_YN( mssp_info->ansi ) );
   ch_printf( ch, "&zMCCP              &W%s\r\n", MSSP_YN( mssp_info->mccp ) );
   ch_printf( ch, "&zMCP               &W%s\r\n", MSSP_YN( mssp_info->mcp ) );
   ch_printf( ch, "&zMSP               &W%s\r\n", MSSP_YN( mssp_info->msp ) );
   ch_printf( ch, "&zSSL               &W%s\r\n", MSSP_YN( mssp_info->ssl ) );
   ch_printf( ch, "&zMXP               &W%s\r\n", MSSP_YN( mssp_info->mxp ) );
   ch_printf( ch, "&zVt100             &W%s\r\n", MSSP_YN( mssp_info->vt100 ) );
   ch_printf( ch, "&zXterm256          &W%s\r\n", MSSP_YN( mssp_info->xterm256 ) );
   ch_printf( ch, "&zPay2Play          &W%s\r\n", MSSP_YN( mssp_info->pay2play ) );
   ch_printf( ch, "&zPay4Perks         &W%s\r\n", MSSP_YN( mssp_info->pay4perks ) );
   ch_printf( ch, "&zHiringBuilders    &W%s\r\n", MSSP_YN( mssp_info->hiringBuilders ) );
   ch_printf( ch, "&zHiringCoders      &W%s\r\n", MSSP_YN( mssp_info->hiringCoders ) );
}

void do_setmssp( CHAR_DATA *ch, const char* argument )
{
   char arg1[MIL];
   char **strptr = NULL;
   bool *ynptr = NULL;

   argument = one_argument( argument, arg1 );

   if( ( arg1[0] == '\0' ) || !str_cmp( arg1, "show" ) ) //Here you go Conner :)
   {
      show_mssp( ch );
      return;
   }
   if( !argument || ( argument[0] == '\0' ) )
   {
      send_to_char( "Syntax: setmssp <field> [value]\r\n", ch );
      send_to_char( "Field being one of:\r\n", ch );
      send_to_char( "hostname       ip                ipv6               contact          icon\r\n", ch );
      send_to_char( "language       location          website            family           genre\r\n", ch );
      send_to_char( "gameplay       game_system       intermud           status           subgenre\r\n", ch );
      send_to_char( "created        min_age           ansi               mccp             mcp\r\n", ch );
      send_to_char( "msp            ssl               mxp                vt100            xterm256\r\n", ch );
      send_to_char( "pay2play       pay4perks         hiring_builders    hiring_coders    crawldelay\r\n", ch );
      return;
   }

   if( !str_cmp( arg1, "hostname" ) )
      strptr = &mssp_info->hostname;
   else if( !str_cmp( arg1, "ip" ) )
      strptr = &mssp_info->ip;
   else if( !str_cmp( arg1, "ipv6" ) )
      strptr = &mssp_info->ipv6;
   else if( !str_cmp( arg1, "contact" ) )
      strptr = &mssp_info->contact;
   else if( !str_cmp( arg1, "icon" ) )
      strptr = &mssp_info->icon;
   else if( !str_cmp( arg1, "language" ) )
      strptr = &mssp_info->language;
   else if( !str_cmp( arg1, "location" ) )
      strptr = &mssp_info->location;
   else if( !str_cmp( arg1, "website" ) )
      strptr = &mssp_info->website;
   else if( !str_cmp( arg1, "family" ) )
      strptr = &mssp_info->family;
   else if( !str_cmp( arg1, "genre" ) )
      strptr = &mssp_info->genre;
   else if( !str_cmp( arg1, "gameplay" ) )
      strptr = &mssp_info->gamePlay;
   else if( !str_cmp( arg1, "game_system" ) )
      strptr = &mssp_info->gameSystem;
   else if( !str_cmp( arg1, "intermud" ) )
      strptr = &mssp_info->intermud;
   else if( !str_cmp( arg1, "status" ) )
      strptr = &mssp_info->status;
   else if( !str_cmp( arg1, "subgenre" ) )
      strptr = &mssp_info->subgenre;

   if( strptr != NULL )
   {
      DISPOSE( *strptr );
      *strptr = strdup( argument );
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }
   if( !str_cmp( arg1, "ansi" ) )
      ynptr = &mssp_info->ansi;
   else if( !str_cmp( arg1, "mccp" ) )
      ynptr = &mssp_info->mccp;
   else if( !str_cmp( arg1, "mcp" ) )
      ynptr = &mssp_info->mcp;
   else if( !str_cmp( arg1, "msp" ) )
      ynptr = &mssp_info->msp;
   else if( !str_cmp( arg1, "mxp" ) )
      ynptr = &mssp_info->mxp;
   else if( !str_cmp( arg1, "vt100" ) )
      ynptr = &mssp_info->vt100;
   else if( !str_cmp( arg1, "xterm256" ) )
      ynptr = &mssp_info->xterm256;
   else if( !str_cmp( arg1, "pay2play" ) )
      ynptr = &mssp_info->pay2play;
   else if( !str_cmp( arg1, "pay4perks" ) )
      ynptr = &mssp_info->pay4perks;
   else if( !str_cmp( arg1, "hiring_builders" ) )
      ynptr = &mssp_info->hiringBuilders;
   else if( !str_cmp( arg1, "hiring_coders" ) )
      ynptr = &mssp_info->hiringCoders;

   if( ynptr != NULL )
   {
      bool newvalue = FALSE;

      if( str_cmp( argument, "yes" ) && str_cmp( argument, "no" ) )
      {
         ch_printf( ch, "You must specify 'yes' or 'no' for the %s value!\r\n", arg1 );
         return;
      }
      newvalue = !str_cmp( argument, "yes" ) ? TRUE : FALSE;
      *ynptr = newvalue;
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }

   if( !str_cmp( arg1, "created" ) )
   {
      int value;

      value = atoi( argument );

      if( !is_number( argument ) || ( value < MSSP_MINCREATED ) || ( value > MSSP_MAXCREATED ) )
      {
         ch_printf( ch, "The value for created must be between %d and %d\r\n", MSSP_MINCREATED, MSSP_MAXCREATED );
         return;
      }
      mssp_info->created = value;
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }
   else if( !str_cmp( arg1, "min_age" ) )
   {
      int value;

      value = atoi( argument );

      if( !is_number( argument ) || ( value < MSSP_MINAGE ) || ( value > MSSP_MAXAGE ) )
      {
         ch_printf( ch, "The value for min_age must be between %d and %d\r\n", MSSP_MINAGE, MSSP_MAXAGE );
         return;
      }
      mssp_info->minAge = value;
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }
   else if( !str_cmp( arg1, "ssl" ) )
   {
      int value;

      value = atoi( argument );

      if( !is_number( argument ) || ( value < 1024 ) || ( value > 65535 ) )
      {
         send_to_char( "The value for SSL must be between 1024 and 65535.\r\n", ch );
         return;
      }
      mssp_info->ssl = value;
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }
   else if( !str_cmp( arg1, "crawldelay" ) )
   {
      int value;

      value = atoi( argument );

      if( !is_number( argument ) || ( value < -1 ) )
      {
         send_to_char( "The value for Crawl Delay must be >= -1.\r\n", ch );
         return;
      }
      mssp_info->crawldelay = value;
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }
   else
      do_setmssp( ch, "" );
}

void mssp_reply( DESCRIPTOR_DATA * d, const char *var, const char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH];
   va_list args;

   if( !d )
   {
      bug( "%s: NULL d", __func__ );
      return;
   }
   if( !var || var[0] == '\0' )
   {
      bug( "%s: NULL var", __func__ );
      return;
   }

   va_start( args, fmt );
   vsprintf( buf, fmt, args );
   va_end( args );

   descriptor_printf( d, "%s\t%s\r\n", var, buf );
}

extern time_t mud_start_time;
extern int top_area;
extern int top_help;
extern int top_room;
extern int top_reset;
extern int top_prog;
extern int top_mob_index;
extern int top_obj_index;
extern short num_skills;
extern int top_prog;

short player_count( void )
{
   DESCRIPTOR_DATA *d;
   short count = 0;

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected >= CON_PLAYING )
         ++count;
   }
   return count;
}

void send_mssp_data( DESCRIPTOR_DATA * d )
{
   if( !d )
   {
      bug( "%s: NULL d", __func__ );
      return;
   }

   write_to_descriptor( d, "\r\nMSSP-REPLY-START\r\n", 0 );

   mssp_reply( d, "NAME", "%s", sysdata.mud_name );
   mssp_reply( d, "HOSTNAME", "%s", mssp_info->hostname );
   mssp_reply( d, "IP", "%s", mssp_info->ip );
   mssp_reply( d, "IPV6", "%s", mssp_info->ipv6 );
   mssp_reply( d, "CRAWL DELAY", "%d", mssp_info->crawldelay );
   mssp_reply( d, "PORT", "%d", port );
   mssp_reply( d, "UPTIME", "%d", (int)mud_start_time );
   mssp_reply( d, "PLAYERS", "%d", player_count( ) );
   mssp_reply( d, "CODEBASE", "%s %s", CODENAME, CODEVERSION );
   mssp_reply( d, "CONTACT", "%s", mssp_info->contact );
   mssp_reply( d, "CREATED", "%d", mssp_info->created );
   mssp_reply( d, "ICON", "%s", mssp_info->icon );
   mssp_reply( d, "LANGUAGE", "%s", mssp_info->language );
   mssp_reply( d, "LOCATION", "%s", mssp_info->location );
   mssp_reply( d, "MINIMUM AGE", "%d", mssp_info->minAge );
   mssp_reply( d, "WEBSITE", "%s", mssp_info->website );
   mssp_reply( d, "FAMILY", "%s", mssp_info->family );
   mssp_reply( d, "GENRE", "%s", mssp_info->genre );
   mssp_reply( d, "SUBGENRE", "%s", mssp_info->subgenre );
   mssp_reply( d, "GAMEPLAY", "%s", mssp_info->gamePlay );
   mssp_reply( d, "GAMESYSTEM", "%s", mssp_info->gameSystem );
   mssp_reply( d, "INTERMUD", "%s", mssp_info->intermud );
   mssp_reply( d, "STATUS", "%s", mssp_info->status );
   mssp_reply( d, "AREAS", "%d", top_area );
   mssp_reply( d, "HELPFILES", "%d", top_help );
   mssp_reply( d, "MOBILES", "%d", top_mob_index );
   mssp_reply( d, "OBJECTS", "%d", top_obj_index );
   mssp_reply( d, "ROOMS", "%d", top_room );
   mssp_reply( d, "RESETS", "%d", top_reset );
   mssp_reply( d, "CLASSES", "%d", MAX_CLASS );
   mssp_reply( d, "LEVELS", "%d", MAX_LEVEL );
   mssp_reply( d, "RACES", "%d", MAX_RACE );
   mssp_reply( d, "SKILLS", "%d", num_skills );
   mssp_reply( d, "ANSI", "%d", mssp_info->ansi );
   mssp_reply( d, "MCCP", "%d", mssp_info->mccp );
   mssp_reply( d, "MCP", "%d", mssp_info->mcp );
   mssp_reply( d, "MSP", "%d", mssp_info->msp );
   mssp_reply( d, "SSL", "%d", mssp_info->ssl );
   mssp_reply( d, "MXP", "%d", mssp_info->mxp );
   mssp_reply( d, "VT100", "%d", mssp_info->vt100 );
   mssp_reply( d, "XTERM 256 COLORS", "%d", mssp_info->xterm256 );
   mssp_reply( d, "PAY TO PLAY", "%d", mssp_info->pay2play );
   mssp_reply( d, "PAY FOR PERKS", "%d", mssp_info->pay4perks );
   mssp_reply( d, "HIRING BUILDERS", "%d", mssp_info->hiringBuilders );
   mssp_reply( d, "HIRING CODERS", "%d", mssp_info->hiringCoders );

   write_to_descriptor( d, "MSSP-REPLY-END\r\n", 0 );
}
