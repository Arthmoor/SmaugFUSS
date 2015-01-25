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
 *                           Informational module                           *
 ****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "mud.h"
#include "mapper.h"
#include "sha256.h"

bool in_same_house( CHAR_DATA * ch, CHAR_DATA * vch );

/*
 * Needed in the do_ignore function. -Orion
 */
bool check_parse_name( const char *name, bool newchar );

/*
 * Local functions.
 */
void show_char_to_char_0( CHAR_DATA * victim, CHAR_DATA * ch );
void show_char_to_char_1( CHAR_DATA * victim, CHAR_DATA * ch );
void show_char_to_char( CHAR_DATA * list, CHAR_DATA * ch );
bool check_blind( CHAR_DATA * ch );
void show_condition( CHAR_DATA * ch, CHAR_DATA * victim );

/*
 * Keep players from defeating examine progs -Druid
 * False = do not trigger
 * True = Trigger
 */
bool EXA_prog_trigger = TRUE;

/* Had to add unknowns because someone added new positions and didn't
 * update them.  Just a band-aid till I have time to fix it right.
 * This was found thanks to mud@mini.axcomp.com pointing it out :)
 * --Shaddai
 */
const char *const where_name[] = {
   "<used as light>     ",
   "<worn on finger>    ",
   "<worn on finger>    ",
   "<worn around neck>  ",
   "<worn around neck>  ",
   "<worn on body>      ",
   "<worn on head>      ",
   "<worn on legs>      ",
   "<worn on feet>      ",
   "<worn on hands>     ",
   "<worn on arms>      ",
   "<worn as shield>    ",
   "<worn about body>   ",
   "<worn about waist>  ",
   "<worn around wrist> ",
   "<worn around wrist> ",
   "<wielded>           ",
   "<held>              ",
   "<dual wielded>      ",
   "<worn on ears>      ",
   "<worn on eyes>      ",
   "<missile wielded>   ",
   "<worn on back>      ",
   "<worn over face>    ",
   "<worn around ankle> ",
   "<worn around ankle> ",
   "<BUG Inform Nivek>  ",
   "<BUG Inform Nivek>  ",
   "<BUG Inform Nivek>  "
};

/*
StarMap was written by Nebseni of Clandestine MUD and ported to Smaug
by Desden, el Chaman Tibetano.
*/

#define NUM_DAYS 35
/* Match this to the number of days per month; this is the moon cycle */
#define NUM_MONTHS 17
/* Match this to the number of months defined in month_name[].  */
#define MAP_WIDTH 72
#define MAP_HEIGHT 8
/* Should be the string length and number of the constants below.*/

const char *star_map[] = {
   "                                               C. C.                  g*",
   "    O:       R*        G*    G.  W* W. W.          C. C.    Y* Y. Y.    ",
   "  O*.                c.          W.W.     W.            C.       Y..Y.  ",
   "O.O. O.              c.  G..G.           W:      B*                   Y.",
   "     O.    c.     c.                     W. W.                  r*    Y.",
   "     O.c.     c.      G.             P..     W.        p.      Y.   Y:  ",
   "        c.                    G*    P.  P.           p.  p:     Y.   Y. ",
   "                 b*             P.: P*                 p.p:             "
};

/****************** CONSTELLATIONS and STARS *****************************
  Cygnus     Mars        Orion      Dragon       Cassiopeia          Venus
           Ursa Ninor                           Mercurius     Pluto    
               Uranus              Leo                Crown       Raptor
*************************************************************************/

const char *sun_map[] = {
   "\\`|'/",
   "- O -",
   "/.|.\\"
};

const char *moon_map[] = {
   " @@@ ",
   "@@@@@",
   " @@@ "
};

void look_sky( CHAR_DATA * ch )
{
   struct WeatherCell *cell = getWeatherCell( ch->in_room->area );
   char buf[MAX_STRING_LENGTH];
   char buf2[4];
   int starpos, sunpos, moonpos, moonphase, i, linenum;

   send_to_pager( "You gaze up towards the heavens and see:\r\n", ch );

   if( isModeratelyCloudy( getCloudCover( cell ) ) )
   {
      send_to_char( "There are too many clouds in the sky so you cannot see anything else.\r\n", ch );
      return;
   }

   sunpos = ( MAP_WIDTH * ( 24 - time_info.hour ) / 24 );
   moonpos = ( sunpos + time_info.day * MAP_WIDTH / NUM_DAYS ) % MAP_WIDTH;
   if( ( moonphase = ( ( ( ( MAP_WIDTH + moonpos - sunpos ) % MAP_WIDTH ) + ( MAP_WIDTH / 16 ) ) * 8 ) / MAP_WIDTH ) > 4 )
      moonphase -= 8;
   starpos = ( sunpos + MAP_WIDTH * time_info.month / NUM_MONTHS ) % MAP_WIDTH;

   /*
    * The left end of the star_map will be straight overhead at midnight during month 0 
    */
   for( linenum = 0; linenum < MAP_HEIGHT; linenum++ )
   {
      if( ( time_info.hour >= 6 && time_info.hour <= 18 ) && ( linenum < 3 || linenum >= 6 ) )
         continue;

      mudstrlcpy( buf, " ", MAX_STRING_LENGTH );

      /*
       * for ( i = MAP_WIDTH/4; i <= 3*MAP_WIDTH/4; i++)
       */
      for( i = 1; i <= MAP_WIDTH; i++ )
      {
         /*
          * plot moon on top of anything else...unless new moon & no eclipse 
          */
         if( ( time_info.hour >= 6 && time_info.hour <= 18 )   /* daytime? */
             && ( moonpos >= MAP_WIDTH / 4 - 2 ) && ( moonpos <= 3 * MAP_WIDTH / 4 + 2 )  /* in sky? */
             && ( i >= moonpos - 2 ) && ( i <= moonpos + 2 )   /* is this pixel near moon? */
             && ( ( sunpos == moonpos && time_info.hour == 12 ) || moonphase != 0 ) /*no eclipse */
             && ( moon_map[linenum - 3][i + 2 - moonpos] == '@' ) )
         {
            if( ( moonphase < 0 && i - 2 - moonpos >= moonphase ) || ( moonphase > 0 && i + 2 - moonpos <= moonphase ) )
               mudstrlcat( buf, "&W@", MAX_STRING_LENGTH );
            else
               mudstrlcat( buf, " ", MAX_STRING_LENGTH );
         }
         else if( ( linenum >= 3 ) && ( linenum < 6 ) && /* nighttime */
                  ( moonpos >= MAP_WIDTH / 4 - 2 ) && ( moonpos <= 3 * MAP_WIDTH / 4 + 2 )   /* in sky? */
                  && ( i >= moonpos - 2 ) && ( i <= moonpos + 2 ) /* is this pixel near moon? */
                  && ( moon_map[linenum - 3][i + 2 - moonpos] == '@' ) )
         {
            if( ( moonphase < 0 && i - 2 - moonpos >= moonphase ) || ( moonphase > 0 && i + 2 - moonpos <= moonphase ) )
               mudstrlcat( buf, "&W@", MAX_STRING_LENGTH );
            else
               mudstrlcat( buf, " ", MAX_STRING_LENGTH );
         }
         else  /* plot sun or stars */
         {
            if( time_info.hour >= 6 && time_info.hour <= 18 )  /* daytime */
            {
               if( i >= sunpos - 2 && i <= sunpos + 2 )
               {
                  snprintf( buf2, 4, "&Y%c", sun_map[linenum - 3][i + 2 - sunpos] );
                  mudstrlcat( buf, buf2, MAX_STRING_LENGTH );
               }
               else
                  mudstrlcat( buf, " ", MAX_STRING_LENGTH );
            }
            else
            {
               switch ( star_map[linenum][( MAP_WIDTH + i - starpos ) % MAP_WIDTH] )
               {
                  default:
                     mudstrlcat( buf, " ", MAX_STRING_LENGTH );
                     break;
                  case ':':
                     mudstrlcat( buf, ":", MAX_STRING_LENGTH );
                     break;
                  case '.':
                     mudstrlcat( buf, ".", MAX_STRING_LENGTH );
                     break;
                  case '*':
                     mudstrlcat( buf, "*", MAX_STRING_LENGTH );
                     break;
                  case 'G':
                     mudstrlcat( buf, "&G ", MAX_STRING_LENGTH );
                     break;
                  case 'g':
                     mudstrlcat( buf, "&g ", MAX_STRING_LENGTH );
                     break;
                  case 'R':
                     mudstrlcat( buf, "&R ", MAX_STRING_LENGTH );
                     break;
                  case 'r':
                     mudstrlcat( buf, "&r ", MAX_STRING_LENGTH );
                     break;
                  case 'C':
                     mudstrlcat( buf, "&C ", MAX_STRING_LENGTH );
                     break;
                  case 'O':
                     mudstrlcat( buf, "&O ", MAX_STRING_LENGTH );
                     break;
                  case 'B':
                     mudstrlcat( buf, "&B ", MAX_STRING_LENGTH );
                     break;
                  case 'P':
                     mudstrlcat( buf, "&P ", MAX_STRING_LENGTH );
                     break;
                  case 'W':
                     mudstrlcat( buf, "&W ", MAX_STRING_LENGTH );
                     break;
                  case 'b':
                     mudstrlcat( buf, "&b ", MAX_STRING_LENGTH );
                     break;
                  case 'p':
                     mudstrlcat( buf, "&p ", MAX_STRING_LENGTH );
                     break;
                  case 'Y':
                     mudstrlcat( buf, "&Y ", MAX_STRING_LENGTH );
                     break;
                  case 'c':
                     mudstrlcat( buf, "&c ", MAX_STRING_LENGTH );
                     break;
               }
            }
         }
      }
      mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
      send_to_pager( buf, ch );
   }
}

