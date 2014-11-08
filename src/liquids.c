/*****************************************************
**     _________       __                           **
**     \_   ___ \_____|__| _____  ________  ___     **
**      /    \  \/_  __ \ |/     \/  ___/_ \/   \   **
**      \     \___|  | \/ |  | |  \___ \  / ) |  \  **
**       \______  /__| |__|__|_|  /____ \__/__|  /  **
**         ____\/____ _        \/ ___ \/      \/    **
**         \______   \ |_____  __| _/___            **
**          |    |  _/ |\__  \/ __ | __ \           **
**          |    |   \ |_/ __ \  / | ___/_          **
**          |_____  /__/____  /_  /___  /           **
**               \/Antipode\/  \/    \/             **
******************************************************
******************************************************
**       Copyright 2000-2003 Crimson Blade          **
******************************************************
** Contributors: Noplex, Krowe, Emberlyna, Lanthos  **
******************************************************/

/*
 * File: liquids.c
 * Name: Liquidtable Module (3.0b)
 * Author: John 'Noplex' Bellone (jbellone@comcast.net)
 * Terms:
 * If this file is to be re-disributed; you must send an email
 * to the author. All headers above the #include calls must be
 * kept intact. All license requirements must be met. License
 * can be found in the included license.txt document or on the
 * website.
 * Description:
 * This module is a rewrite of the original module which allowed for
 * a SMAUG mud to have a fully online editable liquidtable; adding liquids;
 * removing them; and editing them online. It allows an near-endless supply
 * of liquids for builder's to work with.
 * A second addition to this module allowed for builder's to create mixtures;
 * when two liquids were mixed together they would produce a different liquid.
 * Yet another adaptation to the above concept allowed for objects to be mixed
 * with liquids to produce a liquid.
 * This newest version offers a cleaner running code; smaller; and faster in
 * all ways around. Hopefully it'll knock out the old one ten fold ;)
 * Also in the upcoming 'testing' phase of this code; new additions will be added
 * including a better alchemey system for creating poitions as immortals; and as
 * mortals.
 */

#include <stdio.h>
#include "mud.h"

#ifndef NULLSTR
#define NULLSTR(str)         (!str || str[0] == '\0')
#endif

// macro to be used when we know that the string is non-null
// (to avoid compiler warnings)
#ifndef EMPTYSTR
#define EMPTYSTR(str)         (str[0] == '\0')
#endif

/* globals */
LIQ_TABLE *liquid_table[MAX_LIQUIDS];
MIX_TABLE *first_mixture;
MIX_TABLE *last_mixture;

const char *const liquid_types[LIQTYPE_TOP] = {
   "Beverage", "Alcohol", "Poison", "Blood"
};

const char *const mod_types[MAX_CONDS] = {
   "Drunk", "Full", "Thirst", "Bloodthirst"
};

void save_house_by_vnum( int vnum );

/* locals */
int top_liquid;
int liq_count;

/* liquid i/o functions */

/* save the liquids to the liquidtable.dat file in the system directory -Nopey */
void save_liquids( void )
{
   FILE *fp = NULL;
   LIQ_TABLE *liq = NULL;
   char filename[256];
   int i;

   snprintf( filename, 256, "%sliquids.dat", SYSTEM_DIR );
   if( !( fp = fopen( filename, "w" ) ) )
   {
      bug( "%s: cannot open %s for writing", __FUNCTION__, filename );
      return;
   }

   for( i = 0; i <= top_liquid; i++ )
   {
      if( !liquid_table[i] )
         continue;

      liq = liquid_table[i];

      fprintf( fp, "%s", "#LIQUID\n" );
      fprintf( fp, "Name      %s~\n", liq->name );
      fprintf( fp, "Shortdesc %s~\n", liq->shortdesc );
      fprintf( fp, "Color     %s~\n", liq->color );
      fprintf( fp, "Type      %d\n", liq->type );
      fprintf( fp, "Vnum      %d\n", liq->vnum );
      fprintf( fp, "Mod       %d %d %d %d\n", liq->mod[COND_DRUNK], liq->mod[COND_FULL], liq->mod[COND_THIRST],
               liq->mod[COND_BLOODTHIRST] );
      fprintf( fp, "%s", "End\n\n" );
   }
   fprintf( fp, "%s", "#END\n" );
   fclose( fp );
   fp = NULL;
   return;
}

/* read the liquids from a file descriptor and then distribute them accordingly to the struct -Nopey */
LIQ_TABLE *fread_liquid( FILE * fp )
{
   LIQ_TABLE *liq = NULL;
   bool fMatch = FALSE;
   int i;

   CREATE( liq, LIQ_TABLE, 1 );
   liq->vnum = -1;
   liq->type = -1;
   for( i = 0; i < MAX_CONDS; i++ )
      liq->mod[i] = -1;

   for( ;; )
   {
      const char *word = feof( fp ) ? "End" : fread_word( fp );

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fread_to_eol( fp );
            break;

         case 'C':
            KEY( "Color", liq->color, fread_string( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( liq->vnum <= -1 )
                  return NULL;
               return liq;
            }
            break;

         case 'N':
            KEY( "Name", liq->name, fread_string( fp ) );
            break;

         case 'M':
            if( !str_cmp( word, "Mod" ) )
            {
               liq->mod[COND_DRUNK] = fread_number( fp );
               liq->mod[COND_FULL] = fread_number( fp );
               liq->mod[COND_THIRST] = fread_number( fp );
               liq->mod[COND_BLOODTHIRST] = fread_number( fp );
            }
            break;

         case 'S':
            KEY( "Shortdesc", liq->shortdesc, fread_string( fp ) );
            break;

         case 'T':
            KEY( "Type", liq->type, fread_number( fp ) );
            break;

         case 'V':
            KEY( "Vnum", liq->vnum, fread_number( fp ) );
            break;
      }
      if( !fMatch )
      {
         bug( "%s: no match for %s", __FUNCTION__, word );
         fread_to_eol( fp );
      }
   }
}

/* load the liquids from the liquidtable.dat file in the system directory -Nopey */
void load_liquids( void )
{
   FILE *fp = NULL;
   char filename[256];

   snprintf( filename, 256, "%sliquids.dat", SYSTEM_DIR );
   if( !( fp = fopen( filename, "r" ) ) )
   {
      bug( "load_liquids(): cannot open %s for reading", filename );
      return;
   }

   top_liquid = -1;
   liq_count = 0;

   for( ;; )
   {
      char letter = fread_letter( fp );
      char *word;

      if( letter == '*' )
      {
         fread_to_eol( fp );
         continue;
      }

      if( letter != '#' )
      {
         bug( "%s: # not found (%c)", __FUNCTION__, letter );
         return;
      }

      word = fread_word( fp );
      if( !str_cmp( word, "LIQUID" ) )
      {
         LIQ_TABLE *liq = fread_liquid( fp );

         if( !liq )
            bug( "%s: returned NULL liquid", __FUNCTION__ );
         else
         {
            liquid_table[liq->vnum] = liq;
            if( liq->vnum > top_liquid )
               top_liquid = liq->vnum;
            liq_count++;
         }
         continue;
      }
      else if( !str_cmp( word, "END" ) )
         break;
      else
      {
         bug( "%s: no match for %s", __FUNCTION__, word );
         continue;
      }
   }
   fclose( fp );
   fp = NULL;
   return;
}

/* save the mixtures to the mixture table -Nopey */
void save_mixtures( void )
{
   MIX_TABLE *mix = NULL;
   FILE *fp = NULL;
   char filename[256];

   snprintf( filename, 256, "%smixtures.dat", SYSTEM_DIR );
   if( !( fp = fopen( filename, "w" ) ) )
   {
      bug( "%s: cannot open %s for writing", __FUNCTION__, filename );
      return;
   }

   for( mix = first_mixture; mix; mix = mix->next )
   {
      fprintf( fp, "%s", "#MIXTURE\n" );
      fprintf( fp, "Name   %s~\n", mix->name );
      fprintf( fp, "Data   %d %d %d\n", mix->data[0], mix->data[1], mix->data[2] );
      fprintf( fp, "Object %d\n", mix->object );
      fprintf( fp, "%s", "End\n" );
   }
   fprintf( fp, "%s", "#END\n" );
   fclose( fp );
   fp = NULL;
   return;
}

/* read the mixtures into the structure -Nopey */
MIX_TABLE *fread_mixture( FILE * fp )
{
   MIX_TABLE *mix = NULL;
   bool fMatch = FALSE;

   CREATE( mix, MIX_TABLE, 1 );
   mix->data[0] = -1;
   mix->data[1] = -1;
   mix->data[2] = -1;
   mix->object = FALSE;

   for( ;; )
   {
      const char *word = feof( fp ) ? "End" : fread_word( fp );

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fread_to_eol( fp );
            break;

         case 'D':
            if( !str_cmp( word, "Data" ) )
            {
               mix->data[0] = fread_number( fp );
               mix->data[1] = fread_number( fp );
               mix->data[2] = fread_number( fp );
            }
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               return mix;
            }
            break;

         case 'I':
            KEY( "Into", mix->data[2], fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", mix->name, fread_string( fp ) );
            break;

         case 'O':
            KEY( "Object", mix->object, fread_number( fp ) );
            break;

         case 'W':
            if( !str_cmp( word, "With" ) )
            {
               mix->data[0] = fread_number( fp );
               mix->data[1] = fread_number( fp );
            }
            break;
      }
      if( !fMatch )
      {
         bug( "%s: no match for %s", __FUNCTION__, word );
         fread_to_eol( fp );
      }
   }
}

