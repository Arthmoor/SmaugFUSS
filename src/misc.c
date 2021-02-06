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
 *          Misc module for general commands: not skills or spells          *
 ****************************************************************************
 * Note: Most of the stuff in here would go in act_obj.c, but act_obj was   *
 * getting big.                                                             *
 ****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "mud.h"

void do_eat( CHAR_DATA* ch, const char* argument )
{
   char buf[MAX_STRING_LENGTH];
   OBJ_DATA *obj;
   ch_ret retcode;
   int foodcond;
   bool hgflag = TRUE;

   if( argument[0] == '\0' )
   {
      send_to_char( "Eat what?\r\n", ch );
      return;
   }

   if( IS_NPC( ch ) || ch->pcdata->condition[COND_FULL] > 5 )
      if( ms_find_obj( ch ) )
         return;

   if( ( obj = find_obj( ch, argument, TRUE ) ) == NULL )
      return;

   if( !IS_IMMORTAL( ch ) )
   {
      if( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL && obj->item_type != ITEM_COOK )
      {
         act( AT_ACTION, "$n starts to nibble on $p... ($e must really be hungry)", ch, obj, NULL, TO_ROOM );
         act( AT_ACTION, "You try to nibble on $p...", ch, obj, NULL, TO_CHAR );
         return;
      }

      if( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] > 40 )
      {
         send_to_char( "You are too full to eat more.\r\n", ch );
         return;
      }
   }

   if( !IS_NPC( ch ) && ( !IS_PKILL( ch ) || ( IS_PKILL( ch ) && !IS_SET( ch->pcdata->flags, PCFLAG_HIGHGAG ) ) ) )
      hgflag = FALSE;

   /*
    * required due to object grouping 
    */
   separate_obj( obj );
   if( obj->in_obj )
   {
      if( !hgflag )
         act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
      act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
   }
   if( ch->fighting && number_percent(  ) > ( get_curr_dex( ch ) * 2 + 47 ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s",
                ( ch->in_room->sector_type == SECT_UNDERWATER ||
                  ch->in_room->sector_type == SECT_WATER_SWIM ||
                  ch->in_room->sector_type == SECT_WATER_NOSWIM ) ? "dissolves in the water" :
                ( ch->in_room->sector_type == SECT_AIR ||
                  xIS_SET( ch->in_room->room_flags, ROOM_NOFLOOR ) ) ? "falls far below" : "is trampled underfoot" );
      act( AT_MAGIC, "$n drops $p, and it $T.", ch, obj, buf, TO_ROOM );
      if( !hgflag )
         act( AT_MAGIC, "Oops, $p slips from your hand and $T!", ch, obj, buf, TO_CHAR );
   }
   else
   {
      if( !oprog_use_trigger( ch, obj, NULL, NULL ) )
      {
         if( !obj->action_desc || obj->action_desc[0] == '\0' )
         {
            act( AT_ACTION, "$n eats $p.", ch, obj, NULL, TO_ROOM );
            if( !hgflag )
               act( AT_ACTION, "You eat $p.", ch, obj, NULL, TO_CHAR );
         }
         else
            actiondesc( ch, obj );
      }

      switch ( obj->item_type )
      {

         case ITEM_COOK:
         case ITEM_FOOD:
            WAIT_STATE( ch, PULSE_PER_SECOND / 3 );
            if( obj->timer > 0 && obj->value[1] > 0 )
               foodcond = ( obj->timer * 10 ) / obj->value[1];
            else
               foodcond = 10;

            if( !IS_NPC( ch ) )
            {
               int condition;

               condition = ch->pcdata->condition[COND_FULL];
               gain_condition( ch, COND_FULL, ( obj->value[0] * foodcond ) / 10 );
               if( condition <= 1 && ch->pcdata->condition[COND_FULL] > 1 )
                  send_to_char( "You are no longer hungry.\r\n", ch );
               else if( ch->pcdata->condition[COND_FULL] > 40 )
                  send_to_char( "You are full.\r\n", ch );
            }

            if( obj->value[3] != 0
                || ( foodcond < 4 && number_range( 0, foodcond + 1 ) == 0 )
                || ( obj->item_type == ITEM_COOK && obj->value[2] == 0 ) )
            {
               /*
                * The food was poisoned! 
                */
               AFFECT_DATA af;

               if( obj->value[3] != 0 )
               {
                  act( AT_POISON, "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
                  act( AT_POISON, "You choke and gag.", ch, NULL, NULL, TO_CHAR );
                  ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
               }
               else
               {
                  act( AT_POISON, "$n gags on $p.", ch, obj, NULL, TO_ROOM );
                  act( AT_POISON, "You gag on $p.", ch, obj, NULL, TO_CHAR );
                  ch->mental_state = URANGE( 15, ch->mental_state + 5, 100 );
               }

               af.type = gsn_poison;
               af.duration = 2 * obj->value[0] * ( obj->value[3] > 0 ? obj->value[3] : 1 );
               af.location = APPLY_NONE;
               af.modifier = 0;
               af.bitvector = meb( AFF_POISON );
               affect_join( ch, &af );
            }
            break;

         case ITEM_PILL:
            sysdata.upill_val += obj->cost / 100;
            if( who_fighting( ch ) && IS_PKILL( ch ) )
               WAIT_STATE( ch, PULSE_PER_SECOND / 4 );
            else
               WAIT_STATE( ch, PULSE_PER_SECOND / 3 );
            /*
             * allow pills to fill you, if so desired 
             */
            if( !IS_NPC( ch ) && obj->value[4] )
            {
               int condition;

               condition = ch->pcdata->condition[COND_FULL];
               gain_condition( ch, COND_FULL, obj->value[4] );
               if( condition <= 1 && ch->pcdata->condition[COND_FULL] > 1 )
                  send_to_char( "You are no longer hungry.\r\n", ch );
               else if( ch->pcdata->condition[COND_FULL] > 40 )
                  send_to_char( "You are full.\r\n", ch );
            }
            retcode = obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
            if( retcode == rNONE )
               retcode = obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
            if( retcode == rNONE )
               retcode = obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
            break;
      }

   }
   if( obj->serial == cur_obj )
      global_objcode = rOBJ_EATEN;
   extract_obj( obj );
   return;
}

