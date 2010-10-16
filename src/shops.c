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
 *			 Shop and repair shop module                              *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "mud.h"

/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *keeper, *whof;
   SHOP_DATA *pShop;
   int speakswell;

   pShop = NULL;
   for( keeper = ch->in_room->first_person; keeper; keeper = keeper->next_in_room )
      if( IS_NPC( keeper ) && ( pShop = keeper->pIndexData->pShop ) != NULL )
         break;

   if( !pShop )
   {
      send_to_char( "You can't do that here.\r\n", ch );
      return NULL;
   }

   /*
    * Undesirables.
    */
   if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_KILLER ) )
   {
      do_say( keeper, "Murderers are not welcome here!" );
      snprintf( buf, MAX_STRING_LENGTH, "%s the KILLER is over here!\r\n", ch->name );
      do_shout( keeper, buf );
      return NULL;
   }

   if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_THIEF ) )
   {
      do_say( keeper, "Thieves are not welcome here!" );
      snprintf( buf, MAX_STRING_LENGTH, "%s the THIEF is over here!\r\n", ch->name );
      do_shout( keeper, buf );
      return NULL;
   }

   /*
    * Disallow sales during battle
    */
   if( ( whof = who_fighting( keeper ) ) != NULL )
   {
      if( whof == ch )
         send_to_char( "I don't think that's a good idea...\r\n", ch );
      else
         do_say( keeper, "I'm too busy for that!" );
      return NULL;
   }

   if( who_fighting( ch ) )
   {
      ch_printf( ch, "%s doesn't seem to want to get involved.\r\n", PERS( keeper, ch ) );
      return NULL;
   }

   /*
    * Check to see if show is open.
    * Supports closing times after midnight
    */
   if( pShop->open_hour > pShop->close_hour )
   {
      if( time_info.hour < pShop->open_hour && time_info.hour > pShop->close_hour )
      {
         do_say( keeper, "Sorry, come back later." );
         return NULL;
      }
   }
   else
   {
      if( time_info.hour < pShop->open_hour )
      {
         do_say( keeper, "Sorry, come back later." );
         return NULL;
      }
      if( time_info.hour > pShop->close_hour )
      {
         do_say( keeper, "Sorry, come back tomorrow." );
         return NULL;
      }
   }

   if( keeper->position == POS_SLEEPING )
   {
      send_to_char( "While they're asleep?\r\n", ch );
      return NULL;
   }

   if( keeper->position < POS_SLEEPING )
   {
      send_to_char( "I don't think they can hear you...\r\n", ch );
      return NULL;
   }

   /*
    * Invisible or hidden people.
    */
   if( !can_see( keeper, ch ) )
   {
      do_say( keeper, "I don't trade with folks I can't see." );
      return NULL;
   }

   speakswell = UMIN( knows_language( keeper, ch->speaking, ch ), knows_language( ch, ch->speaking, keeper ) );

   if( ( number_percent(  ) % 65 ) > speakswell )
   {
      if( speakswell > 60 )
         snprintf( buf, MAX_STRING_LENGTH, "%s Could you repeat that?  I didn't quite catch it.", ch->name );
      else if( speakswell > 50 )
         snprintf( buf, MAX_STRING_LENGTH, "%s Could you say that a little more clearly please?", ch->name );
      else if( speakswell > 40 )
         snprintf( buf, MAX_STRING_LENGTH, "%s Sorry... What was that you wanted?", ch->name );
      else
         snprintf( buf, MAX_STRING_LENGTH, "%s I can't understand you.", ch->name );
      do_tell( keeper, buf );
      return NULL;
   }

   return keeper;
}

/*
 * repair commands.
 */
CHAR_DATA *find_fixer( CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *keeper, *whof;
   REPAIR_DATA *rShop;
   int speakswell;

   rShop = NULL;
   for( keeper = ch->in_room->first_person; keeper; keeper = keeper->next_in_room )
      if( IS_NPC( keeper ) && ( rShop = keeper->pIndexData->rShop ) != NULL )
         break;

   if( !rShop )
   {
      send_to_char( "You can't do that here.\r\n", ch );
      return NULL;
   }

   /*
    * Undesirables.
    */
   if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_KILLER ) )
   {
      do_say( keeper, "Murderers are not welcome here!" );
      snprintf( buf, MAX_STRING_LENGTH, "%s the KILLER is over here!\r\n", ch->name );
      do_shout( keeper, buf );
      return NULL;
   }

   if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_THIEF ) )
   {
      do_say( keeper, "Thieves are not welcome here!" );
      snprintf( buf, MAX_STRING_LENGTH, "%s the THIEF is over here!\r\n", ch->name );
      do_shout( keeper, buf );
      return NULL;
   }

   /*
    * Disallow sales during battle
    */
   if( ( whof = who_fighting( keeper ) ) != NULL )
   {
      if( whof == ch )
         send_to_char( "I don't think that's a good idea...\r\n", ch );
      else
         do_say( keeper, "I'm too busy for that!" );
      return NULL;
   }

   /*
    * According to rlog, this is the second time I've done this
    * * so mobiles can repair in combat.  -- Blod, 1/98
    */
   if( !IS_NPC( ch ) && who_fighting( ch ) )
   {
      ch_printf( ch, "%s doesn't seem to want to get involved.\r\n", PERS( keeper, ch ) );
      return NULL;
   }

   /*
    * Check to see if show is open.
    * Supports closing times after midnight
    */
   if( rShop->open_hour > rShop->close_hour )
   {
      if( time_info.hour < rShop->open_hour && time_info.hour > rShop->close_hour )
      {
         do_say( keeper, "Sorry, come back later." );
         return NULL;
      }
   }
   else
   {
      if( time_info.hour < rShop->open_hour )
      {
         do_say( keeper, "Sorry, come back later." );
         return NULL;
      }
      if( time_info.hour > rShop->close_hour )
      {
         do_say( keeper, "Sorry, come back tomorrow." );
         return NULL;
      }
   }

   if( keeper->position == POS_SLEEPING )
   {
      send_to_char( "While they're asleep?\r\n", ch );
      return NULL;
   }

   if( keeper->position < POS_SLEEPING )
   {
      send_to_char( "I don't think they can hear you...\r\n", ch );
      return NULL;
   }

   /*
    * Invisible or hidden people.
    */
   if( !can_see( keeper, ch ) )
   {
      do_say( keeper, "I don't trade with folks I can't see." );
      return NULL;
   }

   speakswell = UMIN( knows_language( keeper, ch->speaking, ch ), knows_language( ch, ch->speaking, keeper ) );

   if( ( number_percent(  ) % 65 ) > speakswell )
   {
      if( speakswell > 60 )
         snprintf( buf, MAX_STRING_LENGTH, "%s Could you repeat that?  I didn't quite catch it.", ch->name );
      else if( speakswell > 50 )
         snprintf( buf, MAX_STRING_LENGTH, "%s Could you say that a little more clearly please?", ch->name );
      else if( speakswell > 40 )
         snprintf( buf, MAX_STRING_LENGTH, "%s Sorry... What was that you wanted?", ch->name );
      else
         snprintf( buf, MAX_STRING_LENGTH, "%s I can't understand you.", ch->name );
      do_tell( keeper, buf );
      return NULL;
   }

   return keeper;
}

