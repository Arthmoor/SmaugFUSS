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
 *                            Table load/save Module                        *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#if !defined(WIN32)
#include <dlfcn.h>
#else
#include <windows.h>
#define dlsym( handle, name ) ( (void*)GetProcAddress( (HINSTANCE) (handle), (name) ) )
#define dlerror() GetLastError()
#endif
#include "mud.h"

bool load_race_file( const char *fname );

/* global variables */
int top_herb;
int MAX_PC_CLASS;
int MAX_PC_RACE;

SKILLTYPE *skill_table[MAX_SKILL];
const SKILLTYPE *skill_table_bytype[MAX_SKILL];
struct class_type *class_table[MAX_CLASS];
RACE_TYPE *race_table[MAX_RACE];
const char *title_table[MAX_CLASS][MAX_LEVEL + 1][2];
SKILLTYPE *herb_table[MAX_HERB];
SKILLTYPE *disease_table[MAX_DISEASE];

LANG_DATA *first_lang;
LANG_DATA *last_lang;

const char *skill_tname[] = { "unknown", "Spell", "Skill", "Weapon", "Tongue", "Herb", "Racial", "Disease" };

SPELL_FUN *spell_function( const char *name )
{
   void *funHandle;
#if !defined(WIN32)
   const char *error;
#else
   DWORD error;
#endif

   funHandle = dlsym( sysdata.dlHandle, name );
   if( ( error = dlerror(  ) ) )
   {
      bug( "%s: Error locating %s in symbol table. %s", __func__, name, error );
      return spell_notfound;
   }
   return ( SPELL_FUN * ) funHandle;
}

DO_FUN *skill_function( const char *name )
{
   void *funHandle;
#if !defined(WIN32)
   const char *error;
#else
   DWORD error;
#endif

   funHandle = dlsym( sysdata.dlHandle, name );
   if( ( error = dlerror(  ) ) )
   {
      bug( "%s: Error locating %s in symbol table. %s", __func__, name, error );
      return skill_notfound;
   }
   return ( DO_FUN * ) funHandle;
}

