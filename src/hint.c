/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
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
 *                 Online Hints module by Zedd of Slips in Time             *
 ****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "mud.h"
#include "hint.h"

HINT_DATA *first_hint;
HINT_DATA *last_hint;

void SwapHint( HINT_DATA * pHint1, HINT_DATA * pHint2 )
{
   if( pHint1->prev )
   {
      pHint1->prev->next = pHint2;
   }
   else
   {
      first_hint = pHint2;
   }

   pHint2->prev = pHint1->prev;

   pHint1->next = pHint2->next;

   if( pHint1->next )
   {
      pHint1->next->prev = pHint1;
   }

   pHint2->next = pHint1;
   pHint1->prev = pHint2;
}

const char *get_hint( int level )
{
   HINT_DATA *hintData;
   bool found = FALSE;
   static char buf[MAX_STRING_LENGTH];
   int count, which;

   count = 0;
   if( level < 0 )
   {
      snprintf( buf, MAX_STRING_LENGTH, "HintLevel error, Level was %d", level );
      return buf;
   }
   else
   {
      found = FALSE;
      hintData = first_hint;
      for( hintData = first_hint; hintData; hintData = hintData->next )
      {
         if( level >= hintData->low && level <= hintData->high )
            ++count;
      }
      if( count > 1 )
      {
         which = number_range( 1, count );
         count = 0;
         for( hintData = first_hint; hintData; hintData = hintData->next )
         {
            if( level >= hintData->low && level <= hintData->high )
               ++count;
            if( count == which )
            {
               mudstrlcpy( buf, hintData->text, MAX_STRING_LENGTH );
               return buf;
            }
         }
      }
      else if( count == 1 )
      {
         for( hintData = first_hint; hintData; hintData = hintData->next )
         {
            if( level >= hintData->low && level <= hintData->high )
            {
               mudstrlcpy( buf, hintData->text, MAX_STRING_LENGTH );
               return buf;
            }
         }
      }
      else
         return " ";
   }
   return " ";
}

void write_hint( void )
{
   HINT_DATA *hintData;
   FILE *fp;
   char filename[256];

   sprintf( filename, "%s", HINT_FILE );
   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "%s: fopen", __FUNCTION__ );
      perror( filename );
      return;
   }
   else
   {
      for( hintData = first_hint; hintData; hintData = hintData->next )
      {
         fprintf( fp, "Text %s~\n", hintData->text );
         fprintf( fp, "Low  %d\n", hintData->low );
         fprintf( fp, "High %d\n", hintData->high );
         fprintf( fp, "%s", "End\n" );
      }
      fclose( fp );
      fp = NULL;
      return;
   }
}

