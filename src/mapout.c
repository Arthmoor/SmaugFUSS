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

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


/*
 * Useful Externals
 */
extern int      top_exit;
void note_attach( CHAR_DATA *ch );

/*
 * Local defines.  Undef'ed at end of file.
 */

#define MID	MAP_INDEX_DATA
#define MD	MAP_DATA
#define RID	ROOM_INDEX_DATA
#define CD	CHAR_DATA
#define EDD     EXTRA_DESCR_DATA
#define OD      OBJ_DATA
#define OID     OBJ_INDEX_DATA
#define XD      EXIT_DATA

/*
 * Local function prototypes
 */
MID * 		make_new_map_index (int vnum);
void 		map_to_rooms (CD * ch, MID * m_index);
void 		map_stats (CD * ch, int *rooms, int *rows, int *cols);
int     	num_rooms_avail (CD * ch);
int     	add_new_room_to_map (CD * ch, MID * map, int row, int col, int proto_room, char code);
int     	number_to_room_num(int array_index);
int     	char_to_number (char code);
int     	exit_lookup (int vnum1, int vnum2);
void 		draw_map (CD * ch, RID * rm, int flag, int mode);
char   		*you_are_here (int row, int col, char *map);
char            get_map_code( RID *room, int mode );

/*
 * Local Variables & Structs
 */
char    text_map[4150];
extern  MID * first_map;/* should be global */
struct map_stuff {
    int     vnum;
    int     proto_vnum;
    int     exits;
    int     index;
    char    code;
};


/*************************************************************/
/*                                                           */
/* First section for read/write of map to .are files and     */
/*  on-line map editing, such as it is.                      */
/*                                                           */
/*************************************************************/

/*
 * Be careful not to give
 * this an existing map_index
 */
MID * make_new_map_index (int vnum) {
    MID * map_index;
    int     i, j;

    CREATE (map_index, MID, 1);
    map_index -> vnum = vnum;
    for (i = 0; i < 49; i++) {
	for (j = 0; j < 78; j++) {
	    map_index -> map_of_vnums[i][j] = -1;
	}
    }
    map_index -> next = first_map;
    first_map = map_index;
    return map_index;
}

/* 
 * output goes in global text_map 
 *  flag = 0, do a 'you_are_here'
 *  flag = 1, don't
 *
 *
 *  Modes  :  what characters mean :  character set
 *  --------------------------------------------------------------------
 *        0: room code             : 92 ascii chars, detailed elsewhere
 *        1: # of mobs             : 0 thru 9, +
 *        2: # of pc's             : 0 thru 9, +
 *        3: # of objs             : 0 thru 9, +
 *        4: # of exits            : 0 thru 6
 *        5: sectortype            : hex, 0 thru MAX_SECT
 *        6: light                 : 0 or 1 
 *        7: indoors               : X or O
 *        8: death                 : X or O
 *        9: safe                  : X or O
 *       10: nosummon              : X or O
 *       11: # of descr lines      : 0 thru 9, +
 */

char * const map_opts [] = 
{
"code","mobs","pcs","objs","exits","sector","light","indoors","death","safe","nosummon", "descr", "descrlines"
};

char count_lines(char *txt)
{
int i;
char *c, buf[MAX_STRING_LENGTH];

if(!txt)
  return (char) '0';

i=1;
for(c=txt;*c!='\0';c++)
  if (*c == '\n') i++;

if( i  > 9 ) 
    return (char) '+';
sprintf(buf,"%d",i);
return(buf[0]);
}

char get_map_code( RID *room, int mode )
{
   char buf[MAX_STRING_LENGTH];
   CD *mob;
   OD *obj;
   int count/*, i*/; /* Unused */
   EXIT_DATA *pexit;

   if( !room)
     return (char) 'X';

   count = 0;
   switch(mode)
   {
      case 11: return count_lines(room->description);
      case 10: if(IS_SET(room->room_flags,ROOM_NO_SUMMON)) return (char) 'X';
	       return (char) 'O';
       case 9: if(IS_SET(room->room_flags,ROOM_SAFE)) return (char) 'X';
	       return (char) 'O';
       case 8: if(IS_SET(room->room_flags,ROOM_DEATH)) return (char) 'X';
	       return (char) 'O';
       case 7: if(IS_SET(room->room_flags,ROOM_INDOORS)) return (char) 'X';
	       return (char) 'O';
       case 6: sprintf(buf,"%d",room->light);
	       return(buf[0]);
       case 5: sprintf(buf,"%d",room->sector_type);
	       return(buf[0]);
       case 4: for ( pexit = room->first_exit; pexit; pexit = pexit->next)
		  count++;
	       sprintf(buf,"%d",count);
	       return(buf[0]);
       case 3: for(obj=room->first_content;obj;obj=obj->next_content)
                  count++;
	       if( count  > 9 ) 
		    return (char) '+';
	       sprintf(buf,"%d",count);
	       return(buf[0]);
       case 2: for(mob=room->first_person;mob;mob=mob->next_in_room) {
		  if(!IS_NPC(mob) )
                     count++;
	       }
	       if( count  > 9 ) 
		    return (char) '+';
	       sprintf(buf,"%d",count);
	       return(buf[0]);
       case 1: for(mob=room->first_person;mob;mob=mob->next_in_room) {
		  if(IS_NPC(mob) )
                     count++;
	       }
	       if( count  > 9 ) 
		    return (char) '+';
	       sprintf(buf,"%d",count);
	       return(buf[0]);
       default:
	       if(!room->map) return (char) 'X';
	       return ((char) room->map->entry);
   }
   return (char) '?';
}