char *format_obj_to_char( OBJ_DATA * obj, CHAR_DATA * ch, bool fShort )
{
   static char buf[MAX_STRING_LENGTH];
   bool glowsee = FALSE;

   /*
    * can see glowing invis items in the dark 
    */
   if( IS_OBJ_STAT( obj, ITEM_GLOW ) && IS_OBJ_STAT( obj, ITEM_INVIS )
       && !IS_AFFECTED( ch, AFF_TRUESIGHT ) && !IS_AFFECTED( ch, AFF_DETECT_INVIS ) )
      glowsee = TRUE;

   buf[0] = '\0';
   if( IS_OBJ_STAT( obj, ITEM_INVIS ) )
      mudstrlcat( buf, "(Invis) ", MAX_STRING_LENGTH );
   if( ( IS_AFFECTED( ch, AFF_DETECT_EVIL ) || ch->Class == CLASS_PALADIN ) && IS_OBJ_STAT( obj, ITEM_EVIL ) )
      mudstrlcat( buf, "(Red Aura) ", MAX_STRING_LENGTH );

   if( ch->Class == CLASS_PALADIN
       && ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL ) && !IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL )
            && !IS_OBJ_STAT( obj, ITEM_ANTI_GOOD ) ) )
      mudstrlcat( buf, "(Flaming Red) ", MAX_STRING_LENGTH );
   if( ch->Class == CLASS_PALADIN
       && ( !IS_OBJ_STAT( obj, ITEM_ANTI_EVIL ) && IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL )
            && !IS_OBJ_STAT( obj, ITEM_ANTI_GOOD ) ) )
      mudstrlcat( buf, "(Flaming Grey) ", MAX_STRING_LENGTH );
   if( ch->Class == CLASS_PALADIN
       && ( !IS_OBJ_STAT( obj, ITEM_ANTI_EVIL ) && !IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL )
            && IS_OBJ_STAT( obj, ITEM_ANTI_GOOD ) ) )
      mudstrlcat( buf, "(Flaming White) ", MAX_STRING_LENGTH );

   if( ch->Class == CLASS_PALADIN
       && ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL ) && IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL )
            && !IS_OBJ_STAT( obj, ITEM_ANTI_GOOD ) ) )
      mudstrlcat( buf, "(Smouldering Red-Grey) ", MAX_STRING_LENGTH );
   if( ch->Class == CLASS_PALADIN
       && ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL ) && !IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL )
            && IS_OBJ_STAT( obj, ITEM_ANTI_GOOD ) ) )
      mudstrlcat( buf, "(Smouldering Red-White) ", MAX_STRING_LENGTH );
   if( ch->Class == CLASS_PALADIN
       && ( !IS_OBJ_STAT( obj, ITEM_ANTI_EVIL ) && IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL )
            && IS_OBJ_STAT( obj, ITEM_ANTI_GOOD ) ) )
      mudstrlcat( buf, "(Smouldering Grey-White) ", MAX_STRING_LENGTH );

   if( ( IS_AFFECTED( ch, AFF_DETECT_MAGIC ) || xIS_SET( ch->act, PLR_HOLYLIGHT ) ) && IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      mudstrlcat( buf, "(Magical) ", MAX_STRING_LENGTH );
   if( !glowsee && IS_OBJ_STAT( obj, ITEM_GLOW ) )
      mudstrlcat( buf, "(Glowing) ", MAX_STRING_LENGTH );
   if( IS_OBJ_STAT( obj, ITEM_HUM ) )
      mudstrlcat( buf, "(Humming) ", MAX_STRING_LENGTH );
   if( IS_OBJ_STAT( obj, ITEM_HIDDEN ) )
      mudstrlcat( buf, "(Hidden) ", MAX_STRING_LENGTH );
   if( IS_OBJ_STAT( obj, ITEM_BURIED ) )
      mudstrlcat( buf, "(Buried) ", MAX_STRING_LENGTH );
   if( IS_IMMORTAL( ch ) && IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      mudstrlcat( buf, "(PROTO) ", MAX_STRING_LENGTH );
   if( ( IS_AFFECTED( ch, AFF_DETECTTRAPS ) || xIS_SET( ch->act, PLR_HOLYLIGHT ) ) && is_trapped( obj ) )
      mudstrlcat( buf, "(Trap) ", MAX_STRING_LENGTH );

   if( fShort )
   {
      if( glowsee && ( IS_NPC( ch ) || !xIS_SET( ch->act, PLR_HOLYLIGHT ) ) )
         mudstrlcpy( buf, "the faint glow of something", MAX_STRING_LENGTH );
      else if( obj->short_descr )
         mudstrlcat( buf, obj->short_descr, MAX_STRING_LENGTH );
   }
   else
   {
      if( glowsee && ( IS_NPC( ch ) || !xIS_SET( ch->act, PLR_HOLYLIGHT ) ) )
         mudstrlcpy( buf, "You see the faint glow of something nearby.", MAX_STRING_LENGTH );
      else if( obj->description )
         mudstrlcat( buf, obj->description, MAX_STRING_LENGTH );
   }
   return buf;
}

/*
 * Some increasingly freaky hallucinated objects		-Thoric
 * (Hats off to Albert Hoffman's "problem child")
 */
const char *hallucinated_object( int ms, bool fShort )
{
   int sms = URANGE( 1, ( ms + 10 ) / 5, 20 );

   if( fShort )
      switch ( number_range( 6 - URANGE( 1, sms / 2, 5 ), sms ) )
      {
         case 1:
            return "a sword";
         case 2:
            return "a stick";
         case 3:
            return "something shiny";
         case 4:
            return "something";
         case 5:
            return "something interesting";
         case 6:
            return "something colorful";
         case 7:
            return "something that looks cool";
         case 8:
            return "a nifty thing";
         case 9:
            return "a cloak of flowing colors";
         case 10:
            return "a mystical flaming sword";
         case 11:
            return "a swarm of insects";
         case 12:
            return "a deathbane";
         case 13:
            return "a figment of your imagination";
         case 14:
            return "your gravestone";
         case 15:
            return "the long lost boots of Ranger Thoric";
         case 16:
            return "a glowing tome of arcane knowledge";
         case 17:
            return "a long sought secret";
         case 18:
            return "the meaning of it all";
         case 19:
            return "the answer";
         case 20:
            return "the key to life, the universe and everything";
      }
   switch ( number_range( 6 - URANGE( 1, sms / 2, 5 ), sms ) )
   {
      case 1:
         return "A nice looking sword catches your eye.";
      case 2:
         return "The ground is covered in small sticks.";
      case 3:
         return "Something shiny catches your eye.";
      case 4:
         return "Something catches your attention.";
      case 5:
         return "Something interesting catches your eye.";
      case 6:
         return "Something colorful flows by.";
      case 7:
         return "Something that looks cool calls out to you.";
      case 8:
         return "A nifty thing of great importance stands here.";
      case 9:
         return "A cloak of flowing colors asks you to wear it.";
      case 10:
         return "A mystical flaming sword awaits your grasp.";
      case 11:
         return "A swarm of insects buzzes in your face!";
      case 12:
         return "The extremely rare Deathbane lies at your feet.";
      case 13:
         return "A figment of your imagination is at your command.";
      case 14:
         return "You notice a gravestone here... upon closer examination, it reads your name.";
      case 15:
         return "The long lost boots of Ranger Thoric lie off to the side.";
      case 16:
         return "A glowing tome of arcane knowledge hovers in the air before you.";
      case 17:
         return "A long sought secret of all mankind is now clear to you.";
      case 18:
         return "The meaning of it all, so simple, so clear... of course!";
      case 19:
         return "The answer.  One.  It's always been One.";
      case 20:
         return "The key to life, the universe and everything awaits your hand.";
   }
   return "Whoa!!!";
}

/* This is the punct snippet from Desden el Chaman Tibetano - Nov 1998
   Email: jlalbatros@mx2.redestb.es
*/
char *num_punct( int foo )
{
   int index_new, rest, x;
   unsigned int nindex;
   char buf[16];
   static char buf_new[16];

   snprintf( buf, 16, "%d", foo );
   rest = strlen( buf ) % 3;

   for( nindex = index_new = 0; nindex < strlen( buf ); nindex++, index_new++ )
   {
      x = nindex - rest;
      if( nindex != 0 && ( x % 3 ) == 0 )
      {
         buf_new[index_new] = ',';
         index_new++;
         buf_new[index_new] = buf[nindex];
      }
      else
         buf_new[index_new] = buf[nindex];
   }
   buf_new[index_new] = '\0';
   return buf_new;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing )
{
   char **prgpstrShow;
   int *prgnShow;
   int *pitShow;
   char *pstrShow;
   OBJ_DATA *obj;
   int nShow;
   int iShow;
   int count, offcount, tmp, ms, cnt;
   bool fCombine;

   if( !ch->desc )
      return;

   /*
    * if there's no list... then don't do all this crap!  -Thoric
    */
   if( !list )
   {
      if( fShowNothing )
      {
         if( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
            send_to_char( "     ", ch );
         set_char_color( AT_OBJECT, ch );
         send_to_char( "Nothing.\r\n", ch );
      }
      return;
   }

   /*
    * Alloc space for output lines.
    */
   count = 0;
   for( obj = list; obj; obj = obj->next_content )
      count++;

   ms = ( ch->mental_state ? ch->mental_state : 1 )
      * ( IS_NPC( ch ) ? 1 : ( ch->pcdata->condition[COND_DRUNK] ? ( ch->pcdata->condition[COND_DRUNK] / 12 ) : 1 ) );

   /*
    * If not mentally stable...
    */
   if( abs( ms ) > 40 )
   {
      offcount = URANGE( -( count ), ( count * ms ) / 100, count * 2 );
      if( offcount < 0 )
         offcount += number_range( 0, abs( offcount ) );
      else if( offcount > 0 )
         offcount -= number_range( 0, offcount );
   }
   else
      offcount = 0;

   if( count + offcount <= 0 )
   {
      if( fShowNothing )
      {
         if( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
            send_to_char( "     ", ch );
         set_char_color( AT_OBJECT, ch );
         send_to_char( "Nothing.\r\n", ch );
      }
      return;
   }

   CREATE( prgpstrShow, char *, count + ( ( offcount > 0 ) ? offcount : 0 ) );
   CREATE( prgnShow, int, count + ( ( offcount > 0 ) ? offcount : 0 ) );
   CREATE( pitShow, int, count + ( ( offcount > 0 ) ? offcount : 0 ) );
   nShow = 0;
   tmp = ( offcount > 0 ) ? offcount : 0;
   cnt = 0;

   /*
    * Format the list of objects.
    */
   for( obj = list; obj; obj = obj->next_content )
   {
      if( offcount < 0 && ++cnt > ( count + offcount ) )
         break;
      if( tmp > 0 && number_bits( 1 ) == 0 )
      {
         prgpstrShow[nShow] = str_dup( hallucinated_object( ms, fShort ) );
         prgnShow[nShow] = 1;
         pitShow[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
         nShow++;
         --tmp;
      }
      if( obj->wear_loc == WEAR_NONE
          && can_see_obj( ch, obj ) && ( obj->item_type != ITEM_TRAP || IS_AFFECTED( ch, AFF_DETECTTRAPS ) ) )
      {
         pstrShow = format_obj_to_char( obj, ch, fShort );
         fCombine = FALSE;

         if( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
         {
            /*
             * Look for duplicates, case sensitive.
             * Matches tend to be near end so run loop backwords.
             */
            for( iShow = nShow - 1; iShow >= 0; iShow-- )
            {
               if( !strcmp( prgpstrShow[iShow], pstrShow ) )
               {
                  prgnShow[iShow] += obj->count;
                  fCombine = TRUE;
                  break;
               }
            }
         }

         pitShow[nShow] = obj->item_type;
         /*
          * Couldn't combine, or didn't want to.
          */
         if( !fCombine )
         {
            prgpstrShow[nShow] = str_dup( pstrShow );
            prgnShow[nShow] = obj->count;
            nShow++;
         }
      }
   }
   if( tmp > 0 )
   {
      int x;
      for( x = 0; x < tmp; x++ )
      {
         prgpstrShow[nShow] = str_dup( hallucinated_object( ms, fShort ) );
         prgnShow[nShow] = 1;
         pitShow[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
         nShow++;
      }
   }

   /*
    * Output the formatted list.      -Color support by Thoric
    */
   for( iShow = 0; iShow < nShow; iShow++ )
   {
      switch ( pitShow[iShow] )
      {
         default:
            set_char_color( AT_OBJECT, ch );
            break;
         case ITEM_BLOOD:
            set_char_color( AT_BLOOD, ch );
            break;
         case ITEM_CORPSE_PC:
         case ITEM_CORPSE_NPC:
            set_char_color( AT_ORANGE, ch );
            break;
         case ITEM_MONEY:
         case ITEM_TREASURE:
            set_char_color( AT_YELLOW, ch );
            break;
         case ITEM_COOK:
         case ITEM_FOOD:
            set_char_color( AT_HUNGRY, ch );
            break;
         case ITEM_DRINK_CON:
         case ITEM_FOUNTAIN:
         case ITEM_PUDDLE:
            set_char_color( AT_THIRSTY, ch );
            break;
         case ITEM_FIRE:
            set_char_color( AT_FIRE, ch );
            break;
         case ITEM_SCROLL:
         case ITEM_WAND:
         case ITEM_STAFF:
            set_char_color( AT_MAGIC, ch );
            break;
      }
      if( fShowNothing )
         send_to_char( "     ", ch );
      send_to_char( prgpstrShow[iShow], ch );
/*	if ( IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE) ) */
      {
         if( prgnShow[iShow] != 1 )
            ch_printf( ch, " (%d)", prgnShow[iShow] );
      }

      send_to_char( "\r\n", ch );
      DISPOSE( prgpstrShow[iShow] );
   }

   if( fShowNothing && nShow == 0 )
   {
      if( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
         send_to_char( "     ", ch );
      set_char_color( AT_OBJECT, ch );
      send_to_char( "Nothing.\r\n", ch );
   }

   /*
    * Clean up.
    */
   DISPOSE( prgpstrShow );
   DISPOSE( prgnShow );
   DISPOSE( pitShow );
   return;
}

/*
 * Show fancy descriptions for certain spell affects		-Thoric
 */
void show_visible_affects_to_char( CHAR_DATA * victim, CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   char name[MAX_STRING_LENGTH];

   if( IS_NPC( victim ) )
      mudstrlcpy( name, victim->short_descr, MAX_STRING_LENGTH );
   else
      mudstrlcpy( name, victim->name, MAX_STRING_LENGTH );
   name[0] = toupper( name[0] );

   if( IS_AFFECTED( victim, AFF_SANCTUARY ) )
   {
      set_char_color( AT_WHITE, ch );
      if( IS_GOOD( victim ) )
         ch_printf( ch, "%s glows with an aura of divine radiance.\r\n", name );
      else if( IS_EVIL( victim ) )
         ch_printf( ch, "%s shimmers beneath an aura of dark energy.\r\n", name );
      else
         ch_printf( ch, "%s is shrouded in flowing shadow and light.\r\n", name );
   }
   if( IS_AFFECTED( victim, AFF_FIRESHIELD ) )
   {
      set_char_color( AT_FIRE, ch );
      ch_printf( ch, "%s is engulfed within a blaze of mystical flame.\r\n", name );
   }
   if( IS_AFFECTED( victim, AFF_SHOCKSHIELD ) )
   {
      set_char_color( AT_BLUE, ch );
      ch_printf( ch, "%s is surrounded by cascading torrents of energy.\r\n", name );
   }
   if( IS_AFFECTED( victim, AFF_ACIDMIST ) )
   {
      set_char_color( AT_GREEN, ch );
      ch_printf( ch, "%s is visible through a cloud of churning mist.\r\n", name );
   }
/*Scryn 8/13*/
   if( IS_AFFECTED( victim, AFF_ICESHIELD ) )
   {
      set_char_color( AT_LBLUE, ch );
      ch_printf( ch, "%s is ensphered by shards of glistening ice.\r\n", name );
   }
   if( IS_AFFECTED( victim, AFF_VENOMSHIELD ) )
   {
      set_char_color( AT_GREEN, ch );
      ch_printf( ch, "%s is enshrouded in a choking cloud of gas.\r\n", name );
   }
   if( IS_AFFECTED( victim, AFF_CHARM ) )
   {
      set_char_color( AT_MAGIC, ch );
      ch_printf( ch, "%s wanders in a dazed, zombie-like state.\r\n", name );
   }
   if( !IS_NPC( victim ) && !victim->desc && victim->switched && IS_AFFECTED( victim->switched, AFF_POSSESS ) )
   {
      set_char_color( AT_MAGIC, ch );
      mudstrlcpy( buf, PERS( victim, ch ), MAX_STRING_LENGTH );
      mudstrlcat( buf, " appears to be in a deep trance...\r\n", MAX_STRING_LENGTH );
   }
}

void show_char_to_char_0( CHAR_DATA * victim, CHAR_DATA * ch )
{
   TIMER *timer;
   char buf[MAX_STRING_LENGTH];
   char buf1[MAX_STRING_LENGTH];

   buf[0] = '\0';

   set_char_color( AT_PERSON, ch );
   if( !IS_NPC( victim ) && !victim->desc )
   {
      if( !victim->switched )
         send_to_char_color( "&P[(Link Dead)] ", ch );
      else if( !IS_AFFECTED( victim, AFF_POSSESS ) )
         mudstrlcat( buf, "(Switched) ", MAX_STRING_LENGTH );
   }

   if( IS_NPC( victim ) && IS_AFFECTED( victim, AFF_POSSESS ) && IS_IMMORTAL( ch ) && victim->desc )
   {
      snprintf( buf1, MAX_STRING_LENGTH, "(%s)", victim->desc->original->name );
      mudstrlcat( buf, buf1, MAX_STRING_LENGTH );
   }

   if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_AFK ) )
      mudstrlcat( buf, "[AFK] ", MAX_STRING_LENGTH );

   if( ( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_WIZINVIS ) )
       || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_MOBINVIS ) ) )
   {
      if( !IS_NPC( victim ) )
         snprintf( buf1, MAX_STRING_LENGTH, "(Invis %d) ", victim->pcdata->wizinvis );
      else
         snprintf( buf1, MAX_STRING_LENGTH, "(Mobinvis %d) ", victim->mobinvis );
      mudstrlcat( buf, buf1, MAX_STRING_LENGTH );
   }

   if( !IS_NPC( victim ) )
   {
      if( IS_IMMORTAL( victim ) && victim->level > LEVEL_AVATAR )
         send_to_char_color( "&P(&WImmortal&P) ", ch );
      if( victim->pcdata->clan
          && IS_SET( victim->pcdata->flags, PCFLAG_DEADLY )
          && victim->pcdata->clan->badge
          && ( victim->pcdata->clan->clan_type != CLAN_ORDER && victim->pcdata->clan->clan_type != CLAN_GUILD ) )
         ch_printf_color( ch, "&P%s ", victim->pcdata->clan->badge );
      else if( CAN_PKILL( victim ) && victim->level < LEVEL_IMMORTAL )
         send_to_char_color( "&P(&wUnclanned&P) ", ch );
   }

   set_char_color( AT_PERSON, ch );

   if( IS_AFFECTED( victim, AFF_INVISIBLE ) )
      mudstrlcat( buf, "(Invis) ", MAX_STRING_LENGTH );
   if( IS_AFFECTED( victim, AFF_HIDE ) )
      mudstrlcat( buf, "(Hide) ", MAX_STRING_LENGTH );
   if( IS_AFFECTED( victim, AFF_PASS_DOOR ) )
      mudstrlcat( buf, "(Translucent) ", MAX_STRING_LENGTH );
   if( IS_AFFECTED( victim, AFF_FAERIE_FIRE ) )
      mudstrlcat( buf, "(Pink Aura) ", MAX_STRING_LENGTH );
   if( IS_EVIL( victim ) && ( IS_AFFECTED( ch, AFF_DETECT_EVIL ) || ch->Class == CLASS_PALADIN ) )
      mudstrlcat( buf, "(Red Aura) ", MAX_STRING_LENGTH );
   if( IS_NEUTRAL( victim ) && ch->Class == CLASS_PALADIN )
      mudstrlcat( buf, "(Grey Aura) ", MAX_STRING_LENGTH );
   if( IS_GOOD( victim ) && ch->Class == CLASS_PALADIN )
      mudstrlcat( buf, "(White Aura) ", MAX_STRING_LENGTH );

   if( IS_AFFECTED( victim, AFF_BERSERK ) )
      mudstrlcat( buf, "(Wild-eyed) ", MAX_STRING_LENGTH );
   if( IS_AFFECTED( victim, AFF_GRAPPLE ) )
      mudstrlcat( buf, "(Grappling) ", MAX_STRING_LENGTH );
   if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_ATTACKER ) )
      mudstrlcat( buf, "(ATTACKER) ", MAX_STRING_LENGTH );
   if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_KILLER ) )
      mudstrlcat( buf, "(KILLER) ", MAX_STRING_LENGTH );
   if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_THIEF ) )
      mudstrlcat( buf, "(THIEF) ", MAX_STRING_LENGTH );
   if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_LITTERBUG ) )
      mudstrlcat( buf, "(LITTERBUG) ", MAX_STRING_LENGTH );
   if( IS_NPC( victim ) && IS_IMMORTAL( ch ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
      mudstrlcat( buf, "(PROTO) ", MAX_STRING_LENGTH );
   if( IS_NPC( victim ) && ch->mount && ch->mount == victim && ch->in_room == ch->mount->in_room )
      mudstrlcat( buf, "(Mount) ", MAX_STRING_LENGTH );
   if( victim->desc && victim->desc->connected == CON_EDITING )
      mudstrlcat( buf, "(Writing) ", MAX_STRING_LENGTH );
   if( victim->morph != NULL )
      mudstrlcat( buf, "(Morphed) ", MAX_STRING_LENGTH );

   set_char_color( AT_PERSON, ch );
   if( ( victim->position == victim->defposition && victim->long_descr[0] != '\0' )
       || ( victim->morph && victim->morph->morph && victim->morph->morph->defpos == victim->position ) )
   {
      if( victim->morph != NULL )
      {
         if( !IS_IMMORTAL( ch ) )
         {
            if( victim->morph->morph != NULL )
               mudstrlcat( buf, victim->morph->morph->long_desc, MAX_STRING_LENGTH );
            else
               mudstrlcat( buf, victim->long_descr, MAX_STRING_LENGTH );
         }
         else
         {
            mudstrlcat( buf, PERS( victim, ch ), MAX_STRING_LENGTH );
            if( !IS_NPC( victim ) && !xIS_SET( ch->act, PLR_BRIEF ) )
               mudstrlcat( buf, victim->pcdata->title, MAX_STRING_LENGTH );
            mudstrlcat( buf, ".\r\n", MAX_STRING_LENGTH );
         }
      }
      else
         mudstrlcat( buf, victim->long_descr, MAX_STRING_LENGTH );
      send_to_char( buf, ch );
      show_visible_affects_to_char( victim, ch );
      return;
   }
   else
   {
      if( victim->morph != NULL && victim->morph->morph != NULL && !IS_IMMORTAL( ch ) )
         mudstrlcat( buf, MORPHPERS( victim, ch ), MAX_STRING_LENGTH );
      else
         mudstrlcat( buf, PERS( victim, ch ), MAX_STRING_LENGTH );
   }

   if( !IS_NPC( victim ) && !xIS_SET( ch->act, PLR_BRIEF ) )
      mudstrlcat( buf, victim->pcdata->title, MAX_STRING_LENGTH );

   if( ( timer = get_timerptr( victim, TIMER_DO_FUN ) ) != NULL )
   {
      if( timer->do_fun == do_cast )
         mudstrlcat( buf, " is here chanting.", MAX_STRING_LENGTH );
      else if( timer->do_fun == do_dig )
         mudstrlcat( buf, " is here digging.", MAX_STRING_LENGTH );
      else if( timer->do_fun == do_search )
         mudstrlcat( buf, " is searching the area for something.", MAX_STRING_LENGTH );
      else if( timer->do_fun == do_detrap )
         mudstrlcat( buf, " is working with the trap here.", MAX_STRING_LENGTH );
      else
         mudstrlcat( buf, " is looking rather lost.", MAX_STRING_LENGTH );
   }
   else
   {
      switch ( victim->position )
      {
         case POS_DEAD:
            mudstrlcat( buf, " is DEAD!!", MAX_STRING_LENGTH );
            break;
         case POS_MORTAL:
            mudstrlcat( buf, " is mortally wounded.", MAX_STRING_LENGTH );
            break;
         case POS_INCAP:
            mudstrlcat( buf, " is incapacitated.", MAX_STRING_LENGTH );
            break;
         case POS_STUNNED:
            mudstrlcat( buf, " is lying here stunned.", MAX_STRING_LENGTH );
            break;
         case POS_SLEEPING:
            if( ch->position == POS_SITTING || ch->position == POS_RESTING )
               mudstrlcat( buf, " is sleeping nearby.", MAX_STRING_LENGTH );
            else
               mudstrlcat( buf, " is deep in slumber here.", MAX_STRING_LENGTH );
            break;
         case POS_RESTING:
            if( ch->position == POS_RESTING )
               mudstrlcat( buf, " is sprawled out alongside you.", MAX_STRING_LENGTH );
            else if( ch->position == POS_MOUNTED )
               mudstrlcat( buf, " is sprawled out at the foot of your mount.", MAX_STRING_LENGTH );
            else
               mudstrlcat( buf, " is sprawled out here.", MAX_STRING_LENGTH );
            break;
         case POS_SITTING:
            if( ch->position == POS_SITTING )
               mudstrlcat( buf, " sits here with you.", MAX_STRING_LENGTH );
            else if( ch->position == POS_RESTING )
               mudstrlcat( buf, " sits nearby as you lie around.", MAX_STRING_LENGTH );
            else
               mudstrlcat( buf, " sits upright here.", MAX_STRING_LENGTH );
            break;
         case POS_STANDING:
            if( IS_IMMORTAL( victim ) )
               mudstrlcat( buf, " radiates with a godly light.", MAX_STRING_LENGTH );
            else
               if( ( victim->in_room->sector_type == SECT_UNDERWATER )
                   && !IS_AFFECTED( victim, AFF_AQUA_BREATH ) && !IS_NPC( victim ) )
               mudstrlcat( buf, " is drowning here.", MAX_STRING_LENGTH );
            else if( victim->in_room->sector_type == SECT_UNDERWATER )
               mudstrlcat( buf, " is here in the water.", MAX_STRING_LENGTH );
            else
               if( ( victim->in_room->sector_type == SECT_OCEANFLOOR )
                   && !IS_AFFECTED( victim, AFF_AQUA_BREATH ) && !IS_NPC( victim ) )
               mudstrlcat( buf, " is drowning here.", MAX_STRING_LENGTH );
            else if( victim->in_room->sector_type == SECT_OCEANFLOOR )
               mudstrlcat( buf, " is standing here in the water.", MAX_STRING_LENGTH );
            else if( IS_AFFECTED( victim, AFF_FLOATING ) || IS_AFFECTED( victim, AFF_FLYING ) )
               mudstrlcat( buf, " is hovering here.", MAX_STRING_LENGTH );
            else
               mudstrlcat( buf, " is standing here.", MAX_STRING_LENGTH );
            break;
         case POS_SHOVE:
            mudstrlcat( buf, " is being shoved around.", MAX_STRING_LENGTH );
            break;
         case POS_DRAG:
            mudstrlcat( buf, " is being dragged around.", MAX_STRING_LENGTH );
            break;
         case POS_MOUNTED:
            mudstrlcat( buf, " is here, upon ", MAX_STRING_LENGTH );
            if( !victim->mount )
               mudstrlcat( buf, "thin air???", MAX_STRING_LENGTH );
            else if( victim->mount == ch )
               mudstrlcat( buf, "your back.", MAX_STRING_LENGTH );
            else if( victim->in_room == victim->mount->in_room )
            {
               mudstrlcat( buf, PERS( victim->mount, ch ), MAX_STRING_LENGTH );
               mudstrlcat( buf, ".", MAX_STRING_LENGTH );
            }
            else
               mudstrlcat( buf, "someone who left??", MAX_STRING_LENGTH );
            break;
         case POS_FIGHTING:
         case POS_EVASIVE:
         case POS_DEFENSIVE:
         case POS_AGGRESSIVE:
         case POS_BERSERK:
            mudstrlcat( buf, " is here, fighting ", MAX_STRING_LENGTH );
            if( !victim->fighting )
            {
               mudstrlcat( buf, "thin air???", MAX_STRING_LENGTH );

               /*
                * some bug somewhere.... kinda hackey fix -h 
                */
               if( !victim->mount )
                  victim->position = POS_STANDING;
               else
                  victim->position = POS_MOUNTED;
            }
            else if( who_fighting( victim ) == ch )
               mudstrlcat( buf, "YOU!", MAX_STRING_LENGTH );
            else if( victim->in_room == victim->fighting->who->in_room )
            {
               mudstrlcat( buf, PERS( victim->fighting->who, ch ), MAX_STRING_LENGTH );
               mudstrlcat( buf, ".", MAX_STRING_LENGTH );
            }
            else
               mudstrlcat( buf, "someone who left??", MAX_STRING_LENGTH );
            break;
      }
   }

   mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
   buf[0] = UPPER( buf[0] );
   send_to_char( buf, ch );
   show_visible_affects_to_char( victim, ch );
   return;
}

void show_char_to_char_1( CHAR_DATA * victim, CHAR_DATA * ch )
{
   OBJ_DATA *obj;
   int iWear;
   bool found;

   if( can_see( victim, ch ) && !IS_NPC( ch ) && !xIS_SET( ch->act, PLR_WIZINVIS ) )
   {
      act( AT_ACTION, "$n looks at you.", ch, NULL, victim, TO_VICT );
      if( victim != ch )
         act( AT_ACTION, "$n looks at $N.", ch, NULL, victim, TO_NOTVICT );
      else
         act( AT_ACTION, "$n looks at $mself.", ch, NULL, victim, TO_NOTVICT );
   }

   if( victim->description[0] != '\0' )
   {
      if( victim->morph != NULL && victim->morph->morph != NULL )
         send_to_char( victim->morph->morph->description, ch );
      else
         send_to_char( victim->description, ch );
   }
   else
   {
      if( victim->morph != NULL && victim->morph->morph != NULL )
         send_to_char( victim->morph->morph->description, ch );
      else if( IS_NPC( victim ) )
         act( AT_PLAIN, "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
      else if( ch != victim )
         act( AT_PLAIN, "$E isn't much to look at...", ch, NULL, victim, TO_CHAR );
      else
         act( AT_PLAIN, "You're not much to look at...", ch, NULL, NULL, TO_CHAR );
   }

   show_race_line( ch, victim );
   show_condition( ch, victim );

   found = FALSE;
   for( iWear = 0; iWear < MAX_WEAR; iWear++ )
   {
      if( ( obj = get_eq_char( victim, iWear ) ) != NULL && can_see_obj( ch, obj ) )
      {
         if( !found )
         {
            send_to_char( "\r\n", ch );
            if( victim != ch )
               act( AT_PLAIN, "$N is using:", ch, NULL, victim, TO_CHAR );
            else
               act( AT_PLAIN, "You are using:", ch, NULL, NULL, TO_CHAR );
            found = TRUE;
         }
         if( ( !IS_NPC( victim ) ) && ( victim->race > 0 ) && ( victim->race < MAX_PC_RACE ) )
            send_to_char( race_table[victim->race]->where_name[iWear], ch );
         else
            send_to_char( where_name[iWear], ch );
         send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
         send_to_char( "\r\n", ch );
      }
   }

   /*
    * Crash fix here by Thoric
    */
   if( IS_NPC( ch ) || victim == ch )
      return;

   if( IS_IMMORTAL( ch ) )
   {
      if( IS_NPC( victim ) )
         ch_printf( ch, "\r\nMobile #%d '%s' ", victim->pIndexData->vnum, victim->name );
      else
         ch_printf( ch, "\r\n%s ", victim->name );
      ch_printf( ch, "is a level %d %s %s.\r\n",
                 victim->level,
                 IS_NPC( victim ) ? victim->race < MAX_NPC_RACE && victim->race >= 0 ?
                 npc_race[victim->race] : "unknown" : victim->race < MAX_PC_RACE &&
                 race_table[victim->race]->race_name &&
                 race_table[victim->race]->race_name[0] != '\0' ?
                 race_table[victim->race]->race_name : "unknown",
                 IS_NPC( victim ) ? victim->Class < MAX_NPC_CLASS && victim->Class >= 0 ?
                 npc_class[victim->Class] : "unknown" : victim->Class < MAX_PC_CLASS &&
                 class_table[victim->Class]->who_name &&
                 class_table[victim->Class]->who_name[0] != '\0' ? class_table[victim->Class]->who_name : "unknown" );
   }

   if( number_percent(  ) < LEARNED( ch, gsn_peek ) )
   {
      ch_printf( ch, "\r\nYou peek at %s inventory:\r\n", victim->sex == 1 ? "his" : victim->sex == 2 ? "her" : "its" );
      show_list_to_char( victim->first_carrying, ch, TRUE, TRUE );
      learn_from_success( ch, gsn_peek );
   }
   else if( ch->pcdata->learned[gsn_peek] > 0 )
      learn_from_failure( ch, gsn_peek );

   return;
}

void show_char_to_char( CHAR_DATA * list, CHAR_DATA * ch )
{
   CHAR_DATA *rch;

   for( rch = list; rch; rch = rch->next_in_room )
   {
      if( rch == ch )
         continue;

      if( can_see( ch, rch ) )
      {
         show_char_to_char_0( rch, ch );
      }
      else if( room_is_dark( ch->in_room ) && IS_AFFECTED( ch, AFF_INFRARED ) && !( !IS_NPC( rch ) && IS_IMMORTAL( rch ) ) )
      {
         set_char_color( AT_BLOOD, ch );
         send_to_char( "The red form of a living creature is here.\r\n", ch );
      }
   }
   return;
}

bool check_blind( CHAR_DATA * ch )
{
   if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_HOLYLIGHT ) )
      return TRUE;

   if( IS_AFFECTED( ch, AFF_TRUESIGHT ) )
      return TRUE;

   if( IS_AFFECTED( ch, AFF_BLIND ) )
   {
      send_to_char( "You can't see a thing!\r\n", ch );
      return FALSE;
   }

   return TRUE;
}

/*
 * Returns classical DIKU door direction based on text in arg	-Thoric
 */
int get_door( const char *arg )
{
   int door;

   if( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) )
      door = 0;
   else if( !str_cmp( arg, "e" ) || !str_cmp( arg, "east" ) )
      door = 1;
   else if( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) )
      door = 2;
   else if( !str_cmp( arg, "w" ) || !str_cmp( arg, "west" ) )
      door = 3;
   else if( !str_cmp( arg, "u" ) || !str_cmp( arg, "up" ) )
      door = 4;
   else if( !str_cmp( arg, "d" ) || !str_cmp( arg, "down" ) )
      door = 5;
   else if( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) )
      door = 6;
   else if( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) )
      door = 7;
   else if( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) )
      door = 8;
   else if( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) )
      door = 9;
   else
      door = -1;
   return door;
}

