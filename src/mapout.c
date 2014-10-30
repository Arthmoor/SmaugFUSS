/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * v. 0.9: 6/19/95:  Converts an ascii map to rooms.                        *
 * v. 1.0: 7/05/95:  Read/write maps to .are files.  Efficient storage.     *
 *	             Room qualities based on map code. Can add & remove rms *
 *	                from a map. (Somewhat) intelligent exit decisions.  *
 * v. 1.1: 7/11/95:  Various display options.  See comments over draw_map   *
 *	                                                                    *
 ****************************************************************************/

#include <stdio.h>
#include "mud.h"

struct map_data   /* contains per-room data */
{
   int vnum;   /* which map this room belongs to */
   int x;   /* horizontal coordinate */
   int y;   /* vertical coordinate */
   char entry; /* code that shows up on map */
};

struct map_index_data
{
   MAP_INDEX_DATA *next;
   int vnum;   /* vnum of the map */
   int map_of_vnums[49][81];  /* room vnums aranged as a map */
};

MAP_INDEX_DATA *get_map_index( int vnum );
void init_maps( void );

/* Useful Externals */
extern int top_exit;
void note_attach( CHAR_DATA * ch );

/* Local function prototypes */
MAP_INDEX_DATA *make_new_map_index( int vnum );
void map_to_rooms( CHAR_DATA * ch, MAP_INDEX_DATA * m_index );
int num_rooms_avail( CHAR_DATA * ch );

int number_to_room_num( int array_index );
int exit_lookup( int vnum1, int vnum2 );
void draw_map( CHAR_DATA * ch, ROOM_INDEX_DATA * rm, int flag, int mode );
char *you_are_here( int row, int col, char *map );

/* Local Variables & Structs */
char text_map[MAX_STRING_LENGTH];
extern MAP_INDEX_DATA *first_map;   /* should be global */
struct map_stuff
{
   int vnum;
   int proto_vnum;
   int exits;
   int index;
   char code;
};

/* Lets make it check the map and make sure it uses [ ] instead of [] */
const char *check_map( const char *str )
{
   static char newstr[MAX_STRING_LENGTH];
   int i, j;

   for( i = j = 0; str[i] != '\0'; i++ )
   {
      if( str[i - 1] == '[' && str[i] == ']' )
      {
         newstr[j++] = ' ';
         newstr[j++] = str[i];
      }
      else
         newstr[j++] = str[i];
   }
   newstr[j] = '\0';
   return newstr;
}

/* Be careful not to give
 * this an existing map_index
 */
MAP_INDEX_DATA *make_new_map_index( int vnum )
{
   MAP_INDEX_DATA *map_index;
   int i, j;

   CREATE( map_index, MAP_INDEX_DATA, 1 );
   map_index->vnum = vnum;
   for( i = 0; i < 49; i++ )
   {
      for( j = 0; j < 78; j++ )
         map_index->map_of_vnums[i][j] = -1;
   }
   map_index->next = first_map;
   first_map = map_index;
   return map_index;
}

char count_lines( char *txt )
{
   int i;
   char *c, buf[MAX_STRING_LENGTH];

   if( !txt )
      return ( char )'0';
   i = 1;
   for( c = txt; *c != '\0'; c++ )
      if( *c == '\n' )
         i++;
   if( i > 9 )
      return ( char )'+';
   snprintf( buf, MAX_STRING_LENGTH, "%d", i );
   return ( buf[0] );
}

MAP_INDEX_DATA *get_map_index( int vnum )
{
   MAP_INDEX_DATA *map;

   for( map = first_map; map; map = map->next )
   {
      if( map->vnum == vnum )
         return map;
   }
   return NULL;
}