void do_quaff( CHAR_DATA* ch, const char* argument)
{
   OBJ_DATA *obj;
   ch_ret retcode;
   bool hgflag = TRUE;

   if( argument[0] == '\0' || !str_cmp( argument, "" ) )
   {
      send_to_char( "Quaff what?\r\n", ch );
      return;
   }

   if( ( obj = find_obj( ch, argument, TRUE ) ) == NULL )
      return;

   if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
      return;

   if( obj->item_type != ITEM_POTION )
   {
      if( obj->item_type == ITEM_DRINK_CON )
         do_drink( ch, obj->name );
      else
      {
         act( AT_ACTION, "$n lifts $p up to $s mouth and tries to drink from it...", ch, obj, NULL, TO_ROOM );
         act( AT_ACTION, "You bring $p up to your mouth and try to drink from it...", ch, obj, NULL, TO_CHAR );
      }
      return;
   }

   /*
    * Empty container check              -Shaddai
    */
   if( obj->value[1] == -1 && obj->value[2] == -1 && obj->value[3] == -1 )
   {
      send_to_char( "You suck in nothing but air.\r\n", ch );
      return;
   }
   /*
    * Fullness checking               -Thoric
    */
   if( !IS_NPC( ch ) && ( ch->pcdata->condition[COND_FULL] >= 48 || ch->pcdata->condition[COND_THIRST] >= 48 ) )
   {
      send_to_char( "Your stomach cannot contain any more.\r\n", ch );
      return;
   }

   /*
    * People with nuisance flag feels up quicker. -- Shaddai 
    * Yeah so I can't spell I'm a coder :P --Shaddai 
    * You are now adept at feeling up quickly! -- Blod 
    */
   if( !IS_NPC( ch ) && ch->pcdata->nuisance &&
       ch->pcdata->nuisance->flags > 3
       && ( ch->pcdata->condition[COND_FULL] >= ( 48 - ( 3 * ch->pcdata->nuisance->flags ) + ch->pcdata->nuisance->power )
            || ch->pcdata->condition[COND_THIRST] >= ( 48 - ( ch->pcdata->nuisance->flags ) + ch->pcdata->nuisance->power ) ) )
   {
      send_to_char( "Your stomach cannot contain any more.\r\n", ch );
      return;
   }

   if( !IS_NPC( ch ) && ( !IS_PKILL( ch ) || ( IS_PKILL( ch ) && !IS_SET( ch->pcdata->flags, PCFLAG_HIGHGAG ) ) ) )
      hgflag = FALSE;

   separate_obj( obj );
   if( obj->in_obj )
   {
      if( !CAN_PKILL( ch ) )
      {
         act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
         act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
      }
   }

   /*
    * If fighting, chance of dropping potion         -Thoric
    */
   if( ch->fighting && number_percent(  ) > ( get_curr_dex( ch ) * 2 + 48 ) )
   {
      act( AT_MAGIC, "$n fumbles $p and shatters it into fragments.", ch, obj, NULL, TO_ROOM );
      if( !hgflag )
         act( AT_MAGIC, "Oops... $p is knocked from your hand and shatters!", ch, obj, NULL, TO_CHAR );
   }
   else
   {
      if( !oprog_use_trigger( ch, obj, NULL, NULL ) )
      {
         if( !CAN_PKILL( ch ) || !obj->in_obj )
         {
            act( AT_ACTION, "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
            if( !hgflag )
               act( AT_ACTION, "You quaff $p.", ch, obj, NULL, TO_CHAR );
         }
         else if( obj->in_obj )
         {
            act( AT_ACTION, "$n quaffs $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
            if( !hgflag )
               act( AT_ACTION, "You quaff $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
         }
      }

      if( who_fighting( ch ) && IS_PKILL( ch ) )
         WAIT_STATE( ch, PULSE_PER_SECOND / 5 );
      else
         WAIT_STATE( ch, PULSE_PER_SECOND / 3 );

      gain_condition( ch, COND_THIRST, 1 );
      if( !IS_NPC( ch ) && ch->pcdata->condition[COND_THIRST] > 43 )
         act( AT_ACTION, "You are getting full.", ch, NULL, NULL, TO_CHAR );
      retcode = obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
      if( retcode == rNONE )
         retcode = obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
      if( retcode == rNONE )
         retcode = obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
   }

   if( obj->pIndexData->vnum == OBJ_VNUM_FLASK_BREWING )
      sysdata.brewed_used++;
   else
      sysdata.upotion_val += obj->cost / 100;
   if( cur_obj == obj->serial )
      global_objcode = rOBJ_QUAFFED;
   extract_obj( obj );
   return;
}

void do_recite( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *scroll;
   OBJ_DATA *obj;
   ch_ret retcode;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Recite what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( scroll = get_obj_carry( ch, arg1 ) ) == NULL )
   {
      send_to_char( "You do not have that scroll.\r\n", ch );
      return;
   }

   if( scroll->item_type != ITEM_SCROLL )
   {
      act( AT_ACTION, "$n holds up $p as if to recite something from it...", ch, scroll, NULL, TO_ROOM );
      act( AT_ACTION, "You hold up $p and stand there with your mouth open.  (Now what?)", ch, scroll, NULL, TO_CHAR );
      return;
   }

   if( IS_NPC( ch ) && ( scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING ) )
   {
      send_to_char( "As a mob, this dialect is foreign to you.\r\n", ch );
      return;
   }

   if( ( scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING ) && ( ch->level + 10 < scroll->value[0] ) )
   {
      send_to_char( "This scroll is too complex for you to understand.\r\n", ch );
      return;
   }

   obj = NULL;
   if( arg2[0] == '\0' )
      victim = ch;
   else
   {
      if( ( victim = get_char_room( ch, arg2 ) ) == NULL && ( obj = get_obj_here( ch, arg2 ) ) == NULL )
      {
         send_to_char( "You can't find it.\r\n", ch );
         return;
      }
   }

   if( scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING )
      sysdata.scribed_used++;
   separate_obj( scroll );
   act( AT_MAGIC, "$n recites $p.", ch, scroll, NULL, TO_ROOM );
   act( AT_MAGIC, "You recite $p.", ch, scroll, NULL, TO_CHAR );

   if( victim != ch )
      WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
   else
      WAIT_STATE( ch, PULSE_PER_SECOND / 2 );

   retcode = obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
   if( retcode == rNONE )
      retcode = obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
   if( retcode == rNONE )
      retcode = obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );

   if( scroll->serial == cur_obj )
      global_objcode = rOBJ_USED;
   extract_obj( scroll );
   return;
}

/*
 * Function to handle the state changing of a triggerobject (lever)  -Thoric
 */
void pullorpush( CHAR_DATA * ch, OBJ_DATA * obj, bool pull )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *rch;
   bool isup;
   ROOM_INDEX_DATA *room, *to_room;
   EXIT_DATA *pexit, *pexit_rev;
   int edir;
   const char *txt;

   if( IS_SET( obj->value[0], TRIG_UP ) )
      isup = TRUE;
   else
      isup = FALSE;

   switch ( obj->item_type )
   {
      default:
         ch_printf( ch, "You can't %s that!\r\n", pull ? "pull" : "push" );
         return;
         break;
      case ITEM_SWITCH:
      case ITEM_LEVER:
      case ITEM_PULLCHAIN:
         if( ( !pull && isup ) || ( pull && !isup ) )
         {
            ch_printf( ch, "It is already %s.\r\n", isup ? "up" : "down" );
            return;
         }
      case ITEM_BUTTON:
         if( ( !pull && isup ) || ( pull && !isup ) )
         {
            ch_printf( ch, "It is already %s.\r\n", isup ? "in" : "out" );
            return;
         }
         break;
   }

   if( ( pull ) && HAS_PROG( obj->pIndexData, PULL_PROG ) )
   {
      if( !IS_SET( obj->value[0], TRIG_AUTORETURN ) )
         REMOVE_BIT( obj->value[0], TRIG_UP );
      oprog_pull_trigger( ch, obj );
      return;
   }

   if( ( !pull ) && HAS_PROG( obj->pIndexData, PUSH_PROG ) )
   {
      if( !IS_SET( obj->value[0], TRIG_AUTORETURN ) )
         SET_BIT( obj->value[0], TRIG_UP );
      oprog_push_trigger( ch, obj );
      return;
   }

   if( !oprog_use_trigger( ch, obj, NULL, NULL ) )
   {
      snprintf( buf, MAX_STRING_LENGTH, "$n %s $p.", pull ? "pulls" : "pushes" );
      act( AT_ACTION, buf, ch, obj, NULL, TO_ROOM );
      snprintf( buf, MAX_STRING_LENGTH, "You %s $p.", pull ? "pull" : "push" );
      act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
   }

   if( !IS_SET( obj->value[0], TRIG_AUTORETURN ) )
   {
      if( pull )
         REMOVE_BIT( obj->value[0], TRIG_UP );
      else
         SET_BIT( obj->value[0], TRIG_UP );
   }

   if( IS_SET( obj->value[0], TRIG_TELEPORT )
       || IS_SET( obj->value[0], TRIG_TELEPORTALL ) || IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )
   {
      int flags;

      if( ( room = get_room_index( obj->value[1] ) ) == NULL )
      {
         bug( "%s: obj points to invalid room %d", __func__, obj->value[1] );
         return;
      }
      flags = 0;
      if( IS_SET( obj->value[0], TRIG_SHOWROOMDESC ) )
         SET_BIT( flags, TELE_SHOWDESC );
      if( IS_SET( obj->value[0], TRIG_TELEPORTALL ) )
         SET_BIT( flags, TELE_TRANSALL );
      if( IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )
         SET_BIT( flags, TELE_TRANSALLPLUS );

      teleport( ch, obj->value[1], flags );
      return;
   }

   if( IS_SET( obj->value[0], TRIG_RAND4 ) || IS_SET( obj->value[0], TRIG_RAND6 ) )
   {
      int maxd;

      if( ( room = get_room_index( obj->value[1] ) ) == NULL )
      {
         bug( "%s: obj points to invalid room %d", __func__, obj->value[1] );
         return;
      }

      if( IS_SET( obj->value[0], TRIG_RAND4 ) )
         maxd = 3;
      else
         maxd = 5;

      randomize_exits( room, maxd );
      for( rch = room->first_person; rch; rch = rch->next_in_room )
      {
         send_to_char( "You hear a loud rumbling sound.\r\n", rch );
         send_to_char( "Something seems different...\r\n", rch );
      }
   }

   /*
    * Death support added by Remcon 
    */
   if( IS_SET( obj->value[0], TRIG_DEATH ) )
   {
      /*
       * Should we really send a message to the room? 
       */
      act( AT_DEAD, "$n falls prey to a terrible death!", ch, NULL, NULL, TO_ROOM );
      act( AT_DEAD, "Oopsie... you're dead!\r\n", ch, NULL, NULL, TO_CHAR );
      snprintf( buf, MAX_STRING_LENGTH, "%s hit a DEATH TRIGGER in room %d!", ch->name, ch->in_room->vnum );
      log_string( buf );
      to_channel( buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );

      /*
       * Personaly I fiqured if we wanted it to be a full DT we could just have it send them into a DT. 
       */
      set_cur_char( ch );
      raw_kill( ch, ch );

      return;
   }

   /*
    * Object loading added by Remcon 
    */
   if( IS_SET( obj->value[0], TRIG_OLOAD ) )
   {
      OBJ_INDEX_DATA *pObjIndex;
      OBJ_DATA *tobj;

      /*
       * value[1] for the obj vnum 
       */
      if( !( pObjIndex = get_obj_index( obj->value[1] ) ) )
      {
         bug( "%s: obj points to invalid object vnum %d", __func__, obj->value[1] );
         return;
      }
      /*
       * Set room to NULL before the check 
       */
      room = NULL;
      /*
       * value[2] for the room vnum to put the object in if there is one, 0 for giving it to char or current room 
       */
      if( obj->value[2] > 0 && !( room = get_room_index( obj->value[2] ) ) )
      {
         bug( "%s: obj points to invalid room vnum %d", __func__, obj->value[2] );
         return;
      }
      /*
       * Uses value[3] for level 
       */
      if( !( tobj = create_object( pObjIndex, URANGE( 0, obj->value[3], MAX_LEVEL ) ) ) )
      {
         bug( "%s: obj couldnt create_obj vnum %d at level %d", __func__, obj->value[1], obj->value[3] );
         return;
      }
      if( room )
         obj_to_room( tobj, room );
      else
      {
         if( CAN_WEAR( obj, ITEM_TAKE ) )
            obj_to_char( tobj, ch );
         else
            obj_to_room( tobj, ch->in_room );
      }
      return;
   }

   /*
    * Mob loading added by Remcon 
    */
   if( IS_SET( obj->value[0], TRIG_MLOAD ) )
   {
      MOB_INDEX_DATA *pMobIndex;
      CHAR_DATA *mob;

      /*
       * value[1] for the obj vnum 
       */
      if( !( pMobIndex = get_mob_index( obj->value[1] ) ) )
      {
         bug( "%s: obj points to invalid mob vnum %d", __func__, obj->value[1] );
         return;
      }
      /*
       * Set room to current room before the check 
       */
      room = ch->in_room;
      /*
       * value[2] for the room vnum to put the object in if there is one, 0 for giving it to char or current room 
       */
      if( obj->value[2] > 0 && !( room = get_room_index( obj->value[2] ) ) )
      {
         bug( "%s: obj points to invalid room vnum %d", __func__, obj->value[2] );
         return;
      }
      if( !( mob = create_mobile( pMobIndex ) ) )
      {
         bug( "%s: obj couldnt create_mobile vnum %d", __func__, obj->value[1] );
         return;
      }
      char_to_room( mob, room );
      return;
   }

   /*
    * Spell casting support added by Remcon 
    */
   if( IS_SET( obj->value[0], TRIG_CAST ) )
   {
      if( obj->value[1] <= 0 || !IS_VALID_SN( obj->value[1] ) )
      {
         bug( "%s: obj points to invalid sn [%d]", __func__, obj->value[1] );
         return;
      }
      obj_cast_spell( obj->value[1], URANGE( 1, ( obj->value[2] > 0 ) ? obj->value[2] : ch->level, MAX_LEVEL ), ch, ch,
                      NULL );
      return;
   }

   /*
    * Container support added by Remcon 
    */
   if( IS_SET( obj->value[0], TRIG_CONTAINER ) )
   {
      OBJ_DATA *container = NULL;

      room = get_room_index( obj->value[1] );
      if( !room )
         room = obj->in_room;
      if( !room )
      {
         bug( "%s: obj points to invalid room %d", __func__, obj->value[1] );
         return;
      }

      for( container = ch->in_room->first_content; container; container = container->next_content )
      {
         if( container->pIndexData->vnum == obj->value[2] )
            break;
      }
      if( !container )
      {
         bug( "%s: obj points to a container [%d] thats not where it should be?", __func__, obj->value[2] );
         return;
      }
      if( container->item_type != ITEM_CONTAINER )
      {
         bug( "%s: obj points to object [%d], but it isn't a container.", __func__, obj->value[2] );
         return;
      }
      /*
       * Could toss in some messages. Limit how it is handled etc... I'll leave that to each one to do 
       */
      /*
       * Started to use TRIG_OPEN, TRIG_CLOSE, TRIG_LOCK, and TRIG_UNLOCK like TRIG_DOOR does. 
       */
      /*
       * It limits it alot, but it wouldn't allow for an EATKEY change 
       */
      if( IS_SET( obj->value[3], CONT_CLOSEABLE ) )
         TOGGLE_BIT( container->value[1], CONT_CLOSEABLE );
      if( IS_SET( obj->value[3], CONT_PICKPROOF ) )
         TOGGLE_BIT( container->value[1], CONT_PICKPROOF );
      if( IS_SET( obj->value[3], CONT_CLOSED ) )
         TOGGLE_BIT( container->value[1], CONT_CLOSED );
      if( IS_SET( obj->value[3], CONT_LOCKED ) )
         TOGGLE_BIT( container->value[1], CONT_LOCKED );
      if( IS_SET( obj->value[3], CONT_EATKEY ) )
         TOGGLE_BIT( container->value[1], CONT_EATKEY );
      return;
   }

   if( IS_SET( obj->value[0], TRIG_DOOR ) )
   {
      room = get_room_index( obj->value[1] );
      if( !room )
         room = obj->in_room;
      if( !room )
      {
         bug( "%s: obj points to invalid room %d", __func__, obj->value[1] );
         return;
      }
      if( IS_SET( obj->value[0], TRIG_D_NORTH ) )
      {
         edir = DIR_NORTH;
         txt = "to the north";
      }
      else if( IS_SET( obj->value[0], TRIG_D_SOUTH ) )
      {
         edir = DIR_SOUTH;
         txt = "to the south";
      }
      else if( IS_SET( obj->value[0], TRIG_D_EAST ) )
      {
         edir = DIR_EAST;
         txt = "to the east";
      }
      else if( IS_SET( obj->value[0], TRIG_D_WEST ) )
      {
         edir = DIR_WEST;
         txt = "to the west";
      }
      else if( IS_SET( obj->value[0], TRIG_D_UP ) )
      {
         edir = DIR_UP;
         txt = "from above";
      }
      else if( IS_SET( obj->value[0], TRIG_D_DOWN ) )
      {
         edir = DIR_DOWN;
         txt = "from below";
      }
      else
      {
         bug( "%s: door: no direction flag set.", __func__ );
         return;
      }
      pexit = get_exit( room, edir );
      if( !pexit )
      {
         if( !IS_SET( obj->value[0], TRIG_PASSAGE ) )
         {
            bug( "%s: obj points to non-exit %d", __func__, obj->value[1] );
            return;
         }
         to_room = get_room_index( obj->value[2] );
         if( !to_room )
         {
            bug( "%s: dest points to invalid room %d", __func__, obj->value[2] );
            return;
         }
         pexit = make_exit( room, to_room, edir );
         pexit->keyword = STRALLOC( "" );
         pexit->description = STRALLOC( "" );
         pexit->key = -1;
         pexit->exit_info = 0;
         act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_CHAR );
         act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_ROOM );
         return;
      }
      if( IS_SET( obj->value[0], TRIG_UNLOCK ) && IS_SET( pexit->exit_info, EX_LOCKED ) )
      {
         REMOVE_BIT( pexit->exit_info, EX_LOCKED );
         act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_CHAR );
         act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_ROOM );
         if( ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
            REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
         return;
      }
      if( IS_SET( obj->value[0], TRIG_LOCK ) && !IS_SET( pexit->exit_info, EX_LOCKED ) )
      {
         SET_BIT( pexit->exit_info, EX_LOCKED );
         act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_CHAR );
         act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_ROOM );
         if( ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
            SET_BIT( pexit_rev->exit_info, EX_LOCKED );
         return;
      }
      if( IS_SET( obj->value[0], TRIG_OPEN ) && IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
         REMOVE_BIT( pexit->exit_info, EX_CLOSED );
         for( rch = room->first_person; rch; rch = rch->next_in_room )
            act( AT_ACTION, "The $d opens.", rch, NULL, pexit->keyword, TO_CHAR );
         if( ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
         {
            REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
            /*
             * bug here pointed out by Nick Gammon 
             */
            for( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
               act( AT_ACTION, "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
         }
         check_room_for_traps( ch, trap_door[edir] );
         return;
      }
      if( IS_SET( obj->value[0], TRIG_CLOSE ) && !IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
         SET_BIT( pexit->exit_info, EX_CLOSED );
         for( rch = room->first_person; rch; rch = rch->next_in_room )
            act( AT_ACTION, "The $d closes.", rch, NULL, pexit->keyword, TO_CHAR );
         if( ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
         {
            SET_BIT( pexit_rev->exit_info, EX_CLOSED );
            /*
             * bug here pointed out by Nick Gammon 
             */
            for( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
               act( AT_ACTION, "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
         }
         check_room_for_traps( ch, trap_door[edir] );
         return;
      }
   }
}

void do_pull( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Pull what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( obj = get_obj_here( ch, arg ) ) == NULL )
   {
      act( AT_PLAIN, "I see no $T here.", ch, NULL, arg, TO_CHAR );
      return;
   }

   pullorpush( ch, obj, TRUE );
}

void do_push( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Push what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( obj = get_obj_here( ch, arg ) ) == NULL )
   {
      act( AT_PLAIN, "I see no $T here.", ch, NULL, arg, TO_CHAR );
      return;
   }

   pullorpush( ch, obj, FALSE );
}

void do_rap( CHAR_DATA* ch, const char* argument)
{
   EXIT_DATA *pexit;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Rap on what?\r\n", ch );
      return;
   }
   if( ch->fighting )
   {
      send_to_char( "You have better things to do with your hands right now.\r\n", ch );
      return;
   }
   if( ( pexit = find_door( ch, arg, FALSE ) ) != NULL )
   {
      ROOM_INDEX_DATA *to_room;
      EXIT_DATA *pexit_rev;
      const char *keyword;
      if( !IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
         send_to_char( "Why knock?  It's open.\r\n", ch );
         return;
      }
      if( IS_SET( pexit->exit_info, EX_SECRET ) )
         keyword = "wall";
      else
         keyword = pexit->keyword;
      act( AT_ACTION, "You rap loudly on the $d.", ch, NULL, keyword, TO_CHAR );
      act( AT_ACTION, "$n raps loudly on the $d.", ch, NULL, keyword, TO_ROOM );
      if( ( to_room = pexit->to_room ) != NULL && ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
      {
         CHAR_DATA *rch;
         for( rch = to_room->first_person; rch; rch = rch->next_in_room )
         {
            act( AT_ACTION, "Someone raps loudly from the other side of the $d.", rch, NULL, pexit_rev->keyword, TO_CHAR );
         }
      }
   }
   else
   {
      act( AT_ACTION, "You make knocking motions through the air.", ch, NULL, NULL, TO_CHAR );
      act( AT_ACTION, "$n makes knocking motions through the air.", ch, NULL, NULL, TO_ROOM );
   }
   return;
}

/* pipe commands (light, tamp, smoke) by Thoric */
void do_tamp( CHAR_DATA* ch, const char* argument)
{
   OBJ_DATA *opipe;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Tamp what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( opipe = get_obj_carry( ch, arg ) ) == NULL )
   {
      send_to_char( "You aren't carrying that.\r\n", ch );
      return;
   }
   if( opipe->item_type != ITEM_PIPE )
   {
      send_to_char( "You can't tamp that.\r\n", ch );
      return;
   }
   if( !IS_SET( opipe->value[3], PIPE_TAMPED ) )
   {
      act( AT_ACTION, "You gently tamp $p.", ch, opipe, NULL, TO_CHAR );
      act( AT_ACTION, "$n gently tamps $p.", ch, opipe, NULL, TO_ROOM );
      SET_BIT( opipe->value[3], PIPE_TAMPED );
      return;
   }
   send_to_char( "It doesn't need tamping.\r\n", ch );
}

void do_smoke( CHAR_DATA* ch, const char* argument)
{
   OBJ_DATA *opipe;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Smoke what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( opipe = get_obj_carry( ch, arg ) ) == NULL )
   {
      send_to_char( "You aren't carrying that.\r\n", ch );
      return;
   }
   if( opipe->item_type != ITEM_PIPE )
   {
      act( AT_ACTION, "You try to smoke $p... but it doesn't seem to work.", ch, opipe, NULL, TO_CHAR );
      act( AT_ACTION, "$n tries to smoke $p... (I wonder what $e's been putting in $s pipe?)", ch, opipe, NULL, TO_ROOM );
      return;
   }
   if( !IS_SET( opipe->value[3], PIPE_LIT ) )
   {
      act( AT_ACTION, "You try to smoke $p, but it's not lit.", ch, opipe, NULL, TO_CHAR );
      act( AT_ACTION, "$n tries to smoke $p, but it's not lit.", ch, opipe, NULL, TO_ROOM );
      return;
   }
   if( opipe->value[1] > 0 )
   {
      if( !oprog_use_trigger( ch, opipe, NULL, NULL ) )
      {
         act( AT_ACTION, "You draw thoughtfully from $p.", ch, opipe, NULL, TO_CHAR );
         act( AT_ACTION, "$n draws thoughtfully from $p.", ch, opipe, NULL, TO_ROOM );
      }

      if( IS_VALID_HERB( opipe->value[2] ) && opipe->value[2] < top_herb )
      {
         int sn = opipe->value[2] + TYPE_HERB;
         SKILLTYPE *skill = get_skilltype( sn );

         WAIT_STATE( ch, skill->beats );
         if( skill->spell_fun )
            obj_cast_spell( sn, UMIN( skill->min_level, ch->level ), ch, ch, NULL );
         if( obj_extracted( opipe ) )
            return;
      }
      else
         bug( "%s: bad herb type %d", __func__, opipe->value[2] );

      SET_BIT( opipe->value[3], PIPE_HOT );
      if( --opipe->value[1] < 1 )
      {
         REMOVE_BIT( opipe->value[3], PIPE_LIT );
         SET_BIT( opipe->value[3], PIPE_DIRTY );
         SET_BIT( opipe->value[3], PIPE_FULLOFASH );
      }
   }
}