int get_cost( CHAR_DATA * ch, CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy )
{
   SHOP_DATA *pShop;
   int cost;
   bool richcustomer;
   int profitmod;

   if( !obj || ( pShop = keeper->pIndexData->pShop ) == NULL )
      return 0;

   if( ch->gold > ( ch->level * ch->level * 100000 ) )
      richcustomer = TRUE;
   else
      richcustomer = FALSE;

   if( fBuy )
   {
      profitmod = 13 - get_curr_cha( ch ) + ( richcustomer ? 15 : 0 )
         + ( ( URANGE( 5, ch->level, LEVEL_AVATAR ) - 20 ) / 2 );
      cost = ( int )( obj->cost * UMAX( ( pShop->profit_sell + 1 ), pShop->profit_buy + profitmod ) ) / 100;

      /*
       * Thanks to Nick Gammon for pointing out this line
       * (it was the first line in this block, making it useless) 
       */

      cost = ( int )( cost * ( 80 + UMIN( ch->level, LEVEL_AVATAR ) ) ) / 100;
   }
   else
   {
      OBJ_DATA *obj2;
      int itype;

      profitmod = get_curr_cha( ch ) - 13 - ( richcustomer ? 15 : 0 );
      cost = 0;
      for( itype = 0; itype < MAX_TRADE; itype++ )
      {
         if( obj->item_type == pShop->buy_type[itype] )
         {
            cost = ( int )( obj->cost * UMIN( ( pShop->profit_buy - 1 ), pShop->profit_sell + profitmod ) ) / 100;
            break;
         }
      }

      for( obj2 = keeper->first_carrying; obj2; obj2 = obj2->next_content )
      {
         if( obj->pIndexData == obj2->pIndexData )
         {
            cost = 0;
            break;
         }
      }
   }

   if( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
      cost = ( int )( cost * obj->value[2] / obj->value[1] );

   return cost;
}

int get_repaircost( CHAR_DATA * keeper, OBJ_DATA * obj )
{
   REPAIR_DATA *rShop;
   int cost;
   int itype;
   bool found;

   if( !obj || ( rShop = keeper->pIndexData->rShop ) == NULL )
      return 0;

   cost = 0;
   found = FALSE;
   for( itype = 0; itype < MAX_FIX; itype++ )
   {
      if( obj->item_type == rShop->fix_type[itype] )
      {
         cost = ( int )( obj->cost * rShop->profit_fix / 1000 );
         found = TRUE;
         break;
      }
   }

   if( !found )
      cost = -1;

   if( cost == 0 )
      cost = 1;

   if( found && cost > 0 )
   {
      switch ( obj->item_type )
      {
         case ITEM_ARMOR:
            if( obj->value[0] >= obj->value[1] )
               cost = -2;
            else
               cost *= ( obj->value[1] - obj->value[0] );
            break;
         case ITEM_WEAPON:
            if( INIT_WEAPON_CONDITION == obj->value[0] )
               cost = -2;
            else
               cost *= ( INIT_WEAPON_CONDITION - obj->value[0] );
            break;
         case ITEM_WAND:
         case ITEM_STAFF:
            if( obj->value[2] >= obj->value[1] )
               cost = -2;
            else
               cost *= ( obj->value[1] - obj->value[2] );
      }
   }

   return cost;
}

void do_buy( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   int maxgold;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Buy what?\r\n", ch );
      return;
   }

   if( xIS_SET( ch->in_room->room_flags, ROOM_PET_SHOP ) )
   {
      char buf[MAX_STRING_LENGTH];
      CHAR_DATA *pet;
      ROOM_INDEX_DATA *pRoomIndexNext;
      ROOM_INDEX_DATA *in_room;

      if( IS_NPC( ch ) )
         return;

      pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
      if( !pRoomIndexNext )
      {
         bug( "%s: bad pet shop at vnum %d.", __FUNCTION__, ch->in_room->vnum );
         send_to_char( "Sorry, you can't buy that here.\r\n", ch );
         return;
      }

      in_room = ch->in_room;
      ch->in_room = pRoomIndexNext;
      pet = get_char_room( ch, arg );
      ch->in_room = in_room;

      if( pet == NULL || !IS_NPC( pet ) || !xIS_SET( pet->act, ACT_PET ) )
      {
         send_to_char( "Sorry, you can't buy that here.\r\n", ch );
         return;
      }

      if( xIS_SET( ch->act, PLR_BOUGHT_PET ) )
      {
         send_to_char( "You already bought one pet this level.\r\n", ch );
         return;
      }

      if( ch->gold < 10 * pet->level * pet->level )
      {
         send_to_char( "You can't afford it.\r\n", ch );
         return;
      }

      if( ch->level < pet->level )
      {
         send_to_char( "You're not ready for this pet.\r\n", ch );
         return;
      }

      maxgold = 10 * pet->level * pet->level;
      ch->gold -= maxgold;
      boost_economy( ch->in_room->area, maxgold );
      pet = create_mobile( pet->pIndexData );
      xSET_BIT( ch->act, PLR_BOUGHT_PET );
      xSET_BIT( pet->act, ACT_PET );
      xSET_BIT( pet->affected_by, AFF_CHARM );

      argument = one_argument( argument, arg );
      if( arg[0] != '\0' )
      {
         snprintf( buf, MAX_STRING_LENGTH, "%s %s", pet->name, arg );
         STRFREE( pet->name );
         pet->name = STRALLOC( buf );
      }

      snprintf( buf, MAX_STRING_LENGTH, "%sA neck tag says 'I belong to %s'.\r\n", pet->description, ch->name );
      STRFREE( pet->description );
      pet->description = STRALLOC( buf );

      char_to_room( pet, ch->in_room );
      add_follower( pet, ch );
      send_to_char( "Enjoy your pet.\r\n", ch );
      act( AT_ACTION, "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
      return;
   }
   else
   {
      CHAR_DATA *keeper;
      OBJ_DATA *obj;
      int cost;
      int noi = 1;   /* Number of items */
      short mnoi = 20;  /* Max number of items to be bought at once */

      if( ( keeper = find_keeper( ch ) ) == NULL )
         return;

      maxgold = keeper->level * keeper->level * 50000;

      if( is_number( arg ) )
      {
         noi = atoi( arg );
         argument = one_argument( argument, arg );
         if( noi > mnoi )
         {
            act( AT_TELL, "$n tells you 'I don't sell that many items at" " once.'", keeper, NULL, ch, TO_VICT );
            ch->reply = keeper;
            return;
         }
      }

      obj = get_obj_carry( keeper, arg );
      cost = ( get_cost( ch, keeper, obj, TRUE ) * noi );
      if( cost <= 0 || !can_see_obj( ch, obj ) )
      {
         act( AT_TELL, "$n tells you 'I don't sell that -- try 'list'.'", keeper, NULL, ch, TO_VICT );
         ch->reply = keeper;
         return;
      }

      if( !IS_OBJ_STAT( obj, ITEM_INVENTORY ) && ( noi > 1 ) )
      {
         interpret( keeper, "laugh" );
         act( AT_TELL, "$n tells you 'I don't have enough of those in stock"
              " to sell more than one at a time.'", keeper, NULL, ch, TO_VICT );
         ch->reply = keeper;
         return;
      }

      if( ch->gold < cost )
      {
         act( AT_TELL, "$n tells you 'You can't afford to buy $p.'", keeper, obj, ch, TO_VICT );
         ch->reply = keeper;
         return;
      }

      if( obj->level > ch->level )
      {
         act( AT_TELL, "$n tells you 'You can't use $p yet.'", keeper, obj, ch, TO_VICT );
         ch->reply = keeper;
         return;
      }

      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && get_trust( ch ) < LEVEL_IMMORTAL )
      {
         act( AT_TELL, "$n tells you 'This is a only a prototype!  I can't sell you that...'", keeper, NULL, ch, TO_VICT );
         ch->reply = keeper;
         return;
      }

      if( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
      {
         send_to_char( "You can't carry that many items.\r\n", ch );
         return;
      }

      if( ch->carry_weight + ( get_obj_weight( obj ) * noi ) + ( noi > 1 ? 2 : 0 ) > can_carry_w( ch ) )
      {
         send_to_char( "You can't carry that much weight.\r\n", ch );
         return;
      }

      if( noi == 1 )
      {
         act( AT_ACTION, "$n buys $p.", ch, obj, NULL, TO_ROOM );
         act( AT_ACTION, "You buy $p.", ch, obj, NULL, TO_CHAR );
      }
      else
      {
         snprintf( arg, MAX_INPUT_LENGTH, "$n buys %d $p%s.", noi,
                   ( obj->short_descr[strlen( obj->short_descr ) - 1] == 's' ? "" : "s" ) );
         act( AT_ACTION, arg, ch, obj, NULL, TO_ROOM );
         snprintf( arg, MAX_INPUT_LENGTH, "You buy %d $p%s.", noi,
                   ( obj->short_descr[strlen( obj->short_descr ) - 1] == 's' ? "" : "s" ) );
         act( AT_ACTION, arg, ch, obj, NULL, TO_CHAR );
         act( AT_ACTION, "$N puts them into a bag and hands it to you.", ch, NULL, keeper, TO_CHAR );
      }

      ch->gold -= cost;
      keeper->gold += cost;

      if( keeper->gold > maxgold )
      {
         boost_economy( keeper->in_room->area, keeper->gold - maxgold / 2 );
         keeper->gold = maxgold / 2;
         act( AT_ACTION, "$n puts some gold into a large safe.", keeper, NULL, NULL, TO_ROOM );
      }

      if( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
      {
         OBJ_DATA *buy_obj, *bag;

         buy_obj = create_object( obj->pIndexData, obj->level );

         /*
          * Due to grouped objects and carry limitations in SMAUG
          * The shopkeeper gives you a bag with multiple-buy,
          * and also, only one object needs be created with a count
          * set to the number bought.    -Thoric
          */
         if( noi > 1 )
         {
            bag = create_object( get_obj_index( OBJ_VNUM_SHOPPING_BAG ), 1 );
            xSET_BIT( bag->extra_flags, ITEM_GROUNDROT );
            bag->timer = 10;  /* Blodkai, 4/97 */
            /*
             * perfect size bag ;) 
             */
            bag->value[0] = bag->weight + ( buy_obj->weight * noi );
            buy_obj->count = noi;
            obj->pIndexData->count += ( noi - 1 );
            numobjsloaded += ( noi - 1 );
            obj_to_obj( buy_obj, bag );
            obj_to_char( bag, ch );
         }
         else
            obj_to_char( buy_obj, ch );
      }
      else
      {
         obj_from_char( obj );
         obj_to_char( obj, ch );
      }
      mprog_sell_trigger( keeper, ch, obj );
      return;
   }
}

/*
 * This is a new list function which allows level limits to follow as
 * arguments.  This code relies heavily on the items held by the shopkeeper
 * being sorted in descending order by level.  obj_to_char in handler.c was
 * modified to achieve this.  Anyways, this list command will now throw flags
 * at the levels passed as arguments.  This helps pick out equipment which is
 * usable by the char, etc.  This was done rather than just producing a list
 * of equip at desired level because there would be an inconsistency between
 * #.item on one list and #.item on the other.
 * Syntax:
 *      list            -       list the items for sale, should be sorted by
 *                              level
 *      list #          -       list items and throw a flag at #
 *      list #1 #2      -       list items and throw flags at #1 and #2
 * Note that this won't work in pets stores. Since you can't control
 * the order in which the pets repop you can't guarantee a sorted list.
 * Last Modified: May 25, 1997 -- Fireblade
 */
void do_list( CHAR_DATA* ch, const char* argument)
{
   /*
    * Constants for producing the flags 
    */
   const char *divleft  = "-----------------------------------[ ";
   const char *divright = " ]-----------------------------------";


   if( xIS_SET( ch->in_room->room_flags, ROOM_PET_SHOP ) )
   {
      ROOM_INDEX_DATA *pRoomIndexNext;
      CHAR_DATA *pet;
      bool found;

      pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
      if( !pRoomIndexNext )
      {
         bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
         send_to_char( "You can't do that here.\r\n", ch );
         return;
      }

      found = FALSE;

      for( pet = pRoomIndexNext->first_person; pet; pet = pet->next_in_room )
      {
         if( xIS_SET( pet->act, ACT_PET ) && IS_NPC( pet ) )
         {
            if( !found )
            {
               found = TRUE;
               send_to_pager( "Pets for sale:\r\n", ch );
            }
            pager_printf( ch, "[%2d] %8d - %s\r\n", pet->level, 10 * pet->level * pet->level, pet->short_descr );
         }
      }
      if( !found )
         send_to_char( "Sorry, we're out of pets right now.\r\n", ch );
      return;
   }
   else
   {
      char arg[MAX_INPUT_LENGTH];
      const char *rest;
      CHAR_DATA *keeper;
      OBJ_DATA *obj;
      int cost;
      bool found;
/*      bool listall; */
      int lower, upper;

      rest = one_argument( argument, arg );

      if( ( keeper = find_keeper( ch ) ) == NULL )
         return;

      found = FALSE;
      lower = -2;
      upper = -1;

      /*
       * Get the level limits for the flags 
       */
      if( is_number( arg ) )
      {
         lower = atoi( arg );
         rest = one_argument( rest, arg );

         if( is_number( arg ) )
         {
            upper = atoi( arg );
            rest = one_argument( rest, arg );
         }
      }

      /*
       * Fix the limits if reversed 
       */
      if( lower >= upper )
      {
         int temp;
         temp = lower;
         lower = upper;
         upper = temp;
      }

      /*
       * Loop until you see an object higher level than char 
       */
      /*
       * Note that this depends on the keeper having a sorted list 
       */
      for( obj = keeper->first_carrying; obj; obj = obj->next_content )
      {
         if( obj->wear_loc == WEAR_NONE
             && can_see_obj( ch, obj )
             && ( cost = get_cost( ch, keeper, obj, TRUE ) ) > 0 && ( arg[0] == '\0' || nifty_is_name( arg, obj->name ) ) )
         {
            if( !found )
            {
               found = TRUE;
               send_to_pager( "[Lv Price] Item\r\n", ch );
            }

            if( obj->level <= upper )
            {
               pager_printf( ch, "%s%2d%s\r\n", divleft, upper, divright );
               upper = -1;
            }

            if( obj->level < lower )
            {
               pager_printf( ch, "%s%2d%s\r\n", divleft, lower, divright );
               lower = -1;
            }

            pager_printf( ch, "[%2d %5d] %s.\r\n", obj->level, cost, capitalize( obj->short_descr ) );
         }
      }

      if( lower >= 0 )
      {
         pager_printf( ch, "%s%2d%s\r\n", divleft, lower, divright );
      }

      if( !found )
      {
         if( arg[0] == '\0' )
            send_to_char( "You can't buy anything here.\r\n", ch );
         else
            send_to_char( "You can't buy that here.\r\n", ch );
      }
      return;
   }
}

void do_sell( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *keeper;
   OBJ_DATA *obj;
   int cost;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Sell what?\r\n", ch );
      return;
   }

   if( ( keeper = find_keeper( ch ) ) == NULL )
      return;

   if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
   {
      act( AT_TELL, "$n tells you 'You don't have that item.'", keeper, NULL, ch, TO_VICT );
      ch->reply = keeper;
      return;
   }

   /*
    * Bug report and solution thanks to animal@netwin.co.nz 
    */
   if( !can_see_obj( keeper, obj ) )
   {
      send_to_char( "What are you trying to sell me? I don't buy thin air!\r\n", ch );
      return;
   }

   if( !can_drop_obj( ch, obj ) )
   {
      send_to_char( "You can't let go of it!\r\n", ch );
      return;
   }

   if( obj->timer > 0 )
   {
      act( AT_TELL, "$n tells you, '$p is depreciating in value too quickly...'", keeper, obj, ch, TO_VICT );
      return;
   }

   if( ( cost = get_cost( ch, keeper, obj, FALSE ) ) <= 0 )
   {
      act( AT_ACTION, "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
      return;
   }

   if( cost >= keeper->gold )
   {
      act( AT_TELL, "$n tells you, '$p is worth more than I can afford...'", keeper, obj, ch, TO_VICT );
      return;
   }

   separate_obj( obj );
   act( AT_ACTION, "$n sells $p.", ch, obj, NULL, TO_ROOM );
   snprintf( buf, MAX_STRING_LENGTH, "You sell $p for %d gold piece%s.", cost, cost == 1 ? "" : "s" );
   act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
   ch->gold += cost;
   keeper->gold -= cost;
   if( keeper->gold < 0 )
      keeper->gold = 0;

   if( obj->item_type == ITEM_TRASH )
      extract_obj( obj );
   else
   {
      obj_from_char( obj );
      obj_to_char( obj, keeper );
   }

   return;
}



void do_value( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *keeper;
   OBJ_DATA *obj;
   int cost;

   if( argument[0] == '\0' )
   {
      send_to_char( "Value what?\r\n", ch );
      return;
   }

   if( ( keeper = find_keeper( ch ) ) == NULL )
      return;

   if( ( obj = get_obj_carry( ch, argument ) ) == NULL )
   {
      act( AT_TELL, "$n tells you 'You don't have that item.'", keeper, NULL, ch, TO_VICT );
      ch->reply = keeper;
      return;
   }

   if( !can_drop_obj( ch, obj ) )
   {
      send_to_char( "You can't let go of it!\r\n", ch );
      return;
   }

   if( ( cost = get_cost( ch, keeper, obj, FALSE ) ) <= 0 )
   {
      act( AT_ACTION, "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
      return;
   }

   snprintf( buf, MAX_STRING_LENGTH, "$n tells you 'I'll give you %d gold coins for $p.'", cost );
   act( AT_TELL, buf, keeper, obj, ch, TO_VICT );
   ch->reply = keeper;

   return;
}

/*
 * Repair a single object. Used when handling "repair all" - Gorog
 */
void repair_one_obj( CHAR_DATA * ch, CHAR_DATA * keeper, OBJ_DATA * obj,
                     const char *arg, int maxgold, const char *fixstr, const char *fixstr2 )
{
   char buf[MAX_STRING_LENGTH];
   int cost;

   if( !can_drop_obj( ch, obj ) )
      ch_printf( ch, "You can't let go of %s.\r\n", obj->name );
   else if( ( cost = get_repaircost( keeper, obj ) ) < 0 )
   {
      if( cost != -2 )
         act( AT_TELL, "$n tells you, 'Sorry, I can't do anything with $p.'", keeper, obj, ch, TO_VICT );
      else
         act( AT_TELL, "$n tells you, '$p looks fine to me!'", keeper, obj, ch, TO_VICT );
   }
   /*
    * "repair all" gets a 10% surcharge - Gorog 
    */

   else if( ( cost = strcmp( "all", arg ) ? cost : 11 * cost / 10 ) > ch->gold )
   {
      snprintf( buf, MAX_STRING_LENGTH,
                "$N tells you, 'It will cost %d piece%s of gold to %s %s...'", cost,
                cost == 1 ? "" : "s", fixstr, obj->name );
      act( AT_TELL, buf, ch, NULL, keeper, TO_CHAR );
      act( AT_TELL, "$N tells you, 'Which I see you can't afford.'", ch, NULL, keeper, TO_CHAR );
   }
   else
   {
      snprintf( buf, MAX_STRING_LENGTH, "$n gives $p to $N, who quickly %s it.", fixstr2 );
      act( AT_ACTION, buf, ch, obj, keeper, TO_ROOM );
      snprintf( buf, MAX_STRING_LENGTH, "$N charges you %d gold piece%s to %s $p.", cost, cost == 1 ? "" : "s", fixstr );
      act( AT_ACTION, buf, ch, obj, keeper, TO_CHAR );
      ch->gold -= cost;
      keeper->gold += cost;
      if( keeper->gold < 0 )
         keeper->gold = 0;
      else if( keeper->gold > maxgold )
      {
         boost_economy( keeper->in_room->area, keeper->gold - maxgold / 2 );
         keeper->gold = maxgold / 2;
         act( AT_ACTION, "$n puts some gold into a large safe.", keeper, NULL, NULL, TO_ROOM );
      }

      switch ( obj->item_type )
      {
         default:
            send_to_char( "For some reason, you think you got ripped off...\r\n", ch );
            break;
         case ITEM_ARMOR:
            obj->value[0] = obj->value[1];
            break;
         case ITEM_WEAPON:
            obj->value[0] = INIT_WEAPON_CONDITION;
            break;
         case ITEM_WAND:
         case ITEM_STAFF:
            obj->value[2] = obj->value[1];
            break;
      }

      oprog_repair_trigger( ch, obj );
   }
}

void do_repair( CHAR_DATA* ch, const char* argument)
{
   CHAR_DATA *keeper;
   OBJ_DATA *obj;
   const char *fixstr;
   const char *fixstr2;
   int maxgold;

   if( argument[0] == '\0' )
   {
      send_to_char( "Repair what?\r\n", ch );
      return;
   }

   if( ( keeper = find_fixer( ch ) ) == NULL )
      return;

   maxgold = keeper->level * keeper->level * 100000;
   switch ( keeper->pIndexData->rShop->shop_type )
   {
      default:
      case SHOP_FIX:
         fixstr = "repair";
         fixstr2 = "repairs";
         break;
      case SHOP_RECHARGE:
         fixstr = "recharge";
         fixstr2 = "recharges";
         break;
   }

   if( !strcmp( argument, "all" ) )
   {
      for( obj = ch->first_carrying; obj; obj = obj->next_content )
      {
         if( obj->wear_loc == WEAR_NONE
             && can_see_obj( ch, obj )
             && can_see_obj( keeper, obj )
             && ( obj->item_type == ITEM_ARMOR
                  || obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_WAND || obj->item_type == ITEM_STAFF ) )
            repair_one_obj( ch, keeper, obj, argument, maxgold, fixstr, fixstr2 );
      }
      return;
   }

   if( ( obj = get_obj_carry( ch, argument ) ) == NULL )
   {
      act( AT_TELL, "$n tells you 'You don't have that item.'", keeper, NULL, ch, TO_VICT );
      ch->reply = keeper;
      return;
   }

   repair_one_obj( ch, keeper, obj, argument, maxgold, fixstr, fixstr2 );
}

void appraise_all( CHAR_DATA * ch, CHAR_DATA * keeper, const char *fixstr )
{
   OBJ_DATA *obj;
   char buf[MAX_STRING_LENGTH];
   int cost = 0, total = 0;

   for( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
   {
      if( obj->wear_loc == WEAR_NONE
          && can_see_obj( ch, obj )
          && ( obj->item_type == ITEM_ARMOR
               || obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_WAND || obj->item_type == ITEM_STAFF ) )
      {

         if( !can_drop_obj( ch, obj ) )
            ch_printf( ch, "You can't let go of %s.\r\n", obj->name );
         else if( ( cost = get_repaircost( keeper, obj ) ) < 0 )
         {
            if( cost != -2 )
               act( AT_TELL, "$n tells you, 'Sorry, I can't do anything with $p.'", keeper, obj, ch, TO_VICT );
            else
               act( AT_TELL, "$n tells you, '$p looks fine to me!'", keeper, obj, ch, TO_VICT );
         }
         else
         {
            snprintf( buf, MAX_STRING_LENGTH,
                      "$N tells you, 'It will cost %d piece%s of gold to %s %s'",
                      cost, cost == 1 ? "" : "s", fixstr, obj->name );
            act( AT_TELL, buf, ch, NULL, keeper, TO_CHAR );
            total += cost;
         }
      }
   }
   if( total > 0 )
   {
      send_to_char( "\r\n", ch );
      snprintf( buf, MAX_STRING_LENGTH, "$N tells you, 'It will cost %d piece%s of gold in total.'", total,
                cost == 1 ? "" : "s" );
      act( AT_TELL, buf, ch, NULL, keeper, TO_CHAR );
      act( AT_TELL, "$N tells you, 'Remember there is a 10% surcharge for repair all.'", ch, NULL, keeper, TO_CHAR );
   }
}

void do_appraise( CHAR_DATA* ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *keeper;
   OBJ_DATA *obj;
   int cost;
   const char *fixstr;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Appraise what?\r\n", ch );
      return;
   }

   if( ( keeper = find_fixer( ch ) ) == NULL )
      return;

   switch ( keeper->pIndexData->rShop->shop_type )
   {
      default:
      case SHOP_FIX:
         fixstr = "repair";
         break;
      case SHOP_RECHARGE:
         fixstr = "recharge";
         break;
   }

   if( !strcmp( arg, "all" ) )
   {
      appraise_all( ch, keeper, fixstr );
      return;
   }

   if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
   {
      act( AT_TELL, "$n tells you 'You don't have that item.'", keeper, NULL, ch, TO_VICT );
      ch->reply = keeper;
      return;
   }

   if( !can_drop_obj( ch, obj ) )
   {
      send_to_char( "You can't let go of it.\r\n", ch );
      return;
   }

   if( ( cost = get_repaircost( keeper, obj ) ) < 0 )
   {
      if( cost != -2 )
         act( AT_TELL, "$n tells you, 'Sorry, I can't do anything with $p.'", keeper, obj, ch, TO_VICT );
      else
         act( AT_TELL, "$n tells you, '$p looks fine to me!'", keeper, obj, ch, TO_VICT );
      return;
   }

   snprintf( buf, MAX_STRING_LENGTH, "$N tells you, 'It will cost %d piece%s of gold to %s that...'", cost,
             cost == 1 ? "" : "s", fixstr );
   act( AT_TELL, buf, ch, NULL, keeper, TO_CHAR );
   if( cost > ch->gold )
      act( AT_TELL, "$N tells you, 'Which I see you can't afford.'", ch, NULL, keeper, TO_CHAR );

   return;
}


/* ------------------ Shop Building and Editing Section ----------------- */


void do_makeshop( CHAR_DATA* ch, const char* argument)
{
   SHOP_DATA *shop;
   int vnum;
   MOB_INDEX_DATA *mob;

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: makeshop <mobvnum>\r\n", ch );
      return;
   }

   vnum = atoi( argument );

   if( ( mob = get_mob_index( vnum ) ) == NULL )
   {
      send_to_char( "Mobile not found.\r\n", ch );
      return;
   }

   if( !can_medit( ch, mob ) )
      return;

   if( mob->pShop )
   {
      send_to_char( "This mobile already has a shop.\r\n", ch );
      return;
   }

   CREATE( shop, SHOP_DATA, 1 );

   LINK( shop, first_shop, last_shop, next, prev );
   shop->keeper = vnum;
   shop->profit_buy = 120;
   shop->profit_sell = 90;
   shop->open_hour = 0;
   shop->close_hour = 23;
   mob->pShop = shop;
   send_to_char( "Done.\r\n", ch );
   return;
}


void do_shopset( CHAR_DATA* ch, const char* argument)
{
   SHOP_DATA *shop;
   MOB_INDEX_DATA *mob, *mob2;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int vnum, value;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Usage: shopset <mob vnum> <field> value\r\n", ch );
      send_to_char( "\r\nField being one of:\r\n", ch );
      send_to_char( "  buy0 buy1 buy2 buy3 buy4 buy sell open close keeper\r\n", ch );
      return;
   }

   vnum = atoi( arg1 );

   if( ( mob = get_mob_index( vnum ) ) == NULL )
   {
      send_to_char( "Mobile not found.\r\n", ch );
      return;
   }

   if( !can_medit( ch, mob ) )
      return;

   if( !mob->pShop )
   {
      send_to_char( "This mobile doesn't keep a shop.\r\n", ch );
      return;
   }
   shop = mob->pShop;
   value = atoi( argument );

   if( !str_cmp( arg2, "buy0" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         send_to_char( "Invalid item type!\r\n", ch );
         return;
      }
      shop->buy_type[0] = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "buy1" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         send_to_char( "Invalid item type!\r\n", ch );
         return;
      }
      shop->buy_type[1] = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "buy2" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         send_to_char( "Invalid item type!\r\n", ch );
         return;
      }
      shop->buy_type[2] = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "buy3" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         send_to_char( "Invalid item type!\r\n", ch );
         return;
      }
      shop->buy_type[3] = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "buy4" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         send_to_char( "Invalid item type!\r\n", ch );
         return;
      }
      shop->buy_type[4] = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "buy" ) )
   {
      if( value <= ( shop->profit_sell + 5 ) || value > 1000 )
      {
         send_to_char( "Out of range.\r\n", ch );
         return;
      }
      shop->profit_buy = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "sell" ) )
   {
      if( value < 0 || value >= ( shop->profit_buy - 5 ) )
      {
         send_to_char( "Out of range.\r\n", ch );
         return;
      }
      shop->profit_sell = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "open" ) )
   {
      if( value < 0 || value > 23 )
      {
         send_to_char( "Out of range.\r\n", ch );
         return;
      }
      shop->open_hour = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "close" ) )
   {
      if( value < 0 || value > 23 )
      {
         send_to_char( "Out of range.\r\n", ch );
         return;
      }
      shop->close_hour = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "keeper" ) )
   {
      if( ( mob2 = get_mob_index( vnum ) ) == NULL )
      {
         send_to_char( "Mobile not found.\r\n", ch );
         return;
      }
      if( !can_medit( ch, mob ) )
         return;
      if( mob2->pShop )
      {
         send_to_char( "That mobile already has a shop.\r\n", ch );
         return;
      }
      mob->pShop = NULL;
      mob2->pShop = shop;
      shop->keeper = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   do_shopset( ch, "" );
   return;
}