void draw_map (CD * ch, RID * rm, int flag, int mode) {
    MID * map_index;
    RID * tmp_rm;
    int     i,
            x,
            y,
            nontriv;

    if (!rm -> map) {
	sprintf (text_map, "(no rm->map)\n\r");
	return;
    }
    if ((map_index = get_map_index (rm -> map -> vnum)) == NULL) {
	bug (" No map_index with vnum %d\n\r", rm -> map -> vnum);
	sprintf (text_map, "-ERROR-\n\r");
	return;
    }
    i = 0;
    nontriv = 0;
    for (y = 0; y < 49; y++) {
	for (x = 0; x < 78; x++) {
	    if (map_index -> map_of_vnums[y][x] < 1) {
		text_map[i] = ' ';
		i++;
		continue;
	    }
	    /* tmp_rm = map_index -> map_of_ptrs[y][x]; */
	    tmp_rm = get_room_index( map_index -> map_of_vnums[y][x] ) ;
	    if (tmp_rm == NULL) {
		text_map[i] = ' ';
		i++;
		continue;
	    }
	    if (!tmp_rm -> map) {
		text_map[i] = ' ';
		i++;
		continue;
	    }

	    /*
	     *  Following's kinda convoluted...  If ch has ansi,
	     *   bold the code of the room he's in.  If not, indicate
	     *   room he's in by '*'
	     */
	    if(  (flag==1) )
	    {
	       if(map_index->map_of_vnums[y][x]==ch->in_room->vnum)
	       {
		  if ( xIS_SET(ch->act, PLR_ANSI) ){
	             text_map[i] = (char) '\x1B';    /* Bold */
		     i++;
	             text_map[i] = (char) '[';
		     i++;
	             text_map[i] = (char) '1';
		     i++;
	             text_map[i] = (char) 'm';
		     i++;
	             /*text_map[i] = (char) tmp_rm -> map -> entry;*/
	             text_map[i] = (char) get_map_code(tmp_rm, mode); 
		     i++;
	             text_map[i] = (char) '\x1B';   /* Normal */
		     i++;
	             text_map[i] = (char) '[';
		     i++;
	             text_map[i] = (char) '0';
		     i++;
	             text_map[i] = (char) 'm';
		     i++;
                     
		  } else {
	             text_map[i] = (char) '*';
		     i++;
                  }
	       } else  {
	           text_map[i] = (char) get_map_code(tmp_rm, mode); 
	           /*text_map[i] = (char) tmp_rm -> map -> entry; */
	           i++;
	       }
            } else {
	       text_map[i] = (char) get_map_code(tmp_rm, mode); 
	       /*text_map[i] = (char) tmp_rm -> map -> entry;*/
	       i++;
            }
	    nontriv = i;
	}
	text_map[i] = '\n';
	i++;
    }
    text_map[nontriv + 2] = '\n';
    text_map[nontriv + 3] = '\r';
    text_map[nontriv + 4] = '\0';
}

MID *
get_map_index (int vnum) {
    MID * map;

    for (map = first_map; map; map = map -> next) {
	if (map -> vnum == vnum)
	    return map;
    }
    return NULL;
}



void 
init_maps () {
    int     i;

/*
    for (map_index = first_map; map_index; map_index = map_index -> next) {

	for (i = 0; i < 49; i++) {
	    for (j = 0; j < 78; j++) {
		   map_index -> map_of_ptrs[i][j] = 
		    get_room_index (map_index -> map_of_vnums[i][j]);
                   
	    }
	}

    }
*/

    for (i = 0; i < 49 * 78; i++)
	text_map[i] = '\0';

    return;
}


/******************************************************************
 * These functions convert maps to rooms
 ******************************************************************/
