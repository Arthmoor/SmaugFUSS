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
 *             Commands for personal player settings/statictics             *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "mud.h"

/*
 *  Locals
 */
const char *tiny_affect_loc_name( int location );

void do_gold( CHAR_DATA* ch, const char* argument)
{
   set_char_color( AT_GOLD, ch );
   ch_printf( ch, "You have %s gold pieces.\r\n", num_punct( ch->gold ) );
   return;
}

void do_worth( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
      return;

   set_pager_color( AT_SCORE, ch );
   pager_printf( ch, "\r\nWorth for %s%s.\r\n", ch->name, ch->pcdata->title );
   send_to_pager( " ----------------------------------------------------------------------------\r\n", ch );
   if( !ch->pcdata->deity )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "N/A" );
   else if( ch->pcdata->favor > 2250 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "loved" );
   else if( ch->pcdata->favor > 2000 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "cherished" );
   else if( ch->pcdata->favor > 1750 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "honored" );
   else if( ch->pcdata->favor > 1500 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "praised" );
   else if( ch->pcdata->favor > 1250 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "favored" );
   else if( ch->pcdata->favor > 1000 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "respected" );
   else if( ch->pcdata->favor > 750 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "liked" );
   else if( ch->pcdata->favor > 250 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "tolerated" );
   else if( ch->pcdata->favor > -250 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "ignored" );
   else if( ch->pcdata->favor > -750 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "shunned" );
   else if( ch->pcdata->favor > -1000 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "disliked" );
   else if( ch->pcdata->favor > -1250 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "dishonored" );
   else if( ch->pcdata->favor > -1500 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "disowned" );
   else if( ch->pcdata->favor > -1750 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "abandoned" );
   else if( ch->pcdata->favor > -2000 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "despised" );
   else if( ch->pcdata->favor > -2250 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "hated" );
   else
      snprintf( buf, MAX_STRING_LENGTH, "%s", "damned" );

   if( ch->level < 10 )
   {
      if( ch->alignment > 900 )
         snprintf( buf2, MAX_STRING_LENGTH, "%s", "devout" );
      else if( ch->alignment > 700 )
         snprintf( buf2, MAX_STRING_LENGTH, "%s", "noble" );
      else if( ch->alignment > 350 )
         snprintf( buf2, MAX_STRING_LENGTH, "%s", "honorable" );
      else if( ch->alignment > 100 )
         snprintf( buf2, MAX_STRING_LENGTH, "%s", "worthy" );
      else if( ch->alignment > -100 )
         snprintf( buf2, MAX_STRING_LENGTH, "%s", "neutral" );
      else if( ch->alignment > -350 )
         snprintf( buf2, MAX_STRING_LENGTH, "%s", "base" );
      else if( ch->alignment > -700 )
         snprintf( buf2, MAX_STRING_LENGTH, "%s", "evil" );
      else if( ch->alignment > -900 )
         snprintf( buf2, MAX_STRING_LENGTH, "%s", "ignoble" );
      else
         snprintf( buf2, MAX_STRING_LENGTH, "%s", "fiendish" );
   }
   else
      snprintf( buf2, MAX_STRING_LENGTH, "%d", ch->alignment );
   pager_printf( ch, "|Level: %-4d |Favor: %-10s |Alignment: %-9s |Experience: %-9d|\r\n", ch->level, buf, buf2, ch->exp );
   send_to_pager( " ----------------------------------------------------------------------------\r\n", ch );
   switch ( ch->style )
   {
      case STYLE_EVASIVE:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "evasive" );
         break;
      case STYLE_DEFENSIVE:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "defensive" );
         break;
      case STYLE_AGGRESSIVE:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "aggressive" );
         break;
      case STYLE_BERSERK:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "berserk" );
         break;
      default:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "standard" );
         break;
   }
   pager_printf( ch, "|Glory: %-4d |Weight: %-9d |Style: %-13s |Gold: %-14s |\r\n",
                 ch->pcdata->quest_curr, ch->carry_weight, buf, num_punct( ch->gold ) );
   send_to_pager( " ----------------------------------------------------------------------------\r\n", ch );
   if( ch->level < 15 && !IS_PKILL( ch ) )
      pager_printf( ch, "|            |Hitroll: -------- |Damroll: ----------- |                     |\r\n" );
   else
      pager_printf( ch, "|            |Hitroll: %-8d |Damroll: %-11d |                     |\r\n", GET_HITROLL( ch ),
                    GET_DAMROLL( ch ) );
   send_to_pager( " ----------------------------------------------------------------------------\r\n", ch );
   return;
}

/*
 * New score command by Haus
 */