OBJ_DATA *find_tinder( CHAR_DATA *ch )
{
   OBJ_DATA *tinder;

   for( tinder = ch->last_carrying; tinder; tinder = tinder->prev_content )
      if( ( tinder->item_type == ITEM_TINDER ) && can_see_obj( ch, tinder ) )
         return tinder;
   return NULL;
}

void do_extinguish( CHAR_DATA *ch, const char *argument )
{
   OBJ_DATA *obj;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Extinguish what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( obj = get_obj_wear( ch, arg ) ) == NULL )
      if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
         if( ( obj = get_obj_list_rev( ch, arg, ch->in_room->last_content ) ) == NULL )
   {
      send_to_char( "You aren't carrying that.\r\n", ch );
      return;
   }

   separate_obj( obj );

   if( obj->item_type != ITEM_LIGHT || ( obj->value[1] < 1 ) )
   {
      send_to_char( "You can't extinguish that.\r\n", ch );
      return;
   }

   if( IS_SET( obj->value[3], PIPE_LIT ) )
   {
      act( AT_ACTION, "You extinguish $p.", ch, obj, NULL, TO_CHAR );
      act( AT_ACTION, "$n extinguishes $p.", ch, obj, NULL, TO_ROOM );
      REMOVE_BIT( obj->value[3], PIPE_LIT );
      return;
   }
   else
   {
      send_to_char( "It's not lit.\r\n", ch );
      return;
   }
}