void map_stats (CD * ch, int *rooms, int *rows, int *cols) {
    int     row,
            col,
            n;
    int     leftmost,
            rightmost;
    char   *l,
            c;

    if (!ch -> pnote) {
	bug ("map_stats: ch->pnote==NULL!", 0);
	return;
    }
    n = 0;
    row = col = 0;
    leftmost = rightmost = 0;

    l = ch -> pnote -> text;
    do {
	c = l[0];
	switch (c) {
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
	if (char_to_number (c) > -1) {
	    if (col < leftmost)
		leftmost = col;
	    if (col > rightmost)
		rightmost = col;
	    col++;
	    n++;
	};
	l++;
    } while (c != '\0');

    *cols = rightmost - leftmost + 1;
    *rows = row;		/* [sic.] */
    *rooms = n;
}

int get_mode( char *type)
{
   int x;

   for( x = 0; x < 12; x++ )
     if( !str_cmp( type, map_opts[x] ) )
          return x;
   return -1;
}

void do_lookmap (CD * ch, char *argument) {
char arg1[MAX_STRING_LENGTH];
char buf[MAX_STRING_LENGTH];
int mode;
   

    if (ch -> in_room -> map) {
	if( ! argument ) { 
	   mode = 0;
        } else {
           argument = one_argument( argument, arg1);
           mode = get_mode( arg1 );
        }

        set_char_color( AT_PLAIN, ch );
        sprintf( buf , ".------[Map %5.5d]-----------------------------------------------------------.\n\r", ch -> in_room -> map -> vnum);
	send_to_char (buf , ch);
	draw_map (ch, ch -> in_room, 1, mode);
	send_to_char (text_map, ch);
	sprintf( buf,"`----------------------------------------------------------------------------'\n\r");
	send_to_char (buf , ch);
    }
    else {
	send_to_char ("You see no map here.\n\r", ch);
    }
    return;
}

void do_mapout (CD * ch, char *argument) {
    char    buf[MAX_STRING_LENGTH];
    char    arg[MAX_INPUT_LENGTH];
    char    arg1[MAX_INPUT_LENGTH];
    char    arg2[MAX_INPUT_LENGTH];
    char    arg3[MAX_INPUT_LENGTH];   /* growl */
    char    arg4[MAX_INPUT_LENGTH];   /* rediculous */
    OD      *map_obj;                 /* an obj made with map as an ed */
    OID     *map_obj_index;           /*    obj_index for previous     */
    EDD     *ed;                      /*    the ed for it to go in     */
    MID     *map_index;               /* the "vnum" of map_index to use*/
    MD      *map, *tmp;               /* for new per-room map info to goin*/
    RID     *this_rm, *tmp_r = NULL;  /* room ch is standing in */
    XD      *tmp_x;                   /* exit data */
    char    code;
    int     rooms,                    
            rows,                     /* ints for stats & looping */
            cols,
	    row,
	    col,
	    mapnum,
	    x,
	    y,
            avail_rooms;

    if (!ch) {
	bug ("do_mapout: null ch", 0);
	return;
    }
    if (IS_NPC (ch)) {
	send_to_char ("Not in mobs.\n\r", ch);
	return;
    }
    if (!ch -> desc) {
	bug ("do_mapout: no descriptor", 0);
	return;
    }
    switch (ch -> substate) {
	default: 
	    break;
	case SUB_WRITING_NOTE: 
	    if (ch -> dest_buf != ch -> pnote)
		bug ("do_mapout: sub_writing_map: ch->dest_buf != ch->pnote", 0);
	    STRFREE (ch -> pnote -> text);
	    ch -> pnote -> text = copy_buffer (ch);
	    stop_editing (ch);
	    return;
    }

    set_char_color (AT_NOTE, ch);
    argument = one_argument (argument, arg);
    smash_tilde (argument);

    if (!str_cmp (arg, "stat")) {
	if (!ch -> pnote) {
	    send_to_char ("You have no map in progress.\n\r", ch);
	    return;
	}
	map_stats (ch, &rooms, &rows, &cols);
	sprintf (buf,
		"Map represents %d rooms, and has %d rows and %d columns\n\r",
		rooms,
		rows,
		cols);
	send_to_char (buf, ch);
	avail_rooms = num_rooms_avail (ch);
	sprintf (buf, "You currently have %d unused rooms.\n\r", avail_rooms);
	send_to_char (buf, ch);

	act (AT_ACTION, "$n glances at an etherial map.", ch, NULL, NULL, TO_ROOM);
	return;
    }


    /*
     *  Adds an existing room to a map
     */
    if (!str_cmp (arg, "continue")) {
       if ( ch->prev_cmd == do_north )
       {
           send_to_char ("Your last command was north.\n\r", ch);
	   argument = one_argument (argument, arg1);

	   if ( (arg1[0] == '\0') )
	   {
               code = '#';
	   } else {
               code = arg1[0];     
	   }

	   tmp = NULL;
	   tmp_x = get_exit(ch->in_room, DIR_SOUTH );
	   if ( tmp_x )
	     tmp_r = tmp_x->to_room;
	   if ( tmp_r )
	     tmp = tmp_r->map;
	   if( !tmp )
	   {
                send_to_char("No exit to south, or no map in south room. Aborting. \n\r",ch);
		return;
	   }

	   row = (tmp->y) - 1;
	   col = (tmp->x);

	   if( row < 0) {
                send_to_char("Can't map off the top of the buffer.\n\r",ch);
		return;
	   }
	   if( row >48) {
                send_to_char("Can't map off the bottom of the buffer.\n\r",ch);
		return;
	   }

	   mapnum = tmp->vnum;
	   if( (map_index=get_map_index( mapnum )) ==NULL  )
	   {
	      sprintf ( buf,"Trouble accessing map.(No such map?).\n\r");
              send_to_char (buf, ch);
	      return;
	   }
           sprintf( buf, "addroom %d %d %d %c",mapnum,row,col,code);
	   do_mapout(ch, buf);
	   return;
       }

       if ( ch->prev_cmd == do_east )
       {
           send_to_char ("Your last command started was east\n\r", ch);
	   argument = one_argument (argument, arg1);

	   if ( (arg1[0] == '\0') )
	   {
               code = '#';
	   } else {
               code = arg1[0];     
	   }
	   tmp = NULL;
	   tmp_x = get_exit( ch->in_room, DIR_WEST );
	   if ( tmp_x )
	     tmp_r = tmp_x->to_room;
	   if ( tmp_r )
	     tmp = tmp_r->map;

	   if( !tmp )
	   {
                send_to_char("No exit to west, or no map in west room. Aborting. \n\r",ch);
		return;
	   }

	   row = (tmp->y);
	   col = (tmp->x)+1;

	   if( col < 0) {
                send_to_char("Can't map off the left of the buffer.\n\r",ch);
		return;
	   }
	   if( row >78) {
                send_to_char("Can't map off the right of the buffer.\n\r",ch);
		return;
	   }

	   mapnum = tmp->vnum;
	   if( (map_index=get_map_index( mapnum )) ==NULL  )
	   {
	      sprintf ( buf,"Trouble accessing map.(No such map?).\n\r");
              send_to_char (buf, ch);
	      return;
	   }
           sprintf( buf, "addroom %d %d %d %c",mapnum,row,col,code);
	   do_mapout(ch, buf);
	   return;
       }

       if ( ch->prev_cmd == do_south )
       {
           send_to_char ("Your last command was south\n\r", ch);
	   argument = one_argument (argument, arg1);

	   if ( (arg1[0] == '\0') )
	   {
               code = '#';
	   } else {
               code = arg1[0];     
	   }
	   tmp = NULL;
	   tmp_x = get_exit( ch->in_room, DIR_NORTH );
	   if ( tmp_x )
	     tmp_r = tmp_x->to_room;
	   if ( tmp_r )
	     tmp = tmp_r->map;

	   if( !tmp )
	   {
                send_to_char("No exit to north, or no map in north room. Aborting. \n\r",ch);
		return;
	   }

	   row = (tmp->y)+1;
	   col = (tmp->x);

	   if( row < 0) {
                send_to_char("Can't map off the top of the buffer.\n\r",ch);
		return;
	   }
	   if( row >48) {
                send_to_char("Can't map off the bottom of the buffer.\n\r",ch);
		return;
	   }

	   mapnum = tmp->vnum;
	   if( (map_index=get_map_index( mapnum )) ==NULL  )
	   {
	      sprintf ( buf,"Trouble accessing map.(No such map?).\n\r");
              send_to_char (buf, ch);
	      return;
	   }
           sprintf( buf, "addroom %d %d %d %c",mapnum,row,col,code);
	   do_mapout(ch, buf);
	   return;
       }

       if ( ch->prev_cmd == do_west )
       {
           send_to_char ("Your last command was west\n\r", ch);
	   argument = one_argument (argument, arg1);

	   if ( (arg1[0] == '\0') )
	   {
               code = '#';
	   } else {
               code = arg1[0];     
	   }

	   tmp = NULL;
	   tmp_x = get_exit( ch->in_room, DIR_EAST );
	   if ( tmp_x )
	     tmp_r = tmp_x->to_room;
	   if ( tmp_r )
	     tmp = tmp_r->map;

	   if( !tmp )
	   {
                send_to_char("No exit to east, or no map in east room. Aborting. \n\r",ch);
		return;
	   }

	   row = (tmp->y);
	   col = (tmp->x) - 1;

	   if( col < 0) {
                send_to_char("Can't map off the left of the buffer.\n\r",ch);
		return;
	   }
	   if( row >78) {
                send_to_char("Can't map off the right of the buffer.\n\r",ch);
		return;
	   }

	   mapnum = tmp->vnum;
	   if( (map_index=get_map_index( mapnum )) ==NULL  )
	   {
	      sprintf ( buf,"Trouble accessing map.(No such map?).\n\r");
              send_to_char (buf, ch);
	      return;
	   }
           sprintf( buf, "addroom %d %d %d %c",mapnum,row,col,code);
	   do_mapout(ch, buf);
	   return;
       }

       sprintf (buf, "Your previous command was something I cannot backtrack..\n\r");
       send_to_char (buf,ch);
       return;
    }

    /*
     *  Adds an existing room to a map
     */
    if (!str_cmp (arg, "addroom")) {

        argument = one_argument (argument, arg1);
        argument = one_argument (argument, arg2);
        argument = one_argument (argument, arg3);
        argument = one_argument (argument, arg4);

	mapnum  = atoi ( arg1);   /* i don't like this */
	y 	= atoi ( arg2);
	x 	= atoi ( arg3);

        if ( (arg1[0] == '\0') || (arg2[0] == '\0') || (arg3[0] == '\0') )
	{
          send_to_char("Syntax:                                   \n\r",ch);
          send_to_char("mapout addroom <mapnum> <row> <col> [code]\n\r",ch);
          send_to_char("                                          \n\r",ch);
          send_to_char("where: <mapnum> is the vnum of map to use\n\r",ch);
          send_to_char("       <row> is row of room (start 0)\n\r",ch);
          send_to_char("       <col> is col of room (start 0)\n\r",ch);
          send_to_char("   [code] is optional  room character code \n\r",ch);
            return;
	}

	if ( (arg4[0] == '\0') )
	{
            code = '#';
	} else {
            code = arg4[0];     
	}


	if( (map_index=get_map_index( mapnum )) ==NULL  )
	{
#ifdef HURM
	   sprintf ( buf,"Trouble accessing map.(No such map?).\n\r");
           send_to_char (buf, ch);
	   return;
#endif
	    map_index = make_new_map_index ( mapnum);
	    if (map_index == NULL)
	    {
               send_to_char("Could neither find nor make a map index with that number.\n\r", ch);
	       return;
	    }
	}

        this_rm = ch->in_room;
	if(this_rm->map!=NULL)
	{
	   sprintf ( buf,"This room (vnum %d) is already in map %d.\n\r",
						  ch->in_room->vnum,
						  ch->in_room->map->vnum);
           send_to_char (buf, ch);
	   return;
	}

	if(  (x <0 ) || (x >78) )
	{
	   sprintf ( buf,"Bad map x coordinate. Room(vnum %d), x= %d \n\r",
						  ch->in_room->vnum,
						  x);
           send_to_char (buf, ch);
	   return;
	}
	if(  (y <0 ) || (y >48 ) )
	{
	   sprintf ( buf,"Bad map y coordinate. Room(vnum %d), y= %d \n\r",
						  ch->in_room->vnum,
						  y);
           send_to_char (buf, ch);
	   return;
	}
        if(map_index->map_of_vnums[y][x]!=-1)
	{
	   sprintf ( buf,"That (x,y) coordinate (%d, %d) is already taken by room %d.\n\r",
				  x,y, map_index->map_of_vnums[y][x]);
           send_to_char (buf, ch);
	   return;
	}

        /* all error checking done */
        CREATE( map, MAP_DATA, 1);
	map->vnum        = mapnum;
	map->x		 = x;
	map->y		 = y;
	map->entry	 = code;
	ch->in_room->map = map;

        map_index->map_of_vnums[y][x] = ch->in_room->vnum;

        send_to_char ("Added.\n\r", ch);
	return;


    }

    /*
     *  Removes a room from a map
     */
    if (!str_cmp (arg, "removeroom")) {
        this_rm = ch->in_room;
	if(!this_rm->map)
	{
	   sprintf ( buf,"This room (vnum %d) is in no map \n\r",
						  ch->in_room->vnum);
           send_to_char (buf, ch);
	   return;
	}
        
	if(   (map_index=get_map_index(this_rm->map->vnum )) ==NULL  )
	{
	   sprintf ( buf,"Trouble accessing map room(vnum %d), map vnum %d \n\r",
						  ch->in_room->vnum,
						  this_rm->map->vnum);
           send_to_char (buf, ch);
	   return;
	}
	if(  (this_rm->map->x <0 ) || (this_rm->map->x >78) )
	{
	   sprintf ( buf,"Bad map x coordinate. Room(vnum %d), x= %d \n\r",
						  ch->in_room->vnum,
						  this_rm->map->x);
           send_to_char (buf, ch);
	   return;
	}
	if(  (this_rm->map->y <0 ) || (this_rm->map->y >48 ) )
	{
	   sprintf ( buf,"Bad map y coordinate. Room(vnum %d), y= %d \n\r",
						  ch->in_room->vnum,
						  this_rm->map->y);
           send_to_char (buf, ch);
	   return;
	}
	/* now that all that's out of the way.... */
	   sprintf ( buf,"Removing room (vnum %d), from map %d \n\r",
						  ch->in_room->vnum,
						  this_rm->map->vnum);
           send_to_char (buf, ch);

	/* Thanks to Nick Gammon for pointing out x and y being
	   uninitialized.  -Thoric */
	x = this_rm->map->x;
	y = this_rm->map->y;

	map_index->map_of_vnums[y][x]= -1;
	this_rm->map->vnum        = 0;
	this_rm->map->x	 	  = -1;
	this_rm->map->y	 	  = -1;
	this_rm->map->entry 	  = ' ';  
	DISPOSE(this_rm->map);
	this_rm->map = NULL;   /* redundant? */
        send_to_char ("Removed.\n\r", ch);
	return;
    }

    if (!str_cmp (arg, "write")) {
	note_attach (ch);
	ch -> substate = SUB_WRITING_NOTE;
	ch -> dest_buf = ch -> pnote;
	start_editing (ch, ch -> pnote -> text);
	return;
    }
    if (!str_cmp (arg, "clear")) {
	if (ch -> pnote) {
	    STRFREE (ch -> pnote -> text);
	    STRFREE (ch -> pnote -> subject);
	    STRFREE (ch -> pnote -> to_list);
	    STRFREE (ch -> pnote -> date);
	    STRFREE (ch -> pnote -> sender);
	    DISPOSE (ch -> pnote);
	}
	ch -> pnote = NULL;

	send_to_char ("Map cleared.\n\r", ch);
	return;
    }
    if (!str_cmp (arg, "show")) {
	if (!ch -> pnote) {
	    send_to_char ("You have no map in progress.\n\r", ch);
	    return;
	}
	/* send_to_char(buf, ch); */
	send_to_char (ch -> pnote -> text, ch);
	do_mapout (ch, "stat");
	return;
    }
    if (!str_cmp (arg, "redo")) {
	if (!ch -> pnote) {
	    send_to_char ("You have no map in progress.\n\r", ch);
	    return;
	}
	send_to_char ("This option not yet supported.\n\r", ch);
	return;

    }
    if (!str_cmp (arg, "create")) {
	if (!ch -> pnote) {
	    send_to_char ("You have no map in progress.\n\r", ch);
	    return;
	}
	map_stats (ch, &rooms, &rows, &cols);
	avail_rooms = num_rooms_avail (ch);

	/* check for not enough rooms */
	if (rooms > avail_rooms) {
	    send_to_char ("You don't have enough unused rooms allocated!\n\r", ch);
	    return;
	}
	act (AT_ACTION, "$n warps the very dimensions of space!", ch, NULL, NULL, TO_ROOM);

	map_to_rooms (ch, NULL);/* this does the grunt work */

	map_obj_index = get_obj_index (5013);
	if(!map_obj_index)
	{
	    map_obj = create_object (map_obj_index , 0);
	    ed = SetOExtra (map_obj, "runes map scrawls");
	    STRFREE( ed->description );
	    ed -> description = QUICKLINK (ch -> pnote -> text);
	    obj_to_char (map_obj, ch);
	} else {
            send_to_char ("Couldn't give you a map object.  Need Great Eastern Desert\n\r", ch);
            return;
	}

	do_mapout (ch, "clear");
	send_to_char ("Ok.\n\r", ch);
	return;
    }
    send_to_char ("mapout write: create a map in edit buffer.\n\r", ch);
    send_to_char ("mapout stat: get information about a written, but not yet created map.\n\r", ch);
    send_to_char ("mapout clear: clear a written, but not yet created map.\n\r", ch);
    send_to_char ("mapout show: show a written, but not yet created map.\n\r", ch);
    send_to_char ("mapout create: turn a written map into rooms in your assigned room vnum range.\n\r", ch);
    return;
}



int     
add_new_room_to_map(CD *ch,MID *map,int row,int col,int proto_room,char code) {
    int     i;
    char    buf[MAX_STRING_LENGTH];
    RID * location, *rm;


    /* 
     * Get a room to copy from
     */
    rm = get_room_index (proto_room);
    if (!rm)
	rm = ch -> in_room;


    /* 
     * Get an unused room to copy to
     */
    for (i = ch -> pcdata -> r_range_lo; i <= ch -> pcdata -> r_range_hi; i++) {
	if (get_room_index (i) == NULL) {
	    location = make_room (i);
	    if (!location) {
		bug ("next_rooms_avail: make_room failed", 0);
		return - 1;
	    }
	    /* 
	     * Clones current room  (quietly)
	     */
	    location -> area = ch -> pcdata -> area;
	    location -> name = QUICKLINK (rm -> name);
	    location -> description = QUICKLINK (rm -> description);
	    CREATE (location -> map, MAP_DATA, 1);
	    location -> map -> vnum = map -> vnum;   /* not working? */
	    location -> map -> x = col;
	    location -> map -> y = row;
	    location -> map -> entry = code;

	    location -> room_flags = ROOM_PROTOTYPE && rm -> room_flags;
	    location -> light = rm -> light;
	    location -> sector_type = rm -> sector_type;
	    return i;
	}
    }
    sprintf (buf, "No available room!\n\r");
    send_to_char (buf, ch);
    return - 1;
}

int     num_rooms_avail (CD * ch) {
    int     i,
            n;

    n = 0;
    for (i = ch -> pcdata -> r_range_lo; i <= ch -> pcdata -> r_range_hi; i++)
	if (!get_room_index (i))
	    n++;

    return n;
}

/*
 * This function takes the character string in ch->pnote and
 *  creates rooms laid out in the appropriate configuration.
 */
void map_to_rooms (CD * ch, MID * m_index) {
    struct map_stuff    map[49][78];    /* size of edit buffer */
    int     row,                       
            col,
            i,
            n,
            x,
            y,
            tvnum,
            proto_vnum,
            leftmost,
            rightmost;
    char   *l,
            c;
    RID * newrm;
    MID * map_index = NULL, *tmp;
    XD  * xit;   		/* these are for exits */
    int   exit_type;

    if (!ch -> pnote) {
	bug ("map_to_rooms: ch->pnote==NULL!", 0);
	return;
    }
    n = 0;
    row = col = 0;
    leftmost = rightmost = 0;

    /* 
     * Check to make sure map_index exists.  
     * If not, then make a new one.
     */
    if (!m_index) {		/* Make a new vnum */
	for (i=ch->pcdata->r_range_lo;i<=ch->pcdata->r_range_hi;i++) {
	    if ((tmp = get_map_index (i)) == NULL) {
		map_index = make_new_map_index (i);
		break;
	    }
	}
    }
    else {
	map_index = m_index;
    }

   /*
    *  
    */
    if(!map_index)
    {
	 send_to_char("Couldn't find or make a map_index for you!\n\r",ch);
	 bug("map_to_rooms: Couldn't find or make a map_index\n\r",0);
        /* do something. return failed or somesuch */
    }


    for (x = 0; x < 49; x++) {
	for (y = 0; y < 78; y++) {
	    map[x][y].vnum = 0;
	    map[x][y].proto_vnum = 0;
	    map[x][y].exits = 0;
	    map[x][y].index = 0;
	}
    }
    l = ch -> pnote -> text;
    do {
	c = l[0];
	switch (c) {
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
	if ((map[row][col].index = char_to_number (c)) > -1) {
	    proto_vnum = number_to_room_num (map[row][col].index);
	    map[row][col].vnum = add_new_room_to_map (ch, map_index, row, col, proto_vnum, c);

            map_index->map_of_vnums[row][col] = map[row][col].vnum;
	    map[row][col].proto_vnum = proto_vnum;
	    map[row][col].code = c;
	    col++;
	    n++;

	}
	else {
            map_index->map_of_vnums[row][col] = 0;
	    map[row][col].vnum = 0;
	    map[row][col].exits = 0;
	}

	l++;
    } while (c != '\0');

    for (y = 0; y < row + 1; y++) {/* rows */
	for (x = 0; x < 78; x++) {/* cols (78, i think) */

	    if (map[y][x].vnum == 0)
		continue;

	    newrm = get_room_index (map[y][x].vnum);
	    CREATE (newrm -> map, MAP_DATA, 1);
	    newrm -> map -> vnum = map_index->vnum; 
	    newrm -> map -> x = x;
	    newrm -> map -> y = y;
	    newrm -> map -> entry = map[y][x].code;  

	    /* 
	     * Check north
	     */
	    if (y > 0) {
		if ((tvnum = map[y - 1][x].vnum) != 0) {
		    exit_type = exit_lookup (map[y][x].proto_vnum,
			    map[y - 1][x].proto_vnum);
		    if (exit_type > -1) {
			xit = make_exit( newrm, get_room_index( tvnum ), DIR_NORTH );
			xit->keyword  		= STRALLOC( "" );
			xit->description	= STRALLOC( "" );
			xit->key 		= -1;
			xit->exit_info 		= exit_type;
		    }
		}
	    }
	    /* east */
	    if (x < 79) {
		if ((tvnum = map[y][x + 1].vnum) != 0) {
		    exit_type = exit_lookup (map[y][x].proto_vnum,
			    map[y][x + 1].proto_vnum);
		    if (exit_type > -1) {
			xit = make_exit( newrm, get_room_index( tvnum ), DIR_EAST );
			xit->keyword  		= STRALLOC( "" );
			xit->description	= STRALLOC( "" );
			xit->key 		= -1;
			xit->exit_info 		= exit_type;
		    }
		}
	    }
	    /* south */
	    if (y < 48) {
		if ((tvnum = map[y + 1][x].vnum) != 0) {
		    exit_type = exit_lookup (map[y][x].proto_vnum,
			    map[y + 1][x].proto_vnum);
		    if (exit_type > -1) {
			xit = make_exit( newrm, get_room_index( tvnum ), DIR_SOUTH );
			xit->keyword  		= STRALLOC( "" );
			xit->description	= STRALLOC( "" );
			xit->key 		= -1;
			xit->exit_info 		= exit_type;
		    }
		}
	    }
	    /* west */
	    if (x > 0) {
		if ((tvnum = map[y][x - 1].vnum) != 0) {
		    exit_type = exit_lookup (map[y][x].proto_vnum,
			    map[y][x - 1].proto_vnum);
		    if (exit_type > -1) {
			xit = make_exit( newrm, get_room_index( tvnum ), DIR_WEST );
			xit->keyword  		= STRALLOC( "" );
			xit->description	= STRALLOC( "" );
			xit->key 		= -1;
			xit->exit_info 		= exit_type;
		    }
		}
	    }
	}
    }
}

/******************************************************************
 * Constants & constant-like functions follow
 ******************************************************************/

char   *const standard_room_names[] =
{
    "Hut", "Tent", "Hovel", "Campsite",
    "Shack", "Cabin", "Homested", "Keep",
    "Fortress", "Castle", "GuardHse", "Temple",
    "Store", "Graveyard", "Monastry", "Stable", "Tavern",
    "Basemnt", "Bedroom", "BnquetRm", "Corridor", "Attic",
    "Vault", "SittingRm", "Study", "Passage", "Tower",
    "Crypt", "WorkRoom", "Lab", "Hallway", "Turret",
    "StorRm", "Kitchen", "Larder", "Stairway", "Rooftop",
    "Closet", "Office", "Treasury", "Landing", "Balcony",
    "Foyer", "DrawingRm", "Den", "Ladder", "Catwalk",
    "Entrnce", "Arboretum", "Library", "Vent", "Shaft",
    "Gate", "AudiencRm", "Consrvty", "DumbWatr", "Chimney",
    "Porch", "ClassRoom", "CloakRm", "Lawn", "Garden",
    "Lake", "Forest", "Swamp", "Well", "Street",
    "River", "Canyon", "Beach", "Mine", "Road",
    "Stream", "Clearing", "SnakePit", "Tunnel", "Path",
    "Rapids", "Desert", "SandStrm", "Rope", "Cliff",
    "CaveRiv", "Jungle", "Sandbar", "RopeBrdg", "Bridge",
    "CaveLak", "Cave", "None", "RopeLadr", "NatlBrdg"
};

int     const standard_room_vnums[] =
{
    9500, 9501, 9502, 9503,
    9504, 9505, 9506, 9507,
    9508, 9509, 9510, 9511,
    9512, 9513, 9514, 9515, 9516,
    9517, 9518, 9519, 9520, 9521,
    9522, 9523, 9524, 9525, 9526,
    9527, 9528, 9529, 9530, 9531,
    9532, 9533, 9534, 9535, 9536,
    9537, 9538, 9539, 9540, 9541,
    9542, 9543, 9544, 9545, 9546,
    9547, 9548, 9549, 9550, 9551,
    9552, 9553, 9554, 9555, 9556,
    9557, 9558, 9559, 9560, 9561,
    9562, 9563, 9564, 9565, 9566,
    9567, 9568, 9569, 9570, 9571,
    9572, 9573, 9574, 9575, 9576,
    9577, 9578, 9579, 9580, 9581,
    9582, 9583, 9584, 9585, 9586,
    9587, 9588, 9589, 9590, 9591
};


/*
 *  Picks an entry from standard_room_vnums[], checking
 *   that it's legal
 */
int     
number_to_room_num (int array_index) {
    if      ((array_index < 0) || (array_index > 91))
	        return 2;

    return standard_room_vnums[array_index];
}

/*
 * Attempts to intellignetly determineif two adjecent rooms should
 *  be linked with an exit, and if so, what kind.
 *
 * This fn _depends_ on standard_room_vnums[] to make decisions.
 */
int     exit_lookup (int vnum1, int vnum2) {
    int     sect1,
            sect2;
    ROOM_INDEX_DATA * rm1, *rm2;
    int     exit_flag;

    rm1 = get_room_index (vnum1);
    rm2 = get_room_index (vnum2);

    if ((!rm1) || (!rm2)) {
	bug ("bad room index in exit_lookup!\n\r", 0);
	return (0);
    }
    exit_flag = 0;

    sect1 = rm1 -> sector_type;
    sect2 = rm2 -> sector_type;


    if (rm1 == rm2)		/* adjacent rooms with same std_rm_vnum */
	return 0;		/* assume they're simply linked */

    if ((vnum1 == 9589) || (vnum2 == 9589))
	return 0;		/* if 'none,' no assumptions */



    if ((sect1 == SECT_INSIDE) && (sect2 != SECT_INSIDE)) {
	switch (vnum1) {
	    case 9500: 		/* hut  *//* these are almost always */
	    case 9501: 		/* tent *//* single-roomed buildings */
	    case 9502: 		/* hovel *//* so let them be open on */
	    case 9504: 		/* shack *//* all sides -- user fixes */
	    case 9505: 		/* cabin */
	    case 9506: 		/* homestd */
	    case 9510: 		/* guardhse */
	    case 9511: 		/* temple */
	    case 9512: 		/* store */
	    case 9515: 		/* stable */
	    case 9516: 		/* tavern */
		return 0;
	    case 9542: 		/* foyer *//* these are always closed drs 
				*/
	    case 9547: 		/* entrance */
		return 3;
	    default: 
		exit_flag = -1;
	}
    }
    if ((sect2 == SECT_INSIDE) && (sect1 != SECT_INSIDE)) {
	switch (vnum2) {
	    case 9500: 		/* hut  *//* these are almost always */
	    case 9501: 		/* tent *//* single-roomed buildings */
	    case 9502: 		/* hovel *//* so let them be open on */
	    case 9504: 		/* shack *//* all sides -- user fixes */
	    case 9505: 		/* cabin */
	    case 9506: 		/* homestd */
	    case 9510: 		/* guardhse */
	    case 9511: 		/* temple */
	    case 9512: 		/* store */
	    case 9515: 		/* stable */
	    case 9516: 		/* tavern */
		return 0;
	    case 9542: 		/* foyer *//* these are always closed drs 
				*/
	    case 9547: 		/* entrance */
		return 3;
	    default: 
		exit_flag = -1;
	}
    }
    /* 
     * Can look at these cases again
     * 
     */
    if ((sect1 == SECT_CITY) && (sect2 != SECT_CITY)) {
	switch (vnum1) {
	    case 9511: 		/* temple *//* these are always open */
	    case 9512: 		/* store */
	    case 9516: 		/* tavern */
		return 0;
	    case 9552: 		/* gate *//* these are always closed drs 
				*/
	    case 9547: 
		return 3;
	    default: 
		exit_flag = -1;
	}
    }
    if ((sect2 == SECT_CITY) && (sect1 != SECT_CITY)) {
	switch (vnum2) {
	    case 9511: 		/* temple *//* these are always open */
	    case 9512: 		/* store */
	    case 9516: 		/* tavern */
		return 0;
	    case 9552: 		/* gate *//* these are always closed drs 
				*/
	    case 9547: 
		return 3;
	    default: 
		exit_flag = -1;
	}
    }
    return exit_flag;
}

/*
 *  Given a character 'code' in a map, returns the location
 *   of the corresponding room vnum in the constant array
 *   standard_room_vnums.  If 'code' is illegal, it returns
 *   -1.  To lookup the corresponding room vnum, call
 *   number_to_room_num on value returned here.
 */
int     
char_to_number (char code) {

    switch  (code) {
	case    'a': 
	            return 0;
	case 'b': 
	    return 1;
	case 'c': 
	    return 2;
	case 'd': 
	    return 3;
	case 'e': 
	    return 4;
	case 'f': 
	    return 5;
	case 'g': 
	    return 6;
	case 'h': 
	    return 7;
	case 'i': 
	    return 8;
	case 'j': 
	    return 9;
	case 'k': 
	    return 10;
	case 'l': 
	    return 11;
	case 'm': 
	    return 12;
	case 'n': 
	    return 13;
	case 'o': 
	    return 14;
	case 'p': 
	    return 15;
	case 'q': 
	    return 16;
	case 'r': 
	    return 17;
	case 's': 
	    return 18;
	case 't': 
	    return 19;
	case 'u': 
	    return 20;
	case 'v': 
	    return 21;
	case 'w': 
	    return 22;
	case 'x': 
	    return 23;
	case 'y': 
	    return 24;
	case 'z': 
	    return 25;
	case 'A': 
	    return 26;
	case 'B': 
	    return 27;
	case 'C': 
	    return 28;
	case 'D': 
	    return 29;
	case 'E': 
	    return 30;
	case 'F': 
	    return 31;
	case 'G': 
	    return 32;
	case 'H': 
	    return 33;
	case 'I': 
	    return 34;
	case 'J': 
	    return 35;
	case 'K': 
	    return 36;
	case 'L': 
	    return 37;
	case 'M': 
	    return 38;
	case 'N': 
	    return 39;
	case 'O': 
	    return 40;
	case 'P': 
	    return 41;
	case 'Q': 
	    return 42;
	case 'R': 
	    return 43;
	case 'S': 
	    return 44;
	case 'T': 
	    return 45;
	case 'U': 
	    return 46;
	case 'V': 
	    return 47;
	case 'W': 
	    return 48;
	case 'X': 
	    return 49;
	case 'Y': 
	    return 50;
	case 'Z': 
	    return 51;
	case '0': 
	    return 52;
	case '1': 
	    return 53;
	case '2': 
	    return 54;
	case '3': 
	    return 55;
	case '4': 
	    return 56;
	case '5': 
	    return 57;
	case '6': 
	    return 58;
	case '7': 
	    return 59;
	case '8': 
	    return 60;
	case '9': 
	    return 61;
	case '!': 
	    return 62;
	case '@': 
	    return 63;
	case '#': 
	    return 64;
	case '$': 
	    return 65;
	case '%': 
	    return 66;
	case '^': 
	    return 67;
	case '&': 
	    return 68;
	case '*': 
	    return 69;
	case '(': 
	    return 70;
	case ')': 
	    return 71;

	case '-': 
	    return 72;
	case '_': 
	    return 73;
	case '+': 
	    return 74;
	case '=': 
	    return 75;
	case '|': 
	    return 76;

	    /* case '\\': return 77; */
	    /* case '~': return 78; */
	case '`': 
	    return 79;
	case '{': 
	    return 80;
	case '[': 
	    return 81;

	case '}': 
	    return 82;
	case ']': 
	    return 83;
	case ':': 
	    return 84;
	case '"': 
	    return 85;
	case '\'': 
	    return 86;

	case '<': 
	    return 87;
	case ',': 
	    return 88;
	case '>': 
	    return 89;
	case '.': 
	    return 90;
	case '?': 
	    return 91;
	default: 
	    return - 1;
    }
}

#undef MID	
#undef MD	
#undef RID	
#undef CD	
#undef EDD
#undef OD  
#undef OID  
#undef XD 