void do_score( CHAR_DATA* ch, const char* argument )
{
   char buf[MAX_STRING_LENGTH];
   AFFECT_DATA *paf;
   int iLang;
   /*const char *suf;
   short day;

   day = ch->pcdata->day + 1;

   if( day > 4 && day < 20 )
      suf = "th";
   else if( day % 10 == 1 )
      suf = "st";
   else if( day % 10 == 2 )
      suf = "nd";
   else if( day % 10 == 3 )
      suf = "rd";
   else
      suf = "th";
   * - Uncomment this if you want Birthdays dispayed on score for players - Kayle 1/22/08
   */

   set_pager_color( AT_SCORE, ch );

   pager_printf( ch, "\r\nScore for %s%s.\r\n", ch->name, IS_NPC(ch) ? "" : ch->pcdata->title );
   if( get_trust( ch ) != ch->level )
      pager_printf( ch, "You are trusted at level %d.\r\n", get_trust( ch ) );

   send_to_pager( "----------------------------------------------------------------------------\r\n", ch );

   /*if( time_info.day == ch->pcdata->day && time_info.month == ch->pcdata->month )
      send_to_char( "Today is your birthday!\r\n", ch );
   else
      ch_printf( ch, "Your birthday is: Day of %s, %d%s day in the Month of %s, in the year %d.\r\n",
                 day_name[ch->pcdata->day % sysdata.daysperweek], day, suf, month_name[ch->pcdata->month], ch->pcdata->year );
   send_to_pager( "----------------------------------------------------------------------------\r\n", ch );
   * - Uncomment this if you want players to see their birthday's on score. - Kayle 1/22/08
   */

   pager_printf( ch, "LEVEL: %-3d         Race : %-10.10s        Played: %ld hours\r\n",
                 ch->level, capitalize( get_race( ch ) ), ( long int )GET_TIME_PLAYED( ch ) );

   pager_printf( ch, "YEARS: %-6d      Class: %-11.11s       Log In: %s\r",
                 calculate_age( ch ), capitalize( get_class( ch ) ), ctime( &( ch->logon ) ) );

   if( ch->level >= 15 || IS_PKILL( ch ) )
   {
      pager_printf( ch, "STR  : %2.2d(%2.2d)    HitRoll: %-4d              Saved:  %s\r",
                    get_curr_str( ch ), ch->perm_str, GET_HITROLL( ch ),
                    ch->save_time ? ctime( &( ch->save_time ) ) : "no save this session\n" );

      pager_printf( ch, "INT  : %2.2d(%2.2d)    DamRoll: %-4d              Time:   %s\r",
                    get_curr_int( ch ), ch->perm_int, GET_DAMROLL( ch ), ctime( &current_time ) );
   }
   else
   {
      pager_printf( ch, "STR  : %2.2d(%2.2d)                               Saved:  %s\r",
                    get_curr_str( ch ), ch->perm_str, ch->save_time ? ctime( &( ch->save_time ) ) : "no\n" );

      pager_printf( ch, "INT  : %2.2d(%2.2d)                               Time:   %s\r",
                    get_curr_int( ch ), ch->perm_int, ctime( &current_time ) );
   }

   if( GET_AC( ch ) >= 101 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "the rags of a beggar" );
   else if( GET_AC( ch ) >= 80 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "improper for adventure" );
   else if( GET_AC( ch ) >= 55 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "shabby and threadbare" );
   else if( GET_AC( ch ) >= 40 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "of poor quality" );
   else if( GET_AC( ch ) >= 20 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "scant protection" );
   else if( GET_AC( ch ) >= 10 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "that of a knave" );
   else if( GET_AC( ch ) >= 0 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "moderately crafted" );
   else if( GET_AC( ch ) >= -10 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "well crafted" );
   else if( GET_AC( ch ) >= -20 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "the envy of squires" );
   else if( GET_AC( ch ) >= -40 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "excellently crafted" );
   else if( GET_AC( ch ) >= -60 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "the envy of knights" );
   else if( GET_AC( ch ) >= -80 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "the envy of barons" );
   else if( GET_AC( ch ) >= -100 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "the envy of dukes" );
   else if( GET_AC( ch ) >= -200 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "the envy of emperors" );
   else
      snprintf( buf, MAX_STRING_LENGTH, "%s", "that of an avatar" );
   if( ch->level > 24 )
      pager_printf( ch, "WIS  : %2.2d(%2.2d)      Armor: %4.4d, %s\r\n",
                    get_curr_wis( ch ), ch->perm_wis, GET_AC( ch ), buf );
   else
      pager_printf( ch, "WIS  : %2.2d(%2.2d)      Armor: %s \r\n", get_curr_wis( ch ), ch->perm_wis, buf );

   if( ch->alignment > 900 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "devout" );
   else if( ch->alignment > 700 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "noble" );
   else if( ch->alignment > 350 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "honorable" );
   else if( ch->alignment > 100 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "worthy" );
   else if( ch->alignment > -100 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "neutral" );
   else if( ch->alignment > -350 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "base" );
   else if( ch->alignment > -700 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "evil" );
   else if( ch->alignment > -900 )
      snprintf( buf, MAX_STRING_LENGTH, "%s", "ignoble" );
   else
      snprintf( buf, MAX_STRING_LENGTH, "%s", "fiendish" );
   if( ch->level < 10 )
      pager_printf( ch, "DEX  : %2.2d(%2.2d)      Align: %-20.20s    Items: %5.5d   (max %5.5d)\r\n",
                    get_curr_dex( ch ), ch->perm_dex, buf, ch->carry_number, can_carry_n( ch ) );
   else
      pager_printf( ch, "DEX  : %2.2d(%2.2d)      Align: %+4.4d, %-14.14s   Items: %5.5d   (max %5.5d)\r\n",
                    get_curr_dex( ch ), ch->perm_dex, ch->alignment, buf, ch->carry_number, can_carry_n( ch ) );

   switch ( ch->position )
   {
      case POS_DEAD:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "slowly decomposing" );
         break;
      case POS_MORTAL:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "mortally wounded" );
         break;
      case POS_INCAP:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "incapacitated" );
         break;
      case POS_STUNNED:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "stunned" );
         break;
      case POS_SLEEPING:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "sleeping" );
         break;
      case POS_RESTING:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "resting" );
         break;
      case POS_STANDING:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "standing" );
         break;
      case POS_FIGHTING:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "fighting" );
         break;
      case POS_EVASIVE:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "fighting (evasive)" );   /* Fighting style support -haus */
         break;
      case POS_DEFENSIVE:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "fighting (defensive)" );
         break;
      case POS_AGGRESSIVE:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "fighting (aggressive)" );
         break;
      case POS_BERSERK:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "fighting (berserk)" );
         break;
      case POS_MOUNTED:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "mounted" );
         break;
      case POS_SITTING:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "sitting" );
         break;
   }
   pager_printf( ch, "CON  : %2.2d(%2.2d)      Pos'n: %-21.21s  Weight: %5.5d (max %7.7d)\r\n",
                 get_curr_con( ch ), ch->perm_con, buf, ch->carry_weight, can_carry_w( ch ) );

   /*
    * Fighting style support -haus
    */
   pager_printf( ch, "CHA  : %2.2d(%2.2d)      Wimpy: %-5d      ", get_curr_cha( ch ), ch->perm_cha, ch->wimpy );

   switch ( ch->style )
   {
      case STYLE_EVASIVE:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "evasive" );
         break;
      case STYLE_DEFENSIVE:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "defensive" );
         break;
      case STYLE_AGGRESSIVE:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "aggressive" );
         break;
      case STYLE_BERSERK:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "berserk" );
         break;
      default:
         snprintf( buf, MAX_STRING_LENGTH, "%s", "standard" );
         break;
   }
   pager_printf( ch, "Style: %-10.10s\r\n", buf );

   pager_printf( ch, "LCK  : %2.2d(%2.2d) \r\n", get_curr_lck( ch ), ch->perm_lck );

   pager_printf( ch, "Glory: %4.4d(%4.4d) \r\n", ch->pcdata->quest_curr, ch->pcdata->quest_accum );

   pager_printf( ch, "PRACT: %3.3d         Hitpoints: %-5d of %5d   Pager: (%c) %3d    AutoExit(%c)\r\n",
                 ch->practice, ch->hit, ch->max_hit,
                 IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) ? 'X' : ' ',
                 ch->pcdata->pagerlen, xIS_SET( ch->act, PLR_AUTOEXIT ) ? 'X' : ' ' );

   if( IS_VAMPIRE( ch ) )
      pager_printf( ch, "XP   : %-9d       Blood: %-5d of %5d   MKills:  %-5.5d    AutoLoot(%c)\r\n",
                    ch->exp, ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->pcdata->mkills,
                    xIS_SET( ch->act, PLR_AUTOLOOT ) ? 'X' : ' ' );
   else if( ch->Class == CLASS_WARRIOR )
      pager_printf( ch, "XP   : %-9d                               MKills:  %-5.5d    AutoLoot(%c)\r\n",
                    ch->exp, ch->pcdata->mkills, xIS_SET( ch->act, PLR_AUTOLOOT ) ? 'X' : ' ' );
   else
      pager_printf( ch, "XP   : %-9d        Mana: %-5d of %5d   MKills:  %-5.5d    AutoLoot(%c)\r\n",
                    ch->exp, ch->mana, ch->max_mana, ch->pcdata->mkills, xIS_SET( ch->act, PLR_AUTOLOOT ) ? 'X' : ' ' );

   pager_printf( ch, "GOLD : %-13s    Move: %-5d of %5d   Mdeaths: %-5.5d    AutoSac (%c)\r\n",
                 num_punct( ch->gold ), ch->move, ch->max_move, ch->pcdata->mdeaths, xIS_SET( ch->act,
                                                                                              PLR_AUTOSAC ) ? 'X' : ' ' );

   if( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
      send_to_pager( "You are drunk.\r\n", ch );
   if( !IS_NPC( ch ) && ch->pcdata->condition[COND_THIRST] == 0 )
      send_to_pager( "You are in danger of dehydrating.\r\n", ch );
   if( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] == 0 )
      send_to_pager( "You are starving to death.\r\n", ch );
   if( ch->position != POS_SLEEPING )
      switch ( ch->mental_state / 10 )
      {
         default:
            send_to_pager( "You're completely messed up!\r\n", ch );
            break;
         case -10:
            send_to_pager( "You're barely conscious.\r\n", ch );
            break;
         case -9:
            send_to_pager( "You can barely keep your eyes open.\r\n", ch );
            break;
         case -8:
            send_to_pager( "You're extremely drowsy.\r\n", ch );
            break;
         case -7:
            send_to_pager( "You feel very unmotivated.\r\n", ch );
            break;
         case -6:
            send_to_pager( "You feel sedated.\r\n", ch );
            break;
         case -5:
            send_to_pager( "You feel sleepy.\r\n", ch );
            break;
         case -4:
            send_to_pager( "You feel tired.\r\n", ch );
            break;
         case -3:
            send_to_pager( "You could use a rest.\r\n", ch );
            break;
         case -2:
            send_to_pager( "You feel a little under the weather.\r\n", ch );
            break;
         case -1:
            send_to_pager( "You feel fine.\r\n", ch );
            break;
         case 0:
            send_to_pager( "You feel great.\r\n", ch );
            break;
         case 1:
            send_to_pager( "You feel energetic.\r\n", ch );
            break;
         case 2:
            send_to_pager( "Your mind is racing.\r\n", ch );
            break;
         case 3:
            send_to_pager( "You can't think straight.\r\n", ch );
            break;
         case 4:
            send_to_pager( "Your mind is going 100 miles an hour.\r\n", ch );
            break;
         case 5:
            send_to_pager( "You're high as a kite.\r\n", ch );
            break;
         case 6:
            send_to_pager( "Your mind and body are slipping apart.\r\n", ch );
            break;
         case 7:
            send_to_pager( "Reality is slipping away.\r\n", ch );
            break;
         case 8:
            send_to_pager( "You have no idea what is real, and what is not.\r\n", ch );
            break;
         case 9:
            send_to_pager( "You feel immortal.\r\n", ch );
            break;
         case 10:
            send_to_pager( "You are a Supreme Entity.\r\n", ch );
            break;
      }
   else if( ch->mental_state > 45 )
      send_to_pager( "Your sleep is filled with strange and vivid dreams.\r\n", ch );
   else if( ch->mental_state > 25 )
      send_to_pager( "Your sleep is uneasy.\r\n", ch );
   else if( ch->mental_state < -35 )
      send_to_pager( "You are deep in a much needed sleep.\r\n", ch );
   else if( ch->mental_state < -25 )
      send_to_pager( "You are in deep slumber.\r\n", ch );
   send_to_pager( "Languages: ", ch );
   for( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
      if( knows_language( ch, lang_array[iLang], ch ) || ( IS_NPC( ch ) && ch->speaks == 0 ) )
      {
         if( lang_array[iLang] & ch->speaking || ( IS_NPC( ch ) && !ch->speaking ) )
            set_pager_color( AT_RED, ch );
         send_to_pager( lang_names[iLang], ch );
         send_to_pager( " ", ch );
         set_pager_color( AT_SCORE, ch );
      }
   send_to_pager( "\r\n", ch );

   if( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
      pager_printf( ch, "You are bestowed with the command(s): %s.\r\n", ch->pcdata->bestowments );

   if( ch->morph && ch->morph->morph )
   {
      send_to_pager( "----------------------------------------------------------------------------\r\n", ch );
      if( IS_IMMORTAL( ch ) )
         pager_printf( ch, "Morphed as (%d) %s with a timer of %d.\r\n",
                       ch->morph->morph->vnum, ch->morph->morph->short_desc, ch->morph->timer );
      else
         pager_printf( ch, "You are morphed into a %s.\r\n", ch->morph->morph->short_desc );
      send_to_pager( "----------------------------------------------------------------------------\r\n", ch );
   }
   if( CAN_PKILL( ch ) )
   {
      send_to_pager( "----------------------------------------------------------------------------\r\n", ch );
      pager_printf( ch, "PKILL DATA:  Pkills (%3.3d)     Illegal Pkills (%3.3d)     Pdeaths (%3.3d)\r\n",
                    ch->pcdata->pkills, ch->pcdata->illegal_pk, ch->pcdata->pdeaths );
   }
   if( ch->pcdata->clan && ch->pcdata->clan->clan_type != CLAN_ORDER && ch->pcdata->clan->clan_type != CLAN_GUILD )
   {
      send_to_pager( "----------------------------------------------------------------------------\r\n", ch );
      pager_printf( ch, "CLAN STATS:  %-14.14s  Clan AvPkills : %-5d  Clan NonAvpkills : %-5d\r\n",
                    ch->pcdata->clan->name, ch->pcdata->clan->pkills[6],
                    ( ch->pcdata->clan->pkills[1] + ch->pcdata->clan->pkills[2] +
                      ch->pcdata->clan->pkills[3] + ch->pcdata->clan->pkills[4] + ch->pcdata->clan->pkills[5] ) );
      pager_printf( ch, "                             Clan AvPdeaths: %-5d  Clan NonAvpdeaths: %-5d\r\n",
                    ch->pcdata->clan->pdeaths[6],
                    ( ch->pcdata->clan->pdeaths[1] + ch->pcdata->clan->pdeaths[2] +
                      ch->pcdata->clan->pdeaths[3] + ch->pcdata->clan->pdeaths[4] + ch->pcdata->clan->pdeaths[5] ) );
   }
   if( ch->pcdata->deity )
   {
      send_to_pager( "----------------------------------------------------------------------------\r\n", ch );
      if( ch->pcdata->favor > 2250 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "loved" );
      else if( ch->pcdata->favor > 2000 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "cherished" );
      else if( ch->pcdata->favor > 1750 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "honored" );
      else if( ch->pcdata->favor > 1500 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "praised" );
      else if( ch->pcdata->favor > 1250 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "favored" );
      else if( ch->pcdata->favor > 1000 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "respected" );
      else if( ch->pcdata->favor > 750 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "liked" );
      else if( ch->pcdata->favor > 250 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "tolerated" );
      else if( ch->pcdata->favor > -250 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "ignored" );
      else if( ch->pcdata->favor > -750 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "shunned" );
      else if( ch->pcdata->favor > -1000 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "disliked" );
      else if( ch->pcdata->favor > -1250 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "dishonored" );
      else if( ch->pcdata->favor > -1500 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "disowned" );
      else if( ch->pcdata->favor > -1750 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "abandoned" );
      else if( ch->pcdata->favor > -2000 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "despised" );
      else if( ch->pcdata->favor > -2250 )
         snprintf( buf, MAX_STRING_LENGTH, "%s", "hated" );
      else
         snprintf( buf, MAX_STRING_LENGTH, "%s", "damned" );
      pager_printf( ch, "Deity:  %-20s  Favor: %s\r\n", ch->pcdata->deity->name, buf );
   }
   if( ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_ORDER )
   {
      send_to_pager( "----------------------------------------------------------------------------\r\n", ch );
      pager_printf( ch, "Order:  %-20s  Order Mkills:  %-6d   Order MDeaths:  %-6d\r\n",
                    ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths );
   }
   if( ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_GUILD )
   {
      send_to_pager( "----------------------------------------------------------------------------\r\n", ch );
      pager_printf( ch, "Guild:  %-20s  Guild Mkills:  %-6d   Guild MDeaths:  %-6d\r\n",
                    ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths );
   }
   if( IS_IMMORTAL( ch ) )
   {
      send_to_pager( "----------------------------------------------------------------------------\r\n", ch );

      pager_printf( ch, "IMMORTAL DATA:  Wizinvis [%s]  Wizlevel (%d)\r\n",
                    xIS_SET( ch->act, PLR_WIZINVIS ) ? "X" : " ", ch->pcdata->wizinvis );

      pager_printf( ch, "Bamfin:  %s %s\r\n", ch->name, ( ch->pcdata->bamfin[0] != '\0' )
                    ? ch->pcdata->bamfin : "appears in a swirling mist." );
      pager_printf( ch, "Bamfout: %s %s\r\n", ch->name, ( ch->pcdata->bamfout[0] != '\0' )
                    ? ch->pcdata->bamfout : "leaves in a swirling mist." );

      /*
       * Area Loaded info - Scryn 8/11
       */
      if( ch->pcdata->area )
      {
         pager_printf( ch, "Vnums:   Room (%-5.5d - %-5.5d)   Object (%-5.5d - %-5.5d)   Mob (%-5.5d - %-5.5d)\r\n",
                       ch->pcdata->area->low_r_vnum, ch->pcdata->area->hi_r_vnum,
                       ch->pcdata->area->low_o_vnum, ch->pcdata->area->hi_o_vnum,
                       ch->pcdata->area->low_m_vnum, ch->pcdata->area->hi_m_vnum );
         pager_printf( ch, "Area Loaded [%s]\r\n", ( IS_SET( ch->pcdata->area->status, AREA_LOADED ) ) ? "yes" : "no" );
      }
   }
   if( ch->first_affect )
   {
      int i;
      SKILLTYPE *sktmp;

      i = 0;
      send_to_pager( "----------------------------------------------------------------------------\r\n", ch );
      send_to_pager( "AFFECT DATA:                            ", ch );
      for( paf = ch->first_affect; paf; paf = paf->next )
      {
         if( ( sktmp = get_skilltype( paf->type ) ) == NULL )
            continue;
         if( ch->level < 20 )
         {
            pager_printf( ch, "[%-34.34s]    ", sktmp->name );
            if( i == 0 )
               i = 2;
            if( ( ++i % 3 ) == 0 )
               send_to_pager( "\r\n", ch );
         }
         if( ch->level >= 20 )
         {
            if( paf->modifier == 0 )
               pager_printf( ch, "[%-24.24s;%5d rds]    ", sktmp->name, paf->duration );
            else if( paf->modifier > 999 )
               pager_printf( ch, "[%-15.15s; %7.7s;%5d rds]    ",
                             sktmp->name, tiny_affect_loc_name( paf->location ), paf->duration );
            else
               pager_printf( ch, "[%-11.11s;%+-3.3d %7.7s;%5d rds]    ",
                             sktmp->name, paf->modifier, tiny_affect_loc_name( paf->location ), paf->duration );
            if( i == 0 )
               i = 1;
            if( ( ++i % 2 ) == 0 )
               send_to_pager( "\r\n", ch );
         }
      }
   }
   send_to_pager( "\r\n", ch );
   return;
}

/*
 * Return ascii name of an affect location.
 */
const char *tiny_affect_loc_name( int location )
{
   switch ( location )
   {
      case APPLY_NONE:
         return "NIL";
      case APPLY_STR:
         return " STR  ";
      case APPLY_DEX:
         return " DEX  ";
      case APPLY_INT:
         return " INT  ";
      case APPLY_WIS:
         return " WIS  ";
      case APPLY_CON:
         return " CON  ";
      case APPLY_CHA:
         return " CHA  ";
      case APPLY_LCK:
         return " LCK  ";
      case APPLY_SEX:
         return " SEX  ";
      case APPLY_CLASS:
         return " CLASS";
      case APPLY_LEVEL:
         return " LVL  ";
      case APPLY_AGE:
         return " AGE  ";
      case APPLY_MANA:
         return " MANA ";
      case APPLY_HIT:
         return " HV   ";
      case APPLY_MOVE:
         return " MOVE ";
      case APPLY_GOLD:
         return " GOLD ";
      case APPLY_EXP:
         return " EXP  ";
      case APPLY_AC:
         return " AC   ";
      case APPLY_HITROLL:
         return " HITRL";
      case APPLY_DAMROLL:
         return " DAMRL";
      case APPLY_SAVING_POISON:
         return "SV POI";
      case APPLY_SAVING_ROD:
         return "SV ROD";
      case APPLY_SAVING_PARA:
         return "SV PARA";
      case APPLY_SAVING_BREATH:
         return "SV BRTH";
      case APPLY_SAVING_SPELL:
         return "SV SPLL";
      case APPLY_HEIGHT:
         return "HEIGHT";
      case APPLY_WEIGHT:
         return "WEIGHT";
      case APPLY_AFFECT:
         return "AFF BY";
      case APPLY_RESISTANT:
         return "RESIST";
      case APPLY_IMMUNE:
         return "IMMUNE";
      case APPLY_SUSCEPTIBLE:
         return "SUSCEPT";
      case APPLY_WEAPONSPELL:
         return " WEAPON";
      case APPLY_BACKSTAB:
         return "BACKSTB";
      case APPLY_PICK:
         return " PICK  ";
      case APPLY_TRACK:
         return " TRACK ";
      case APPLY_STEAL:
         return " STEAL ";
      case APPLY_SNEAK:
         return " SNEAK ";
      case APPLY_HIDE:
         return " HIDE  ";
      case APPLY_PALM:
         return " PALM  ";
      case APPLY_DETRAP:
         return " DETRAP";
      case APPLY_DODGE:
         return " DODGE ";
      case APPLY_PEEK:
         return " PEEK  ";
      case APPLY_SCAN:
         return " SCAN  ";
      case APPLY_GOUGE:
         return " GOUGE ";
      case APPLY_SEARCH:
         return " SEARCH";
      case APPLY_MOUNT:
         return " MOUNT ";
      case APPLY_DISARM:
         return " DISARM";
      case APPLY_KICK:
         return " KICK  ";
      case APPLY_PARRY:
         return " PARRY ";
      case APPLY_BASH:
         return " BASH  ";
      case APPLY_STUN:
         return " STUN  ";
      case APPLY_PUNCH:
         return " PUNCH ";
      case APPLY_CLIMB:
         return " CLIMB ";
      case APPLY_GRIP:
         return " GRIP  ";
      case APPLY_SCRIBE:
         return " SCRIBE";
      case APPLY_BREW:
         return " BREW  ";
      case APPLY_WEARSPELL:
         return " WEAR  ";
      case APPLY_REMOVESPELL:
         return " REMOVE";
      case APPLY_EMOTION:
         return "EMOTION";
      case APPLY_MENTALSTATE:
         return " MENTAL";
      case APPLY_STRIPSN:
         return " DISPEL";
      case APPLY_REMOVE:
         return " REMOVE";
      case APPLY_DIG:
         return " DIG   ";
      case APPLY_FULL:
         return " HUNGER";
      case APPLY_THIRST:
         return " THIRST";
      case APPLY_DRUNK:
         return " DRUNK ";
      case APPLY_BLOOD:
         return " BLOOD ";
      case APPLY_COOK:
         return " COOK  ";
      case APPLY_RECURRINGSPELL:
         return " RECURR";
      case APPLY_CONTAGIOUS:
         return "CONTGUS";
      case APPLY_ODOR:
         return " ODOR  ";
      case APPLY_ROOMFLAG:
         return " RMFLG ";
      case APPLY_SECTORTYPE:
         return " SECTOR";
      case APPLY_ROOMLIGHT:
         return " LIGHT ";
      case APPLY_TELEVNUM:
         return " TELEVN";
      case APPLY_TELEDELAY:
         return " TELEDY";
   };

   bug( "%s: unknown location %d.", __func__, location );
   return "(?)";
}

const char *get_class( CHAR_DATA * ch )
{
   if( IS_NPC( ch ) && ch->Class < MAX_NPC_CLASS && ch->Class >= 0 )
      return ( npc_class[ch->Class] );
   else if( !IS_NPC( ch ) && ch->Class < MAX_PC_CLASS && ch->Class >= 0 )
      return class_table[ch->Class]->who_name;
   return ( "Unknown" );
}

const char *get_race( CHAR_DATA * ch )
{
   if( ch->race < MAX_PC_RACE && ch->race >= 0 )
      return ( race_table[ch->race]->race_name );
   if( ch->race < MAX_NPC_RACE && ch->race >= 0 )
      return ( npc_race[ch->race] );
   return ( "Unknown" );
}

/*								-Thoric
 * Display your current exp, level, and surrounding level exp requirements
 */
void do_level( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   int x, lowlvl, hilvl;

   if( ch->level == 1 )
      lowlvl = 1;
   else
      lowlvl = UMAX( 2, ch->level - 5 );
   hilvl = URANGE( ch->level, ch->level + 5, MAX_LEVEL );
   set_char_color( AT_SCORE, ch );
   ch_printf( ch, "\r\nExperience required, levels %d to %d:\r\n______________________________________________\r\n\r\n",
              lowlvl, hilvl );
   snprintf( buf, MAX_STRING_LENGTH, " exp  (Current: %12s)", num_punct( ch->exp ) );
   snprintf( buf2, MAX_STRING_LENGTH, " exp  (Needed:  %12s)", num_punct( exp_level( ch, ch->level + 1 ) - ch->exp ) );
   for( x = lowlvl; x <= hilvl; x++ )
      ch_printf( ch, " (%2d) %12s%s\r\n", x, num_punct( exp_level( ch, x ) ),
                 ( x == ch->level ) ? buf : ( x == ch->level + 1 ) ? buf2 : " exp" );
   send_to_char( "______________________________________________\r\n", ch );
}

/* 1997, Blodkai */
void do_remains( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   OBJ_DATA *obj;
   bool found = FALSE;

   if( IS_NPC( ch ) )
      return;

   set_char_color( AT_MAGIC, ch );
   if( !ch->pcdata->deity )
   {
      send_to_pager( "You have no deity from which to seek such assistance...\r\n", ch );
      return;
   }

   if( ch->pcdata->favor < ch->level * 2 )
   {
      send_to_pager( "Your favor is insufficient for such assistance...\r\n", ch );
      return;
   }

   pager_printf( ch, "%s appears in a vision, revealing that your remains... ", ch->pcdata->deity->name );
   snprintf( buf, MAX_STRING_LENGTH, "the corpse of %s", ch->name );
   for( obj = first_object; obj; obj = obj->next )
   {
      if( obj->in_room && !str_cmp( buf, obj->short_descr ) && ( obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC ) )
      {
         found = TRUE;
         pager_printf( ch, "\r\n  - at %s will endure for %d ticks", obj->in_room->name, obj->timer );
      }
   }
   if( !found )
      send_to_pager( " no longer exist.\r\n", ch );
   else
   {
      send_to_pager( "\r\n", ch );
      ch->pcdata->favor -= ch->level * 2;
   }
   return;
}

/* Affects-at-a-glance, Blodkai */
void do_affected( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   AFFECT_DATA *paf;
   SKILLTYPE *skill;

   if( IS_NPC( ch ) )
      return;

   set_char_color( AT_SCORE, ch );

   argument = one_argument( argument, arg );
   if( !str_cmp( arg, "by" ) )
   {
      send_to_char_color( "\r\n&BImbued with:\r\n", ch );
      ch_printf_color( ch, "&C%s\r\n", !xIS_EMPTY( ch->affected_by ) ? affect_bit_name( &ch->affected_by ) : "nothing" );
      if( ch->level >= 20 )
      {
         send_to_char( "\r\n", ch );
         if( ch->resistant > 0 )
         {
            send_to_char_color( "&BResistances:  ", ch );
            ch_printf_color( ch, "&C%s\r\n", flag_string( ch->resistant, ris_flags ) );
         }
         if( ch->immune > 0 )
         {
            send_to_char_color( "&BImmunities:   ", ch );
            ch_printf_color( ch, "&C%s\r\n", flag_string( ch->immune, ris_flags ) );
         }
         if( ch->susceptible > 0 )
         {
            send_to_char_color( "&BSuscepts:     ", ch );
            ch_printf_color( ch, "&C%s\r\n", flag_string( ch->susceptible, ris_flags ) );
         }
      }
      return;
   }

   if( !ch->first_affect )
   {
      send_to_char_color( "\r\n&CNo cantrip or skill affects you.\r\n", ch );
   }
   else
   {
      send_to_char( "\r\n", ch );
      for( paf = ch->first_affect; paf; paf = paf->next )
         if( ( skill = get_skilltype( paf->type ) ) != NULL )
         {
            set_char_color( AT_BLUE, ch );
            send_to_char( "Affected:  ", ch );
            set_char_color( AT_SCORE, ch );
            if( ch->level >= 20 || IS_PKILL( ch ) )
            {
               if( paf->duration < 25 )
                  set_char_color( AT_WHITE, ch );
               if( paf->duration < 6 )
                  set_char_color( AT_WHITE + AT_BLINK, ch );
               ch_printf( ch, "(%5d)   ", paf->duration );
            }
            ch_printf( ch, "%-18s\r\n", skill->name );
         }
   }
   return;
}

void do_inventory( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim;

   if( !argument || argument[0] == '\0' || !IS_IMMORTAL( ch ) )
      victim = ch;
   else
   {
      if( !( victim = get_char_world( ch, argument ) ) )
      {
         ch_printf( ch, "There is nobody named %s online.\r\n", argument );
         return;
      }
   }

   if( victim != ch )
      ch_printf( ch, "&R%s is carrying:\r\n", IS_NPC( victim ) ? victim->short_descr : victim->name );
   else
      send_to_char( "&RYou are carrying:\r\n", ch );

   show_list_to_char( victim->first_carrying, ch, TRUE, TRUE );
   return;
}

void do_equipment( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *victim = ch;
   OBJ_DATA *obj;
   int iWear;
   bool found;

   if( argument[0] == '\0' || get_trust( ch ) <= LEVEL_GOD )
      victim = ch;
   else
   {
      if( ( victim = get_char_world( ch, argument ) ) == NULL )
      {
         send_to_char( "They're not here.\r\n", ch );
         return;
      }
   }

   if( victim != ch )
      ch_printf( ch, "&R%s is using:\r\n", IS_NPC( victim ) ? victim->short_descr : victim->name );
   else
      send_to_char( "&RYou are using:\r\n", ch );

   found = FALSE;
   set_char_color( AT_OBJECT, ch );
   for( iWear = 0; iWear < MAX_WEAR; iWear++ )
   {
      for( obj = victim->first_carrying; obj; obj = obj->next_content )
      {
         if( obj->wear_loc == iWear )
         {
            if( ( !IS_NPC( victim ) ) && ( victim->race > 0 ) && ( victim->race < MAX_PC_RACE ) )
               send_to_char( race_table[victim->race]->where_name[iWear], ch );
            else
               send_to_char( where_name[iWear], ch );

            if( can_see_obj( ch, obj ) )
            {
               send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
               send_to_char( "\r\n", ch );
            }
            else
               send_to_char( "something.\r\n", ch );
            found = TRUE;
         }
      }
   }

   if( !found )
      send_to_char( "Nothing.\r\n", ch );
}

void set_title( CHAR_DATA * ch, const char *title )
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
   {
      bug( "%s: NPC.", __func__ );
      return;
   }

   if( isalpha( title[0] ) || isdigit( title[0] ) )
   {
      buf[0] = ' ';
      mudstrlcpy( buf + 1, title, MAX_STRING_LENGTH - 1 );
   }
   else
      mudstrlcpy( buf, title, MAX_STRING_LENGTH );

   STRFREE( ch->pcdata->title );
   ch->pcdata->title = STRALLOC( buf );
}