bool load_class_file( const char *fname )
{
   char buf[MAX_STRING_LENGTH];
   const char *word;
   bool fMatch;
   struct class_type *Class;
   int cl = -1;
   int tlev = 0;
   FILE *fp;

   snprintf( buf, MAX_STRING_LENGTH, "%s%s", CLASS_DIR, fname );
   if( ( fp = fopen( buf, "r" ) ) == NULL )
   {
      perror( buf );
      return FALSE;
   }

   CREATE( Class, struct class_type, 1 );

   /*
    * Setup defaults for additions to class structure 
    */
   Class->attr_second = 0;
   Class->attr_deficient = 0;
   xCLEAR_BITS( Class->affected );
   Class->resist = 0;
   Class->suscept = 0;

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            KEY( "Affected", Class->affected, fread_bitvector( fp ) );
            KEY( "AttrPrime", Class->attr_prime, fread_number( fp ) );
            KEY( "AttrSecond", Class->attr_second, fread_number( fp ) );
            KEY( "AttrDeficient", Class->attr_deficient, fread_number( fp ) );
            break;

         case 'C':
            KEY( "Class", cl, fread_number( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               FCLOSE( fp );
               if( cl < 0 || cl >= MAX_CLASS )
               {
                  bug( "%s: Class (%s) bad/not found (%d)", __func__,
                       Class->who_name ? Class->who_name : "name not found", cl );
                  if( Class->who_name )
                     STRFREE( Class->who_name );
                  DISPOSE( Class );
                  return FALSE;
               }
               class_table[cl] = Class;
               return TRUE;
            }
            KEY( "ExpBase", Class->exp_base, fread_number( fp ) );
            break;

         case 'G':
            KEY( "Guild", Class->guild, fread_number( fp ) );
            break;

         case 'H':
            KEY( "HpMax", Class->hp_max, fread_number( fp ) );
            KEY( "HpMin", Class->hp_min, fread_number( fp ) );
            break;

         case 'M':
            KEY( "Mana", Class->fMana, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", Class->who_name, fread_string( fp ) );
            break;

         case 'R':
            KEY( "Resist", Class->resist, fread_number( fp ) );
            break;

         case 'S':
            if( !str_cmp( word, "Skill" ) )
            {
               int sn, lev, adp;

               word = fread_word( fp );
               lev = fread_number( fp );
               adp = fread_number( fp );
               sn = skill_lookup( word );
               if( cl < 0 || cl >= MAX_CLASS )
               {
                  bug( "%s: Skill %s -- class bad/not found (%d)", __func__, word, cl );
               }
               else if( !IS_VALID_SN( sn ) )
               {
                  bug( "%s: Skill %s unknown", __func__, word );
               }
               else
               {
                  skill_table[sn]->skill_level[cl] = lev;
                  skill_table[sn]->skill_adept[cl] = adp;
               }
               fMatch = TRUE;
               break;
            }
            KEY( "Skilladept", Class->skill_adept, fread_number( fp ) );
            KEY( "Suscept", Class->suscept, fread_number( fp ) );
            break;

         case 'T':
            if( !str_cmp( word, "Title" ) )
            {
               if( cl < 0 || cl >= MAX_CLASS )
               {
                  char *tmp;

                  bug( "%s: Title -- class bad/not found (%d)", __func__, cl );
                  tmp = fread_string_nohash( fp );
                  DISPOSE( tmp );
                  tmp = fread_string_nohash( fp );
                  DISPOSE( tmp );
               }
               else if( tlev < MAX_LEVEL + 1 )
               {
                  title_table[cl][tlev][0] = fread_string_nohash( fp );
                  title_table[cl][tlev][1] = fread_string_nohash( fp );
                  ++tlev;
               }
               else
                  bug( "%s: Too many titles", __func__ );
               fMatch = TRUE;
               break;
            }
            KEY( "Thac0", Class->thac0_00, fread_number( fp ) );
            KEY( "Thac32", Class->thac0_32, fread_number( fp ) );
            break;

         case 'W':
            KEY( "Weapon", Class->weapon, fread_number( fp ) );
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
 * Load in all the class files.
 */
void load_classes(  )
{
   FILE *fpList;
   const char *filename;
   char classlist[256];
   int i;

   MAX_PC_CLASS = 0;

   /*
    * Pre-init the class_table with blank classes
    */
   for( i = 0; i < MAX_CLASS; ++i )
      class_table[i] = NULL;

   snprintf( classlist, 256, "%s%s", CLASS_DIR, CLASS_LIST );
   if( ( fpList = fopen( classlist, "r" ) ) == NULL )
   {
      perror( classlist );
      exit( 1 );
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      if( filename[0] == '$' )
         break;

      if( !load_class_file( filename ) )
      {
         bug( "%s: Cannot load class file: %s", __func__, filename );
      }
      else
         ++MAX_PC_CLASS;
   }
   FCLOSE( fpList );
   for( i = 0; i < MAX_CLASS; ++i )
   {
      if( class_table[i] == NULL )
      {
         CREATE( class_table[i], struct class_type, 1 );
         create_new_class( i, "" );
      }
   }
   return;
}

void write_class_file( int cl )
{
   FILE *fpout;
   char filename[MAX_INPUT_LENGTH];
   struct class_type *Class = class_table[cl];
   int x, y;

   snprintf( filename, MAX_INPUT_LENGTH, "%s%s.class", CLASS_DIR, Class->who_name );
   if( ( fpout = fopen( filename, "w" ) ) == NULL )
   {
      bug( "%s: Cannot open: %s for writing", __func__, filename );
      return;
   }
   fprintf( fpout, "Name        %s~\n", Class->who_name );
   fprintf( fpout, "Class       %d\n", cl );
   fprintf( fpout, "AttrPrime   %d\n", Class->attr_prime );
   fprintf( fpout, "AttrSecond   %d\n", Class->attr_second );
   fprintf( fpout, "AttrDeficient   %d\n", Class->attr_deficient );
   fprintf( fpout, "Weapon      %d\n", Class->weapon );
   fprintf( fpout, "Guild       %d\n", Class->guild );
   fprintf( fpout, "Skilladept  %d\n", Class->skill_adept );
   fprintf( fpout, "Thac0       %d\n", Class->thac0_00 );
   fprintf( fpout, "Thac32      %d\n", Class->thac0_32 );
   fprintf( fpout, "Hpmin       %d\n", Class->hp_min );
   fprintf( fpout, "Hpmax       %d\n", Class->hp_max );
   fprintf( fpout, "Mana        %d\n", Class->fMana );
   fprintf( fpout, "Expbase     %d\n", Class->exp_base );
   fprintf( fpout, "Affected    %s\n", print_bitvector( &Class->affected ) );
   fprintf( fpout, "Resist	 %d\n", Class->resist );
   fprintf( fpout, "Suscept	 %d\n", Class->suscept );
   for( x = 0; x < num_skills; ++x )
   {
      if( !skill_table[x]->name || skill_table[x]->name[0] == '\0' )
         break;
      if( ( y = skill_table[x]->skill_level[cl] ) < LEVEL_IMMORTAL )
         fprintf( fpout, "Skill '%s' %d %d\n", skill_table[x]->name, y, skill_table[x]->skill_adept[cl] );
   }
   for( x = 0; x <= MAX_LEVEL; ++x )
      fprintf( fpout, "Title\n%s~\n%s~\n", title_table[cl][x][0], title_table[cl][x][1] );
   fprintf( fpout, "End\n" );
   FCLOSE( fpout );
}

/*
 * Load in all the race files.
 */
void load_races(  )
{
   FILE *fpList;
   const char *filename;
   char racelist[256];
   int i;

   MAX_PC_RACE = 0;
   /*
    * Pre-init the race_table with blank races
    */
   for( i = 0; i < MAX_RACE; ++i )
      race_table[i] = NULL;

   snprintf( racelist, 256, "%s%s", RACE_DIR, RACE_LIST );
   if( ( fpList = fopen( racelist, "r" ) ) == NULL )
   {
      perror( racelist );
      exit( 1 );
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      if( filename[0] == '$' )
         break;

      if( !load_race_file( filename ) )
         bug( "%s: Cannot load race file: %s", __func__, filename );
      else
         ++MAX_PC_RACE;
   }
   for( i = 0; i < MAX_RACE; ++i )
   {
      if( race_table[i] == NULL )
      {
         CREATE( race_table[i], struct race_type, 1 );
         race_table[i]->race_name = STRALLOC( "unused" );
      }
   }
   FCLOSE( fpList );
   return;
}

void write_race_file( int ra )
{
   FILE *fpout;
   char filename[MAX_INPUT_LENGTH];
   struct race_type *race = race_table[ra];
   int i;
   int x, y;

   if( !race->race_name )
   {
      bug( "%s: Race %d has null name, not writing .race file.", __func__, ra );
      return;
   }

   snprintf( filename, MAX_INPUT_LENGTH, "%s%s.race", RACE_DIR, race->race_name );
   if( ( fpout = fopen( filename, "w+" ) ) == NULL )
   {
      bug( "%s: Cannot open: %s for writing", __func__, filename );
      return;
   }

   fprintf( fpout, "Name        %s~\n", race->race_name );
   fprintf( fpout, "Race        %d\n", ra );
   fprintf( fpout, "Classes     %d\n", race->class_restriction );
   fprintf( fpout, "Str_Plus    %d\n", race->str_plus );
   fprintf( fpout, "Dex_Plus    %d\n", race->dex_plus );
   fprintf( fpout, "Wis_Plus    %d\n", race->wis_plus );
   fprintf( fpout, "Int_Plus    %d\n", race->int_plus );
   fprintf( fpout, "Con_Plus    %d\n", race->con_plus );
   fprintf( fpout, "Cha_Plus    %d\n", race->cha_plus );
   fprintf( fpout, "Lck_Plus    %d\n", race->lck_plus );
   fprintf( fpout, "Hit         %d\n", race->hit );
   fprintf( fpout, "Mana        %d\n", race->mana );
   fprintf( fpout, "Affected    %s\n", print_bitvector( &race->affected ) );
   fprintf( fpout, "Resist      %d\n", race->resist );
   fprintf( fpout, "Suscept     %d\n", race->suscept );
   fprintf( fpout, "Language    %d\n", race->language );
   fprintf( fpout, "Align       %d\n", race->alignment );
   fprintf( fpout, "Min_Align  %d\n", race->minalign );
   fprintf( fpout, "Max_Align	%d\n", race->maxalign );
   fprintf( fpout, "AC_Plus    %d\n", race->ac_plus );
   fprintf( fpout, "Exp_Mult   %d\n", race->exp_multiplier );
   fprintf( fpout, "Attacks    %s\n", print_bitvector( &race->attacks ) );
   fprintf( fpout, "Defenses   %s\n", print_bitvector( &race->defenses ) );
   fprintf( fpout, "Height     %d\n", race->height );
   fprintf( fpout, "Weight     %d\n", race->weight );
   fprintf( fpout, "Hunger_Mod  %d\n", race->hunger_mod );
   fprintf( fpout, "Thirst_mod  %d\n", race->thirst_mod );
   fprintf( fpout, "Mana_Regen  %d\n", race->mana_regen );
   fprintf( fpout, "HP_Regen    %d\n", race->hp_regen );
   fprintf( fpout, "Race_Recall %d\n", race->race_recall );
   for( i = 0; i < MAX_WHERE_NAME; ++i )
      fprintf( fpout, "WhereName  %s~\n", race->where_name[i] );

   for( x = 0; x < num_skills; ++x )
   {
      if( !skill_table[x]->name || skill_table[x]->name[0] == '\0' )
         break;
      if( ( y = skill_table[x]->race_level[ra] ) < LEVEL_IMMORTAL )
         fprintf( fpout, "Skill '%s' %d %d\n", skill_table[x]->name, y, skill_table[x]->race_adept[ra] );
   }
   fprintf( fpout, "End\n" );
   FCLOSE( fpout );
}

bool load_race_file( const char *fname )
{
   char buf[MAX_STRING_LENGTH];
   const char *word;
   bool fMatch;
   struct race_type *race;
   int ra = -1;
   FILE *fp;
   int i, wear = 0;

   snprintf( buf, MAX_STRING_LENGTH, "%s%s", RACE_DIR, fname );
   if( !( fp = fopen( buf, "r" ) ) )
   {
      perror( buf );
      return FALSE;
   }

   CREATE( race, struct race_type, 1 );
   for( i = 0; i < MAX_WHERE_NAME; ++i )
      race->where_name[i] = str_dup( where_name[i] );

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            KEY( "Align", race->alignment, fread_number( fp ) );
            KEY( "AC_Plus", race->ac_plus, fread_number( fp ) );
            KEY( "Affected", race->affected, fread_bitvector( fp ) );
            KEY( "Attacks", race->attacks, fread_bitvector( fp ) );
            break;

         case 'C':
            KEY( "Con_Plus", race->con_plus, fread_number( fp ) );
            KEY( "Cha_Plus", race->cha_plus, fread_number( fp ) );
            KEY( "Classes", race->class_restriction, fread_number( fp ) );
            break;

         case 'D':
            KEY( "Dex_Plus", race->dex_plus, fread_number( fp ) );
            KEY( "Defenses", race->defenses, fread_bitvector( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               FCLOSE( fp );
               if( ra < 0 || ra >= MAX_RACE )
               {
                  bug( "%s: Race (%s) bad/not found (%d)", __func__,
                       race->race_name ? race->race_name : "name not found", ra );
                  for( i = 0; i < MAX_WHERE_NAME; ++i )
                     DISPOSE( race->where_name[i] );
                  DISPOSE( race );
                  return FALSE;
               }
               race_table[ra] = race;
               return TRUE;
            }

            KEY( "Exp_Mult", race->exp_multiplier, fread_number( fp ) );
            break;

         case 'I':
            KEY( "Int_Plus", race->int_plus, fread_number( fp ) );
            break;

         case 'H':
            KEY( "Height", race->height, fread_number( fp ) );
            KEY( "Hit", race->hit, fread_number( fp ) );
            KEY( "HP_Regen", race->hp_regen, fread_number( fp ) );
            KEY( "Hunger_Mod", race->hunger_mod, fread_number( fp ) );
            break;

         case 'L':
            KEY( "Language", race->language, fread_number( fp ) );
            KEY( "Lck_Plus", race->lck_plus, fread_number( fp ) );
            break;

         case 'M':
            KEY( "Mana", race->mana, fread_number( fp ) );
            KEY( "Mana_Regen", race->mana_regen, fread_number( fp ) );
            KEY( "Min_Align", race->minalign, fread_number( fp ) );
            KEY( "Max_Align", race->maxalign, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", race->race_name, fread_string( fp ) );
            break;

         case 'R':
            KEY( "Race", ra, fread_number( fp ) );
            KEY( "Race_Recall", race->race_recall, fread_number( fp ) );
            KEY( "Resist", race->resist, fread_number( fp ) );
            break;

         case 'S':
            KEY( "Str_Plus", race->str_plus, fread_number( fp ) );
            KEY( "Suscept", race->suscept, fread_number( fp ) );
            if( !str_cmp( word, "Skill" ) )
            {
               int sn, lev, adp;

               word = fread_word( fp );
               lev = fread_number( fp );
               adp = fread_number( fp );
               sn = skill_lookup( word );
               if( ra < 0 || ra >= MAX_RACE )
               {
                  bug( "%s: Skill %s -- race bad/not found (%d)", __func__, word, ra );
               }
               else if( !IS_VALID_SN( sn ) )
               {
                  bug( "%s: Skill %s unknown", __func__, word );
               }
               else
               {
                  skill_table[sn]->race_level[ra] = lev;
                  skill_table[sn]->race_adept[ra] = adp;
               }
               fMatch = TRUE;
               break;
            }
            break;

         case 'T':
            KEY( "Thirst_Mod", race->thirst_mod, fread_number( fp ) );
            break;

         case 'W':
            KEY( "Weight", race->weight, fread_number( fp ) );
            KEY( "Wis_Plus", race->wis_plus, fread_number( fp ) );
            if( !str_cmp( word, "WhereName" ) )
            {
               if( ra < 0 || ra >= MAX_RACE )
               {
                  char *tmp;

                  bug( "%s: Title -- race bad/not found (%d)", __func__, ra );
                  tmp = fread_string_nohash( fp );
                  DISPOSE( tmp );
                  tmp = fread_string_nohash( fp );
                  DISPOSE( tmp );
               }
               else if( wear < MAX_WHERE_NAME )
               {
                  DISPOSE( race->where_name[wear] );
                  race->where_name[wear] = fread_string_nohash( fp );
                  ++wear;
               }
               else
                  bug( "%s: Too many where_names", __func__ );
               fMatch = TRUE;
               break;
            }
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
 * Function used by qsort to sort skills; sorts by name, not case sensitive.
 */
int skill_comp( SKILLTYPE ** sk1, SKILLTYPE ** sk2 )
{
   SKILLTYPE *skill1 = ( *sk1 );
   SKILLTYPE *skill2 = ( *sk2 );

   if( !skill1 && skill2 )
      return 1;
   if( skill1 && !skill2 )
      return -1;
   if( !skill1 && !skill2 )
      return 0;
   // Sort without regard to case.
   return strcasecmp( skill1->name, skill2->name );
}

/*
 * Function used by qsort to sort skills; sorts by type,
 * then by name, not case sensitive.
 */
int skill_comp_bytype( SKILLTYPE ** sk1, SKILLTYPE ** sk2 )
{
   SKILLTYPE *skill1 = ( *sk1 );
   SKILLTYPE *skill2 = ( *sk2 );

   if( !skill1 && skill2 )
      return 1;
   if( skill1 && !skill2 )
      return -1;
   if( !skill1 && !skill2 )
      return 0;

   // sort by type, first
   if( skill1->type < skill2->type )
      return -1;
   if( skill1->type > skill2->type )
      return 1;

   // Sort without regard to case.
   return strcasecmp( skill1->name, skill2->name );
}

/*
 * Sort the skill table with qsort
 */
void sort_skill_table(  )
{
   int i;

   log_string( "Sorting skill table..." );
   qsort( &skill_table[1], num_skills - 1, sizeof( SKILLTYPE * ), ( int ( * )( const void *, const void * ) )skill_comp );

   log_string( "Creating skill table sorted by type..." );
   // first, refresh the bytype table from the skill table
   for( i = 0; i < MAX_SKILL; ++i )
   {
      skill_table_bytype[i] = skill_table[i];
   }
   qsort( &skill_table_bytype[1], num_skills - 1, sizeof( SKILLTYPE * ),
          ( int ( * )( const void *, const void * ) )skill_comp_bytype );
}

/*
 * Remap slot numbers to sn values
 */
void remap_slot_numbers(  )
{
   SKILLTYPE *skill;
   SMAUG_AFF *aff;
   char tmp[32];
   int sn;

   log_string( "Remapping slots to sns" );

   for( sn = 0; sn < num_skills; ++sn )
   {
      if( ( skill = skill_table[sn] ) != NULL )
      {
         for( aff = skill->first_affect; aff; aff = aff->next )
            if( aff->location == APPLY_WEAPONSPELL
                || aff->location == APPLY_WEARSPELL
                || aff->location == APPLY_REMOVESPELL
                || aff->location == APPLY_STRIPSN || aff->location == APPLY_RECURRINGSPELL )
            {
               snprintf( tmp, 32, "%d", slot_lookup( atoi( aff->modifier ) ) );
               DISPOSE( aff->modifier );
               aff->modifier = str_dup( tmp );
            }
      }
   }
}

/*
 * Write skill data to a file
 */
void fwrite_skill( FILE * fpout, SKILLTYPE * skill )
{
   SMAUG_AFF *aff;
   int modifier;

   fprintf( fpout, "Name         %s~\n", skill->name );
   fprintf( fpout, "Type         %s\n", skill_tname[skill->type] );
   fprintf( fpout, "Info         %d\n", skill->info );
   fprintf( fpout, "Flags        %d\n", skill->flags );
   if( skill->target )
      fprintf( fpout, "Target       %d\n", skill->target );
   /*
    * store as new minpos (minpos>=100 flags new style in character loading)
    */
   if( skill->minimum_position )
      fprintf( fpout, "Minpos       %d\n", skill->minimum_position + 100 );
   if( skill->spell_sector )
      fprintf( fpout, "Ssector      %d\n", skill->spell_sector );
   if( skill->saves )
      fprintf( fpout, "Saves        %d\n", skill->saves );
   if( skill->slot )
      fprintf( fpout, "Slot         %d\n", skill->slot );
   if( skill->min_mana )
      fprintf( fpout, "Mana         %d\n", skill->min_mana );
   if( skill->beats )
      fprintf( fpout, "Rounds       %d\n", skill->beats );
   if( skill->range )
      fprintf( fpout, "Range        %d\n", skill->range );
   if( skill->guild != -1 )
      fprintf( fpout, "Guild        %d\n", skill->guild );
   if( skill->skill_fun )
      fprintf( fpout, "Code         %s\n", skill->skill_fun_name );
   else if( skill->spell_fun )
      fprintf( fpout, "Code         %s\n", skill->spell_fun_name );
   fprintf( fpout, "Dammsg       %s~\n", skill->noun_damage );
   if( skill->msg_off && skill->msg_off[0] != '\0' )
      fprintf( fpout, "Wearoff      %s~\n", skill->msg_off );

   if( skill->hit_char && skill->hit_char[0] != '\0' )
      fprintf( fpout, "Hitchar      %s~\n", skill->hit_char );
   if( skill->hit_vict && skill->hit_vict[0] != '\0' )
      fprintf( fpout, "Hitvict      %s~\n", skill->hit_vict );
   if( skill->hit_room && skill->hit_room[0] != '\0' )
      fprintf( fpout, "Hitroom      %s~\n", skill->hit_room );
   if( skill->hit_dest && skill->hit_dest[0] != '\0' )
      fprintf( fpout, "Hitdest      %s~\n", skill->hit_dest );

   if( skill->miss_char && skill->miss_char[0] != '\0' )
      fprintf( fpout, "Misschar     %s~\n", skill->miss_char );
   if( skill->miss_vict && skill->miss_vict[0] != '\0' )
      fprintf( fpout, "Missvict     %s~\n", skill->miss_vict );
   if( skill->miss_room && skill->miss_room[0] != '\0' )
      fprintf( fpout, "Missroom     %s~\n", skill->miss_room );

   if( skill->die_char && skill->die_char[0] != '\0' )
      fprintf( fpout, "Diechar      %s~\n", skill->die_char );
   if( skill->die_vict && skill->die_vict[0] != '\0' )
      fprintf( fpout, "Dievict      %s~\n", skill->die_vict );
   if( skill->die_room && skill->die_room[0] != '\0' )
      fprintf( fpout, "Dieroom      %s~\n", skill->die_room );

   if( skill->imm_char && skill->imm_char[0] != '\0' )
      fprintf( fpout, "Immchar      %s~\n", skill->imm_char );
   if( skill->imm_vict && skill->imm_vict[0] != '\0' )
      fprintf( fpout, "Immvict      %s~\n", skill->imm_vict );
   if( skill->imm_room && skill->imm_room[0] != '\0' )
      fprintf( fpout, "Immroom      %s~\n", skill->imm_room );

   if( skill->dice && skill->dice[0] != '\0' )
      fprintf( fpout, "Dice         %s~\n", skill->dice );
   if( skill->value )
      fprintf( fpout, "Value        %d\n", skill->value );
   if( skill->difficulty )
      fprintf( fpout, "Difficulty   %d\n", skill->difficulty );
   if( skill->participants )
      fprintf( fpout, "Participants %d\n", skill->participants );
   if( skill->components && skill->components[0] != '\0' )
      fprintf( fpout, "Components   %s~\n", skill->components );
   if( skill->teachers && skill->teachers[0] != '\0' )
      fprintf( fpout, "Teachers     %s~\n", skill->teachers );
   for( aff = skill->first_affect; aff; aff = aff->next )
   {
      fprintf( fpout, "Affect       '%s' %d ", aff->duration, aff->location );
      modifier = atoi( aff->modifier );
      if( ( aff->location == APPLY_WEAPONSPELL
            || aff->location == APPLY_WEARSPELL
            || aff->location == APPLY_REMOVESPELL
            || aff->location == APPLY_STRIPSN || aff->location == APPLY_RECURRINGSPELL ) && IS_VALID_SN( modifier ) )
         fprintf( fpout, "'%d' ", skill_table[modifier]->slot );
      else
         fprintf( fpout, "'%s' ", aff->modifier );
      fprintf( fpout, "%d\n", aff->bitvector );
   }

   if( skill->type != SKILL_HERB )
   {
      int y;
      int min = 1000;
      for( y = 0; y < MAX_PC_CLASS; ++y )
         if( skill->skill_level[y] < min )
            min = skill->skill_level[y];

      fprintf( fpout, "Minlevel     %d\n", min );

      min = 1000;
      for( y = 0; y < MAX_PC_RACE; ++y )
         if( skill->race_level[y] < min )
            min = skill->race_level[y];

   }
   fprintf( fpout, "End\n\n" );
}

/*
 * Save the skill table to disk
 */
void save_skill_table(  )
{
   int x;
   FILE *fpout;

   if( ( fpout = fopen( SKILL_FILE, "w" ) ) == NULL )
   {
      perror( SKILL_FILE );
      bug( "%s: Cannot open %s for writting", __func__, SKILL_FILE );
      return;
   }

   for( x = 0; x < num_skills; ++x )
   {
      if( !skill_table[x]->name || skill_table[x]->name[0] == '\0' )
         break;
      fprintf( fpout, "#SKILL\n" );
      fwrite_skill( fpout, skill_table[x] );
   }
   fprintf( fpout, "#END\n" );
   FCLOSE( fpout );
}

/*
 * Save the herb table to disk
 */
void save_herb_table(  )
{
   int x;
   FILE *fpout;

   if( !( fpout = fopen( HERB_FILE, "w" ) ) )
   {
      bug( "%s: Cannot open %s for writting", __func__, HERB_FILE );
      perror( HERB_FILE );
      return;
   }

   for( x = 0; x < top_herb; ++x )
   {
      if( !herb_table[x]->name || herb_table[x]->name[0] == '\0' )
         break;
      fprintf( fpout, "#HERB\n" );
      fwrite_skill( fpout, herb_table[x] );
   }
   fprintf( fpout, "#END\n" );
   FCLOSE( fpout );
}

/*
 * Save the socials to disk
 */
void save_socials(  )
{
   FILE *fpout;
   SOCIALTYPE *social;
   int x;

   if( ( fpout = fopen( SOCIAL_FILE, "w" ) ) == NULL )
   {
      bug( "%s: Cannot open %s for writting", __func__, SOCIAL_FILE );
      perror( SOCIAL_FILE );
      return;
   }

   for( x = 0; x < 27; ++x )
   {
      for( social = social_index[x]; social; social = social->next )
      {
         if( !social->name || social->name[0] == '\0' )
         {
            bug( "%s: blank social in hash bucket %d", __func__, x );
            continue;
         }
         fprintf( fpout, "#SOCIAL\n" );
         fprintf( fpout, "Name        %s~\n", social->name );
         if( social->char_no_arg )
            fprintf( fpout, "CharNoArg   %s~\n", social->char_no_arg );
         else
            bug( "%s: NULL char_no_arg in hash bucket %d", __func__, x );
         if( social->others_no_arg )
            fprintf( fpout, "OthersNoArg %s~\n", social->others_no_arg );
         if( social->char_found )
            fprintf( fpout, "CharFound   %s~\n", social->char_found );
         if( social->others_found )
            fprintf( fpout, "OthersFound %s~\n", social->others_found );
         if( social->vict_found )
            fprintf( fpout, "VictFound   %s~\n", social->vict_found );
         if( social->char_auto )
            fprintf( fpout, "CharAuto    %s~\n", social->char_auto );
         if( social->others_auto )
            fprintf( fpout, "OthersAuto  %s~\n", social->others_auto );
         fprintf( fpout, "End\n\n" );
      }
   }
   fprintf( fpout, "#END\n" );
   FCLOSE( fpout );
}

int get_skill( const char *skilltype )
{
   if( !str_cmp( skilltype, "Racial" ) )
      return SKILL_RACIAL;
   if( !str_cmp( skilltype, "Spell" ) )
      return SKILL_SPELL;
   if( !str_cmp( skilltype, "Skill" ) )
      return SKILL_SKILL;
   if( !str_cmp( skilltype, "Weapon" ) )
      return SKILL_WEAPON;
   if( !str_cmp( skilltype, "Tongue" ) )
      return SKILL_TONGUE;
   if( !str_cmp( skilltype, "Herb" ) )
      return SKILL_HERB;
   return SKILL_UNKNOWN;
}

/*
 * Save the commands to disk
 * Added flags Aug 25, 1997 --Shaddai
 */
void save_commands(  )
{
   FILE *fpout;
   CMDTYPE *command;
   int x;

   if( ( fpout = fopen( COMMAND_FILE, "w" ) ) == NULL )
   {
      bug( "%s: Cannot open %s for writing", __func__, COMMAND_FILE );
      perror( COMMAND_FILE );
      return;
   }

   for( x = 0; x < 126; ++x )
   {
      for( command = command_hash[x]; command; command = command->next )
      {
         if( !command->name || command->name[0] == '\0' )
         {
            bug( "%s: blank command in hash bucket %d", __func__, x );
            continue;
         }
         fprintf( fpout, "#COMMAND\n" );
         fprintf( fpout, "Name        %s~\n", command->name );
         fprintf( fpout, "Code        %s\n", command->fun_name ? command->fun_name : "" );   // Modded to use new field - Trax
         /*
          * Oops I think this may be a bad thing so I changed it -- Shaddai 
          */
         if( command->position < 100 )
            fprintf( fpout, "Position    %d\n", command->position + 100 );
         else
            fprintf( fpout, "Position    %d\n", command->position );
         fprintf( fpout, "Level       %d\n", command->level );
         fprintf( fpout, "Log         %d\n", command->log );
         if( command->flags )
            fprintf( fpout, "Flags       %d\n", command->flags );
         fprintf( fpout, "End\n\n" );
      }
   }
   fprintf( fpout, "#END\n" );
   FCLOSE( fpout );
}

SKILLTYPE *fread_skill( FILE * fp )
{
   const char *word;
   bool fMatch;
   bool got_info = FALSE;
   SKILLTYPE *skill;
   int x;

   CREATE( skill, SKILLTYPE, 1 );
   skill->slot = 0;
   skill->min_mana = 0;
   for( x = 0; x < MAX_CLASS; ++x )
   {
      skill->skill_level[x] = LEVEL_IMMORTAL;
      skill->skill_adept[x] = 95;
   }
   for( x = 0; x < MAX_RACE; ++x )
   {
      skill->race_level[x] = LEVEL_IMMORTAL;
      skill->race_adept[x] = 95;
   }
   skill->guild = -1;
   skill->target = 0;
   skill->skill_fun = NULL;
   skill->spell_fun = NULL;
   skill->spell_sector = 0;

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            if( !str_cmp( word, "Affect" ) )
            {
               SMAUG_AFF *aff;

               CREATE( aff, SMAUG_AFF, 1 );
               aff->duration = str_dup( fread_word( fp ) );
               aff->location = fread_number( fp );
               aff->modifier = str_dup( fread_word( fp ) );
               aff->bitvector = fread_number( fp );

               if( !got_info )
               {
                  for( x = 0; x < 32; ++x )
                  {
                     if( IS_SET( aff->bitvector, 1 << x ) )
                     {
                        aff->bitvector = x;
                        break;
                     }
                  }
                  if( x == 32 )
                     aff->bitvector = -1;
               }
               LINK( aff, skill->first_affect, skill->last_affect, next, prev );
               fMatch = TRUE;
               break;
            }
            KEY( "Alignment", skill->alignment, fread_number( fp ) );
            break;

         case 'C':
            if( !str_cmp( word, "Class" ) )
            {
               int Class = fread_number( fp );

               skill->skill_level[Class] = fread_number( fp );
               skill->skill_adept[Class] = fread_number( fp );
               fMatch = TRUE;
               break;
            }

            if( !str_cmp( word, "Code" ) )
            {
               SPELL_FUN *spellfun;
               DO_FUN *dofun;
               char *w = fread_word( fp );

               fMatch = TRUE;
               if( !str_prefix( "do_", w ) && ( dofun = skill_function( w ) ) != skill_notfound )
               {
                  skill->skill_fun = dofun;
                  skill->spell_fun = NULL;
                  skill->skill_fun_name = str_dup( w );
               }
               else if( str_prefix( "do_", w ) && ( spellfun = spell_function( w ) ) != spell_notfound )
               {
                  skill->spell_fun = spellfun;
                  skill->skill_fun = NULL;
                  skill->spell_fun_name = str_dup( w );
               }
               else
               {
                  bug( "%s: unknown skill/spell %s", __func__, w );
                  skill->spell_fun = spell_null;
               }
               break;
            }
            KEY( "Components", skill->components, fread_string_nohash( fp ) );
            break;

         case 'D':
            KEY( "Dammsg", skill->noun_damage, fread_string_nohash( fp ) );
            KEY( "Dice", skill->dice, fread_string_nohash( fp ) );
            KEY( "Diechar", skill->die_char, fread_string_nohash( fp ) );
            KEY( "Dieroom", skill->die_room, fread_string_nohash( fp ) );
            KEY( "Dievict", skill->die_vict, fread_string_nohash( fp ) );
            KEY( "Difficulty", skill->difficulty, fread_number( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( skill->saves != 0 && SPELL_SAVE( skill ) == SE_NONE )
               {
                  bug( "%s (%s):  Has saving throw (%d) with no saving effect.", __func__, skill->name, skill->saves );
                  SET_SSAV( skill, SE_NEGATE );
               }
               return skill;
            }
            break;

         case 'F':
            if( !str_cmp( word, "Flags" ) )
            {
               skill->flags = fread_number( fp );
               /*
                * convert to new style       -Thoric
                */
               if( !got_info )
               {
                  skill->info = skill->flags & ( BV11 - 1 );
                  if( IS_SET( skill->flags, OLD_SF_SAVE_NEGATES ) )
                  {
                     if( IS_SET( skill->flags, OLD_SF_SAVE_HALF_DAMAGE ) )
                     {
                        SET_SSAV( skill, SE_QUARTERDAM );
                        REMOVE_BIT( skill->flags, OLD_SF_SAVE_HALF_DAMAGE );
                     }
                     else
                        SET_SSAV( skill, SE_NEGATE );
                     REMOVE_BIT( skill->flags, OLD_SF_SAVE_NEGATES );
                  }
                  else if( IS_SET( skill->flags, OLD_SF_SAVE_HALF_DAMAGE ) )
                  {
                     SET_SSAV( skill, SE_HALFDAM );
                     REMOVE_BIT( skill->flags, OLD_SF_SAVE_HALF_DAMAGE );
                  }
                  skill->flags >>= 11;
               }
               fMatch = TRUE;
               break;
            }
            break;

         case 'G':
            KEY( "Guild", skill->guild, fread_number( fp ) );
            break;

         case 'H':
            KEY( "Hitchar", skill->hit_char, fread_string_nohash( fp ) );
            KEY( "Hitdest", skill->hit_dest, fread_string_nohash( fp ) );
            KEY( "Hitroom", skill->hit_room, fread_string_nohash( fp ) );
            KEY( "Hitvict", skill->hit_vict, fread_string_nohash( fp ) );
            break;

         case 'I':
            KEY( "Immchar", skill->imm_char, fread_string_nohash( fp ) );
            KEY( "Immroom", skill->imm_room, fread_string_nohash( fp ) );
            KEY( "Immvict", skill->imm_vict, fread_string_nohash( fp ) );
            if( !str_cmp( word, "Info" ) )
            {
               skill->info = fread_number( fp );
               got_info = TRUE;
               fMatch = TRUE;
               break;
            }
            break;

         case 'M':
            KEY( "Mana", skill->min_mana, fread_number( fp ) );
            if( !str_cmp( word, "Minlevel" ) )
            {
               fread_to_eol( fp );
               fMatch = TRUE;
               break;
            }

            /*
             * 
             */
            if( !str_cmp( word, "Minpos" ) )
            {
               fMatch = TRUE;
               skill->minimum_position = fread_number( fp );
               if( skill->minimum_position < 100 )
               {
                  switch ( skill->minimum_position )
                  {
                     default:
                     case 0:
                     case 1:
                     case 2:
                     case 3:
                     case 4:
                        break;
                     case 5:
                        skill->minimum_position = 6;
                        break;
                     case 6:
                        skill->minimum_position = 8;
                        break;
                     case 7:
                        skill->minimum_position = 9;
                        break;
                     case 8:
                        skill->minimum_position = 12;
                        break;
                     case 9:
                        skill->minimum_position = 13;
                        break;
                     case 10:
                        skill->minimum_position = 14;
                        break;
                     case 11:
                        skill->minimum_position = 15;
                        break;
                  }
               }
               else
                  skill->minimum_position -= 100;
               break;
            }

            KEY( "Misschar", skill->miss_char, fread_string_nohash( fp ) );
            KEY( "Missroom", skill->miss_room, fread_string_nohash( fp ) );
            KEY( "Missvict", skill->miss_vict, fread_string_nohash( fp ) );
            break;

         case 'N':
            KEY( "Name", skill->name, fread_string_nohash( fp ) );
            break;

         case 'P':
            KEY( "Participants", skill->participants, fread_number( fp ) );
            break;

         case 'R':
            KEY( "Range", skill->range, fread_number( fp ) );
            KEY( "Rounds", skill->beats, fread_number( fp ) );
            if( !str_cmp( word, "Race" ) )
            {
               int race = fread_number( fp );

               skill->race_level[race] = fread_number( fp );
               skill->race_adept[race] = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'S':
            KEY( "Saves", skill->saves, fread_number( fp ) );
            KEY( "Slot", skill->slot, fread_number( fp ) );
            KEY( "Ssector", skill->spell_sector, fread_number( fp ) );
            break;

         case 'T':
            KEY( "Target", skill->target, fread_number( fp ) );
            KEY( "Teachers", skill->teachers, fread_string_nohash( fp ) );
            KEY( "Type", skill->type, get_skill( fread_word( fp ) ) );
            break;

         case 'V':
            KEY( "Value", skill->value, fread_number( fp ) );
            break;

         case 'W':
            KEY( "Wearoff", skill->msg_off, fread_string_nohash( fp ) );
            break;
      }

      if( !fMatch )
      {
         bug( "%s: no match: %s", __func__, word );
         fread_to_eol( fp );
      }
   }
}

void load_skill_table(  )
{
   FILE *fp;

   if( ( fp = fopen( SKILL_FILE, "r" ) ) != NULL )
   {
      num_skills = 0;
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
            bug( "%s: # not found.", __func__ );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "SKILL" ) )
         {
            if( num_skills >= MAX_SKILL )
            {
               bug( "%s: more skills than MAX_SKILL %d", __func__, MAX_SKILL );
               FCLOSE( fp );
               return;
            }
            skill_table[num_skills++] = fread_skill( fp );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section.", __func__ );
            continue;
         }
      }
      FCLOSE( fp );
   }
   else
   {
      perror( SKILL_FILE );
      bug( "%s: Cannot open %s", __func__, SKILL_FILE );
      exit( 0 );
   }
}

void load_herb_table(  )
{
   FILE *fp;

   if( ( fp = fopen( HERB_FILE, "r" ) ) != NULL )
   {
      top_herb = 0;
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
            bug( "%s: # not found.", __func__ );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "HERB" ) )
         {
            if( top_herb >= MAX_HERB )
            {
               bug( "%s: more herbs than MAX_HERB %d", __func__, MAX_HERB );
               FCLOSE( fp );
               return;
            }
            herb_table[top_herb++] = fread_skill( fp );
            if( herb_table[top_herb - 1]->slot == 0 )
               herb_table[top_herb - 1]->slot = top_herb - 1;
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section.", __func__ );
            continue;
         }
      }
      FCLOSE( fp );
   }
   else
   {
      bug( "%s: Cannot open %s", __func__, HERB_FILE );
      exit( 0 );
   }
}