void map_stats( CHAR_DATA * ch, int *rooms, int *rows, int *cols )
{
   int row, col, n;
   int leftmost, rightmost;
   const char *l;
   char c;

   if( !ch->pnote )
   {
      bug( "%s", "map_stats: ch->pnote == NULL!" );
      return;
   }
   n = 0;
   row = col = 0;
   leftmost = rightmost = 0;
   l = ch->pnote->text;
   do
   {
      c = l[0];
      switch ( c )
      {
         case '\n':
            break;
         case '\r':
            col = 0;
            row++;
            break;
         case ' ':
            col++;
            break;
      }
      if( c == '[' )
      {
         /*
          * Increase col 
          */
         col++;
         /*
          * This is a room since in [ ] 
          */
         n++;
         /*
          * This will later handle a letter etc 
          */
         col++;
         /*
          * Make sure it has a closeing ] 
          */
         if( c == ']' )
            col++;
         if( col < leftmost )
            leftmost = col;
         if( col > rightmost )
            rightmost = col;
      }
      if( ( c == ' ' || c == '-' || c == '|' || c == '=' || c == '\\' || c == '/' || c == '^' || c == ':' ) )
         col++;
      l++;
   }
   while( c != '\0' );
   *cols = ( rightmost - leftmost );
   *rows = row;   /* [sic.] */
   *rooms = n;
   return;
}

void do_mapout( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *map_obj;   /* an obj made with map as an ed */
   OBJ_INDEX_DATA *map_obj_index;   /*    obj_index for previous     */
   EXTRA_DESCR_DATA *ed;   /*    the ed for it to go in     */
   int rooms, rows, cols, avail_rooms;

   if( !ch )
   {
      bug( "%s", "do_mapout: null ch" );
      return;
   }
   if( IS_NPC( ch ) )
   {
      send_to_char( "Not in mobs.\r\n", ch );
      return;
   }
   if( !ch->desc )
   {
      bug( "%s", "do_mapout: no descriptor" );
      return;
   }
   switch ( ch->substate )
   {
      default:
         break;
      case SUB_WRITING_NOTE:
         if( ch->dest_buf != ch->pnote )
            bug( "%s", "do_mapout: sub_writing_map: ch->dest_buf != ch->pnote" );
         STRFREE( ch->pnote->text );
         ch->pnote->text = copy_buffer( ch );
         stop_editing( ch );
         return;
   }

   set_char_color( AT_NOTE, ch );
   argument = one_argument( argument, arg );
   smash_tilde( argument );

   if( !str_cmp( arg, "stat" ) )
   {
      if( !ch->pnote )
      {
         send_to_char( "You have no map in progress.\r\n", ch );
         return;
      }
      map_stats( ch, &rooms, &rows, &cols );
      ch_printf( ch, "Map represents %d rooms, %d rows, and %d columns\r\n", rooms, rows, cols );
      avail_rooms = num_rooms_avail( ch );
      ch_printf( ch, "You currently have %d unused rooms.\r\n", avail_rooms );
      act( AT_ACTION, "$n glances at an etherial map.", ch, NULL, NULL, TO_ROOM );
      return;
   }


   if( !str_cmp( arg, "write" ) )
   {
      note_attach( ch );
      ch->substate = SUB_WRITING_NOTE;
      ch->dest_buf = ch->pnote;
      start_editing( ch, ch->pnote->text );
      return;
   }
   if( !str_cmp( arg, "clear" ) )
   {
      if( !ch->pnote )
      {
         send_to_char( "You have no map in progress\r\n", ch );
         return;
      }
      STRFREE( ch->pnote->text );
      STRFREE( ch->pnote->subject );
      STRFREE( ch->pnote->to_list );
      STRFREE( ch->pnote->date );
      STRFREE( ch->pnote->sender );
      DISPOSE( ch->pnote );
      ch->pnote = NULL;
      send_to_char( "Map cleared.\r\n", ch );
      return;
   }
   if( !str_cmp( arg, "show" ) )
   {
      if( !ch->pnote )
      {
         send_to_char( "You have no map in progress.\r\n", ch );
         return;
      }
      send_to_char( ch->pnote->text, ch );
      do_mapout( ch, "stat" );
      return;
   }
   if( !str_cmp( arg, "create" ) )
   {
      if( !ch->pnote )
      {
         send_to_char( "You have no map in progress.\r\n", ch );
         return;
      }
      map_stats( ch, &rooms, &rows, &cols );
      avail_rooms = num_rooms_avail( ch );

      /*
       * check for not enough rooms 
       */
      if( rooms > avail_rooms )
      {
         send_to_char( "You don't have enough unused rooms allocated!\r\n", ch );
         return;
      }
      act( AT_ACTION, "$n warps the very dimensions of space!", ch, NULL, NULL, TO_ROOM );

      map_to_rooms( ch, NULL );  /* this does the grunt work */

      map_obj_index = get_obj_index( 91 );
      if( map_obj_index )
      {
         map_obj = create_object( map_obj_index, 0 );
         ed = SetOExtra( map_obj, "runes map scrawls" );
         STRFREE( ed->description );
         ed->description = QUICKLINK( ch->pnote->text );
         obj_to_char( map_obj, ch );
      }
      else
      {
         send_to_char( "Couldn't give you a map object.  Need Great Eastern Desert\r\n", ch );
         return;
      }

      do_mapout( ch, "clear" );
      send_to_char( "Ok.\r\n", ch );
      return;
   }
   send_to_char( "mapout write: create a map in edit buffer.\r\n", ch );
   send_to_char( "mapout stat: get information about a written, but not yet created map.\r\n", ch );
   send_to_char( "mapout clear: clear a written, but not yet created map.\r\n", ch );
   send_to_char( "mapout show: show a written, but not yet created map.\r\n", ch );
   send_to_char( "mapout create: turn a written map into rooms in your assigned room vnum range.\r\n", ch );
   return;
}