void do_title( CHAR_DATA* ch, const char* argument)
{
   if( IS_NPC( ch ) )
      return;

   set_char_color( AT_SCORE, ch );

   if( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ) )
   {
      set_char_color( AT_IMMORT, ch );
      send_to_char( "The Gods prohibit you from changing your title.\r\n", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Change your title to what?\r\n", ch );
      return;
   }

   char title[50];
   mudstrlcpy(title, argument, 50);

   smash_tilde( title );
   set_title( ch, title );
   send_to_char( "Ok.\r\n", ch );
}

void do_homepage( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
      return;

   if( IS_SET( ch->pcdata->flags, PCFLAG_NOHOMEPAGE ) )
   {
      send_to_char( "The Gods prohibit you from changing your homepage.\r\n", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      if( !ch->pcdata->homepage )
         ch->pcdata->homepage = str_dup( "" );
      ch_printf( ch, "Your homepage is: %s\r\n", show_tilde( ch->pcdata->homepage ) );
      return;
   }

   if( !str_cmp( argument, "clear" ) )
   {
      if( ch->pcdata->homepage )
         DISPOSE( ch->pcdata->homepage );
      ch->pcdata->homepage = str_dup( "" );
      send_to_char( "Homepage cleared.\r\n", ch );
      return;
   }

   if( strstr( argument, "://" ) )
      mudstrlcpy( buf, argument, MAX_STRING_LENGTH );
   else
      snprintf( buf, MAX_STRING_LENGTH, "http://%s", argument );
   if( strlen( buf ) > 70 )
      buf[70] = '\0';

   hide_tilde( buf );
   if( ch->pcdata->homepage )
      DISPOSE( ch->pcdata->homepage );
   ch->pcdata->homepage = str_dup( buf );
   send_to_char( "Homepage set.\r\n", ch );
}

/*
 * Set your personal description				-Thoric
 */
void do_description( CHAR_DATA* ch, const char* argument)
{
   if( IS_NPC( ch ) )
   {
      send_to_char( "Monsters are too dumb to do that!\r\n", ch );
      return;
   }

   if( IS_SET( ch->pcdata->flags, PCFLAG_NODESC ) )
   {
      send_to_char( "You cannot set your description.\r\n", ch );
      return;
   }

   if( !ch->desc )
   {
      bug( "%s: no descriptor", __func__ );
      return;
   }

   switch ( ch->substate )
   {
      default:
         bug( "%s: illegal substate", __func__ );
         return;

      case SUB_RESTRICTED:
         send_to_char( "You cannot use this command from within another command.\r\n", ch );
         return;

      case SUB_NONE:
         ch->substate = SUB_PERSONAL_DESC;
         ch->dest_buf = ch;
         start_editing( ch, ch->description );
         return;

      case SUB_PERSONAL_DESC:
         STRFREE( ch->description );
         ch->description = copy_buffer( ch );
         stop_editing( ch );
         return;
   }
}

/* Ripped off do_description for whois bio's -- Scryn*/
void do_bio( CHAR_DATA* ch, const char* argument)
{
   if( IS_NPC( ch ) )
   {
      send_to_char( "Mobs cannot set a bio.\r\n", ch );
      return;
   }

   if( IS_SET( ch->pcdata->flags, PCFLAG_NOBIO ) )
   {
      set_char_color( AT_RED, ch );
      send_to_char( "The gods won't allow you to do that!\r\n", ch );
      return;
   }

   if( !ch->desc )
   {
      bug( "%s: no descriptor", __func__ );
      return;
   }

   switch ( ch->substate )
   {
      default:
         bug( "%s: illegal substate", __func__ );
         return;

      case SUB_RESTRICTED:
         send_to_char( "You cannot use this command from within another command.\r\n", ch );
         return;

      case SUB_NONE:
         ch->substate = SUB_PERSONAL_BIO;
         ch->dest_buf = ch;
         start_editing( ch, ch->pcdata->bio );
         return;

      case SUB_PERSONAL_BIO:
         STRFREE( ch->pcdata->bio );
         ch->pcdata->bio = copy_buffer( ch );
         stop_editing( ch );
         return;
   }
}

/*
 * New stat and statreport command coded by Morphina
 * Bug fixes by Shaddai
 */
void do_statreport( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   if( IS_VAMPIRE( ch ) )
   {
      ch_printf( ch, "You report: %d/%d hp %d/%d blood %d/%d mv %d xp.\r\n",
                 ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST],
                 10 + ch->level, ch->move, ch->max_move, ch->exp );
      snprintf( buf, MAX_STRING_LENGTH, "$n reports: %d/%d hp %d/%d blood %d/%d mv %d xp.",
                ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST],
                10 + ch->level, ch->move, ch->max_move, ch->exp );
      act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );
   }
   else
   {
      ch_printf( ch, "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\r\n",
                 ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp );
      snprintf( buf, MAX_STRING_LENGTH, "$n reports: %d/%d hp %d/%d mana %d/%d mv %d xp.",
                ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp );
      act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );
   }

   ch_printf( ch, "Your base stats:    %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.\r\n",
              ch->perm_str, ch->perm_wis, ch->perm_int, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck );
   snprintf( buf, MAX_STRING_LENGTH, "$n's base stats:    %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.",
             ch->perm_str, ch->perm_wis, ch->perm_int, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck );
   act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );

   ch_printf( ch, "Your current stats: %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.\r\n",
              get_curr_str( ch ), get_curr_wis( ch ), get_curr_int( ch ),
              get_curr_dex( ch ), get_curr_con( ch ), get_curr_cha( ch ), get_curr_lck( ch ) );
   snprintf( buf, MAX_STRING_LENGTH, "$n's current stats: %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.",
             get_curr_str( ch ), get_curr_wis( ch ), get_curr_int( ch ),
             get_curr_dex( ch ), get_curr_con( ch ), get_curr_cha( ch ), get_curr_lck( ch ) );
   act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );
   return;
}