void do_light( CHAR_DATA *ch, const char *argument )
{
   OBJ_DATA *obj;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Light what?\r\n", ch );
      return;
   }

   if( ms_find_obj(ch) )
      return;

   if( ( obj = get_obj_wear( ch, arg ) ) == NULL )
      if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
         if( ( obj = get_obj_list_rev( ch, arg, ch->in_room->last_content ) ) == NULL )
   {
      send_to_char( "You aren't carrying that.\r\n", ch );
      return;
   }

   if( !find_tinder( ch ) )
   {
      send_to_char( "You need some tinder to light with.\r\n", ch );
      return;
   }

   separate_obj( obj );

   switch( obj->item_type )
   {
      default:
         send_to_char( "You can't light that.\r\n", ch );
         return;

      case ITEM_PIPE:
         if( !IS_SET( obj->value[3], PIPE_LIT ) )
         {
            if( obj->value[1] < 1 )
            {
               act( AT_ACTION, "You try to light $p, but it's empty.", ch, obj, NULL, TO_CHAR );
               act( AT_ACTION, "$n tries to light $p, but it's empty.", ch, obj, NULL, TO_ROOM );
               return;
            }
            act( AT_ACTION, "You carefully light $p.", ch, obj, NULL, TO_CHAR );
            act( AT_ACTION, "$n carefully lights $p.", ch, obj, NULL, TO_ROOM );
            SET_BIT( obj->value[3], PIPE_LIT );
            return;
         }
         send_to_char( "It's already lit.\r\n", ch );
         break;

      case ITEM_LIGHT:
         if( obj->value[1] > 0 )
         {
            if( !IS_SET( obj->value[3], PIPE_LIT ) )
            {
               act( AT_ACTION, "You carefully light $p.", ch, obj, NULL, TO_CHAR );
               act( AT_ACTION, "$n carefully lights $p.", ch, obj, NULL, TO_ROOM );
               SET_BIT( obj->value[3], PIPE_LIT );
            }
            else
               send_to_char( "It's already lit.\r\n", ch );
            return;
         }
         else
            send_to_char( "You can't light that.\r\n", ch );
         break;
   }
   return;
}