int add_new_room_to_map( CHAR_DATA * ch, char code )
{
   int i;
   ROOM_INDEX_DATA *location;

   /*
    * Get an unused room to copy to 
    */
   for( i = ch->pcdata->area->low_r_vnum; i <= ch->pcdata->area->hi_r_vnum; i++ )
   {
      if( get_room_index( i ) == NULL )
      {
         if( !( location = make_room( i, ch->pcdata->area ) ) )
         {
            bug( "%s: make_room failed", __FUNCTION__ );
            return -1;
         }
         /*
          * Clones current room  (quietly) 
          */
         location->area = ch->pcdata->area;
         location->name = STRALLOC( "Newly Created Room" );
         location->description = STRALLOC( "Newly Created Room\r\n" );
         xSET_BIT( location->room_flags, ROOM_PROTOTYPE );
         location->light = 0;
         if( code == 'I' )
            location->sector_type = SECT_INSIDE;
         else if( code == 'C' )
            location->sector_type = SECT_CITY;
         else if( code == 'f' )
            location->sector_type = SECT_FIELD;
         else if( code == 'F' )
            location->sector_type = SECT_FOREST;
         else if( code == 'H' )
            location->sector_type = SECT_HILLS;
         else if( code == 'M' )
            location->sector_type = SECT_MOUNTAIN;
         else if( code == 's' )
            location->sector_type = SECT_WATER_SWIM;
         else if( code == 'S' )
            location->sector_type = SECT_WATER_NOSWIM;
         else if( code == 'A' )
            location->sector_type = SECT_AIR;
         else if( code == 'D' )
            location->sector_type = SECT_DESERT;
         else if( code == 'U' )
            location->sector_type = SECT_DUNNO;
         else if( code == 'O' )
            location->sector_type = SECT_OCEANFLOOR;
         else if( code == 'u' )
            location->sector_type = SECT_UNDERGROUND;
         else if( code == 'U' )
            location->sector_type = SECT_UNDERWATER;
         else if( code == 'L' )
            location->sector_type = SECT_LAVA;
         else if( code == 'W' )
            location->sector_type = SECT_SWAMP;
         else
            location->sector_type = SECT_DUNNO;
         return i;
      }
   }
   send_to_char( "No available room in your vnums!", ch );
   return -1;
}

int num_rooms_avail( CHAR_DATA * ch )
{
   int i, n;

   n = 0;
   for( i = ch->pcdata->area->low_r_vnum; i <= ch->pcdata->area->hi_r_vnum; i++ )
      if( !get_room_index( i ) )
         n++;
   return n;
}

/* This function takes the character string in ch->pnote and
 *  creates rooms laid out in the appropriate configuration.
 */
