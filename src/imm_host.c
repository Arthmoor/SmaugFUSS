/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
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
 *                        Immortal Host module by Shaddai                   *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "mud.h"

/*
 * This loads the Immortal host data structure to keep certain immortals
 * from logging on from certain sites...  IE keep hackers out of the high
 * level immortals.  Shaddai July 27, 1997
 */
int load_imm_host(  )
{
   FILE *fp;
   const char *word;
   bool my_continue = TRUE;
   bool fMatch = FALSE;
   IMMORTAL_HOST *host;

   if( ( fp = fopen( IMM_HOST_FILE, "r" ) ) == NULL )
   {
      log_string( "No immortal host data" );
      return rNONE;
   }

   while( my_continue )
   {
      word = feof( fp ) ? "#END" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '#':
            if( !str_cmp( word, "#END" ) )
            {
               fclose( fp );
               fMatch = TRUE;
               my_continue = FALSE;
            }
            break;

         case 'S':
            if( !str_cmp( word, "Start" ) )
            {
               CREATE( host, IMMORTAL_HOST, 1 );
               LINK( host, immortal_host_start, immortal_host_end, next, prev );
               fread_imm_host( fp, host );
               fMatch = TRUE;
            }
            break;
      }

      if( !fMatch )
      {
         bug( "%s: no match: %s", __FUNCTION__, word );
         fread_to_eol( fp );
      }
   }
   log_string( "Done." );
   return rNONE;
}

/* 
 * This function reads one immortal host to a file.
 * Shaddai  July 27, 1997
 */
int fread_imm_host( FILE * fp, IMMORTAL_HOST * data )
{
   const char *word;
   bool fMatch;
   int temp;

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case 'E':
            if( !str_cmp( word, "End" ) )
               return rNONE;

         case 'H':
            KEY( "Host", data->host, fread_string_nohash( fp ) );
            break;

         case 'N':
            KEY( "Name", data->name, fread_string_nohash( fp ) );
            break;

         case 'P':
            if( !str_cmp( word, "Prefix" ) )
            {
               temp = fread_number( fp );
               if( temp )
                  data->prefix = TRUE;
               else
                  data->prefix = FALSE;
               fMatch = TRUE;
            }
            break;

         case 'S':
            if( !str_cmp( word, "Suffix" ) )
            {
               temp = fread_number( fp );
               if( temp )
                  data->suffix = TRUE;
               else
                  data->suffix = FALSE;
               fMatch = TRUE;
            }
            break;
      }
      if( !fMatch )
      {
         bug( "%s: no match: %s", __FUNCTION__, word );
         fread_to_eol( fp );
      }
   }
}

bool check_immortal_domain( CHAR_DATA * ch, const char *host )
{
   bool found = FALSE;
   IMMORTAL_HOST *temp;
   char my_host[MAX_STRING_LENGTH];
   char my_name[MAX_STRING_LENGTH];
   unsigned int i = 0;

   for( i = 0; i < strlen( host ); i++ )
      my_host[i] = LOWER( host[i] );
   my_host[i] = '\0';

   for( i = 0; i < strlen( ch->name ); i++ )
      my_name[i] = LOWER( ch->name[i] );
   my_name[i] = '\0';

   for( temp = immortal_host_start; temp; temp = temp->next )
   {
      if( !str_cmp( my_name, strlower( temp->name ) ) )
      {
         found = TRUE;
         if( temp->prefix && temp->suffix && strstr( my_host, temp->host ) )
            return TRUE;
         else if( temp->prefix && !str_suffix( temp->host, my_host ) )
            return TRUE;
         else if( temp->suffix && !str_prefix( temp->host, my_host ) )
            return TRUE;
         else if( !str_cmp( temp->host, my_host ) )
         {
            return TRUE;
         }
      }
   }

   if( !found )
      return TRUE;
   else
      return FALSE;
}