/*
 * Apply a salve/ointment					-Thoric
 * Support for applying to others.  Pkill concerns dealt with elsewhere.
 */
void do_apply( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *salve;
   OBJ_DATA *obj;
   ch_ret retcode;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Apply what?\r\n", ch );
      return;
   }
   if( ch->fighting )
   {
      send_to_char( "You're too busy fighting ...\r\n", ch );
      return;
   }
   if( ms_find_obj( ch ) )
      return;
   if( ( salve = get_obj_carry( ch, arg1 ) ) == NULL )
   {
      send_to_char( "You do not have that.\r\n", ch );
      return;
   }

   obj = NULL;
   if( arg2[0] == '\0' )
      victim = ch;
   else
   {
      if( ( victim = get_char_room( ch, arg2 ) ) == NULL && ( obj = get_obj_here( ch, arg2 ) ) == NULL )
      {
         send_to_char( "Apply it to what or who?\r\n", ch );
         return;
      }
   }

   /*
    * apply salve to another object 
    */
   if( obj )
   {
      send_to_char( "You can't do that... yet.\r\n", ch );
      return;
   }

   if( victim->fighting )
   {
      send_to_char( "Wouldn't work very well while they're fighting ...\r\n", ch );
      return;
   }

   if( salve->item_type != ITEM_SALVE )
   {
      if( victim == ch )
      {
         act( AT_ACTION, "$n starts to rub $p on $mself...", ch, salve, NULL, TO_ROOM );
         act( AT_ACTION, "You try to rub $p on yourself...", ch, salve, NULL, TO_CHAR );
      }
      else
      {
         act( AT_ACTION, "$n starts to rub $p on $N...", ch, salve, victim, TO_NOTVICT );
         act( AT_ACTION, "$n starts to rub $p on you...", ch, salve, victim, TO_VICT );
         act( AT_ACTION, "You try to rub $p on $N...", ch, salve, victim, TO_CHAR );
      }
      return;
   }
   separate_obj( salve );
   --salve->value[1];

   if( !oprog_use_trigger( ch, salve, NULL, NULL ) )
   {
      if( !salve->action_desc || salve->action_desc[0] == '\0' )
      {
         if( salve->value[1] < 1 )
         {
            if( victim != ch )
            {
               act( AT_ACTION, "$n rubs the last of $p onto $N.", ch, salve, victim, TO_NOTVICT );
               act( AT_ACTION, "$n rubs the last of $p onto you.", ch, salve, victim, TO_VICT );
               act( AT_ACTION, "You rub the last of $p onto $N.", ch, salve, victim, TO_CHAR );
            }
            else
            {
               act( AT_ACTION, "You rub the last of $p onto yourself.", ch, salve, NULL, TO_CHAR );
               act( AT_ACTION, "$n rubs the last of $p onto $mself.", ch, salve, NULL, TO_ROOM );
            }
         }
         else
         {
            if( victim != ch )
            {
               act( AT_ACTION, "$n rubs $p onto $N.", ch, salve, victim, TO_NOTVICT );
               act( AT_ACTION, "$n rubs $p onto you.", ch, salve, victim, TO_VICT );
               act( AT_ACTION, "You rub $p onto $N.", ch, salve, victim, TO_CHAR );
            }
            else
            {
               act( AT_ACTION, "You rub $p onto yourself.", ch, salve, NULL, TO_CHAR );
               act( AT_ACTION, "$n rubs $p onto $mself.", ch, salve, NULL, TO_ROOM );
            }
         }
      }
      else
         actiondesc( ch, salve );
   }

   WAIT_STATE( ch, salve->value[3] );
   retcode = obj_cast_spell( salve->value[4], salve->value[0], ch, victim, NULL );
   if( retcode == rNONE )
      retcode = obj_cast_spell( salve->value[5], salve->value[0], ch, victim, NULL );
   if( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
   {
      bug( "%s:  char died", __func__ );
      return;
   }

   if( !obj_extracted( salve ) && salve->value[1] <= 0 )
      extract_obj( salve );
   return;
}