void map_to_rooms( CHAR_DATA * ch, MAP_INDEX_DATA * m_index )
{
   struct map_stuff map[49][78]; /* size of edit buffer */
   const char *newmap;
   int row, col, i, n, x, y, tvnum, proto_vnum = 0;
   int newx, newy;
   const char *l;
   char c;
   ROOM_INDEX_DATA *newrm;
   MAP_INDEX_DATA *map_index = NULL, *tmp;
   EXIT_DATA *xit;   /* these are for exits */
   bool getroomnext = FALSE;

   if( !ch->pnote )
   {
      bug( "%s: ch->pnote==NULL!", __FUNCTION__ );
      return;
   }

   /*
    * Make sure format is right 
    */
   newmap = check_map( ch->pnote->text );
   STRFREE( ch->pnote->text );
   ch->pnote->text = STRALLOC( newmap );

   n = 0;
   row = col = 0;

   /*
    * Check to make sure map_index exists.  
    * If not, then make a new one.
    */
   if( !m_index )
   {
      /*
       * Make a new vnum 
       */
      for( i = ch->pcdata->area->low_r_vnum; i <= ch->pcdata->area->hi_r_vnum; i++ )
      {
         if( ( tmp = get_map_index( i ) ) == NULL )
         {
            map_index = make_new_map_index( i );
            break;
         }
      }
   }
   else
      map_index = m_index;

   /*
    *  
    */
   if( !map_index )
   {
      send_to_char( "Couldn't find or make a map_index for you!\r\n", ch );
      bug( "%s", "map_to_rooms: Couldn't find or make a map_index\r\n" );
      /*
       * do something. return failed or somesuch 
       */
      return;
   }

   for( x = 0; x < 49; x++ )
   {
      for( y = 0; y < 78; y++ )
      {
         map[x][y].vnum = 0;
         map[x][y].proto_vnum = 0;
         map[x][y].exits = 0;
         map[x][y].index = 0;
      }
   }

   l = ch->pnote->text;
   do
   {
      c = l[0];
      switch ( c )
      {
         case '\n':
            break;
         case '\r':
            col = 0;
            row++;
            break;
      }
      if( c != ' ' && c != '-' && c != '|' && c != '=' && c != '\\' && c != '/' && c != '^'
          && c != ':' && c != '[' && c != ']' && c != '^' && !getroomnext )
      {
         l++;
         continue;
      }
      if( getroomnext )
      {
         n++;
         /*
          * Actual room info 
          */
         map[row][col].vnum = add_new_room_to_map( ch, c );
         map_index->map_of_vnums[row][col] = map[row][col].vnum;
         map[row][col].proto_vnum = proto_vnum;
         getroomnext = FALSE;
      }
      else
      {
         map_index->map_of_vnums[row][col] = 0;
         map[row][col].vnum = 0;
         map[row][col].exits = 0;
      }
      map[row][col].code = c;
      /*
       * Handle rooms 
       */
      if( c == '[' )
         getroomnext = TRUE;
      col++;
      l++;
   }
   while( c != '\0' );

   for( y = 0; y < ( row + 1 ); y++ )
   {  /* rows */
      for( x = 0; x < 78; x++ )
      {  /* cols (78, i think) */

         if( map[y][x].vnum == 0 )
            continue;

         newrm = get_room_index( map[y][x].vnum );
         /*
          * Continue if no newrm 
          */
         if( !newrm )
            continue;

         /*
          * Check up 
          */
         if( y > 1 )
         {
            newx = x;
            newy = y;
            newy--;
            while( newy >= 0 && ( map[newy][x].code == '^' ) )
               newy--;

            /*
             * dont link it to itself 
             */
            if( map[y][x].vnum == map[newy][x].vnum )
               break;
            if( ( tvnum = map[newy][x].vnum ) != 0 )
            {
               xit = make_exit( newrm, get_room_index( tvnum ), DIR_UP );
               xit->keyword = STRALLOC( "" );
               xit->description = STRALLOC( "" );
               xit->key = -1;
               xit->exit_info = 0;
            }
         }

         /*
          * Check down 
          */
         if( y < 48 )
         {
            newx = x;
            newy = y;
            newy++;
            while( newy <= 48 && ( map[newy][x].code == '^' ) )
               newy++;
            /*
             * dont link it to itself 
             */
            if( map[y][x].vnum == map[newy][x].vnum )
               break;
            if( ( tvnum = map[newy][x].vnum ) != 0 )
            {
               xit = make_exit( newrm, get_room_index( tvnum ), DIR_DOWN );
               xit->keyword = STRALLOC( "" );
               xit->description = STRALLOC( "" );
               xit->key = -1;
               xit->exit_info = 0;
            }
         }

         /*
          * Check north 
          */
         if( y > 1 )
         {
            newx = x;
            newy = y;
            newy--;
            while( newy >= 0 && ( map[newy][x].code == '|' || map[newy][x].code == ':' || map[newy][x].code == '=' ) )
               newy--;
            /*
             * dont link it to itself 
             */
            if( map[y][x].vnum == map[newy][x].vnum )
               break;
            if( ( tvnum = map[newy][x].vnum ) != 0 )
            {
               xit = make_exit( newrm, get_room_index( tvnum ), DIR_NORTH );
               xit->keyword = STRALLOC( "" );
               xit->description = STRALLOC( "" );
               xit->key = -1;
               if( map[newy + 1][x].code == ':' || map[newy + 1][x].code == '=' )
               {
                  SET_BIT( xit->exit_info, EX_ISDOOR );
                  SET_BIT( xit->exit_info, EX_CLOSED );
               }
               else
                  xit->exit_info = 0;
            }
         }

         /*
          * Check south 
          */
         if( y < 48 )
         {
            newx = x;
            newy = y;
            newy++;
            while( newy <= 48 && ( map[newy][x].code == '|' || map[newy][x].code == ':' || map[newy][x].code == '=' ) )
               newy++;
            /*
             * dont link it to itself 
             */
            if( map[y][x].vnum == map[newy][x].vnum )
               break;
            if( ( tvnum = map[newy][x].vnum ) != 0 )
            {
               xit = make_exit( newrm, get_room_index( tvnum ), DIR_SOUTH );
               xit->keyword = STRALLOC( "" );
               xit->description = STRALLOC( "" );
               xit->key = -1;
               if( map[newy - 1][x].code == ':' || map[newy - 1][x].code == '=' )
               {
                  SET_BIT( xit->exit_info, EX_ISDOOR );
                  SET_BIT( xit->exit_info, EX_CLOSED );
               }
               else
                  xit->exit_info = 0;
            }
         }

         /*
          * Check east 
          */
         if( x < 79 )
         {
            newx = x;
            newy = y;
            newx++;
            while( newx <= 79 && ( map[y][newx].code == '-' || map[y][newx].code == ':' || map[y][newx].code == '='
                                   || map[y][newx].code == '[' || map[y][newx].code == ']' ) )
               newx++;
            /*
             * dont link it to itself 
             */
            if( map[y][x].vnum == map[y][newx].vnum )
               break;
            if( ( tvnum = map[y][newx].vnum ) != 0 )
            {
               xit = make_exit( newrm, get_room_index( tvnum ), DIR_EAST );
               xit->keyword = STRALLOC( "" );
               xit->description = STRALLOC( "" );
               xit->key = -1;
               if( map[y][newx - 2].code == ':' || map[y][newx - 2].code == '=' )
               {
                  SET_BIT( xit->exit_info, EX_ISDOOR );
                  SET_BIT( xit->exit_info, EX_CLOSED );
               }
               else
                  xit->exit_info = 0;
            }
         }

         /*
          * Check west 
          */
         if( x > 1 )
         {
            newx = x;
            newy = y;
            newx--;
            while( newx >= 0 && ( map[y][newx].code == '-' || map[y][newx].code == ':' || map[y][newx].code == '='
                                  || map[y][newx].code == '[' || map[y][newx].code == ']' ) )
               newx--;
            /*
             * dont link it to itself 
             */
            if( map[y][x].vnum == map[y][newx].vnum )
               break;
            if( ( tvnum = map[y][newx].vnum ) != 0 )
            {
               xit = make_exit( newrm, get_room_index( tvnum ), DIR_WEST );
               xit->keyword = STRALLOC( "" );
               xit->description = STRALLOC( "" );
               xit->key = -1;
               if( map[y][newx + 2].code == ':' || map[y][newx + 2].code == '=' )
               {
                  SET_BIT( xit->exit_info, EX_ISDOOR );
                  SET_BIT( xit->exit_info, EX_CLOSED );
               }
               else
                  xit->exit_info = 0;
            }
         }

         /*
          * Check southeast 
          */
         if( y < 48 && x < 79 )
         {
            newx = x;
            newy = y;
            newx += 2;
            newy++;
            while( newx <= 79 && newy <= 48 && ( map[newy][newx].code == '\\' || map[newy][newx].code == ':'
                                                 || map[newy][newx].code == '=' ) )
            {
               newx++;
               newy++;
            }
            if( map[newy][newx].code == '[' )
               newx++;
            /*
             * dont link it to itself 
             */
            if( map[y][x].vnum == map[newy][newx].vnum )
               break;
            if( ( tvnum = map[newy][newx].vnum ) != 0 )
            {
               xit = make_exit( newrm, get_room_index( tvnum ), DIR_SOUTHEAST );
               xit->keyword = STRALLOC( "" );
               xit->description = STRALLOC( "" );
               xit->key = -1;
               xit->exit_info = 0;
            }
         }

         /*
          * Check northeast 
          */
         if( y > 1 && x < 79 )
         {
            newx = x;
            newy = y;
            newx += 2;
            newy--;
            while( newx >= 0 && newy <= 48 && ( map[newy][newx].code == '/' || map[newy][newx].code == ':'
                                                || map[newy][newx].code == '=' ) )
            {
               newx++;
               newy--;
            }
            if( map[newy][newx].code == '[' )
               newx++;

            /*
             * dont link it to itself 
             */
            if( map[y][x].vnum == map[newy][newx].vnum )
               break;
            if( ( tvnum = map[newy][newx].vnum ) != 0 )
            {
               xit = make_exit( newrm, get_room_index( tvnum ), DIR_NORTHEAST );
               xit->keyword = STRALLOC( "" );
               xit->description = STRALLOC( "" );
               xit->key = -1;
               xit->exit_info = 0;
            }
         }

         /*
          * Check northwest 
          */
         if( y > 1 && x > 1 )
         {
            newx = x;
            newy = y;
            newx -= 2;
            newy--;
            while( newx >= 0 && newy >= 0 && ( map[newy][newx].code == '\\' || map[newy][newx].code == ':'
                                               || map[newy][newx].code == '=' ) )
            {
               newx--;
               newy--;
            }
            if( map[newy][newx].code == ']' )
               newx--;
            /*
             * dont link it to itself 
             */
            if( map[y][x].vnum == map[newy][newx].vnum )
               break;
            if( ( tvnum = map[newy][newx].vnum ) != 0 )
            {
               xit = make_exit( newrm, get_room_index( tvnum ), DIR_NORTHWEST );
               xit->keyword = STRALLOC( "" );
               xit->description = STRALLOC( "" );
               xit->key = -1;
               xit->exit_info = 0;
            }
         }

         /*
          * Check southwest 
          */
         if( y < 48 && x > 1 )
         {
            newx = x;
            newy = y;
            newx -= 2;
            newy++;
            while( newx >= 0 && newy <= 48 && ( map[newy][newx].code == '/' || map[newy][newx].code == ':'
                                                || map[newy][newx].code == '=' ) )
            {
               newx--;
               newy++;
            }
            if( map[newy][newx].code == ']' )
               newx--;
            /*
             * dont link it to itself 
             */
            if( map[y][x].vnum == map[newy][newx].vnum )
               break;
            if( ( tvnum = map[newy][newx].vnum ) != 0 )
            {
               xit = make_exit( newrm, get_room_index( tvnum ), DIR_SOUTHWEST );
               xit->keyword = STRALLOC( "" );
               xit->description = STRALLOC( "" );
               xit->key = -1;
               xit->exit_info = 0;
            }
         }
      }
   }
}