void do_hintedit( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_STRING_LENGTH];
   char arg2[MAX_STRING_LENGTH];
   char arg3[MAX_STRING_LENGTH];
   HINT_DATA *hintData;
   int i;
   int no = 0;
   int ano = 0;
   bool found = FALSE;

   if( IS_NPC( ch ) )
      return;

   if( !IS_IMMORTAL( ch ) )
      return;

   set_char_color( AT_LBLUE, ch );
   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   if( !str_cmp( arg, "help" ) || arg[0] == '\0' )
   {
      do_help( ch, "imm_hints" );
      return;
   }

   if( !str_cmp( arg, "list" ) )
   {
      if( first_hint )
      {
         pager_printf( ch, "No | Low | High |            Text             \r\n" );
         pager_printf( ch, "---|-----|------|--------------------------------------------------\r\n" );
         i = 0;
         for( hintData = first_hint; hintData; hintData = hintData->next )
         {
            ++i;
            pager_printf( ch, "%2d | %3d | %4d | %-30s\r\n", i, hintData->low, hintData->high, hintData->text );
         }
         pager_printf( ch, "\r\n%d hints in file.\r\n", i );
      }
      else
         send_to_char( "No hints in file.\r\n", ch );
      return;
   }

   else if( !str_cmp( arg, "remove" ) )
   {
      no = 0;
      if( !is_number( arg2 ) )
      {
         send_to_char_color( "Remove which hint?\r\n", ch );
         return;
      }
      ano = atoi( arg2 );
      found = FALSE;
      for( hintData = first_hint; hintData; hintData = hintData->next )
      {
         ++no;
         if( no == ano )
         {
            ch_printf_color( ch, "&CHint Number %d removed\r\n", ano );
            UNLINK( hintData, first_hint, last_hint, next, prev );
            STRFREE( hintData->text );
            DISPOSE( hintData );
            found = TRUE;
            break;
         }
      }
      if( !found )
      {
         send_to_char( "Hint not found\r\n", ch );
         return;
      }
      return;
   }
   else if( !str_cmp( arg, "add" ) )
   {
      if( arg2 == '\0' )
      {
         send_to_char( "What is the minimum level for this hint?\r\n", ch );
         return;
      }
      if( arg3 == '\0' )
      {
         send_to_char( "What is the maximum level for this hint?\r\n", ch );
         return;
      }
      if( atoi( arg2 ) > atoi( arg3 ) )
      {
         send_to_char( "Aborting:  max less than min!\r\n", ch );
         return;
      }
      CREATE( hintData, HINT_DATA, 1 );
      hintData->low = atoi( arg2 );
      hintData->high = atoi( arg3 );
      hintData->text = STRALLOC( argument );
      LINK( hintData, first_hint, last_hint, next, prev );
      send_to_char( "Ok.  Hint created\r\n", ch );
      return;
   }
   else if( !str_cmp( arg, "force" ) )
   {
      ch_printf_color( ch, "&p( &wHINT&p ):  &P%s\r\n", get_hint( LEVEL_AVATAR ) );
      return;
   }
   else if( !str_cmp( arg, "edit" ) )
   {
      no = 0;
      i = 0;

      if( arg2[0] == '\0' )
      {
         send_to_char( "Edit which hint number?\r\n", ch );
         return;
      }
      else
         no = atoi( arg2 );
      if( arg3[0] == '\0' )
      {
         ch_printf( ch, "Edit which field of hint %d (low/high/text)?\r\n", no );
         return;
      }
      if( argument[0] == '\0' )
      {
         ch_printf( ch, "Change hint %d's field %s to what ?\r\n", no, arg3 );
         return;
      }
      for( hintData = first_hint; hintData; hintData = hintData->next )
      {
         ++i;
         if( i == no )
         {
            found = TRUE;
            break;
         }
      }
      if( !found )
      {
         ch_printf( ch, "Hint %d not found.\r\n", no );
         return;
      }
      else
      {
         if( !str_cmp( arg3, "text" ) )
         {
            STRFREE( hintData->text );
            hintData->text = STRALLOC( argument );
            send_to_char( "Hint text changed!\r\n", ch );
            return;
         }
         else if( !str_cmp( arg3, "low" ) )
         {
            if( atoi( argument ) > hintData->high )
            {
               send_to_char( "Aborting:  min higher than max.\r\n", ch );
               return;
            }
            hintData->low = atoi( argument );
            send_to_char( "Minimum level for hint changed.\r\n", ch );
            return;
         }
         else if( !str_cmp( arg3, "high" ) )
         {
            if( atoi( argument ) < hintData->low )
            {
               send_to_char( "Aborting:  max lower than min.\r\n", ch );
               return;
            }
            hintData->high = atoi( argument );
            send_to_char( "Maximum level for hint changed.\r\n", ch );
            return;
         }
         else
         {
            send_to_char( "Valid fields are:  low/high/text\r\n", ch );
            return;
         }
      }
   }
   else if( !str_cmp( arg, "save" ) )
   {
      write_hint(  );
      send_to_char( "Saved.\r\n", ch );
      return;
   }
   else
   {
      send_to_char( "Syntax:  hint (list/add/remove/edit/save/force)\r\n", ch );
      return;
   }
}

HINT_DATA *read_hint( char *filename, FILE * fp )
{
   HINT_DATA *hintData;
   const char *word;
   bool fMatch;
   char letter;

   do
   {
      letter = getc( fp );
      if( feof( fp ) )
      {
         fclose( fp );
         fp = NULL;
         return NULL;
      }
   }
   while( isspace( letter ) );
   ungetc( letter, fp );

   CREATE( hintData, HINT_DATA, 1 );
   hintData->next = NULL;
   hintData->prev = NULL;
   hintData->text = STRALLOC( "" );
   hintData->low = 0;
   hintData->high = 0;

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case 'T':
            if( !str_cmp( word, "Text" ) )
               STRFREE( hintData->text );
            KEY( "Text", hintData->text, fread_string( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !hintData->text )
                  hintData->text = STRALLOC( "" );
               return hintData;
            }
            break;

         case 'H':
            KEY( "High", hintData->high, fread_number( fp ) );
            break;

         case 'L':
            KEY( "Low", hintData->low, fread_number( fp ) );
            break;
      }

      if( !fMatch )
         bug( "%s: no match: %s", __FUNCTION__, word );
   }
   STRFREE( hintData->text );
   DISPOSE( hintData );
   return NULL;
}

void load_hint( void )
{
   char filename[256];
   FILE *fp;
   HINT_DATA *hintData;

   first_hint = last_hint = NULL;

   snprintf( filename, 256, "%s", HINT_FILE );
   if( ( fp = fopen( filename, "r" ) ) == NULL )
      return;

   while( ( hintData = read_hint( filename, fp ) ) != NULL )
      LINK( hintData, first_hint, last_hint, next, prev );

   return;
}