void do_stat( CHAR_DATA* ch, const char* argument)
{
   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   if( IS_VAMPIRE( ch ) )
      ch_printf( ch, "You report: %d/%d hp %d/%d blood %d/%d mv %d xp.\r\n",
                 ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST],
                 10 + ch->level, ch->move, ch->max_move, ch->exp );
   else
      ch_printf( ch, "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\r\n",
                 ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp );

   ch_printf( ch, "Your base stats:    %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.\r\n",
              ch->perm_str, ch->perm_wis, ch->perm_int, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck );

   ch_printf( ch, "Your current stats: %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.\r\n",
              get_curr_str( ch ), get_curr_wis( ch ), get_curr_int( ch ),
              get_curr_dex( ch ), get_curr_con( ch ), get_curr_cha( ch ), get_curr_lck( ch ) );
   return;
}

void do_report( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) && ch->fighting )
      return;

   if( IS_AFFECTED( ch, AFF_POSSESS ) )
   {
      send_to_char( "You can't do that in your current state of mind!\r\n", ch );
      return;
   }


   if( IS_VAMPIRE( ch ) )
      ch_printf( ch,
                 "You report: %d/%d hp %d/%d blood %d/%d mv %d xp.\r\n",
                 ch->hit, ch->max_hit,
                 ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move, ch->exp );
   else
      ch_printf( ch,
                 "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\r\n",
                 ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp );

   if( IS_VAMPIRE( ch ) )
      snprintf( buf, MAX_INPUT_LENGTH, "$n reports: %d/%d hp %d/%d blood %d/%d mv %d xp.\r\n",
                ch->hit, ch->max_hit,
                ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move, ch->exp );
   else
      snprintf( buf, MAX_INPUT_LENGTH, "$n reports: %d/%d hp %d/%d mana %d/%d mv %d xp.",
                ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp );

   act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );

   return;
}