/* generate an action description message */
void actiondesc( CHAR_DATA * ch, OBJ_DATA * obj )
{
   char charbuf[MAX_STRING_LENGTH];
   char roombuf[MAX_STRING_LENGTH];
/*   char buf[MAX_STRING_LENGTH]; */
   const char *srcptr = obj->action_desc;
   char *charptr = charbuf;
   char *roomptr = roombuf;
   const char *ichar = "You";
   const char *iroom = "Someone";

   while( *srcptr != '\0' )
   {
      if( *srcptr == '$' )
      {
         srcptr++;
         switch ( *srcptr )
         {
            case 'e':
               ichar = "you";
               iroom = "$e";
               break;

            case 'm':
               ichar = "you";
               iroom = "$m";
               break;

            case 'n':
               ichar = "you";
               iroom = "$n";
               break;

            case 's':
               ichar = "your";
               iroom = "$s";
               break;

               /*
                * case 'q':
                * iroom = "s";
                * break;
                */

            default:
               srcptr--;
               *charptr++ = *srcptr;
               *roomptr++ = *srcptr;
               break;
         }
      }
      else if( *srcptr == '%' && *++srcptr == 's' )
      {
         ichar = "You";
         iroom = "$n";
      }
      else
      {
         *charptr++ = *srcptr;
         *roomptr++ = *srcptr;
         srcptr++;
         continue;
      }

      while( ( *charptr = *ichar ) != '\0' )
      {
         charptr++;
         ichar++;
      }

      while( ( *roomptr = *iroom ) != '\0' )
      {
         roomptr++;
         iroom++;
      }
      srcptr++;
   }

   *charptr = '\0';
   *roomptr = '\0';

   switch ( obj->item_type )
   {
      case ITEM_BLOOD:
      case ITEM_FOUNTAIN:
         act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
         act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
         return;

      case ITEM_DRINK_CON:
      {
         LIQ_TABLE *liq = get_liq_vnum( obj->value[2] );

         act( AT_ACTION, charbuf, ch, obj, ( liq == NULL ? "water" : liq->name ), TO_CHAR );
         act( AT_ACTION, roombuf, ch, obj, ( liq == NULL ? "water" : liq->name ), TO_ROOM );
         return;
      }

      case ITEM_PIPE:
         return;

      case ITEM_ARMOR:
      case ITEM_WEAPON:
      case ITEM_LIGHT:
         return;

      case ITEM_COOK:
      case ITEM_FOOD:
      case ITEM_PILL:
         act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
         act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
         return;

      default:
         return;
   }
   return;
}