void print_compass( CHAR_DATA * ch )
{
   EXIT_DATA *pexit;
   int exit_info[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   static const char *const exit_colors[] = { "&w", "&Y", "&C", "&b", "&w", "&R" };
   for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
   {
      if( !pexit->to_room || IS_SET( pexit->exit_info, EX_HIDDEN ) ||
          ( IS_SET( pexit->exit_info, EX_SECRET ) && IS_SET( pexit->exit_info, EX_CLOSED ) ) )
         continue;
      if( IS_SET( pexit->exit_info, EX_WINDOW ) )
         exit_info[pexit->vdir] = 2;
      else if( IS_SET( pexit->exit_info, EX_SECRET ) )
         exit_info[pexit->vdir] = 3;
      else if( IS_SET( pexit->exit_info, EX_CLOSED ) )
         exit_info[pexit->vdir] = 4;
      else if( IS_SET( pexit->exit_info, EX_LOCKED ) )
         exit_info[pexit->vdir] = 5;
      else
         exit_info[pexit->vdir] = 1;
   }
   set_char_color( AT_RMNAME, ch );
   ch_printf_color( ch, "\r\n%-50s         %s%s    %s%s    %s%s\r\n",
                    ch->in_room->name,
                    exit_colors[exit_info[DIR_NORTHWEST]], exit_info[DIR_NORTHWEST] ? "NW" : "- ",
                    exit_colors[exit_info[DIR_NORTH]], exit_info[DIR_NORTH] ? "N" : "-",
                    exit_colors[exit_info[DIR_NORTHEAST]], exit_info[DIR_NORTHEAST] ? "NE" : " -" );
   if( IS_IMMORTAL( ch ) && xIS_SET( ch->act, PLR_ROOMVNUM ) )
      ch_printf_color( ch, "&w-<---- &YVnum: %6d &w----------------------------->-        ", ch->in_room->vnum );
   else
      send_to_char_color( "&w-<----------------------------------------------->-        ", ch );
   ch_printf_color( ch, "%s%s&w<-%s%s&w-&W(*)&w-%s%s&w->%s%s\r\n", exit_colors[exit_info[DIR_WEST]],
                    exit_info[DIR_WEST] ? "W" : "-", exit_colors[exit_info[DIR_UP]], exit_info[DIR_UP] ? "U" : "-",
                    exit_colors[exit_info[DIR_DOWN]], exit_info[DIR_DOWN] ? "D" : "-", exit_colors[exit_info[DIR_EAST]],
                    exit_info[DIR_EAST] ? "E" : "-" );
   ch_printf_color( ch, "                                                           %s%s    %s%s    %s%s\r\n\r\n",
                    exit_colors[exit_info[DIR_SOUTHWEST]], exit_info[DIR_SOUTHWEST] ? "SW" : "- ",
                    exit_colors[exit_info[DIR_SOUTH]], exit_info[DIR_SOUTH] ? "S" : "-",
                    exit_colors[exit_info[DIR_SOUTHEAST]], exit_info[DIR_SOUTHEAST] ? "SE" : " -" );
   return;
}

char *roomdesc( CHAR_DATA * ch )
{
   static char rdesc[MAX_STRING_LENGTH];

   rdesc[0] = '\0';

   if( !xIS_SET( ch->act, PLR_BRIEF ) )
   {
      if( ch->in_room->description && ch->in_room->description[0] != '\0' )
         mudstrlcat( rdesc, ch->in_room->description, MAX_STRING_LENGTH );
   }
   if( rdesc[0] == '\0' )
      mudstrlcpy( rdesc, "(Not set)", MAX_STRING_LENGTH );
   return rdesc;
}

void do_look( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH], arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
   EXIT_DATA *pexit;
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *original;
   const char *pdesc;
   short door;
   int number, cnt;

   if( !ch->desc )
      return;

   if( ch->position < POS_SLEEPING )
   {
      send_to_char( "You can't see anything but stars!\r\n", ch );
      return;
   }

   if( ch->position == POS_SLEEPING )
   {
      send_to_char( "You can't see anything, you're sleeping!\r\n", ch );
      return;
   }

   if( !check_blind( ch ) )
      return;

   if( !IS_NPC( ch )
       && !xIS_SET( ch->act, PLR_HOLYLIGHT ) && !IS_AFFECTED( ch, AFF_TRUESIGHT ) && room_is_dark( ch->in_room ) )
   {
      set_char_color( AT_DGREY, ch );
      send_to_char( "It is pitch black ... \r\n", ch );
      if( !*argument || !str_cmp( argument, "auto" ) )
         show_char_to_char( ch->in_room->first_person, ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   if( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
   {
      /*
       * 'look' or 'look auto' 
       */
      if( xIS_SET( ch->act, PLR_COMPASS ) )
         print_compass( ch );
      else
      {
         set_char_color( AT_RMNAME, ch );
         send_to_char( ch->in_room->name, ch );
         send_to_char( "\r\n", ch );
      }
      set_char_color( AT_RMDESC, ch );

      if( arg1[0] == '\0' || ( !IS_NPC( ch ) && !xIS_SET( ch->act, PLR_BRIEF ) ) )
      {
         if( xIS_SET( ch->act, PLR_AUTOMAP ) )
            draw_room_map( ch, roomdesc( ch ) );
         else
            send_to_char( roomdesc( ch ), ch );
      }

      /*
       * Added AUTOMAP check because it shows them next to the map now if its active 
       */
      if( !IS_NPC( ch ) && ( xIS_SET( ch->act, PLR_AUTOEXIT ) && !xIS_SET( ch->act, PLR_AUTOMAP ) ) )
         do_exits( ch, "auto" );

      show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE );
      show_char_to_char( ch->in_room->first_person, ch );
      return;
   }

   if( !str_cmp( arg1, "sky" ) || !str_cmp( arg1, "stars" ) )
   {
      if( !IS_OUTSIDE( ch ) || NO_WEATHER_SECT( ch->in_room->sector_type ) )
         send_to_char( "You can't see the sky from here.\r\n", ch );
      else
         look_sky( ch );

      return;
   }

   if( !str_cmp( arg1, "under" ) )
   {
      int count;

      /*
       * 'look under' 
       */
      if( arg2[0] == '\0' )
      {
         send_to_char( "Look beneath what?\r\n", ch );
         return;
      }

      if( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
      {
         send_to_char( "You do not see that here.\r\n", ch );
         return;
      }

      if( !CAN_WEAR( obj, ITEM_TAKE ) && ch->level < sysdata.level_getobjnotake )
      {
         send_to_char( "You can't seem to get a grip on it.\r\n", ch );
         return;
      }

      if( ch->carry_weight + obj->weight > can_carry_w( ch ) )
      {
         send_to_char( "It's too heavy for you to look under.\r\n", ch );
         return;
      }

      count = obj->count;
      obj->count = 1;
      act( AT_PLAIN, "You lift $p and look beneath it:", ch, obj, NULL, TO_CHAR );
      act( AT_PLAIN, "$n lifts $p and looks beneath it:", ch, obj, NULL, TO_ROOM );
      obj->count = count;
      if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
         show_list_to_char( obj->first_content, ch, TRUE, TRUE );
      else
         send_to_char( "Nothing.\r\n", ch );
      if( EXA_prog_trigger )
         oprog_examine_trigger( ch, obj );
      return;
   }

   if( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
   {
      int count;

      /*
       * 'look in' 
       */
      if( arg2[0] == '\0' )
      {
         send_to_char( "Look in what?\r\n", ch );
         return;
      }

      if( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
      {
         send_to_char( "You do not see that here.\r\n", ch );
         return;
      }

      switch ( obj->item_type )
      {
         default:
            send_to_char( "That is not a container.\r\n", ch );
            break;

         case ITEM_DRINK_CON:
            if( obj->value[1] <= 0 )
            {
               send_to_char( "It is empty.\r\n", ch );
               if( EXA_prog_trigger )
                  oprog_examine_trigger( ch, obj );
               break;
            }

            {
               LIQ_TABLE *liq = get_liq_vnum( obj->value[2] );
               ch_printf( ch, "It's %s full of a %s liquid.\r\n",
                          obj->value[1] < obj->value[0] / 4
                          ? "less than" : obj->value[1] < 3 * obj->value[0] / 4 ? "about" : "more than", liq->color );
            }

            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            break;

         case ITEM_PORTAL:
            for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
            {
               if( pexit->vdir == DIR_PORTAL && IS_SET( pexit->exit_info, EX_PORTAL ) )
               {
                  if( room_is_private( pexit->to_room ) && get_trust( ch ) < sysdata.level_override_private )
                  {
                     set_char_color( AT_WHITE, ch );
                     send_to_char( "That room is private buster!\r\n", ch );
                     return;
                  }
                  original = ch->in_room;
                  char_from_room( ch );
                  char_to_room( ch, pexit->to_room );
                  do_look( ch, "auto" );
                  char_from_room( ch );
                  char_to_room( ch, original );
                  return;
               }
            }
            send_to_char( "You see swirling chaos...\r\n", ch );
            break;

         case ITEM_CONTAINER:
         case ITEM_QUIVER:
         case ITEM_CORPSE_NPC:
         case ITEM_CORPSE_PC:
            if( IS_SET( obj->value[1], CONT_CLOSED ) )
            {
               send_to_char( "It is closed.\r\n", ch );
               break;
            }

         case ITEM_KEYRING:
            count = obj->count;
            obj->count = 1;
            if( obj->item_type == ITEM_CONTAINER )
               act( AT_PLAIN, "$p contains:", ch, obj, NULL, TO_CHAR );
            else
               act( AT_PLAIN, "$p holds:", ch, obj, NULL, TO_CHAR );
            obj->count = count;
            show_list_to_char( obj->first_content, ch, TRUE, TRUE );
            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            break;
      }
      return;
   }

   if( ( pdesc = get_extra_descr( arg1, ch->in_room->first_extradesc ) ) != NULL )
   {
      send_to_char_color( pdesc, ch );
      return;
   }

   door = get_door( arg1 );
   if( ( pexit = find_door( ch, arg1, TRUE ) ) != NULL )
   {
      if( IS_SET( pexit->exit_info, EX_CLOSED ) && !IS_SET( pexit->exit_info, EX_WINDOW ) )
      {
         if( ( IS_SET( pexit->exit_info, EX_SECRET ) || IS_SET( pexit->exit_info, EX_DIG ) ) && door != -1 )
            send_to_char( "Nothing special there.\r\n", ch );
         else
         {
            if( pexit->keyword[strlen( pexit->keyword ) - 1] == 's'
                || ( pexit->keyword[strlen( pexit->keyword ) - 1] == '\''
                     && pexit->keyword[strlen( pexit->keyword ) - 2] == 's' ) )
               act( AT_PLAIN, "The $d are closed.", ch, NULL, pexit->keyword, TO_CHAR );
            else
               act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
         }
         return;
      }
      if( IS_SET( pexit->exit_info, EX_BASHED ) )
      {
         if( pexit->keyword[strlen( pexit->keyword ) - 1] == 's'
             || ( pexit->keyword[strlen( pexit->keyword ) - 1] == '\''
                  && pexit->keyword[strlen( pexit->keyword ) - 2] == 's' ) )
            act( AT_PLAIN, "The $d have been bashed from their hinges.", ch, NULL, pexit->keyword, TO_CHAR );
         else
            act( AT_PLAIN, "The $d has been bashed from its hinges.", ch, NULL, pexit->keyword, TO_CHAR );
      }

      if( pexit->description && pexit->description[0] != '\0' )
         send_to_char( pexit->description, ch );
      else
         send_to_char( "Nothing special there.\r\n", ch );

      /*
       * Ability to look into the next room        -Thoric
       */
      if( pexit->to_room
          && ( IS_AFFECTED( ch, AFF_SCRYING )
               || ch->Class == CLASS_THIEF || IS_SET( pexit->exit_info, EX_xLOOK ) || get_trust( ch ) >= LEVEL_IMMORTAL ) )
      {
         if( !IS_SET( pexit->exit_info, EX_xLOOK ) && get_trust( ch ) < LEVEL_IMMORTAL )
         {
            set_char_color( AT_MAGIC, ch );
            send_to_char( "You attempt to scry...\r\n", ch );
            /*
             * Change by Narn, Sept 96 to allow characters who don't have the
             * scry spell to benefit from objects that are affected by scry.
             */
            if( !IS_NPC( ch ) )
            {
               int percent = LEARNED( ch, skill_lookup( "scry" ) );
               if( !percent )
               {
                  if( ch->Class == CLASS_THIEF )
                     percent = 95;
                  else
                     percent = 55;  /* 95 was too good -Thoric */
               }

               if( number_percent(  ) > percent )
               {
                  send_to_char( "You fail.\r\n", ch );
                  return;
               }
            }
         }
         if( room_is_private( pexit->to_room ) && get_trust( ch ) < sysdata.level_override_private )
         {
            set_char_color( AT_WHITE, ch );
            send_to_char( "That room is private buster!\r\n", ch );
            return;
         }
         original = ch->in_room;
         char_from_room( ch );
         char_to_room( ch, pexit->to_room );
         do_look( ch, "auto" );
         char_from_room( ch );
         char_to_room( ch, original );
      }
      return;
   }
   else if( door != -1 )
   {
      send_to_char( "Nothing special there.\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) != NULL )
   {
      show_char_to_char_1( victim, ch );
      return;
   }

   /*
    * finally fixed the annoying look 2.obj desc bug -Thoric 
    */
   number = number_argument( arg1, arg );
   for( cnt = 0, obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( can_see_obj( ch, obj ) )
      {
         if( ( pdesc = get_extra_descr( arg, obj->first_extradesc ) ) != NULL )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            send_to_char_color( pdesc, ch );
            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            return;
         }

         if( ( pdesc = get_extra_descr( arg, obj->pIndexData->first_extradesc ) ) != NULL )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            send_to_char_color( pdesc, ch );
            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            return;
         }
         if( nifty_is_name_prefix( arg, obj->name ) )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
            if( !pdesc )
               pdesc = get_extra_descr( obj->name, obj->first_extradesc );
            if( !pdesc )
               send_to_char_color( "You see nothing special.\r\n", ch );
            else
               send_to_char_color( pdesc, ch );
            if( obj->item_type == ITEM_PUDDLE )
            {
               LIQ_TABLE *liq = get_liq_vnum( obj->value[2] );

               ch_printf( ch, "It's a puddle of %s liquid.\r\n", ( liq == NULL ? "clear" : liq->color ) );
            }
            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            return;
         }
      }
   }

   for( obj = ch->in_room->last_content; obj; obj = obj->prev_content )
   {
      if( can_see_obj( ch, obj ) )
      {
         if( ( pdesc = get_extra_descr( arg, obj->first_extradesc ) ) != NULL )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            send_to_char_color( pdesc, ch );
            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            return;
         }

         if( ( pdesc = get_extra_descr( arg, obj->pIndexData->first_extradesc ) ) != NULL )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            send_to_char_color( pdesc, ch );
            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            return;
         }
         if( nifty_is_name_prefix( arg, obj->name ) )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
            if( !pdesc )
               pdesc = get_extra_descr( obj->name, obj->first_extradesc );
            if( !pdesc )
               send_to_char( "You see nothing special.\r\n", ch );
            else
               send_to_char_color( pdesc, ch );
            if( obj->item_type == ITEM_PUDDLE )
            {
               LIQ_TABLE *liq = get_liq_vnum( obj->value[2] );

               ch_printf( ch, "It's a puddle of %s liquid.\r\n", ( liq == NULL ? "clear" : liq->color ) );
            }
            if( EXA_prog_trigger )
               oprog_examine_trigger( ch, obj );
            return;
         }
      }
   }

   send_to_char( "You do not see that here.\r\n", ch );
}

void show_race_line( CHAR_DATA * ch, CHAR_DATA * victim )
{
   int feet, inches;

   if( !IS_NPC( victim ) && ( victim != ch ) )
   {
      feet = victim->height / 12;
      inches = victim->height % 12;
      ch_printf( ch, "%s is %d'%d\" and weighs %d pounds.\r\n", PERS( victim, ch ), feet, inches, victim->weight );
      return;
   }
   if( !IS_NPC( victim ) && ( victim == ch ) )
   {
      feet = victim->height / 12;
      inches = victim->height % 12;
      ch_printf( ch, "You are %d'%d\" and weigh %d pounds.\r\n", feet, inches, victim->weight );
      return;
   }
}

void show_condition( CHAR_DATA * ch, CHAR_DATA * victim )
{
   char buf[MAX_STRING_LENGTH];
   int percent;

   if( victim->max_hit > 0 )
      percent = ( int )( ( 100.0 * ( double )( victim->hit ) ) / ( double )( victim->max_hit ) );
   else
      percent = -1;

   if( victim != ch )
   {
      mudstrlcpy( buf, PERS( victim, ch ), MAX_STRING_LENGTH );
      if( percent >= 100 )
         mudstrlcat( buf, " is in perfect health.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 90 )
         mudstrlcat( buf, " is slightly scratched.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 80 )
         mudstrlcat( buf, " has a few bruises.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 70 )
         mudstrlcat( buf, " has some cuts.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 60 )
         mudstrlcat( buf, " has several wounds.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 50 )
         mudstrlcat( buf, " has many nasty wounds.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 40 )
         mudstrlcat( buf, " is bleeding freely.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 30 )
         mudstrlcat( buf, " is covered in blood.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 20 )
         mudstrlcat( buf, " is leaking guts.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 10 )
         mudstrlcat( buf, " is almost dead.\r\n", MAX_STRING_LENGTH );
      else
         mudstrlcat( buf, " is DYING.\r\n", MAX_STRING_LENGTH );
   }
   else
   {
      mudstrlcpy( buf, "You", MAX_STRING_LENGTH );
      if( percent >= 100 )
         mudstrlcat( buf, " are in perfect health.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 90 )
         mudstrlcat( buf, " are slightly scratched.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 80 )
         mudstrlcat( buf, " have a few bruises.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 70 )
         mudstrlcat( buf, " have some cuts.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 60 )
         mudstrlcat( buf, " have several wounds.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 50 )
         mudstrlcat( buf, " have many nasty wounds.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 40 )
         mudstrlcat( buf, " are bleeding freely.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 30 )
         mudstrlcat( buf, " are covered in blood.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 20 )
         mudstrlcat( buf, " are leaking guts.\r\n", MAX_STRING_LENGTH );
      else if( percent >= 10 )
         mudstrlcat( buf, " are almost dead.\r\n", MAX_STRING_LENGTH );
      else
         mudstrlcat( buf, " are DYING.\r\n", MAX_STRING_LENGTH );
   }

   buf[0] = UPPER( buf[0] );
   send_to_char( buf, ch );
}

/* A much simpler version of look, this function will show you only
the condition of a mob or pc, or if used without an argument, the
same you would see if you enter the room and have config +brief.
-- Narn, winter '96
*/
void do_glance( CHAR_DATA * ch, const char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   bool brief;

   if( !ch->desc )
      return;

   if( ch->position < POS_SLEEPING )
   {
      send_to_char( "You can't see anything but stars!\r\n", ch );
      return;
   }

   if( ch->position == POS_SLEEPING )
   {
      send_to_char( "You can't see anything, you're sleeping!\r\n", ch );
      return;
   }

   if( !check_blind( ch ) )
      return;

   set_char_color( AT_ACTION, ch );
   argument = one_argument( argument, arg1 );

   if( arg1[0] == '\0' )
   {
      if( xIS_SET( ch->act, PLR_BRIEF ) )
         brief = TRUE;
      else
         brief = FALSE;
      xSET_BIT( ch->act, PLR_BRIEF );
      do_look( ch, "auto" );
      if( !brief )
         xREMOVE_BIT( ch->act, PLR_BRIEF );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They're not here.\r\n", ch );
      return;
   }
   else
   {
      if( can_see( victim, ch ) )
      {
         act( AT_ACTION, "$n glances at you.", ch, NULL, victim, TO_VICT );
         act( AT_ACTION, "$n glances at $N.", ch, NULL, victim, TO_NOTVICT );
      }
      if( IS_IMMORTAL( ch ) && victim != ch )
      {
         if( IS_NPC( victim ) )
            ch_printf( ch, "Mobile #%d '%s' ", victim->pIndexData->vnum, victim->name );
         else
            ch_printf( ch, "%s ", victim->name );
         ch_printf( ch, "is a level %d %s %s.\r\n",
                    victim->level,
                    IS_NPC( victim ) ? victim->race < MAX_NPC_RACE && victim->race >= 0 ?
                    npc_race[victim->race] : "unknown" : victim->race < MAX_PC_RACE &&
                    race_table[victim->race]->race_name &&
                    race_table[victim->race]->race_name[0] != '\0' ?
                    race_table[victim->race]->race_name : "unknown",
                    IS_NPC( victim ) ? victim->Class < MAX_NPC_CLASS && victim->Class >= 0 ?
                    npc_class[victim->Class] : "unknown" : victim->Class < MAX_PC_CLASS &&
                    class_table[victim->Class]->who_name &&
                    class_table[victim->Class]->who_name[0] != '\0' ? class_table[victim->Class]->who_name : "unknown" );
      }
      show_condition( ch, victim );
      return;
   }
   return;
}

void do_examine( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   BOARD_DATA *board;
   short dam;

   if( !argument )
   {
      bug( "%s", "do_examine: null argument." );
      return;
   }

   if( !ch )
   {
      bug( "%s", "do_examine: null ch." );
      return;
   }

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Examine what?\r\n", ch );
      return;
   }

   EXA_prog_trigger = FALSE;
   do_look( ch, arg );
   EXA_prog_trigger = TRUE;

   /*
    * Support for looking at boards, checking equipment conditions,
    * and support for trigger positions by Thoric
    */
   if( ( obj = get_obj_here( ch, arg ) ) != NULL )
   {
      if( ( board = get_board( obj ) ) != NULL )
      {
         if( board->num_posts )
            ch_printf( ch, "There are about %d notes posted here.  Type 'note list' to list them.\r\n", board->num_posts );
         else
            send_to_char( "There aren't any notes posted here.\r\n", ch );
      }

      switch ( obj->item_type )
      {
         default:
            break;

         case ITEM_ARMOR:
            if( obj->value[1] == 0 )
               obj->value[1] = obj->value[0];
            if( obj->value[1] == 0 )
               obj->value[1] = 1;
            dam = ( short )( ( obj->value[0] * 10 ) / obj->value[1] );
            mudstrlcpy( buf, "As you look more closely, you notice that it is ", MAX_STRING_LENGTH );
            if( dam >= 10 )
               mudstrlcat( buf, "in superb condition.", MAX_STRING_LENGTH );
            else if( dam == 9 )
               mudstrlcat( buf, "in very good condition.", MAX_STRING_LENGTH );
            else if( dam == 8 )
               mudstrlcat( buf, "in good shape.", MAX_STRING_LENGTH );
            else if( dam == 7 )
               mudstrlcat( buf, "showing a bit of wear.", MAX_STRING_LENGTH );
            else if( dam == 6 )
               mudstrlcat( buf, "a little run down.", MAX_STRING_LENGTH );
            else if( dam == 5 )
               mudstrlcat( buf, "in need of repair.", MAX_STRING_LENGTH );
            else if( dam == 4 )
               mudstrlcat( buf, "in great need of repair.", MAX_STRING_LENGTH );
            else if( dam == 3 )
               mudstrlcat( buf, "in dire need of repair.", MAX_STRING_LENGTH );
            else if( dam == 2 )
               mudstrlcat( buf, "very badly worn.", MAX_STRING_LENGTH );
            else if( dam == 1 )
               mudstrlcat( buf, "practically worthless.", MAX_STRING_LENGTH );
            else if( dam <= 0 )
               mudstrlcat( buf, "broken.", MAX_STRING_LENGTH );
            mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
            send_to_char( buf, ch );
            break;

         case ITEM_JOURNAL:
         {
            short count = 0;
            EXTRA_DESCR_DATA *ed;

            for( ed = obj->first_extradesc; ed; ed = ed->next )
               count++;

            ch_printf( ch, "%s has %d %s written in out of a possible %d.\r\n",
                       obj->short_descr, count, count == 1 ? "page" : "pages", obj->value[0] );

            break;
         }

         case ITEM_WEAPON:
            dam = INIT_WEAPON_CONDITION - obj->value[0];
            mudstrlcpy( buf, "As you look more closely, you notice that it is ", MAX_STRING_LENGTH );
            if( dam <= 0 )
               mudstrlcat( buf, "in superb condition.", MAX_STRING_LENGTH );
            else if( dam == 1 )
               mudstrlcat( buf, "in excellent condition.", MAX_STRING_LENGTH );
            else if( dam == 2 )
               mudstrlcat( buf, "in very good condition.", MAX_STRING_LENGTH );
            else if( dam == 3 )
               mudstrlcat( buf, "in good shape.", MAX_STRING_LENGTH );
            else if( dam == 4 )
               mudstrlcat( buf, "showing a bit of wear.", MAX_STRING_LENGTH );
            else if( dam == 5 )
               mudstrlcat( buf, "a little run down.", MAX_STRING_LENGTH );
            else if( dam == 6 )
               mudstrlcat( buf, "in need of repair.", MAX_STRING_LENGTH );
            else if( dam == 7 )
               mudstrlcat( buf, "in great need of repair.", MAX_STRING_LENGTH );
            else if( dam == 8 )
               mudstrlcat( buf, "in dire need of repair.", MAX_STRING_LENGTH );
            else if( dam == 9 )
               mudstrlcat( buf, "very badly worn.", MAX_STRING_LENGTH );
            else if( dam == 10 )
               mudstrlcat( buf, "practically worthless.", MAX_STRING_LENGTH );
            else if( dam == 11 )
               mudstrlcat( buf, "almost broken.", MAX_STRING_LENGTH );
            else if( dam == 12 )
               mudstrlcat( buf, "broken.", MAX_STRING_LENGTH );
            mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
            send_to_char( buf, ch );
            break;

         case ITEM_COOK:
            mudstrlcpy( buf, "As you examine it carefully you notice that it ", MAX_STRING_LENGTH );
            dam = obj->value[2];
            if( dam >= 3 )
               mudstrlcat( buf, "is burned to a crisp.", MAX_STRING_LENGTH );
            else if( dam == 2 )
               mudstrlcat( buf, "is a little over cooked.", MAX_STRING_LENGTH );
            else if( dam == 1 )
               mudstrlcat( buf, "is perfectly roasted.", MAX_STRING_LENGTH );
            else
               mudstrlcat( buf, "is raw.", MAX_STRING_LENGTH );
            mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
            send_to_char( buf, ch );

         case ITEM_FOOD:
            if( obj->timer > 0 && obj->value[1] > 0 )
               dam = ( obj->timer * 10 ) / obj->value[1];
            else
               dam = 10;
            if( obj->item_type == ITEM_FOOD )
               mudstrlcpy( buf, "As you examine it carefully you notice that it ", MAX_STRING_LENGTH );
            else
               mudstrlcpy( buf, "Also it ", MAX_STRING_LENGTH );
            if( dam >= 10 )
               mudstrlcat( buf, "is fresh.", MAX_STRING_LENGTH );
            else if( dam == 9 )
               mudstrlcat( buf, "is nearly fresh.", MAX_STRING_LENGTH );
            else if( dam == 8 )
               mudstrlcat( buf, "is perfectly fine.", MAX_STRING_LENGTH );
            else if( dam == 7 )
               mudstrlcat( buf, "looks good.", MAX_STRING_LENGTH );
            else if( dam == 6 )
               mudstrlcat( buf, "looks ok.", MAX_STRING_LENGTH );
            else if( dam == 5 )
               mudstrlcat( buf, "is a little stale.", MAX_STRING_LENGTH );
            else if( dam == 4 )
               mudstrlcat( buf, "is a bit stale.", MAX_STRING_LENGTH );
            else if( dam == 3 )
               mudstrlcat( buf, "smells slightly off.", MAX_STRING_LENGTH );
            else if( dam == 2 )
               mudstrlcat( buf, "smells quite rank.", MAX_STRING_LENGTH );
            else if( dam == 1 )
               mudstrlcat( buf, "smells revolting!", MAX_STRING_LENGTH );
            else if( dam <= 0 )
               mudstrlcat( buf, "is crawling with maggots!", MAX_STRING_LENGTH );
            mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
            send_to_char( buf, ch );
            break;

         case ITEM_SWITCH:
         case ITEM_LEVER:
         case ITEM_PULLCHAIN:
            if( IS_SET( obj->value[0], TRIG_UP ) )
               send_to_char( "You notice that it is in the up position.\r\n", ch );
            else
               send_to_char( "You notice that it is in the down position.\r\n", ch );
            break;

         case ITEM_BUTTON:
            if( IS_SET( obj->value[0], TRIG_UP ) )
               send_to_char( "You notice that it is depressed.\r\n", ch );
            else
               send_to_char( "You notice that it is not depressed.\r\n", ch );
            break;

         case ITEM_CORPSE_PC:
         case ITEM_CORPSE_NPC:
         {
            short timerfrac = obj->timer;
            if( obj->item_type == ITEM_CORPSE_PC )
               timerfrac = ( int )obj->timer / 8 + 1;

            switch ( timerfrac )
            {
               default:
                  send_to_char( "This corpse has recently been slain.\r\n", ch );
                  break;
               case 4:
                  send_to_char( "This corpse was slain a little while ago.\r\n", ch );
                  break;
               case 3:
                  send_to_char( "A foul smell rises from the corpse, and it is covered in flies.\r\n", ch );
                  break;
               case 2:
                  send_to_char( "A writhing mass of maggots and decay, you can barely go near this corpse.\r\n", ch );
                  break;
               case 1:
               case 0:
                  send_to_char( "Little more than bones, there isn't much left of this corpse.\r\n", ch );
                  break;
            }
         }

         case ITEM_CONTAINER:
            if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
               break;
         case ITEM_DRINK_CON:
         case ITEM_QUIVER:
            send_to_char( "When you look inside, you see:\r\n", ch );
         case ITEM_KEYRING:
            EXA_prog_trigger = FALSE;
            snprintf( buf, MAX_STRING_LENGTH, "in %s", arg );
            do_look( ch, buf );
            EXA_prog_trigger = TRUE;
            break;
      }

      if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
      {
         EXA_prog_trigger = FALSE;
         snprintf( buf, MAX_STRING_LENGTH, "under %s", arg );
         do_look( ch, buf );
         EXA_prog_trigger = TRUE;
      }
      oprog_examine_trigger( ch, obj );
      if( char_died( ch ) || obj_extracted( obj ) )
         return;

      check_for_trap( ch, obj, TRAP_EXAMINE );
   }
   return;
}

void do_exits( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   EXIT_DATA *pexit;
   bool found;
   bool fAuto;

   buf[0] = '\0';
   fAuto = !str_cmp( argument, "auto" );

   if( !check_blind( ch ) )
      return;

   if( !IS_NPC( ch )
       && !xIS_SET( ch->act, PLR_HOLYLIGHT )
       && !IS_AFFECTED( ch, AFF_TRUESIGHT ) && !IS_AFFECTED( ch, AFF_INFRARED ) && room_is_dark( ch->in_room ) )
   {
      set_char_color( AT_DGREY, ch );
      send_to_char( "It is pitch black ... \r\n", ch );
      return;
   }

   set_char_color( AT_EXITS, ch );
   mudstrlcpy( buf, fAuto ? "Exits:" : "Obvious exits:\r\n", MAX_STRING_LENGTH );

   found = FALSE;
   for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
   {
      if( pexit->to_room && ( !IS_SET( pexit->exit_info, EX_WINDOW ) || IS_SET( pexit->exit_info, EX_ISDOOR ) )
         && !IS_SET( pexit->exit_info, EX_SECRET ) && !IS_SET( pexit->exit_info, EX_HIDDEN ) && !IS_SET( pexit->exit_info, EX_DIG ) )
      {
         found = TRUE;
         if( fAuto )
         {
            if( IS_SET( pexit->exit_info, EX_CLOSED ) )
            {
               if( pexit->keyword && ( !str_cmp( "door", pexit->keyword ) || !str_cmp( "gate", pexit->keyword ) || pexit->keyword[0] == '\0' ) )
                  snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "[%s]", dir_name[pexit->vdir] );
            }
            else
            {
               mudstrlcat( buf, dir_name[pexit->vdir], MAX_STRING_LENGTH );
               mudstrlcat( buf, " ", MAX_STRING_LENGTH );
            }
         }
         else
         {
            snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH - strlen( buf ) ), "%-5s - %s\r\n",
               capitalize( dir_name[pexit->vdir] ), room_is_dark( pexit->to_room ) ? "Too dark to tell" : pexit->to_room->name );
         }
      }
   }

   if( !found )
      mudstrlcat( buf, fAuto ? "none.\r\n" : "None.\r\n", MAX_STRING_LENGTH );
   else if( fAuto )
      mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
   send_to_char( buf, ch );
   return;
}

/*
 * Moved into a separate function so it can be used for other things
 * ie: online help editing				-Thoric
 */
HELP_DATA *get_help( CHAR_DATA * ch, const char *argument )
{
   char argall[MAX_INPUT_LENGTH];
   char argone[MAX_INPUT_LENGTH];
   char argnew[MAX_INPUT_LENGTH];
   HELP_DATA *pHelp;
   int lev;

   if( argument[0] == '\0' )
      argument = "summary";

   if( isdigit( argument[0] ) && !is_number( argument ) )
   {
      lev = number_argument( argument, argnew );
      argument = argnew;
   }
   else
      lev = -2;
   /*
    * Tricky argument handling so 'help a b' doesn't match a.
    */
   argall[0] = '\0';
   while( argument[0] != '\0' )
   {
      argument = one_argument( argument, argone );
      if( argall[0] != '\0' )
         mudstrlcat( argall, " ", MAX_INPUT_LENGTH );
      mudstrlcat( argall, argone, MAX_INPUT_LENGTH );
   }

   for( pHelp = first_help; pHelp; pHelp = pHelp->next )
   {
      if( pHelp->level > get_trust( ch ) )
         continue;
      if( lev != -2 && pHelp->level != lev )
         continue;

      if( is_name( argall, pHelp->keyword ) )
         return pHelp;
   }

   return NULL;
}

/*
 * LAWS command
 */
void do_laws( CHAR_DATA* ch, const char* argument)
{
   char buf[1024];

   if( argument == NULL )
      do_help( ch, "laws" );
   else
   {
      snprintf( buf, 1024, "law %s", argument );
      do_help( ch, buf );
   }
}

/*
 * Now this is cleaner
 */
/* Updated do_help command provided by Remcon of The Lands of Pabulum 03/20/2004 */
void do_help( CHAR_DATA* ch, const char* argument)
{
   HELP_DATA *pHelp;
   const char *keyword;
   char arg[MAX_INPUT_LENGTH];
   char oneword[MAX_STRING_LENGTH], lastmatch[MAX_STRING_LENGTH];
   short matched = 0, checked = 0, totalmatched = 0, found = 0;
   bool uselevel = FALSE;
   int value = 0;

   set_pager_color( AT_NOTE, ch );

   if( !argument || argument[0] == '\0' )
      argument = "summary";
   if( !( pHelp = get_help( ch, argument ) ) )
   {
      pager_printf( ch, "No help on \'%s\' found.\r\n", argument );
      /*
       * Get an arg incase they do a number seperate 
       */
      one_argument( argument, arg );
      /*
       * See if arg is a number if so update argument 
       */
      if( is_number( arg ) )
      {
         argument = one_argument( argument, arg );
         if( argument && argument[0] != '\0' )
         {
            value = atoi( arg );
            uselevel = TRUE;
         }
         else  /* If no more argument put arg as argument */
            argument = arg;
      }
      if( value > 0 )
         pager_printf( ch, "Checking for suggested helps that are level %d.\r\n", value );
      send_to_pager( "Suggested Help Files:\r\n", ch );
      strncpy( lastmatch, " ", MAX_STRING_LENGTH );
      for( pHelp = first_help; pHelp; pHelp = pHelp->next )
      {
         matched = 0;
         if( !pHelp || !pHelp->keyword || pHelp->keyword[0] == '\0' || pHelp->level > get_trust( ch ) )
            continue;
         /*
          * Check arg if its avaliable 
          */
         if( uselevel && pHelp->level != value )
            continue;
         keyword = pHelp->keyword;
         while( keyword && keyword[0] != '\0' )
         {
            matched = 0;   /* Set to 0 for each time we check lol */
            keyword = one_argument( keyword, oneword );
            /*
             * Lets check only up to 10 spots
             */
            for( checked = 0; checked <= 10; checked++ )
            {
               if( !oneword[checked] || !argument[checked] )
                  break;
               if( LOWER( oneword[checked] ) == LOWER( argument[checked] ) )
                  matched++;
            }
            if( ( matched > 1 && matched > ( checked / 2 ) ) || ( matched > 0 && checked < 2 ) )
            {
               pager_printf( ch, " %-20s ", oneword );
               if( ++found % 4 == 0 )
               {
                  found = 0;
                  send_to_pager( "\r\n", ch );
               }
               strncpy( lastmatch, oneword, MAX_STRING_LENGTH );
               totalmatched++;
               break;
            }
         }
      }
      if( found != 0 )
         send_to_pager( "\r\n", ch );
      if( totalmatched == 0 )
      {
         send_to_pager( "No suggested help files.\r\n", ch );
         return;
      }
      if( totalmatched == 1 && lastmatch[0] != '\0' && str_cmp( lastmatch, argument ) )
      {
         send_to_pager( "Opening only suggested helpfile.\r\n", ch );
         do_help( ch, lastmatch );
         return;
      }
      return;
   }
   /*
    * Make newbies do a help start. --Shaddai
    */
   if( !IS_NPC( ch ) && !str_cmp( argument, "start" ) )
      SET_BIT( ch->pcdata->flags, PCFLAG_HELPSTART );

   if( IS_IMMORTAL( ch ) )
      pager_printf( ch, "Help level: %d\r\n", pHelp->level );

   set_pager_color( AT_NOTE, ch );

   /*
    * Strip leading '.' to allow initial blanks.
    */
   if( pHelp->text[0] == '.' )
      send_to_pager( pHelp->text + 1, ch );
   else
      send_to_pager( pHelp->text, ch );
   return;
}

extern const char *help_greeting;   /* so we can edit the greeting online */

void free_help( HELP_DATA * pHelp )
{
   UNLINK( pHelp, first_help, last_help, next, prev );
   STRFREE( pHelp->text );
   STRFREE( pHelp->keyword );
   DISPOSE( pHelp );
   return;
}

void free_helps( void )
{
   HELP_DATA *pHelp, *pHelp_next;

   for( pHelp = first_help; pHelp; pHelp = pHelp_next )
   {
      pHelp_next = pHelp->next;
      free_help( pHelp );
   }
   return;
}

/*
 * Help editor							-Thoric
 */
void do_hedit( CHAR_DATA* ch, const char* argument)
{
   HELP_DATA *pHelp;

   if( !ch->desc )
   {
      send_to_char( "You have no descriptor.\r\n", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_HELP_EDIT:
         if( ( pHelp = ( HELP_DATA * ) ch->dest_buf ) == NULL )
         {
            bug( "%s", "hedit: sub_help_edit: NULL ch->dest_buf" );
            stop_editing( ch );
            return;
         }
         if( help_greeting == pHelp->text )
            help_greeting = NULL;
         STRFREE( pHelp->text );
         pHelp->text = copy_buffer( ch );
         if( !help_greeting )
            help_greeting = pHelp->text;
         stop_editing( ch );
         return;
   }
   if( ( pHelp = get_help( ch, argument ) ) == NULL ) /* new help */
   {
      HELP_DATA *tHelp;
      char argnew[MAX_INPUT_LENGTH];
      int lev;
      bool new_help = TRUE;

      for( tHelp = first_help; tHelp; tHelp = tHelp->next )
         if( !str_cmp( argument, tHelp->keyword ) )
         {
            pHelp = tHelp;
            new_help = FALSE;
            break;
         }
      if( new_help )
      {
         if( isdigit( argument[0] ) )
         {
            lev = number_argument( argument, argnew );
            argument = argnew;
         }
         else
            lev = get_trust( ch );
         CREATE( pHelp, HELP_DATA, 1 );
         pHelp->keyword = STRALLOC( strupper( argument ) );
         pHelp->text = STRALLOC( "" );
         pHelp->level = lev;
         add_help( pHelp );
      }
   }

   ch->substate = SUB_HELP_EDIT;
   ch->dest_buf = pHelp;
   start_editing( ch, pHelp->text );
}

/*
 * Stupid leading space muncher fix				-Thoric
 */
const char *help_fix( const char *text )
{
   char *fixed;

   if( !text )
      return "";

   fixed = strip_cr( text );
   if( fixed[0] == ' ' )
      fixed[0] = '.';
   return fixed;
}

void do_hset( CHAR_DATA* ch, const char* argument)
{
   HELP_DATA *pHelp;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   argument = smash_tilde_copy( argument );

   argument = one_argument( argument, arg1 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: hset <field> [value] [help page]\r\n", ch );
      send_to_char( "\r\n", ch );
      send_to_char( "Field being one of:\r\n", ch );
      send_to_char( "  level keyword remove save\r\n", ch );
      return;
   }

   if( !str_cmp( arg1, "save" ) )
   {
      FILE *fpout;

      log_string_plus( "Saving help.are...", LOG_NORMAL, LEVEL_GREATER );

      rename( "help.are", "help.are.bak" );
      if( ( fpout = fopen( "help.are", "w" ) ) == NULL )
      {
         bug( "%s: cant open help.are", __func__ );
         perror( "help.are" );
         return;
      }

      fprintf( fpout, "#HELPS\n\n" );
      for( pHelp = first_help; pHelp; pHelp = pHelp->next )
         fprintf( fpout, "%d %s~\n%s~\n\n", pHelp->level, pHelp->keyword, help_fix( pHelp->text ) );

      fprintf( fpout, "0 $~\n\n\n#$\n" );
      fclose( fpout );
      fpout = NULL;
      send_to_char( "Saved.\r\n", ch );
      return;
   }

   if( str_cmp( arg1, "remove" ) )
      argument = one_argument( argument, arg2 );

   if( !( pHelp = get_help( ch, argument ) ) )
   {
      send_to_char( "Cannot find help on that subject.\r\n", ch );
      return;
   }

   if( !str_cmp( arg1, "remove" ) )
   {
      UNLINK( pHelp, first_help, last_help, next, prev );
      STRFREE( pHelp->text );
      STRFREE( pHelp->keyword );
      DISPOSE( pHelp );
      send_to_char( "Removed.\r\n", ch );
      return;
   }

   if( !str_cmp( arg1, "level" ) )
   {
      int lev;

      if( !is_number( arg2 ) )
      {
         send_to_char( "Level field must be numeric.\r\n", ch );
         return;
      }

      lev = atoi( arg2 );
      if( lev < -1 || lev > get_trust( ch ) )
      {
         send_to_char( "You can't set the level to that.\r\n", ch );
         return;
      }
      pHelp->level = lev;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg1, "keyword" ) )
   {
      STRFREE( pHelp->keyword );
      pHelp->keyword = STRALLOC( strupper( arg2 ) );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   do_hset( ch, "" );
}

void do_hl( CHAR_DATA* ch, const char* argument)
{
   send_to_char( "If you want to use HLIST, spell it out.\r\n", ch );
   return;
}

/*
 * Show help topics in a level range				-Thoric
 * Idea suggested by Gorog
 * prefix keyword indexing added by Fireblade
 */
void do_hlist( CHAR_DATA* ch, const char* argument)
{
   int min, max, minlimit, maxlimit, cnt;
   char arg[MAX_INPUT_LENGTH];
   HELP_DATA *help;
   bool minfound, maxfound;
   const char *idx;

   maxlimit = get_trust( ch );
   minlimit = maxlimit >= LEVEL_GREATER ? -1 : 0;

   min = minlimit;
   max = maxlimit;

   idx = NULL;
   minfound = FALSE;
   maxfound = FALSE;

   for( argument = one_argument( argument, arg ); arg[0] != '\0'; argument = one_argument( argument, arg ) )
   {
      if( !isdigit( arg[0] ) )
      {
         if( idx )
         {
            set_char_color( AT_GREEN, ch );
            ch_printf( ch, "You may only use a single keyword to index the list.\r\n" );
            return;
         }
         idx = STRALLOC( arg );
      }
      else
      {
         if( !minfound )
         {
            min = URANGE( minlimit, atoi( arg ), maxlimit );
            minfound = TRUE;
         }
         else if( !maxfound )
         {
            max = URANGE( minlimit, atoi( arg ), maxlimit );
            maxfound = TRUE;
         }
         else
         {
            set_char_color( AT_GREEN, ch );
            ch_printf( ch, "You may only use two level limits.\r\n" );
            return;
         }
      }
   }

   if( min > max )
   {
      int temp = min;

      min = max;
      max = temp;
   }

   set_pager_color( AT_GREEN, ch );
   pager_printf( ch, "Help Topics in level range %d to %d:\r\n\r\n", min, max );
   for( cnt = 0, help = first_help; help; help = help->next )
      if( help->level >= min && help->level <= max && ( !idx || nifty_is_name_prefix( idx, help->keyword ) ) )
      {
         pager_printf( ch, "  %3d %s\r\n", help->level, help->keyword );
         ++cnt;
      }
   if( cnt )
      pager_printf( ch, "\r\n%d pages found.\r\n", cnt );
   else
      send_to_char( "None found.\r\n", ch );

   if( idx )
      STRFREE( idx );

   return;
}

/* 
 * New do_who with WHO REQUEST, clan, race and homepage support.  -Thoric
 *
 * Latest version of do_who eliminates redundant code by using linked lists.
 * Shows imms separately, indicates guest and retired immortals.
 * Narn, Oct/96
 *
 * Who group by Altrag, Feb 28/97
 */
struct whogr_s
{
   struct whogr_s *next;
   struct whogr_s *follower;
   struct whogr_s *l_follow;
   DESCRIPTOR_DATA *d;
   int indent;
} *first_whogr, *last_whogr;

struct whogr_s *find_whogr( DESCRIPTOR_DATA * d, struct whogr_s *first )
{
   struct whogr_s *whogr, *whogr_t;

   for( whogr = first; whogr; whogr = whogr->next )
      if( whogr->d == d )
         return whogr;
      else if( whogr->follower && ( whogr_t = find_whogr( d, whogr->follower ) ) )
         return whogr_t;
   return NULL;
}

void indent_whogr( CHAR_DATA * looker, struct whogr_s *whogr, int ilev )
{
   for( ; whogr; whogr = whogr->next )
   {
      if( whogr->follower )
      {
         int nlev = ilev;
         CHAR_DATA *wch = ( whogr->d->original ? whogr->d->original : whogr->d->character );

         if( can_see( looker, wch ) && !IS_IMMORTAL( wch ) )
            nlev += 3;
         indent_whogr( looker, whogr->follower, nlev );
      }
      whogr->indent = ilev;
   }
}

/* This a great big mess to backwards-structure the ->leader character
   fields */
void create_whogr( CHAR_DATA * looker )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *wch;
   struct whogr_s *whogr, *whogr_t;
   int dc = 0, wc = 0;

   while( ( whogr = first_whogr ) != NULL )
   {
      first_whogr = whogr->next;
      DISPOSE( whogr );
   }
   first_whogr = last_whogr = NULL;
   /*
    * Link in the ones without leaders first 
    */
   for( d = last_descriptor; d; d = d->prev )
   {
      if( d->connected != CON_PLAYING && d->connected != CON_EDITING )
         continue;
      ++dc;
      wch = ( d->original ? d->original : d->character );
      if( !wch->leader || wch->leader == wch || !wch->leader->desc ||
          IS_NPC( wch->leader ) || IS_IMMORTAL( wch ) || IS_IMMORTAL( wch->leader ) )
      {
         CREATE( whogr, struct whogr_s, 1 );
         if( !last_whogr )
            first_whogr = last_whogr = whogr;
         else
         {
            last_whogr->next = whogr;
            last_whogr = whogr;
         }
         whogr->next = NULL;
         whogr->follower = whogr->l_follow = NULL;
         whogr->d = d;
         whogr->indent = 0;
         ++wc;
      }
   }
   /*
    * Now for those who have leaders.. 
    */
   while( wc < dc )
      for( d = last_descriptor; d; d = d->prev )
      {
         if( d->connected != CON_PLAYING && d->connected != CON_EDITING )
            continue;
         if( find_whogr( d, first_whogr ) )
            continue;
         wch = ( d->original ? d->original : d->character );
         if( wch->leader && wch->leader != wch && wch->leader->desc &&
             !IS_NPC( wch->leader ) && !IS_IMMORTAL( wch ) &&
             !IS_IMMORTAL( wch->leader ) && ( whogr_t = find_whogr( wch->leader->desc, first_whogr ) ) )
         {
            CREATE( whogr, struct whogr_s, 1 );
            if( !whogr_t->l_follow )
               whogr_t->follower = whogr_t->l_follow = whogr;
            else
            {
               whogr_t->l_follow->next = whogr;
               whogr_t->l_follow = whogr;
            }
            whogr->next = NULL;
            whogr->follower = whogr->l_follow = NULL;
            whogr->d = d;
            whogr->indent = 0;
            ++wc;
         }
      }
   /*
    * Set up indentation levels 
    */
   indent_whogr( looker, first_whogr, 0 );

   /*
    * And now to linear link them.. 
    */
   for( whogr_t = NULL, whogr = first_whogr; whogr; )
      if( whogr->l_follow )
      {
         whogr->l_follow->next = whogr;
         whogr->l_follow = NULL;
         if( whogr_t )
            whogr_t->next = whogr = whogr->follower;
         else
            first_whogr = whogr = whogr->follower;
      }
      else
      {
         whogr_t = whogr;
         whogr = whogr->next;
      }
}

void do_who( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   char clan_name[MAX_INPUT_LENGTH];
   char council_name[MAX_INPUT_LENGTH];
   char invis_str[MAX_INPUT_LENGTH];
   char char_name[MAX_INPUT_LENGTH];
   const char *extra_title;
   char class_text[MAX_INPUT_LENGTH];
   struct whogr_s *whogr, *whogr_p;
   DESCRIPTOR_DATA *d;
   int iClass, iRace;
   int iLevelLower;
   int iLevelUpper;
   int nNumber;
   int nMatch;
   bool rgfClass[MAX_CLASS];
   bool rgfRace[MAX_RACE];
   bool fClassRestrict;
   bool fRaceRestrict;
   bool fImmortalOnly;
   bool fLeader;
   bool fPkill;
   bool fShowHomepage;
   bool fClanMatch;  /* SB who clan (order),who guild, and who council */
   bool fCouncilMatch;
   bool fDeityMatch;
   bool fGroup;
   CLAN_DATA *pClan = NULL;
   COUNCIL_DATA *pCouncil = NULL;
   DEITY_DATA *pDeity = NULL;
   FILE *whoout = NULL;

   /*
    * #define WT_IMM    0;
    * #define WT_MORTAL 1;
    * #define WT_DEADLY 2;
    */

   WHO_DATA *cur_who = NULL;
   WHO_DATA *next_who = NULL;
   WHO_DATA *first_mortal = NULL;
   WHO_DATA *first_imm = NULL;
   WHO_DATA *first_deadly = NULL;
   WHO_DATA *first_grouped = NULL;
   WHO_DATA *first_groupwho = NULL;


   /*
    * Set default arguments.
    */
   iLevelLower = 0;
   iLevelUpper = MAX_LEVEL;
   fClassRestrict = FALSE;
   fRaceRestrict = FALSE;
   fImmortalOnly = FALSE;
   fPkill = FALSE;
   fShowHomepage = FALSE;
   fClanMatch = FALSE;  /* SB who clan (order), who guild, who council */
   fCouncilMatch = FALSE;
   fDeityMatch = FALSE;
   fGroup = FALSE;   /* Alty who group */
   fLeader = FALSE;
   for( iClass = 0; iClass < MAX_CLASS; iClass++ )
      rgfClass[iClass] = FALSE;
   for( iRace = 0; iRace < MAX_RACE; iRace++ )
      rgfRace[iRace] = FALSE;

#ifdef REQWHOARG
   /*
    * The who command must have at least one argument because we often
    * have up to 500 players on. Too much spam if a player accidentally
    * types "who" with no arguments.           --Gorog
    */
   if( ch && argument[0] == '\0' )
   {
      send_to_pager_color( "\r\n&GYou must specify at least one argument.\r\nUse 'who 1' to view the entire who list.\r\n",
                           ch );
      return;
   }
#endif

   /*
    * Parse arguments.
    */
   nNumber = 0;
   for( ;; )
   {
      char arg[MAX_STRING_LENGTH];

      argument = one_argument( argument, arg );
      if( arg[0] == '\0' )
         break;

      if( is_number( arg ) )
      {
         switch ( ++nNumber )
         {
            case 1:
               iLevelLower = atoi( arg );
               break;
            case 2:
               iLevelUpper = atoi( arg );
               break;
            default:
               send_to_char( "Only two level numbers allowed.\r\n", ch );
               return;
         }
      }
      else
      {
         if( strlen( arg ) < 3 )
         {
            send_to_char( "Arguments must be longer than that.\r\n", ch );
            return;
         }

         /*
          * Look for classes to turn on.
          */
         if( !str_cmp( arg, "deadly" ) || !str_cmp( arg, "pkill" ) )
            fPkill = TRUE;
         else if( !str_cmp( arg, "imm" ) || !str_cmp( arg, "gods" ) )
            fImmortalOnly = TRUE;
         else if( !str_cmp( arg, "leader" ) )
            fLeader = TRUE;
         else if( !str_cmp( arg, "www" ) )
            fShowHomepage = TRUE;
         else if( !str_cmp( arg, "group" ) && ch )
            fGroup = TRUE;
         else /* SB who clan (order), guild, council */ if( ( pClan = get_clan( arg ) ) )
            fClanMatch = TRUE;
         else if( ( pCouncil = get_council( arg ) ) )
            fCouncilMatch = TRUE;
         else if( ( pDeity = get_deity( arg ) ) )
            fDeityMatch = TRUE;
         else
         {
            for( iClass = 0; iClass < MAX_CLASS; iClass++ )
            {
               if( !str_cmp( arg, class_table[iClass]->who_name ) )
               {
                  rgfClass[iClass] = TRUE;
                  break;
               }
            }
            if( iClass != MAX_CLASS )
               fClassRestrict = TRUE;

            for( iRace = 0; iRace < MAX_RACE; iRace++ )
            {
               if( !str_cmp( arg, race_table[iRace]->race_name ) )
               {
                  rgfRace[iRace] = TRUE;
                  break;
               }
            }
            if( iRace != MAX_RACE )
               fRaceRestrict = TRUE;

            if( iClass == MAX_CLASS && iRace == MAX_RACE
                && fClanMatch == FALSE && fCouncilMatch == FALSE && fDeityMatch == FALSE )
            {
               send_to_char( "That's not a class, race, order, guild," " council or deity.\r\n", ch );
               return;
            }
         }
      }
   }

   /*
    * Now find matching chars.
    */
   nMatch = 0;
   buf[0] = '\0';
   if( ch )
      set_pager_color( AT_GREEN, ch );
   else
   {
      if( fShowHomepage )
         whoout = fopen( WEBWHO_FILE, "w" );
      else
         whoout = fopen( WHO_FILE, "w" );
      if( !whoout )
      {
         bug( "do_who: cannot open who file!" );
         return;
      }
   }

/* start from last to first to get it in the proper order */
   if( fGroup )
   {
      create_whogr( ch );
      whogr = first_whogr;
      d = whogr->d;
   }
   else
   {
      whogr = NULL;
      d = last_descriptor;
   }
   whogr_p = NULL;
   for( ; d; whogr_p = whogr, whogr = ( fGroup ? whogr->next : NULL ),
        d = ( fGroup ? ( whogr ? whogr->d : NULL ) : d->prev ) )
   {
      CHAR_DATA *wch;
      char const *Class;

      if( ( d->connected != CON_PLAYING && d->connected != CON_EDITING ) || !can_see( ch, d->character ) || d->original )
         continue;
      wch = d->original ? d->original : d->character;
      if( wch->level < iLevelLower || wch->level > iLevelUpper || ( fPkill && !CAN_PKILL( wch ) ) || ( fImmortalOnly && wch->level < LEVEL_IMMORTAL ) || ( fClassRestrict && !rgfClass[wch->Class] ) || ( fRaceRestrict && !rgfRace[wch->race] ) || ( fClanMatch && ( pClan != wch->pcdata->clan ) )  /* SB */
          || ( fCouncilMatch && ( pCouncil != wch->pcdata->council ) )  /* SB */
          || ( fDeityMatch && ( pDeity != wch->pcdata->deity ) ) )
         continue;
      if( fLeader && !( wch->pcdata->council &&
                        ( ( wch->pcdata->council->head2 &&
                            !str_cmp( wch->pcdata->council->head2, wch->name ) ) ||
                          ( wch->pcdata->council->head &&
                            !str_cmp( wch->pcdata->council->head, wch->name ) ) ) ) &&
          !( wch->pcdata->clan && ( ( wch->pcdata->clan->deity &&
                                      !str_cmp( wch->pcdata->clan->deity, wch->name ) )
                                    || ( wch->pcdata->clan->leader
                                         && !str_cmp( wch->pcdata->clan->leader, wch->name ) )
                                    || ( wch->pcdata->clan->number1
                                         && !str_cmp( wch->pcdata->clan->number1, wch->name ) )
                                    || ( wch->pcdata->clan->number2
                                         && !str_cmp( wch->pcdata->clan->number2, wch->name ) ) ) ) )
         continue;

      if( fGroup && !wch->leader && !IS_SET( wch->pcdata->flags, PCFLAG_GROUPWHO ) && ( !whogr_p || !whogr_p->indent ) )
         continue;

      nMatch++;

      if( fShowHomepage && wch->pcdata->homepage && wch->pcdata->homepage[0] != '\0' )
         snprintf( char_name, MAX_INPUT_LENGTH, "<A HREF=\"%s\">%s</A>", show_tilde( wch->pcdata->homepage ), wch->name );
      else
         mudstrlcpy( char_name, wch->name, MAX_INPUT_LENGTH );

      snprintf( class_text, MAX_INPUT_LENGTH, "%s%2d %s", NOT_AUTHED( wch ) ? "N" : " ", wch->level,
                class_table[wch->Class]->who_name );
      Class = class_text;
      switch ( wch->level )
      {
         default:
            break;
         case MAX_LEVEL - 0:
            Class = "Supreme Entity";
            break;
         case MAX_LEVEL - 1:
            Class = "Infinite";
            break;
         case MAX_LEVEL - 2:
            Class = "Eternal";
            break;
         case MAX_LEVEL - 3:
            Class = "Ancient";
            break;
         case MAX_LEVEL - 4:
            Class = "Exalted God";
            break;
         case MAX_LEVEL - 5:
            Class = "Ascendant God";
            break;
         case MAX_LEVEL - 6:
            Class = "Greater God";
            break;
         case MAX_LEVEL - 7:
            Class = "God";
            break;
         case MAX_LEVEL - 8:
            Class = "Lesser God";
            break;
         case MAX_LEVEL - 9:
            Class = "Immortal";
            break;
         case MAX_LEVEL - 10:
            Class = "Demi God";
            break;
         case MAX_LEVEL - 11:
            Class = "Savior";
            break;
         case MAX_LEVEL - 12:
            Class = "Creator";
            break;
         case MAX_LEVEL - 13:
            Class = "Acolyte";
            break;
         case MAX_LEVEL - 14:
            Class = "Neophyte";
            break;
         case MAX_LEVEL - 15:
            Class = "Avatar";
            break;
      }

      if( !str_cmp( wch->name, sysdata.guild_overseer ) )
         extra_title = " [Overseer of Guilds]";
      else if( !str_cmp( wch->name, sysdata.guild_advisor ) )
         extra_title = " [Advisor to Guilds]";
      else
         extra_title = "";

      if( IS_RETIRED( wch ) )
         Class = "Retired";
      else if( IS_GUEST( wch ) )
         Class = "Guest";
      else if( wch->pcdata->clan
               && !str_cmp( wch->name, wch->pcdata->clan->leader ) && wch->pcdata->clan->leadrank[0] != '\0' )
         Class = wch->pcdata->clan->leadrank;
      else if( wch->pcdata->clan
               && !str_cmp( wch->name, wch->pcdata->clan->number1 ) && wch->pcdata->clan->onerank[0] != '\0' )
         Class = wch->pcdata->clan->onerank;
      else if( wch->pcdata->clan
               && !str_cmp( wch->name, wch->pcdata->clan->number2 ) && wch->pcdata->clan->tworank[0] != '\0' )
         Class = wch->pcdata->clan->tworank;
      else if( wch->pcdata->rank && wch->pcdata->rank[0] != '\0' )
         Class = wch->pcdata->rank;

      if( wch->pcdata->clan )
      {
         CLAN_DATA *pclan = wch->pcdata->clan;
         if( pclan->clan_type == CLAN_GUILD )
            mudstrlcpy( clan_name, " <", MAX_INPUT_LENGTH );
         else
            mudstrlcpy( clan_name, " (", MAX_INPUT_LENGTH );

         if( pclan->clan_type == CLAN_ORDER )
         {
            if( !str_cmp( wch->name, pclan->deity ) )
               mudstrlcat( clan_name, "Deity, Order of ", MAX_INPUT_LENGTH );
            else if( !str_cmp( wch->name, pclan->leader ) )
               mudstrlcat( clan_name, "Leader, Order of ", MAX_INPUT_LENGTH );
            else if( !str_cmp( wch->name, pclan->number1 ) )
               mudstrlcat( clan_name, "Number One, Order of ", MAX_INPUT_LENGTH );
            else if( !str_cmp( wch->name, pclan->number2 ) )
               mudstrlcat( clan_name, "Number Two, Order of ", MAX_INPUT_LENGTH );
            else
               mudstrlcat( clan_name, "Order of ", MAX_INPUT_LENGTH );
         }
         else if( pclan->clan_type == CLAN_GUILD )
         {
            if( !str_cmp( wch->name, pclan->leader ) )
               mudstrlcat( clan_name, "Leader, ", MAX_INPUT_LENGTH );
            if( !str_cmp( wch->name, pclan->number1 ) )
               mudstrlcat( clan_name, "First, ", MAX_INPUT_LENGTH );
            if( !str_cmp( wch->name, pclan->number2 ) )
               mudstrlcat( clan_name, "Second, ", MAX_INPUT_LENGTH );
         }
         else
         {
            if( !str_cmp( wch->name, pclan->deity ) )
               mudstrlcat( clan_name, "Deity of ", MAX_INPUT_LENGTH );
            else if( !str_cmp( wch->name, pclan->leader ) )
               mudstrlcat( clan_name, "Leader of ", MAX_INPUT_LENGTH );
            else if( !str_cmp( wch->name, pclan->number1 ) )
               mudstrlcat( clan_name, "Number One ", MAX_INPUT_LENGTH );
            else if( !str_cmp( wch->name, pclan->number2 ) )
               mudstrlcat( clan_name, "Number Two ", MAX_INPUT_LENGTH );
         }
         mudstrlcat( clan_name, pclan->name, MAX_INPUT_LENGTH );
         if( pclan->clan_type == CLAN_GUILD )
            mudstrlcat( clan_name, ">", MAX_INPUT_LENGTH );
         else
            mudstrlcat( clan_name, ")", MAX_INPUT_LENGTH );
      }
      else
         clan_name[0] = '\0';

      if( wch->pcdata->council )
      {
         mudstrlcpy( council_name, " [", MAX_INPUT_LENGTH );
         if( wch->pcdata->council->head2 == NULL )
         {
            if( !str_cmp( wch->name, wch->pcdata->council->head ) )
               mudstrlcat( council_name, "Head of ", MAX_INPUT_LENGTH );
         }
         else
         {
            if( !str_cmp( wch->name, wch->pcdata->council->head ) || !str_cmp( wch->name, wch->pcdata->council->head2 ) )
               mudstrlcat( council_name, "Co-Head of ", MAX_INPUT_LENGTH );
         }
         mudstrlcat( council_name, wch->pcdata->council_name, MAX_INPUT_LENGTH );
         mudstrlcat( council_name, "]", MAX_INPUT_LENGTH );
      }
      else
         council_name[0] = '\0';

      if( xIS_SET( wch->act, PLR_WIZINVIS ) )
         snprintf( invis_str, MAX_INPUT_LENGTH, "(%d) ", wch->pcdata->wizinvis );
      else
         invis_str[0] = '\0';
      snprintf( buf, MAX_STRING_LENGTH, "%*s%-15s %s%s%s%s%s%s%s%s.%s%s%s\r\n",
                ( fGroup ? whogr->indent : 0 ), "",
                Class,
                invis_str,
                ( wch->desc && wch->desc->connected ) ? "[WRITING] " : "",
                xIS_SET( wch->act, PLR_AFK ) ? "[AFK] " : "",
                xIS_SET( wch->act, PLR_ATTACKER ) ? "(ATTACKER) " : "",
                xIS_SET( wch->act, PLR_KILLER ) ? "(KILLER) " : "",
                xIS_SET( wch->act, PLR_THIEF ) ? "(THIEF) " : "",
                char_name, wch->pcdata->title, extra_title, clan_name, council_name );

      /*
       * This is where the old code would display the found player to the ch.
       * What we do instead is put the found data into a linked list
       */

      /*
       * First make the structure. 
       */
      CREATE( cur_who, WHO_DATA, 1 );
      cur_who->text = str_dup( buf );
      if( wch->level > LEVEL_AVATAR && IS_IMMORTAL( wch ) )
         cur_who->type = WT_IMM;
      else if( fGroup )
         if( wch->leader || ( whogr_p && whogr_p->indent ) )
            cur_who->type = WT_GROUPED;
         else
            cur_who->type = WT_GROUPWHO;
      else if( CAN_PKILL( wch ) )
         cur_who->type = WT_DEADLY;
      else
         cur_who->type = WT_MORTAL;

      /*
       * Then put it into the appropriate list. 
       */
      switch ( cur_who->type )
      {
         case WT_MORTAL:
            cur_who->next = first_mortal;
            first_mortal = cur_who;
            break;
         case WT_DEADLY:
            cur_who->next = first_deadly;
            first_deadly = cur_who;
            break;
         case WT_GROUPED:
            cur_who->next = first_grouped;
            first_grouped = cur_who;
            break;
         case WT_GROUPWHO:
            cur_who->next = first_groupwho;
            first_groupwho = cur_who;
            break;
         case WT_IMM:
            cur_who->next = first_imm;
            first_imm = cur_who;
            break;
      }

   }


   /*
    * Ok, now we have three separate linked lists and what remains is to 
    * * display the information and clean up.
    */
   /*
    * Two extras now for grouped and groupwho (wanting group). -- Alty
    */

   for( cur_who = first_mortal; cur_who; cur_who = next_who )
   {
      if( !ch )
         fprintf( whoout, "%s", cur_who->text );
      else
         send_to_pager( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who );
   }

   if( first_deadly )
   {
      if( !ch )
         fprintf( whoout, "%s",
                  "\r\n-------------------------------[ DEADLY CHARACTERS ]-------------------------\r\n\r\n" );
      else
         send_to_pager( "\r\n-------------------------------[ DEADLY CHARACTERS ]--------------------------\r\n\r\n", ch );
   }

   for( cur_who = first_deadly; cur_who; cur_who = next_who )
   {
      if( !ch )
         fprintf( whoout, "%s", cur_who->text );
      else
         send_to_pager( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who );
   }

   if( first_grouped )
   {
/*      if ( !ch )
        fprintf( whoout, "%s", "\r\n-----------------------------[ GROUPED CHARACTERS ]---------------------------\r\n\r\n" );
      else*/
      send_to_pager( "\r\n-----------------------------[ GROUPED CHARACTERS ]---------------------------\r\n\r\n", ch );
   }
   for( cur_who = first_grouped; cur_who; cur_who = next_who )
   {
/*      if ( !ch )
        fprintf( whoout, cur_who->text );
      else*/
      send_to_pager( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who );
   }

   if( first_groupwho )
   {
      if( !ch )
         fprintf( whoout, "%s",
                  "\r\n-------------------------------[ WANTING GROUP ]------------------------------\r\n\r\n" );
      else
         send_to_pager( "\r\n-------------------------------[ WANTING GROUP ]------------------------------\r\n\r\n", ch );
   }
   for( cur_who = first_groupwho; cur_who; cur_who = next_who )
   {
      if( !ch )
         fprintf( whoout, "%s", cur_who->text );
      else
         send_to_pager( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who );
   }

   if( first_imm )
   {
      if( !ch )
         fprintf( whoout, "%s",
                  "\r\n-----------------------------------[ IMMORTALS ]-----------------------------\r\n\r\n" );
      else
         send_to_pager( "\r\n-----------------------------------[ IMMORTALS ]------------------------------\r\n\r\n", ch );
   }

   for( cur_who = first_imm; cur_who; cur_who = next_who )
   {
      if( !ch )
         fprintf( whoout, "%s", cur_who->text );
      else
         send_to_pager( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who );
   }

   if( !ch )
   {
      fprintf( whoout, "%d player%s.\r\n", nMatch, nMatch == 1 ? "" : "s" );
      fclose( whoout );
      return;
   }

   set_char_color( AT_YELLOW, ch );
   ch_printf( ch, "%d player%s.\r\n", nMatch, nMatch == 1 ? "" : "s" );
   return;
}

void do_compare( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_DATA *obj1;
   OBJ_DATA *obj2;
   int value1;
   int value2;
   const char *msg;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Compare what to what?\r\n", ch );
      return;
   }

   if( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
   {
      send_to_char( "You do not have that item.\r\n", ch );
      return;
   }

   if( arg2[0] == '\0' )
   {
      for( obj2 = ch->first_carrying; obj2; obj2 = obj2->next_content )
      {
         if( obj2->wear_loc != WEAR_NONE
             && can_see_obj( ch, obj2 )
             && obj1->item_type == obj2->item_type && ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE ) != 0 )
            break;
      }

      if( !obj2 )
      {
         send_to_char( "You aren't wearing anything comparable.\r\n", ch );
         return;
      }
   }
   else
   {
      if( ( obj2 = get_obj_carry( ch, arg2 ) ) == NULL )
      {
         send_to_char( "You do not have that item.\r\n", ch );
         return;
      }
   }

   msg = NULL;
   value1 = 0;
   value2 = 0;

   if( obj1 == obj2 )
   {
      msg = "You compare $p to itself.  It looks about the same.";
   }
   else if( obj1->item_type != obj2->item_type )
   {
      msg = "You can't compare $p and $P.";
   }
   else
   {
      switch ( obj1->item_type )
      {
         default:
            msg = "You can't compare $p and $P.";
            break;

         case ITEM_ARMOR:
            value1 = obj1->value[0];
            value2 = obj2->value[0];
            break;

         case ITEM_WEAPON:
            value1 = obj1->value[1] + obj1->value[2];
            value2 = obj2->value[1] + obj2->value[2];
            break;
      }
   }

   if( !msg )
   {
      if( value1 == value2 )
         msg = "$p and $P look about the same.";
      else if( value1 > value2 )
         msg = "$p looks better than $P.";
      else
         msg = "$p looks worse than $P.";
   }

   act( AT_PLAIN, msg, ch, obj1, obj2, TO_CHAR );
   return;
}

void do_where( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   bool found;

   one_argument( argument, arg );

   if( arg[0] != '\0'
       && ( victim = get_char_world( ch, arg ) ) && !IS_NPC( victim )
       && IS_SET( victim->pcdata->flags, PCFLAG_DND ) && get_trust( ch ) < get_trust( victim ) )
   {
      act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
      return;
   }

   set_pager_color( AT_PERSON, ch );
   if( arg[0] == '\0' )
   {
      pager_printf( ch, "\r\nPlayers near you in %s:\r\n", ch->in_room->area->name );
      found = FALSE;
      for( d = first_descriptor; d; d = d->next )
      {
         if( ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
             && ( victim = d->character ) != NULL
             && !IS_NPC( victim )
             && victim->in_room
             && ( ( victim->in_room->area == ch->in_room->area )
                  && ( !xIS_SET( victim->in_room->room_flags, ROOM_HOUSE )
                       || in_same_house( victim, ch ) ) )
             && can_see( ch, victim )
             && ( victim->in_room == ch->in_room
                  || IS_IMMORTAL( ch )
                  || ( !IS_SET( ch->in_room->area->flags, AFLAG_NOWHERE )
                       && !xIS_SET( victim->in_room->room_flags, ROOM_NOWHERE ) ) )
             && ( get_trust( ch ) >= get_trust( victim ) || !IS_SET( victim->pcdata->flags, PCFLAG_DND ) ) )
            /* if victim has the DND flag ch must outrank them */ 
         {
            found = TRUE;

            pager_printf_color( ch, "&P%-13s  ", victim->name );
            if( IS_IMMORTAL( victim ) && victim->level > LEVEL_AVATAR )
               send_to_pager_color( "&P(&WImmortal&P)\t", ch );
            else if( CAN_PKILL( victim ) && victim->pcdata->clan
                     && victim->pcdata->clan->clan_type != CLAN_ORDER && victim->pcdata->clan->clan_type != CLAN_GUILD )
               pager_printf_color( ch, "%-18s\t", victim->pcdata->clan->badge );
            else if( CAN_PKILL( victim ) )
               send_to_pager_color( "(&wUnclanned&P)\t", ch );
            else
               send_to_pager( "\t\t\t", ch );
            pager_printf_color( ch, "&P%s\r\n", victim->in_room->name );
         }
      }
      if( !found )
         send_to_char( "None\r\n", ch );
   }
   else
   {
      found = FALSE;
      for( victim = first_char; victim; victim = victim->next )
         if( victim->in_room
             && victim->in_room->area == ch->in_room->area
             && !IS_AFFECTED( victim, AFF_HIDE )
             && !IS_AFFECTED( victim, AFF_SNEAK ) && can_see( ch, victim ) && is_name( arg, victim->name ) )
         {
            found = TRUE;
            pager_printf( ch, "%-28s %s\r\n", PERS( victim, ch ), victim->in_room->name );
            break;
         }
      if( !found )
         act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
   }

   return;
}

void do_consider( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   const char *msg;
   int diff;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Consider killing whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They're not here.\r\n", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "You decide you're pretty sure you could take yourself in a fight.\r\n", ch );
      return;
   }
   diff = victim->level - ch->level;

   if( diff <= -10 )
      msg = "You are far more experienced than $N.";
   else if( diff <= -5 )
      msg = "$N is not nearly as experienced as you.";
   else if( diff <= -2 )
      msg = "You are more experienced than $N.";
   else if( diff <= 1 )
      msg = "You are just about as experienced as $N.";
   else if( diff <= 4 )
      msg = "You are not nearly as experienced as $N.";
   else if( diff <= 9 )
      msg = "$N is far more experienced than you!";
   else
      msg = "$N would make a great teacher for you!";
   act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

   diff = ( int )( victim->max_hit - ch->max_hit ) / 6;

   if( diff <= -200 )
      msg = "$N looks like a feather!";
   else if( diff <= -150 )
      msg = "You could kill $N with your hands tied!";
   else if( diff <= -100 )
      msg = "Hey! Where'd $N go?";
   else if( diff <= -50 )
      msg = "$N is a wimp.";
   else if( diff <= 0 )
      msg = "$N looks weaker than you.";
   else if( diff <= 50 )
      msg = "$N looks about as strong as you.";
   else if( diff <= 100 )
      msg = "It would take a bit of luck...";
   else if( diff <= 150 )
      msg = "It would take a lot of luck, and equipment!";
   else if( diff <= 200 )
      msg = "Why don't you dig a grave for yourself first?";
   else
      msg = "$N is built like a TANK!";
   act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

   return;
}

/*
 * Place any skill types you don't want them to be able to practice
 * normally in this list.  Separate each with a space.
 * (Uses an is_name check). -- Altrag
 */
#define CANT_PRAC "Tongue"

void do_practice( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *mob;
   char buf[MAX_STRING_LENGTH];
   int sn;

   if( IS_NPC( ch ) )
      return;

   for( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
      if( IS_NPC( mob ) && xIS_SET( mob->act, ACT_PRACTICE ) )
         break;

   if( argument[0] == '\0' )
   {
      int col;
      short lasttype, cnt;
      bool is_ok;

      col = cnt = 0;
      lasttype = SKILL_SPELL;
      set_pager_color( AT_MAGIC, ch );

      for( sn = 0; sn < num_skills; ++sn )
      {
         const SKILLTYPE *skill;
         int normalSn;

         // the first num_sorted_skills are sorted by type, so we don't want
         // to index into skill_table -- that is sorted alphabetically -- so
         // we need to do the following dance to check if we are in the
         // sorted section or the unsorted section.
         if( sn < num_sorted_skills )
         {
            skill = skill_table_bytype[sn];

            // we are looping over the skills sorted by type.
            // So, we need to get the normal sn as well.
            normalSn = skill_lookup( skill->name );
         }
         else
         {
            skill = skill_table[sn];
            normalSn = sn;
         }

         if( !skill || !skill->name || skill->name[0] == '\0' )
            continue;

         if( strcmp( skill->name, "reserved" ) == 0 && ( IS_IMMORTAL( ch ) || CAN_CAST( ch ) ) )
         {
            if( col % 3 != 0 )
               send_to_pager( "\r\n", ch );
            set_pager_color( AT_MAGIC, ch );
            send_to_pager_color( " ----------------------------------[&CSpells&B]----------------------------------\r\n",
                                 ch );
            col = 0;
         }

         if( skill->type != lasttype )
         {
            if( !cnt )
               send_to_pager( "                                   (none)\r\n", ch );
            else if( col % 3 != 0 )
               send_to_pager( "\r\n", ch );
            set_pager_color( AT_MAGIC, ch );
            pager_printf_color( ch,
                                " ----------------------------------&C%ss&B----------------------------------\r\n",
                                skill_tname[skill->type] );
            col = cnt = 0;
         }
         lasttype = skill->type;

         if( !IS_IMMORTAL( ch )
             && ( skill->guild != CLASS_NONE && ( !IS_GUILDED( ch ) || ( ch->pcdata->clan->Class != skill->guild ) ) ) )
            continue;

         /**
         * mob will only display practice skills it can train a player in (up to its own level in rank)  
         * (e.g. A level 10 NPC trainer mob will display the skills and spells a character can train
         * up to level 10) 
         **/
         if( mob )  
         {
            if( skill->skill_level[ch->Class] > mob->level && skill->race_level[ch->race] > mob->level )
               continue;
         }
        
         is_ok = FALSE;

         if( ch->level >= skill->skill_level[ch->Class] )
            is_ok = TRUE;
         if( ch->level >= skill->race_level[ch->race] )
            is_ok = TRUE;

         if( !is_ok )
            continue;
          

         if( ch->pcdata->learned[sn] <= 0 && SPELL_FLAG( skill, SF_SECRETSKILL ) )
            continue;

         ++cnt;
         set_pager_color( AT_MAGIC, ch );
         pager_printf( ch, "%20.20s", skill->name );
         if( ch->pcdata->learned[normalSn] > 0 )
            set_pager_color( AT_SCORE, ch );
         pager_printf( ch, " %3d%% ", ch->pcdata->learned[normalSn] );
         if( ++col % 3 == 0 )
            send_to_pager( "\r\n", ch );
      }

      if( col % 3 != 0 )
         send_to_pager( "\r\n", ch );
      set_pager_color( AT_MAGIC, ch );
      pager_printf( ch, "You have %d practice sessions left.\r\n", ch->practice );
   }
   else
   {
      int adept;
      bool can_prac = TRUE;

      if( !IS_AWAKE( ch ) )
      {
         send_to_char( "In your dreams, or what?\r\n", ch );
         return;
      }

      if( !mob )
      {
         send_to_char( "You can't do that here.\r\n", ch );
         return;
      }

      if( ch->practice <= 0 )
      {
         act( AT_TELL, "$n tells you 'You must earn some more practice sessions.'", mob, NULL, ch, TO_VICT );
         return;
      }

      sn = skill_lookup( argument );

      if( can_prac && ( ( sn == -1 ) || ( !IS_NPC( ch ) && ch->level < skill_table[sn]->skill_level[ch->Class]
                                          && ch->level < skill_table[sn]->race_level[ch->race] ) ) )
      {
         act( AT_TELL, "$n tells you 'You're not ready to learn that yet...'", mob, NULL, ch, TO_VICT );
         return;
      }

      if( is_name( skill_tname[skill_table[sn]->type], CANT_PRAC ) )
      {
         act( AT_TELL, "$n tells you 'I do not know how to teach that.'", mob, NULL, ch, TO_VICT );
         return;
      }

      /*
       * Skill requires a special teacher
       */
      if( skill_table[sn]->teachers && skill_table[sn]->teachers[0] != '\0' )
      {
         snprintf( buf, MAX_STRING_LENGTH, "%d", mob->pIndexData->vnum );
         if( !is_name( buf, skill_table[sn]->teachers ) )
         {
            act( AT_TELL, "$n tells you, 'I know not know how to teach that.'", mob, NULL, ch, TO_VICT );
            return;
         }
      }

/*
 * Guild checks - right now, cant practice guild skills - done on 
 * induct/outcast
 */
/*	
	if ( !IS_NPC(ch) && !IS_GUILDED(ch)
	&&    skill_table[sn]->guild != CLASS_NONE)
	{
	    act( AT_TELL, "$n tells you 'Only guild members can use that..'"
		mob, NULL, ch, TO_VICT );
	    return;
	}

	if ( !IS_NPC(ch) && skill_table[sn]->guild != CLASS_NONE 
	     && ch->pcdata->clan->class != skill_table[sn]->guild )
	{
	    act( AT_TELL, "$n tells you 'That can not be used by your guild.'"
		mob, NULL, ch, TO_VICT );
	    return;
	}
*/
      if( !IS_NPC( ch ) && skill_table[sn]->guild != CLASS_NONE )
      {
         act( AT_TELL, "$n tells you 'That is only for members of guilds...'", mob, NULL, ch, TO_VICT );
         return;
      }

      /*
       * Disabled for now
       if ( mob->level < skill_table[sn]->skill_level[ch->class]
       ||   mob->level < skill_table[sn]->skill_level[mob->class] )
       {
       act( AT_TELL, "$n tells you 'You must seek another to teach you that...'",
       mob, NULL, ch, TO_VICT );
       return;
       }
       */

      adept = ( int )( class_table[ch->Class]->skill_adept * 0.2 );

      if( ch->pcdata->learned[sn] >= adept )
      {
         snprintf( buf, MAX_STRING_LENGTH, "$n tells you, 'I've taught you everything I can about %s.'",
                   skill_table[sn]->name );
         act( AT_TELL, buf, mob, NULL, ch, TO_VICT );
         act( AT_TELL, "$n tells you, 'You'll have to practice it on your own now...'", mob, NULL, ch, TO_VICT );
      }
      else
      {
         ch->practice--;
         ch->pcdata->learned[sn] += int_app[get_curr_int( ch )].learn;
         act( AT_ACTION, "You practice $T.", ch, NULL, skill_table[sn]->name, TO_CHAR );
         act( AT_ACTION, "$n practices $T.", ch, NULL, skill_table[sn]->name, TO_ROOM );
         if( ch->pcdata->learned[sn] >= adept )
         {
            ch->pcdata->learned[sn] = adept;
            act( AT_TELL, "$n tells you. 'You'll have to practice it on your own now...'", mob, NULL, ch, TO_VICT );
         }
      }
   }
   return;
}

void do_wimpy( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   int wimpy;

   set_char_color( AT_YELLOW, ch );
   one_argument( argument, arg );
   if( !str_cmp( arg, "max" ) )
   {
      if( IS_PKILL( ch ) )
         wimpy = ( int )( ch->max_hit / 2.25 );
      else
         wimpy = ( int )( ch->max_hit / 1.2 );
   }
   else if( arg[0] == '\0' )
      wimpy = ( int )ch->max_hit / 5;
   else
      wimpy = atoi( arg );

   if( wimpy < 0 )
   {
      send_to_char( "Your courage exceeds your wisdom.\r\n", ch );
      return;
   }
   if( IS_PKILL( ch ) && wimpy > ( int )ch->max_hit / 2.25 )
   {
      send_to_char( "Such cowardice ill becomes you.\r\n", ch );
      return;
   }
   else if( wimpy > ( int )ch->max_hit / 1.2 )
   {
      send_to_char( "Such cowardice ill becomes you.\r\n", ch );
      return;
   }
   ch->wimpy = wimpy;
   ch_printf( ch, "Wimpy set to %d hit points.\r\n", wimpy );
   return;
}

void do_password( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
   char *pArg, *pwdnew;
   char cEnd;

   if( IS_NPC( ch ) )
      return;

   /*
    * Can't use one_argument here because it smashes case.
    * So we just steal all its code.  Bleagh.
    */
   pArg = arg1;
   while( isspace( *argument ) )
      argument++;

   cEnd = ' ';
   if( *argument == '\'' || *argument == '"' )
      cEnd = *argument++;

   while( *argument != '\0' )
   {
      if( *argument == cEnd )
      {
         argument++;
         break;
      }
      *pArg++ = *argument++;
   }
   *pArg = '\0';

   pArg = arg2;
   while( isspace( *argument ) )
      argument++;

   cEnd = ' ';
   if( *argument == '\'' || *argument == '"' )
      cEnd = *argument++;

   while( *argument != '\0' )
   {
      if( *argument == cEnd )
      {
         argument++;
         break;
      }
      *pArg++ = *argument++;
   }
   *pArg = '\0';

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Syntax: password <new> <again>.\r\n", ch );
      return;
   }

   /*
    * This should stop all the mistyped password problems --Shaddai 
    */
   if( strcmp( arg1, arg2 ) )
   {
      send_to_char( "Passwords don't match try again.\r\n", ch );
      return;
   }
   if( strlen( arg2 ) < 5 )
   {
      send_to_char( "New password must be at least five characters long.\r\n", ch );
      return;
   }

   if( arg1[0] == '!' || arg2[0] == '!' )
   {
      send_to_char( "New password cannot begin with the '!' character.", ch );
      return;
   }

   pwdnew = sha256_crypt( arg2 );   /* SHA-256 Encryption */

   DISPOSE( ch->pcdata->pwd );
   ch->pcdata->pwd = str_dup( pwdnew );
   if( IS_SET( sysdata.save_flags, SV_PASSCHG ) )
      save_char_obj( ch );
   if( ch->desc && ch->desc->host[0] != '\0' )
      log_printf( "%s changing password from site %s\n", ch->name, ch->desc->host );
   else
      log_printf( "%s changing thier password with no descriptor!", ch->name );
   send_to_char( "Ok.\r\n", ch );
}

void do_socials( CHAR_DATA* ch, const char* argument)
{
   int iHash;
   int col = 0;
   SOCIALTYPE *social;

   set_pager_color( AT_PLAIN, ch );
   for( iHash = 0; iHash < 27; iHash++ )
      for( social = social_index[iHash]; social; social = social->next )
      {
         pager_printf( ch, "%-12s", social->name );
         if( ++col % 6 == 0 )
            send_to_pager( "\r\n", ch );
      }

   if( col % 6 != 0 )
      send_to_pager( "\r\n", ch );
   return;
}

void do_commands( CHAR_DATA* ch, const char* argument)
{
   int col;
   bool found;
   int hash;
   CMDTYPE *command;

   col = 0;
   set_pager_color( AT_PLAIN, ch );
   if( argument[0] == '\0' )
   {
      for( hash = 0; hash < 126; hash++ )
         for( command = command_hash[hash]; command; command = command->next )
            if( command->level < LEVEL_HERO
                && command->level <= get_trust( ch ) && ( command->name[0] != 'm' || command->name[1] != 'p' ) )
            {
               pager_printf( ch, "%-12s", command->name );
               if( ++col % 6 == 0 )
                  send_to_pager( "\r\n", ch );
            }
      if( col % 6 != 0 )
         send_to_pager( "\r\n", ch );
   }
   else
   {
      found = FALSE;
      for( hash = 0; hash < 126; hash++ )
         for( command = command_hash[hash]; command; command = command->next )
            if( command->level < LEVEL_HERO
                && command->level <= get_trust( ch )
                && !str_prefix( argument, command->name ) && ( command->name[0] != 'm' || command->name[1] != 'p' ) )
            {
               pager_printf( ch, "%-12s", command->name );
               found = TRUE;
               if( ++col % 6 == 0 )
                  send_to_pager( "\r\n", ch );
            }

      if( col % 6 != 0 )
         send_to_pager( "\r\n", ch );
      if( !found )
         ch_printf( ch, "No command found under %s.\r\n", argument );
   }
   return;
}

void do_channels( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   one_argument( argument, arg );

   if( IS_NPC( ch ) )
      return;

   if( arg[0] == '\0' )
   {
      if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_SILENCE ) )
      {
         set_char_color( AT_GREEN, ch );
         send_to_char( "You are silenced.\r\n", ch );
         return;
      }

      /*
       * Channels everyone sees regardless of affiliation --Blodkai 
       */
      send_to_char_color( "\r\n &gPublic channels  (severe penalties for abuse)&G:\r\n  ", ch );
      ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_RACETALK ) ? " &G+RACETALK" : " &g-racetalk" );
      ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_CHAT ) ? " &G+CHAT" : " &g-chat" );
      if( get_trust( ch ) > 2 && !NOT_AUTHED( ch ) )
         ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_AUCTION ) ? " &G+AUCTION" : " &g-auction" );
      ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_TRAFFIC ) ? " &G+TRAFFIC" : " &g-traffic" );
      ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_QUEST ) ? " &G+QUEST" : " &g-quest" );
      ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_WARTALK ) ? " &G+WARTALK" : " &g-wartalk" );
      if( IS_HERO( ch ) )
         ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_AVTALK ) ? " &G+AVATAR" : " &g-avatar" );
      ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_MUSIC ) ? " &G+MUSIC" : " &g-music" );
      ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_ASK ) ? " &G+ASK" : " &g-ask" );
      ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_SHOUT ) ? " &G+SHOUT" : " &g-shout" );
      ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_YELL ) ? " &G+YELL" : " &g-yell" );

      /*
       * For organization channels (orders, clans, guilds, councils) 
       */
      send_to_char_color( "\r\n &gPrivate channels (severe penalties for abuse)&G:\r\n ", ch );
      ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_TELLS ) ? " &G+TELLS" : " &g-tells" );
      ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_WHISPER ) ? " &G+WHISPER" : " &g-whisper" );
      if( !IS_NPC( ch ) && ch->pcdata->clan )
      {
         if( ch->pcdata->clan->clan_type == CLAN_ORDER )
            send_to_char_color( !IS_SET( ch->deaf, CHANNEL_ORDER ) ? " &G+ORDER" : " &g-order", ch );

         else if( ch->pcdata->clan->clan_type == CLAN_GUILD )
            send_to_char_color( !IS_SET( ch->deaf, CHANNEL_GUILD ) ? " &G+GUILD" : " &g-guild", ch );
         else
            send_to_char_color( !IS_SET( ch->deaf, CHANNEL_CLAN ) ? " &G+CLAN" : " &g-clan", ch );
      }
      if( IS_IMMORTAL( ch ) || ( ch->pcdata->council && !str_cmp( ch->pcdata->council->name, "Newbie Council" ) ) )
         ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_NEWBIE ) ? " &G+NEWBIE" : " &g-newbie" );
      if( !IS_NPC( ch ) && ch->pcdata->council )
         ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_COUNCIL ) ? " &G+COUNCIL" : " &g-council" );
      if( !IS_NPC( ch ) && IS_SET( ch->pcdata->flags, PCFLAG_RETIRED ) )
         ch_printf_color( ch, "%s", !IS_SET( ch->deaf, CHANNEL_RETIRED ) ? " &G+RETIRED" : " &g-retired" );

      /*
       * Immortal channels 
       */
      if( IS_IMMORTAL( ch ) )
      {
         send_to_char_color( "\r\n &gImmortal Channels&G:\r\n  ", ch );
         send_to_char_color( !IS_SET( ch->deaf, CHANNEL_IMMTALK ) ? " &G+IMMTALK" : " &g-immtalk", ch );
/*          send_to_char_color( !IS_SET( ch->deaf, CHANNEL_PRAY )       ?
		" &G+PRAY"	:	" &g-pray", ch ); */
         if( get_trust( ch ) >= sysdata.muse_level )
            send_to_char_color( !IS_SET( ch->deaf, CHANNEL_HIGHGOD ) ? " &G+MUSE" : " &g-muse", ch );
         send_to_char_color( !IS_SET( ch->deaf, CHANNEL_MONITOR ) ? " &G+MONITOR" : " &g-monitor", ch );
         send_to_char_color( !IS_SET( ch->deaf, CHANNEL_DEATH ) ? " &G+DEATH" : " &g-death", ch );
         send_to_char_color( !IS_SET( ch->deaf, CHANNEL_AUTH ) ? " &G+AUTH" : " &g-auth", ch );
      }
      if( get_trust( ch ) >= sysdata.log_level )
      {
         send_to_char_color( !IS_SET( ch->deaf, CHANNEL_LOG ) ? " &G+LOG" : " &g-log", ch );
         send_to_char_color( !IS_SET( ch->deaf, CHANNEL_BUILD ) ? " &G+BUILD" : " &g-build", ch );
         send_to_char_color( !IS_SET( ch->deaf, CHANNEL_COMM ) ? " &G+COMM" : " &g-comm", ch );
         send_to_char_color( !IS_SET( ch->deaf, CHANNEL_WARN ) ? " &G+WARN" : " &g-warn", ch );
         if( get_trust( ch ) >= sysdata.think_level )
            send_to_char_color( !IS_SET( ch->deaf, CHANNEL_HIGH ) ? " &G+HIGH" : " &g-high", ch );
         if( get_trust( ch ) >= LEVEL_LESSER )
            send_to_char_color( !IS_SET( ch->deaf, CHANNEL_BUG ) ? " &G+BUG" : " &g-bug", ch );
      }
      send_to_char( "\r\n", ch );
   }
   else
   {
      bool fClear;
      bool ClearAll;
      int bit;

      bit = 0;
      ClearAll = FALSE;

      if( arg[0] == '+' )
         fClear = TRUE;
      else if( arg[0] == '-' )
         fClear = FALSE;
      else
      {
         send_to_char( "Channels -channel or +channel?\r\n", ch );
         return;
      }

      if( !str_cmp( arg + 1, "auction" ) && get_trust( ch ) > 2 && !NOT_AUTHED( ch ) )
         bit = CHANNEL_AUCTION;
      else if( !str_cmp( arg + 1, "traffic" ) )
         bit = CHANNEL_TRAFFIC;
      else if( !str_cmp( arg + 1, "chat" ) )
         bit = CHANNEL_CHAT;
      else if( !str_cmp( arg + 1, "clan" ) && !IS_NPC( ch ) && ch->pcdata->clan
               && ch->pcdata->clan->clan_type != CLAN_ORDER && ch->pcdata->clan->clan_type != CLAN_GUILD )
         bit = CHANNEL_CLAN;
      else if( !str_cmp( arg + 1, "council" ) && !IS_NPC( ch ) && ch->pcdata->council )
         bit = CHANNEL_COUNCIL;
      else if( !str_cmp( arg + 1, "guild" ) && !IS_NPC( ch ) && ch->pcdata->clan
               && ch->pcdata->clan->clan_type == CLAN_GUILD )
         bit = CHANNEL_GUILD;
      else if( !str_cmp( arg + 1, "quest" ) )
         bit = CHANNEL_QUEST;
      else if( !str_cmp( arg + 1, "tells" ) )
         bit = CHANNEL_TELLS;
      else if( !str_cmp( arg + 1, "immtalk" ) && IS_IMMORTAL( ch ) )
         bit = CHANNEL_IMMTALK;
      else if( !str_cmp( arg + 1, "log" ) && get_trust( ch ) >= sysdata.log_level )
         bit = CHANNEL_LOG;
      else if( !str_cmp( arg + 1, "build" ) && get_trust( ch ) >= sysdata.log_level )
         bit = CHANNEL_BUILD;
      else if( !str_cmp( arg + 1, "high" ) && get_trust( ch ) >= sysdata.log_level
               && get_trust( ch ) >= sysdata.think_level )
         bit = CHANNEL_HIGH;
      /*
       * pray channel is not used for now maybe we should look into activating it
       * even though it would mess up the pray social :P
       */
/*
      else if( !str_cmp( arg + 1, "pray" ) && IS_IMMORTAL( ch ) )
         bit = CHANNEL_PRAY;
*/
      else if( !str_cmp( arg + 1, "avatar" ) && IS_HERO( ch ) )
         bit = CHANNEL_AVTALK;
      else if( !str_cmp( arg + 1, "monitor" ) && IS_IMMORTAL( ch ) )
         bit = CHANNEL_MONITOR;
      else if( !str_cmp( arg + 1, "death" ) )
         bit = CHANNEL_DEATH;
      else if( !str_cmp( arg + 1, "auth" ) && IS_IMMORTAL( ch ) )
         bit = CHANNEL_AUTH;
      else if( !str_cmp( arg + 1, "newbie" ) && ( IS_IMMORTAL( ch )
                                                  || ( ch->pcdata->council
                                                       && !str_cmp( ch->pcdata->council->name, "Newbie Council" ) ) ) )
         bit = CHANNEL_NEWBIE;
      else if( !str_cmp( arg + 1, "music" ) )
         bit = CHANNEL_MUSIC;
      else if( !str_cmp( arg + 1, "muse" ) && IS_IMMORTAL( ch ) && get_trust( ch ) >= sysdata.muse_level )
         bit = CHANNEL_HIGHGOD;
      else if( !str_cmp( arg + 1, "ask" ) )
         bit = CHANNEL_ASK;
      else if( !str_cmp( arg + 1, "shout" ) )
         bit = CHANNEL_SHOUT;
      else if( !str_cmp( arg + 1, "yell" ) )
         bit = CHANNEL_YELL;
      else if( !str_cmp( arg + 1, "comm" ) && get_trust( ch ) >= sysdata.log_level )
         bit = CHANNEL_COMM;
      else if( !str_cmp( arg + 1, "warn" ) && get_trust( ch ) >= sysdata.log_level )
         bit = CHANNEL_WARN;
      else if( !str_cmp( arg + 1, "bug" ) && get_trust( ch ) >= LEVEL_LESSER )
         bit = CHANNEL_BUG;
      else if( !str_cmp( arg + 1, "order" ) && !IS_NPC( ch ) && ch->pcdata->clan
               && ch->pcdata->clan->clan_type == CLAN_ORDER )
         bit = CHANNEL_ORDER;
      else if( !str_cmp( arg + 1, "wartalk" ) )
         bit = CHANNEL_WARTALK;
      else if( !str_cmp( arg + 1, "whisper" ) )
         bit = CHANNEL_WHISPER;
      else if( !str_cmp( arg + 1, "racetalk" ) )
         bit = CHANNEL_RACETALK;
      else if( !str_cmp( arg + 1, "retired" ) )
         bit = CHANNEL_RETIRED;
      else if( !str_cmp( arg + 1, "all" ) )
         ClearAll = TRUE;
      else
      {
         send_to_char( "Set or clear which channel?\r\n", ch );
         return;
      }

      if( ( fClear ) && ( ClearAll ) )
      {
         REMOVE_BIT( ch->deaf, CHANNEL_RACETALK );
         REMOVE_BIT( ch->deaf, CHANNEL_AUCTION );
         REMOVE_BIT( ch->deaf, CHANNEL_CHAT );
         REMOVE_BIT( ch->deaf, CHANNEL_QUEST );
         REMOVE_BIT( ch->deaf, CHANNEL_WARTALK );
         REMOVE_BIT( ch->deaf, CHANNEL_PRAY );
         REMOVE_BIT( ch->deaf, CHANNEL_TRAFFIC );
         REMOVE_BIT( ch->deaf, CHANNEL_MUSIC );
         REMOVE_BIT( ch->deaf, CHANNEL_ASK );
         REMOVE_BIT( ch->deaf, CHANNEL_SHOUT );
         REMOVE_BIT( ch->deaf, CHANNEL_YELL );

         /*
          * if (ch->pcdata->clan)
          * REMOVE_BIT (ch->deaf, CHANNEL_CLAN);
          * 
          * if (ch->pcdata->council)
          * REMOVE_BIT (ch->deaf, CHANNEL_COUNCIL);
          * 
          * if (ch->pcdata->guild)
          * REMOVE_BIT (ch->deaf, CHANNEL_GUILD);
          */
         if( ch->level >= LEVEL_IMMORTAL )
            REMOVE_BIT( ch->deaf, CHANNEL_AVTALK );

         /*
          * if (ch->level >= sysdata.log_level )
          * REMOVE_BIT (ch->deaf, CHANNEL_COMM);
          */

      }
      else if( ( !fClear ) && ( ClearAll ) )
      {
         SET_BIT( ch->deaf, CHANNEL_RACETALK );
         SET_BIT( ch->deaf, CHANNEL_AUCTION );
         SET_BIT( ch->deaf, CHANNEL_TRAFFIC );
         SET_BIT( ch->deaf, CHANNEL_CHAT );
         SET_BIT( ch->deaf, CHANNEL_QUEST );
         SET_BIT( ch->deaf, CHANNEL_PRAY );
         SET_BIT( ch->deaf, CHANNEL_MUSIC );
         SET_BIT( ch->deaf, CHANNEL_ASK );
         SET_BIT( ch->deaf, CHANNEL_SHOUT );
         SET_BIT( ch->deaf, CHANNEL_WARTALK );
         SET_BIT( ch->deaf, CHANNEL_YELL );

         /*
          * if (ch->pcdata->clan)
          * SET_BIT (ch->deaf, CHANNEL_CLAN);
          * 
          * if (ch->pcdata->council)
          * SET_BIT (ch->deaf, CHANNEL_COUNCIL);
          * 
          * if ( IS_GUILDED(ch) )
          * SET_BIT (ch->deaf, CHANNEL_GUILD);
          */
         if( ch->level >= LEVEL_IMMORTAL )
            SET_BIT( ch->deaf, CHANNEL_AVTALK );

         /*
          * if (ch->level >= sysdata.log_level)
          * SET_BIT (ch->deaf, CHANNEL_COMM);
          */

      }
      else if( fClear )
      {
         REMOVE_BIT( ch->deaf, bit );
      }
      else
      {
         SET_BIT( ch->deaf, bit );
      }

      send_to_char( "Ok.\r\n", ch );
   }
   return;
}