void do_fprompt( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];

   set_char_color( AT_GREY, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "NPC's can't change their prompt..\r\n", ch );
      return;
   }

   smash_tilde( argument );
   one_argument( argument, arg );
   if( !*arg || !str_cmp( arg, "display" ) )
   {
      send_to_char( "Your current fighting prompt string:\r\n", ch );
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "%s\r\n", !str_cmp( ch->pcdata->fprompt, "" ) ? "(default prompt)" : ch->pcdata->fprompt );
      set_char_color( AT_GREY, ch );
      send_to_char( "Type 'help prompt' for information on changing your prompt.\r\n", ch );
      return;
   }

   send_to_char( "Replacing old prompt of:\r\n", ch );
   set_char_color( AT_WHITE, ch );
   ch_printf( ch, "%s\r\n", !str_cmp( ch->pcdata->fprompt, "" ) ? "(default prompt)" : ch->pcdata->fprompt );
   if( ch->pcdata->fprompt )
      STRFREE( ch->pcdata->fprompt );

   char prompt[128];
   mudstrlcpy(prompt, argument, 128);

   /*
    * Can add a list of pre-set prompts here if wanted.. perhaps
    * 'prompt 1' brings up a different, pre-set prompt 
    */
   if( !str_cmp( arg, "default" ) )
      ch->pcdata->fprompt = STRALLOC( "" );
   else if( !str_cmp( arg, "none" ) )
      ch->pcdata->fprompt = STRALLOC( ch->pcdata->prompt );
   else
      ch->pcdata->fprompt = STRALLOC( argument );
   return;
}

