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
 *                   Character saving and loading module                    *
 ****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "mud.h"
#include "house.h"

/*
 * Increment with every major format change.
 * Upped to 5 for addition of new Age Setup. - Kayle 1/22/08
 */
const int SAVEVERSION = 5;

/*
 * Array to keep track of equipment temporarily. - Thoric
 */
OBJ_DATA *save_equipment[MAX_WEAR][MAX_LAYERS];
OBJ_DATA *mob_save_equipment[MAX_WEAR][MAX_LAYERS];
CHAR_DATA *quitting_char, *loading_char, *saving_char;

int file_ver;

/*
 * Externals
 */
void fwrite_comments( CHAR_DATA * ch, FILE * fp );
void fread_comment( CHAR_DATA * ch, FILE * fp );
void fwrite_variables( CHAR_DATA * ch, FILE * fp );
void fread_variable( CHAR_DATA * ch, FILE * fp );
bool check_parse_name( const char *name, bool newchar );
void fwrite_fuss_exdesc( FILE * fpout, EXTRA_DESCR_DATA * ed );
void fwrite_fuss_affect( FILE * fp, AFFECT_DATA * paf );

/*
 * Array of containers read for proper re-nesting of objects.
 */
static OBJ_DATA *rgObjNest[MAX_NEST];

/*
 * Local functions.
 */
void fwrite_char( CHAR_DATA * ch, FILE * fp );
void fread_char( CHAR_DATA * ch, FILE * fp, bool preload, bool copyover );
void write_corpses( CHAR_DATA * ch, char *name, OBJ_DATA * objrem );

#ifdef WIN32   /* NJG */
UINT timer_code = 0; /* needed to kill the timer */
/* Note: need to include: WINMM.LIB to link to timer functions */
void caught_alarm(  );
void CALLBACK alarm_handler( UINT IDEvent,   /* identifies timer event */
                             UINT uReserved, /* not used */
                             DWORD dwUser,   /* application-defined instance data */
                             DWORD dwReserved1, /* not used */
                             DWORD dwReserved2 )   /* not used */
{
   caught_alarm(  );
}

void kill_timer(  )
{
   if( timer_code )
      timeKillEvent( timer_code );
   timer_code = 0;
}

#endif

/*
 * Un-equip character before saving to ensure proper	-Thoric
 * stats are saved in case of changes to or removal of EQ
 */
void de_equip_char( CHAR_DATA * ch )
{
   OBJ_DATA *obj;
   int x, y;

   for( x = 0; x < MAX_WEAR; x++ )
   {
      for( y = 0; y < MAX_LAYERS; y++ )
      {
         if( IS_NPC( ch ) )
            mob_save_equipment[x][y] = NULL;
         else
            save_equipment[x][y] = NULL;
      }
   }

   for( obj = ch->first_carrying; obj; obj = obj->next_content )
   {
      if( obj->wear_loc > -1 && obj->wear_loc < MAX_WEAR )
      {
         if( get_trust( ch ) >= obj->level )
         {
            for( x = 0; x < MAX_LAYERS; x++ )
            {
               if( IS_NPC( ch ) )
               {
                  if( !mob_save_equipment[obj->wear_loc][x] )
                  {
                     mob_save_equipment[obj->wear_loc][x] = obj;
                     break;
                  }
               }
               else
               {
                  if( !save_equipment[obj->wear_loc][x] )
                  {
                     save_equipment[obj->wear_loc][x] = obj;
                     break;
                  }
               }
            }
            if( x == MAX_LAYERS )
            {
               bug( "%s: %s had on more than %d layers of clothing in one location (%d): %s", __func__,
                    ch->name, MAX_LAYERS, obj->wear_loc, obj->name );
            }
         }
         else
         {
            bug( "%s: %s had on %s:  ch->level = %d  obj->level = %d", __func__, ch->name, obj->name, ch->level, obj->level );
         }
         unequip_char( ch, obj );
      }
   }

   if( ch->in_room )
   {
      AFFECT_DATA *paf;

      /*
       * If the char is altered by room affects, strip those before recording the pfile!!
       */
      for( paf = ch->in_room->first_affect; paf; paf = paf->next )
         if( paf->location != APPLY_WEARSPELL && paf->location != APPLY_REMOVESPELL )
            affect_modify( ch, paf, FALSE );
      for( paf = ch->in_room->first_permaffect; paf; paf = paf->next )
         if( paf->location != APPLY_WEARSPELL && paf->location != APPLY_REMOVESPELL )
            affect_modify( ch, paf, FALSE );
   }
}

/*
 * Re-equip character					-Thoric
 */
void re_equip_char( CHAR_DATA * ch )
{
   int x, y;

   for( x = 0; x < MAX_WEAR; x++ )
   {
      for( y = 0; y < MAX_LAYERS; y++ )
      {
         if( IS_NPC( ch ) )
         {
            if( mob_save_equipment[x][y] != NULL )
            {
               if( quitting_char != ch )
                  equip_char( ch, mob_save_equipment[x][y], x );
               mob_save_equipment[x][y] = NULL;
            }
            else
               break;
         }
         else
         {
            if( save_equipment[x][y] != NULL )
            {
               if( quitting_char != ch )
                  equip_char( ch, save_equipment[x][y], x );
               save_equipment[x][y] = NULL;
            }
            else
               break;
         }
      }
   }

   if( ch->in_room )
   {
      AFFECT_DATA *paf;

      /*
       * We had to strip room affects from the char to get their stats correct for
       * writing the pfile.  Now add the room affects back on.
       */
      for( paf = ch->in_room->first_affect; paf; paf = paf->next )
         if( paf->location != APPLY_WEARSPELL && paf->location != APPLY_REMOVESPELL )
            affect_modify( ch, paf, TRUE );
      for( paf = ch->in_room->first_permaffect; paf; paf = paf->next )
         if( paf->location != APPLY_WEARSPELL && paf->location != APPLY_REMOVESPELL )
            affect_modify( ch, paf, TRUE );
   }
}

short find_old_age( CHAR_DATA * ch )
{
   short age;

   if( IS_NPC( ch ) )
      return -1;

   age = ch->played / 86400;   /* Calculate realtime number of days played */

   age = age / 7; /* Calculates rough estimate on number of mud years played */

   age += 17;  /* Add 17 years, new characters begin at 17. */

   ch->pcdata->day = ( number_range( 1, sysdata.dayspermonth ) - 1 );   /* Assign random day of birth */
   ch->pcdata->month = ( number_range( 1, sysdata.monthsperyear ) - 1 );   /* Assign random month of birth */
   ch->pcdata->year = time_info.year - age;  /* Assign birth year based on calculations above */

   return age;
}

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA * ch )
{
   char strsave[MAX_INPUT_LENGTH];
   char strback[MAX_INPUT_LENGTH];
   FILE *fp;

   if( !ch )
   {
      bug( "%s: null ch!", __func__ );
      return;
   }

   if( IS_NPC( ch ) || ch->level < 2 )
      return;

   saving_char = ch;

   if( ch->desc && ch->desc->original )
      ch = ch->desc->original;

   de_equip_char( ch );

   ch->save_time = current_time;
   snprintf( strsave, MAX_INPUT_LENGTH, "%s%c/%s", PLAYER_DIR, tolower( ch->pcdata->filename[0] ),
             capitalize( ch->pcdata->filename ) );

   /*
    * Auto-backup pfile (can cause lag with high disk access situtations).
    */
   /*
    * Backup of each pfile on save as above can cause lag in high disk
    * access situations on big muds like Realms.  Quitbackup saves most
    * of that and keeps an adequate backup -- Blodkai, 10/97 
    */

   if( IS_SET( sysdata.save_flags, SV_BACKUP ) || ( IS_SET( sysdata.save_flags, SV_QUITBACKUP ) && quitting_char == ch ) )
   {
      snprintf( strback, MAX_INPUT_LENGTH, "%s%c/%s", BACKUP_DIR, tolower( ch->pcdata->filename[0] ),
                capitalize( ch->pcdata->filename ) );
      rename( strsave, strback );
   }

   /*
    * Save immortal stats, level & vnums for wizlist    -Thoric
    * and do_vnums command
    *
    * Also save the player flags so we the wizlist builder can see
    * who is a guest and who is retired.
    */
   if( ch->level >= LEVEL_IMMORTAL || IS_SET( ch->pcdata->flags, PCFLAG_RETIRED ) )
   {
      snprintf( strback, MAX_INPUT_LENGTH, "%s%s", GOD_DIR, capitalize( ch->pcdata->filename ) );

      if( ( fp = fopen( strback, "w" ) ) == NULL )
      {
         perror( strback );
         bug( "%s: cant open %s", __func__, strback );
      }
      else
      {
         fprintf( fp, "Level        %d\n", ch->level );
         fprintf( fp, "Pcflags      %d\n", ch->pcdata->flags );
         if( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
            fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo, ch->pcdata->r_range_hi );
         if( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
            fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo, ch->pcdata->o_range_hi );
         if( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
            fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo, ch->pcdata->m_range_hi );
         FCLOSE( fp );
      }
   }

   if( IS_SET( sysdata.save_flags, SV_TMPSAVE ) )
      fp = fopen( TEMP_FILE, "w" );
   else
      fp = fopen( strsave, "w" );

   if( fp == NULL )
   {
      if( IS_SET( sysdata.save_flags, SV_TMPSAVE ) )
         perror( TEMP_FILE );
      else
         perror( strsave );
      bug( "%s: fopen", __func__ );
   }
   else
   {
      bool ferr;

      fwrite_char( ch, fp );
      if( ch->morph )
         fwrite_morph_data( ch, fp );
      if( ch->first_carrying )
         fwrite_obj( ch, ch->last_carrying, fp, 0, OS_CARRY, ch->pcdata->hotboot );
      if( sysdata.save_pets && ch->pcdata->pet )
         fwrite_mobile( fp, ch->pcdata->pet );
      if( ch->variables )
         fwrite_variables( ch, fp );
      if( ch->comments )   /* comments */
         fwrite_comments( ch, fp ); /* comments */
      fprintf( fp, "#END\n" );

      ferr = ferror( fp );
      FCLOSE( fp );

      if( IS_SET( sysdata.save_flags, SV_TMPSAVE ) )
      {
         if( ferr )
         {
            perror( strsave );
            bug( "%s: Error writing temp file for %s -- not copying", __func__, strsave );
         }
         else
            rename( TEMP_FILE, strsave );
      }
   }

   re_equip_char( ch );

   quitting_char = NULL;
   saving_char = NULL;
}