void fread_social( FILE * fp )
{
   const char *word;
   bool fMatch;
   SOCIALTYPE *social;

   CREATE( social, SOCIALTYPE, 1 );

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'C':
            KEY( "CharNoArg", social->char_no_arg, fread_string_nohash( fp ) );
            KEY( "CharFound", social->char_found, fread_string_nohash( fp ) );
            KEY( "CharAuto", social->char_auto, fread_string_nohash( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !social->name )
               {
                  bug( "%s: Name not found", __func__ );
                  free_social( social );
                  return;
               }
               if( !social->char_no_arg )
               {
                  bug( "%s: CharNoArg not found", __func__ );
                  free_social( social );
                  return;
               }
               add_social( social );
               return;
            }
            break;

         case 'N':
            KEY( "Name", social->name, fread_string_nohash( fp ) );
            break;

         case 'O':
            KEY( "OthersNoArg", social->others_no_arg, fread_string_nohash( fp ) );
            KEY( "OthersFound", social->others_found, fread_string_nohash( fp ) );
            KEY( "OthersAuto", social->others_auto, fread_string_nohash( fp ) );
            break;

         case 'V':
            KEY( "VictFound", social->vict_found, fread_string_nohash( fp ) );
            break;
      }

      if( !fMatch )
      {
         bug( "%s: no match: %s", __func__, word );
         fread_to_eol( fp );
      }
   }
}