void do_shopstat( CHAR_DATA* ch, const char* argument)
{
   SHOP_DATA *shop;
   MOB_INDEX_DATA *mob;
   int vnum;

   if( argument[0] == '\0' )
   {
      send_to_char( "Usage: shopstat <keeper vnum>\r\n", ch );
      return;
   }

   vnum = atoi( argument );

   if( ( mob = get_mob_index( vnum ) ) == NULL )
   {
      send_to_char( "Mobile not found.\r\n", ch );
      return;
   }

   if( !mob->pShop )
   {
      send_to_char( "This mobile doesn't keep a shop.\r\n", ch );
      return;
   }
   shop = mob->pShop;

   ch_printf( ch, "Keeper: %d  %s\r\n", shop->keeper, mob->short_descr );
   ch_printf( ch, "buy0 [%s]  buy1 [%s]  buy2 [%s]  buy3 [%s]  buy4 [%s]\r\n",
              o_types[shop->buy_type[0]],
              o_types[shop->buy_type[1]],
              o_types[shop->buy_type[2]], o_types[shop->buy_type[3]], o_types[shop->buy_type[4]] );
   ch_printf( ch, "Profit:  buy %3d%%  sell %3d%%\r\n", shop->profit_buy, shop->profit_sell );
   ch_printf( ch, "Hours:   open %2d  close %2d\r\n", shop->open_hour, shop->close_hour );
   return;
}