/*
 * Extended Bitvector Routines					-Thoric
 */

/* check to see if the extended bitvector is completely empty */
bool ext_is_empty( EXT_BV * bits )
{
   int x;

   for( x = 0; x < XBI; x++ )
      if( bits->bits[x] != 0 )
         return FALSE;

   return TRUE;
}

void ext_clear_bits( EXT_BV * bits )
{
   int x;

   for( x = 0; x < XBI; x++ )
      bits->bits[x] = 0;
}

/* for use by xHAS_BITS() -- works like IS_SET() */
int ext_has_bits( EXT_BV * var, EXT_BV * bits )
{
   int x, bit;

   for( x = 0; x < XBI; x++ )
      if( ( bit = ( var->bits[x] & bits->bits[x] ) ) != 0 )
         return bit;

   return 0;
}

/* for use by xSAME_BITS() -- works like == */
bool ext_same_bits( EXT_BV * var, EXT_BV * bits )
{
   int x;

   for( x = 0; x < XBI; x++ )
      if( var->bits[x] != bits->bits[x] )
         return FALSE;

   return TRUE;
}

/* for use by xSET_BITS() -- works like SET_BIT() */
void ext_set_bits( EXT_BV * var, EXT_BV * bits )
{
   int x;

   for( x = 0; x < XBI; x++ )
      var->bits[x] |= bits->bits[x];
}

