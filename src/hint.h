/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
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
 *                 Online Hints module by Zedd of Slips in Time             *
 ****************************************************************************/

#define HINT_FILE       SYSTEM_DIR "hints.txt"  /* For Hints */
#define HINT_UPDATEFREQ         1

DECLARE_DO_FUN( do_hintedit );

typedef struct hint_data HINT_DATA;
struct hint_data
{
   HINT_DATA *next;
   HINT_DATA *prev;
   char *text;
   int low;
   int high;
};

extern HINT_DATA *hint;
extern HINT_DATA *first_hint;
extern HINT_DATA *last_hint;

char *get_hint( int level );
void hint_update( void );