void load_socials(  )
{
   FILE *fp;

   if( ( fp = fopen( SOCIAL_FILE, "r" ) ) != NULL )
   {
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
            bug( "%s: # not found.", __func__ );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "SOCIAL" ) )
         {
            fread_social( fp );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section.", __func__ );
            continue;
         }
      }
      FCLOSE( fp );
   }
   else
   {
      bug( "%s: Cannot open %s", __func__, SOCIAL_FILE );
      exit( 0 );
   }
}

/*
 *  Added the flags Aug 25, 1997 --Shaddai
 */
void fread_command( FILE * fp )
{
   const char *word;
   bool fMatch;
   CMDTYPE *command;

   CREATE( command, CMDTYPE, 1 );
   command->lag_count = 0; /* can't have caused lag yet... FB */
   command->flags = 0;  /* Default to no flags set */

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'C':
            KEY( "Code", command->fun_name, str_dup( fread_word( fp ) ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !command->name )
               {
                  bug( "%s: Name not found", __func__ );
                  free_command( command );
                  return;
               }
               if( !command->fun_name )
               {
                  bug( "%s: No function name supplied for %s", __func__, command->name );
                  free_command( command );
                  return;
               }
               /*
                * Mods by Trax
                * Fread in code into char* and try linkage here then
                * deal in the "usual" way I suppose..
                */
               command->do_fun = skill_function( command->fun_name );
               if( command->do_fun == skill_notfound )
               {
                  bug( "%s: Function %s not found for %s", __func__, command->fun_name, command->name );
                  free_command( command );
                  return;
               }
               add_command( command );
               return;
            }
            break;

         case 'F':
            KEY( "Flags", command->flags, fread_number( fp ) );
            break;

         case 'L':
            KEY( "Level", command->level, fread_number( fp ) );
            KEY( "Log", command->log, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", command->name, fread_string_nohash( fp ) );
            break;

         case 'P':
            /*
             * KEY( "Position", command->position,   fread_number(fp) ); 
             */
            if( !str_cmp( word, "Position" ) )
            {
               fMatch = TRUE;
               command->position = fread_number( fp );
               if( command->position < 100 )
               {
                  switch ( command->position )
                  {
                     default:
                     case 0:
                     case 1:
                     case 2:
                     case 3:
                     case 4:
                        break;
                     case 5:
                        command->position = 6;
                        break;
                     case 6:
                        command->position = 8;
                        break;
                     case 7:
                        command->position = 9;
                        break;
                     case 8:
                        command->position = 12;
                        break;
                     case 9:
                        command->position = 13;
                        break;
                     case 10:
                        command->position = 14;
                        break;
                     case 11:
                        command->position = 15;
                        break;
                  }
               }
               else
                  command->position -= 100;
               break;
            }
            break;
      }

      if( !fMatch )
      {
         bug( "%s: no match: %s", __func__, word );
         fread_to_eol( fp );
      }
   }
}