void do_prompt( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];

   set_char_color( AT_GREY, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "NPC's can't change their prompt..\r\n", ch );
      return;
   }
   smash_tilde( argument );
   one_argument( argument, arg );
   if( !*arg || !str_cmp( arg, "display" ) )
   {
      send_to_char( "Your current prompt string:\r\n", ch );
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "%s\r\n", !str_cmp( ch->pcdata->prompt, "" ) ? "(default prompt)" : ch->pcdata->prompt );
      set_char_color( AT_GREY, ch );
      send_to_char( "Type 'help prompt' for information on changing your prompt.\r\n", ch );
      return;
   }
   send_to_char( "Replacing old prompt of:\r\n", ch );
   set_char_color( AT_WHITE, ch );
   ch_printf( ch, "%s\r\n", !str_cmp( ch->pcdata->prompt, "" ) ? "(default prompt)" : ch->pcdata->prompt );
   if( ch->pcdata->prompt )
      STRFREE( ch->pcdata->prompt );

   char prompt[128];
   mudstrlcpy(prompt, argument, 128);

   /*
    * Can add a list of pre-set prompts here if wanted.. perhaps
    * 'prompt 1' brings up a different, pre-set prompt 
    */
   if( !str_cmp( arg, "default" ) )
      ch->pcdata->prompt = STRALLOC( "" );
   else if( !str_cmp( arg, "fprompt" ) )
      ch->pcdata->prompt = STRALLOC( ch->pcdata->fprompt );
   else
      ch->pcdata->prompt = STRALLOC( argument );
   return;
}