void do_add_imm_host( CHAR_DATA* ch, const char* argument)
{
   char type[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char *arg3 = NULL;
   char *name;
   IMMORTAL_HOST *temp, *host;

   argument = one_argument( argument, type );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   set_char_color( AT_IMMORT, ch );

   if( type[0] == '\0' )
   {
      if( immortal_host_start == NULL )
      {
         send_to_char( "No immortals are protected at this time.\r\n", ch );
         return;
      }
      send_to_char( "Immortal   Host\r\n", ch );
      set_char_color( AT_PLAIN, ch );
      for( temp = immortal_host_start; temp; temp = temp->next )
         ch_printf( ch, "%-8s  %c%s%c\r\n",
                    temp->name, ( temp->prefix ? '*' : ' ' ), temp->host, ( temp->suffix ? '*' : ' ' ) );
      return;
   }

   /*
    * Ok we have a new entry make sure it doesn't contain a ~ 
    */
   if( !str_cmp( type, "save" ) )
   {
      do_write_imm_host(  );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( arg2[0] == '\0' || arg1[0] == '\0' )
   {
      send_to_char( "Syntax: immhost add    <name> <host>\r\n", ch );
      send_to_char( "Syntax: immhost delete <name> <host>\r\n", ch );
      send_to_char( "Syntax: immhost save\r\n", ch );
      return;
   }

   if( !str_cmp( type, "delete" ) )
   {
      IMMORTAL_HOST *it = NULL;

      arg3 = arg2;
      if( arg3[0] == '*' )
         arg3++;
      if( arg3[strlen( arg3 ) - 1] == '*' )
         arg3[strlen( arg3 ) - 1] = '\0';

      for( temp = immortal_host_start; temp; temp = temp->next )
      {
         if( !str_cmp( arg1, temp->name ) && !str_cmp( arg3, temp->host ) )
         {
            it = temp;
            break;
         }
      }
      if( it == NULL )
      {
         send_to_char( "Didn't find that entry.\r\n", ch );
         return;
      }
      DISPOSE( temp->name );
      DISPOSE( temp->host );
      UNLINK( it, immortal_host_start, immortal_host_end, next, prev );
      DISPOSE( it );
   }
   else if( !str_cmp( type, "add" ) )
   {
      bool prefix = FALSE, suffix = FALSE;
      int i;

      smash_tilde( arg1 );
      smash_tilde( arg2 );
      name = arg2;

      if( arg2[0] == '*' )
      {
         prefix = TRUE;
         name++;
      }

      if( name[strlen( name ) - 1] == '*' )
      {
         suffix = TRUE;
         name[strlen( name ) - 1] = '\0';
      }

      arg1[0] = toupper( arg1[0] );
      for( i = 0; i < ( int )strlen( name ); i++ )
         name[i] = LOWER( name[i] );
      for( temp = immortal_host_start; temp; temp = temp->next )
      {
         if( !str_cmp( temp->name, arg1 ) && !str_cmp( temp->host, name ) )
         {
            send_to_char( "Entry already exists.\r\n", ch );
            return;
         }
      }
      CREATE( host, IMMORTAL_HOST, 1 );
      host->name = str_dup( arg1 );
      host->host = str_dup( name );
      host->prefix = prefix;
      host->suffix = suffix;
      LINK( host, immortal_host_start, immortal_host_end, next, prev );
   }
   else
   {
      send_to_char( "Syntax: immhost add    <name> <host>\r\n", ch );
      send_to_char( "Syntax: immhost delete <name> <host>\r\n", ch );
      send_to_char( "Syntax: immhost save\r\n", ch );
      return;
   }
   send_to_char( "Done.\r\n", ch );
   return;
}

void do_write_imm_host( void )
{
   IMMORTAL_HOST *temp;
   FILE *fp;

   if( immortal_host_start == NULL )
   {
      unlink( IMM_HOST_FILE );
      return;
   }

   if( ( fp = fopen( IMM_HOST_FILE, "w" ) ) == NULL )
   {
      bug( "Do_write_imm_host: cant open %s", IMM_HOST_FILE );
      perror( IMM_HOST_FILE );
      return;
   }

   for( temp = immortal_host_start; temp; temp = temp->next )
   {
      fprintf( fp, "Start\n" );
      fprintf( fp, "Name\t%s~\n", temp->name );
      fprintf( fp, "Host\t%s~\n", temp->host );
      if( temp->prefix )
         fprintf( fp, "Prefix\t1\n" );
      if( temp->suffix )
         fprintf( fp, "Suffix\t1\n" );
      fprintf( fp, "End\n" );
   }
   fprintf( fp, "#END\n" );
   fclose( fp );
   return;
}