void load_commands(  )
{
   FILE *fp;

   if( ( fp = fopen( COMMAND_FILE, "r" ) ) != NULL )
   {
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
            bug( "%s: # not found.", __func__ );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "COMMAND" ) )
         {
            fread_command( fp );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section.", __func__ );
            continue;
         }
      }
      FCLOSE( fp );
   }
   else
   {
      bug( "%s: Cannot open %s", __func__, COMMAND_FILE );
      exit( 0 );
   }
}

void save_classes(  )
{
   int x;

   for( x = 0; x < MAX_PC_CLASS; ++x )
      write_class_file( x );
}

void save_races(  )
{
   int x;

   for( x = 0; x < MAX_PC_RACE; ++x )
      write_race_file( x );
}

void free_tongues( void )
{
   LANG_DATA *lang, *lang_next;
   LCNV_DATA *lcnv, *lcnv_next;

   for( lang = first_lang; lang; lang = lang_next )
   {
      lang_next = lang->next;

      for( lcnv = lang->first_precnv; lcnv; lcnv = lcnv_next )
      {
         lcnv_next = lcnv->next;
         UNLINK( lcnv, lang->first_precnv, lang->last_precnv, next, prev );
         DISPOSE( lcnv->old );
         DISPOSE( lcnv->lnew );
         DISPOSE( lcnv );
      }
      for( lcnv = lang->first_cnv; lcnv; lcnv = lcnv_next )
      {
         lcnv_next = lcnv->next;
         UNLINK( lcnv, lang->first_cnv, lang->last_cnv, next, prev );
         DISPOSE( lcnv->old );
         DISPOSE( lcnv->lnew );
         DISPOSE( lcnv );
      }
      STRFREE( lang->name );
      STRFREE( lang->alphabet );
      UNLINK( lang, first_lang, last_lang, next, prev );
      DISPOSE( lang );
   }
   return;
}