void do_shops( CHAR_DATA* ch, const char* argument)
{
   SHOP_DATA *shop;

   if( !first_shop )
   {
      send_to_char( "There are no shops.\r\n", ch );
      return;
   }

   set_char_color( AT_NOTE, ch );
   for( shop = first_shop; shop; shop = shop->next )
      ch_printf( ch, "Keeper: %5d Buy: %3d Sell: %3d Open: %2d Close: %2d Buy: %2d %2d %2d %2d %2d\r\n",
                 shop->keeper, shop->profit_buy, shop->profit_sell,
                 shop->open_hour, shop->close_hour,
                 shop->buy_type[0], shop->buy_type[1], shop->buy_type[2], shop->buy_type[3], shop->buy_type[4] );
   return;
}


/* -------------- Repair Shop Building and Editing Section -------------- */


void do_makerepair( CHAR_DATA* ch, const char* argument)
{
   REPAIR_DATA *repair;
   int vnum;
   MOB_INDEX_DATA *mob;

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: makerepair <mobvnum>\r\n", ch );
      return;
   }

   vnum = atoi( argument );

   if( ( mob = get_mob_index( vnum ) ) == NULL )
   {
      send_to_char( "Mobile not found.\r\n", ch );
      return;
   }

   if( !can_medit( ch, mob ) )
      return;

   if( mob->rShop )
   {
      send_to_char( "This mobile already has a repair shop.\r\n", ch );
      return;
   }

   CREATE( repair, REPAIR_DATA, 1 );

   LINK( repair, first_repair, last_repair, next, prev );
   repair->keeper = vnum;
   repair->profit_fix = 100;
   repair->shop_type = SHOP_FIX;
   repair->open_hour = 0;
   repair->close_hour = 23;
   mob->rShop = repair;
   send_to_char( "Done.\r\n", ch );
   return;
}