/*
 * display WIZLIST file						-Thoric
 */
void do_wizlist( CHAR_DATA* ch, const char* argument)
{
   set_pager_color( AT_IMMORT, ch );
   show_file( ch, WIZLIST_FILE );
}

/*
 * Contributed by Grodyn.
 * Display completely overhauled, 2/97 -- Blodkai
 */
void do_config( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) )
      return;

   one_argument( argument, arg );

   set_char_color( AT_GREEN, ch );

   if( arg[0] == '\0' )
   {
      set_char_color( AT_DGREEN, ch );
      send_to_char( "\r\nConfigurations ", ch );
      set_char_color( AT_GREEN, ch );
      send_to_char( "(use 'config +/- <keyword>' to toggle, see 'help config')\r\n\r\n", ch );
      set_char_color( AT_DGREEN, ch );
      send_to_char( "Display:   ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf( ch, "%-12s   %-12s   %-12s   %-12s\r\n           %-12s   %-12s   %-12s   %-12s\r\n           %-12s   %-12s",
                 IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) ? "[+] PAGER"
                 : "[-] pager",
                 IS_SET( ch->pcdata->flags, PCFLAG_GAG ) ? "[+] GAG"
                 : "[-] gag",
                 xIS_SET( ch->act, PLR_BRIEF ) ? "[+] BRIEF"
                 : "[-] brief",
                 xIS_SET( ch->act, PLR_COMBINE ) ? "[+] COMBINE"
                 : "[-] combine",
                 xIS_SET( ch->act, PLR_BLANK ) ? "[+] BLANK"
                 : "[-] blank",
                 xIS_SET( ch->act, PLR_PROMPT ) ? "[+] PROMPT"
                 : "[-] prompt",
                 xIS_SET( ch->act, PLR_ANSI ) ? "[+] ANSI"
                 : "[-] ansi",
                 xIS_SET( ch->act, PLR_RIP ) ? "[+] RIP"
                 : "[-] rip",
                 xIS_SET( ch->act, PLR_COMPASS ) ? "[+] COMPASS"
                 : "[-] compass", xIS_SET( ch->act, PLR_AUTOMAP ) ? "[+] AUTOMAP" : "[-] automap" );
      set_char_color( AT_DGREEN, ch );
      send_to_char( "\r\n\r\nAuto:      ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf( ch, "%-12s   %-12s   %-12s   %-12s",
                 xIS_SET( ch->act, PLR_AUTOSAC ) ? "[+] AUTOSAC"
                 : "[-] autosac",
                 xIS_SET( ch->act, PLR_AUTOGOLD ) ? "[+] AUTOGOLD"
                 : "[-] autogold",
                 xIS_SET( ch->act, PLR_AUTOLOOT ) ? "[+] AUTOLOOT"
                 : "[-] autoloot", xIS_SET( ch->act, PLR_AUTOEXIT ) ? "[+] AUTOEXIT" : "[-] autoexit" );

      set_char_color( AT_DGREEN, ch );
      send_to_char( "\r\n\r\nSafeties:  ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf( ch, "%-12s   %-12s",
                 IS_SET( ch->pcdata->flags, PCFLAG_NORECALL ) ? "[+] NORECALL"
                 : "[-] norecall", IS_SET( ch->pcdata->flags, PCFLAG_NOSUMMON ) ? "[+] NOSUMMON" : "[-] nosummon" );

      if( !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
         ch_printf( ch, "   %-12s   %-12s",
                    xIS_SET( ch->act, PLR_SHOVEDRAG ) ? "[+] DRAG"
                    : "[-] drag", xIS_SET( ch->act, PLR_NICE ) ? "[+] NICE" : "[-] nice" );

      set_char_color( AT_DGREEN, ch );
      send_to_char( "\r\n\r\nMisc:      ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf( ch, "%-12s   %-12s   %-12s   %-12s",
                 xIS_SET( ch->act, PLR_TELNET_GA ) ? "[+] TELNETGA" : "[-] telnetga",
                 IS_SET( ch->pcdata->flags, PCFLAG_GROUPWHO ) ? "[+] GROUPWHO" : "[-] groupwho",
                 IS_SET( ch->pcdata->flags, PCFLAG_NOINTRO ) ? "[+] NOINTRO" : "[-] nointro",
                 IS_SET( ch->pcdata->flags, PCFLAG_BECKON ) ? "[+] BECKON" : "[-] beckon" );

      set_char_color( AT_DGREEN, ch );
      send_to_char( "\r\n\r\nSettings:  ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf_color( ch, "Pager Length (%d)    Wimpy (&W%d&w)", ch->pcdata->pagerlen, ch->wimpy );

      if( IS_IMMORTAL( ch ) )
      {
         set_char_color( AT_DGREEN, ch );
         send_to_char( "\r\n\r\nImmortal toggles:  ", ch );
         set_char_color( AT_GREY, ch );
         ch_printf( ch, "Roomvnum [%s]", xIS_SET( ch->act, PLR_ROOMVNUM ) ? "+" : " " );
      }

      set_char_color( AT_DGREEN, ch );
      send_to_char( "\r\n\r\nSentences imposed on you (if any):", ch );
      set_char_color( AT_YELLOW, ch );
      ch_printf( ch, "\r\n%s%s%s%s%s%s%s%s",
                 xIS_SET( ch->act, PLR_SILENCE ) ?
                 " For your abuse of channels, you are currently silenced.\r\n" : "",
                 xIS_SET( ch->act, PLR_NO_EMOTE ) ?
                 " The gods have removed your emotes.\r\n" : "",
                 xIS_SET( ch->act, PLR_NO_TELL ) ?
                 " You are not permitted to send 'tells' to others.\r\n" : "",
                 xIS_SET( ch->act, PLR_LITTERBUG ) ?
                 " A convicted litterbug.  You cannot drop anything.\r\n" : "",
                 xIS_SET( ch->act, PLR_THIEF ) ?
                 " A proven thief, you will be hunted by the authorities.\r\n" : "",
                 xIS_SET( ch->act, PLR_KILLER ) ?
                 " For the crime of murder you are sentenced to death...\r\n" : "",
                 IS_SET( ch->pcdata->flags, PCFLAG_NOHOMEPAGE ) ?
                 " You are not permitted to set your homepage.\r\n" : "",
                 IS_SET( ch->pcdata->flags, PCFLAG_NODESC ) ? " You are not permitted to set your description.\r\n" : "" );
   }
   else
   {
      bool fSet;
      int bit = 0;

      if( arg[0] == '+' )
         fSet = TRUE;
      else if( arg[0] == '-' )
         fSet = FALSE;
      else
      {
         send_to_char( "Config -option or +option?\r\n", ch );
         return;
      }

      if( !str_prefix( arg + 1, "autoexit" ) )
         bit = PLR_AUTOEXIT;
      else if( !str_prefix( arg + 1, "autoloot" ) )
         bit = PLR_AUTOLOOT;
      else if( !str_prefix( arg + 1, "autosac" ) )
         bit = PLR_AUTOSAC;
      else if( !str_prefix( arg + 1, "autogold" ) )
         bit = PLR_AUTOGOLD;
      else if( !str_prefix( arg + 1, "blank" ) )
         bit = PLR_BLANK;
      else if( !str_prefix( arg + 1, "brief" ) )
         bit = PLR_BRIEF;
      else if( !str_prefix( arg + 1, "combine" ) )
         bit = PLR_COMBINE;
      else if( !str_prefix( arg + 1, "prompt" ) )
         bit = PLR_PROMPT;
      else if( !str_prefix( arg + 1, "telnetga" ) )
         bit = PLR_TELNET_GA;
      else if( !str_prefix( arg + 1, "ansi" ) )
         bit = PLR_ANSI;
      else if( !str_prefix( arg + 1, "rip" ) )
         bit = PLR_RIP;
      else if( !str_prefix( arg + 1, "compass" ) )
         bit = PLR_COMPASS;
      else if( !str_prefix( arg + 1, "automap" ) )
         bit = PLR_AUTOMAP;
      else if( !str_prefix( arg + 1, "nice" ) )
         bit = PLR_NICE;
      else if( !str_prefix( arg + 1, "drag" ) )
         bit = PLR_SHOVEDRAG;
      else if( IS_IMMORTAL( ch ) && !str_prefix( arg + 1, "vnum" ) )
         bit = PLR_ROOMVNUM;

      if( bit )
      {
         if( ( bit == PLR_FLEE || bit == PLR_NICE || bit == PLR_SHOVEDRAG ) && IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
         {
            send_to_char( "Pkill characters can not config that option.\r\n", ch );
            return;
         }

         if( fSet )
            xSET_BIT( ch->act, bit );
         else
            xREMOVE_BIT( ch->act, bit );
         send_to_char( "Ok.\r\n", ch );
         return;
      }
      else
      {
         if( !str_prefix( arg + 1, "norecall" ) )
            bit = PCFLAG_NORECALL;
         else if( !str_prefix( arg + 1, "nointro" ) )
            bit = PCFLAG_NOINTRO;
         else if( !str_prefix( arg + 1, "beckon" ) )
            bit = PCFLAG_BECKON;
         else if( !str_prefix( arg + 1, "nosummon" ) )
            bit = PCFLAG_NOSUMMON;
         else if( !str_prefix( arg + 1, "gag" ) )
            bit = PCFLAG_GAG;
         else if( !str_prefix( arg + 1, "pager" ) )
            bit = PCFLAG_PAGERON;
         else if( !str_prefix( arg + 1, "groupwho" ) )
            bit = PCFLAG_GROUPWHO;
         else if( !str_prefix( arg + 1, "@hgflag_" ) )
            bit = PCFLAG_HIGHGAG;
         else
         {
            send_to_char( "Config which option?\r\n", ch );
            return;
         }

         if( fSet )
            SET_BIT( ch->pcdata->flags, bit );
         else
            REMOVE_BIT( ch->pcdata->flags, bit );

         send_to_char( "Ok.\r\n", ch );
         return;
      }
   }
   return;
}

void do_credits( CHAR_DATA* ch, const char* argument)
{
   do_help( ch, "credits" );
}

extern int top_area;

/*
 * New do_areas, written by Fireblade, last modified - 4/27/97
 *
 *   Syntax: area            ->      lists areas in alphanumeric order
 *           area <a>        ->      lists areas with soft max less than
 *                                                    parameter a
 *           area <a> <b>    ->      lists areas with soft max bewteen
 *                                                    numbers a and b
 *           area old        ->      list areas in order loaded
 *
 */
void do_areas( CHAR_DATA* ch, const char* argument)
{
   const char *header_string1 = "\r\n   Author    |             Area" "                     | " "Recommended |  Enforced\r\n";
   const char *header_string2 = "-------------+-----------------" "---------------------+----" "---------+-----------\r\n";
   const char *print_string = "%-12s | %-36s | %4d - %-4d | %3d - " "%-3d \r\n";

   AREA_DATA *pArea;
   int lower_bound = 0;
   int upper_bound = MAX_LEVEL + 1;
   /*
    * make sure is to init. > max area level 
    */
   char arg[MAX_STRING_LENGTH];

   argument = one_argument( argument, arg );

   if( arg[0] != '\0' )
   {
      if( !is_number( arg ) )
      {
         if( !strcmp( arg, "old" ) )
         {
            set_pager_color( AT_PLAIN, ch );
            send_to_pager( header_string1, ch );
            send_to_pager( header_string2, ch );
            for( pArea = first_area; pArea; pArea = pArea->next )
            {
               if( IS_SET( pArea->flags, AFLAG_HIDDEN ) ) /* Blod, 2000 */
                  continue;

               pager_printf( ch, print_string,
                             pArea->author, pArea->name,
                             pArea->low_soft_range, pArea->hi_soft_range, pArea->low_hard_range, pArea->hi_hard_range );
            }
            return;
         }
         else
         {
            send_to_char( "Area may only be followed by numbers, or 'old'.\r\n", ch );
            return;
         }
      }

      upper_bound = atoi( arg );
      lower_bound = upper_bound;

      argument = one_argument( argument, arg );

      if( arg[0] != '\0' )
      {
         if( !is_number( arg ) )
         {
            send_to_char( "Area may only be followed by numbers.\r\n", ch );
            return;
         }

         upper_bound = atoi( arg );

         argument = one_argument( argument, arg );
         if( arg[0] != '\0' )
         {
            send_to_char( "Only two level numbers allowed.\r\n", ch );
            return;
         }
      }
   }

   if( lower_bound > upper_bound )
   {
      int swap = lower_bound;
      lower_bound = upper_bound;
      upper_bound = swap;
   }

   set_pager_color( AT_PLAIN, ch );
   send_to_pager( header_string1, ch );
   send_to_pager( header_string2, ch );

   for( pArea = first_area_name; pArea; pArea = pArea->next_sort_name )
   {
      if( IS_SET( pArea->flags, AFLAG_HIDDEN ) ) /* Blod, 2000 */
         continue;

      if( pArea->hi_soft_range >= lower_bound && pArea->low_soft_range <= upper_bound )
      {
         pager_printf( ch, print_string,
                       pArea->author, pArea->name,
                       pArea->low_soft_range, pArea->hi_soft_range, pArea->low_hard_range, pArea->hi_hard_range );
      }
   }
}

void do_afk( CHAR_DATA* ch, const char* argument)
{
   if( IS_NPC( ch ) )
      return;

   if( xIS_SET( ch->act, PLR_AFK ) )
   {
      xREMOVE_BIT( ch->act, PLR_AFK );
      send_to_char( "You are no longer afk.\r\n", ch );
      act( AT_GREY, "$n is no longer afk.", ch, NULL, NULL, TO_CANSEE );
   }
   else
   {
      xSET_BIT( ch->act, PLR_AFK );
      send_to_char( "You are now afk.\r\n", ch );
      act( AT_GREY, "$n is now afk.", ch, NULL, NULL, TO_CANSEE );
      return;
   }
}

void do_slist( CHAR_DATA* ch, const char* argument)
{
   int sn, i, lFound;
   char skn[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int lowlev, hilev;
   short lasttype = SKILL_SPELL;

   if( IS_NPC( ch ) )
      return;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   lowlev = 1;
   hilev = LEVEL_AVATAR;

   if( arg1[0] != '\0' )
      lowlev = atoi( arg1 );

   if( ( lowlev < 1 ) || ( lowlev > LEVEL_IMMORTAL ) )
      lowlev = 1;

   if( arg2[0] != '\0' )
      hilev = atoi( arg2 );

   if( ( hilev < 0 ) || ( hilev >= LEVEL_IMMORTAL ) )
      hilev = LEVEL_HERO;

   if( lowlev > hilev )
      lowlev = hilev;

   set_pager_color( AT_MAGIC, ch );
   send_to_pager( "SPELL & SKILL LIST\r\n", ch );
   send_to_pager( "------------------\r\n", ch );

   for( i = lowlev; i <= hilev; i++ )
   {
      lFound = 0;
      snprintf( skn, MAX_INPUT_LENGTH, "%s", "Spell" );
      for( sn = 0; sn < num_skills; ++sn )
      {
         const SKILLTYPE *skill;
         int normalSn;

         // the first num_sorted_skills are sorted by type, so we don't want
         // to index into skill_table -- that is sorted alphabetically -- so
         // we need to do the following dance to check if we are in the 
         // sorted section or the unsorted section.
         if( sn < num_sorted_skills )
         {
            skill = skill_table_bytype[sn];

            // we are looping over the skills sorted by type.
            // So, we need to get the normal sn as well.
            normalSn = skill_lookup( skill->name );
         }
         else
         {
            skill = skill_table[sn];
            normalSn = sn;
         }

         if( !skill || !skill->name || skill->name[0] == '\0' )
            continue;

         if( skill->type != lasttype )
         {
            lasttype = skill->type;
            mudstrlcpy( skn, skill_tname[lasttype], MAX_INPUT_LENGTH );
         }

         if( ch->pcdata->learned[normalSn] <= 0 && SPELL_FLAG( skill, SF_SECRETSKILL ) )
            continue;

         if( i == skill->skill_level[ch->Class] || i == skill->race_level[ch->race] )
         {
            if( !lFound )
            {
               lFound = 1;
               pager_printf( ch, "Level %d\r\n", i );
            }

            switch ( skill->minimum_position )
            {
               default:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "Invalid" );
                  bug( "%s: skill with invalid minpos, skill=%s", __func__, skill->name );
                  break;

               case POS_DEAD:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "any" );
                  break;

               case POS_MORTAL:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "mortally wounded" );
                  break;

               case POS_INCAP:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "incapacitated" );
                  break;

               case POS_STUNNED:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "stunned" );
                  break;

               case POS_SLEEPING:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "sleeping" );
                  break;

               case POS_RESTING:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "resting" );
                  break;

               case POS_STANDING:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "standing" );
                  break;

               case POS_FIGHTING:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "fighting" );
                  break;

               case POS_EVASIVE:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "fighting (evasive)" ); /* Fighting style support -haus */
                  break;

               case POS_DEFENSIVE:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "fighting (defensive)" );
                  break;

               case POS_AGGRESSIVE:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "fighting (aggressive)" );
                  break;

               case POS_BERSERK:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "fighting (berserk)" );
                  break;
               case POS_MOUNTED:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "mounted" );
                  break;

               case POS_SITTING:
                  snprintf( buf, MAX_INPUT_LENGTH, "%s", "sitting" );
                  break;
            }

            pager_printf( ch, "%7s: %20.20s \t Current: %-3d Max: %-3d  MinPos: %s \r\n",
                          skn, skill->name, ch->pcdata->learned[normalSn], skill->skill_adept[ch->Class], buf );
         }
      }
   }
   return;
}