/* load the mixtures from the mixture table         -Nopey */
void load_mixtures( void )
{
   FILE *fp = NULL;
   char filename[256];

   snprintf( filename, 256, "%smixtures.dat", SYSTEM_DIR );
   if( !( fp = fopen( filename, "r" ) ) )
   {
      bug( "%s: cannot open %s for reading", __FUNCTION__, filename );
      return;
   }

   for( ;; )
   {
      char letter = fread_letter( fp );
      char *word;

      if( letter == '*' )
      {
         fread_to_eol( fp );
         break;
      }

      if( letter != '#' )
      {
         bug( "%s: # not found (%c)", __FUNCTION__, letter );
         return;
      }

      word = fread_word( fp );
      if( !str_cmp( word, "MIXTURE" ) )
      {
         MIX_TABLE *mix = NULL;

         mix = fread_mixture( fp );
         if( !mix )
            bug( "%s", "load_mixtures(): mixture returned NULL" );
         else
            LINK( mix, first_mixture, last_mixture, next, prev );
      }
      else if( !str_cmp( word, "END" ) )
         break;
      else
      {
         bug( "%s: no match for %s", __FUNCTION__, word );
         break;
      }
   }
   fclose( fp );
   fp = NULL;
   return;
}

/* figure out a vnum for the next liquid  -Nopey */
static int figure_liq_vnum( void )
{
   int i;

   /*
    * incase a liquid gets removed; we can fill it's place 
    */
   for( i = 0; liquid_table[i] != NULL; i++ );

   /*
    * add to the top 
    */
   if( i > top_liquid )
      top_liquid = i;

   return i;
}

/* lookup func for liquids      -Nopey */
LIQ_TABLE *get_liq( const char *str )
{
   int i;

   if( is_number( str ) )
   {
      i = atoi( str );

      return liquid_table[i];
   }
   else
   {
      for( i = 0; i < top_liquid; i++ )
         if( !str_cmp( liquid_table[i]->name, str ) )
            return liquid_table[i];
   }
   return NULL;
}

LIQ_TABLE *get_liq_vnum( int vnum )
{
   return liquid_table[vnum];
}

/* lookup func for mixtures      -Nopey */
MIX_TABLE *get_mix( const char *str )
{
   MIX_TABLE *mix = NULL;

   for( mix = first_mixture; mix; mix = mix->next )
      if( !str_cmp( mix->name, str ) )
         return mix;

   return NULL;
}