/*
 * Tongues / Languages loading/saving functions			-Altrag
 */
void fread_cnv( FILE * fp, LCNV_DATA ** first_cnv, LCNV_DATA ** last_cnv )
{
   LCNV_DATA *cnv;
   char letter;

   for( ;; )
   {
      letter = fread_letter( fp );
      if( letter == '~' || letter == EOF )
         break;
      ungetc( letter, fp );
      CREATE( cnv, LCNV_DATA, 1 );

      cnv->old = str_dup( fread_word( fp ) );
      cnv->olen = strlen( cnv->old );
      cnv->lnew = str_dup( fread_word( fp ) );
      cnv->nlen = strlen( cnv->lnew );
      fread_to_eol( fp );
      LINK( cnv, *first_cnv, *last_cnv, next, prev );
   }
}

void load_tongues(  )
{
   FILE *fp;
   LANG_DATA *lng;
   char *word;
   char letter;

   if( !( fp = fopen( TONGUE_FILE, "r" ) ) )
   {
      perror( "Load_tongues" );
      return;
   }
   for( ;; )
   {
      letter = fread_letter( fp );
      if( letter == EOF )
         return;
      else if( letter == '*' )
      {
         fread_to_eol( fp );
         continue;
      }
      else if( letter != '#' )
      {
         bug( "%s: Letter '%c' not #.", __func__, letter );
         exit( 0 );
      }
      word = fread_word( fp );
      if( !str_cmp( word, "end" ) )
         break;
      fread_to_eol( fp );
      CREATE( lng, LANG_DATA, 1 );
      lng->name = STRALLOC( word );
      fread_cnv( fp, &lng->first_precnv, &lng->last_precnv );
      lng->alphabet = fread_string( fp );
      fread_cnv( fp, &lng->first_cnv, &lng->last_cnv );
      fread_to_eol( fp );
      LINK( lng, first_lang, last_lang, next, prev );
   }
   FCLOSE( fp );
   return;
}

void fwrite_langs( void )
{
   FILE *fp;
   LANG_DATA *lng;
   LCNV_DATA *cnv;

   if( !( fp = fopen( TONGUE_FILE, "w" ) ) )
   {
      perror( "fwrite_langs" );
      return;
   }
   for( lng = first_lang; lng; lng = lng->next )
   {
      fprintf( fp, "#%s\n", lng->name );
      for( cnv = lng->first_precnv; cnv; cnv = cnv->next )
         fprintf( fp, "'%s' '%s'\n", cnv->old, cnv->lnew );
      fprintf( fp, "~\n%s~\n", lng->alphabet );
      for( cnv = lng->first_cnv; cnv; cnv = cnv->next )
         fprintf( fp, "'%s' '%s'\n", cnv->old, cnv->lnew );
      fprintf( fp, "\n" );
   }
   fprintf( fp, "#end\n\n" );
   FCLOSE( fp );
   return;
}