void do_repairset( CHAR_DATA* ch, const char* argument)
{
   REPAIR_DATA *repair;
   MOB_INDEX_DATA *mob, *mob2;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int vnum, value;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Usage: repairset <mob vnum> <field> value\r\n", ch );
      send_to_char( "\r\nField being one of:\r\n", ch );
      send_to_char( "  fix0 fix1 fix2 profit type open close keeper\r\n", ch );
      return;
   }

   vnum = atoi( arg1 );

   if( ( mob = get_mob_index( vnum ) ) == NULL )
   {
      send_to_char( "Mobile not found.\r\n", ch );
      return;
   }

   if( !can_medit( ch, mob ) )
      return;

   if( !mob->rShop )
   {
      send_to_char( "This mobile doesn't keep a repair shop.\r\n", ch );
      return;
   }
   repair = mob->rShop;
   value = atoi( argument );

   if( !str_cmp( arg2, "fix0" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         send_to_char( "Invalid item type!\r\n", ch );
         return;
      }
      repair->fix_type[0] = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "fix1" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         send_to_char( "Invalid item type!\r\n", ch );
         return;
      }
      repair->fix_type[1] = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "fix2" ) )
   {
      if( !is_number( argument ) )
         value = get_otype( argument );
      if( value < 0 || value > MAX_ITEM_TYPE )
      {
         send_to_char( "Invalid item type!\r\n", ch );
         return;
      }
      repair->fix_type[2] = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "profit" ) )
   {
      if( value < 1 || value > 1000 )
      {
         send_to_char( "Out of range.\r\n", ch );
         return;
      }
      repair->profit_fix = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "type" ) )
   {
      if( value < 1 || value > 2 )
      {
         send_to_char( "Out of range.\r\n", ch );
         return;
      }
      repair->shop_type = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "open" ) )
   {
      if( value < 0 || value > 23 )
      {
         send_to_char( "Out of range.\r\n", ch );
         return;
      }
      repair->open_hour = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "close" ) )
   {
      if( value < 0 || value > 23 )
      {
         send_to_char( "Out of range.\r\n", ch );
         return;
      }
      repair->close_hour = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "keeper" ) )
   {
      if( ( mob2 = get_mob_index( vnum ) ) == NULL )
      {
         send_to_char( "Mobile not found.\r\n", ch );
         return;
      }
      if( !can_medit( ch, mob ) )
         return;
      if( mob2->rShop )
      {
         send_to_char( "That mobile already has a repair shop.\r\n", ch );
         return;
      }
      mob->rShop = NULL;
      mob2->rShop = repair;
      repair->keeper = value;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   do_repairset( ch, "" );
   return;
}