/* Function to display liquid list. - Tarl 9 Jan 03 */
void do_showliquid( CHAR_DATA* ch, const char* argument)
{
   LIQ_TABLE *liq = NULL;
   int i;

   if( !IS_IMMORTAL( ch ) || IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   if( !NULLSTR( argument ) && ( ( liq = get_liq( argument ) ) != NULL ) )
   {
      if( !NULLSTR( liq->name ) )
         pager_printf( ch, "&GLiquid information for:&g %s\r\n", liq->name );
      if( !NULLSTR( liq->shortdesc ) )
         pager_printf( ch, "&GLiquid shortdesc:&g\t %s\r\n", liq->shortdesc );
      if( !NULLSTR( liq->color ) )
         pager_printf( ch, "&GLiquid color:&g\t %s\r\n", liq->color );
      pager_printf( ch, "&GLiquid vnum:&g\t %d\r\n", liq->vnum );
      pager_printf( ch, "&GLiquid type:&g\t %s\r\n", liquid_types[liq->type] );
      send_to_pager( "&GLiquid Modifiers\r\n", ch );
      for( i = 0; i < MAX_CONDS; i++ )
         if( liquid_table[i] )
            pager_printf( ch, "&G%s:&g\t %d\r\n", mod_types[i], liq->mod[i] );
      return;
   }
   else if( !NULLSTR( argument ) && ( ( liq = get_liq( argument ) ) == NULL ) )
   {
      send_to_char( "Invaild liquid-vnum.\r\nUse 'showliquid' to gain a vaild liquidvnum.\r\n", ch );
      return;
   }
   send_to_pager( "&G[&gVnum&G] [&gName&G]\r\n", ch );
   send_to_pager( "-------------------------\r\n", ch );
   for( i = 0; i <= top_liquid; i++ )
   {
      if( !liquid_table[i] )
         continue;
      pager_printf( ch, "  %-7d %s\r\n", liquid_table[i]->vnum, liquid_table[i]->name );
   }
   send_to_pager( "\r\nUse 'showliquid [vnum]' to view individual liquids.\r\n", ch );
   send_to_pager( "Use 'showmixture' to view the mixturetable.\r\n", ch );
   return;

}

/* olc function for liquids   -Nopey */
void do_setliquid( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];

   if( !IS_IMMORTAL( ch ) || IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   smash_tilde( argument );
   argument = one_argument( argument, arg );

   if( EMPTYSTR( arg ) )
   {
      send_to_char( "Syntax: setliquid <vnum> <field> <value>\r\n"
                    "        setliquid create <name>\r\n" "        setliquid delete <vnum>\r\n", ch );
      send_to_char( " Fields being one of the following:\r\n" " name color type shortdesc drunk thrist blood full\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "create" ) )
   {
      LIQ_TABLE *liq = NULL;
      int i;

      if( liq_count >= MAX_LIQUIDS )
      {
         send_to_char( "Liquid count is at the hard-coded max. Remove some liquids or raise\r\n"
                       "the hard-coded max number of liquids.\r\n", ch );
         return;
      }

      if( NULLSTR( argument ) )
      {
         send_to_char( "Syntax: setliquid create <name>\r\n", ch );
         return;
      }

      CREATE( liq, LIQ_TABLE, 1 );
      liq->name = STRALLOC( argument );
      liq->shortdesc = STRALLOC( argument );
      liq->vnum = figure_liq_vnum(  );
      liq->type = -1;
      for( i = 0; i < MAX_CONDS; i++ )
         liq->mod[i] = -1;
      liquid_table[liq->vnum] = liq;
      liq_count++;
      send_to_char( "Done.\r\n", ch );
      save_liquids(  );
      return;
   }
   else if( !str_cmp( arg, "delete" ) )
   {
      LIQ_TABLE *liq = NULL;

      if( NULLSTR( argument ) )
      {
         send_to_char( "Syntax: setliquid delete <vnum>\r\n", ch );
         return;
      }

      if( !is_number( argument ) )
      {
         if( !( liq = get_liq( argument ) ) )
         {
            send_to_char( "No such liquid type. Use 'showliquid' to get a valid list.\r\n", ch );
            return;
         }
      }
      else
      {
         int i = atoi( argument );

         if( !( liq = get_liq_vnum( i ) ) )
         {
            send_to_char( "No such vnum. Use 'showliquid' to get the vnum.\r\n", ch );
            return;
         }
      }

      STRFREE( liq->name );
      STRFREE( liq->color );
      STRFREE( liq->shortdesc );
      if( liq->vnum >= top_liquid )
      {
         int j;

         for( j = 0; j != liq->vnum; j++ )
            if( j > top_liquid )
               top_liquid = j;
      }
      liquid_table[liq->vnum] = NULL;
      liq_count--;
      DISPOSE( liq );
      send_to_char( "Done.. Liquids saved.\r\n", ch );
      save_liquids(  );
      return;
   }
   else
   {
      char arg2[MAX_INPUT_LENGTH];
      LIQ_TABLE *liq = NULL;

      argument = one_argument( argument, arg2 );
      if( EMPTYSTR( arg2 ) )
      {
         send_to_char( "Syntax: setliquid <vnum> <field> <value>\r\n", ch );
         send_to_char( " Fields being one of the following:\r\n" " name color shortdesc drunk thrist blood full\r\n", ch );
         return;
      }

      if( ( liq = get_liq( arg ) ) == NULL )
      {
         send_to_char( "Invaild liquid-name or vnum.\r\n", ch );
         return;
      }

      if( !str_cmp( arg2, "name" ) )
      {
         if( NULLSTR( argument ) )
         {
            send_to_char( "Syntax: setliquid <vnum> name <name>\r\n", ch );
            return;
         }
         STRFREE( liq->name );
         liq->name = STRALLOC( argument );
      }
      else if( !str_cmp( arg2, "color" ) )
      {
         if( NULLSTR( argument ) )
         {
            send_to_char( "Syntax: setliquid <vnum> color <color>\r\n", ch );
            return;
         }
         STRFREE( liq->color );
         liq->color = STRALLOC( argument );
      }
      else if( !str_cmp( arg2, "shortdesc" ) )
      {
         if( NULLSTR( argument ) )
         {
            send_to_char( "Syntax: setliquid <vnum> shortdesc <shortdesc>\r\n", ch );
            return;
         }
         STRFREE( liq->shortdesc );
         liq->shortdesc = STRALLOC( argument );
      }
      else if( !str_cmp( arg2, "type" ) )
      {
         char arg3[MAX_INPUT_LENGTH];
         int i;
         bool found = FALSE;

         argument = one_argument( argument, arg3 );

         /*
          * bah; forgot to add this shit -- 
          */
         for( i = 0; i < LIQTYPE_TOP; i++ )
            if( !str_cmp( arg3, liquid_types[i] ) )
            {
               found = TRUE;
               liq->type = i;
            }
         if( !found )
         {
            send_to_char( "Syntax: setliquid <vnum> type <liquidtype>\r\n", ch );
            return;
         }
      }
      else
      {
         int i;
         bool found = FALSE;
         static const char *const arg_names[MAX_CONDS] = { "drunk", "full", "thirst", "blood" };

         if( NULLSTR( argument ) )
         {
            send_to_char( "Syntax: setliquid <vnum> <field> <value>\r\n", ch );
            send_to_char( " Fields being one of the following:\r\n"
                          " name color shortdesc drunk thrist blood full\r\n", ch );
            return;
         }

         for( i = 0; i < MAX_CONDS; i++ )
         {
            if( !str_cmp( arg2, arg_names[i] ) )
            {
               found = TRUE;
               liq->mod[i] = atoi( argument );
            }
         }

         if( !found )
         {
            do_setliquid( ch, "" );
            return;
         }
      }
      send_to_char( "Done.\r\n", ch );
      save_liquids(  );
      return;
   }
}

void displaymixture( CHAR_DATA * ch, MIX_TABLE * mix )
{
   send_to_pager( " .-.                ,\r\n", ch );
   send_to_pager( "`._ ,\r\n", ch );
   send_to_pager( "   \\ \\                 o\r\n", ch );
   send_to_pager( "    \\ `-,.\r\n", ch );
   send_to_pager( "   .'o .  `.[]           o\r\n", ch );
   send_to_pager( "<~- - , ,[].'.[] ~>     ___\r\n", ch );
   send_to_pager( " :               :     (-~.)\r\n", ch );
   send_to_pager( "  `             '       `|'\r\n", ch );
   send_to_pager( "   `           '         |\r\n", ch );
   send_to_pager( "    `-.     .-'          |\r\n", ch );
   send_to_pager( "-----{. _ _ .}-------------------\r\n", ch );

   pager_printf( ch, "&gRecipe for Mixture &G%s&g:\r\n", mix->name );
   send_to_pager( "---------------------------------\r\n", ch );
   if( !mix->object )   //this is an object
   {
      LIQ_TABLE *ingred1 = get_liq_vnum( mix->data[0] );
      LIQ_TABLE *ingred2 = get_liq_vnum( mix->data[1] );
      send_to_pager( "&wCombine two liquids to create this mixture:\r\n", ch );
      if( !ingred1 )
      {
         pager_printf( ch, "Vnum1 (%d) is invalid, tell an Admin\r\n", mix->data[0] );
      }
      else
      {
         pager_printf( ch, "&wOne part &G%s&w (%d)\r\n", ingred1->name, mix->data[0] );
      }

      if( !ingred2 )
      {
         pager_printf( ch, "Vnum2 (%d) is invalid, tell an Admin\r\n", mix->data[1] );
      }
      else
      {
         pager_printf( ch, "&wAnd part &G%s&w (%d)&D\r\n", ingred2->name, mix->data[1] );
      }
   }
   else
   {
      OBJ_INDEX_DATA *obj = get_obj_index( mix->data[0] );
      if( !obj )
      {
         pager_printf( ch, "%s has a bad object vnum %d, inform an Admin\r\n", mix->name, mix->data[0] );
         return;
      }
      else
      {
         LIQ_TABLE *ingred1 = get_liq_vnum( mix->data[1] );
         send_to_pager( "Combine an object and a liquid in this mixture\r\n", ch );
         pager_printf( ch, "&wMix &G%s&w (%d)\r\n", obj->name, mix->data[0] );
         pager_printf( ch, "&winto one part &G%s&w (%d)&D\r\n", ingred1->name, mix->data[1] );
      }
   }
   return;
}

/* Function for showmixture - Tarl 9 Jan 03 */
void do_showmixture( CHAR_DATA* ch, const char* argument)
{
   MIX_TABLE *mix = NULL;

   if( !IS_IMMORTAL( ch ) || IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   if( !NULLSTR( argument ) && ( ( mix = get_mix( argument ) ) != NULL ) )
   {
      displaymixture( ch, mix );
      return;
   }
   else if( !NULLSTR( argument ) && ( ( mix = get_mix( argument ) ) == NULL ) )
   {
      send_to_char( "Invaild mixture-name.\r\nUse 'setmixture list' to gain a vaild name.\r\n", ch );
      return;
   }

   if( !first_mixture )
   {
      send_to_char( "There are currently no mixtures loaded.\r\n", ch );
      return;
   }

   send_to_pager( "&G[&gType&G] &G[&gName&G]\r\n", ch );
   send_to_pager( "-----------------------\r\n", ch );
   for( mix = first_mixture; mix; mix = mix->next )
      pager_printf( ch, "  %-12s &g%s&D\r\n", mix->object ? "&PObject&D" : "&BLiquid&D", mix->name );

   send_to_pager( "\r\n&gUse 'showmixture [name]' to view individual mixtures.\r\n", ch );
   send_to_pager( "&gUse 'showliquid' to view the liquidtable.\r\n&d", ch );
   return;
}

/* olc funciton for mixtures  -Nopey */
void do_setmixture( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   LIQ_TABLE *liq = NULL;

   if( !IS_IMMORTAL( ch ) || IS_NPC( ch ) )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   smash_tilde( argument );
   argument = one_argument( argument, arg );

   if( EMPTYSTR( arg ) )
   {
      send_to_char( "Syntax: setmixture create <name>\r\n"
                    "        setmixture delete <name>\r\n"
                    "        setmixture list [name]\r\n"
                    "        setmixture save - (saves table)\r\n" "        setmixture <name> <field> <value>\r\n", ch );
      send_to_char( " Fields being one of the following:\r\n" " name vnum1 vnum2 into object\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "list" ) )
   {
      MIX_TABLE *mix = NULL;

      if( !NULLSTR( argument ) && ( ( mix = get_mix( argument ) ) != NULL ) )
      {
         displaymixture( ch, mix );
         return;
      }
      else if( !NULLSTR( argument ) && ( ( mix = get_mix( argument ) ) == NULL ) )
      {
         send_to_char( "Invaild mixture-name.\r\nUse 'setmixture list' to gain a vaild name.\r\n", ch );
         return;
      }

      if( !first_mixture )
      {
         send_to_char( "There are currently no mixtures loaded.\r\n", ch );
         return;
      }

      send_to_pager( "&G[&gType&G] &G[&gName&G]\r\n", ch );
      send_to_pager( "-----------------------\r\n", ch );
      for( mix = first_mixture; mix; mix = mix->next )
         pager_printf( ch, "  %-12s &g%s&D\r\n", mix->object ? "&PObject&D" : "&BLiquid&D", mix->name );

      send_to_pager( "\r\n&gUse 'showmixture [name]' to view individual mixtures.\r\n", ch );
      send_to_pager( "&gUse 'showliquid' to view the liquidtable.&d\r\n", ch );
      return;
   }
   else if( !str_cmp( arg, "create" ) )
   {
      MIX_TABLE *mix = NULL;

      if( NULLSTR( argument ) )
      {
         send_to_char( "Syntax: setmixture create <name>\r\n", ch );
         return;
      }

      CREATE( mix, MIX_TABLE, 1 );
      mix->name = STRALLOC( argument );
      mix->data[0] = -1;
      mix->data[1] = -1;
      mix->data[2] = -1;
      mix->object = FALSE;
      LINK( mix, first_mixture, last_mixture, next, prev );
      send_to_char( "Done.\r\n", ch );
      save_mixtures(  );
      return;
   }
   else if( !str_cmp( arg, "save" ) )
   {
      save_mixtures(  );
      send_to_char( "Mixture table saved.\r\n", ch );
      return;
   }
   else if( !str_cmp( arg, "delete" ) )
   {
      MIX_TABLE *mix = NULL;

      if( NULLSTR( argument ) )
      {
         send_to_char( "Syntax: setmixture delete <name>\r\n", ch );
         return;
      }

      if( ( mix = get_mix( argument ) ) == NULL )
      {
         send_to_char( "That's not a mixture name.\r\n", ch );
         return;
      }

      UNLINK( mix, first_mixture, last_mixture, next, prev );
      STRFREE( mix->name );
      DISPOSE( mix );
      send_to_char( "Done.\r\n", ch );
      save_mixtures(  );
      return;
   }
   else
   {
      char arg2[MAX_INPUT_LENGTH];
      MIX_TABLE *mix = NULL;

      if( EMPTYSTR( arg ) || ( ( mix = get_mix( arg ) ) == NULL ) )
      {
         send_to_char( "Syntax: setmixture <mixname> <field> <value>\r\n", ch );
         send_to_char( " Fields being one of the following:\r\n" " name vnum1 vnum2 into object\r\n", ch );
         return;
      }

      argument = one_argument( argument, arg2 );

      if( !str_cmp( arg2, "name" ) )
      {
         if( NULLSTR( argument ) )
         {
            send_to_char( "Syntax: setmixture <mixname> name <name>\r\n", ch );
            return;
         }
         STRFREE( mix->name );
         mix->name = STRALLOC( argument );
      }
      else if( !str_cmp( arg2, "vnum1" ) )
      {
         int i = 0;

         if( is_number( argument ) )
         {
            i = atoi( argument );
         }
         else
         {
            send_to_char( "Invalid liquid vnum.\r\n", ch );
            send_to_char( "Syntax: setmixture <mixname> vnum1 <liqvnum or objvnum>\r\n", ch );
            return;
         }

         if( mix->object == TRUE )
         {
            OBJ_INDEX_DATA *obj = get_obj_index( i );
            if( !obj )
            {
               ch_printf( ch, "Invalid object vnum %d\r\n", i );
               return;
            }
            else
            {
               mix->data[0] = i;
               ch_printf( ch, "Mixture object set to %d - %s\r\n", i, obj->name );
            }
         }
         else
         {
            liq = get_liq_vnum( i );
            if( !liq )
            {
               ch_printf( ch, "Liquid vnum %d does not exist\r\n", i );
               return;
            }
            else
            {
               mix->data[0] = i;
               ch_printf( ch, "Mixture Vnum1 set to %s \r\n", liq->name );

            }
         }
      }
      else if( !str_cmp( arg2, "vnum2" ) )
      {
         int i = 0;

         if( is_number( argument ) )
         {
            i = atoi( argument );
         }
         else
         {
            send_to_char( "Invalid liquid vnum.\r\n", ch );
            send_to_char( "Syntax: setmixture <mixname> vnum2 <liqvnum>\r\n", ch );
            return;
         }

         //Verify liq exists
         liq = get_liq_vnum( i );
         if( !liq )
         {
            ch_printf( ch, "Liquid vnum %d does not exist\r\n", i );
            return;
         }
         else
         {
            mix->data[1] = i;
            ch_printf( ch, "Mixture Vnum2 set to %s \r\n", liq->name );
         }
      }
      else if( !str_cmp( arg2, "object" ) )
      {
         if( mix->object == FALSE )
         {
            mix->object = TRUE;
            send_to_char( "Mixture -vnum1- is now an object-vnum.\r\n", ch );
         }
         else
         {
            mix->object = FALSE;
            send_to_char( "Both mixture vnums are now liquids.\r\n", ch );
         }
      }
      else if( !str_cmp( arg2, "into" ) )
      {
         int i;

         if( is_number( argument ) )
         {
            i = atoi( argument );
         }
         else
         {
            send_to_char( "Invalid liquid vnum.\r\n", ch );
            send_to_char( "Syntax: setmixture <mixname> into <liqvnum>\r\n", ch );
            return;
         }

         liq = get_liq_vnum( i );
         if( !liq )
         {
            ch_printf( ch, "Liquid vnum %d does not exist\r\n", i );
            return;
         }
         else
         {
            mix->data[2] = i;
            ch_printf( ch, "Mixture will now turn into %s \r\n", liq->name );
         }
      }

      send_to_char( "Done.. Saving Mixtures.\r\n", ch );
      save_mixtures(  );
      return;
   }
}

/* mix a liquid with a liquid; return the final product    -Nopey */
LIQ_TABLE *liq_can_mix( OBJ_DATA * iObj, OBJ_DATA * tObj )
{
   MIX_TABLE *mix = NULL;
   bool mix_found = FALSE;

   for( mix = first_mixture; mix; mix = mix->next )
      if( mix->data[0] == iObj->value[2] || mix->data[1] == iObj->value[2] )
      {
         mix_found = TRUE;
         break;
      }

   if( !mix_found )
      return NULL;

   if( mix->data[2] > -1 )
   {
      LIQ_TABLE *liq = NULL;

      if( ( liq = get_liq_vnum( mix->data[2] ) ) == NULL )
         return NULL;
      else
      {
         iObj->value[1] += tObj->value[1];
         iObj->value[2] = liq->vnum;
         tObj->value[1] = 0;
         tObj->value[2] = -1;
         return liq;
      }
   }
   return NULL;
}

/* used to mix an object with a liquid to form another liquid; returns the result  -Nopey */
LIQ_TABLE *liqobj_can_mix( OBJ_DATA * iObj, OBJ_DATA * oLiq )
{
   MIX_TABLE *mix = NULL;
   bool mix_found = FALSE;

   for( mix = first_mixture; mix; mix = mix->next )
      if( mix->object && ( mix->data[0] == iObj->value[2] || mix->data[1] == iObj->value[2] ) )
         if( mix->data[0] == oLiq->value[2] || mix->data[1] == oLiq->value[2] )
         {
            mix_found = TRUE;
            break;
         }

   if( !mix_found )
      return NULL;

   if( mix->data[2] > -1 )
   {
      LIQ_TABLE *liq = NULL;

      if( ( liq = get_liq_vnum( mix->data[2] ) ) == NULL )
         return NULL;
      else
      {
         oLiq->value[1] += iObj->value[1];
         oLiq->value[2] = liq->vnum;
         separate_obj( iObj );
         obj_from_char( iObj );
         extract_obj( iObj );
         return liq;
      }
   }
   return NULL;
}

/* the actual -mix- funciton  -Nopey */
void do_mix( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *iObj, *tObj = NULL;

   argument = one_argument( argument, arg );
   /*
    * null arguments 
    */
   if( EMPTYSTR( arg ) || NULLSTR( argument ) )
   {
      send_to_char( "What would you like to mix together?\r\n", ch );
      return;
   }

   /*
    * check for objects in the inventory 
    */
   if( ( ( iObj = get_obj_carry( ch, arg ) ) == NULL ) || ( ( tObj = get_obj_carry( ch, argument ) ) == NULL ) )
   {
      send_to_char( "You aren't carrying that.\r\n", ch );
      return;
   }

   /*
    * check itemtypes 
    */
   if( ( iObj->item_type != ITEM_DRINK_CON && iObj->item_type != ITEM_DRINK_MIX )
       || ( tObj->item_type != ITEM_DRINK_CON && tObj->item_type != ITEM_DRINK_MIX ) )
   {
      send_to_char( "You can't mix that!\r\n", ch );
      return;
   }

   /*
    * check to see if it's empty or not 
    */
   if( iObj->value[1] <= 0 || tObj->value[1] <= 0 )
   {
      send_to_char( "It's empty.\r\n", ch );
      return;
   }

   /*
    * two liquids 
    */
   if( iObj->item_type == ITEM_DRINK_CON && tObj->item_type == ITEM_DRINK_CON )
   {
      /*
       * check to see if the two liquids can be mixed together and return the final liquid -Nopey 
       */
      if( !liq_can_mix( iObj, tObj ) )
      {
         send_to_char( "Those two don't mix well together.\r\n", ch );
         return;
      }
   }
   else if( iObj->item_type == ITEM_DRINK_MIX && tObj->item_type == ITEM_DRINK_CON )
   {
      if( !liqobj_can_mix( tObj, iObj ) )
      {
         send_to_char( "Those two don't mix well together.\r\n", ch );
         return;
      }
   }
   else if( iObj->item_type == ITEM_DRINK_CON && tObj->item_type == ITEM_DRINK_MIX )
   {
      if( !liqobj_can_mix( iObj, tObj ) )
      {
         send_to_char( "Those two don't mix well together.\r\n", ch );
         return;
      }
   }
   else
   {
      send_to_char( "Those two don't mix well together.\r\n", ch );
      return;
   }
   send_to_char( "&cYou mix them together.&g\r\n", ch );
   return;
}

/* modified do_drink function -Nopey */
void do_drink( CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   AFFECT_DATA af;
   int amount;

   argument = one_argument( argument, arg );
   /*
    * munch optional words 
    */
   if( !str_cmp( arg, "from" ) && argument[0] != '\0' )
      argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
         if( ( obj->item_type == ITEM_FOUNTAIN ) || ( obj->item_type == ITEM_BLOOD ) || ( obj->item_type == ITEM_PUDDLE ) )
            break;

      if( !obj )
      {
         send_to_char( "Drink what?\r\n", ch );
         return;
      }
   }
   else
   {
      if( ( obj = get_obj_here( ch, arg ) ) == NULL )
      {
         send_to_char( "You can't find it.\r\n", ch );
         return;
      }
   }

   if( obj->count > 1 && obj->item_type != ITEM_FOUNTAIN )
      separate_obj( obj );

   if( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > MAX_COND_VALUE - 8 )
   {
      send_to_char( "You fail to reach your mouth.  *Hic*\r\n", ch );
      return;
   }

   switch ( obj->item_type )
   {
      default:
         if( obj->carried_by == ch )
         {
            act( AT_ACTION, "$n lifts $p up to $s mouth and tries to drink from it...", ch, obj, NULL, TO_ROOM );
            act( AT_ACTION, "You bring $p up to your mouth and try to drink from it...", ch, obj, NULL, TO_CHAR );
         }
         else
         {
            act( AT_ACTION, "$n gets down and tries to drink from $p... (Is $e feeling ok?)", ch, obj, NULL, TO_ROOM );
            act( AT_ACTION, "You get down on the ground and try to drink from $p...", ch, obj, NULL, TO_CHAR );
         }
         break;

      case ITEM_BLOOD:
         if( IS_VAMPIRE( ch ) && !IS_NPC( ch ) )
         {
            if( obj->timer > 0   /* if timer, must be spilled blood */
                && ch->level > 5 && ch->pcdata->condition[COND_BLOODTHIRST] > ( 5 + ch->level / 10 ) )
            {
               send_to_char( "It is beneath you to stoop to drinking blood from the ground!\r\n", ch );
               send_to_char( "Unless in dire need, you'd much rather have blood from a victim's neck!\r\n", ch );
               return;
            }

            if( ch->pcdata->condition[COND_BLOODTHIRST] < ( 10 + ch->level ) )
            {
               if( ch->pcdata->condition[COND_FULL] >= MAX_COND_VALUE
                   || ch->pcdata->condition[COND_THIRST] >= MAX_COND_VALUE )
               {
                  send_to_char( "You are too full to drink any blood.\r\n", ch );
                  return;
               }

               if( !oprog_use_trigger( ch, obj, NULL, NULL ) )
               {
                  act( AT_BLOOD, "$n drinks from the spilled blood.", ch, NULL, NULL, TO_ROOM );
                  set_char_color( AT_BLOOD, ch );
                  send_to_char( "You relish in the replenishment of this vital fluid...\r\n", ch );
                  if( obj->value[1] <= 1 )
                  {
                     set_char_color( AT_BLOOD, ch );
                     send_to_char( "You drink the last drop of blood from the spill.\r\n", ch );
                     act( AT_BLOOD, "$n drinks the last drop of blood from the spill.", ch, NULL, NULL, TO_ROOM );
                  }
               }

               gain_condition( ch, COND_BLOODTHIRST, 1 );
               gain_condition( ch, COND_FULL, 1 );
               gain_condition( ch, COND_THIRST, 1 );
               if( --obj->value[1] <= 0 )
               {
                  if( obj->serial == cur_obj )
                     global_objcode = rOBJ_DRUNK;
                  extract_obj( obj );
                  make_bloodstain( ch );
               }
            }
            else
               send_to_char( "Alas... you cannot consume any more blood.\r\n", ch );
         }
         else
            send_to_char( "It is not in your nature to do such things.\r\n", ch );
         break;

      case ITEM_POTION:
         if( obj->carried_by == ch )
         {
            char buf[MAX_STRING_LENGTH];

            snprintf( buf, MAX_STRING_LENGTH, "quaff %s", obj->name );
            interpret( ch, buf );
         }
         else
            send_to_char( "You're not carrying that.\r\n", ch );
         break;

      case ITEM_FOUNTAIN:
      {
         LIQ_TABLE *liq = NULL;

         if( obj->value[1] <= 0 )
            obj->value[1] = MAX_COND_VALUE;

         if( ( liq = get_liq_vnum( obj->value[2] ) ) == NULL )
         {
            bug( "Do_drink: bad liquid number %d.", obj->value[2] );
            liq = get_liq_vnum( 0 );
         }

         if( !IS_NPC( ch ) && obj->value[2] != 0 )
         {
            gain_condition( ch, COND_THIRST, liq->mod[COND_THIRST] );
            gain_condition( ch, COND_FULL, liq->mod[COND_FULL] );
            gain_condition( ch, COND_DRUNK, liq->mod[COND_DRUNK] );
            if( IS_VAMPIRE( ch ) )
               gain_condition( ch, COND_BLOODTHIRST, liq->mod[COND_BLOODTHIRST] );
         }
         else if( !IS_NPC( ch ) && obj->value[2] == 0 )
            ch->pcdata->condition[COND_THIRST] = MAX_COND_VALUE;

         if( !oprog_use_trigger( ch, obj, NULL, NULL ) )
         {
            act( AT_ACTION, "$n drinks from the fountain.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You take a long thirst quenching drink.\r\n", ch );
         }
         break;
      }

      case ITEM_DRINK_CON:
      {
         LIQ_TABLE *liq = NULL;

         if( obj->value[1] <= 0 )
         {
            send_to_char( "It is already empty.\r\n", ch );
            return;
         }

         /*
          * allow water to be drank; but nothing else on a full stomach     -Nopey 
          */
         if( !IS_NPC( ch ) && ( ch->pcdata->condition[COND_THIRST] == MAX_COND_VALUE
                                || ch->pcdata->condition[COND_FULL] == MAX_COND_VALUE ) )
         {
            send_to_char( "Your stomach is too full to drink anymore!\r\n", ch );
            return;
         }

         if( ( liq = get_liq_vnum( obj->value[2] ) ) == NULL )
         {
            bug( "%s: bad liquid number %d.", __FUNCTION__, obj->value[2] );
            liq = get_liq_vnum( 0 );
         }

         if( !oprog_use_trigger( ch, obj, NULL, NULL ) )
         {
            act( AT_ACTION, "$n drinks $T from $p.", ch, obj, liq->shortdesc, TO_ROOM );
            act( AT_ACTION, "You drink $T from $p.", ch, obj, liq->shortdesc, TO_CHAR );
         }

         amount = 1;

         /*
          * gain conditions accordingly              -Nopey 
          */
         gain_condition( ch, COND_DRUNK, liq->mod[COND_DRUNK] );
         gain_condition( ch, COND_FULL, liq->mod[COND_FULL] );
         gain_condition( ch, COND_THIRST, liq->mod[COND_THIRST] );

         if( IS_VAMPIRE( ch ) )
            gain_condition( ch, COND_BLOODTHIRST, liq->mod[COND_BLOODTHIRST] );

         if( liq->type == LIQTYPE_POISON )
         {
            act( AT_POISON, "$n sputters and gags.", ch, NULL, NULL, TO_ROOM );
            act( AT_POISON, "You sputter and gag.", ch, NULL, NULL, TO_CHAR );
            ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
            af.type = gsn_poison;
            af.duration = obj->value[3];
            af.location = APPLY_NONE;
            af.modifier = 0;
            af.bitvector = meb( AFF_POISON );
            affect_join( ch, &af );
         }

         if( !IS_NPC( ch ) )
         {
            if( ch->pcdata->condition[COND_DRUNK] > ( MAX_COND_VALUE / 2 )
                && ch->pcdata->condition[COND_DRUNK] < ( MAX_COND_VALUE * .4 ) )
               send_to_char( "You feel quite sloshed.\r\n", ch );
            else if( ch->pcdata->condition[COND_DRUNK] >= ( MAX_COND_VALUE * .4 )
                     && ch->pcdata->condition[COND_DRUNK] < ( MAX_COND_VALUE * .6 ) )
               send_to_char( "You start to feel a little drunk.\r\n", ch );
            else if( ch->pcdata->condition[COND_DRUNK] >= ( MAX_COND_VALUE * .6 )
                     && ch->pcdata->condition[COND_DRUNK] < ( MAX_COND_VALUE * .9 ) )
               send_to_char( "Your vision starts to get blurry.\r\n", ch );
            else if( ch->pcdata->condition[COND_DRUNK] >= ( MAX_COND_VALUE * .9 )
                     && ch->pcdata->condition[COND_DRUNK] < MAX_COND_VALUE )
               send_to_char( "You feel very drunk.\r\n", ch );
            else if( ch->pcdata->condition[COND_DRUNK] == MAX_COND_VALUE )
               send_to_char( "You feel like your going to pass out.\r\n", ch );

            if( ch->pcdata->condition[COND_THIRST] > ( MAX_COND_VALUE / 2 )
                && ch->pcdata->condition[COND_THIRST] < ( MAX_COND_VALUE * .4 ) )
               send_to_char( "Your stomach begins to slosh around.\r\n", ch );
            else if( ch->pcdata->condition[COND_THIRST] >= ( MAX_COND_VALUE * .4 )
                     && ch->pcdata->condition[COND_THIRST] < ( MAX_COND_VALUE * .6 ) )
               send_to_char( "You start to feel bloated.\r\n", ch );
            else if( ch->pcdata->condition[COND_THIRST] >= ( MAX_COND_VALUE * .6 )
                     && ch->pcdata->condition[COND_THIRST] < ( MAX_COND_VALUE * .9 ) )
               send_to_char( "You feel bloated.\r\n", ch );
            else if( ch->pcdata->condition[COND_THIRST] >= ( MAX_COND_VALUE * .9 )
                     && ch->pcdata->condition[COND_THIRST] < MAX_COND_VALUE )
               send_to_char( "You stomach is almost filled to it's brim!\r\n", ch );
            else if( ch->pcdata->condition[COND_THIRST] == MAX_COND_VALUE )
               send_to_char( "Your stomach is full, you can't manage to get anymore down.\r\n", ch );

            /*
             * Hopefully this is the reason why that crap was happening. =0P 
             */
            if( IS_VAMPIRE( ch ) )
            {
               if( ch->pcdata->condition[COND_BLOODTHIRST] > ( MAX_COND_VALUE / 2 )
                   && ch->pcdata->condition[COND_BLOODTHIRST] < ( MAX_COND_VALUE * .4 ) )
                  send_to_char( "&rYou replenish your body with the vidal fluid.\r\n", ch );
               else if( ch->pcdata->condition[COND_BLOODTHIRST] >= ( MAX_COND_VALUE * .4 )
                        && ch->pcdata->condition[COND_BLOODTHIRST] < ( MAX_COND_VALUE * .6 ) )
                  send_to_char( "&rYour thirst for blood begins to decrease.\r\n", ch );
               else if( ch->pcdata->condition[COND_BLOODTHIRST] >= ( MAX_COND_VALUE * .6 )
                        && ch->pcdata->condition[COND_BLOODTHIRST] < ( MAX_COND_VALUE * .9 ) )
                  send_to_char( "&rThe thirst for blood begins to leave you...\r\n", ch );
               else if( ch->pcdata->condition[COND_BLOODTHIRST] >= ( MAX_COND_VALUE * .9 )
                        && ch->pcdata->condition[COND_BLOODTHIRST] < MAX_COND_VALUE )
                  send_to_char( "&rYou drink the last drop of the fluid, the thirst for more leaves your body.\r\n", ch );
            }
            else if( !IS_VAMPIRE( ch ) && ch->pcdata->condition[COND_BLOODTHIRST] >= MAX_COND_VALUE )
            {
               ch->pcdata->condition[COND_BLOODTHIRST] = MAX_COND_VALUE;
            }
         }

         obj->value[1] -= amount;
         if( obj->value[1] <= 0 )   /* Come now, what good is a drink container that vanishes?? */
         {
            obj->value[1] = 0;   /* Prevents negative values - Samson */
            send_to_char( "You drink the last drop from your container.\r\n", ch );
         }
         break;
      }

      case ITEM_PUDDLE: 
      {
         LIQ_TABLE *liq = NULL;

         if( obj->value[1] <= 0 )
         { 
            bug( "%s: empty puddle %d.", __FUNCTION__, obj->in_room->vnum ); 
            return; 
         } 

         if( ( liq = get_liq_vnum( obj->value[2] ) ) == NULL )
         { 
            bug( "%s: bad liquid number %d.", __FUNCTION__, obj->value[2] );
            liq = get_liq_vnum( 0 );
         }

         if( !oprog_use_trigger( ch, obj, NULL, NULL ) )
         { 
            act( AT_ACTION, "$n stoops to the ground and drinks from $p.", ch, obj, NULL, TO_ROOM );
            act( AT_ACTION, "You stoop to the ground and drink $T from $p.", ch, obj, liq->name, TO_CHAR );
         }

         amount = 1;

         gain_condition( ch, COND_DRUNK, liq->mod[COND_DRUNK] );
         gain_condition( ch, COND_FULL, liq->mod[COND_FULL] );
         gain_condition( ch, COND_THIRST, liq->mod[COND_THIRST] );

         if( IS_VAMPIRE( ch ) )
            gain_condition( ch, COND_BLOODTHIRST, liq->mod[COND_BLOODTHIRST] );

         if( liq->type == LIQTYPE_POISON )
         {
            act( AT_POISON, "$n sputters and gags.", ch, NULL, NULL, TO_ROOM );
            act( AT_POISON, "You sputter and gag.", ch, NULL, NULL, TO_CHAR );
            ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
            af.type = gsn_poison;
            af.duration = obj->value[3];
            af.location = APPLY_NONE;
            af.modifier = 0;
            af.bitvector = meb( AFF_POISON );
            affect_join( ch, &af );
         }

         if( !IS_NPC( ch ) )
         {
            if( ch->pcdata->condition[COND_DRUNK] > ( MAX_COND_VALUE / 2 )
                && ch->pcdata->condition[COND_DRUNK] < ( MAX_COND_VALUE * .4 ) )
               send_to_char( "You feel quite sloshed.\r\n", ch );
            else if( ch->pcdata->condition[COND_DRUNK] >= ( MAX_COND_VALUE * .4 )
                     && ch->pcdata->condition[COND_DRUNK] < ( MAX_COND_VALUE * .6 ) )
               send_to_char( "You start to feel a little drunk.\r\n", ch );
            else if( ch->pcdata->condition[COND_DRUNK] >= ( MAX_COND_VALUE * .6 )
                     && ch->pcdata->condition[COND_DRUNK] < ( MAX_COND_VALUE * .9 ) )
               send_to_char( "Your vision starts to get blurry.\r\n", ch );
            else if( ch->pcdata->condition[COND_DRUNK] >= ( MAX_COND_VALUE * .9 )
                     && ch->pcdata->condition[COND_DRUNK] < MAX_COND_VALUE )
               send_to_char( "You feel very drunk.\r\n", ch );
            else if( ch->pcdata->condition[COND_DRUNK] == MAX_COND_VALUE )
               send_to_char( "You feel like your going to pass out.\r\n", ch );

            if( ch->pcdata->condition[COND_THIRST] > ( MAX_COND_VALUE / 2 )
                && ch->pcdata->condition[COND_THIRST] < ( MAX_COND_VALUE * .4 ) )
               send_to_char( "Your stomach begins to slosh around.\r\n", ch );
            else if( ch->pcdata->condition[COND_THIRST] >= ( MAX_COND_VALUE * .4 )
                     && ch->pcdata->condition[COND_THIRST] < ( MAX_COND_VALUE * .6 ) )
               send_to_char( "You start to feel bloated.\r\n", ch );
            else if( ch->pcdata->condition[COND_THIRST] >= ( MAX_COND_VALUE * .6 )
                     && ch->pcdata->condition[COND_THIRST] < ( MAX_COND_VALUE * .9 ) )
               send_to_char( "You feel bloated.\r\n", ch );
            else if( ch->pcdata->condition[COND_THIRST] >= ( MAX_COND_VALUE * .9 )
                     && ch->pcdata->condition[COND_THIRST] < MAX_COND_VALUE )
               send_to_char( "You stomach is almost filled to it's brim!\r\n", ch );
            else if( ch->pcdata->condition[COND_THIRST] == MAX_COND_VALUE )
               send_to_char( "Your stomach is full, you can't manage to get anymore down.\r\n", ch );

            /*
             * Hopefully this is the reason why that crap was happening. =0P 
             */
            if( IS_VAMPIRE( ch ) )
            {
               if( ch->pcdata->condition[COND_BLOODTHIRST] > ( MAX_COND_VALUE / 2 )
                   && ch->pcdata->condition[COND_BLOODTHIRST] < ( MAX_COND_VALUE * .4 ) )
                  send_to_char( "&rYou replenish your body with the vidal fluid.\r\n", ch );
               else if( ch->pcdata->condition[COND_BLOODTHIRST] >= ( MAX_COND_VALUE * .4 )
                        && ch->pcdata->condition[COND_BLOODTHIRST] < ( MAX_COND_VALUE * .6 ) )
                  send_to_char( "&rYour thirst for blood begins to decrease.\r\n", ch );
               else if( ch->pcdata->condition[COND_BLOODTHIRST] >= ( MAX_COND_VALUE * .6 )
                        && ch->pcdata->condition[COND_BLOODTHIRST] < ( MAX_COND_VALUE * .9 ) )
                  send_to_char( "&rThe thirst for blood begins to leave you...\r\n", ch );
               else if( ch->pcdata->condition[COND_BLOODTHIRST] >= ( MAX_COND_VALUE * .9 )
                        && ch->pcdata->condition[COND_BLOODTHIRST] < MAX_COND_VALUE )
                  send_to_char( "&rYou drink the last drop of the fluid, the thirst for more leaves your body.\r\n", ch );
            }
            else if( !IS_VAMPIRE( ch ) && ch->pcdata->condition[COND_BLOODTHIRST] >= MAX_COND_VALUE )
            {
               ch->pcdata->condition[COND_BLOODTHIRST] = MAX_COND_VALUE;
            }
         }

         obj->value[1] -= amount;
         if( obj->value[1] <= 0 )
         {
            send_to_char( "The remainder of the puddle seeps into the ground.\r\n", ch );
            if( cur_obj == obj->serial )
               global_objcode = rOBJ_DRUNK;
            extract_obj( obj );
         }
         break;
      }
   }

   if( who_fighting( ch ) && IS_PKILL( ch ) )
      WAIT_STATE( ch, PULSE_PER_SECOND / 3 );
   else
      WAIT_STATE( ch, PULSE_PER_SECOND );
   return;
}

/* standard liquid functions           -Nopey */
void do_fill( CHAR_DATA* ch, const char* argument)
{
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   OBJ_DATA *source;
   short dest_item, src_item1, src_item2, src_item3;
   int diff = 0;
   bool all = FALSE;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   /*
    * munch optional words 
    */
   if( ( !str_cmp( arg2, "from" ) || !str_cmp( arg2, "with" ) ) && argument[0] != '\0' )
      argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Fill what?\r\n", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
   {
      send_to_char( "You do not have that item.\r\n", ch );
      return;
   }
   else
      dest_item = obj->item_type;

   src_item1 = src_item2 = src_item3 = -1;
   switch ( dest_item )
   {
      default:
         act( AT_ACTION, "$n tries to fill $p... (Don't ask me how)", ch, obj, NULL, TO_ROOM );
         send_to_char( "You cannot fill that.\r\n", ch );
         return;
         /*
          * place all fillable item types here 
          */
      case ITEM_DRINK_CON:
         src_item1 = ITEM_FOUNTAIN;
         src_item2 = ITEM_BLOOD;
         src_item3 = ITEM_PUDDLE;
         break;
      case ITEM_HERB_CON:
         src_item1 = ITEM_HERB;
         src_item2 = ITEM_HERB_CON;
         break;
      case ITEM_PIPE:
         src_item1 = ITEM_HERB;
         src_item2 = ITEM_HERB_CON;
         break;
      case ITEM_CONTAINER:
         src_item1 = ITEM_CONTAINER;
         src_item2 = ITEM_CORPSE_NPC;
         src_item3 = ITEM_CORPSE_PC;
         break;
   }

   if( dest_item == ITEM_CONTAINER )
   {
      if( IS_SET( obj->value[1], CONT_CLOSED ) )
      {
         act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
         return;
      }
      if( get_real_obj_weight( obj ) / obj->count >= obj->value[0] )
      {
         send_to_char( "It's already full as it can be.\r\n", ch );
         return;
      }
   }
   else
   {
      diff = MAX_COND_VALUE;
      if( diff < 1 || obj->value[1] >= obj->value[0] )
      {
         send_to_char( "It's already full as it can be.\r\n", ch );
         return;
      }
   }

   if( dest_item == ITEM_PIPE && IS_SET( obj->value[3], PIPE_FULLOFASH ) )
   {
      send_to_char( "It's full of ashes, and needs to be emptied first.\r\n", ch );
      return;
   }

   if( arg2[0] != '\0' )
   {
      if( dest_item == ITEM_CONTAINER && ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) ) )
      {
         all = TRUE;
         source = NULL;
      }
      /*
       * This used to let you fill a pipe from an object on the ground.  Seems
       * to me you should be holding whatever you want to fill a pipe with.
       * It's nitpicking, but I needed to change it to get a mobprog to work
       * right.  Check out Lord Fitzgibbon if you're curious.  -Narn 
       */
      else if( dest_item == ITEM_PIPE )
      {
         if( ( source = get_obj_carry( ch, arg2 ) ) == NULL )
         {
            send_to_char( "You don't have that item.\r\n", ch );
            return;
         }
         if( source->item_type != src_item1 && source->item_type != src_item2 && source->item_type != src_item3 )
         {
            act( AT_PLAIN, "You cannot fill $p with $P!", ch, obj, source, TO_CHAR );
            return;
         }
      }
      else
      {
         if( ( source = get_obj_here( ch, arg2 ) ) == NULL )
         {
            send_to_char( "You cannot find that item.\r\n", ch );
            return;
         }
      }
   }
   else
      source = NULL;

   if( !source && dest_item == ITEM_PIPE )
   {
      send_to_char( "Fill it with what?\r\n", ch );
      return;
   }

   if( !source )
   {
      bool found = FALSE;
      OBJ_DATA *src_next;

      separate_obj( obj );
      for( source = ch->in_room->first_content; source; source = src_next )
      {
         src_next = source->next_content;
         if( dest_item == ITEM_CONTAINER )
         {
            if( !CAN_WEAR( source, ITEM_TAKE ) || IS_OBJ_STAT( source, ITEM_BURIED )
                || ( IS_OBJ_STAT( source, ITEM_PROTOTYPE ) && !can_take_proto( ch ) )
                || ch->carry_weight + get_obj_weight( source ) > can_carry_w( ch )
                || ( get_real_obj_weight( source ) + get_real_obj_weight( obj ) / obj->count ) > obj->value[0] )
               continue;

            if( all && arg2[3] == '.' && !nifty_is_name( &arg2[4], source->name ) )
               continue;

            obj_from_room( source );
            if( source->item_type == ITEM_MONEY )
            {
               ch->gold += source->value[0];
               extract_obj( source );
            }
            else
               obj_to_obj( source, obj );
            found = TRUE;
         }
         else if( source->item_type == src_item1 || source->item_type == src_item2 || source->item_type == src_item3 )
         {
            found = TRUE;
            break;
         }
      }
      if( !found )
      {
         switch ( src_item1 )
         {
            default:
               send_to_char( "There is nothing appropriate here!\r\n", ch );
               return;
            case ITEM_FOUNTAIN:
               send_to_char( "There is no fountain, pool, or puddle here!\r\n", ch );
               return;
            case ITEM_BLOOD:
               send_to_char( "There is no blood pool here!\r\n", ch );
               return;
            case ITEM_HERB_CON:
               send_to_char( "There are no herbs here!\r\n", ch );
               return;
            case ITEM_HERB:
               send_to_char( "You cannot find any smoking herbs.\r\n", ch );
               return;
         }
      }
      if( dest_item == ITEM_CONTAINER )
      {
         act( AT_ACTION, "You fill $p.", ch, obj, NULL, TO_CHAR );
         act( AT_ACTION, "$n fills $p.", ch, obj, NULL, TO_ROOM );
         if( xIS_SET( ch->in_room->room_flags, ROOM_HOUSE ) )
            save_house_by_vnum( ch->in_room->vnum );
         return;
      }
   }

   if( dest_item == ITEM_CONTAINER )
   {
      OBJ_DATA *otmp, *otmp_next;
      char name[MAX_INPUT_LENGTH];
      CHAR_DATA *gch;
      const char *pd;
      bool found = FALSE;

      if( source == obj )
      {
         send_to_char( "You can't fill something with itself!\r\n", ch );
         return;
      }

      switch ( source->item_type )
      {
         default:   /* put something in container */
            if( !source->in_room /* disallow inventory items */
                || !CAN_WEAR( source, ITEM_TAKE ) || ( IS_OBJ_STAT( source, ITEM_PROTOTYPE ) && !can_take_proto( ch ) )
                || ch->carry_weight + get_obj_weight( source ) > can_carry_w( ch )
                || ( get_real_obj_weight( source ) + get_real_obj_weight( obj ) / obj->count ) > obj->value[0] )
            {
               send_to_char( "You can't do that.\r\n", ch );
               return;
            }
            separate_obj( obj );
            act( AT_ACTION, "You take $P and put it inside $p.", ch, obj, source, TO_CHAR );
            act( AT_ACTION, "$n takes $P and puts it inside $p.", ch, obj, source, TO_ROOM );
            obj_from_room( source );
            obj_to_obj( source, obj );
            break;

         case ITEM_MONEY:
            send_to_char( "You can't do that... yet.\r\n", ch );
            break;

         case ITEM_CORPSE_PC:
            if( IS_NPC( ch ) )
            {
               send_to_char( "You can't do that.\r\n", ch );
               return;
            }
            if( IS_OBJ_STAT( source, ITEM_CLANCORPSE ) && !IS_IMMORTAL( ch ) )
            {
               send_to_char( "Your hands fumble.  Maybe you better loot a different way.\r\n", ch );
               return;
            }
            if( !IS_OBJ_STAT( source, ITEM_CLANCORPSE ) || !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
            {
               pd = source->short_descr;
               pd = one_argument( pd, name );
               pd = one_argument( pd, name );
               pd = one_argument( pd, name );
               pd = one_argument( pd, name );

               if( str_cmp( name, ch->name ) && !IS_IMMORTAL( ch ) )
               {
                  bool fGroup;

                  fGroup = FALSE;
                  for( gch = first_char; gch; gch = gch->next )
                  {
                     if( !IS_NPC( gch ) && is_same_group( ch, gch ) && !str_cmp( name, gch->name ) )
                     {
                        fGroup = TRUE;
                        break;
                     }
                  }
                  if( !fGroup )
                  {
                     send_to_char( "That's someone else's corpse.\r\n", ch );
                     return;
                  }
               }
            }

         case ITEM_CONTAINER:
            if( source->item_type == ITEM_CONTAINER /* don't remove */  && IS_SET( source->value[1], CONT_CLOSED ) )
            {
               act( AT_PLAIN, "The $d is closed.", ch, NULL, source->name, TO_CHAR );
               return;
            }

         case ITEM_CORPSE_NPC:
            if( ( otmp = source->first_content ) == NULL )
            {
               send_to_char( "It's empty.\r\n", ch );
               return;
            }
            separate_obj( obj );
            for( ; otmp; otmp = otmp_next )
            {
               otmp_next = otmp->next_content;

               if( !CAN_WEAR( otmp, ITEM_TAKE ) || ( IS_OBJ_STAT( otmp, ITEM_PROTOTYPE ) && !can_take_proto( ch ) )
                   || ch->carry_number + otmp->count > can_carry_n( ch )
                   || ch->carry_weight + get_obj_weight( otmp ) > can_carry_w( ch )
                   || ( get_real_obj_weight( source ) + get_real_obj_weight( obj ) / obj->count ) > obj->value[0] )
                  continue;

               obj_from_obj( otmp );
               obj_to_obj( otmp, obj );
               found = TRUE;
            }
            if( found )
            {
               if( xIS_SET( ch->in_room->room_flags, ROOM_HOUSE ) )
                  save_house_by_vnum( ch->in_room->vnum );
               act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
               act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
            }
            else
               send_to_char( "There is nothing appropriate in there.\r\n", ch );
            break;
      }
      return;
   }

   if( source->value[1] < 1 )
   {
      send_to_char( "There's none left!\r\n", ch );
      return;
   }
   if( source->count > 1 && source->item_type != ITEM_FOUNTAIN )
      separate_obj( source );
   separate_obj( obj );

   switch ( source->item_type )
   {
      default:
         bug( "%s: got bad item type: %d", __FUNCTION__, source->item_type );
         send_to_char( "Something went wrong...\r\n", ch );
         return;

      case ITEM_FOUNTAIN:
         if( obj->value[1] != 0 && obj->value[2] != 0 )
         {
            send_to_char( "There is already another liquid in it.\r\n", ch );
            return;
         }
         obj->value[2] = 0;
         obj->value[1] = obj->value[0];
         act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
         act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
         return;

      case ITEM_BLOOD:
         if( obj->value[1] != 0 && obj->value[2] != 13 )
         {
            send_to_char( "There is already another liquid in it.\r\n", ch );
            return;
         }
         obj->value[2] = 13;
         if( source->value[1] < diff )
            diff = source->value[1];
         obj->value[1] += diff;
         act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
         act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
         if( ( source->value[1] -= diff ) < 1 )
         {
            extract_obj( source );
            make_bloodstain( ch );
         }
         return;

      case ITEM_HERB:
         if( obj->value[1] != 0 && obj->value[2] != source->value[2] )
         {
            send_to_char( "There is already another type of herb in it.\r\n", ch );
            return;
         }
         obj->value[2] = source->value[2];
         if( source->value[1] < diff )
            diff = source->value[1];
         obj->value[1] += diff;
         act( AT_ACTION, "You fill $p with $P.", ch, obj, source, TO_CHAR );
         act( AT_ACTION, "$n fills $p with $P.", ch, obj, source, TO_ROOM );
         if( ( source->value[1] -= diff ) < 1 )
            extract_obj( source );
         return;

      case ITEM_HERB_CON:
         if( obj->value[1] != 0 && obj->value[2] != source->value[2] )
         {
            send_to_char( "There is already another type of herb in it.\r\n", ch );
            return;
         }
         obj->value[2] = source->value[2];
         if( source->value[1] < diff )
            diff = source->value[1];
         obj->value[1] += diff;
         source->value[1] -= diff;
         act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
         act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
         return;

      case ITEM_DRINK_CON:
         if( obj->value[1] != 0 && obj->value[2] != source->value[2] )
         {
            send_to_char( "There is already another liquid in it.\r\n", ch );
            return;
         }
         obj->value[2] = source->value[2];
         if( source->value[1] < diff )
            diff = source->value[1];
         obj->value[1] += diff;
         source->value[1] -= diff;
         act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
         act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
         return;

      case ITEM_PUDDLE:
         if( obj->value[1] != 0 && obj->value[2] != source->value[2] )
         {
            send_to_char( "There is already another liquid in it.\r\n", ch );
            return;
         }
         obj->value[2] = source->value[2];

         if( source->value[1] < diff )
	         diff = source->value[1];

         obj->value[1] += diff;
         source->value[1] -= diff;

         if( source->item_type == ITEM_PUDDLE )
         {
            char buf[20];
            char buf2[70];

            if( source->value[1] > 15 )
               mudstrlcpy( buf, "large", 20 );
            else if( source->value[1] > 10 )
               mudstrlcpy( buf, "rather large", 20 );
            else if( source->value[1] > 5 )
               mudstrlcpy( buf, "rather small", 20 );
            else
               mudstrlcpy( buf, "small", 20 );
            snprintf( buf2, 70, "There is a %s puddle of %s.", buf,
               ( source->value[2] >= LIQ_MAX ? "water" : liq_table[source->value[2]].liq_name ) );
            source->description = STRALLOC( buf2 );
         }
         act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
         act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );

         if( source->value[1] < 1 )
         {
            act( AT_ACTION, "The remaining contents of the puddle seep into the ground.", ch, NULL, NULL, TO_CHAR );
            act( AT_ACTION, "The remaining contents of the puddle seep into the ground.",ch, NULL, NULL, TO_ROOM );
            extract_obj( source );
         }
         return;
   }
}

void do_empty( CHAR_DATA* ch, const char* argument)
{
   OBJ_DATA *obj;
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( !str_cmp( arg2, "into" ) && argument[0] != '\0' )
      argument = one_argument( argument, arg2 );

   if( EMPTYSTR(arg1) )
   {
      send_to_char( "Empty what?\r\n", ch );
      return;
   }
   if( ms_find_obj( ch ) )
      return;

   if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
   {
      send_to_char( "You aren't carrying that.\r\n", ch );
      return;
   }
   if( obj->count > 1 )
      separate_obj( obj );

   switch ( obj->item_type )
   {
      default:
         act( AT_ACTION, "You shake $p in an attempt to empty it...", ch, obj, NULL, TO_CHAR );
         act( AT_ACTION, "$n begins to shake $p in an attempt to empty it...", ch, obj, NULL, TO_ROOM );
         return;

      case ITEM_PIPE:
         act( AT_ACTION, "You gently tap $p and empty it out.", ch, obj, NULL, TO_CHAR );
         act( AT_ACTION, "$n gently taps $p and empties it out.", ch, obj, NULL, TO_ROOM );
         REMOVE_BIT( obj->value[3], PIPE_FULLOFASH );
         REMOVE_BIT( obj->value[3], PIPE_LIT );
         obj->value[1] = 0;
         return;

      case ITEM_DRINK_CON:
         if( obj->value[1] < 1 )
         {
            send_to_char( "It's already empty.\r\n", ch );
            return;
         }
         make_puddle( ch, obj );
         act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
         act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
         obj->value[1] = 0;
         return;

      case ITEM_CONTAINER:
      case ITEM_QUIVER:
         if( IS_SET( obj->value[1], CONT_CLOSED ) )
         {
            act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
            return;
         }

      case ITEM_KEYRING:
         if( !obj->first_content )
         {
            send_to_char( "It's already empty.\r\n", ch );
            return;
         }
         if( EMPTYSTR(arg2) )
         {
            if( xIS_SET( ch->in_room->room_flags, ROOM_NODROP ) || xIS_SET( ch->act, PLR_LITTERBUG ) )
            {
               send_to_char( "&[magic]A magical force stops you!\r\n", ch );
               send_to_char( "&[tell]Someone tells you, 'No littering here!'\r\n", ch );
               return;
            }
            if( xIS_SET( ch->in_room->room_flags, ROOM_NODROPALL )
                || xIS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) )
            {
               send_to_char( "You can't seem to do that here...\r\n", ch );
               return;
            }
            if( empty_obj( obj, NULL, ch->in_room ) )
            {
               act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
               act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
               if( IS_SET( sysdata.save_flags, SV_EMPTY ) )
                  save_char_obj( ch );
            }
            else
               send_to_char( "Hmmm... didn't work.\r\n", ch );
         }
         else
         {
            OBJ_DATA *dest = get_obj_here( ch, arg2 );

            if( !dest )
            {
               send_to_char( "You can't find it.\r\n", ch );
               return;
            }
            if( dest == obj )
            {
               send_to_char( "You can't empty something into itself!\r\n", ch );
               return;
            }
            if( dest->item_type != ITEM_CONTAINER && dest->item_type != ITEM_KEYRING && dest->item_type != ITEM_QUIVER )
            {
               send_to_char( "That's not a container!\r\n", ch );
               return;
            }
            if( IS_SET( dest->value[1], CONT_CLOSED ) )
            {
               act( AT_PLAIN, "The $d is closed.", ch, NULL, dest->name, TO_CHAR );
               return;
            }
            separate_obj( dest );
            if( empty_obj( obj, dest, NULL ) )
            {
               act( AT_ACTION, "You empty $p into $P.", ch, obj, dest, TO_CHAR );
               act( AT_ACTION, "$n empties $p into $P.", ch, obj, dest, TO_ROOM );
               if( !dest->carried_by && IS_SET( sysdata.save_flags, SV_EMPTY ) )
                  save_char_obj( ch );
            }
            else
               act( AT_ACTION, "$P is too full.", ch, obj, dest, TO_CHAR );
         }
         if( ch->in_room && xIS_SET( ch->in_room->room_flags, ROOM_HOUSE ) )
            save_house_by_vnum( ch->in_room->vnum );

         if( xIS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) )
         {
            VAULT_DATA *vault;

            for( vault = first_vault; vault; vault = vault->next )
               if( vault->vnum == ch->in_room->vnum )
                  save_storeroom( ch, vault->vnum );
         }
         return;
   }
}

void free_liquiddata( void )
{
   MIX_TABLE *mix, *mix_next;
   LIQ_TABLE *liq;
   int loopa;

   for( mix = first_mixture; mix; mix = mix_next )
   {
      mix_next = mix->next;
      UNLINK( mix, first_mixture, last_mixture, next, prev );
      STRFREE( mix->name );
      DISPOSE( mix );
   }
   for( loopa = 0; loopa <= top_liquid; loopa++ )
   {
      liq = get_liq_vnum( loopa );
      STRFREE( liq->name );
      STRFREE( liq->color );
      STRFREE( liq->shortdesc );
      DISPOSE( liq );
   }
   return;
}