void do_whois( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;
   CLAN_DATA *pclan;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];

   buf[0] = '\0';

   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' )
   {
      send_to_pager( "You must input the name of an online character.\r\n", ch );
      return;
   }

   mudstrlcat( buf, "0.", MAX_STRING_LENGTH );
   mudstrlcat( buf, argument, MAX_STRING_LENGTH );
   if( ( ( victim = get_char_world( ch, buf ) ) == NULL ) )
   {
      send_to_pager( "No such character online.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_pager( "That's not a player!\r\n", ch );
      return;
   }

   set_pager_color( AT_GREY, ch );
   pager_printf( ch, "\r\n'%s%s.'\r\n %s is a %s level %d %s %s, %d years of age.\r\n",
                 victim->name,
                 victim->pcdata->title,
                 victim->sex == SEX_MALE ? "He" :
                 victim->sex == SEX_FEMALE ? "She" : "It",
                 victim->sex == SEX_MALE ? "male" :
                 victim->sex == SEX_FEMALE ? "female" : "neutral",
                 victim->level,
                 capitalize( race_table[victim->race]->race_name ),
                 class_table[victim->Class]->who_name, calculate_age( victim ) );

   pager_printf( ch, " %s is a %sdeadly player",
                 victim->sex == SEX_MALE ? "He" :
                 victim->sex == SEX_FEMALE ? "She" : "It", IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) ? "" : "non-" );

   if( ( pclan = victim->pcdata->clan ) != NULL )
   {
      switch( pclan->clan_type )
      {
         default:
            if( !str_cmp( victim->name, pclan->deity ) )
               send_to_pager( ", and is Deity of ", ch );
            else if( !str_cmp( victim->name, pclan->leader ) )
               send_to_pager( ", and is Leader of ", ch );
            else if( !str_cmp( victim->name, pclan->number1 ) )
               send_to_pager( ", and is Number One of ", ch );
            else if( !str_cmp( victim->name, pclan->number2 ) )
               send_to_pager( ", and is Number Two of ", ch );
            else
               send_to_pager( ", and belongs to Clan ", ch );
            break;

         case CLAN_ORDER:
            if( !str_cmp( victim->name, pclan->deity ) )
               send_to_pager( ", and is Deity of the Order of ", ch );
            else if( !str_cmp( victim->name, pclan->leader ) )
               send_to_pager( ", and is Leader of the Order of ", ch );
            else if( !str_cmp( victim->name, pclan->number1 ) )
               send_to_pager( ", and is Number One of the Order of ", ch );
            else if( !str_cmp( victim->name, pclan->number2 ) )
               send_to_pager( ", and is Number Two of the Order of ", ch );
            else
               send_to_pager( ", and belongs to the Order of ", ch );
            break;

         case CLAN_GUILD:
            if( !str_cmp( victim->name, pclan->leader ) )
               pager_printf( ch, ", and is the %s of the ",
                  victim->sex == SEX_FEMALE ? "Guildmistress" : "Guildmaster" );
            else if( !str_cmp( victim->name, pclan->number1 ) )
               send_to_pager( ", and is First of the ", ch );
            else if( !str_cmp( victim->name, pclan->number2 ) )
               send_to_pager( ", and is Second of the ", ch );
            else
               send_to_pager( ", and belongs to the ", ch );
            break;
      }
      send_to_pager( pclan->name, ch );
   }
   send_to_pager( ".\r\n", ch );

   if( victim->pcdata->council )
   {
      if( !str_cmp( victim->name, victim->pcdata->council->head ) )
         pager_printf( ch, " %s is the %s of:  %s\r\n",
            victim->sex == SEX_MALE ? "He" :
            victim->sex == SEX_FEMALE ? "She" : "It",
            victim->pcdata->council->head2 == NULL ? "Head" : "Co-Head",
            victim->pcdata->council->name );
      else if( victim->pcdata->council->head2 && !str_cmp ( victim->name, victim->pcdata->council->head2 ) )
         pager_printf( ch, " %s is the Co-Head of:  %s\r\n",
            victim->sex == SEX_MALE ? "He" :
            victim->sex == SEX_FEMALE ? "She" : "It",
            victim->pcdata->council->name );
      else
         pager_printf( ch, " %s holds a seat on:  %s\r\n",
            victim->sex == SEX_MALE ? "He" :
            victim->sex == SEX_FEMALE ? "She" : "It",
            victim->pcdata->council->name );
   }

   if( victim->pcdata->deity )
      pager_printf( ch, " %s has found succor in the deity %s.\r\n",
                    victim->sex == SEX_MALE ? "He" : victim->sex == SEX_FEMALE ? "She" : "It", victim->pcdata->deity->name );

   if( victim->pcdata->homepage && victim->pcdata->homepage[0] != '\0' )
      pager_printf( ch, " %s homepage can be found at %s\r\n",
                    victim->sex == SEX_MALE ? "His" :
                    victim->sex == SEX_FEMALE ? "Her" : "Its", show_tilde( victim->pcdata->homepage ) );

   if( victim->pcdata->bio && victim->pcdata->bio[0] != '\0' )
      pager_printf( ch, " %s's personal bio:\r\n%s", victim->name, victim->pcdata->bio );
   else
      pager_printf( ch, " %s has yet to create a bio.\r\n", victim->name );

   if( IS_IMMORTAL( ch ) )
   {
      send_to_pager( "-------------------\r\n", ch );
      send_to_pager( "Info for immortals:\r\n", ch );

      if( victim->pcdata->authed_by && victim->pcdata->authed_by[0] != '\0' )
         pager_printf( ch, "%s was authorized by %s.\r\n", victim->name, victim->pcdata->authed_by );

      pager_printf( ch, "%s has killed %d mobiles, and been killed by a mobile %d times.\r\n",
                    victim->name, victim->pcdata->mkills, victim->pcdata->mdeaths );
      if( victim->pcdata->pkills || victim->pcdata->pdeaths )
         pager_printf( ch, "%s has killed %d players, and been killed by a player %d times.\r\n",
                       victim->name, victim->pcdata->pkills, victim->pcdata->pdeaths );
      if( victim->pcdata->illegal_pk )
         pager_printf( ch, "%s has committed %d illegal player kills.\r\n", victim->name, victim->pcdata->illegal_pk );

      pager_printf( ch, "%s is %shelled at the moment.\r\n",
                    victim->name, ( victim->pcdata->release_date == 0 ) ? "not " : "" );

      if( victim->pcdata->nuisance )
      {
         pager_printf_color( ch, "&RNuisance   &cStage: (&R%d&c/%d)  Power:  &w%d  &cTime:  &w%s.\r\n",
                             victim->pcdata->nuisance->flags, MAX_NUISANCE_STAGE, victim->pcdata->nuisance->power,
                             ctime( &victim->pcdata->nuisance->set_time ) );
      }
      if( victim->pcdata->release_date != 0 )
         pager_printf( ch, "%s was helled by %s, and will be released on %24.24s.\r\n",
                       victim->sex == SEX_MALE ? "He" :
                       victim->sex == SEX_FEMALE ? "She" : "It",
                       victim->pcdata->helled_by, ctime( &victim->pcdata->release_date ) );

      if( get_trust( victim ) < get_trust( ch ) )
      {
         snprintf( buf2, MAX_STRING_LENGTH, "list %s", buf );
         do_comment( ch, buf2 );
      }

      if( xIS_SET( victim->act, PLR_SILENCE ) || xIS_SET( victim->act, PLR_FREEZE ) || xIS_SET( victim->act, PLR_NO_EMOTE )
         || xIS_SET( victim->act, PLR_NO_TELL ) || IS_SET( victim->pcdata->flags, PCFLAG_NOBECKON ) || IS_SET( victim->pcdata->flags, PCFLAG_NOTITLE )
         || xIS_SET( victim->act, PLR_THIEF ) || xIS_SET( victim->act, PLR_KILLER ) || xIS_SET( victim->act, PLR_LITTERBUG )
         || IS_SET( victim->pcdata->flags, PCFLAG_NODESC ) || IS_SET( victim->pcdata->flags, PCFLAG_NOBIO ) || IS_SET( victim->pcdata->flags, PCFLAG_NOHOMEPAGE ) )
      {
         mudstrlcat( buf2, "&GThis player has the following sanctions: &Y", MAX_STRING_LENGTH );
         if( xIS_SET( victim->act, PLR_SILENCE ) )
            mudstrlcat( buf2, " silence", MAX_STRING_LENGTH );
         if( xIS_SET( victim->act, PLR_FREEZE ) )
            mudstrlcat( buf2, " frozen", MAX_STRING_LENGTH );
         if( xIS_SET( victim->act, PLR_NO_EMOTE ) )
            mudstrlcat( buf2, " noemote", MAX_STRING_LENGTH );
         if( xIS_SET( victim->act, PLR_NO_TELL ) )
            mudstrlcat( buf2, " notell", MAX_STRING_LENGTH );
         if( IS_SET( victim->pcdata->flags, PCFLAG_NOBECKON ) )
            mudstrlcat( buf2, " nobeckon", MAX_STRING_LENGTH );
         if( IS_SET( victim->pcdata->flags, PCFLAG_NOTITLE ) )
            mudstrlcat( buf2, " notitle", MAX_STRING_LENGTH );
         if( IS_SET( victim->pcdata->flags, PCFLAG_NOBIO ) )
            mudstrlcat( buf2, " nobio", MAX_STRING_LENGTH );
         if( IS_SET( victim->pcdata->flags, PCFLAG_NODESC ) )
            mudstrlcat( buf2, " nodesc", MAX_STRING_LENGTH );
         if( xIS_SET( victim->act, PLR_THIEF ) )
            mudstrlcat( buf2, " thief", MAX_STRING_LENGTH );
         if( xIS_SET( victim->act, PLR_KILLER ) )
            mudstrlcat( buf2, " killer", MAX_STRING_LENGTH );
         if( xIS_SET( victim->act, PLR_LITTERBUG ) )
            mudstrlcat( buf2, " litterbug", MAX_STRING_LENGTH );
         if( IS_SET( victim->pcdata->flags, PCFLAG_NOHOMEPAGE ) )
            mudstrlcat( buf2, " nohomepage", MAX_STRING_LENGTH );
         mudstrlcat( buf2, ".\r\n", MAX_STRING_LENGTH );
         send_to_pager( buf2, ch );
      }
      if( victim->desc && victim->desc->host[0] != '\0' )   /* added by Gorog */
      {
         pager_printf( ch, "%s's IP info: %s\r\n", victim->name, victim->desc->host );
      }
      else if( victim->pcdata->recent_site )
      {
         pager_printf( ch, "%s's most recent IP: %s\r\n ", victim->name, victim->pcdata->recent_site );
      }
   }
}