/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA * ch, FILE * fp )
{
   AFFECT_DATA *paf;
   int sn, track;
   short pos;
   SKILLTYPE *skill = NULL;

   fprintf( fp, "#PLAYER\n" );

   fprintf( fp, "Version      %d\n", SAVEVERSION );
   fprintf( fp, "Name         %s~\n", ch->name );
   if( ch->description[0] != '\0' )
      fprintf( fp, "Description  %s~\n", ch->description );
   fprintf( fp, "Sex          %d\n", ch->sex );
   fprintf( fp, "Class        %d\n", ch->Class );
   fprintf( fp, "Race         %d\n", ch->race );
   fprintf( fp, "Age          %d %d %d %d\n",
            ch->pcdata->age_bonus, ch->pcdata->day, ch->pcdata->month, ch->pcdata->year );
   fprintf( fp, "Languages    %d %d\n", ch->speaks, ch->speaking );
   fprintf( fp, "Level        %d\n", ch->level );
   fprintf( fp, "Played       %d\n", ch->played + ( int )( current_time - ch->logon ) );
   fprintf( fp, "Room         %d\n",
            ( ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
              && ch->was_in_room ) ? ch->was_in_room->vnum : ch->in_room->vnum );

   fprintf( fp, "HpManaMove   %d %d %d %d %d %d\n", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
   fprintf( fp, "Gold         %d\n", ch->gold );
   fprintf( fp, "Exp          %d\n", ch->exp );
   fprintf( fp, "Height          %d\n", ch->height );
   fprintf( fp, "Weight          %d\n", ch->weight );
   if( !xIS_EMPTY( ch->act ) )
      fprintf( fp, "Act          %s\n", print_bitvector( &ch->act ) );
   if( !xIS_EMPTY( ch->affected_by ) )
      fprintf( fp, "AffectedBy   %s\n", print_bitvector( &ch->affected_by ) );
   if( !xIS_EMPTY( ch->no_affected_by ) )
      fprintf( fp, "NoAffectedBy %s\n", print_bitvector( &ch->no_affected_by ) );

   /*
    * Strip off fighting positions & store as
    * new style (pos>=100 flags new style in character loading)
    */
   pos = ch->position;
   if( pos == POS_BERSERK || pos == POS_AGGRESSIVE || pos == POS_FIGHTING || pos == POS_DEFENSIVE || pos == POS_EVASIVE )
      pos = POS_STANDING;
   pos += 100;
   fprintf( fp, "Position     %d\n", pos );

   fprintf( fp, "Style     %d\n", ch->style );

   fprintf( fp, "Practice     %d\n", ch->practice );
   fprintf( fp, "SavingThrows %d %d %d %d %d\n",
            ch->saving_poison_death, ch->saving_wand, ch->saving_para_petri, ch->saving_breath, ch->saving_spell_staff );
   fprintf( fp, "Alignment    %d\n", ch->alignment );
   fprintf( fp, "Favor        %d\n", ch->pcdata->favor );
   fprintf( fp, "Glory        %d\n", ch->pcdata->quest_curr );
   fprintf( fp, "MGlory       %d\n", ch->pcdata->quest_accum );
   fprintf( fp, "Hitroll      %d\n", ch->hitroll );
   fprintf( fp, "Damroll      %d\n", ch->damroll );
   fprintf( fp, "Armor        %d\n", ch->armor );
   if( ch->wimpy )
      fprintf( fp, "Wimpy        %d\n", ch->wimpy );
   if( ch->deaf )
      fprintf( fp, "Deaf         %d\n", ch->deaf );
   if( ch->resistant )
      fprintf( fp, "Resistant    %d\n", ch->resistant );
   if( ch->no_resistant )
      fprintf( fp, "NoResistant  %d\n", ch->no_resistant );
   if( ch->immune )
      fprintf( fp, "Immune       %d\n", ch->immune );
   if( ch->no_immune )
      fprintf( fp, "NoImmune     %d\n", ch->no_immune );
   if( ch->susceptible )
      fprintf( fp, "Susceptible  %d\n", ch->susceptible );
   if( ch->no_susceptible )
      fprintf( fp, "NoSusceptible  %d\n", ch->no_susceptible );
   if( ch->pcdata && ch->pcdata->outcast_time )
      fprintf( fp, "Outcast_time %ld\n", ch->pcdata->outcast_time );
   if( ch->pcdata && ch->pcdata->nuisance )
      fprintf( fp, "NuisanceNew %ld %ld %d %d\n", ch->pcdata->nuisance->set_time,
               ch->pcdata->nuisance->max_time, ch->pcdata->nuisance->flags, ch->pcdata->nuisance->power );
   if( ch->mental_state != -10 )
      fprintf( fp, "Mentalstate  %d\n", ch->mental_state );

   fprintf( fp, "Password     %s~\n", ch->pcdata->pwd );
   if( ch->pcdata->rank && ch->pcdata->rank[0] != '\0' )
      fprintf( fp, "Rank         %s~\n", ch->pcdata->rank );
   if( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
      fprintf( fp, "Bestowments  %s~\n", ch->pcdata->bestowments );
   fprintf( fp, "Title        %s~\n", ch->pcdata->title );
   if( ch->pcdata->homepage && ch->pcdata->homepage[0] != '\0' )
      fprintf( fp, "Homepage     %s~\n", ch->pcdata->homepage );
   if( ch->pcdata->bio && ch->pcdata->bio[0] != '\0' )
      fprintf( fp, "Bio          %s~\n", ch->pcdata->bio );
   if( ch->pcdata->authed_by && ch->pcdata->authed_by[0] != '\0' )
      fprintf( fp, "AuthedBy     %s~\n", ch->pcdata->authed_by );
   if( ch->pcdata->min_snoop )
      fprintf( fp, "Minsnoop     %d\n", ch->pcdata->min_snoop );
   if( ch->pcdata->prompt && *ch->pcdata->prompt )
      fprintf( fp, "Prompt       %s~\n", ch->pcdata->prompt );
   if( ch->pcdata->fprompt && *ch->pcdata->fprompt )
      fprintf( fp, "FPrompt	     %s~\n", ch->pcdata->fprompt );
   if( ch->pcdata->pagerlen != 24 )
      fprintf( fp, "Pagerlen     %d\n", ch->pcdata->pagerlen );

   /*
    * If ch is ignoring players then store those players 
    */
   {
      IGNORE_DATA *temp;
      for( temp = ch->pcdata->first_ignored; temp; temp = temp->next )
      {
         fprintf( fp, "Ignored      %s~\n", temp->name );
      }
   }

   if( IS_IMMORTAL( ch ) )
   {
      if( ch->pcdata->bamfin && ch->pcdata->bamfin[0] != '\0' )
         fprintf( fp, "Bamfin       %s~\n", ch->pcdata->bamfin );
      if( ch->pcdata->bamfout && ch->pcdata->bamfout[0] != '\0' )
         fprintf( fp, "Bamfout      %s~\n", ch->pcdata->bamfout );
      if( ch->trust )
         fprintf( fp, "Trust        %d\n", ch->trust );
      if( ch->pcdata && ch->pcdata->restore_time )
         fprintf( fp, "Restore_time %ld\n", ch->pcdata->restore_time );
      fprintf( fp, "WizInvis     %d\n", ch->pcdata->wizinvis );
      if( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
         fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo, ch->pcdata->r_range_hi );
      if( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
         fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo, ch->pcdata->o_range_hi );
      if( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
         fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo, ch->pcdata->m_range_hi );
   }
   if( ch->pcdata->council )
      fprintf( fp, "Council      %s~\n", ch->pcdata->council_name );
   if( ch->pcdata->deity_name && ch->pcdata->deity_name[0] != '\0' )
      fprintf( fp, "Deity	     %s~\n", ch->pcdata->deity_name );
   if( ch->pcdata->clan_name && ch->pcdata->clan_name[0] != '\0' )
      fprintf( fp, "Clan         %s~\n", ch->pcdata->clan_name );
   fprintf( fp, "Flags        %d\n", ch->pcdata->flags );
   if( ch->pcdata->release_date )
      fprintf( fp, "Helled       %d %s~\n", ( int )ch->pcdata->release_date, ch->pcdata->helled_by );
   fprintf( fp, "PKills       %d\n", ch->pcdata->pkills );
   fprintf( fp, "PDeaths      %d\n", ch->pcdata->pdeaths );
   if( get_timer( ch, TIMER_PKILLED ) && ( get_timer( ch, TIMER_PKILLED ) > 0 ) )
      fprintf( fp, "PTimer       %d\n", get_timer( ch, TIMER_PKILLED ) );
   fprintf( fp, "MKills       %d\n", ch->pcdata->mkills );
   fprintf( fp, "MDeaths      %d\n", ch->pcdata->mdeaths );
   fprintf( fp, "IllegalPK    %d\n", ch->pcdata->illegal_pk );
   fprintf( fp, "Timezone     %d\n", ch->pcdata->timezone );
   fprintf( fp, "AttrPerm     %d %d %d %d %d %d %d\n",
            ch->perm_str, ch->perm_int, ch->perm_wis, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck );

   fprintf( fp, "AttrMod      %d %d %d %d %d %d %d\n",
            ch->mod_str, ch->mod_int, ch->mod_wis, ch->mod_dex, ch->mod_con, ch->mod_cha, ch->mod_lck );

   fprintf( fp, "Condition    %d %d %d %d\n",
            ch->pcdata->condition[0], ch->pcdata->condition[1], ch->pcdata->condition[2], ch->pcdata->condition[3] );
   if( ch->pcdata->recent_site )
      fprintf( fp, "Site         %s\n", ch->pcdata->recent_site );
   else
      fprintf( fp, "Site         (Link-Dead)\n" );

   /* This MUST come after Room in pfile */
   if( xIS_SET( ch->in_room->room_flags, ROOM_HOUSE ) && !IS_IMMORTAL( ch ) )
   {
      HOME_DATA *tmphome;
      int i;

      for( tmphome = first_home; tmphome; tmphome = tmphome->next )
         for( i = 0; i < MAX_HOUSE_ROOMS; ++i )
            if( tmphome->vnum[i] == ch->in_room->vnum )
               fprintf( fp, "Homeowner %s~\n", tmphome->name );
   }
   else
   {
      HOMEBUY_DATA *tmphb;

      for( tmphb = first_homebuy; tmphb; tmphb = tmphb->next )
         if( ch->in_room->vnum == tmphb->vnum )
            fprintf( fp, "Homeowner %s~\n", tmphb->seller );
   }

   for( sn = 1; sn < num_skills; ++sn )
   {
      if( skill_table[sn]->name && ch->pcdata->learned[sn] > 0 )
         switch ( skill_table[sn]->type )
         {
            default:
               fprintf( fp, "Skill        %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
               break;
            case SKILL_RACIAL:
               fprintf( fp, "Ability      %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
               break;
            case SKILL_SPELL:
               fprintf( fp, "Spell        %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
               break;
            case SKILL_WEAPON:
               fprintf( fp, "Weapon       %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
               break;
            case SKILL_TONGUE:
               fprintf( fp, "Tongue       %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
               break;
         }
   }

   for( paf = ch->first_affect; paf; paf = paf->next )
   {
      if( paf->type >= 0 && ( skill = get_skilltype( paf->type ) ) == NULL )
         continue;

      if( paf->type >= 0 && paf->type < TYPE_PERSONAL )
         fprintf( fp, "AffectData   '%s' %3d %3d %3d %s\n",
                  skill->name, paf->duration, paf->modifier, paf->location, print_bitvector( &paf->bitvector ) );
      else
         fprintf( fp, "Affect       %3d %3d %3d %3d %s\n",
                  paf->type, paf->duration, paf->modifier, paf->location, print_bitvector( &paf->bitvector ) );
   }

   track = URANGE( 2, ( ( ch->level + 3 ) * MAX_KILLTRACK ) / LEVEL_AVATAR, MAX_KILLTRACK );
   for( sn = 0; sn < track; sn++ )
   {
      if( ch->pcdata->killed[sn].vnum == 0 )
         break;
      fprintf( fp, "Killed       %d %d\n", ch->pcdata->killed[sn].vnum, ch->pcdata->killed[sn].count );
   }

   /*
    * Save color values - Samson 9-29-98 
    */
   {
      int x;
      fprintf( fp, "MaxColors    %d\n", MAX_COLORS );
      fprintf( fp, "Colors       " );
      for( x = 0; x < MAX_COLORS; x++ )
         fprintf( fp, "%d ", ch->colors[x] );
      fprintf( fp, "\n" );
   }

   fprintf( fp, "End\n\n" );
}

/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest, short os_type, bool hotboot )
{
   EXTRA_DESCR_DATA *ed;
   AFFECT_DATA *paf;
   short wear, wear_loc, x;

   if( iNest >= MAX_NEST )
   {
      bug( "%s: iNest hit MAX_NEST %d", __func__, iNest );
      return;
   }

   if( !obj )
   {
      bug( "%s: NULL obj", __func__ );
      return;
   }

   /*
    * Slick recursion to write lists backwards,
    *   so loading them will load in forwards order.
    */
   if( obj->prev_content && os_type != OS_CORPSE )
      fwrite_obj( ch, obj->prev_content, fp, iNest, os_type, hotboot );

   /*
    * Castrate storage characters.
    * Catch deleted objects                 -Thoric
    * Do NOT save prototype items!          -Thoric
    */
   if( !hotboot )
   {
      if( (ch && ch->level < obj->level)
         || ( obj->item_type == ITEM_KEY && !IS_OBJ_STAT( obj, ITEM_CLANOBJECT ) )
         || ( (os_type == OS_VAULT) && (obj->item_type == ITEM_CORPSE_PC) )
         || obj_extracted( obj )
         || IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      return;
   }

   /*
    * DO NOT save corpses lying on the ground as a hotboot item, they already saved elsewhere! - Samson 
    */
   if( hotboot && obj->item_type == ITEM_CORPSE_PC )
      return;

   /*
    * Corpse saving. -- Altrag 
    */
   fprintf( fp, ( os_type == OS_CORPSE ? "#CORPSE\n" : "#OBJECT\n" ) );

   fprintf( fp, "Version      %d\n", SAVEVERSION );
   if( iNest )
      fprintf( fp, "Nest         %d\n", iNest );
   if( obj->count > 1 )
      fprintf( fp, "Count        %d\n", obj->count );
   if( obj->name && ( !obj->pIndexData->name || str_cmp( obj->name, obj->pIndexData->name ) ) )
      fprintf( fp, "Name         %s~\n", obj->name );
   if( obj->short_descr && ( !obj->pIndexData->short_descr || str_cmp( obj->short_descr, obj->pIndexData->short_descr ) ) )
      fprintf( fp, "ShortDescr   %s~\n", obj->short_descr );
   if( obj->description && ( !obj->pIndexData->description || str_cmp( obj->description, obj->pIndexData->description ) ) )
      fprintf( fp, "Description  %s~\n", obj->description );
   if( obj->action_desc && ( !obj->pIndexData->action_desc || str_cmp( obj->action_desc, obj->pIndexData->action_desc ) ) )
      fprintf( fp, "ActionDesc   %s~\n", obj->action_desc );
   if( obj->owner && obj->owner[0] != '\0' )
      fprintf( fp, "Owner        %s~\n", obj->owner );
   fprintf( fp, "Vnum         %d\n", obj->pIndexData->vnum );
   if( ( os_type == OS_CORPSE || os_type == OS_VAULT || hotboot ) && obj->in_room )
   {
      fprintf( fp, "Room      %d\n", obj->in_room->vnum );
      fprintf( fp, "Rvnum	   %d\n", obj->room_vnum );
   }
   if( !xSAME_BITS( obj->extra_flags, obj->pIndexData->extra_flags ) )
      fprintf( fp, "ExtraFlags   %s\n", print_bitvector( &obj->extra_flags ) );
   if( obj->wear_flags != obj->pIndexData->wear_flags )
      fprintf( fp, "WearFlags    %d\n", obj->wear_flags );

   wear_loc = WEAR_NONE;
   for( wear = 0; wear < MAX_WEAR && wear_loc == WEAR_NONE; wear++ )
   {
      for( x = 0; x < MAX_LAYERS; x++ )
      {
         if( ch )
         {
            if( IS_NPC( ch ) )
            {
               if( obj == mob_save_equipment[wear][x] )
               {
                  wear_loc = wear;
                  break;
               }
               else if( !mob_save_equipment[wear][x] )
                  break;
            }
            else
            {
               if( obj == save_equipment[wear][x] )
               {
                  wear_loc = wear;
                  break;
               }
               else if( !save_equipment[wear][x] )
                  break;
            }
         }
      }
   }
   if( wear_loc != -1 )
      fprintf( fp, "WearLoc      %d\n", wear_loc );
   if( obj->item_type != obj->pIndexData->item_type )
      fprintf( fp, "ItemType     %d\n", obj->item_type );
   if( obj->weight != obj->pIndexData->weight )
      fprintf( fp, "Weight       %d\n", obj->weight );
   if( obj->level )
      fprintf( fp, "Level        %d\n", obj->level );
   if( obj->timer )
      fprintf( fp, "Timer        %d\n", obj->timer );
   if( obj->cost != obj->pIndexData->cost )
      fprintf( fp, "Cost         %d\n", obj->cost );
   if( obj->value[0] || obj->value[1] || obj->value[2] || obj->value[3] || obj->value[4] || obj->value[5] )
      fprintf( fp, "Values       %d %d %d %d %d %d\n",
               obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4], obj->value[5] );

   switch ( obj->item_type )
   {
      case ITEM_PILL:  /* was down there with staff and wand, wrongly - Scryn */
      case ITEM_POTION:
      case ITEM_SCROLL:
         if( IS_VALID_SN( obj->value[1] ) )
            fprintf( fp, "Spell 1      '%s'\n", skill_table[obj->value[1]]->name );

         if( IS_VALID_SN( obj->value[2] ) )
            fprintf( fp, "Spell 2      '%s'\n", skill_table[obj->value[2]]->name );

         if( IS_VALID_SN( obj->value[3] ) )
            fprintf( fp, "Spell 3      '%s'\n", skill_table[obj->value[3]]->name );

         break;

      case ITEM_STAFF:
      case ITEM_WAND:
         if( IS_VALID_SN( obj->value[3] ) )
            fprintf( fp, "Spell 3      '%s'\n", skill_table[obj->value[3]]->name );

         break;
      case ITEM_SALVE:
         if( IS_VALID_SN( obj->value[4] ) )
            fprintf( fp, "Spell 4      '%s'\n", skill_table[obj->value[4]]->name );

         if( IS_VALID_SN( obj->value[5] ) )
            fprintf( fp, "Spell 5      '%s'\n", skill_table[obj->value[5]]->name );
         break;
   }

   for( paf = obj->first_affect; paf; paf = paf->next )
   {
      /*
       * Save extra object affects           -Thoric
       */
      fwrite_fuss_affect( fp, paf );
   }

   for( ed = obj->first_extradesc; ed; ed = ed->next )
      fwrite_fuss_exdesc( fp, ed );

   fprintf( fp, "End\n\n" );

   if( obj->first_content )
      fwrite_obj( ch, obj->last_content, fp, iNest + 1, OS_CARRY, hotboot );
}

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA * d, char *name, bool preload, bool copyover )
{
   char strsave[MAX_INPUT_LENGTH];
   CHAR_DATA *ch;
   FILE *fp;
   bool found;
   struct stat fst;
   int i, x;

   CREATE( ch, CHAR_DATA, 1 );
   for( x = 0; x < MAX_WEAR; x++ )
      for( i = 0; i < MAX_LAYERS; i++ )
      {
         save_equipment[x][i] = NULL;
         mob_save_equipment[x][i] = NULL;
      }
   clear_char( ch );
   loading_char = ch;

   CREATE( ch->pcdata, PC_DATA, 1 );
   d->character = ch;
   ch->desc = d;
   ch->pcdata->filename = STRALLOC( name );
   ch->name = NULL;
   if( d->host )
      ch->pcdata->recent_site = STRALLOC( d->host );
   ch->act = multimeb( PLR_BLANK, PLR_COMBINE, PLR_PROMPT, -1 );
   ch->perm_str = 13;
   ch->perm_int = 13;
   ch->perm_wis = 13;
   ch->perm_dex = 13;
   ch->perm_con = 13;
   ch->perm_cha = 13;
   ch->perm_lck = 13;
   ch->no_resistant = 0;
   ch->no_susceptible = 0;
   ch->no_immune = 0;
   ch->was_in_room = NULL;
   xCLEAR_BITS( ch->no_affected_by );
   ch->pcdata->condition[COND_THIRST] = 48;
   ch->pcdata->condition[COND_FULL] = 48;
   ch->pcdata->condition[COND_BLOODTHIRST] = 10;
   ch->pcdata->nuisance = NULL;
   ch->pcdata->wizinvis = 0;
   ch->pcdata->charmies = 0;
   ch->mental_state = -10;
   ch->mobinvis = 0;
   for( i = 0; i < MAX_SKILL; i++ )
      ch->pcdata->learned[i] = 0;
   ch->pcdata->release_date = 0;
   ch->pcdata->helled_by = NULL;
   ch->saving_poison_death = 0;
   ch->saving_wand = 0;
   ch->saving_para_petri = 0;
   ch->saving_breath = 0;
   ch->saving_spell_staff = 0;
   ch->style = STYLE_FIGHTING;
   ch->comments = NULL; /* comments */
   ch->pcdata->pagerlen = 24;
   ch->pcdata->first_ignored = NULL;   /* Ignore list */
   ch->pcdata->last_ignored = NULL;
   ch->pcdata->tell_history = NULL; /* imm only lasttell cmnd */
   ch->pcdata->lt_index = 0;  /* last tell index */
   ch->morph = NULL;
   ch->pcdata->hotboot = FALSE;  /* Never changed except when PC is saved during hotboot save */

   found = FALSE;
   snprintf( strsave, MAX_INPUT_LENGTH, "%s%c/%s", PLAYER_DIR, tolower( name[0] ), capitalize( name ) );
   if( stat( strsave, &fst ) != -1 )
   {
      if( fst.st_size == 0 )
      {
         snprintf( strsave, MAX_INPUT_LENGTH, "%s%c/%s", BACKUP_DIR, tolower( name[0] ), capitalize( name ) );
         send_to_char( "Restoring your backup player file...", ch );
      }
      else
      {
         log_printf_plus( LOG_COMM, LEVEL_GREATER, "%s player data for: %s (%dK)",
                          preload ? "Preloading" : "Loading", ch->pcdata->filename, ( int )fst.st_size / 1024 );
      }
   }

   /*
    * else no player file 
    */
   if( ( fp = fopen( strsave, "r" ) ) != NULL )
   {
      int iNest;

      for( iNest = 0; iNest < MAX_NEST; iNest++ )
         rgObjNest[iNest] = NULL;

      found = TRUE;
      /*
       * Cheat so that bug will show line #'s -- Altrag 
       */
      fpArea = fp;
      mudstrlcpy( strArea, strsave, MAX_INPUT_LENGTH );
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
            bug( "%s: # not found. (%s)", __func__, name );
            break;
         }

         word = fread_word( fp );
         if( !strcmp( word, "PLAYER" ) )
         {
            fread_char( ch, fp, preload, copyover );
            if( preload )
               break;
         }
         else if( !strcmp( word, "OBJECT" ) )   /* Objects  */
            fread_obj( ch, fp, OS_CARRY );
         else if( !strcmp( word, "MorphData" ) )   /* Morphs */
            fread_morph_data( ch, fp );
         else if( !strcmp( word, "COMMENT" ) )
            fread_comment( ch, fp );   /* Comments */
         else if( !strcmp( word, "MOBILE" ) )
         {
            CHAR_DATA *mob;

            if( ( mob = fread_mobile( fp ) ) )
            {
               ch->pcdata->pet = mob;
               mob->master = ch;
               xSET_BIT( mob->affected_by, AFF_CHARM );
            }
            else
               bug( "%s: Deleted mob saved on %s - skipping", __func__, ch->name );
         }
         else if( !str_cmp( word, "VARIABLE" ) )   // Quest Flags
            fread_variable( ch, fp );
         else if( !strcmp( word, "END" ) )   /* Done     */
            break;
         else
         {
            bug( "%s: bad section: %s", __func__, word );
            break;
         }
      }
      FCLOSE( fp );
      fpArea = NULL;
      mudstrlcpy( strArea, "$", MAX_INPUT_LENGTH );
   }

   if( !found )
   {
      ch->name = STRALLOC( name );
      ch->short_descr = STRALLOC( "" );
      ch->long_descr = STRALLOC( "" );
      ch->description = STRALLOC( "" );
      ch->editor = NULL;
      ch->pcdata->clan_name = STRALLOC( "" );
      ch->pcdata->clan = NULL;
      ch->pcdata->council_name = STRALLOC( "" );
      ch->pcdata->council = NULL;
      ch->pcdata->deity_name = STRALLOC( "" );
      ch->pcdata->deity = NULL;
      ch->pcdata->pet = NULL;
      ch->pcdata->pwd = str_dup( "" );
      ch->pcdata->bamfin = str_dup( "" );
      ch->pcdata->bamfout = str_dup( "" );
      ch->pcdata->rank = str_dup( "" );
      ch->pcdata->bestowments = str_dup( "" );
      ch->pcdata->title = STRALLOC( "" );
      ch->pcdata->homepage = str_dup( "" );
      ch->pcdata->bio = STRALLOC( "" );
      ch->pcdata->authed_by = STRALLOC( "" );
      ch->pcdata->prompt = STRALLOC( "" );
      ch->pcdata->fprompt = STRALLOC( "" );
      ch->pcdata->r_range_lo = 0;
      ch->pcdata->r_range_hi = 0;
      ch->pcdata->m_range_lo = 0;
      ch->pcdata->m_range_hi = 0;
      ch->pcdata->o_range_lo = 0;
      ch->pcdata->o_range_hi = 0;
      ch->pcdata->wizinvis = 0;
      ch->pcdata->timezone = -1;
   }
   else
   {
      if( !ch->name )
         ch->name = STRALLOC( name );
      if( !ch->pcdata->clan_name )
      {
         ch->pcdata->clan_name = STRALLOC( "" );
         ch->pcdata->clan = NULL;
      }
      if( !ch->pcdata->council_name )
      {
         ch->pcdata->council_name = STRALLOC( "" );
         ch->pcdata->council = NULL;
      }
      if( !ch->pcdata->deity_name )
      {
         ch->pcdata->deity_name = STRALLOC( "" );
         ch->pcdata->deity = NULL;
      }
      if( !ch->pcdata->bio )
         ch->pcdata->bio = STRALLOC( "" );

      if( !ch->pcdata->authed_by )
         ch->pcdata->authed_by = STRALLOC( "" );

      if( xIS_SET( ch->act, PLR_FLEE ) )
         xREMOVE_BIT( ch->act, PLR_FLEE );

      if( IS_IMMORTAL( ch ) )
      {
         if( ch->pcdata->wizinvis < 2 )
            ch->pcdata->wizinvis = ch->level;
         assign_area( ch );
      }
      if( file_ver > 1 )
      {
         for( i = 0; i < MAX_WEAR; i++ )
            for( x = 0; x < MAX_LAYERS; x++ )
               if( save_equipment[i][x] )
               {
                  equip_char( ch, save_equipment[i][x], i );
                  save_equipment[i][x] = NULL;
               }
               else
                  break;
      }
   }

   /*
    * Rebuild affected_by and RIS to catch errors - FB 
    */
   update_aris( ch );
   loading_char = NULL;
   return found;
}

/*
 * Read in a char.
 */
void fread_char( CHAR_DATA * ch, FILE * fp, bool preload, bool copyover )
{
   char buf[MAX_STRING_LENGTH];
   char *line;
   const char *word;
   int x1, x2, x3, x4, x5, x6, x7;
   short killcnt;
   bool fMatch;
   int max_colors = 0;  /* Color code */

   file_ver = 0;
   killcnt = 0;

   /*
    * Setup color values in case player has none set - Samson 
    */
   memcpy( &ch->colors, &default_set, sizeof( default_set ) );

   for( ;; )
   {
      word = ( feof( fp ) ? "End" : fread_word( fp ) );

      if( word[0] == '\0' )
      {
         bug( "%s: EOF encountered reading file!", __func__ );
         word = "End";
      }
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            KEY( "Act", ch->act, fread_bitvector( fp ) );
            KEY( "AffectedBy", ch->affected_by, fread_bitvector( fp ) );
            KEY( "Alignment", ch->alignment, fread_number( fp ) );
            KEY( "Armor", ch->armor, fread_number( fp ) );

            if( !str_cmp( word, "Ability" ) )
            {
               int sn, value;

               if( preload )
                  word = "End";
               else
               {
                  value = fread_number( fp );
                  if( file_ver < 3 )
                     sn = skill_lookup( fread_word( fp ) );
                  else
                     sn = find_ability( NULL, fread_word( fp ), FALSE );

                  if( sn < 0 )
                     bug( "%s: unknown skill.", __func__ );
                  else
                  {
                     ch->pcdata->learned[sn] = value;
                     if( ch->level < LEVEL_IMMORTAL )
                     {
                        if( skill_table[sn]->race_level[ch->race] >= LEVEL_IMMORTAL )
                        {
                           ch->pcdata->learned[sn] = 0;
                           ++ch->practice;
                        }
                     }
                  }
                  fMatch = TRUE;
                  break;
               }
            }

            if( !strcmp( word, "Affect" ) || !strcmp( word, "AffectData" ) )
            {
               AFFECT_DATA *paf;

               if( preload )
               {
                  fMatch = TRUE;
                  fread_to_eol( fp );
                  break;
               }

               CREATE( paf, AFFECT_DATA, 1 );
               if( !strcmp( word, "Affect" ) )
               {
                  paf->type = fread_number( fp );
               }
               else
               {
                  int sn;
                  char *sname = fread_word( fp );

                  if( ( sn = skill_lookup( sname ) ) < 0 )
                  {
                     if( ( sn = herb_lookup( sname ) ) < 0 )
                        bug( "%s: unknown skill %s.", __func__, sname );
                     else
                        sn += TYPE_HERB;
                  }
                  paf->type = sn;
               }

               paf->duration = fread_number( fp );
               paf->modifier = fread_number( fp );
               paf->location = fread_number( fp );
               if( paf->location == APPLY_WEAPONSPELL
                   || paf->location == APPLY_WEARSPELL
                   || paf->location == APPLY_REMOVESPELL
                   || paf->location == APPLY_STRIPSN || paf->location == APPLY_RECURRINGSPELL )
                  paf->modifier = slot_lookup( paf->modifier );
               paf->bitvector = fread_bitvector( fp );
               LINK( paf, ch->first_affect, ch->last_affect, next, prev );
               fMatch = TRUE;
               break;
            }

            if( file_ver < 5 )
               find_old_age( ch );
            else
            {
               if( !str_cmp( word, "Age" ) )
               {
                  line = fread_line( fp );
                  x1 = x2 = x3 = x4 = 0;
                  sscanf( line, "%d %d %d %d", &x1, &x2, &x3, &x4 );
                  ch->pcdata->age_bonus = x1;
                  ch->pcdata->day = x2;
                  ch->pcdata->month = x3;
                  ch->pcdata->year = x4;
                  fMatch = TRUE;
                  break;
               }
            }

            if( !strcmp( word, "AttrMod" ) )
            {
               line = fread_line( fp );
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = 13;
               sscanf( line, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
               ch->mod_str = x1;
               ch->mod_int = x2;
               ch->mod_wis = x3;
               ch->mod_dex = x4;
               ch->mod_con = x5;
               ch->mod_cha = x6;
               ch->mod_lck = x7;
               if( !x7 )
                  ch->mod_lck = 0;
               fMatch = TRUE;
               break;
            }

            if( !strcmp( word, "AttrPerm" ) )
            {
               line = fread_line( fp );
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;
               sscanf( line, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
               ch->perm_str = x1;
               ch->perm_int = x2;
               ch->perm_wis = x3;
               ch->perm_dex = x4;
               ch->perm_con = x5;
               ch->perm_cha = x6;
               ch->perm_lck = x7;
               if( x7 == 0 )
                  ch->perm_lck = 13;
               fMatch = TRUE;
               break;
            }
            KEY( "AuthedBy", ch->pcdata->authed_by, fread_string( fp ) );
            break;

         case 'B':
            KEY( "Bamfin", ch->pcdata->bamfin, fread_string_nohash( fp ) );
            KEY( "Bamfout", ch->pcdata->bamfout, fread_string_nohash( fp ) );
            KEY( "Bestowments", ch->pcdata->bestowments, fread_string_nohash( fp ) );
            KEY( "Bio", ch->pcdata->bio, fread_string( fp ) );
            break;

         case 'C':
            if( !strcmp( word, "Clan" ) )
            {
               ch->pcdata->clan_name = fread_string( fp );

               if( !preload
                   && ch->pcdata->clan_name[0] != '\0' && ( ch->pcdata->clan = get_clan( ch->pcdata->clan_name ) ) == NULL )
               {
                  snprintf( buf, MAX_STRING_LENGTH, "&R\r\nWarning: The organization %s no longer exists, and therefore you no longer\r\nbelong to that organization.\r\n",
                     ch->pcdata->clan_name );
                  add_loginmsg( ch->name, 18, buf );
                  STRFREE( ch->pcdata->clan_name );
                  ch->pcdata->clan_name = STRALLOC( "" );
               }
               fMatch = TRUE;
               break;
            }

            KEY( "Class", ch->Class, fread_number( fp ) );

            /*
             * Load color values - Samson 9-29-98 
             */
            if( !str_cmp( word, "Colors" ) )
            {
               int x;

               for( x = 0; x < max_colors; x++ )
                  ch->colors[x] = fread_number( fp );
               fread_to_eol( fp );
               fMatch = TRUE;
               break;
            }

            if( !str_cmp( word, "Condition" ) )
            {
               line = fread_line( fp );
               sscanf( line, "%d %d %d %d", &x1, &x2, &x3, &x4 );
               ch->pcdata->condition[0] = x1;
               ch->pcdata->condition[1] = x2;
               ch->pcdata->condition[2] = x3;
               ch->pcdata->condition[3] = x4;
               fMatch = TRUE;
               break;
            }

            if( !strcmp( word, "Council" ) )
            {
               ch->pcdata->council_name = fread_string( fp );
               if( !preload
                   && ch->pcdata->council_name[0] != '\0'
                   && ( ch->pcdata->council = get_council( ch->pcdata->council_name ) ) == NULL )
               {
                  snprintf( buf, MAX_STRING_LENGTH, "&Y\r\nWarning:  the council %s no longer exists, and therefore you no longer\r\nbelong to a council.\r\n",
                     ch->pcdata->council_name );
                  add_loginmsg( ch->name, 18, buf );
                  STRFREE( ch->pcdata->council_name );
                  ch->pcdata->council_name = STRALLOC( "" );
               }
               fMatch = TRUE;
               break;
            }
            break;

         case 'D':
            KEY( "Damroll", ch->damroll, fread_number( fp ) );
            KEY( "Deaf", ch->deaf, fread_number( fp ) );
            if( !strcmp( word, "Deity" ) )
            {
               ch->pcdata->deity_name = fread_string( fp );

               if( !preload
                   && ch->pcdata->deity_name[0] != '\0'
                   && ( ch->pcdata->deity = get_deity( ch->pcdata->deity_name ) ) == NULL )
               {
                  snprintf( buf, MAX_STRING_LENGTH, "&R\r\nYour deity, %s, has met its demise!\r\n", ch->pcdata->deity_name );
                  add_loginmsg( ch->name, 18, buf );
                  STRFREE( ch->pcdata->deity_name );
                  ch->pcdata->deity_name = STRALLOC( "" );
                  ch->pcdata->favor = 0;
               }
               fMatch = TRUE;
               break;
            }
            KEY( "Description", ch->description, fread_string( fp ) );
            break;

            /*
             * 'E' was moved to after 'S' 
             */
         case 'F':
            KEY( "Favor", ch->pcdata->favor, fread_number( fp ) );
            if( !strcmp( word, "Filename" ) )
            {
               /*
                * File Name already set externally.
                */
               fread_to_eol( fp );
               fMatch = TRUE;
               break;
            }
            KEY( "Flags", ch->pcdata->flags, fread_number( fp ) );
            KEY( "FPrompt", ch->pcdata->fprompt, fread_string( fp ) );
            break;

         case 'G':
            KEY( "Glory", ch->pcdata->quest_curr, fread_number( fp ) );
            KEY( "Gold", ch->gold, fread_number( fp ) );
            /*
             * temporary measure 
             */
            if( !strcmp( word, "Guild" ) )
            {
               ch->pcdata->clan_name = fread_string( fp );

               if( !preload
                   && ch->pcdata->clan_name[0] != '\0' && ( ch->pcdata->clan = get_clan( ch->pcdata->clan_name ) ) == NULL )
               {
                  snprintf( buf, MAX_STRING_LENGTH, "&R\r\nWarning: The organization %s no longer exists, and therefore you no longer\r\nbelong to that organization.\r\n",
                     ch->pcdata->clan_name );
                  add_loginmsg( ch->name, 18, buf );
                  STRFREE( ch->pcdata->clan_name );
                  ch->pcdata->clan_name = STRALLOC( "" );
               }
               fMatch = TRUE;
               break;
            }
            break;

         case 'H':
            KEY( "Height", ch->height, fread_number( fp ) );

            if( !strcmp( word, "Helled" ) )
            {
               ch->pcdata->release_date = fread_number( fp );
               ch->pcdata->helled_by = fread_string( fp );
               fMatch = TRUE;
               break;
            }

            KEY( "Hitroll", ch->hitroll, fread_number( fp ) );
            KEY( "Homepage", ch->pcdata->homepage, fread_string_nohash( fp ) );

            if( !strcmp( word, "Homeowner" ) )
            {
               int i;

               fMatch = TRUE;

               if( ch->in_room && xIS_SET( ch->in_room->room_flags, ROOM_HOUSE ) )
               {
                  HOME_DATA *tmphome;

                  for( tmphome = first_home; tmphome; tmphome = tmphome->next )
                     for( i = 0; i < MAX_HOUSE_ROOMS; ++i )
                        if( tmphome->vnum[i] == ch->in_room->vnum )
                           if( strcmp( tmphome->name, fread_string( fp ) ) )
                              ch->in_room = get_room_index( ROOM_VNUM_TEMPLE );
               }
               else
                  fread_flagstring( fp );
               break;
            }

            if( !strcmp( word, "HpManaMove" ) )
            {
               ch->hit = fread_number( fp );
               ch->max_hit = fread_number( fp );
               ch->mana = fread_number( fp );
               ch->max_mana = fread_number( fp );
               ch->move = fread_number( fp );
               ch->max_move = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'I':
            if( !strcmp( word, "Ignored" ) )
            {
               const char *temp;
               char fname[1024];
               struct stat fst;
               int ign;
               IGNORE_DATA *inode;

               /*
                * Get the name 
                */
               temp = fread_string( fp );

               snprintf( fname, 1024, "%s%c/%s", PLAYER_DIR, tolower( temp[0] ), capitalize( temp ) );

               /*
                * If there isn't a pfile for the name 
                */
               /*
                * then don't add it to the list       
                */
               if( stat( fname, &fst ) == -1 )
               {
                  if( temp )
                     STRFREE( temp );
                  fMatch = TRUE;
                  break;
               }

               /*
                * Count the number of names already ignored 
                */
               for( ign = 0, inode = ch->pcdata->first_ignored; inode; inode = inode->next )
               {
                  ++ign;
               }

               /*
                * Add the name unless the limit has been reached 
                */
               if( ign >= MAX_IGN )
               {
                  bug( "%s: too many ignored names", __func__ );
               }
               else
               {
                  /*
                   * Add the name to the list 
                   */
                  CREATE( inode, IGNORE_DATA, 1 );
                  inode->name = STRALLOC( temp );
                  inode->next = NULL;
                  inode->prev = NULL;

                  LINK( inode, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev );
               }
               if( temp )
                  STRFREE( temp );
               fMatch = TRUE;
               break;
            }
            KEY( "IllegalPK", ch->pcdata->illegal_pk, fread_number( fp ) );
            KEY( "Immune", ch->immune, fread_number( fp ) );
            break;

         case 'K':
            if( !strcmp( word, "Killed" ) )
            {
               if( killcnt >= MAX_KILLTRACK )
                  bug( "%s: killcnt (%d) >= MAX_KILLTRACK", __func__, killcnt );
               else
               {
                  ch->pcdata->killed[killcnt].vnum = fread_number( fp );
                  ch->pcdata->killed[killcnt++].count = fread_number( fp );
               }
               fMatch = TRUE;
               break;
            }
            break;

         case 'L':
            KEY( "Level", ch->level, fread_number( fp ) );
            KEY( "LongDescr", ch->long_descr, fread_string( fp ) );
            if( !strcmp( word, "Languages" ) )
            {
               ch->speaks = fread_number( fp );
               ch->speaking = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'M':
            if( !str_cmp( word, "MaxColors" ) )
            {
               int temp = fread_number( fp );

               max_colors = UMIN( temp, MAX_COLORS );

               fMatch = TRUE;
               break;
            }
            KEY( "MDeaths", ch->pcdata->mdeaths, fread_number( fp ) );
            KEY( "Mentalstate", ch->mental_state, fread_number( fp ) );
            KEY( "MGlory", ch->pcdata->quest_accum, fread_number( fp ) );
            KEY( "Minsnoop", ch->pcdata->min_snoop, fread_number( fp ) );
            KEY( "MKills", ch->pcdata->mkills, fread_number( fp ) );
            KEY( "Mobinvis", ch->mobinvis, fread_number( fp ) );
            if( !strcmp( word, "MobRange" ) )
            {
               ch->pcdata->m_range_lo = fread_number( fp );
               ch->pcdata->m_range_hi = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'N':
            KEY( "Name", ch->name, fread_string( fp ) );
            KEY( "NoAffectedBy", ch->no_affected_by, fread_bitvector( fp ) );
            KEY( "NoImmune", ch->no_immune, fread_number( fp ) );
            KEY( "NoResistant", ch->no_resistant, fread_number( fp ) );
            KEY( "NoSusceptible", ch->no_susceptible, fread_number( fp ) );
            if( !str_cmp( "Nuisance", word ) )
            {
               CREATE( ch->pcdata->nuisance, NUISANCE_DATA, 1 );

               ch->pcdata->nuisance->set_time = fread_number( fp );
               ch->pcdata->nuisance->max_time = fread_number( fp );
               ch->pcdata->nuisance->flags = fread_number( fp );
               ch->pcdata->nuisance->power = 1;

               fMatch = TRUE;
               break;
            }

            if( !str_cmp( "NuisanceNew", word ) )
            {
               CREATE( ch->pcdata->nuisance, NUISANCE_DATA, 1 );

               ch->pcdata->nuisance->set_time = fread_number( fp );
               ch->pcdata->nuisance->max_time = fread_number( fp );
               ch->pcdata->nuisance->flags = fread_number( fp );
               ch->pcdata->nuisance->power = fread_number( fp );

               fMatch = TRUE;
               break;
            }
            break;

         case 'O':
            KEY( "Outcast_time", ch->pcdata->outcast_time, fread_number( fp ) );
            if( !strcmp( word, "ObjRange" ) )
            {
               ch->pcdata->o_range_lo = fread_number( fp );
               ch->pcdata->o_range_hi = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'P':
            KEY( "Pagerlen", ch->pcdata->pagerlen, fread_number( fp ) );
            KEY( "Password", ch->pcdata->pwd, fread_string_nohash( fp ) );
            KEY( "PDeaths", ch->pcdata->pdeaths, fread_number( fp ) );
            KEY( "PKills", ch->pcdata->pkills, fread_number( fp ) );
            KEY( "Played", ch->played, fread_number( fp ) );
            /*
             *  new positions are stored in the file from 100 up
             *  old positions are from 0 up
             *  if reading an old position, some translation is necessary
             */
            if( !strcmp( word, "Position" ) )
            {
               ch->position = fread_number( fp );
               if( ch->position < 100 )
               {
                  switch ( ch->position )
                  {
                     default:;
                     case 0:;
                     case 1:;
                     case 2:;
                     case 3:;
                     case 4:
                        break;
                     case 5:
                        ch->position = 6;
                        break;
                     case 6:
                        ch->position = 8;
                        break;
                     case 7:
                        ch->position = 9;
                        break;
                     case 8:
                        ch->position = 12;
                        break;
                     case 9:
                        ch->position = 13;
                        break;
                     case 10:
                        ch->position = 14;
                        break;
                     case 11:
                        ch->position = 15;
                        break;
                  }
                  fMatch = TRUE;
               }
               else
               {
                  ch->position -= 100;
                  fMatch = TRUE;
               }
               break;
            }
            KEY( "Practice", ch->practice, fread_number( fp ) );
            KEY( "Prompt", ch->pcdata->prompt, fread_string( fp ) );
            if( !strcmp( word, "PTimer" ) )
            {
               add_timer( ch, TIMER_PKILLED, fread_number( fp ), NULL, 0 );
               fMatch = TRUE;
               break;
            }
            break;

         case 'R':
            KEY( "Race", ch->race, fread_number( fp ) );
            KEY( "Rank", ch->pcdata->rank, fread_string_nohash( fp ) );
            KEY( "Resistant", ch->resistant, fread_number( fp ) );
            KEY( "Restore_time", ch->pcdata->restore_time, fread_number( fp ) );

            if( !strcmp( word, "Room" ) )
            {
               ch->in_room = get_room_index( fread_number( fp ) );
               if( !ch->in_room )
                  ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
               fMatch = TRUE;
               break;
            }

            if( !strcmp( word, "RoomRange" ) )
            {
               ch->pcdata->r_range_lo = fread_number( fp );
               ch->pcdata->r_range_hi = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'S':
            KEY( "Sex", ch->sex, fread_number( fp ) );
            KEY( "ShortDescr", ch->short_descr, fread_string( fp ) );
            KEY( "Style", ch->style, fread_number( fp ) );
            KEY( "Susceptible", ch->susceptible, fread_number( fp ) );
            if( !strcmp( word, "SavingThrow" ) )
            {
               ch->saving_wand = fread_number( fp );
               ch->saving_poison_death = ch->saving_wand;
               ch->saving_para_petri = ch->saving_wand;
               ch->saving_breath = ch->saving_wand;
               ch->saving_spell_staff = ch->saving_wand;
               fMatch = TRUE;
               break;
            }

            if( !strcmp( word, "SavingThrows" ) )
            {
               ch->saving_poison_death = fread_number( fp );
               ch->saving_wand = fread_number( fp );
               ch->saving_para_petri = fread_number( fp );
               ch->saving_breath = fread_number( fp );
               ch->saving_spell_staff = fread_number( fp );
               fMatch = TRUE;
               break;
            }

            if( !strcmp( word, "Site" ) )
            {
               if( !preload && !copyover )
               {
                  ch->pcdata->prev_site = STRALLOC( fread_word( fp ) );
                  ch_printf( ch, "Last connected from: %s\r\n", ch->pcdata->prev_site );
               }
               else
                  fread_to_eol( fp );
               fMatch = TRUE;
               if( preload )
                  word = "End";
               else
                  break;
            }

            if( !strcmp( word, "Skill" ) )
            {
               int sn, value;

               if( preload )
                  word = "End";
               else
               {
                  value = fread_number( fp );
                  if( file_ver < 3 )
                     sn = skill_lookup( fread_word( fp ) );
                  else
                     sn = find_skill( NULL, fread_word( fp ), FALSE );

                  if( sn < 0 )
                     bug( "%s: unknown skill.", __func__ );
                  else
                  {
                     ch->pcdata->learned[sn] = value;
                     /*
                      * Take care of people who have stuff they shouldn't     *
                      * * Assumes class and level were loaded before. -- Altrag *
                      * * Assumes practices are loaded first too now. -- Altrag 
                      */
                     if( ch->level < LEVEL_IMMORTAL )
                     {
                        if( skill_table[sn]->skill_level[ch->Class] >= LEVEL_IMMORTAL )
                        {
                           ch->pcdata->learned[sn] = 0;
                           ++ch->practice;
                        }
                     }
                  }
                  fMatch = TRUE;
                  break;
               }
            }

            if( !strcmp( word, "Spell" ) )
            {
               int sn, value;

               if( preload )
                  word = "End";
               else
               {
                  value = fread_number( fp );

                  sn = find_spell( NULL, fread_word( fp ), FALSE );
                  if( sn < 0 )
                     bug( "%s: unknown spell.", __func__ );
                  else
                  {
                     ch->pcdata->learned[sn] = value;
                     if( ch->level < LEVEL_IMMORTAL )
                        if( skill_table[sn]->skill_level[ch->Class] >= LEVEL_IMMORTAL )
                        {
                           ch->pcdata->learned[sn] = 0;
                           ++ch->practice;
                        }
                  }
                  fMatch = TRUE;
                  break;
               }
            }
            if( strcmp( word, "End" ) )
               break;

         case 'E':
            if( !strcmp( word, "End" ) )
            {
               if( !ch->short_descr )
                  ch->short_descr = STRALLOC( "" );
               if( !ch->long_descr )
                  ch->long_descr = STRALLOC( "" );
               if( !ch->description )
                  ch->description = STRALLOC( "" );
               if( !ch->pcdata->pwd )
                  ch->pcdata->pwd = str_dup( "" );
               if( !ch->pcdata->bamfin )
                  ch->pcdata->bamfin = str_dup( "" );
               if( !ch->pcdata->bamfout )
                  ch->pcdata->bamfout = str_dup( "" );
               if( !ch->pcdata->bio )
                  ch->pcdata->bio = STRALLOC( "" );
               if( !ch->pcdata->rank )
                  ch->pcdata->rank = str_dup( "" );
               if( !ch->pcdata->bestowments )
                  ch->pcdata->bestowments = str_dup( "" );
               if( !ch->pcdata->title )
                  ch->pcdata->title = STRALLOC( "" );
               if( !ch->pcdata->homepage )
                  ch->pcdata->homepage = str_dup( "" );
               if( !ch->pcdata->authed_by )
                  ch->pcdata->authed_by = STRALLOC( "" );
               if( !ch->pcdata->prompt )
                  ch->pcdata->prompt = STRALLOC( "" );
               if( !ch->pcdata->fprompt )
                  ch->pcdata->fprompt = STRALLOC( "" );
               ch->editor = NULL;
               killcnt = URANGE( 2, ( ( ch->level + 3 ) * MAX_KILLTRACK ) / LEVEL_AVATAR, MAX_KILLTRACK );
               if( killcnt < MAX_KILLTRACK )
                  ch->pcdata->killed[killcnt].vnum = 0;

               /*
                * no good for newbies at all 
                */
               if( !IS_IMMORTAL( ch ) && !ch->speaking )
                  ch->speaking = LANG_COMMON;
               /*
                * ch->speaking = race_table[ch->race]->language; 
                */
               if( IS_IMMORTAL( ch ) )
               {
                  int i;

                  ch->speaks = ~0;
                  if( ch->speaking == 0 )
                     ch->speaking = ~0;

                  CREATE( ch->pcdata->tell_history, const char *, 26 );
                  for( i = 0; i < 26; i++ )
                     ch->pcdata->tell_history[i] = NULL;
               }
               if( !ch->pcdata->prompt )
                  ch->pcdata->prompt = STRALLOC( "" );

               /*
                * this disallows chars from being 6', 180lbs, but easier than a flag 
                */
               if( ch->height == 72 )
                  ch->height =
                     number_range( ( int )( race_table[ch->race]->height * .9 ),
                                   ( int )( race_table[ch->race]->height * 1.1 ) );
               if( ch->weight == 180 )
                  ch->weight =
                     number_range( ( int )( race_table[ch->race]->weight * .9 ),
                                   ( int )( race_table[ch->race]->weight * 1.1 ) );
               if( ch->pcdata->clan )
                  update_roster( ch );
               return;
            }
            KEY( "Exp", ch->exp, fread_number( fp ) );
            break;

         case 'T':
            if( !strcmp( word, "Tongue" ) )
            {
               int sn, value;

               if( preload )
                  word = "End";
               else
               {
                  value = fread_number( fp );

                  sn = find_tongue( NULL, fread_word( fp ), FALSE );
                  if( sn < 0 )
                     bug( "%s: unknown tongue.", __func__ );
                  else
                  {
                     ch->pcdata->learned[sn] = value;
                     if( ch->level < LEVEL_IMMORTAL )
                        if( skill_table[sn]->skill_level[ch->Class] >= LEVEL_IMMORTAL )
                        {
                           ch->pcdata->learned[sn] = 0;
                           ch->practice++;
                        }
                  }
                  fMatch = TRUE;
               }
               break;
            }
            KEY( "Trust", ch->trust, fread_number( fp ) );
            KEY( "Timezone", ch->pcdata->timezone, fread_number( fp )); 
            /*
             * Let no character be trusted higher than one below maxlevel -- Narn 
             */
            ch->trust = UMIN( ch->trust, MAX_LEVEL - 1 );

            if( !strcmp( word, "Title" ) )
            {
               ch->pcdata->title = fread_string( fp );
               if( isalpha( ch->pcdata->title[0] ) || isdigit( ch->pcdata->title[0] ) )
               {
                  snprintf( buf, MAX_STRING_LENGTH, " %s", ch->pcdata->title );
                  if( ch->pcdata->title )
                     STRFREE( ch->pcdata->title );
                  ch->pcdata->title = STRALLOC( buf );
               }
               fMatch = TRUE;
               break;
            }
            break;

         case 'V':
            KEY( "Version", file_ver, fread_number( fp ) );
            break;

         case 'W':
            KEY( "Weight", ch->weight, fread_number( fp ) );
            if( !strcmp( word, "Weapon" ) )
            {
               int sn, value;

               if( preload )
                  word = "End";
               else
               {
                  value = fread_number( fp );

                  sn = find_weapon( NULL, fread_word( fp ), FALSE );
                  if( sn < 0 )
                     bug( "%s: unknown weapon.", __func__ );
                  else
                  {
                     ch->pcdata->learned[sn] = value;
                     if( ch->level < LEVEL_IMMORTAL )
                        if( skill_table[sn]->skill_level[ch->Class] >= LEVEL_IMMORTAL )
                        {
                           ch->pcdata->learned[sn] = 0;
                           ++ch->practice;
                        }
                  }
                  fMatch = TRUE;
               }
               break;
            }
            KEY( "Wimpy", ch->wimpy, fread_number( fp ) );
            KEY( "WizInvis", ch->pcdata->wizinvis, fread_number( fp ) );
            break;
      }

      if( !fMatch )
      {
         bug( "%s: no match: %s", __func__, word );
         fread_to_eol( fp );
      }
   }
}

bool check_owner( OBJ_DATA *obj )
{ 
   HOME_DATA *home; 

   for( home = first_home; home; home = home->next )
      if( home->vnum[0] == obj->pIndexData->vnum )
         if( !str_cmp( home->name, obj->owner ) )
            return TRUE;
   return FALSE;
}

void fread_obj( CHAR_DATA * ch, FILE * fp, short os_type )
{
   OBJ_DATA *obj;
   const char *word;
   int iNest, obj_file_ver = 0;
   bool fMatch, fNest, fVnum;
   ROOM_INDEX_DATA *room = NULL;

   if( ch )
   {
      room = ch->in_room;
      if( ch->tempnum == -9999 )
         file_ver = 0;
   }
   CREATE( obj, OBJ_DATA, 1 );
   obj->count = 1;
   obj->wear_loc = -1;
   obj->weight = 1;
   obj->owner = STRALLOC( "" );

   fNest = TRUE;  // Requiring a Nest 0 is a waste
   fVnum = FALSE; // We can't assume this - what if Vnum isn't written to the file? Crashy crashy is what. - Pulled from Smaug 1.8
   iNest = 0;

   for( ;; )
   {
      word = ( feof( fp ) ? "End" : fread_word( fp ) );

      if( word[0] == '\0' )
      {
         bug( "%s: EOF encountered reading file!", __func__ );
         word = "End";
      }
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            KEY( "ActionDesc", obj->action_desc, fread_string( fp ) );
            if( !strcmp( word, "Affect" ) || !strcmp( word, "AffectData" ) )
            {
               AFFECT_DATA *paf;
               int pafmod;

               CREATE( paf, AFFECT_DATA, 1 );
               if( !strcmp( word, "Affect" ) )
               {
                  paf->type = fread_number( fp );
               }
               else
               {
                  int sn;

                  sn = skill_lookup( fread_word( fp ) );
                  if( sn < 0 )
                     bug( "%s: unknown skill.", __func__ );
                  else
                     paf->type = sn;
               }
               paf->duration = fread_number( fp );
               pafmod = fread_number( fp );
               paf->location = fread_number( fp );
               paf->bitvector = fread_bitvector( fp );
               if( paf->location == APPLY_WEAPONSPELL
                   || paf->location == APPLY_WEARSPELL
                   || paf->location == APPLY_STRIPSN
                   || paf->location == APPLY_REMOVESPELL || paf->location == APPLY_RECURRINGSPELL )
                  paf->modifier = slot_lookup( pafmod );
               else
                  paf->modifier = pafmod;
               LINK( paf, obj->first_affect, obj->last_affect, next, prev );
               fMatch = TRUE;
               break;
            }
            break;

         case 'C':
            KEY( "Cost", obj->cost, fread_number( fp ) );
            KEY( "Count", obj->count, fread_number( fp ) );
            break;

         case 'D':
            KEY( "Description", obj->description, fread_string( fp ) );
            break;

         case 'E':
            KEY( "ExtraFlags", obj->extra_flags, fread_bitvector( fp ) );

            if( !strcmp( word, "ExtraDescr" ) )
            {
               EXTRA_DESCR_DATA *ed;

               CREATE( ed, EXTRA_DESCR_DATA, 1 );
               ed->keyword = fread_string( fp );
               ed->description = fread_string( fp );
               LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
               fMatch = TRUE;
            }

            if( !strcmp( word, "End" ) )
            {
               if( obj_file_ver == 0 )
                  ; // Do nothing. This is just to keep GCC quiet.

               if( obj->item_type == ITEM_HOUSEKEY )
               {
                  if( !check_owner( obj ) )
                  {
                     bug( "%s: house key found not belonging to correct house", __func__ );
                     if( obj->name )
                        STRFREE( obj->name );
                     if( obj->description )
                        STRFREE( obj->description );
                     if( obj->short_descr )
                        STRFREE( obj->short_descr );
                     DISPOSE( obj );
                     return;
                  }
               }

               if( !fNest || !fVnum )
               {
                  if( obj->name )
                     bug( "%s: %s incomplete object.", __func__, obj->name );
                  else
                     bug( "%s: incomplete object.", __func__ );
                  free_obj( obj );
                  return;
               }
               else
               {
                  short wear_loc = obj->wear_loc;

                  if( !obj->name )
                     obj->name = QUICKLINK( obj->pIndexData->name );
                  if( !obj->description )
                     obj->description = QUICKLINK( obj->pIndexData->description );
                  if( !obj->short_descr )
                     obj->short_descr = QUICKLINK( obj->pIndexData->short_descr );
                  if( !obj->action_desc )
                     obj->action_desc = QUICKLINK( obj->pIndexData->action_desc );
                  if( IS_OBJ_STAT( obj, ITEM_PERSONAL ) && !obj->owner && ch )
                     obj->owner = QUICKLINK( ch->name );
                  if( !obj->owner )
                     obj->owner = STRALLOC( "" );
                  LINK( obj, first_object, last_object, next, prev );
                  obj->pIndexData->count += obj->count;
                  if( !obj->serial )
                  {
                     cur_obj_serial = UMAX( ( cur_obj_serial + 1 ) & ( BV30 - 1 ), 1 );
                     obj->serial = obj->pIndexData->serial = cur_obj_serial;
                  }
                  if( fNest )
                     rgObjNest[iNest] = obj;
                  numobjsloaded += obj->count;
                  ++physicalobjects;
                  if( file_ver > 1 || obj->wear_loc < -1 || obj->wear_loc >= MAX_WEAR )
                     obj->wear_loc = -1;
                  /*
                   * Corpse saving. -- Altrag 
                   */
                  if( os_type == OS_CORPSE )
                  {
                     if( !room )
                     {
                        bug( "%s: Corpse without room", __func__ );
                        room = get_room_index( ROOM_VNUM_LIMBO );
                     }

                     /*
                      * Give the corpse a timer if there isn't one 
                      */
                     if( obj->timer < 1 )
                        obj->timer = 40;
                     if( room->vnum == ROOM_VNUM_HALLOFFALLEN && obj->first_content )
                        obj->timer = -1;
                     obj = obj_to_room( obj, room );
                  }
                  else if( os_type == OS_VAULT && room )
                  {
                     obj = obj_to_room( obj, room );
                  }
                  else if( iNest == 0 || rgObjNest[iNest] == NULL )
                  {
                     int slot = -1;
                     bool reslot = FALSE;

                     if( file_ver > 1 && wear_loc > -1 && wear_loc < MAX_WEAR )
                     {
                        int x;

                        for( x = 0; x < MAX_LAYERS; x++ )
                        {
                           if( IS_NPC( ch ) )
                           {
                              if( !mob_save_equipment[wear_loc][x] )
                              {
                                 mob_save_equipment[wear_loc][x] = obj;
                                 slot = x;
                                 reslot = TRUE;
                                 break;
                              }
                           }
                           else
                           {
                              if( !save_equipment[wear_loc][x] )
                              {
                                 save_equipment[wear_loc][x] = obj;
                                 slot = x;
                                 reslot = TRUE;
                                 break;
                              }
                           }
                        }
                        if( x == MAX_LAYERS )
                           bug( "%s: too many layers %d", __func__, wear_loc );
                     }
                     obj = obj_to_char( obj, ch );
                     if( reslot && slot != -1 )
                     {
                        if( IS_NPC( ch ) )
                           mob_save_equipment[wear_loc][slot] = obj;
                        else
                           save_equipment[wear_loc][slot] = obj;
                     }
                  }
                  else
                  {
                     if( rgObjNest[iNest - 1] )
                     {
                        separate_obj( rgObjNest[iNest - 1] );
                        obj = obj_to_obj( obj, rgObjNest[iNest - 1] );
                     }
                     else
                        bug( "%s: nest layer missing %d", __func__, iNest - 1 );
                  }
                  if( fNest )
                     rgObjNest[iNest] = obj;
                  return;
               }
            }
            break;

         case 'I':
            KEY( "ItemType", obj->item_type, fread_number( fp ) );
            break;

         case 'L':
            KEY( "Level", obj->level, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", obj->name, fread_string( fp ) );

            if( !strcmp( word, "Nest" ) )
            {
               iNest = fread_number( fp );
               if( iNest < 0 || iNest >= MAX_NEST )
               {
                  bug( "%s: bad nest %d.", __func__, iNest );
                  iNest = 0;
                  fNest = FALSE;
               }
               fMatch = TRUE;
            }
            break;

         case 'O':
            if( !strcmp( word, "Owner" ) )
            {
               STRFREE( obj->owner );
               obj->owner = fread_string( fp );
               xSET_BIT( obj->extra_flags, ITEM_PERSONAL );
               fMatch = TRUE;
            }
            break;

         case 'R':
            KEY( "Room", room, get_room_index( fread_number( fp ) ) );
            KEY( "Rvnum", obj->room_vnum, fread_number( fp ) );
            break;

         case 'S':
            KEY( "ShortDescr", obj->short_descr, fread_string( fp ) );

            if( !strcmp( word, "Spell" ) )
            {
               int iValue;
               int sn;

               iValue = fread_number( fp );
               sn = skill_lookup( fread_word( fp ) );
               if( iValue < 0 || iValue > 5 )
                  bug( "%s: bad iValue %d.", __func__, iValue );
               else if( sn < 0 )
                  bug( "%s: unknown skill.", __func__ );
               else
                  obj->value[iValue] = sn;
               fMatch = TRUE;
               break;
            }

            break;

         case 'T':
            KEY( "Timer", obj->timer, fread_number( fp ) );
            break;

         case 'V':
            if( !strcmp( word, "Values" ) )
            {
               int x1, x2, x3, x4, x5, x6;
               char *ln = fread_line( fp );

               x1 = x2 = x3 = x4 = x5 = x6 = 0;
               sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );

               /* clean up some garbage */
               if( file_ver < 3 && os_type != OS_VAULT )
                  x5 = x6 = 0;

               obj->value[0] = x1;
               obj->value[1] = x2;
               obj->value[2] = x3;
               obj->value[3] = x4;
               obj->value[4] = x5;
               obj->value[5] = x6;
               fMatch = TRUE;
               break;
            }

            if( !strcmp( word, "Vnum" ) )
            {
               int vnum;

               vnum = fread_number( fp );

               if( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
                  fVnum = FALSE;
               else
               {
                  fVnum = TRUE;
                  obj->cost = obj->pIndexData->cost;
                  obj->weight = obj->pIndexData->weight;
                  obj->item_type = obj->pIndexData->item_type;
                  obj->wear_flags = obj->pIndexData->wear_flags;
                  obj->extra_flags = obj->pIndexData->extra_flags;
               }
               fMatch = TRUE;
               break;
            }

            KEY( "Version", obj_file_ver, fread_number( fp ) );
            break;

         case 'W':
            KEY( "WearFlags", obj->wear_flags, fread_number( fp ) );
            KEY( "WearLoc", obj->wear_loc, fread_number( fp ) );
            KEY( "Weight", obj->weight, fread_number( fp ) );
            break;
      }

      if( !fMatch )
      {
         EXTRA_DESCR_DATA *ed;
         AFFECT_DATA *paf;

         bug( "%s: no match: %s", __func__, word );
         fread_to_eol( fp );
         if( obj->name )
            STRFREE( obj->name );
         if( obj->description )
            STRFREE( obj->description );
         if( obj->short_descr )
            STRFREE( obj->short_descr );
         while( ( ed = obj->first_extradesc ) != NULL )
         {
            STRFREE( ed->keyword );
            STRFREE( ed->description );
            UNLINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
            DISPOSE( ed );
         }
         while( ( paf = obj->first_affect ) != NULL )
         {
            UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
            DISPOSE( paf );
         }
         DISPOSE( obj );
         return;
      }
   }
}

void set_alarm( long seconds )
{
#ifdef WIN32
   kill_timer(  );   /* kill old timer */
   timer_code = timeSetEvent( seconds * 1000L, 1000, alarm_handler, 0, TIME_PERIODIC );
#else
   alarm( seconds );
#endif
}

/*
 * Based on last time modified, show when a player was last on	-Thoric
 */
void do_last( CHAR_DATA* ch, const char* argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char name[MAX_INPUT_LENGTH];
   struct stat fst;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Usage: last <playername>\r\n", ch );
      return;
   }
   mudstrlcpy( name, capitalize( arg ), MAX_INPUT_LENGTH );
   snprintf( buf, MAX_STRING_LENGTH, "%s%c/%s", PLAYER_DIR, tolower( arg[0] ), name );
   if( stat( buf, &fst ) != -1 && check_parse_name( capitalize( name ), FALSE ) )
      ch_printf( ch, "%s was last on: %s\r", name, ctime( &fst.st_mtime ) );
   else
      ch_printf( ch, "%s was not found.\r\n", name );
}

/*
 * Added support for removeing so we could take out the write_corpses
 * so we could take it out of the save_char_obj function. --Shaddai
 */
void write_corpses( CHAR_DATA * ch, const char *name, OBJ_DATA * objrem )
{
   OBJ_DATA *corpse;
   FILE *fp = NULL;

   /*
    * Name and ch support so that we dont have to have a char to save their
    * corpses.. (ie: decayed corpses while offline) 
    */
   if( ch && IS_NPC( ch ) )
   {
      bug( "%s: writing NPC corpse.", __func__ );
      return;
   }
   if( ch )
      name = ch->name;
   /*
    * Go by vnum, less chance of screwups. -- Altrag 
    */
   for( corpse = first_object; corpse; corpse = corpse->next )
      if( corpse->pIndexData->vnum == OBJ_VNUM_CORPSE_PC &&
          corpse->in_room != NULL && !str_cmp( corpse->short_descr + 14, name ) && objrem != corpse )
      {
         if( !fp )
         {
            char buf[127];

            snprintf( buf, 127, "%s%s", CORPSE_DIR, capitalize( name ) );
            if( !( fp = fopen( buf, "w" ) ) )
            {
               bug( "%s: Cannot open file %s.", __func__, buf );
               perror( buf );
               return;
            }
         }
         fwrite_obj( ch, corpse, fp, 0, OS_CORPSE, FALSE );
      }
   if( fp )
   {
      fprintf( fp, "#END\n\n" );
      FCLOSE( fp );
   }
   else
   {
      char buf[127];

      snprintf( buf, 127, "%s%s", CORPSE_DIR, capitalize( name ) );
      remove( buf );
   }
}

void load_corpses( void )
{
   DIR *dp;
   struct dirent *de;
   extern int falling;
   char corpsefile[MAX_STRING_LENGTH];
   FILE *cfp;

   if( !( dp = opendir( CORPSE_DIR ) ) )
   {
      bug( "%s: can't open %s", __func__, CORPSE_DIR );
      perror( CORPSE_DIR );
      return;
   }

   falling = 1;   /* Arbitrary, must be >0 though. */
   while( ( de = readdir( dp ) ) != NULL )
   {
      if( de->d_name[0] != '.' )
      {
         snprintf( corpsefile, MAX_STRING_LENGTH, "%s%s", CORPSE_DIR, de->d_name );
         fprintf( stderr, "Corpse -> %s\n", corpsefile );
         if( !( cfp = fopen( corpsefile, "r" ) ) )
         {
            perror( corpsefile );
            continue;
         }
         for( ;; )
         {
            char letter;
            char *word;

            letter = fread_letter( cfp );
            if( letter == '*' )
            {
               fread_to_eol( cfp );
               continue;
            }
            if( letter != '#' )
            {
               bug( "%s: # not found.", __func__ );
               break;
            }
            word = fread_word( cfp );
            if( !strcmp( word, "CORPSE" ) )
               fread_obj( NULL, cfp, OS_CORPSE );
            else if( !strcmp( word, "OBJECT" ) )
               fread_obj( NULL, cfp, OS_CARRY );
            else if( !strcmp( word, "END" ) )
               break;
            else
            {
               bug( "%s: bad section.", __func__ );
               break;
            }
         }
         FCLOSE( cfp );
      }
   }
   cfp = NULL;
   closedir( dp );
   falling = 0;
}

/*
 * This will write one mobile structure pointed to by fp --Shaddai
 */
void fwrite_mobile( FILE * fp, CHAR_DATA * mob )
{
   if( !IS_NPC( mob ) || !fp )
      return;
   de_equip_char( mob );
   fprintf( fp, "#MOBILE\n" );
   fprintf( fp, "Version %d\n", SAVEVERSION );
   fprintf( fp, "Vnum	%d\n", mob->pIndexData->vnum );
   if( mob->in_room )
      fprintf( fp, "Room	%d\n",
               ( mob->in_room == get_room_index( ROOM_VNUM_LIMBO )
                 && mob->was_in_room ) ? mob->was_in_room->vnum : mob->in_room->vnum );
   if( mob->name && mob->pIndexData->player_name && str_cmp( mob->name, mob->pIndexData->player_name ) )
      fprintf( fp, "Name     %s~\n", mob->name );
   if( mob->short_descr && mob->pIndexData->short_descr && str_cmp( mob->short_descr, mob->pIndexData->short_descr ) )
      fprintf( fp, "Short	%s~\n", mob->short_descr );
   if( mob->long_descr && mob->pIndexData->long_descr && str_cmp( mob->long_descr, mob->pIndexData->long_descr ) )
      fprintf( fp, "Long	%s~\n", mob->long_descr );
   if( mob->description && mob->pIndexData->description && str_cmp( mob->description, mob->pIndexData->description ) )
      fprintf( fp, "Description %s~\n", mob->description );
   fprintf( fp, "Position %d\n", mob->position );
   fprintf( fp, "Flags %s\n", print_bitvector( &mob->act ) );
   if( mob->first_carrying )
      fwrite_obj( mob, mob->last_carrying, fp, 0, OS_CARRY, FALSE );
   fprintf( fp, "EndMobile\n" );
   re_equip_char( mob );
}

/*
 * This will read one mobile structure pointer to by fp --Shaddai
 */
CHAR_DATA *fread_mobile( FILE * fp )
{
   CHAR_DATA *mob = NULL;
   const char *word;
   bool fMatch;
   int inroom = 0, mob_file_ver = 0;
   ROOM_INDEX_DATA *pRoomIndex = NULL;

   word = ( feof( fp ) ? "EndMobile" : fread_word( fp ) );

   if( word[0] == '\0' )
   {
      bug( "%s: EOF encountered reading file!", __func__ );
      word = "EndMobile";
   }

   if( !str_cmp( word, "Vnum" ) )
   {
      int vnum;

      vnum = fread_number( fp );
      mob = create_mobile( get_mob_index( vnum ) );
      if( !mob )
      {
         for( ;; )
         {
            word = ( feof( fp ) ? "EndMobile" : fread_word( fp ) );

            if( word[0] == '\0' )
            {
               bug( "%s: EOF encountered reading file!", __func__ );
               word = "EndMobile";
            }

            /*
             * So we don't get so many bug messages when something messes up
             * * --Shaddai 
             */
            if( !str_cmp( word, "EndMobile" ) )
               break;
         }
         bug( "%s: No index data for vnum %d", __func__, vnum );
         return NULL;
      }
   }
   else
   {
      for( ;; )
      {
         word = ( feof( fp ) ? "EndMobile" : fread_word( fp ) );

         if( word[0] == '\0' )
         {
            bug( "%s: EOF encountered reading file!", __func__ );
            word = "EndMobile";
         }

         /*
          * So we don't get so many bug messages when something messes up
          * * --Shaddai 
          */
         if( !str_cmp( word, "EndMobile" ) )
            break;
      }
      bug( "%s: Vnum not found", __func__ );
      return NULL;
   }

   for( ;; )
   {
      word = ( feof( fp ) ? "EndMobile" : fread_word( fp ) );

      if( word[0] == '\0' )
      {
         bug( "%s: EOF encountered reading file!", __func__ );
         word = "EndMobile";
      }

      fMatch = FALSE;
      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case '#':
            if( !str_cmp( word, "#OBJECT" ) )
            {
               fread_obj( mob, fp, OS_CARRY );
               fMatch = TRUE;
               break;
            }
            break;

         case 'D':
            if( !str_cmp( word, "Description" ) )
            {
               if( mob->description )
                  STRFREE( mob->description );
               mob->description = fread_string( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'E':
            if( !str_cmp( word, "EndMobile" ) )
            {
               if( mob_file_ver == 0 )
                  ; // Do nothing. This is just to keep GCC quiet.

               if( inroom == 0 )
                  inroom = ROOM_VNUM_TEMPLE;
               pRoomIndex = get_room_index( inroom );
               if( !pRoomIndex )
                  pRoomIndex = get_room_index( ROOM_VNUM_TEMPLE );
               char_to_room( mob, pRoomIndex );
               re_equip_char( mob );
               return mob;
            }
            break;

         case 'F':
            KEY( "Flags", mob->act, fread_bitvector( fp ) );
            break;

         case 'L':
            if( !str_cmp( word, "Long" ) )
            {
               if( mob->long_descr )
                  STRFREE( mob->long_descr );
               mob->long_descr = fread_string( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'N':
            if( !str_cmp( word, "Name" ) )
            {
               if( mob->name )
                  STRFREE( mob->name );
               mob->name = fread_string( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'P':
            KEY( "Position", mob->position, fread_number( fp ) );
            break;

         case 'R':
            KEY( "Room", inroom, fread_number( fp ) );
            break;

         case 'S':
            if( !str_cmp( word, "Short" ) )
            {
               if( mob->short_descr )
                  STRFREE( mob->short_descr );
               mob->short_descr = fread_string( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'V':
            KEY( "Version", mob_file_ver, fread_number( fp ) );
            break;
      }

      if( !fMatch )
      {
         bug( "%s: no match: %s", __func__, word );
         fread_to_eol( fp );
      }
   }
}

/*
 * This will write in the saved mobile for a char --Shaddai
 */
void write_char_mobile( CHAR_DATA * ch, char *argument )
{
   FILE *fp;
   CHAR_DATA *mob;

   if( IS_NPC( ch ) || !ch->pcdata->pet )
      return;

   if( ( fp = fopen( argument, "w" ) ) == NULL )
   {
      bug( "%s: couldn't open %s for writing!\r\n", __func__, argument );
      return;
   }
   mob = ch->pcdata->pet;
   xSET_BIT( mob->affected_by, AFF_CHARM );
   fwrite_mobile( fp, mob );
   FCLOSE( fp );
}

/*
 * This will read in the saved mobile for a char --Shaddai
 */

void read_char_mobile( char *argument )
{
   FILE *fp;
   CHAR_DATA *mob;

   if( ( fp = fopen( argument, "r" ) ) == NULL )
   {
      bug( "%s: couldn't open %s for reading!\r\n", __func__, argument );
      return;
   }
   mob = fread_mobile( fp );
   if( !mob )
      bug( "%s: failed to fread_mobile.", __func__ );
   FCLOSE( fp );
}