void do_repairstat( CHAR_DATA* ch, const char* argument)
{
   REPAIR_DATA *repair;
   MOB_INDEX_DATA *mob;
   int vnum;

   if( argument[0] == '\0' )
   {
      send_to_char( "Usage: repairstat <keeper vnum>\r\n", ch );
      return;
   }

   vnum = atoi( argument );

   if( ( mob = get_mob_index( vnum ) ) == NULL )
   {
      send_to_char( "Mobile not found.\r\n", ch );
      return;
   }

   if( !mob->rShop )
   {
      send_to_char( "This mobile doesn't keep a repair shop.\r\n", ch );
      return;
   }
   repair = mob->rShop;

   ch_printf( ch, "Keeper: %d  %s\r\n", repair->keeper, mob->short_descr );
   ch_printf( ch, "fix0 [%s]  fix1 [%s]  fix2 [%s]\r\n",
              o_types[repair->fix_type[0]], o_types[repair->fix_type[1]], o_types[repair->fix_type[2]] );
   ch_printf( ch, "Profit: %3d%%  Type: %d\r\n", repair->profit_fix, repair->shop_type );
   ch_printf( ch, "Hours:   open %2d  close %2d\r\n", repair->open_hour, repair->close_hour );
   return;
}

void do_repairshops( CHAR_DATA* ch, const char* argument)
{
   REPAIR_DATA *repair;

   if( !first_repair )
   {
      send_to_char( "There are no repair shops.\r\n", ch );
      return;
   }

   set_char_color( AT_NOTE, ch );
   for( repair = first_repair; repair; repair = repair->next )
      ch_printf( ch, "Keeper: %5d Profit: %3d Type: %d Open: %2d Close: %2d Fix: %2d %2d %2d\r\n",
                 repair->keeper, repair->profit_fix, repair->shop_type,
                 repair->open_hour, repair->close_hour, repair->fix_type[0], repair->fix_type[1], repair->fix_type[2] );
   return;
}