void do_pager( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) )
      return;
   set_char_color( AT_NOTE, ch );
   argument = one_argument( argument, arg );
   if( !*arg )
   {
      if( IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) )
      {
         send_to_char( "Pager disabled.\r\n", ch );
         do_config( ch, "-pager" );
      }
      else
      {
         ch_printf( ch, "Pager is now enabled at %d lines.\r\n", ch->pcdata->pagerlen );
         do_config( ch, "+pager" );
      }
      return;
   }
   if( !is_number( arg ) )
   {
      send_to_char( "Set page pausing to how many lines?\r\n", ch );
      return;
   }
   ch->pcdata->pagerlen = atoi( arg );
   if( ch->pcdata->pagerlen < 5 )
      ch->pcdata->pagerlen = 5;
   ch_printf( ch, "Page pausing set to %d lines.\r\n", ch->pcdata->pagerlen );
   return;
}

/*
 * The ignore command allows players to ignore up to MAX_IGN
 * other players. Players may ignore other characters whether
 * they are online or not. This is to prevent people from
 * spamming someone and then logging off quickly to evade
 * being ignored.
 * Syntax:
 *	ignore		-	lists players currently ignored
 *	ignore none	-	sets it so no players are ignored
 *	ignore <player>	-	start ignoring player if not already
 *				ignored otherwise stop ignoring player
 *	ignore reply	-	start ignoring last player to send a
 *				tell to ch, to deal with invis spammers
 * Last Modified: June 26, 1997
 * - Fireblade
 */