void do_compass( CHAR_DATA *ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      if( xIS_SET( ch->act, PLR_COMPASS ) )
      {
         xREMOVE_BIT( ch->act, PLR_COMPASS );
         send_to_char( "Compass is now off.\r\n", ch );
      }
      else
      {
         xSET_BIT( ch->act, PLR_COMPASS );
         do_look( ch, "auto" );
      }
   }
}

/*
 *  Command to show current favor by Edmond
 */
void do_favor( CHAR_DATA * ch, const char *argument )
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   set_char_color( AT_GREEN, ch );
   if( !ch->pcdata->deity )
   {
      send_to_char( "You have not yet chosen a deity.\r\n", ch );
      return;
   }
   else if( ch->pcdata->favor > 2250 )
      mudstrlcpy( buf, "loved", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > 2000 )
      mudstrlcpy( buf, "cherished", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > 1750 )
      mudstrlcpy( buf, "honored", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > 1500 )
      mudstrlcpy( buf, "praised", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > 1250 )
      mudstrlcpy( buf, "favored", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > 1000 )
      mudstrlcpy( buf, "respected", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > 750 )
      mudstrlcpy( buf, "liked", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > 250 )
      mudstrlcpy( buf, "tolerated", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > -250 )
      mudstrlcpy( buf, "ignored", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > -750 )
      mudstrlcpy( buf, "shunned", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > -1000 )
      mudstrlcpy( buf, "disliked", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > -1250 )
      mudstrlcpy( buf, "dishonored", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > -1500 )
      mudstrlcpy( buf, "disowned", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > -1750 )
      mudstrlcpy( buf, "abandoned", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > -2000 )
      mudstrlcpy( buf, "despised", MAX_STRING_LENGTH );
   else if( ch->pcdata->favor > -2250 )
      mudstrlcpy( buf, "hated", MAX_STRING_LENGTH );
   else
      mudstrlcpy( buf, "damned", MAX_STRING_LENGTH );

   ch_printf( ch, "%s considers you to be %s.\r\n", ch->pcdata->deity->name, buf );
}
