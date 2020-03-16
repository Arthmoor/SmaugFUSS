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
 *                               Planes Module                              *
 ****************************************************************************/

#include <stdio.h>
#include "mud.h"

PLANE_DATA *first_plane, *last_plane;

void do_plist( CHAR_DATA* ch, const char* argument)
{
   PLANE_DATA *p;

   send_to_char( "Planes:\r\n-------\r\n", ch );
   for( p = first_plane; p; p = p->next )
      ch_printf( ch, "%s\r\n", p->name );
}

void do_pstat( CHAR_DATA* ch, const char* argument)
{
   PLANE_DATA *p;
   char arg[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg );
   if( !( p = plane_lookup( arg ) ) )
   {
      send_to_char( "Stat which plane?\r\n", ch );
      return;
   }
   ch_printf( ch, "Name: %s\r\n", p->name );
}

void do_pset( CHAR_DATA* ch, const char* argument)
{
   PLANE_DATA *p;
   char arg[MAX_INPUT_LENGTH];
   char mod[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "Syntax: pset <plane> create\r\n", ch );
      send_to_char( "        pset save\r\n", ch );
      send_to_char( "        pset <plane> delete\r\n", ch );
      send_to_char( "        pset <plane> <field> <value>\r\n", ch );
      send_to_char( "\r\n", ch );
      send_to_char( "  Where <field> is one of:\r\n", ch );
      send_to_char( "    name\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "save" ) )
   {
      save_planes(  );
      send_to_char( "Planes saved.\r\n", ch );
      return;
   }

   argument = one_argument( argument, mod );
   p = plane_lookup( arg );

   if( !str_prefix( mod, "create" ) )
   {
      if( p )
      {
         send_to_char( "Plane already exists.\r\n", ch );
         return;
      }
      CREATE( p, PLANE_DATA, 1 );
      p->name = STRALLOC( arg );
      LINK( p, first_plane, last_plane, next, prev );
      send_to_char( "Plane created.\r\n", ch );
      return;
   }

   if( !p )
   {
      send_to_char( "Plane doesn't exist.\r\n", ch );
      return;
   }

   if( !str_prefix( mod, "delete" ) )
   {
      UNLINK( p, first_plane, last_plane, next, prev );
      STRFREE( p->name );
      DISPOSE( p );
      check_planes( p );
      send_to_char( "Plane deleted.\r\n", ch );
      return;
   }

   if( !str_prefix( mod, "name" ) )
   {
      if( plane_lookup( argument ) )
      {
         send_to_char( "Another plane has that name.\r\n", ch );
         return;
      }
      STRFREE( p->name );
      p->name = STRALLOC( argument );
      send_to_char( "Name changed.\r\n", ch );
      return;
   }
   do_pset( ch, "" );
}

PLANE_DATA *plane_lookup( const char *name )
{
   PLANE_DATA *p;

   for( p = first_plane; p; p = p->next )
      if( !str_cmp( name, p->name ) )
         return p;

   for( p = first_plane; p; p = p->next )
      if( !str_prefix( name, p->name ) )
         return p;
   return NULL;
}

void save_planes( void )
{
   FILE *fp;
   PLANE_DATA *p;

   if( !( fp = fopen( PLANE_FILE, "w" ) ) )
   {
      perror( PLANE_FILE );
      bug( "%s: can't open plane file", __func__ );
      return;
   }

   for( p = first_plane; p; p = p->next )
   {
      fprintf( fp, "#PLANE\n" );
      fprintf( fp, "Name      %s\n", p->name );
      fprintf( fp, "End\n\n" );
   }
   fprintf( fp, "#END\n" );
   FCLOSE( fp );
}

void read_plane( FILE * fp )
{
   PLANE_DATA *p;
   const char *word;
   bool fMatch;

   CREATE( p, PLANE_DATA, 1 );

   for( ;; )
   {
      word = ( feof( fp ) ? "End" : fread_word( fp ) );
      fMatch = FALSE;

      switch ( UPPER( *word ) )
      {
         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( plane_lookup( p->name ) )
               {
                  bug( "%s: duplicate plane name!", __func__ );
                  STRFREE( p->name );
                  DISPOSE( p );
               }
               else
                  LINK( p, first_plane, last_plane, next, prev );
               return;
            }
            break;

         case 'N':
            KEY( "Name", p->name, fread_string( fp ) );
            break;
      }

      if( !fMatch )
      {
         bug( "%s: unknown field '%s'", __func__, word );
         fread_to_eol( fp );
      }
   }
}

void load_planes( void )
{
   char *word;
   FILE *fp;

   if( !( fp = fopen( PLANE_FILE, "r" ) ) )
   {
      perror( PLANE_FILE );
      bug( "%s: can't open plane file for read.", __func__ );
      return;
   }
   for( ; !feof( fp ); )
   {
      if( fread_letter( fp ) != '#' )
      {
         bug( "%s: # not found.", __func__ );
         break;
      }

      word = fread_word( fp );
      if( !str_cmp( word, "END" ) )
         break;
      else if( !str_cmp( word, "PLANE" ) )
         read_plane( fp );
      else
      {
         bug( "%s: invalid section '%s'", __func__, word );
         break;
      }
   }
   FCLOSE( fp );
   fpArea = NULL;
}

void build_prime_plane( void )
{
   PLANE_DATA *p;

   CREATE( p, PLANE_DATA, 1 );
   p->name = STRALLOC( "Prime Material" );
   LINK( p, first_plane, last_plane, next, prev );
   return;
}

void check_planes( PLANE_DATA * p )
{
   extern ROOM_INDEX_DATA *room_index_hash[];
   int vnum;
   ROOM_INDEX_DATA *r;

   if( !first_plane )
      build_prime_plane(  );

   for( vnum = 0; vnum < MAX_KEY_HASH; ++vnum )
      for( r = room_index_hash[vnum]; r; r = r->next )
         if( !r->plane || r->plane == p )
            r->plane = first_plane;
   return;
}

void free_plane( PLANE_DATA * p )
{
   if( !p )
      return;
   UNLINK( p, first_plane, last_plane, next, prev );
   STRFREE( p->name );
   DISPOSE( p );
   return;
}

void free_all_planes( void )
{
   PLANE_DATA *p, *p_next;

   for( p = first_plane; p; p = p_next )
   {
      p_next = p->next;
      free_plane( p );
   }
   return;
}