/* for use by xREMOVE_BITS() -- works like REMOVE_BIT() */
void ext_remove_bits( EXT_BV * var, EXT_BV * bits )
{
   int x;

   for( x = 0; x < XBI; x++ )
      var->bits[x] &= ~( bits->bits[x] );
}

/* for use by xTOGGLE_BITS() -- works like TOGGLE_BIT() */
void ext_toggle_bits( EXT_BV * var, EXT_BV * bits )
{
   int x;

   for( x = 0; x < XBI; x++ )
      var->bits[x] ^= bits->bits[x];
}

/*
 * Read an extended bitvector from a file.			-Thoric
 */
EXT_BV fread_bitvector( FILE * fp )
{
   EXT_BV ret;
   int c, x = 0;
   int num = 0;

   memset( &ret, '\0', sizeof( ret ) );
   for( ;; )
   {
      num = fread_number( fp );
      if( x < XBI )
         ret.bits[x] = num;
      ++x;
      if( ( c = getc( fp ) ) != '&' )
      {
         ungetc( c, fp );
         break;
      }
   }

   return ret;
}

/* return a string for writing a bitvector to a file */
char *print_bitvector( EXT_BV * bits )
{
   static char buf[XBI * 12];
   char *p = buf;
   int x, cnt = 0;

   for( cnt = XBI - 1; cnt > 0; cnt-- )
      if( bits->bits[cnt] )
         break;
   for( x = 0; x <= cnt; x++ )
   {
      snprintf( p, ( XBI * 12 ) - ( p - buf ), "%ud", bits->bits[x] );
      p += strlen( p );
      if( x < cnt )
         *p++ = '&';
   }
   *p = '\0';

   return buf;
}

/*
 * Write an extended bitvector to a file			-Thoric
 */
void fwrite_bitvector( EXT_BV * bits, FILE * fp )
{
   fputs( print_bitvector( bits ), fp );
}

EXT_BV meb( int bit )
{
   EXT_BV bits;

   xCLEAR_BITS( bits );
   if( bit >= 0 )
      xSET_BIT( bits, bit );

   return bits;
}

EXT_BV multimeb( int bit, ... )
{
   EXT_BV bits;
   va_list param;
   int b;

   xCLEAR_BITS( bits );
   if( bit < 0 )
      return bits;

   xSET_BIT( bits, bit );

   va_start( param, bit );

   while( ( b = va_arg( param, int ) ) != -1 )
        xSET_BIT( bits, b );

   va_end( param );

   return bits;
}