void do_ignore( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   IGNORE_DATA *temp, *next;
   char fname[1024];
   struct stat fst;
   CHAR_DATA *victim;

   if( IS_NPC( ch ) )
      return;

   argument = one_argument( argument, arg );

   snprintf( fname, 1024, "%s%c/%s", PLAYER_DIR, tolower( arg[0] ), capitalize( arg ) );

   victim = NULL;

   /*
    * If no arguements, then list players currently ignored 
    */
   if( arg[0] == '\0' )
   {
      set_char_color( AT_DIVIDER, ch );
      ch_printf( ch, "\r\n----------------------------------------\r\n" );
      set_char_color( AT_DGREEN, ch );
      ch_printf( ch, "You are currently ignoring:\r\n" );
      set_char_color( AT_DIVIDER, ch );
      ch_printf( ch, "----------------------------------------\r\n" );
      set_char_color( AT_IGNORE, ch );

      if( !ch->pcdata->first_ignored )
      {
         ch_printf( ch, "\t    no one\r\n" );
         return;
      }

      for( temp = ch->pcdata->first_ignored; temp; temp = temp->next )
      {
         ch_printf( ch, "\t  - %s\r\n", temp->name );
      }

      return;
   }
   /*
    * Clear players ignored if given arg "none" 
    */
   else if( !strcmp( arg, "none" ) )
   {
      for( temp = ch->pcdata->first_ignored; temp; temp = next )
      {
         next = temp->next;
         UNLINK( temp, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev );
         STRFREE( temp->name );
         DISPOSE( temp );
      }

      set_char_color( AT_IGNORE, ch );
      ch_printf( ch, "You now ignore no one.\r\n" );

      return;
   }
   /*
    * Prevent someone from ignoring themself... 
    */
   else if( !strcmp( arg, "self" ) || nifty_is_name( arg, ch->name ) )
   {
      set_char_color( AT_IGNORE, ch );
      ch_printf( ch, "Did you type something?\r\n" );
      return;
   }
   else
   {
      int i;

      /*
       * get the name of the char who last sent tell to ch 
       */
      if( !strcmp( arg, "reply" ) )
      {
         if( !ch->reply )
         {
            set_char_color( AT_IGNORE, ch );
            ch_printf( ch, "They're not here.\r\n" );
            return;
         }
         else
         {
            mudstrlcpy( arg, ch->reply->name, MAX_INPUT_LENGTH );
         }
      }

      /*
       * Loop through the linked list of ignored players 
       */
      /*
       * keep track of how many are being ignored     
       */
      for( temp = ch->pcdata->first_ignored, i = 0; temp; temp = temp->next, i++ )
      {
         /*
          * If the argument matches a name in list remove it 
          */
         if( !strcmp( temp->name, capitalize( arg ) ) )
         {
            UNLINK( temp, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev );
            set_char_color( AT_IGNORE, ch );
            ch_printf( ch, "You no longer ignore %s.\r\n", temp->name );
            STRFREE( temp->name );
            DISPOSE( temp );
            return;
         }
      }

      /*
       * if there wasn't a match check to see if the name   
       * is valid. This if-statement may seem like overkill 
       * but it is intended to prevent people from doing the
       * spam and log thing while still allowing ya to      
       * ignore new chars without pfiles yet...             
       */
      if( stat( fname, &fst ) == -1 &&
          ( !( victim = get_char_world( ch, arg ) ) || IS_NPC( victim ) || strcmp( capitalize( arg ), victim->name ) != 0 ) )
      {
         set_char_color( AT_IGNORE, ch );
         ch_printf( ch, "No player exists by that" " name.\r\n" );
         return;
      }

      if( victim )
      {
         mudstrlcpy( capitalize( arg ), victim->name, MAX_INPUT_LENGTH );
      }

      if( !check_parse_name( capitalize( arg ), TRUE ) )
      {
         set_char_color( AT_IGNORE, ch );
         send_to_char( "No player exists by that name.\r\n", ch );
         return;
      }

      /*
       * If its valid and the list size limit has not been 
       * reached create a node and at it to the list       
       */
      if( i < MAX_IGN )
      {
         IGNORE_DATA *inew;
         CREATE( inew, IGNORE_DATA, 1 );
         inew->name = STRALLOC( capitalize( arg ) );
         inew->next = NULL;
         inew->prev = NULL;
         LINK( inew, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev );
         set_char_color( AT_IGNORE, ch );
         ch_printf( ch, "You now ignore %s.\r\n", inew->name );
         return;
      }
      else
      {
         set_char_color( AT_IGNORE, ch );
         ch_printf( ch, "You may only ignore %d players.\r\n", MAX_IGN );
         return;
      }
   }
}

/*
 * This function simply checks to see if ch is ignoring ign_ch.
 * Last Modified: October 10, 1997
 * - Fireblade
 */
bool is_ignoring( CHAR_DATA * ch, CHAR_DATA * ign_ch )
{
   IGNORE_DATA *temp;

   if( IS_NPC( ch ) || IS_NPC( ign_ch ) )
      return FALSE;

   for( temp = ch->pcdata->first_ignored; temp; temp = temp->next )
   {
      if( nifty_is_name( temp->name, ign_ch->name ) )
         return TRUE;
   }

   return FALSE;
}

/* Version info -- Scryn */
void do_version( CHAR_DATA* ch, const char* argument)
{
   if( IS_NPC( ch ) )
      return;

   set_char_color( AT_YELLOW, ch );
   ch_printf( ch, "SMAUG %s.%s\r\n", SMAUG_VERSION_MAJOR, SMAUG_VERSION_MINOR );

   if( IS_IMMORTAL( ch ) )
      ch_printf( ch, "Compiled on %s at %s.\r\n", __DATE__, __TIME__ );

   return;
}
