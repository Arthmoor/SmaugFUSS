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
 * Win32 port by Nick Gammon                                                *
 * ------------------------------------------------------------------------ *
 *			    Main mud header file			    *
 ****************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/* force the who command to require an argument (should use cset) */
#define REQWHOARG

#ifdef WIN32
  #include <winsock.h>
  #include <sys/types.h>
  #pragma warning( disable: 4018 4244 4761)
  #define NOCRYPT
  #define index strchr
  #define rindex strrchr
#else
  #include <unistd.h>
#ifndef SYSV
  #include <sys/cdefs.h>
#else
  #include <re_comp.h>
#endif
  #include <sys/time.h>
#endif

typedef	int				ch_ret;
typedef	int				obj_ret;

/* 
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )	ch_ret fun( )
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#endif


/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	!defined(BERR)
#define BERR	 255
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int				sh_int;
typedef int				bool;
#define unix
#else
typedef short    int			sh_int;
typedef unsigned char			bool;
#endif

/*
 * Structure types.
 */
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct  auction_data            AUCTION_DATA;	/* auction data */
typedef struct	watch_data		WATCH_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct	extracted_char_data	EXTRACT_CHAR_DATA;
typedef struct	char_data		CHAR_DATA;
typedef struct	hunt_hate_fear		HHF_DATA;
typedef struct	fighting_data		FIGHT_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	menu_data		MENU_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct 	char_morph 		CHAR_MORPH;
typedef struct  morph_data 	        MORPH_DATA;
typedef struct  nuisance_data		NUISANCE_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct	comment_data		COMMENT_DATA;
typedef struct	board_data		BOARD_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef	struct	plane_data		PLANE_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	map_index_data		MAP_INDEX_DATA;		/* maps */
typedef struct	map_data		MAP_DATA;		/* maps */
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	race_type		RACE_TYPE;
typedef struct	repairshop_data		REPAIR_DATA;
typedef struct	reserve_data		RESERVE_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	hour_min_sec		HOUR_MIN_SEC;
typedef struct	weather_data		WEATHER_DATA;
typedef struct	neighbor_data		NEIGHBOR_DATA; /* FB */
typedef	struct	clan_data		CLAN_DATA;
typedef struct  council_data 		COUNCIL_DATA;
typedef struct  tourney_data            TOURNEY_DATA;
typedef struct	mob_prog_data		MPROG_DATA;
typedef struct	mob_prog_act_list	MPROG_ACT_LIST;
typedef	struct	editor_data		EDITOR_DATA;
typedef struct	teleport_data		TELEPORT_DATA;
typedef struct	timer_data		TIMER;
typedef struct  godlist_data		GOD_DATA;
typedef struct	system_data		SYSTEM_DATA;
typedef	struct	smaug_affect		SMAUG_AFF;
typedef struct  who_data                WHO_DATA;
typedef	struct	skill_type		SKILLTYPE;
typedef	struct	social_type		SOCIALTYPE;
typedef	struct	cmd_type		CMDTYPE;
typedef	struct	killed_data		KILLED_DATA;
typedef struct  deity_data		DEITY_DATA;
typedef struct	wizent			WIZENT;
typedef struct  ignore_data		IGNORE_DATA;
typedef struct  immortal_host           IMMORTAL_HOST;
typedef struct	project_data		PROJECT_DATA;
typedef struct	extended_bitvector	EXT_BV;
typedef	struct	lcnv_data		LCNV_DATA;
typedef	struct	lang_data		LANG_DATA;



/*
 * Function types.
 */
typedef	void	DO_FUN		args( ( CHAR_DATA *ch, char *argument ) );
typedef bool	SPEC_FUN	args( ( CHAR_DATA *ch ) );
typedef ch_ret	SPELL_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo ) );

#define DUR_CONV	23.333333333333333333333333
#define HIDDEN_TILDE	'*'

/* 32bit bitvector defines */
#define BV00		(1 <<  0)
#define BV01		(1 <<  1)
#define BV02		(1 <<  2)
#define BV03		(1 <<  3)
#define BV04		(1 <<  4)
#define BV05		(1 <<  5)
#define BV06		(1 <<  6)
#define BV07		(1 <<  7)
#define BV08		(1 <<  8)
#define BV09		(1 <<  9)
#define BV10		(1 << 10)
#define BV11		(1 << 11)
#define BV12		(1 << 12)
#define BV13		(1 << 13)
#define BV14		(1 << 14)
#define BV15		(1 << 15)
#define BV16		(1 << 16)
#define BV17		(1 << 17)
#define BV18		(1 << 18)
#define BV19		(1 << 19)
#define BV20		(1 << 20)
#define BV21		(1 << 21)
#define BV22		(1 << 22)
#define BV23		(1 << 23)
#define BV24		(1 << 24)
#define BV25		(1 << 25)
#define BV26		(1 << 26)
#define BV27		(1 << 27)
#define BV28		(1 << 28)
#define BV29		(1 << 29)
#define BV30		(1 << 30)
#define BV31		(1 << 31)
/* 32 USED! DO NOT ADD MORE! SB */

/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH		 2048
#define MAX_STRING_LENGTH	 4096  /* buf */
#define MAX_INPUT_LENGTH	 1024  /* arg */
#define MAX_INBUF_SIZE		 1024

#define HASHSTR			 /* use string hashing */

#define	MAX_LAYERS		 8	/* maximum clothing layers */
#define MAX_NEST	       100	/* maximum container nesting */

#define MAX_KILLTRACK		25	/* track mob vnums killed */

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_EXP_WORTH	       500000
#define MIN_EXP_WORTH		   20

#define MAX_REXITS		   20	/* Maximum exits allowed in 1 room */
#define MAX_SKILL		  500
#define SPELL_SILENT_MARKER   "silent"	/* No OK. or Failed. */
/*#define MAX_CLASS           	   12  */
#define MAX_CLASS           	   20  
#define MAX_NPC_CLASS		   26
/*#define MAX_RACE                 20  Trying to fix a bunch of problems-- Scryn*/  
/*#define MAX_RACE                   15 */     /*  added 6 for new race code */
#define MAX_RACE                   20
#define MAX_NPC_RACE		   91

extern int MAX_PC_RACE;
extern int MAX_PC_CLASS;

#define MAX_LEVEL		   65
#define MAX_CLAN		   50
#define MAX_DEITY		   50
#define MAX_CPD			    4   /* Maximum council power level difference */
#define	MAX_HERB		   20
#define	MAX_DISEASE		   20
#define MAX_PERSONAL		    5   /* Maximum personal skills */
#define MAX_WHERE_NAME             29
#define LEVEL_HERO		   (MAX_LEVEL - 15)
#define LEVEL_IMMORTAL		   (MAX_LEVEL - 14)
#define LEVEL_SUPREME		   MAX_LEVEL
#define LEVEL_INFINITE		   (MAX_LEVEL - 1)
#define LEVEL_ETERNAL		   (MAX_LEVEL - 2)
#define LEVEL_IMPLEMENTOR	   (MAX_LEVEL - 3)
#define LEVEL_SUB_IMPLEM	   (MAX_LEVEL - 4)
#define LEVEL_ASCENDANT		   (MAX_LEVEL - 5)
#define LEVEL_GREATER		   (MAX_LEVEL - 6)
#define LEVEL_GOD		   (MAX_LEVEL - 7)
#define LEVEL_LESSER		   (MAX_LEVEL - 8)
#define LEVEL_TRUEIMM		   (MAX_LEVEL - 9)
#define LEVEL_DEMI		   (MAX_LEVEL - 10)
#define LEVEL_SAVIOR		   (MAX_LEVEL - 11)
#define LEVEL_CREATOR		   (MAX_LEVEL - 12)
#define LEVEL_ACOLYTE		   (MAX_LEVEL - 13)
#define LEVEL_NEOPHYTE		   (MAX_LEVEL - 14)
#define LEVEL_AVATAR		   (MAX_LEVEL - 15)

#define LEVEL_LOG		    LEVEL_LESSER
#define LEVEL_HIGOD		    LEVEL_GOD

/* This is to tell if act uses uppercasestring or not --Shaddai */
bool DONT_UPPER;

#define	SECONDS_PER_TICK			 70

#define PULSE_PER_SECOND			  4
#define PULSE_VIOLENCE				 (3 * PULSE_PER_SECOND)
#define PULSE_MOBILE				 (4 * PULSE_PER_SECOND)
#define PULSE_TICK		  (SECONDS_PER_TICK * PULSE_PER_SECOND)
#define PULSE_AREA				(60 * PULSE_PER_SECOND)
#define PULSE_AUCTION				 (9 * PULSE_PER_SECOND)

/*
 * SMAUG Version -- Scryn
 */
#define SMAUG_VERSION_MAJOR "1"
#define SMAUG_VERSION_MINOR "4a"

/* 
 * Stuff for area versions --Shaddai
 */
int     area_version;
#define HAS_SPELL_INDEX     -1
#define AREA_VERSION_WRITE 1

/*
 * Command logging types.
 */
typedef enum
{
  LOG_NORMAL, LOG_ALWAYS, LOG_NEVER, LOG_BUILD, LOG_HIGH, LOG_COMM, 
  LOG_WARN, LOG_ALL
} log_types;

/*
 * Return types for move_char, damage, greet_trigger, etc, etc
 * Added by Thoric to get rid of bugs
 */
typedef enum
{
  rNONE, rCHAR_DIED, rVICT_DIED, rBOTH_DIED, rCHAR_QUIT, rVICT_QUIT,
  rBOTH_QUIT, rSPELL_FAILED, rOBJ_SCRAPPED, rOBJ_EATEN, rOBJ_EXPIRED,
  rOBJ_TIMER, rOBJ_SACCED, rOBJ_QUAFFED, rOBJ_USED, rOBJ_EXTRACTED,
  rOBJ_DRUNK, rCHAR_IMMUNE, rVICT_IMMUNE,
  rCHAR_AND_OBJ_EXTRACTED = 128,
  rERROR = 255
} ret_types;

/* Echo types for echo_to_all */
#define ECHOTAR_ALL	0
#define ECHOTAR_PC	1
#define ECHOTAR_IMM	2

/* defines for new do_who */
#define WT_MORTAL	0
#define WT_DEADLY	1
#define WT_IMM		2
#define WT_GROUPED	3
#define WT_GROUPWHO	4

/*
 * Defines for extended bitvectors
 */
#ifndef INTBITS
  #define INTBITS	32
#endif
#define XBM		31	/* extended bitmask   ( INTBITS - 1 )	*/
#define RSV		5	/* right-shift value  ( sqrt(XBM+1) )	*/
#define XBI		4	/* integers in an extended bitvector	*/
#define MAX_BITS	XBI * INTBITS
/*
 * Structure for extended bitvectors -- Thoric
 */
struct extended_bitvector
{
    int		bits[XBI];
};



/*
 * Structure for a morph -- Shaddai
 */
/*
 *  Morph structs.
 */

#define ONLY_PKILL  	1
#define ONLY_PEACEFULL  2

struct char_morph
{
  MORPH_DATA  * morph;
  EXT_BV	affected_by;    /* New affected_by added */
  EXT_BV	no_affected_by; /* Prevents affects from being added */
  int           no_immune;      /* Prevents Immunities */
  int           no_resistant;   /* Prevents resistances */
  int           no_suscept;     /* Prevents Susceptibilities */
  int		immune;		/* Immunities added */
  int		resistant;	/* Resistances added */
  int		suscept;	/* Suscepts added */
  int           timer;          /* How much time is left */
  sh_int        ac;
  sh_int        blood;
  sh_int        cha;
  sh_int        con;
  sh_int        damroll;
  sh_int        dex;
  sh_int        dodge;
  sh_int        hit;
  sh_int        hitroll;
  sh_int        inte;
  sh_int        lck;
  sh_int        mana;
  sh_int        move;
  sh_int        parry;
  sh_int        saving_breath;
  sh_int        saving_para_petri;
  sh_int        saving_poison_death;
  sh_int        saving_spell_staff;
  sh_int        saving_wand;
  sh_int        str;
  sh_int        tumble;
  sh_int        wis;
};

struct morph_data
{
  MORPH_DATA    *next;          /* Next morph file */
  MORPH_DATA    *prev;          /* Previous morph file */
  char          *blood;         /* Blood added vamps only */
  char          *damroll;
  char		*deity;
  char          *description;
  char          *help;          /* What player sees for info on morph */
  char          *hit;           /* Hitpoints added */
  char          *hitroll;
  char          *key_words;     /* Keywords added to your name */
  char          *long_desc;     /* New long_desc for player */
  char          *mana;          /* Mana added not for vamps */
  char          *morph_other;   /* What others see when you morph */
  char          *morph_self;    /* What you see when you morph */
  char          *move;          /* Move added */
  char          *name;          /* Name used to polymorph into this */
  char          *short_desc;    /* New short desc for player */
  char          *no_skills;     /* Prevented Skills */
  char          *skills;
  char          *unmorph_other; /* What others see when you unmorph */
  char          *unmorph_self;  /* What you see when you unmorph */
  EXT_BV	affected_by;    /* New affected_by added */
  int           class;          /* Classes not allowed to use this */
  int		defpos;		/* Default position */
  EXT_BV	no_affected_by;	/* Prevents affects from being added */
  int           no_immune;	/* Prevents Immunities */
  int           no_resistant;	/* Prevents resistances */
  int           no_suscept;	/* Prevents Susceptibilities */
  int           immune;		/* Immunities added */
  int           resistant;      /* Resistances added */
  int           suscept;        /* Suscepts added */
  int           obj[3];		/* Object needed to morph you */
  int           race;           /* Races not allowed to use this */
  int		timer;		/* Timer for how long it lasts */
  int           used;           /* How many times has this morph been used */
  int           vnum;           /* Unique identifier */
  sh_int        ac;
  sh_int	bloodused;	/* Amount of blood morph requires Vamps only */
  sh_int        cha;		/* Amount Cha gained/Lost */
  sh_int        con;		/* Amount of Con gained/Lost */
  sh_int	dayfrom;	/* Starting Day you can morph into this */
  sh_int	dayto;		/* Ending Day you can morph into this */
  sh_int        dex;		/* Amount of dex added */
  sh_int        dodge;		/* Percent of dodge added IE 1 = 1% */
  sh_int	favourused;	/* Amount of favour to morph */
  sh_int	gloryused;	/* Amount of glory used to morph */
  sh_int	hpused;		/* Amount of hps used to morph */
  sh_int        inte;		/* Amount of Int gained/lost */
  sh_int        lck;		/* Amount of Lck gained/lost */
  sh_int        level;          /* Minimum level to use this morph */
  sh_int	manaused;	/* Amount of mana used to morph */
  sh_int	moveused;	/* Amount of move used to morph */
  sh_int        parry;		/* Percent of parry added IE 1 = 1% */
  sh_int	pkill;		/* Pkill Only, Peacefull Only or Both */
  sh_int        saving_breath;	/* Below are saving adjusted */
  sh_int        saving_para_petri;
  sh_int        saving_poison_death;
  sh_int        saving_spell_staff;
  sh_int        saving_wand;
  sh_int	sex;		/* The sex that can morph into this */
  sh_int        str;		/* Amount of str gained lost */
  sh_int	timefrom;	/* Hour starting you can morph */
  sh_int	timeto;		/* Hour ending that you can morph */
  sh_int        tumble;		/* Percent of tumble added IE 1 = 1% */
  sh_int        wis;		/* Amount of Wis gained/lost */
  bool          no_cast;	/* Can you cast a spell to morph into it */
  bool		objuse[3];	/* Objects needed to morph */
};

/*
 * Tongues / Languages structures
 */

struct lcnv_data
{
    LCNV_DATA *		next;
    LCNV_DATA *		prev;
    char *		old;
    int			olen;
    char *		new;
    int			nlen;
};

struct lang_data
{
    LANG_DATA *		next;
    LANG_DATA *		prev;
    char *		name;
    LCNV_DATA *		first_precnv;
    LCNV_DATA *		last_precnv;
    char *		alphabet;
    LCNV_DATA *		first_cnv;
    LCNV_DATA *		last_cnv;
};



/*
 * do_who output structure -- Narn
 */ 
struct who_data
{
  WHO_DATA *prev;
  WHO_DATA *next;
  char *text;
  int  type;
};

/*
 * Player watch data structure  --Gorog
 */
struct  watch_data
{
    WATCH_DATA *        next;
    WATCH_DATA *        prev;
    sh_int              imm_level;
    char *              imm_name;        /* imm doing the watching */
    char *              target_name;     /* player or command being watched   */
    char *              player_site;     /* site being watched     */
};

/*
 * Nuisance structure
 */

#define MAX_NUISANCE_STAGE 10	/* How many nuisance stages */
struct nuisance_data 
{
    long int            time;     /* The time nuisance flag was set */
    long int            max_time; /* Time for max penalties */
    int                 flags;    /* Stage of nuisance */
    int                 power;    /* Power of nuisance */
};

/*
 * Ban Types --- Shaddai
 */
#define BAN_SITE        1
#define BAN_CLASS       2
#define BAN_RACE        3
#define BAN_WARN        -1

/*
 * Site ban structure.
 */
struct	ban_data
{
    BAN_DATA *next;
    BAN_DATA *prev;
    char     *name;     /* Name of site/class/race banned */
    char     *user;     /* Name of user from site */
    char     *note;     /* Why it was banned */
    char     *ban_by;   /* Who banned this site */
    char     *ban_time; /* Time it was banned */
    int      flag;      /* Class or Race number */
    int      unban_date;/* When ban expires */
    sh_int   duration;  /* How long it is banned for */
    sh_int   level;     /* Level that is banned */
    bool     warn;      /* Echo on warn channel */
    bool     prefix;    /* Use of *site */
    bool     suffix;    /* Use of site* */
};



/*
 * Yeesh.. remind us of the old MERC ban structure? :)
 */
struct	reserve_data
{
    RESERVE_DATA *next;
    RESERVE_DATA *prev;
    char *name;
};



/*
 * Time and weather stuff.
 */
typedef enum
{
  SUN_DARK, SUN_RISE, SUN_LIGHT, SUN_SET
} sun_positions;

typedef enum
{
  SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING
} sky_conditions;

struct	time_info_data
{
    int		hour;
    int		day;
    int		month;
    int		year;
    int		sunlight;
};

struct hour_min_sec
{
  int hour;
  int min;
  int sec;
  int manual;
};

/* Define maximum number of climate settings - FB */
#define MAX_CLIMATE 5

struct	weather_data
{
/*    int			mmhg;
    int			change;
    int			sky;
    int			sunlight; */
    int 		temp;		/* temperature */
    int			precip;		/* precipitation */
    int			wind;		/* umm... wind */
    int			temp_vector;	/* vectors controlling */
    int			precip_vector;	/* rate of change */
    int			wind_vector;
    int			climate_temp;	/* climate of the area */
    int			climate_precip;
    int			climate_wind;
    NEIGHBOR_DATA *	first_neighbor;	/* areas which affect weather sys */
    NEIGHBOR_DATA *	last_neighbor;
    char *		echo;		/* echo string */
    int			echo_color;	/* color for the echo */
};

struct neighbor_data
{
    NEIGHBOR_DATA *next;
    NEIGHBOR_DATA *prev;
    char *name;
    AREA_DATA *address;
};

/*
 * Structure used to build wizlist
 */
struct	wizent
{
    WIZENT *		next;
    WIZENT *		last;
    char *		name;
    sh_int		level;
};

/*
 * Structure to only allow immortals domains to access their chars.
 */
struct immortal_host
{
    IMMORTAL_HOST *next;
    IMMORTAL_HOST *prev;
    char *name;
    char *host;
    bool prefix;
    bool suffix;
};

struct	project_data
{
    PROJECT_DATA * next;		/* Next project in list		   */
    PROJECT_DATA * prev;		/* Previous project in list	   */
    NOTE_DATA *  first_log;		/* First log on project		   */
    NOTE_DATA *  last_log;		/* Last log  on project		   */
    char *name;
    char *owner;
    char *coder;
    char *status;
    char *date;
    char *description;
    bool taken;				/* Has someone taken project?      */
};


/*
 * Connected state for a channel.
 */
typedef enum
{
  CON_PLAYING,		CON_GET_NAME,		CON_GET_OLD_PASSWORD,
  CON_CONFIRM_NEW_NAME,	CON_GET_NEW_PASSWORD,	CON_CONFIRM_NEW_PASSWORD,
  CON_GET_NEW_SEX,	CON_GET_NEW_CLASS,	CON_READ_MOTD,
  CON_GET_NEW_RACE,	CON_GET_EMULATION,	CON_EDITING,
  CON_GET_WANT_RIPANSI,	CON_TITLE,		CON_PRESS_ENTER,
  CON_WAIT_1,		CON_WAIT_2,		CON_WAIT_3,
  CON_ACCEPTED,         CON_GET_PKILL,		CON_READ_IMOTD
} connection_types;

/*
 * Character substates
 */
typedef enum
{
  SUB_NONE, SUB_PAUSE, SUB_PERSONAL_DESC, SUB_BAN_DESC, SUB_OBJ_SHORT, 
  SUB_OBJ_LONG, SUB_OBJ_EXTRA, SUB_MOB_LONG, SUB_MOB_DESC, SUB_ROOM_DESC, 
  SUB_ROOM_EXTRA, SUB_ROOM_EXIT_DESC, SUB_WRITING_NOTE, SUB_MPROG_EDIT, 
  SUB_HELP_EDIT, SUB_WRITING_MAP, SUB_PERSONAL_BIO, SUB_REPEATCMD, 
  SUB_RESTRICTED, SUB_DEITYDESC, SUB_MORPH_DESC, SUB_MORPH_HELP,
  SUB_PROJ_DESC,
  /* timer types ONLY below this point */
  SUB_TIMER_DO_ABORT = 128, SUB_TIMER_CANT_ABORT
} char_substates;

/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	prev;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    char *		host;
    int			port;
    int			descriptor;
    sh_int		connected;
    sh_int		idle;
    sh_int		lines;
    sh_int		scrlen;
    bool		fcommand;
    char		inbuf		[MAX_INBUF_SIZE];
    char		incomm		[MAX_INPUT_LENGTH];
    char		inlast		[MAX_INPUT_LENGTH];
    int			repeat;
    char *		outbuf;
    unsigned long	outsize;
    int			outtop;
    char *		pagebuf;
    unsigned long	pagesize;
    int			pagetop;
    char *		pagepoint;
    char		pagecmd;
    char		pagecolor;
    char *		user;
    int			newstate;
    unsigned char	prevcolor;
};



/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
    sh_int	tohit;
    sh_int	todam;
    sh_int	carry;
    sh_int	wield;
};

struct	int_app_type
{
    sh_int	learn;
};

struct	wis_app_type
{
    sh_int	practice;
};

struct	dex_app_type
{
    sh_int	defensive;
};

struct	con_app_type
{
    sh_int	hitp;
    sh_int	shock;
};

struct	cha_app_type
{
    sh_int	charm;
};

struct  lck_app_type
{
    sh_int	luck;
};

/* the races */
typedef enum {
  RACE_HUMAN, RACE_ELF, RACE_DWARF, RACE_HALFLING, RACE_PIXIE, RACE_VAMPIRE, 
  RACE_HALF_OGRE, RACE_HALF_ORC, RACE_HALF_TROLL, RACE_HALF_ELF, RACE_GITH, 
  RACE_DROW, RACE_SEA_ELF, RACE_LIZARDMAN
} race_types;

/* npc races */
#define	RACE_DRAGON	    31

#define CLASS_NONE	   -1 /* For skill/spells according to guild */
#define CLASS_MAGE	    0
#define CLASS_CLERIC	    1
#define CLASS_THIEF	    2
#define CLASS_WARRIOR	    3
#define CLASS_VAMPIRE	    4
#define CLASS_DRUID	    5
#define CLASS_RANGER	    6
#define CLASS_AUGURER	    7 /* 7-7-96 SB */
#define CLASS_PALADIN	    8 /* 7-7-96 SB */
#define CLASS_NEPHANDI	    9 
#define CLASS_SAVAGE	   10

/*
 * Languages -- Altrag
 */
#define LANG_COMMON      BV00  /* Human base language */
#define LANG_ELVEN       BV01  /* Elven base language */
#define LANG_DWARVEN     BV02  /* Dwarven base language */
#define LANG_PIXIE       BV03  /* Pixie/Fairy base language */
#define LANG_OGRE        BV04  /* Ogre base language */
#define LANG_ORCISH      BV05  /* Orc base language */
#define LANG_TROLLISH    BV06  /* Troll base language */
#define LANG_RODENT      BV07  /* Small mammals */
#define LANG_INSECTOID   BV08  /* Insects */
#define LANG_MAMMAL      BV09  /* Larger mammals */
#define LANG_REPTILE     BV10  /* Small reptiles */
#define LANG_DRAGON      BV11  /* Large reptiles, Dragons */
#define LANG_SPIRITUAL   BV12  /* Necromancers or undeads/spectres */
#define LANG_MAGICAL     BV13  /* Spells maybe?  Magical creatures */
#define LANG_GOBLIN      BV14  /* Goblin base language */
#define LANG_GOD         BV15  /* Clerics possibly?  God creatures */
#define LANG_ANCIENT     BV16  /* Prelude to a glyph read skill? */
#define LANG_HALFLING    BV17  /* Halfling base language */
#define LANG_CLAN	 BV18  /* Clan language */
#define LANG_GITH	 BV19  /* Gith Language */
#define LANG_UNKNOWN        0  /* Anything that doesnt fit a category */
#define VALID_LANGS    ( LANG_COMMON | LANG_ELVEN | LANG_DWARVEN | LANG_PIXIE  \
		       | LANG_OGRE | LANG_ORCISH | LANG_TROLLISH | LANG_GOBLIN \
		       | LANG_HALFLING | LANG_GITH )
/* 18 Languages */

/*
 * TO types for act.
 */
typedef enum { TO_ROOM, TO_NOTVICT, TO_VICT, TO_CHAR, TO_CANSEE } to_types;

/*
 * Real action "TYPES" for act.
 */
#define AT_COLORIZE	   -1	/* Color sequence to interpret color codes */
#define AT_BLACK	    0
#define AT_BLOOD	    1
#define AT_DGREEN           2
#define AT_ORANGE	    3
#define AT_DBLUE	    4
#define AT_PURPLE	    5
#define AT_CYAN	  	    6
#define AT_GREY		    7
#define AT_DGREY	    8
#define AT_RED		    9
#define AT_GREEN	   10
#define AT_YELLOW	   11
#define AT_BLUE		   12
#define AT_PINK		   13
#define AT_LBLUE	   14
#define AT_WHITE	   15
#define AT_BLINK	   16
typedef enum
{ AT_COLORBASE = 1024,
  AT_PLAIN = AT_COLORBASE,
  AT_ACTION,
  AT_SAY,
  AT_GOSSIP,
  AT_YELL,
  AT_TELL,
  AT_WHISPER,
  AT_HIT,
  AT_HITME,
  AT_IMMORT,
  AT_HURT,
  AT_FALLING,
  AT_DANGER, 
  AT_MAGIC, 
  AT_CONSIDER,
  AT_REPORT,
  AT_POISON,
  AT_SOCIAL,
  AT_DYING,
  AT_DEAD,
  AT_SKILL,
  AT_CARNAGE,
  AT_DAMAGE,
  AT_FLEE,
  AT_RMNAME,
  AT_RMDESC,
  AT_OBJECT,
  AT_PERSON,
  AT_LIST,
  AT_BYE,
  AT_GOLD,
  AT_GTELL,
  AT_NOTE,
  AT_HUNGRY,
  AT_THIRSTY,
  AT_FIRE,
  AT_SOBER,
  AT_WEAROFF,
  AT_EXITS,
  AT_SCORE,
  AT_RESET,
  AT_LOG,
  AT_DIEMSG,
  AT_WARTALK,
  AT_RACETALK,
  AT_IGNORE,
  AT_DIVIDER,
  AT_MORPH,
  AT_TOPCOLOR
} at_color_types;
#define AT_MAXCOLOR	(AT_TOPCOLOR-AT_COLORBASE)

#if 0
#define AT_PLAIN	   AT_GREY
#define AT_ACTION	   AT_GREY
#define AT_SAY		   AT_LBLUE
#define AT_GOSSIP	   AT_LBLUE
#define AT_YELL	           AT_WHITE
#define AT_TELL		   AT_WHITE
#define AT_WHISPER	   AT_WHITE
#define AT_HIT		   AT_WHITE
#define AT_HITME	   AT_YELLOW
#define AT_IMMORT	   AT_YELLOW
#define AT_HURT		   AT_RED
#define AT_FALLING	   AT_WHITE + AT_BLINK
#define AT_DANGER	   AT_RED + AT_BLINK
#define AT_MAGIC	   AT_BLUE
#define AT_CONSIDER	   AT_GREY
#define AT_REPORT	   AT_GREY
#define AT_POISON	   AT_GREEN
#define AT_SOCIAL	   AT_CYAN
#define AT_DYING	   AT_YELLOW
#define AT_DEAD		   AT_RED
#define AT_SKILL	   AT_GREEN
#define AT_CARNAGE	   AT_BLOOD
#define AT_DAMAGE	   AT_WHITE
#define AT_FLEE		   AT_YELLOW
#define AT_RMNAME	   AT_WHITE
#define AT_RMDESC	   AT_YELLOW
#define AT_OBJECT	   AT_GREEN
#define AT_PERSON	   AT_PINK
#define AT_LIST		   AT_BLUE
#define AT_BYE		   AT_GREEN
#define AT_GOLD		   AT_YELLOW
#define AT_GTELL	   AT_BLUE
#define AT_NOTE		   AT_GREEN
#define AT_HUNGRY	   AT_ORANGE
#define AT_THIRSTY	   AT_BLUE
#define	AT_FIRE		   AT_RED
#define AT_SOBER	   AT_WHITE
#define AT_WEAROFF	   AT_YELLOW
#define AT_EXITS	   AT_WHITE
#define AT_SCORE	   AT_LBLUE
#define AT_RESET	   AT_DGREEN
#define AT_LOG		   AT_PURPLE
#define AT_DIEMSG	   AT_WHITE
#define AT_WARTALK         AT_RED
#define AT_RACETALK	   AT_DGREEN
#define AT_IGNORE	   AT_GREEN
#define AT_DIVIDER	   AT_PLAIN
#define AT_MORPH           AT_GREY
#endif

#define INIT_WEAPON_CONDITION    12
#define MAX_ITEM_IMPACT		 30

/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA *	next;
    HELP_DATA * prev;
    sh_int	level;
    char *	keyword;
    char *	text;
};



/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
    SHOP_DATA *	next;			/* Next shop in list		*/
    SHOP_DATA * prev;			/* Previous shop in list	*/
    int		keeper;			/* Vnum of shop keeper mob	*/
    sh_int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    sh_int	profit_buy;		/* Cost multiplier for buying	*/
    sh_int	profit_sell;		/* Cost multiplier for selling	*/
    sh_int	open_hour;		/* First opening hour		*/
    sh_int	close_hour;		/* First closing hour		*/
};

#define MAX_FIX		3
#define SHOP_FIX	1
#define SHOP_RECHARGE	2

struct	repairshop_data
{
    REPAIR_DATA * next;			/* Next shop in list		*/
    REPAIR_DATA * prev;			/* Previous shop in list	*/
    int		  keeper;		/* Vnum of shop keeper mob	*/
    sh_int	  fix_type [MAX_FIX];	/* Item types shop will fix	*/
    sh_int	  profit_fix;		/* Cost multiplier for fixing	*/
    sh_int	  shop_type;		/* Repair shop type		*/
    sh_int	  open_hour;		/* First opening hour		*/
    sh_int	  close_hour;		/* First closing hour		*/
};


/* Mob program structures */
struct  act_prog_data
{
    struct act_prog_data *next;
    void *vo;
};

struct	mob_prog_act_list
{
    MPROG_ACT_LIST * next;
    char *	     buf;
    CHAR_DATA *      ch;
    OBJ_DATA *	     obj;
    void *	     vo;
};

struct	mob_prog_data
{
    MPROG_DATA * next;
    sh_int	 type;
    bool	 triggered;
    int		 resetdelay;
    char *	 arglist;
    char *	 comlist;
};

bool	MOBtrigger;

/*
 * Per-class stuff.
 */
struct	class_type
{
    char *	who_name;		/* Name for 'who'		*/
    EXT_BV	affected;
    sh_int	attr_prime;		/* Prime attribute		*/
    sh_int	attr_second;		/* Second attribute		*/
    sh_int	attr_deficient;		/* Deficient attribute		*/
    int		resist;
    int		suscept;
    int		weapon;			/* First weapon			*/
    int		guild;			/* Vnum of guild room		*/
    sh_int	skill_adept;		/* Maximum skill level		*/
    sh_int	thac0_00;		/* Thac0 for level  0		*/
    sh_int	thac0_32;		/* Thac0 for level 32		*/
    sh_int	hp_min;			/* Min hp gained on leveling	*/
    sh_int	hp_max;			/* Max hp gained on leveling	*/
    bool	fMana;			/* Class gains mana on level	*/
    sh_int	exp_base;		/* Class base exp		*/
};

/* race dedicated stuff */
struct	race_type
{
    char 	race_name	[16];	/* Race name			*/
    EXT_BV	affected;		/* Default affect bitvectors	*/
    sh_int	str_plus;		/* Str bonus/penalty		*/
    sh_int	dex_plus;		/* Dex      "			*/
    sh_int	wis_plus;		/* Wis      "			*/
    sh_int	int_plus;		/* Int      "			*/
    sh_int	con_plus;		/* Con      "			*/
    sh_int	cha_plus;		/* Cha      "			*/
    sh_int	lck_plus;		/* Lck 	    "			*/
    sh_int      hit;
    sh_int      mana;
    int      	resist;
    int      	suscept;
    int		class_restriction;	/* Flags for illegal classes	*/
    int         language;               /* Default racial language      */
    sh_int      ac_plus;               
    sh_int      alignment;               
    EXT_BV      attacks;
    EXT_BV      defenses;
    sh_int      minalign;
    sh_int      maxalign;
    sh_int      exp_multiplier;
    sh_int      height;
    sh_int      weight;
    sh_int      hunger_mod;
    sh_int      thirst_mod;
    sh_int	saving_poison_death;
    sh_int	saving_wand;
    sh_int	saving_para_petri;
    sh_int	saving_breath;
    sh_int	saving_spell_staff;
    char *	where_name[MAX_WHERE_NAME];
    sh_int      mana_regen;
    sh_int      hp_regen;
    sh_int      race_recall;
};

typedef enum
{
  CLAN_PLAIN, CLAN_VAMPIRE, CLAN_WARRIOR, CLAN_DRUID, CLAN_MAGE, CLAN_CELTIC,
  CLAN_THIEF, CLAN_CLERIC, CLAN_UNDEAD, CLAN_CHAOTIC, CLAN_NEUTRAL, CLAN_LAWFUL,
  CLAN_NOKILL, CLAN_ORDER, CLAN_GUILD
} clan_types;

typedef enum
{
  GROUP_CLAN, GROUP_COUNCIL, GROUP_GUILD
} group_types;


struct	clan_data
{
    CLAN_DATA * next;		/* next clan in list			*/
    CLAN_DATA * prev;		/* previous clan in list		*/
    char *	filename;	/* Clan filename			*/
    char *	name;		/* Clan name				*/
    char *	motto;		/* Clan motto				*/
    char *	description;	/* A brief description of the clan	*/
    char *	deity;		/* Clan's deity				*/
    char *	leader;		/* Head clan leader			*/
    char *	number1;	/* First officer			*/
    char *	number2;	/* Second officer			*/
    char *	badge;		/* Clan badge on who/where/to_room      */
    char *      leadrank;	/* Leader's rank			*/
    char *      onerank;	/* Number One's rank			*/
    char *	tworank;	/* Number Two's rank			*/
    int		pkills[7];	/* Number of pkills on behalf of clan	*/
    int		pdeaths[7];	/* Number of pkills against clan	*/
    int		mkills;		/* Number of mkills on behalf of clan	*/
    int		mdeaths;	/* Number of clan deaths due to mobs	*/
    int		illegal_pk;	/* Number of illegal pk's by clan	*/
    int		score;		/* Overall score			*/
    sh_int	clan_type;	/* See clan type defines		*/
    sh_int	favour;		/* Deities favour upon the clan		*/
    sh_int	strikes;	/* Number of strikes against the clan	*/
    sh_int	members;	/* Number of clan members		*/
    sh_int	mem_limit;	/* Number of clan members allowed	*/
    sh_int	alignment;	/* Clan's general alignment		*/
    int		board;		/* Vnum of clan board			*/
    int		clanobj1;	/* Vnum of first clan obj		*/
    int		clanobj2;	/* Vnum of second clan obj		*/
    int		clanobj3;	/* Vnum of third clan obj		*/
    int		clanobj4;	/* Vnum of fourth clan obj		*/
    int		clanobj5;	/* Vnum of fifth clan obj		*/
    int		recall;		/* Vnum of clan's recall room		*/
    int		storeroom;	/* Vnum of clan's store room		*/
    int		guard1;		/* Vnum of clan guard type 1		*/
    int		guard2;		/* Vnum of clan guard type 2		*/
    int		class;		/* For guilds				*/
};

struct	council_data
{
    COUNCIL_DATA * next;	/* next council in list			*/
    COUNCIL_DATA * prev;	/* previous council in list		*/
    char *	filename;	/* Council filename			*/
    char *	name;		/* Council name				*/
    char *	description;	/* A brief description of the council	*/
    char *	head;		/* Council head 			*/
    char *	head2;          /* Council co-head                      */
    char *	powers;		/* Council powers			*/
    sh_int	members;	/* Number of council members		*/
    int		board;		/* Vnum of council board		*/
    int		meeting;	/* Vnum of council's meeting room	*/
};

struct	deity_data
{
    DEITY_DATA * next;
    DEITY_DATA * prev;
    char *	filename;
    char *	name;
    char *	description;
    sh_int	alignment;
    sh_int	worshippers;
    sh_int	scorpse;
    sh_int	sdeityobj;
    sh_int	savatar;
    sh_int	srecall;
    sh_int	flee;
    sh_int	flee_npcrace;
    sh_int	flee_npcfoe;
    sh_int	kill;
    sh_int	kill_magic;
    sh_int	kill_npcrace;
    sh_int	kill_npcfoe;
    sh_int	sac;
    sh_int	bury_corpse;
    sh_int	aid_spell;
    sh_int	aid;
    sh_int	backstab;
    sh_int	steal;
    sh_int	die;
    sh_int	die_npcrace;
    sh_int	die_npcfoe;
    sh_int	spell_aid;
    sh_int	dig_corpse;
    int		race;
    int		race2;
    int		class;
    int		sex;
    int		npcrace;
    int		npcfoe;
    int		suscept;
    int		element;
    EXT_BV	affected;
    int		susceptnum;
    int		elementnum;
    int		affectednum;
    int	        objstat;
};


struct tourney_data
{
    int    open;
    int    low_level;
    int    hi_level;
};

/*
 * Data structure for notes.
 */
struct	note_data
{
    NOTE_DATA *	next;
    NOTE_DATA * prev;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    int         voting;
    char *	yesvotes;
    char *	novotes;
    char *	abstentions;
    char *	text;
};

struct	board_data
{
    BOARD_DATA * next;			/* Next board in list		   */
    BOARD_DATA * prev;			/* Previous board in list	   */
    NOTE_DATA *  first_note;		/* First note on board		   */
    NOTE_DATA *  last_note;		/* Last note on board		   */
    char *	 note_file;		/* Filename to save notes to	   */
    char *	 read_group;		/* Can restrict a board to a       */
    char *	 post_group;		/* council, clan, guild etc        */
    char *	 extra_readers;		/* Can give read rights to players */
    char *       extra_removers;        /* Can give remove rights to players */
    char *	 otakemessg;		/* Next items set what is seen when */
    char *	 opostmessg;		/* that action is taken. --Shaddai  */
    char *	 oremovemessg;
    char *	 ocopymessg;
    char *	 olistmessg;
    char *	 postmessg;
    char *	 oreadmessg;
    int		 board_obj;		/* Vnum of board object		   */
    sh_int	 num_posts;		/* Number of notes on this board   */
    sh_int	 min_read_level;	/* Minimum level to read a note	   */
    sh_int	 min_post_level;	/* Minimum level to post a note    */
    sh_int	 min_remove_level;	/* Minimum level to remove a note  */
    sh_int	 max_posts;		/* Maximum amount of notes allowed */
    int          type;                  /* Normal board or mail board? */
};



struct at_color_type
{
  char *name;
  sh_int def_color;
};



/*
 * An affect.
 *
 * So limited... so few fields... should we add more?
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    AFFECT_DATA *	prev;
    sh_int		type;
    sh_int		duration;
    sh_int		location;
    int			modifier;
    EXT_BV		bitvector;
};


/*
 * A SMAUG spell
 */
struct	smaug_affect
{
    SMAUG_AFF *		next;
    char *		duration;
    sh_int		location;
    char *		modifier;
    int			bitvector;	/* this is the bit number */
};


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_CITYGUARD	   3060
#define MOB_VNUM_VAMPIRE	   80
#define MOB_VNUM_ANIMATED_CORPSE   5
#define MOB_VNUM_POLY_WOLF	   10
#define MOB_VNUM_POLY_MIST	   11
#define MOB_VNUM_POLY_BAT	   12
#define MOB_VNUM_POLY_HAWK	   13
#define MOB_VNUM_POLY_CAT	   14
#define MOB_VNUM_POLY_DOVE	   15
#define MOB_VNUM_POLY_FISH	   16
#define MOB_VNUM_DEITY		   17

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		  0		/* Auto set for mobs	*/
#define ACT_SENTINEL		  1		/* Stays in one room	*/
#define ACT_SCAVENGER		  2		/* Picks up objects	*/
#define ACT_AGGRESSIVE		  5		/* Attacks PC's		*/
#define ACT_STAY_AREA		  6		/* Won't leave area	*/
#define ACT_WIMPY		  7		/* Flees when hurt	*/
#define ACT_PET			  8		/* Auto set for pets	*/
#define ACT_TRAIN		  9		/* Can train PC's	*/
#define ACT_PRACTICE		 10		/* Can practice PC's	*/
#define ACT_IMMORTAL		 11		/* Cannot be killed	*/
#define ACT_DEADLY		 12		/* Has a deadly poison  */
#define ACT_POLYSELF		 13
#define ACT_META_AGGR		 14		/* Attacks other mobs	*/
#define ACT_GUARDIAN		 15		/* Protects master	*/
#define ACT_RUNNING		 16		/* Hunts quickly	*/
#define ACT_NOWANDER		 17		/* Doesn't wander	*/
#define ACT_MOUNTABLE		 18		/* Can be mounted	*/
#define ACT_MOUNTED		 19		/* Is mounted		*/
#define ACT_SCHOLAR              20		/* Can teach languages  */
#define ACT_SECRETIVE		 21		/* actions aren't seen	*/
#define ACT_HARDHAT	         22		/* Immune to falling item damage */
#define ACT_MOBINVIS		 23		/* Like wizinvis	*/
#define ACT_NOASSIST		 24		/* Doesn't assist mobs	*/
#define ACT_AUTONOMOUS		 25		/* Doesn't auto switch tanks */
#define ACT_PACIFIST             26		/* Doesn't ever fight   */
#define ACT_NOATTACK		 27		/* No physical attacks */
#define ACT_ANNOYING		 28		/* Other mobs will attack */
#define ACT_STATSHIELD		 29		/* prevent statting */
#define ACT_PROTOTYPE		 30		/* A prototype mob	*/
/* 28 acts */

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 *
 * hold and flaming are yet uncoded
 */
typedef enum
{
  AFF_BLIND, AFF_INVISIBLE, AFF_DETECT_EVIL, AFF_DETECT_INVIS, 
  AFF_DETECT_MAGIC, AFF_DETECT_HIDDEN, AFF_HOLD, AFF_SANCTUARY, 
  AFF_FAERIE_FIRE, AFF_INFRARED, AFF_CURSE, AFF_FLAMING, AFF_POISON, 
  AFF_PROTECT, AFF_PARALYSIS, AFF_SNEAK, AFF_HIDE, AFF_SLEEP, AFF_CHARM, 
  AFF_FLYING, AFF_PASS_DOOR, AFF_FLOATING, AFF_TRUESIGHT, AFF_DETECTTRAPS, 
  AFF_SCRYING, AFF_FIRESHIELD, AFF_SHOCKSHIELD, AFF_HAUS1, AFF_ICESHIELD, 
  AFF_POSSESS, AFF_BERSERK, AFF_AQUA_BREATH, AFF_RECURRINGSPELL,
  AFF_CONTAGIOUS, AFF_ACIDMIST,  AFF_VENOMSHIELD, MAX_AFFECTED_BY
} affected_by_types;

/*
 * Resistant Immune Susceptible flags
 */
#define RIS_FIRE		  BV00
#define RIS_COLD		  BV01
#define RIS_ELECTRICITY		  BV02
#define RIS_ENERGY		  BV03
#define RIS_BLUNT		  BV04
#define RIS_PIERCE		  BV05
#define RIS_SLASH		  BV06
#define RIS_ACID		  BV07
#define RIS_POISON		  BV08
#define RIS_DRAIN		  BV09
#define RIS_SLEEP		  BV10
#define RIS_CHARM		  BV11
#define RIS_HOLD		  BV12
#define RIS_NONMAGIC		  BV13
#define RIS_PLUS1		  BV14
#define RIS_PLUS2		  BV15
#define RIS_PLUS3		  BV16
#define RIS_PLUS4		  BV17
#define RIS_PLUS5		  BV18
#define RIS_PLUS6		  BV19
#define RIS_MAGIC		  BV20
#define RIS_PARALYSIS		  BV21
/* 21 RIS's*/

/* 
 * Attack types
 */
typedef enum
{
  ATCK_BITE, ATCK_CLAWS, ATCK_TAIL, ATCK_STING, ATCK_PUNCH, ATCK_KICK,
  ATCK_TRIP, ATCK_BASH, ATCK_STUN, ATCK_GOUGE, ATCK_BACKSTAB, ATCK_FEED,
  ATCK_DRAIN,  ATCK_FIREBREATH, ATCK_FROSTBREATH, ATCK_ACIDBREATH,
  ATCK_LIGHTNBREATH, ATCK_GASBREATH, ATCK_POISON, ATCK_NASTYPOISON, ATCK_GAZE,
  ATCK_BLINDNESS, ATCK_CAUSESERIOUS, ATCK_EARTHQUAKE, ATCK_CAUSECRITICAL,
  ATCK_CURSE, ATCK_FLAMESTRIKE, ATCK_HARM, ATCK_FIREBALL, ATCK_COLORSPRAY,
  ATCK_WEAKEN, ATCK_SPIRALBLAST, MAX_ATTACK_TYPE
} attack_types;

/*
 * Defense types
 */
typedef enum
{
  DFND_PARRY, DFND_DODGE, DFND_HEAL, DFND_CURELIGHT, DFND_CURESERIOUS, 
  DFND_CURECRITICAL, DFND_DISPELMAGIC, DFND_DISPELEVIL, DFND_SANCTUARY, 
  DFND_FIRESHIELD, DFND_SHOCKSHIELD, DFND_SHIELD, DFND_BLESS, DFND_STONESKIN, 
  DFND_TELEPORT, DFND_MONSUM1, DFND_MONSUM2, DFND_MONSUM3, DFND_MONSUM4, 
  DFND_DISARM, DFND_ICESHIELD, DFND_GRIP, DFND_TRUESIGHT, DFND_ACIDMIST,
  DFND_VENOMSHIELD, MAX_DEFENSE_TYPE
} defense_types;

/*
 * Body parts
 */
#define PART_HEAD		  BV00
#define PART_ARMS		  BV01
#define PART_LEGS		  BV02
#define PART_HEART		  BV03
#define PART_BRAINS		  BV04
#define PART_GUTS		  BV05
#define PART_HANDS		  BV06
#define PART_FEET		  BV07
#define PART_FINGERS		  BV08
#define PART_EAR		  BV09
#define PART_EYE		  BV10
#define PART_LONG_TONGUE	  BV11
#define PART_EYESTALKS		  BV12
#define PART_TENTACLES		  BV13
#define PART_FINS		  BV14
#define PART_WINGS		  BV15
#define PART_TAIL		  BV16
#define PART_SCALES		  BV17
/* for combat */
#define PART_CLAWS		  BV18
#define PART_FANGS		  BV19
#define PART_HORNS		  BV20
#define PART_TUSKS		  BV21
#define PART_TAILATTACK		  BV22
#define PART_SHARPSCALES	  BV23
#define PART_BEAK		  BV24

#define PART_HAUNCH		  BV25
#define PART_HOOVES		  BV26
#define PART_PAWS		  BV27
#define PART_FORELEGS		  BV28
#define PART_FEATHERS		  BV29

/*
 * Autosave flags
 */
#define SV_DEATH		  BV00 /* Save on death */
#define SV_KILL			  BV01 /* Save when kill made */
#define SV_PASSCHG		  BV02 /* Save on password change */
#define SV_DROP			  BV03 /* Save on drop */
#define SV_PUT			  BV04 /* Save on put */
#define SV_GIVE			  BV05 /* Save on give */
#define SV_AUTO			  BV06 /* Auto save every x minutes (define in cset) */
#define SV_ZAPDROP		  BV07 /* Save when eq zaps */
#define SV_AUCTION		  BV08 /* Save on auction */
#define SV_GET			  BV09 /* Save on get */
#define SV_RECEIVE		  BV10 /* Save when receiving */
#define SV_IDLE			  BV11 /* Save when char goes idle */
#define SV_BACKUP		  BV12 /* Make backup of pfile on save */
#define SV_QUITBACKUP		  BV13 /* Backup on quit only --Blod */
#define SV_FILL			  BV14 /* Save on do_fill */
#define SV_EMPTY		  BV15 /* Save on do_empty */

/*
 * Pipe flags
 */
#define PIPE_TAMPED		  BV01
#define PIPE_LIT		  BV02
#define PIPE_HOT		  BV03
#define PIPE_DIRTY		  BV04
#define PIPE_FILTHY		  BV05
#define PIPE_GOINGOUT		  BV06
#define PIPE_BURNT		  BV07
#define PIPE_FULLOFASH		  BV08

/*
 * Flags for act_string -- Shaddai
 */
#define STRING_NONE               0
#define STRING_IMM                BV01


/*
 * old flags for conversion purposes -- will not conflict with the flags below
 */
#define OLD_SF_SAVE_HALF_DAMAGE	  BV18  /* old save for half damage	*/
#define OLD_SF_SAVE_NEGATES	  BV19  /* old save negates affect	*/

/*
 * Skill/Spell flags	The minimum BV *MUST* be 11!
 */
#define SF_WATER		  BV00
#define SF_EARTH		  BV01
#define SF_AIR			  BV02
#define SF_ASTRAL		  BV03
#define SF_AREA			  BV04  /* is an area spell		*/
#define SF_DISTANT		  BV05  /* affects something far away	*/
#define SF_REVERSE		  BV06
#define SF_NOSELF		  BV07  /* Can't target yourself!	*/
#define SF_UNUSED2		  BV08  /* free for use!		*/
#define SF_ACCUMULATIVE		  BV09  /* is accumulative		*/
#define SF_RECASTABLE		  BV10  /* can be refreshed		*/
#define SF_NOSCRIBE		  BV11  /* cannot be scribed		*/
#define SF_NOBREW		  BV12  /* cannot be brewed		*/
#define SF_GROUPSPELL		  BV13  /* only affects group members	*/
#define SF_OBJECT		  BV14	/* directed at an object	*/
#define SF_CHARACTER		  BV15  /* directed at a character	*/
#define SF_SECRETSKILL		  BV16	/* hidden unless learned	*/
#define SF_PKSENSITIVE		  BV17	/* much harder for plr vs. plr	*/
#define SF_STOPONFAIL		  BV18	/* stops spell on first failure */
#define SF_NOFIGHT		  BV19  /* stops if char fighting       */
#define SF_NODISPEL               BV20  /* stops spell from being dispelled */
#define SF_RANDOMTARGET		  BV21	/* chooses a random target	*/
typedef enum { SS_NONE, SS_POISON_DEATH, SS_ROD_WANDS, SS_PARA_PETRI,
	       SS_BREATH, SS_SPELL_STAFF } save_types;

#define ALL_BITS		INT_MAX
#define SDAM_MASK		ALL_BITS & ~(BV00 | BV01 | BV02)
#define SACT_MASK		ALL_BITS & ~(BV03 | BV04 | BV05)
#define SCLA_MASK		ALL_BITS & ~(BV06 | BV07 | BV08)
#define SPOW_MASK		ALL_BITS & ~(BV09 | BV10)
#define SSAV_MASK		ALL_BITS & ~(BV11 | BV12 | BV13)

typedef enum { SD_NONE, SD_FIRE, SD_COLD, SD_ELECTRICITY, SD_ENERGY, SD_ACID,
	       SD_POISON, SD_DRAIN } spell_dam_types;

typedef enum { SA_NONE, SA_CREATE, SA_DESTROY, SA_RESIST, SA_SUSCEPT,
	       SA_DIVINATE, SA_OBSCURE, SA_CHANGE } spell_act_types;

typedef enum { SP_NONE, SP_MINOR, SP_GREATER, SP_MAJOR } spell_power_types;

typedef enum { SC_NONE, SC_LUNAR, SC_SOLAR, SC_TRAVEL, SC_SUMMON,
	       SC_LIFE, SC_DEATH, SC_ILLUSION } spell_class_types;

typedef enum { SE_NONE, SE_NEGATE, SE_EIGHTHDAM, SE_QUARTERDAM, SE_HALFDAM,
	       SE_3QTRDAM, SE_REFLECT, SE_ABSORB } spell_save_effects;

/*
 * Sex.
 * Used in #MOBILES.
 */
typedef enum { SEX_NEUTRAL, SEX_MALE, SEX_FEMALE } sex_types;

typedef enum
{
  TRAP_TYPE_POISON_GAS = 1, TRAP_TYPE_POISON_DART,    TRAP_TYPE_POISON_NEEDLE,
  TRAP_TYPE_POISON_DAGGER,  TRAP_TYPE_POISON_ARROW,   TRAP_TYPE_BLINDNESS_GAS,
  TRAP_TYPE_SLEEPING_GAS,   TRAP_TYPE_FLAME,	      TRAP_TYPE_EXPLOSION,
  TRAP_TYPE_ACID_SPRAY,	    TRAP_TYPE_ELECTRIC_SHOCK, TRAP_TYPE_BLADE,
  TRAP_TYPE_SEX_CHANGE
} trap_types;

#define MAX_TRAPTYPE		   TRAP_TYPE_SEX_CHANGE

#define TRAP_ROOM      		   BV00
#define TRAP_OBJ	      	   BV01
#define TRAP_ENTER_ROOM		   BV02
#define TRAP_LEAVE_ROOM		   BV03
#define TRAP_OPEN		   BV04 
#define TRAP_CLOSE		   BV05
#define TRAP_GET		   BV06
#define TRAP_PUT		   BV07
#define TRAP_PICK		   BV08
#define TRAP_UNLOCK		   BV09
#define TRAP_N			   BV10
#define TRAP_S			   BV11 
#define TRAP_E	      		   BV12
#define TRAP_W	      		   BV13
#define TRAP_U	      		   BV14
#define TRAP_D	      		   BV15
#define TRAP_EXAMINE		   BV16
#define TRAP_NE			   BV17
#define TRAP_NW			   BV18
#define TRAP_SE			   BV19
#define TRAP_SW			   BV20

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE	      2
#define OBJ_VNUM_MONEY_SOME	      3

#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_CORPSE_PC	     11
#define OBJ_VNUM_SEVERED_HEAD	     12
#define OBJ_VNUM_TORN_HEART	     13
#define OBJ_VNUM_SLICED_ARM	     14
#define OBJ_VNUM_SLICED_LEG	     15
#define OBJ_VNUM_SPILLED_GUTS	     16
#define OBJ_VNUM_BLOOD		     17
#define OBJ_VNUM_BLOODSTAIN	     18
#define OBJ_VNUM_SCRAPS		     19

#define OBJ_VNUM_MUSHROOM	     20
#define OBJ_VNUM_LIGHT_BALL	     21
#define OBJ_VNUM_SPRING		     22

#define OBJ_VNUM_SKIN		     23
#define OBJ_VNUM_SLICE		     24
#define OBJ_VNUM_SHOPPING_BAG	     25

#define OBJ_VNUM_BLOODLET	     26

#define OBJ_VNUM_FIRE		     30
#define OBJ_VNUM_TRAP		     31
#define OBJ_VNUM_PORTAL		     32

#define OBJ_VNUM_BLACK_POWDER	     33
#define OBJ_VNUM_SCROLL_SCRIBING     34
#define OBJ_VNUM_FLASK_BREWING       35
#define OBJ_VNUM_NOTE		     36
#define OBJ_VNUM_DEITY		     64

/* Academy eq */
#define OBJ_VNUM_SCHOOL_MACE	  10315
#define OBJ_VNUM_SCHOOL_DAGGER	  10312
#define OBJ_VNUM_SCHOOL_SWORD	  10313
#define OBJ_VNUM_SCHOOL_VEST	  10308
#define OBJ_VNUM_SCHOOL_SHIELD	  10310
#define OBJ_VNUM_SCHOOL_BANNER    10311

/*
 * Item types.
 * Used in #OBJECTS.
 */
typedef enum
{
  ITEM_NONE, ITEM_LIGHT, ITEM_SCROLL, ITEM_WAND, ITEM_STAFF, ITEM_WEAPON,
  ITEM_FIREWEAPON, ITEM_MISSILE, ITEM_TREASURE, ITEM_ARMOR, ITEM_POTION,
  ITEM_WORN, ITEM_FURNITURE, ITEM_TRASH, ITEM_OLDTRAP, ITEM_CONTAINER,
  ITEM_NOTE, ITEM_DRINK_CON, ITEM_KEY, ITEM_FOOD, ITEM_MONEY, ITEM_PEN,
  ITEM_BOAT, ITEM_CORPSE_NPC, ITEM_CORPSE_PC, ITEM_FOUNTAIN, ITEM_PILL,
  ITEM_BLOOD, ITEM_BLOODSTAIN, ITEM_SCRAPS, ITEM_PIPE, ITEM_HERB_CON,
  ITEM_HERB, ITEM_INCENSE, ITEM_FIRE, ITEM_BOOK, ITEM_SWITCH, ITEM_LEVER,
  ITEM_PULLCHAIN, ITEM_BUTTON, ITEM_DIAL, ITEM_RUNE, ITEM_RUNEPOUCH,
  ITEM_MATCH, ITEM_TRAP, ITEM_MAP, ITEM_PORTAL, ITEM_PAPER,
  ITEM_TINDER, ITEM_LOCKPICK, ITEM_SPIKE, ITEM_DISEASE, ITEM_OIL, ITEM_FUEL,
  ITEM_EMPTY1, ITEM_EMPTY2, ITEM_MISSILE_WEAPON, ITEM_PROJECTILE, ITEM_QUIVER,
  ITEM_SHOVEL, ITEM_SALVE, ITEM_COOK, ITEM_KEYRING, ITEM_ODOR, ITEM_CHANCE
} item_types;

#define MAX_ITEM_TYPE		     ITEM_CHANCE

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
typedef enum
{
  ITEM_GLOW, ITEM_HUM, ITEM_DARK, ITEM_LOYAL, ITEM_EVIL, ITEM_INVIS, ITEM_MAGIC, 
  ITEM_NODROP, ITEM_BLESS, ITEM_ANTI_GOOD, ITEM_ANTI_EVIL, ITEM_ANTI_NEUTRAL, 
  ITEM_NOREMOVE, ITEM_INVENTORY, ITEM_ANTI_MAGE, ITEM_ANTI_THIEF, 
  ITEM_ANTI_WARRIOR, ITEM_ANTI_CLERIC, ITEM_ORGANIC, ITEM_METAL, ITEM_DONATION, 
  ITEM_CLANOBJECT, ITEM_CLANCORPSE, ITEM_ANTI_VAMPIRE, ITEM_ANTI_DRUID, 
  ITEM_HIDDEN, ITEM_POISONED, ITEM_COVERING, ITEM_DEATHROT, ITEM_BURIED, 
  ITEM_PROTOTYPE, ITEM_NOLOCATE, ITEM_GROUNDROT, ITEM_LOOTABLE, MAX_ITEM_FLAG
} item_extra_flags;

/* Magic flags - extra extra_flags for objects that are used in spells */
#define ITEM_RETURNING		BV00
#define ITEM_BACKSTABBER  	BV01
#define ITEM_BANE		BV02
#define ITEM_MAGIC_LOYAL	BV03
#define ITEM_HASTE		BV04
#define ITEM_DRAIN		BV05
#define ITEM_LIGHTNING_BLADE  	BV06
#define ITEM_PKDISARMED		BV07 /* Maybe temporary, not a perma flag */

/* Lever/dial/switch/button/pullchain flags */
#define TRIG_UP			BV00
#define TRIG_UNLOCK		BV01
#define TRIG_LOCK		BV02
#define TRIG_D_NORTH		BV03
#define TRIG_D_SOUTH		BV04
#define TRIG_D_EAST		BV05
#define TRIG_D_WEST		BV06
#define TRIG_D_UP		BV07
#define TRIG_D_DOWN		BV08
#define TRIG_DOOR		BV09
#define TRIG_CONTAINER		BV10
#define TRIG_OPEN		BV11
#define TRIG_CLOSE		BV12
#define TRIG_PASSAGE		BV13
#define TRIG_OLOAD		BV14
#define TRIG_MLOAD		BV15
#define TRIG_TELEPORT		BV16
#define TRIG_TELEPORTALL	BV17
#define TRIG_TELEPORTPLUS	BV18
#define TRIG_DEATH		BV19
#define TRIG_CAST		BV20
#define TRIG_FAKEBLADE		BV21
#define TRIG_RAND4		BV22
#define TRIG_RAND6		BV23
#define TRIG_TRAPDOOR		BV24
#define TRIG_ANOTHEROOM		BV25
#define TRIG_USEDIAL		BV26
#define TRIG_ABSOLUTEVNUM	BV27
#define TRIG_SHOWROOMDESC	BV28
#define TRIG_AUTORETURN		BV29

#define TELE_SHOWDESC		BV00
#define TELE_TRANSALL		BV01
#define TELE_TRANSALLPLUS	BV02


/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		BV00
#define ITEM_WEAR_FINGER	BV01
#define ITEM_WEAR_NECK		BV02
#define ITEM_WEAR_BODY		BV03
#define ITEM_WEAR_HEAD		BV04
#define ITEM_WEAR_LEGS		BV05
#define ITEM_WEAR_FEET		BV06
#define ITEM_WEAR_HANDS		BV07
#define ITEM_WEAR_ARMS		BV08
#define ITEM_WEAR_SHIELD	BV09
#define ITEM_WEAR_ABOUT		BV10
#define ITEM_WEAR_WAIST		BV11
#define ITEM_WEAR_WRIST		BV12
#define ITEM_WIELD		BV13
#define ITEM_HOLD		BV14
#define ITEM_DUAL_WIELD		BV15
#define ITEM_WEAR_EARS		BV16
#define ITEM_WEAR_EYES		BV17
#define ITEM_MISSILE_WIELD	BV18
#define ITEM_WEAR_BACK		BV19
#define ITEM_WEAR_FACE		BV20
#define ITEM_WEAR_ANKLE		BV21
#define ITEM_WEAR_MAX		21

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
typedef enum
{
  APPLY_NONE, APPLY_STR, APPLY_DEX, APPLY_INT, APPLY_WIS, APPLY_CON,
  APPLY_SEX, APPLY_CLASS, APPLY_LEVEL, APPLY_AGE, APPLY_HEIGHT, APPLY_WEIGHT,
  APPLY_MANA, APPLY_HIT, APPLY_MOVE, APPLY_GOLD, APPLY_EXP, APPLY_AC,
  APPLY_HITROLL, APPLY_DAMROLL, APPLY_SAVING_POISON, APPLY_SAVING_ROD,
  APPLY_SAVING_PARA, APPLY_SAVING_BREATH, APPLY_SAVING_SPELL, APPLY_CHA,
  APPLY_AFFECT, APPLY_RESISTANT, APPLY_IMMUNE, APPLY_SUSCEPTIBLE,
  APPLY_WEAPONSPELL, APPLY_LCK, APPLY_BACKSTAB, APPLY_PICK, APPLY_TRACK,
  APPLY_STEAL, APPLY_SNEAK, APPLY_HIDE, APPLY_PALM, APPLY_DETRAP, APPLY_DODGE,
  APPLY_PEEK, APPLY_SCAN, APPLY_GOUGE, APPLY_SEARCH, APPLY_MOUNT, APPLY_DISARM,
  APPLY_KICK, APPLY_PARRY, APPLY_BASH, APPLY_STUN, APPLY_PUNCH, APPLY_CLIMB,
  APPLY_GRIP, APPLY_SCRIBE, APPLY_BREW, APPLY_WEARSPELL, APPLY_REMOVESPELL,
  APPLY_EMOTION, APPLY_MENTALSTATE, APPLY_STRIPSN, APPLY_REMOVE, APPLY_DIG,
  APPLY_FULL, APPLY_THIRST, APPLY_DRUNK, APPLY_BLOOD, APPLY_COOK,
  APPLY_RECURRINGSPELL, APPLY_CONTAGIOUS, APPLY_EXT_AFFECT, APPLY_ODOR,
  APPLY_ROOMFLAG, APPLY_SECTORTYPE, APPLY_ROOMLIGHT, APPLY_TELEVNUM,
  APPLY_TELEDELAY, MAX_APPLY_TYPE
} apply_types;

#define REVERSE_APPLY		   1000

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		   BV00
#define CONT_PICKPROOF		   BV01
#define CONT_CLOSED		   BV02
#define CONT_LOCKED		   BV03
#define CONT_EATKEY		   BV04

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_POLY		      3
#define ROOM_VNUM_CHAT		   1200
#define ROOM_VNUM_TEMPLE	  21001
#define ROOM_VNUM_ALTAR		  21194 
#define ROOM_VNUM_SCHOOL	  10300
#define ROOM_AUTH_START		    100
#define ROOM_VNUM_HALLOFFALLEN    21195

/*
 * New bit values for sector types.  Code by Mystaric
 */
 #define BVSECT_INSIDE 			BV00
 #define BVSECT_CITY 			BV01
 #define BVSECT_FIELD 			BV02
 #define BVSECT_FOREST 			BV03
 #define BVSECT_HILLS 			BV04
 #define BVSECT_MOUNTAIN 		BV05
 #define BVSECT_WATER_SWIM 		BV06
 #define BVSECT_WATER_NOSWIM 		BV07
 #define BVSECT_UNDERWATER 		BV08
 #define BVSECT_AIR 			BV09
 #define BVSECT_DESERT 			BV10
 #define BVSECT_DUNNO 			BV11
 #define BVSECT_OCEANFLOOR 		BV12
 #define BVSECT_UNDERGROUND 		BV13
 #define BVSECT_LAVA			BV14
 #define BVSECT_SWAMP			BV15
 #define MAX_SECFLAG 			15

/*
 * Room flags.           Holy cow!  Talked about stripped away..
 * Used in #ROOMS.       Those merc guys know how to strip code down.
 *			 Lets put it all back... ;)
 */

#define ROOM_DARK		BV00
#define ROOM_DEATH		BV01
#define ROOM_NO_MOB		BV02
#define ROOM_INDOORS		BV03
#define ROOM_LAWFUL		BV04
#define ROOM_NEUTRAL		BV05
#define ROOM_CHAOTIC		BV06
#define ROOM_NO_MAGIC		BV07
#define ROOM_TUNNEL		BV08
#define ROOM_PRIVATE		BV09
#define ROOM_SAFE		BV10
#define ROOM_SOLITARY		BV11
#define ROOM_PET_SHOP		BV12
#define ROOM_NO_RECALL		BV13
#define ROOM_DONATION		BV14
#define ROOM_NODROPALL		BV15
#define ROOM_SILENCE		BV16
#define ROOM_LOGSPEECH		BV17
#define ROOM_NODROP		BV18
#define ROOM_CLANSTOREROOM	BV19
#define ROOM_NO_SUMMON		BV20
#define ROOM_NO_ASTRAL		BV21
#define ROOM_TELEPORT		BV22
#define ROOM_TELESHOWDESC	BV23
#define ROOM_NOFLOOR		BV24
#define ROOM_NOSUPPLICATE       BV25
#define ROOM_ARENA		BV26
#define ROOM_NOMISSILE		BV27
#define ROOM_PROTOTYPE	     	BV30
#define ROOM_DND	     	BV31

/*
 * Directions.
 * Used in #ROOMS.
 */
typedef enum
{
  DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN,
  DIR_NORTHEAST, DIR_NORTHWEST, DIR_SOUTHEAST, DIR_SOUTHWEST, DIR_SOMEWHERE
} dir_types;

#define PT_WATER	100
#define PT_AIR		200
#define PT_EARTH	300
#define PT_FIRE		400

/*
 * Push/pull types for exits					-Thoric
 * To differentiate between the current of a river, or a strong gust of wind
 */
typedef enum
{
  PULL_UNDEFINED, PULL_VORTEX, PULL_VACUUM, PULL_SLIP, PULL_ICE, PULL_MYSTERIOUS,
  PULL_CURRENT	 = PT_WATER, PULL_WAVE,	    PULL_WHIRLPOOL, PULL_GEYSER,
  PULL_WIND	 = PT_AIR,   PULL_STORM,    PULL_COLDWIND,  PULL_BREEZE,
  PULL_LANDSLIDE = PT_EARTH, PULL_SINKHOLE, PULL_QUICKSAND, PULL_EARTHQUAKE,
  PULL_LAVA 	 = PT_FIRE,  PULL_HOTAIR
} dir_pulltypes;


#define MAX_DIR			DIR_SOUTHWEST	/* max for normal walking */
#define DIR_PORTAL		DIR_SOMEWHERE	/* portal direction	  */


/*
 * Exit flags.			EX_RES# are reserved for use by the
 * Used in #ROOMS.		SMAUG development team
 */
#define EX_ISDOOR		  BV00
#define EX_CLOSED		  BV01
#define EX_LOCKED		  BV02
#define EX_SECRET		  BV03
#define EX_SWIM			  BV04
#define EX_PICKPROOF		  BV05
#define EX_FLY			  BV06
#define EX_CLIMB		  BV07
#define EX_DIG			  BV08
#define EX_EATKEY		  BV09
#define EX_NOPASSDOOR		  BV10
#define EX_HIDDEN		  BV11
#define EX_PASSAGE		  BV12
#define EX_PORTAL 		  BV13
#define EX_RES1			  BV14
#define EX_RES2			  BV15
#define EX_xCLIMB		  BV16
#define EX_xENTER		  BV17
#define EX_xLEAVE		  BV18
#define EX_xAUTO		  BV19
#define EX_NOFLEE	  	  BV20
#define EX_xSEARCHABLE		  BV21
#define EX_BASHED                 BV22
#define EX_BASHPROOF              BV23
#define EX_NOMOB		  BV24
#define EX_WINDOW		  BV25
#define EX_xLOOK		  BV26
#define EX_ISBOLT		  BV27
#define EX_BOLTED		  BV28
#define MAX_EXFLAG		  28

/*
 * Sector types.
 * Used in #ROOMS.
 */
typedef enum
{
  SECT_INSIDE, SECT_CITY, SECT_FIELD, SECT_FOREST, SECT_HILLS, SECT_MOUNTAIN,
  SECT_WATER_SWIM, SECT_WATER_NOSWIM, SECT_UNDERWATER, SECT_AIR, SECT_DESERT,
  SECT_DUNNO, SECT_OCEANFLOOR, SECT_UNDERGROUND, SECT_LAVA, SECT_SWAMP,
  SECT_MAX
} sector_types;

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
typedef enum
{
  WEAR_NONE = -1, WEAR_LIGHT = 0, WEAR_FINGER_L, WEAR_FINGER_R, WEAR_NECK_1,
  WEAR_NECK_2, WEAR_BODY, WEAR_HEAD, WEAR_LEGS, WEAR_FEET, WEAR_HANDS,
  WEAR_ARMS, WEAR_SHIELD, WEAR_ABOUT, WEAR_WAIST, WEAR_WRIST_L, WEAR_WRIST_R,
  WEAR_WIELD, WEAR_HOLD, WEAR_DUAL_WIELD, WEAR_EARS, WEAR_EYES,
  WEAR_MISSILE_WIELD, WEAR_BACK, WEAR_FACE, WEAR_ANKLE_L, WEAR_ANKLE_R,
  MAX_WEAR
} wear_locations;

/* Board Types */
typedef enum { BOARD_NOTE, BOARD_MAIL } board_types;

/* Auth Flags */
#define FLAG_WRAUTH		      1
#define FLAG_AUTH		      2

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
typedef enum
{
  COND_DRUNK, COND_FULL, COND_THIRST, COND_BLOODTHIRST, MAX_CONDS
} conditions;

/*
 * Positions.
 */
typedef enum
{
  POS_DEAD, POS_MORTAL, POS_INCAP, POS_STUNNED, POS_SLEEPING, POS_BERSERK,
  POS_RESTING, POS_AGGRESSIVE, POS_SITTING, POS_FIGHTING, POS_DEFENSIVE,
  POS_EVASIVE, POS_STANDING, POS_MOUNTED, POS_SHOVE, POS_DRAG
} positions;

/*
 * Styles.
 */
typedef enum
{
  STYLE_BERSERK, STYLE_AGGRESSIVE, STYLE_FIGHTING, STYLE_DEFENSIVE,
  STYLE_EVASIVE,
} styles;

/*
 * ACT bits for players.
 */
typedef enum
{
  PLR_IS_NPC,PLR_BOUGHT_PET, PLR_SHOVEDRAG, PLR_AUTOEXIT, PLR_AUTOLOOT, 
  PLR_AUTOSAC, PLR_BLANK, PLR_OUTCAST, PLR_BRIEF, PLR_COMBINE, PLR_PROMPT, 
  PLR_TELNET_GA, PLR_HOLYLIGHT, PLR_WIZINVIS, PLR_ROOMVNUM, PLR_SILENCE, 
  PLR_NO_EMOTE, PLR_ATTACKER, PLR_NO_TELL, PLR_LOG, PLR_DENY, PLR_FREEZE, 
  PLR_THIEF, PLR_KILLER, PLR_LITTERBUG, PLR_ANSI, PLR_RIP, PLR_NICE, PLR_FLEE, 
  PLR_AUTOGOLD, PLR_AUTOMAP, PLR_AFK, PLR_INVISPROMPT
} player_flags;

/* Bits for pc_data->flags. */
#define PCFLAG_R1                  BV00
#define PCFLAG_DEADLY              BV01
#define PCFLAG_UNAUTHED		   BV02
#define PCFLAG_NORECALL            BV03
#define PCFLAG_NOINTRO             BV04
#define PCFLAG_GAG		   BV05
#define PCFLAG_RETIRED             BV06
#define PCFLAG_GUEST               BV07
#define PCFLAG_NOSUMMON		   BV08
#define PCFLAG_PAGERON		   BV09
#define PCFLAG_NOTITLE             BV10
#define PCFLAG_GROUPWHO		   BV11
#define PCFLAG_DIAGNOSE		   BV12
#define PCFLAG_HIGHGAG		   BV13
#define PCFLAG_WATCH		   BV14 /* see function "do_watch" */
#define PCFLAG_HELPSTART	   BV15 /* Force new players to help start */
#define PCFLAG_DND      	   BV16 /* Do Not Disturb flage */
  /* DND flag prevents unwanted transfers of imms by lower level imms */
#define PCFLAG_IDLE		   BV17 /* Player is Linkdead */

typedef enum
{
  TIMER_NONE, TIMER_RECENTFIGHT, TIMER_SHOVEDRAG, TIMER_DO_FUN, 
  TIMER_APPLIED, TIMER_PKILLED, TIMER_ASUPRESSED, TIMER_NUISANCE
} timer_types;

struct timer_data
{
    TIMER  *	prev;
    TIMER  *	next;
    DO_FUN *	do_fun;
    int		value;
    sh_int	type;
    int		count;
};


/*
 * Channel bits.
 */
#define	CHANNEL_AUCTION		   BV00
#define	CHANNEL_CHAT		   BV01
#define	CHANNEL_QUEST		   BV02
#define	CHANNEL_IMMTALK		   BV03
#define	CHANNEL_MUSIC		   BV04
#define	CHANNEL_ASK		   BV05
#define	CHANNEL_SHOUT		   BV06
#define	CHANNEL_YELL		   BV07
#define CHANNEL_MONITOR		   BV08
#define CHANNEL_LOG		   BV09
#define CHANNEL_HIGHGOD		   BV10
#define CHANNEL_CLAN		   BV11
#define CHANNEL_BUILD		   BV12
#define CHANNEL_HIGH		   BV13
#define CHANNEL_AVTALK		   BV14
#define CHANNEL_PRAY		   BV15
#define CHANNEL_COUNCIL 	   BV16
#define CHANNEL_GUILD              BV17
#define CHANNEL_COMM		   BV18
#define CHANNEL_TELLS		   BV19
#define CHANNEL_ORDER              BV20
#define CHANNEL_NEWBIE             BV21
#define CHANNEL_WARTALK            BV22
#define CHANNEL_RACETALK           BV23
#define CHANNEL_WARN               BV24
#define CHANNEL_WHISPER		   BV25
#define CHANNEL_AUTH		   BV26
#define CHANNEL_TRAFFIC		   BV27

/* Area defines - Scryn 8/11
 *
 */
#define AREA_DELETED		   BV00
#define AREA_LOADED                BV01

/* Area flags - Narn Mar/96 */
#define AFLAG_NOPKILL               BV00
#define AFLAG_FREEKILL		    BV01
#define AFLAG_NOTELEPORT	    BV02
#define AFLAG_SPELLLIMIT	    BV03

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
    MOB_INDEX_DATA *	next;
    MOB_INDEX_DATA *	next_sort;
    SPEC_FUN *		spec_fun;
    SHOP_DATA *		pShop;
    REPAIR_DATA *	rShop;
    MPROG_DATA *	mudprogs;
    EXT_BV		progtypes;
    char *		player_name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    sh_int		vnum;
    sh_int		count;
    sh_int		killed;
    sh_int		sex;
    sh_int		level;
    EXT_BV		act;
    EXT_BV		affected_by;
    sh_int		alignment;
    sh_int		mobthac0;		/* Unused */
    sh_int		ac;
    sh_int		hitnodice;
    sh_int		hitsizedice;
    sh_int		hitplus;
    sh_int		damnodice;
    sh_int		damsizedice;
    sh_int		damplus;
    sh_int		numattacks;
    int			gold;
    int			exp;
    int			xflags;
    int			immune;
    int			resistant;
    int			susceptible;
    EXT_BV		attacks;
    EXT_BV		defenses;
    int			speaks;
    int 		speaking;
    sh_int		position;
    sh_int		defposition;
    sh_int		height;
    sh_int		weight;
    sh_int		race;
    sh_int		class;
    sh_int		hitroll;
    sh_int		damroll;
    sh_int		perm_str;
    sh_int		perm_int;
    sh_int		perm_wis;
    sh_int		perm_dex;
    sh_int		perm_con;
    sh_int		perm_cha;
    sh_int		perm_lck;
    sh_int		saving_poison_death;
    sh_int		saving_wand;
    sh_int		saving_para_petri;
    sh_int		saving_breath;
    sh_int		saving_spell_staff;
};


struct hunt_hate_fear
{
    char *		name;
    CHAR_DATA *		who;
};

struct fighting_data
{
    CHAR_DATA *		who;
    int			xp;
    sh_int		align;
    sh_int		duration;
    sh_int		timeskilled;
};

struct	editor_data
{
    sh_int		numlines;
    sh_int		on_line;
    sh_int		size;
    char		line[49][81];
};

struct	extracted_char_data
{
    EXTRACT_CHAR_DATA *	next;
    CHAR_DATA *		ch;
    ROOM_INDEX_DATA *	room;
    ch_ret		retcode;
    bool		extract;
};

/*
 * One character (PC or NPC).
 * (Shouldn't most of that build interface stuff use substate, dest_buf,
 * spare_ptr and tempnum?  Seems a little redundant)
 */
struct	char_data
{
    CHAR_DATA *		next;
    CHAR_DATA *		prev;
    CHAR_DATA *		next_in_room;
    CHAR_DATA *		prev_in_room;
    CHAR_DATA *		master;
    CHAR_DATA *		leader;
    FIGHT_DATA *	fighting;
    CHAR_DATA *		reply;
    CHAR_DATA *		retell;
    CHAR_DATA *		switched;
    CHAR_DATA *		mount;
    HHF_DATA *		hunting;
    HHF_DATA *		fearing;
    HHF_DATA *		hating;
    SPEC_FUN *		spec_fun;
    MPROG_ACT_LIST *	mpact;
    int			mpactnum;
    sh_int		mpscriptpos;
    MOB_INDEX_DATA *	pIndexData;
    DESCRIPTOR_DATA *	desc;
    AFFECT_DATA *	first_affect;
    AFFECT_DATA *	last_affect;
    NOTE_DATA *		pnote;
    NOTE_DATA *		comments;
    OBJ_DATA *		first_carrying;
    OBJ_DATA *		last_carrying;
    ROOM_INDEX_DATA *	in_room;
    ROOM_INDEX_DATA *	was_in_room;
    PC_DATA *		pcdata;
    DO_FUN *		last_cmd;
    DO_FUN *		prev_cmd;   /* mapping */
    void *		dest_buf;  /* This one is to assign to differen things */
    char *		alloc_ptr; /* Must str_dup and free this one */
    void *		spare_ptr;
    int			tempnum;
    EDITOR_DATA *	editor;
    TIMER	*	first_timer;
    TIMER	*	last_timer;
    CHAR_MORPH  *	morph;
    char *		name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    sh_int		num_fighting;
    sh_int		substate;
    sh_int		sex;
    sh_int		class;
    sh_int		race;
    sh_int		level;
    sh_int		trust;
    int			played;
    time_t		logon;
    time_t		save_time;
    sh_int		timer;
    sh_int		wait;
    sh_int		hit;
    sh_int		max_hit;
    sh_int		mana;
    sh_int		max_mana;
    sh_int		move;
    sh_int		max_move;
    sh_int		practice;
    sh_int		numattacks;
    int			gold;
    int			exp;
    EXT_BV		act;
    EXT_BV		affected_by;
    EXT_BV		no_affected_by;
    int			carry_weight;
    int			carry_number;
    int			xflags;
    int			no_immune;
    int			no_resistant;
    int			no_susceptible;
    int			immune;
    int			resistant;
    int			susceptible;
    EXT_BV		attacks;
    EXT_BV		defenses;
    int			speaks;
    int			speaking;
    sh_int		saving_poison_death;
    sh_int		saving_wand;
    sh_int		saving_para_petri;
    sh_int		saving_breath;
    sh_int		saving_spell_staff;
    sh_int		alignment;
    sh_int		barenumdie;
    sh_int		baresizedie;
    sh_int		mobthac0;
    sh_int		hitroll;
    sh_int		damroll;
    sh_int		hitplus;
    sh_int		damplus;
    sh_int		position;
    sh_int		defposition;
    sh_int		style;
    sh_int		height;
    sh_int		weight;
    sh_int		armor;
    sh_int		wimpy;
    int			deaf;
    sh_int		perm_str;
    sh_int		perm_int;
    sh_int		perm_wis;
    sh_int		perm_dex;
    sh_int		perm_con;
    sh_int		perm_cha;
    sh_int		perm_lck;
    sh_int		mod_str;
    sh_int		mod_int;
    sh_int		mod_wis;
    sh_int		mod_dex;
    sh_int		mod_con;
    sh_int		mod_cha;
    sh_int		mod_lck;
    sh_int		mental_state;		/* simplified */
    sh_int		emotional_state;	/* simplified */
    int			pagelen;                        /* BUILD INTERFACE */
    sh_int		inter_page;                     /* BUILD INTERFACE */
    sh_int		inter_type;                     /* BUILD INTERFACE */
    char  		*inter_editing;                 /* BUILD INTERFACE */
    int			inter_editing_vnum;             /* BUILD INTERFACE */
    sh_int		inter_substate;                 /* BUILD INTERFACE */
    int			retran;
    int			regoto;
    sh_int		mobinvis;	/* Mobinvis level SB */
};


struct killed_data
{
    sh_int		vnum;
    char		count;
};

/* Structure for link list of ignored players */
struct ignore_data
{
    IGNORE_DATA *next;
    IGNORE_DATA *prev;
    char *name;
};

/* Max number of people you can ignore at once */
#define MAX_IGN		6


/*
 * Data which only PC's have.
 */
struct	pc_data
{
    CHAR_DATA *		pet;
    CLAN_DATA *		clan;
    COUNCIL_DATA * 	council;
    AREA_DATA *		area;
    DEITY_DATA *	deity;
    char *		homepage;
    char *		clan_name;
    char * 		council_name;
    char *		deity_name;
    char *		pwd;
    char *		bamfin;
    char *		bamfout;
    char *		filename;       /* For the safe mset name -Shaddai */
    char *              rank;
    char *		title;
    char *		bestowments;	/* Special bestowed commands	   */
    int                 flags;		/* Whether the player is deadly and whatever else we add.      */
    int			pkills;		/* Number of pkills on behalf of clan */
    int			pdeaths;	/* Number of times pkilled (legally)  */
    int			mkills;		/* Number of mobs killed		   */
    int			mdeaths;	/* Number of deaths due to mobs       */
    int			illegal_pk;	/* Number of illegal pk's committed   */
    long int            outcast_time;	/* The time at which the char was outcast */
    NUISANCE_DATA       *nuisance;       /* New Nuisance structure */

    long int            restore_time;	/* The last time the char did a restore all */
    sh_int		r_range_lo;	/* room range */
    sh_int		r_range_hi;
    sh_int		m_range_lo;	/* mob range  */
    sh_int		m_range_hi;
    sh_int		o_range_lo;	/* obj range  */
    sh_int		o_range_hi;		
    sh_int		wizinvis;	/* wizinvis level */
    sh_int		min_snoop;	/* minimum snoop level */
    sh_int		condition	[MAX_CONDS];
    sh_int		learned		[MAX_SKILL];
    KILLED_DATA		killed		[MAX_KILLTRACK];
    sh_int		quest_number;	/* current *QUEST BEING DONE* DON'T REMOVE! */
    sh_int		quest_curr;	/* current number of quest points */
    int			quest_accum;	/* quest points accumulated in players life */
    sh_int		favor;		/* deity favor */
    sh_int		charmies;	/* Number of Charmies */
    int			auth_state;
    time_t		release_date;	/* Auto-helling.. Altrag */
    char *		helled_by;
    char *		bio;		/* Personal Bio */
    char *		authed_by;	/* what crazy imm authed this name ;) */
    SKILLTYPE *		special_skills[MAX_PERSONAL]; /* personalized skills/spells */
    char *		prompt;		/* User config prompts */
    char *		fprompt;	/* Fight prompts */
    char *		subprompt;	/* Substate prompt */
    sh_int		pagerlen;	/* For pager (NOT menus) */
    bool		openedtourney;
    IGNORE_DATA	*	first_ignored; 	/* keep track of who to ignore */
    IGNORE_DATA	*	last_ignored;    
    char **		tell_history;	/* for immortal only command lasttell */
    sh_int		lt_index;	/* last_tell index */
    
    long	imc_deaf;    /* IMC channel def flags */
    long	imc_allow;   /* IMC channel allow flags */
    long	imc_deny;    /* IMC channel deny flags */
    char *	rreply;      /* IMC reply-to */
    char *	rreply_name; /* IMC reply-to shown to char */
    char *	ice_listen;  /* ICE channels */
    
    sh_int	colorize	[AT_MAXCOLOR];
};



/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		18

struct	liq_type
{
    char *	liq_name;
    char *	liq_color;
    sh_int	liq_affect[3];
};


/*
 * Damage types from the attack_table[]
 */
typedef enum
{
   DAM_HIT, DAM_SLICE, DAM_STAB, DAM_SLASH, DAM_WHIP, DAM_CLAW,
   DAM_BLAST, DAM_POUND, DAM_CRUSH, DAM_GREP, DAM_BITE, DAM_PIERCE,
   DAM_SUCTION, DAM_BOLT, DAM_ARROW, DAM_DART, DAM_STONE, DAM_PEA
} damage_types;


/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	/* Next in list                     */
    EXTRA_DESCR_DATA *prev;	/* Previous in list                 */
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct	obj_index_data
{
    OBJ_INDEX_DATA *	next;
    OBJ_INDEX_DATA *	next_sort;
    EXTRA_DESCR_DATA *	first_extradesc;
    EXTRA_DESCR_DATA *	last_extradesc;
    AFFECT_DATA *	first_affect;
    AFFECT_DATA *	last_affect;
    MPROG_DATA *	mudprogs;               /* objprogs */
    EXT_BV		progtypes;              /* objprogs */
    char *		name;
    char *		short_descr;
    char *		description;
    char *		action_desc;
    int			vnum;
    sh_int              level;
    sh_int		item_type;
    EXT_BV		extra_flags;
    int			magic_flags; /*Need more bitvectors for spells - Scryn*/
    int			wear_flags;
    sh_int		count;
    sh_int		weight;
    int			cost;
    int			value	[6];
    int			serial;
    sh_int		layers;
    int			rent;			/* Unused */
};


/*
 * One object.
 */
struct	obj_data
{
    OBJ_DATA *		next;
    OBJ_DATA *		prev;
    OBJ_DATA *		next_content;
    OBJ_DATA *		prev_content;
    OBJ_DATA *		first_content;
    OBJ_DATA *		last_content;
    OBJ_DATA *		in_obj;
    CHAR_DATA *		carried_by;
    EXTRA_DESCR_DATA *	first_extradesc;
    EXTRA_DESCR_DATA *	last_extradesc;
    AFFECT_DATA *	first_affect;
    AFFECT_DATA *	last_affect;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
    char *		name;
    char *		short_descr;
    char *		description;
    char *		action_desc;
    sh_int		item_type;
    sh_int		mpscriptpos;
    EXT_BV		extra_flags;
    int			magic_flags; /*Need more bitvectors for spells - Scryn*/
    int			wear_flags; 
    MPROG_ACT_LIST *	mpact;		/* mudprogs */
    int			mpactnum;	/* mudprogs */
    sh_int		wear_loc;
    sh_int		weight;
    int			cost;
    sh_int		level;
    sh_int		timer;
    int			value	[6];
    sh_int		count;		/* support for object grouping */
    int			serial;		/* serial number	       */
};


/*
 * Exit data.
 */
struct	exit_data
{
    EXIT_DATA *		prev;		/* previous exit in linked list	*/
    EXIT_DATA *		next;		/* next exit in linked list	*/
    EXIT_DATA *		rexit;		/* Reverse exit pointer		*/
    ROOM_INDEX_DATA *	to_room;	/* Pointer to destination room	*/
    char *		keyword;	/* Keywords for exit or door	*/
    char *		description;	/* Description of exit		*/
    int			vnum;		/* Vnum of room exit leads to	*/
    int			rvnum;		/* Vnum of room in opposite dir	*/
    int			exit_info;	/* door states & other flags	*/
    int			key;		/* Key vnum			*/
    sh_int		vdir;		/* Physical "direction"		*/
    sh_int		distance;	/* how far to the next room	*/
    sh_int		pull;		/* pull of direction (current)	*/
    sh_int		pulltype;	/* type of pull (current, wind)	*/
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'H': hide an object
 *   'B': set a bitvector
 *   'T': trap an object
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
    RESET_DATA *	next;
    RESET_DATA *	prev;
    char		command;
    int			extra;
    int			arg1;
    int			arg2;
    int			arg3;
};

/* Constants for arg2 of 'B' resets. */
#define	BIT_RESET_DOOR			0
#define BIT_RESET_OBJECT		1
#define BIT_RESET_MOBILE		2
#define BIT_RESET_ROOM			3
#define BIT_RESET_TYPE_MASK		0xFF	/* 256 should be enough */
#define BIT_RESET_DOOR_THRESHOLD	8
#define BIT_RESET_DOOR_MASK		0xFF00	/* 256 should be enough */
#define BIT_RESET_SET			BV30
#define BIT_RESET_TOGGLE		BV31
#define BIT_RESET_FREEBITS	  0x3FFF0000	/* For reference */



/*
 * Area definition.
 */
struct	area_data
{
    AREA_DATA *		next;
    AREA_DATA *		prev;
    AREA_DATA *		next_sort;
    AREA_DATA *		prev_sort;
    AREA_DATA *         next_sort_name; /* Used for alphanum. sort */
    AREA_DATA *         prev_sort_name; /* Ditto, Fireblade */
    RESET_DATA *	first_reset;
    RESET_DATA *	last_reset;
    char *		name;
    char *		filename;
    int                 flags;
    sh_int              status;  /* h, 8/11 */
    sh_int		age;
    sh_int		nplayer;
    sh_int		reset_frequency;
    int			low_r_vnum;
    int			hi_r_vnum;
    int			low_o_vnum;
    int			hi_o_vnum;
    sh_int		low_m_vnum;
    sh_int		hi_m_vnum;
    int			low_soft_range;
    int			hi_soft_range;
    int			low_hard_range;
    int			hi_hard_range;
    int		        spelllimit;
    int			curr_spell_count;
    char *		author; /* Scryn */
    char *              resetmsg; /* Rennard */
    RESET_DATA *	last_mob_reset;
    RESET_DATA *	last_obj_reset;
    sh_int		max_players;
    int			mkills;
    int			mdeaths;
    int			pkills;
    int			pdeaths;
    int			gold_looted;
    int			illegal_pk;
    int			high_economy;
    int			low_economy;
    WEATHER_DATA *	weather; /* FB */
};



/*
 * Load in the gods building data. -- Altrag
 */
struct	godlist_data
{
    GOD_DATA *		next;
    GOD_DATA *		prev;
    int			level;
    int			low_r_vnum;
    int			hi_r_vnum;
    int			low_o_vnum;
    int			hi_o_vnum;
    sh_int		low_m_vnum;
    sh_int		hi_m_vnum;
};


/*
 * Used to keep track of system settings and statistics		-Thoric
 */
struct	system_data
{
    int		maxplayers;		/* Maximum players this boot   */
    int		alltimemax;		/* Maximum players ever	  */
    int		global_looted;		/* Gold looted this boot */
    int		upill_val;		/* Used pill value */
    int		upotion_val;		/* Used potion value */
    int		brewed_used;		/* Brewed potions used */
    int		scribed_used;		/* Scribed scrolls used */
    char *	time_of_max;		/* Time of max ever */
    char *	mud_name;		/* Name of mud */
    bool	NO_NAME_RESOLVING;	/* Hostnames are not resolved  */
    bool    	DENY_NEW_PLAYERS;	/* New players cannot connect  */
    bool	WAIT_FOR_AUTH;		/* New players must be auth'ed */
    sh_int	read_all_mail;		/* Read all player mail(was 54)*/
    sh_int	read_mail_free;		/* Read mail for free (was 51) */
    sh_int	write_mail_free;	/* Write mail for free(was 51) */
    sh_int	take_others_mail;	/* Take others mail (was 54)   */
    sh_int	imc_mail_vnum;		/* Board vnum for IMC mail     */
    sh_int	imc_mail_level;		/* Min level to send IMC mail  */
    sh_int	muse_level;		/* Level of muse channel */
    sh_int	think_level;		/* Level of think channel LEVEL_HIGOD*/
    sh_int	build_level;		/* Level of build channel LEVEL_BUILD*/
    sh_int	log_level;		/* Level of log channel LEVEL LOG*/
    sh_int	level_modify_proto;	/* Level to modify prototype stuff LEVEL_LESSER */
    sh_int	level_override_private;	/* override private flag */
    sh_int	level_mset_player;	/* Level to mset a player */
    sh_int	bash_plr_vs_plr;	/* Bash mod player vs. player */
    sh_int	bash_nontank;		/* Bash mod basher != primary attacker */
    sh_int      gouge_plr_vs_plr;       /* Gouge mod player vs. player */
    sh_int      gouge_nontank;	        /* Gouge mod player != primary attacker */
    sh_int	stun_plr_vs_plr;	/* Stun mod player vs. player */
    sh_int	stun_regular;		/* Stun difficult */
    sh_int	dodge_mod;		/* Divide dodge chance by */
    sh_int	parry_mod;		/* Divide parry chance by */
    sh_int	tumble_mod;		/* Divide tumble chance by */
    sh_int	dam_plr_vs_plr;		/* Damage mod player vs. player */
    sh_int	dam_plr_vs_mob;		/* Damage mod player vs. mobile */
    sh_int	dam_mob_vs_plr;		/* Damage mod mobile vs. player */
    sh_int	dam_mob_vs_mob;		/* Damage mod mobile vs. mobile */
    sh_int	level_getobjnotake;     /* Get objects without take flag */
    sh_int      level_forcepc;          /* The level at which you can use force on players. */
    sh_int	bestow_dif;		/* Max # of levels between trust and command level for a bestow to work --Blodkai */
    sh_int	max_sn;			/* Max skills */
    char       *guild_overseer;         /* Pointer to char containing the name of the */
    char       *guild_advisor;		/* guild overseer and advisor. */ 
    int		save_flags;		/* Toggles for saving conditions */
    sh_int	save_frequency;		/* How old to autosave someone */
    sh_int check_imm_host;      /* Do we check immortal's hosts? */
    sh_int morph_opt;           /* Do we optimize morph's? */
    sh_int save_pets;		/* Do pets save? */
    sh_int ban_site_level;      /* Level to ban sites */
    sh_int ban_class_level;     /* Level to ban classes */
    sh_int ban_race_level;      /* Level to ban races */
    sh_int ident_retries;	/* Number of times to retry broken pipes. */
    sh_int pk_loot;		/* Pkill looting allowed? */
};



struct	plane_data
{
    PLANE_DATA *	next;
    PLANE_DATA *	prev;
    char *		name;
};



/*
 * Room type.
 */
struct	room_index_data
{
    ROOM_INDEX_DATA *	next;
    ROOM_INDEX_DATA *	next_sort;
    CHAR_DATA *		first_person;	    /* people in the room	*/
    CHAR_DATA *		last_person;	    /*		..		*/
    OBJ_DATA *		first_content;	    /* objects on floor		*/
    OBJ_DATA *		last_content;	    /*		..		*/
    EXTRA_DESCR_DATA *	first_extradesc;    /* extra descriptions	*/
    EXTRA_DESCR_DATA *	last_extradesc;	    /*		..		*/
    AREA_DATA *		area;
    EXIT_DATA *		first_exit;	    /* exits from the room	*/
    EXIT_DATA *		last_exit;	    /*		..		*/
    AFFECT_DATA *	first_affect;	    /* effects on the room	*/
    AFFECT_DATA *	last_affect;	    /*		..		*/
    MAP_DATA *		map;		    /* maps */
    PLANE_DATA *	plane;		    /* do it by room rather than area */
    MPROG_ACT_LIST *	mpact;		    /* mudprogs */
    int			mpactnum;	    /* mudprogs */
    MPROG_DATA *	mudprogs;	    /* mudprogs */
    sh_int		mpscriptpos;
    char *		name;
    char *		description;
    int			vnum;
    int			room_flags;
    EXT_BV		progtypes;           /* mudprogs */
    sh_int		light;		     /* amount of light in the room */
    sh_int		sector_type;
    int			tele_vnum;
    sh_int		tele_delay;
    sh_int		tunnel;		     /* max people that will fit */
};

/*
 * Delayed teleport type.
 */
struct	teleport_data
{
    TELEPORT_DATA *	next;
    TELEPORT_DATA *	prev;
    ROOM_INDEX_DATA *	room;
    sh_int		timer;
};


/*
 * Types of skill numbers.  Used to keep separate lists of sn's
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000  /* allows for 1000 skills/spells */
#define TYPE_HERB		     2000  /* allows for 1000 attack types  */
#define TYPE_PERSONAL		     3000  /* allows for 1000 herb types    */
#define TYPE_RACIAL		     4000  /* allows for 1000 personal types*/
#define TYPE_DISEASE		     5000  /* allows for 1000 racial types  */

/*
 *  Target types.
 */
typedef enum
{
  TAR_IGNORE, TAR_CHAR_OFFENSIVE, TAR_CHAR_DEFENSIVE, TAR_CHAR_SELF,
  TAR_OBJ_INV,
} target_types;

typedef enum
{
  SKILL_UNKNOWN, SKILL_SPELL, SKILL_SKILL, SKILL_WEAPON, SKILL_TONGUE,
  SKILL_HERB, SKILL_RACIAL, SKILL_DISEASE
} skill_types;



struct timerset
{
  int num_uses;
  struct timeval total_time;
  struct timeval min_time;
  struct timeval max_time;
};



/*
 * Skills include spells as a particular case.
 */
struct	skill_type
{
    char *	name;			/* Name of skill		*/
    sh_int	skill_level[MAX_CLASS];	/* Level needed by class	*/
    sh_int	skill_adept[MAX_CLASS];	/* Max attainable % in this skill */
    sh_int	race_level[MAX_RACE];	/* Racial abilities: level      */ 
    sh_int	race_adept[MAX_RACE];	/* Racial abilities: adept      */
    SPELL_FUN *	spell_fun;		/* Spell pointer (for spells)	*/
    DO_FUN *	skill_fun;		/* Skill pointer (for skills)	*/
    sh_int	target;			/* Legal targets		*/
    sh_int	minimum_position;	/* Position for caster / user	*/
    sh_int	slot;			/* Slot for #OBJECT loading	*/
    sh_int	min_mana;		/* Minimum mana used		*/
    sh_int	beats;			/* Rounds required to use skill	*/
    char *	noun_damage;		/* Damage message		*/
    char *	msg_off;		/* Wear off message		*/
    sh_int	guild;			/* Which guild the skill belongs to */
    sh_int	min_level;		/* Minimum level to be able to cast */
    sh_int	type;			/* Spell/Skill/Weapon/Tongue	*/
    sh_int	range;			/* Range of spell (rooms)	*/
    int		info;			/* Spell action/class/etc	*/
    int		flags;			/* Flags			*/
    char *	hit_char;		/* Success message to caster	*/
    char *	hit_vict;		/* Success message to victim	*/
    char *	hit_room;		/* Success message to room	*/
    char *	hit_dest;		/* Success message to dest room	*/
    char *	miss_char;		/* Failure message to caster	*/
    char *	miss_vict;		/* Failure message to victim	*/
    char *	miss_room;		/* Failure message to room	*/
    char *	die_char;		/* Victim death msg to caster	*/
    char *	die_vict;		/* Victim death msg to victim	*/
    char *	die_room;		/* Victim death msg to room	*/
    char *	imm_char;		/* Victim immune msg to caster	*/
    char *	imm_vict;		/* Victim immune msg to victim	*/
    char *	imm_room;		/* Victim immune msg to room	*/
    char *	dice;			/* Dice roll			*/
    int		value;			/* Misc value			*/
    int		spell_sector;		/* Sector Spell work	 	*/
    char	saves;			/* What saving spell applies	*/
    char	difficulty;		/* Difficulty of casting/learning */
    SMAUG_AFF *	affects;		/* Spell affects, if any	*/
    char *	components;		/* Spell components, if any	*/
    char *	teachers;		/* Skill requires a special teacher */
    char	participants;		/* # of required participants	*/
    struct	timerset	userec;	/* Usage record			*/
};


/* how many items to track.... prevent repeat auctions */
#define AUCTION_MEM 3

struct  auction_data
{
    OBJ_DATA  * item;   /* a pointer to the item */
    CHAR_DATA * seller; /* a pointer to the seller - which may NOT quit */
    CHAR_DATA * buyer;  /* a pointer to the buyer - which may NOT quit */
    int         bet;    /* last bet - or 0 if noone has bet anything */
    sh_int      going;  /* 1,2, sold */
    sh_int      pulse;  /* how many pulses (.25 sec) until another call-out ? */
    int 	starting;
    OBJ_INDEX_DATA *	history[AUCTION_MEM];	/* store auction history */
    sh_int	hist_timer;		/* clear out history buffer if auction is idle */	
};

/*
 * So we can have different configs for different ports -- Shaddai
 */
extern int port;

/*
 * These are skill_lookup return values for common skills and spells.
 */
extern	sh_int	gsn_style_evasive;
extern	sh_int	gsn_style_defensive;
extern	sh_int	gsn_style_standard;
extern	sh_int	gsn_style_aggressive;
extern	sh_int	gsn_style_berserk;

extern	sh_int	gsn_detrap;
extern	sh_int	gsn_backstab;
extern  sh_int  gsn_circle;
extern  sh_int  gsn_cook;
extern	sh_int	gsn_dodge;
extern	sh_int	gsn_hide;
extern	sh_int	gsn_peek;
extern	sh_int	gsn_pick_lock;
extern  sh_int  gsn_scan;
extern	sh_int	gsn_sneak;
extern	sh_int	gsn_steal;
extern	sh_int	gsn_gouge;
extern	sh_int	gsn_track;
extern	sh_int	gsn_search;
extern  sh_int  gsn_dig;
extern	sh_int	gsn_mount;
extern  sh_int  gsn_bashdoor;
extern	sh_int	gsn_berserk;
extern	sh_int	gsn_hitall;

extern	sh_int	gsn_disarm;
extern	sh_int	gsn_enhanced_damage;
extern	sh_int	gsn_kick;
extern	sh_int	gsn_parry;
extern	sh_int	gsn_rescue;
extern	sh_int	gsn_second_attack;
extern	sh_int	gsn_third_attack;
extern	sh_int	gsn_fourth_attack;
extern	sh_int	gsn_fifth_attack;
extern	sh_int	gsn_dual_wield;

extern	sh_int	gsn_feed;
extern  sh_int  gsn_bloodlet;
extern  sh_int  gsn_broach;
extern  sh_int  gsn_mistwalk;

extern	sh_int	gsn_aid;

/* used to do specific lookups */
extern	sh_int	gsn_first_spell;
extern	sh_int	gsn_first_skill;
extern	sh_int	gsn_first_weapon;
extern	sh_int	gsn_first_tongue;
extern	sh_int	gsn_top_sn;

/* spells */
extern	sh_int	gsn_blindness;
extern	sh_int	gsn_charm_person;
extern  sh_int  gsn_aqua_breath;
extern	sh_int	gsn_curse;
extern	sh_int	gsn_invis;
extern	sh_int	gsn_mass_invis;
extern	sh_int	gsn_poison;
extern	sh_int	gsn_sleep;
extern  sh_int  gsn_possess;
extern	sh_int	gsn_fireball;		/* for fireshield  */
extern	sh_int	gsn_chill_touch;	/* for iceshield   */
extern	sh_int	gsn_lightning_bolt;	/* for shockshield */

/* newer attack skills */
extern	sh_int	gsn_punch;
extern	sh_int	gsn_bash;
extern	sh_int	gsn_stun;
extern	sh_int	gsn_bite;
extern	sh_int	gsn_claw;
extern	sh_int	gsn_sting;
extern	sh_int	gsn_tail;

extern  sh_int  gsn_poison_weapon;
extern  sh_int  gsn_scribe;
extern  sh_int  gsn_brew;
extern	sh_int	gsn_climb;

extern	sh_int	gsn_pugilism;
extern	sh_int	gsn_long_blades;
extern	sh_int	gsn_short_blades;
extern	sh_int	gsn_flexible_arms;
extern	sh_int	gsn_talonous_arms;
extern	sh_int	gsn_bludgeons;
extern  sh_int  gsn_missile_weapons;
extern	sh_int	gsn_shieldwork;

extern  sh_int  gsn_grip;
extern  sh_int  gsn_slice;

extern  sh_int  gsn_tumble;

/* Language gsns. -- Altrag */
extern  sh_int  gsn_common;
extern  sh_int  gsn_elven;
extern  sh_int  gsn_dwarven;
extern  sh_int  gsn_pixie;
extern  sh_int  gsn_ogre;
extern  sh_int  gsn_orcish;
extern  sh_int  gsn_trollish;
extern  sh_int  gsn_goblin;
extern  sh_int  gsn_halfling;

/*
 * Cmd flag names --Shaddai
 */
extern char *const cmd_flags[];

/*
 * Utility macros.
 */
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))


/*
 * Old-style Bit manipulation macros
 *
 * The bit passed is the actual value of the bit (Use the BV## defines)
 */ 
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)	((var) ^= (bit))

/*
 * Macros for accessing virtually unlimited bitvectors.		-Thoric
 *
 * Note that these macros use the bit number rather than the bit value
 * itself -- which means that you can only access _one_ bit at a time
 *
 * This code uses an array of integers
 */

/*
 * The functions for these prototypes can be found in misc.c
 * They are up here because they are used by the macros below
 */
bool	ext_is_empty		args( ( EXT_BV *bits ) );
void	ext_clear_bits		args( ( EXT_BV *bits ) );
int	ext_has_bits		args( ( EXT_BV *var, EXT_BV *bits) );
bool	ext_same_bits		args( ( EXT_BV *var, EXT_BV *bits) );
void	ext_set_bits		args( ( EXT_BV *var, EXT_BV *bits) );
void	ext_remove_bits		args( ( EXT_BV *var, EXT_BV *bits) );
void	ext_toggle_bits		args( ( EXT_BV *var, EXT_BV *bits) );

/*
 * Here are the extended bitvector macros:
 */
#define xIS_SET(var, bit)	((var).bits[(bit) >> RSV] & 1 << ((bit) & XBM))
#define xSET_BIT(var, bit)	((var).bits[(bit) >> RSV] |= 1 << ((bit) & XBM))
#define xSET_BITS(var, bit)	(ext_set_bits(&(var), &(bit)))
#define xREMOVE_BIT(var, bit)	((var).bits[(bit) >> RSV] &= ~(1 << ((bit) & XBM)))
#define xREMOVE_BITS(var, bit)	(ext_remove_bits(&(var), &(bit)))
#define xTOGGLE_BIT(var, bit)	((var).bits[(bit) >> RSV] ^= 1 << ((bit) & XBM))
#define xTOGGLE_BITS(var, bit)	(ext_toggle_bits(&(var), &(bit)))
#define xCLEAR_BITS(var)	(ext_clear_bits(&(var)))
#define xIS_EMPTY(var)		(ext_is_empty(&(var)))
#define xHAS_BITS(var, bit)	(ext_has_bits(&(var), &(bit)))
#define xSAME_BITS(var, bit)	(ext_same_bits(&(var), &(bit)))

/*
 * Memory allocation macros.
 */

#define CREATE(result, type, number)				\
do								\
{								\
    if (!((result) = (type *) calloc ((number), sizeof(type))))	\
    {								\
	perror("malloc failure");				\
	fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();						\
    }								\
} while(0)

#define RECREATE(result,type,number)				\
do								\
{								\
    if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
    {								\
	perror("realloc failure");				\
	fprintf(stderr, "Realloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();						\
    }								\
} while(0)


#define DISPOSE(point) 						\
do								\
{								\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
	fprintf( stderr, "DISPOSEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free(point);						\
  point = NULL;							\
} while(0)

#ifdef HASHSTR
#define STRALLOC(point)		str_alloc((point))
#define QUICKLINK(point)	quick_link((point))
#define QUICKMATCH(p1, p2)	(int) (p1) == (int) (p2)
#define STRFREE(point)						\
do								\
{								\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
	fprintf( stderr, "STRFREEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else if (str_free((point))==-1) 				\
    fprintf( stderr, "STRFREEing bad pointer in %s, line %d\n", __FILE__, __LINE__ ); \
  point = NULL;							\
} while(0)
#else
#define STRALLOC(point)		str_dup((point))
#define QUICKLINK(point)	str_dup((point))
#define QUICKMATCH(p1, p2)	strcmp((p1), (p2)) == 0
#define STRFREE(point)						\
do								\
{								\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
	fprintf( stderr, "STRFREEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free((point));						\
  point = NULL;							\
} while(0)
#endif

/* double-linked list handling macros -Thoric */

#define LINK(link, first, last, next, prev)			\
do								\
{								\
    if ( !(first) )						\
      (first)			= (link);			\
    else							\
      (last)->next		= (link);			\
    (link)->next		= NULL;				\
    (link)->prev		= (last);			\
    (last)			= (link);			\
} while(0)

#define INSERT(link, insert, first, next, prev)			\
do								\
{								\
    (link)->prev		= (insert)->prev;		\
    if ( !(insert)->prev )					\
      (first)			= (link);			\
    else							\
      (insert)->prev->next	= (link);			\
    (insert)->prev		= (link);			\
    (link)->next		= (insert);			\
} while(0)

#define UNLINK(link, first, last, next, prev)			\
do								\
{								\
    if ( !(link)->prev )					\
      (first)			= (link)->next;			\
    else							\
      (link)->prev->next	= (link)->next;			\
    if ( !(link)->next )					\
      (last)			= (link)->prev;			\
    else							\
      (link)->next->prev	= (link)->prev;			\
} while(0)


#define CHECK_LINKS(first, last, next, prev, type)		\
do {								\
  type *ptr, *pptr = NULL;					\
  if ( !(first) && !(last) )					\
    break;							\
  if ( !(first) )						\
  {								\
    bug( "CHECK_LINKS: last with NULL first!  %s.",		\
        __STRING(first) );					\
    for ( ptr = (last); ptr->prev; ptr = ptr->prev );		\
    (first) = ptr;						\
  }								\
  else if ( !(last) )						\
  {								\
    bug( "CHECK_LINKS: first with NULL last!  %s.",		\
        __STRING(first) );					\
    for ( ptr = (first); ptr->next; ptr = ptr->next );		\
    (last) = ptr;						\
  }								\
  if ( (first) )						\
  {								\
    for ( ptr = (first); ptr; ptr = ptr->next )			\
    {								\
      if ( ptr->prev != pptr )					\
      {								\
        bug( "CHECK_LINKS(%s): %p:->prev != %p.  Fixing.",	\
            __STRING(first), ptr, pptr );			\
        ptr->prev = pptr;					\
      }								\
      if ( ptr->prev && ptr->prev->next != ptr )		\
      {								\
        bug( "CHECK_LINKS(%s): %p:->prev->next != %p.  Fixing.",\
            __STRING(first), ptr, ptr );			\
        ptr->prev->next = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
    pptr = NULL;						\
  }								\
  if ( (last) )							\
  {								\
    for ( ptr = (last); ptr; ptr = ptr->prev )			\
    {								\
      if ( ptr->next != pptr )					\
      {								\
        bug( "CHECK_LINKS (%s): %p:->next != %p.  Fixing.",	\
            __STRING(first), ptr, pptr );			\
        ptr->next = pptr;					\
      }								\
      if ( ptr->next && ptr->next->prev != ptr )		\
      {								\
        bug( "CHECK_LINKS(%s): %p:->next->prev != %p.  Fixing.",\
            __STRING(first), ptr, ptr );			\
        ptr->next->prev = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
  }								\
} while(0)


#define ASSIGN_GSN(gsn, skill)					\
do								\
{								\
    if ( ((gsn) = skill_lookup((skill))) == -1 )		\
	fprintf( stderr, "ASSIGN_GSN: Skill %s not found.\n",	\
		(skill) );					\
} while(0)

#define CHECK_SUBRESTRICTED(ch)					\
do								\
{								\
    if ( (ch)->substate == SUB_RESTRICTED )			\
    {								\
	send_to_char( "You cannot use this command from within another command.\n\r", ch );	\
	return;							\
    }								\
} while(0)


/*
 * Character macros.
 */
#define IS_NPC(ch)		(xIS_SET((ch)->act, ACT_IS_NPC))
#define IS_IMMORTAL(ch)		(get_trust((ch)) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust((ch)) >= LEVEL_HERO)
#define IS_AFFECTED(ch, sn)	(xIS_SET((ch)->affected_by, (sn)))
#define HAS_BODYPART(ch, part)	((ch)->xflags == 0 || IS_SET((ch)->xflags, (part)))

#define CAN_CAST(ch)		((ch)->class != 2 && (ch)->class != 3)

#define IS_VAMPIRE(ch)		(!IS_NPC(ch)				    \
				&& ((ch)->race==RACE_VAMPIRE		    \
				||  (ch)->class==CLASS_VAMPIRE))
#define IS_GOOD(ch)		((ch)->alignment >= 350)
#define IS_EVIL(ch)		((ch)->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)		((ch)->position > POS_SLEEPING)
#define GET_AC(ch)		((ch)->armor				    \
				    + ( IS_AWAKE(ch)			    \
				    ? dex_app[get_curr_dex(ch)].defensive   \
				    : 0 )				    \
				    + VAMP_AC(ch))
#define GET_HITROLL(ch)		((ch)->hitroll				    \
				    +str_app[get_curr_str(ch)].tohit	    \
				    +(2-(abs((ch)->mental_state)/10)))

/* Thanks to Chriss Baeke for noticing damplus was unused */
#define GET_DAMROLL(ch)		((ch)->damroll                              \
				    +(ch)->damplus			    \
				    +str_app[get_curr_str(ch)].todam	    \
				    +(((ch)->mental_state > 5		    \
				    &&(ch)->mental_state < 15) ? 1 : 0) )

#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS) && !IS_SET(               \
				    (ch)->in_room->room_flags,              \
				    ROOM_TUNNEL))

#define NO_WEATHER_SECT(sect)  (  sect == SECT_INSIDE || 	           \
				  sect == SECT_UNDERWATER ||               \
                                  sect == SECT_OCEANFLOOR ||               \
                                  sect == SECT_UNDERGROUND )

#define IS_DRUNK(ch, drunk)     (number_percent() < \
			        ( (ch)->pcdata->condition[COND_DRUNK] \
				* 2 / (drunk) ) )

#define IS_CLANNED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type != CLAN_ORDER  \
				&& (ch)->pcdata->clan->clan_type != CLAN_GUILD)

#define IS_ORDERED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type == CLAN_ORDER)

#define IS_GUILDED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type == CLAN_GUILD)

#define IS_DEADLYCLAN(ch)	(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type != CLAN_NOKILL) \
				&& (ch)->pcdata->clan->clan_type != CLAN_ORDER)  \
				&& (ch)->pcdata->clan->clan_type != CLAN_GUILD)

#define IS_DEVOTED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->deity)

#define IS_IDLE(ch)		(ch->pcdata && IS_SET( ch->pcdata->flags, PCFLAG_IDLE ))
#define IS_PKILL(ch)            (ch->pcdata && IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ))

#define CAN_PKILL(ch)           (IS_PKILL(ch) && ch->level >= 5 && get_age( ch ) >= 18 )

/* Addition to make people with nuisance flag have more wait */

#define WAIT_STATE(ch, npulse) ((ch)->wait=(!IS_NPC(ch)&&ch->pcdata->nuisance&&\
			      (ch->pcdata->nuisance->flags>4))?UMAX((ch)->wait,\
			      (npulse+((ch)->pcdata->nuisance->flags-4)+ \
               		      ch->pcdata->nuisance->power)): \
			      UMAX((ch)->wait, (npulse)))


#define EXIT(ch, door)		( get_exit( (ch)->in_room, door ) )

#define CAN_GO(ch, door)	(EXIT((ch),(door))			 \
				&& (EXIT((ch),(door))->to_room != NULL)  \
                          	&& !IS_SET(EXIT((ch), (door))->exit_info, EX_CLOSED))

#define IS_FLOATING(ch)		( IS_AFFECTED((ch), AFF_FLYING) || IS_AFFECTED((ch), AFF_FLOATING) )

#define IS_VALID_SN(sn)		( (sn) >=0 && (sn) < MAX_SKILL		     \
				&& skill_table[(sn)]			     \
				&& skill_table[(sn)]->name )

#define IS_VALID_HERB(sn)	( (sn) >=0 && (sn) < MAX_HERB		     \
				&& herb_table[(sn)]			     \
				&& herb_table[(sn)]->name )

#define IS_VALID_DISEASE(sn)	( (sn) >=0 && (sn) < MAX_DISEASE	     \
				&& disease_table[(sn)]			     \
				&& disease_table[(sn)]->name )

#define IS_PACIFIST(ch)		(IS_NPC(ch) && xIS_SET(ch->act, ACT_PACIFIST))

#define SPELL_FLAG(skill, flag)	( IS_SET((skill)->flags, (flag)) )
#define SPELL_DAMAGE(skill)	( ((skill)->info      ) & 7 )
#define SPELL_ACTION(skill)	( ((skill)->info >>  3) & 7 )
#define SPELL_CLASS(skill)	( ((skill)->info >>  6) & 7 )
#define SPELL_POWER(skill)	( ((skill)->info >>  9) & 3 )
#define SPELL_SAVE(skill)	( ((skill)->info >> 11) & 7 )
#define SET_SDAM(skill, val)	( (skill)->info =  ((skill)->info & SDAM_MASK) + ((val) & 7) )
#define SET_SACT(skill, val)	( (skill)->info =  ((skill)->info & SACT_MASK) + (((val) & 7) << 3) )
#define SET_SCLA(skill, val)	( (skill)->info =  ((skill)->info & SCLA_MASK) + (((val) & 7) << 6) )
#define SET_SPOW(skill, val)	( (skill)->info =  ((skill)->info & SPOW_MASK) + (((val) & 3) << 9) )
#define SET_SSAV(skill, val)	( (skill)->info =  ((skill)->info & SSAV_MASK) + (((val) & 7) << 11) )

/* Retired and guest imms. */
#define IS_RETIRED(ch) (ch->pcdata && IS_SET(ch->pcdata->flags,PCFLAG_RETIRED))
#define IS_GUEST(ch) (ch->pcdata && IS_SET(ch->pcdata->flags,PCFLAG_GUEST))

/* RIS by gsn lookups. -- Altrag.
   Will need to add some || stuff for spells that need a special GSN. */

#define IS_FIRE(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_FIRE )
#define IS_COLD(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_COLD )
#define IS_ACID(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ACID )
#define IS_ELECTRICITY(dt)	( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ELECTRICITY )
#define IS_ENERGY(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ENERGY )

#define IS_DRAIN(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_DRAIN )

#define IS_POISON(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_POISON )


#define NOT_AUTHED(ch)		(!IS_NPC(ch) && ch->pcdata->auth_state <= 3  \
			      && IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) )

#define IS_WAITING_FOR_AUTH(ch) (!IS_NPC(ch) && ch->desc		     \
			      && ch->pcdata->auth_state == 1		     \
			      && IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) ) 

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(xIS_SET((obj)->extra_flags, (stat)))

/*
 * MudProg macros.						-Thoric
 */
#define HAS_PROG(what, prog)	(xIS_SET((what)->progtypes, (prog)))

/*
 * Description macros.
 */
#define PERS(ch, looker)	( can_see( (looker), (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descr	\
				: (ch)->name ) : "someone" )

#define MORPHPERS(ch, looker)   ( can_see( (looker), (ch) ) ?           \
                                (ch)->morph->morph->short_desc       \
                                : "someone" )


#define log_string(txt)		( log_string_plus( (txt), LOG_NORMAL, LEVEL_LOG ) )
#define dam_message(ch, victim, dam, dt)	( new_dam_message((ch), (victim), (dam), (dt), NULL) )

/*
 *  Defines for the command flags. --Shaddai
 */
#define	CMD_FLAG_POSSESS	BV00
#define CMD_FLAG_POLYMORPHED	BV01
#define CMD_WATCH		BV02	/* FB */

/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    CMDTYPE *		next;
    char *		name;
    DO_FUN *		do_fun;
    int                 flags;  /* Added for Checking interpret stuff -Shaddai*/
    sh_int		position;
    sh_int		level;
    sh_int		log;
    struct		timerset	userec;
    int			lag_count;	/* count lag flags for this cmd - FB */
};



/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
    SOCIALTYPE *	next;
    char *		name;
    char *		char_no_arg;
    char *		others_no_arg;
    char *		char_found;
    char *		others_found;
    char *		vict_found;
    char *		char_auto;
    char *		others_auto;
};



/*
 * Global constants.
 */
extern  time_t last_restore_all_time;
extern  time_t boot_time;  /* this should be moved down */
extern  HOUR_MIN_SEC * set_boot_time; 
extern  struct  tm *new_boot_time;
extern  time_t new_boot_time_t;

extern	const	struct	str_app_type	str_app		[26];
extern	const	struct	int_app_type	int_app		[26];
extern	const	struct	wis_app_type	wis_app		[26];
extern	const	struct	dex_app_type	dex_app		[26];
extern	const	struct	con_app_type	con_app		[26];
extern	const	struct	cha_app_type	cha_app		[26];
extern  const	struct	lck_app_type	lck_app		[26];

extern	const struct	race_type _race_table	[MAX_RACE];
extern	struct	race_type *	race_table	[MAX_RACE];
extern	struct	at_color_type	at_color_table	[AT_MAXCOLOR];
extern	const	struct	liq_type	liq_table	[LIQ_MAX];
extern	char *	const			attack_table	[18];

extern  char ** const			s_message_table [18];
extern	char ** const			p_message_table	[18];

extern	char *	const	skill_tname	[];
extern	sh_int	const	movement_loss	[SECT_MAX];
extern	char *	const	dir_name	[];
extern	char *	const	where_name	[MAX_WHERE_NAME];
extern	const	sh_int	rev_dir		[];
extern	const	int	trap_door	[];
extern	char *	const	r_flags		[];
extern	char *	const	w_flags		[];
extern	char *	const	item_w_flags	[];
extern	char *	const	o_flags		[];
extern	char *	const	a_flags		[];
extern	char *	const	o_types		[];
extern	char *	const	a_types		[];
extern	char *	const	act_flags	[];
extern	char *	const	plr_flags	[];
extern	char *	const	pc_flags	[];
extern	char *	const	trap_flags	[];
extern	char *	const	ris_flags	[];
extern	char *	const	trig_flags	[];
extern	char *	const	part_flags	[];
extern	char *	const	npc_race	[];
extern	char *	const	npc_class	[];
extern	char *	const	defense_flags	[];
extern	char *	const	attack_flags	[];
extern	char *	const	area_flags	[];
extern	char *	const	ex_pmisc	[];
extern	char *	const	ex_pwater	[];
extern	char *	const	ex_pair		[];
extern	char *	const	ex_pearth	[];
extern	char *	const	ex_pfire	[];

extern	int	const	lang_array      [];
extern	char *	const	lang_names      [];

extern	char *	const	temp_settings	[]; /* FB */
extern	char *	const	precip_settings	[];
extern	char *	const	wind_settings	[];
extern	char *	const	preciptemp_msg	[6][6];
extern	char *	const	windtemp_msg	[6][6];
extern	char *	const	precip_msg	[];
extern	char *	const	wind_msg	[];

/*
 * Global variables.
 */
extern char *   bigregex;
extern char *   preg;

extern char *	target_name;
extern char *	ranged_target_name;
extern	int	numobjsloaded;
extern	int	nummobsloaded;
extern	int	physicalobjects;
extern 	int	last_pkroom;
extern	int	num_descriptors;
extern	struct	system_data		sysdata;
extern	int	top_sn;
extern	int	top_vroom;
extern	int	top_herb;

extern		CMDTYPE		  *	command_hash	[126];

extern		struct	class_type *	class_table	[MAX_CLASS];
extern		char *			title_table	[MAX_CLASS]
							[MAX_LEVEL+1]
							[2];

extern		SKILLTYPE	  *	skill_table	[MAX_SKILL];
extern		SOCIALTYPE	  *	social_index	[27];
extern		CHAR_DATA	  *	cur_char;
extern		ROOM_INDEX_DATA	  *	cur_room;
extern		bool			cur_char_died;
extern		ch_ret			global_retcode;
extern		SKILLTYPE	  *	herb_table	[MAX_HERB];
extern		SKILLTYPE	  *	disease_table	[MAX_DISEASE];

extern		int			cur_obj;
extern		int			cur_obj_serial;
extern		bool			cur_obj_extracted;
extern		obj_ret			global_objcode;

extern		HELP_DATA	  *	first_help;
extern		HELP_DATA	  *	last_help;
extern		SHOP_DATA	  *	first_shop;
extern		SHOP_DATA	  *	last_shop;
extern		REPAIR_DATA	  *	first_repair;
extern		REPAIR_DATA	  *	last_repair;

extern		WATCH_DATA	  *	first_watch;
extern		WATCH_DATA	  *	last_watch;
extern		BAN_DATA	  *	first_ban;
extern		BAN_DATA	  *	last_ban;
extern 		BAN_DATA 	  *     first_ban_class;
extern 		BAN_DATA 	  *	last_ban_class;
extern 		BAN_DATA 	  *	first_ban_race;
extern 		BAN_DATA 	  *	last_ban_race;
extern		RESERVE_DATA	  *	first_reserved;
extern		RESERVE_DATA	  *	last_reserved;
extern		CHAR_DATA	  *	first_char;
extern		CHAR_DATA	  *	last_char;
extern		DESCRIPTOR_DATA   *	first_descriptor;
extern		DESCRIPTOR_DATA   *	last_descriptor;
extern		BOARD_DATA	  *	first_board;
extern		BOARD_DATA	  *	last_board;
extern		PLANE_DATA	  *	first_plane;
extern		PLANE_DATA	  *	last_plane;
extern		PROJECT_DATA	  * 	first_project;
extern		PROJECT_DATA	  * 	last_project;
extern		OBJ_DATA	  *	first_object;
extern		OBJ_DATA	  *	last_object;
extern		CLAN_DATA	  *	first_clan;
extern		CLAN_DATA	  *	last_clan;
extern 		COUNCIL_DATA 	  *	first_council;
extern		COUNCIL_DATA	  * 	last_council;
extern		DEITY_DATA	  *	first_deity;
extern		DEITY_DATA	  *	last_deity;
extern		AREA_DATA	  *	first_area;
extern		AREA_DATA	  *	last_area;
extern		AREA_DATA	  *	first_build;
extern		AREA_DATA	  *	last_build;
extern		AREA_DATA	  *	first_asort;
extern		AREA_DATA	  *	last_asort;
extern		AREA_DATA	  *	first_bsort;
extern		AREA_DATA	  *	last_bsort;
extern          AREA_DATA         *     first_area_name; /*alphanum. sort*/
extern          AREA_DATA         *     last_area_name;  /* Fireblade */

extern		LANG_DATA	  *	first_lang;
extern		LANG_DATA	  *	last_lang;

/*
extern		GOD_DATA	  *	first_imm;
extern		GOD_DATA	  *	last_imm;
*/
extern		TELEPORT_DATA	  *	first_teleport;
extern		TELEPORT_DATA	  *	last_teleport;
extern		OBJ_DATA	  *	extracted_obj_queue;
extern		EXTRACT_CHAR_DATA *	extracted_char_queue;
extern		OBJ_DATA	  *	save_equipment[MAX_WEAR][MAX_LAYERS];
extern		CHAR_DATA	  *	quitting_char;
extern		CHAR_DATA	  *	loading_char;
extern		CHAR_DATA	  *	saving_char;
extern		OBJ_DATA	  *	all_obj;

extern		char			bug_buf		[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		FILE *			fpLOG;
extern		char			log_buf		[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;
extern          IMMORTAL_HOST *         immortal_host_start;
extern          IMMORTAL_HOST *         immortal_host_end;
extern		int			weath_unit;
extern		int			rand_factor;
extern		int			climate_factor;
extern		int			neigh_factor;
extern		int			max_vector;

extern          AUCTION_DATA      *     auction;
extern		struct act_prog_data *	mob_act_list;


/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN( skill_notfound	);
DECLARE_DO_FUN( do_aassign	);
DECLARE_DO_FUN( do_add_imm_host );
DECLARE_DO_FUN( do_adminlist	);
DECLARE_DO_FUN(	do_advance	);
DECLARE_DO_FUN( do_affected     );
DECLARE_DO_FUN( do_afk          );
DECLARE_DO_FUN(	do_aid		);
DECLARE_DO_FUN(	do_allow	);
DECLARE_DO_FUN( do_ansi		);
DECLARE_DO_FUN(	do_answer	);
DECLARE_DO_FUN( do_apply	);
DECLARE_DO_FUN(	do_appraise	);
DECLARE_DO_FUN(	do_areas	);
DECLARE_DO_FUN( do_aset		);
DECLARE_DO_FUN(	do_ask		);
DECLARE_DO_FUN( do_astat	);
DECLARE_DO_FUN(	do_at		);
DECLARE_DO_FUN(	do_atobj	);
DECLARE_DO_FUN(	do_auction	);
DECLARE_DO_FUN( do_authorize	);
DECLARE_DO_FUN( do_avtalk	);
DECLARE_DO_FUN(	do_backstab	);
DECLARE_DO_FUN(	do_balzhur	);
DECLARE_DO_FUN(	do_bamfin	);
DECLARE_DO_FUN(	do_bamfout	);
DECLARE_DO_FUN(	do_watch	);
DECLARE_DO_FUN(	do_ban		);
DECLARE_DO_FUN(	do_bash		);
DECLARE_DO_FUN( do_bashdoor     );
DECLARE_DO_FUN( do_berserk	);
DECLARE_DO_FUN( do_bestow	);
DECLARE_DO_FUN( do_bestowarea	);
DECLARE_DO_FUN(	do_bio		);
DECLARE_DO_FUN(	do_bite		);
DECLARE_DO_FUN( do_bloodlet     );
DECLARE_DO_FUN( do_boards	);
DECLARE_DO_FUN( do_bodybag	);
DECLARE_DO_FUN( do_bolt		);
DECLARE_DO_FUN(	do_brandish	);
DECLARE_DO_FUN( do_brew 	);
DECLARE_DO_FUN( do_broach	);
DECLARE_DO_FUN( do_bset		);
DECLARE_DO_FUN( do_bstat	);
DECLARE_DO_FUN(	do_bug		);
DECLARE_DO_FUN( do_bury		);
DECLARE_DO_FUN(	do_buy		);
DECLARE_DO_FUN(	do_cast		);
DECLARE_DO_FUN(	do_cedit	);
DECLARE_DO_FUN(	do_channels	);
DECLARE_DO_FUN(	do_chat		);
DECLARE_DO_FUN(	do_check_vnums  );
DECLARE_DO_FUN( do_circle	);
DECLARE_DO_FUN(	do_clans	);
DECLARE_DO_FUN(	do_clantalk	);
DECLARE_DO_FUN(	do_claw		);
DECLARE_DO_FUN( do_climate	); /* FB */
DECLARE_DO_FUN(	do_climb	);
DECLARE_DO_FUN(	do_close	);
DECLARE_DO_FUN(	do_cmdtable	);
DECLARE_DO_FUN(	do_cmenu	);
DECLARE_DO_FUN( do_colorize	); /* Alty */
DECLARE_DO_FUN(	do_commands	);
DECLARE_DO_FUN(	do_comment	);
DECLARE_DO_FUN(	do_compare	);
DECLARE_DO_FUN(	do_config	);
DECLARE_DO_FUN(	do_consider	);
DECLARE_DO_FUN( do_cook		);
DECLARE_DO_FUN( do_council_induct);
DECLARE_DO_FUN( do_council_outcast);
DECLARE_DO_FUN( do_councils	);
DECLARE_DO_FUN( do_counciltalk	);
DECLARE_DO_FUN( do_credits	);
DECLARE_DO_FUN(	do_cset		);
DECLARE_DO_FUN( do_deities	);
DECLARE_DO_FUN( do_delay	);
DECLARE_DO_FUN(	do_deny		);
DECLARE_DO_FUN(	do_description	);
DECLARE_DO_FUN( do_destro       );
DECLARE_DO_FUN( do_destroy      );
DECLARE_DO_FUN(	do_detrap	);
DECLARE_DO_FUN( do_devote	);
DECLARE_DO_FUN( do_dig		);
DECLARE_DO_FUN(	do_disarm	);
DECLARE_DO_FUN(	do_disconnect	);
DECLARE_DO_FUN( do_dismiss      );
DECLARE_DO_FUN( do_dismount	);
DECLARE_DO_FUN(	do_dmesg	);
DECLARE_DO_FUN(	do_dnd  	);
DECLARE_DO_FUN(	do_down		);
DECLARE_DO_FUN( do_drag 	);
DECLARE_DO_FUN(	do_drink	);
DECLARE_DO_FUN(	do_drop		);
DECLARE_DO_FUN(	do_diagnose	);
DECLARE_DO_FUN(	do_east		);
DECLARE_DO_FUN(	do_eat		);
DECLARE_DO_FUN( do_ech		);
DECLARE_DO_FUN(	do_echo		);
DECLARE_DO_FUN( do_elevate      );
DECLARE_DO_FUN(	do_emote	);
DECLARE_DO_FUN(	do_empty	);
DECLARE_DO_FUN(	do_enter	);
DECLARE_DO_FUN(	do_equipment	);
DECLARE_DO_FUN(	do_examine	);
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN(	do_feed		);
DECLARE_DO_FUN(	do_fill		);
DECLARE_DO_FUN(	do_findnote	);
DECLARE_DO_FUN(	do_fire		);
DECLARE_DO_FUN(	do_fixchar	);
DECLARE_DO_FUN( do_fixed	);
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN( do_foldarea	);
DECLARE_DO_FUN(	do_follow	);
DECLARE_DO_FUN( do_for          );
DECLARE_DO_FUN(	do_force	);
DECLARE_DO_FUN( do_forceclose	);
DECLARE_DO_FUN( do_fprompt	);
DECLARE_DO_FUN( do_fquit	);     /* Gorog */
DECLARE_DO_FUN( do_form_password);
DECLARE_DO_FUN(	do_freeze	);
DECLARE_DO_FUN( do_fshow	);
DECLARE_DO_FUN(	do_gaso	        );
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN( do_gfighting    );
DECLARE_DO_FUN(	do_give		);
DECLARE_DO_FUN(	do_glance	);
DECLARE_DO_FUN( do_gold         );
DECLARE_DO_FUN(	do_goto		);
DECLARE_DO_FUN( do_gouge	);
DECLARE_DO_FUN(	do_group	);
DECLARE_DO_FUN(	do_grub 	);
DECLARE_DO_FUN(	do_gtell	);
DECLARE_DO_FUN( do_guilds       );
DECLARE_DO_FUN( do_guildtalk    );
DECLARE_DO_FUN( do_gwhere	);
DECLARE_DO_FUN( do_hedit	);
DECLARE_DO_FUN( do_hell		);
DECLARE_DO_FUN(	do_help		);
DECLARE_DO_FUN(	do_hide		);
DECLARE_DO_FUN( do_hitall	);
DECLARE_DO_FUN( do_hl           );
DECLARE_DO_FUN( do_hlist	);
DECLARE_DO_FUN(	do_holylight	);
DECLARE_DO_FUN(	do_homepage	);
DECLARE_DO_FUN( do_hset		);
DECLARE_DO_FUN(	do_ide		);
DECLARE_DO_FUN(	do_idea		);
DECLARE_DO_FUN( do_ignore	);
DECLARE_DO_FUN(	do_immortalize	);
DECLARE_DO_FUN(	do_immtalk	);
DECLARE_DO_FUN( do_imm_morph	);
DECLARE_DO_FUN( do_imm_unmorph	);
DECLARE_DO_FUN(	do_induct	);
DECLARE_DO_FUN( do_installarea	);
DECLARE_DO_FUN( do_instaroom	);
DECLARE_DO_FUN( do_instazone	);
DECLARE_DO_FUN(	do_inventory	);
DECLARE_DO_FUN(	do_invis	);
DECLARE_DO_FUN( do_ipcompare    );
DECLARE_DO_FUN( do_khistory	);
DECLARE_DO_FUN(	do_kick		);
DECLARE_DO_FUN(	do_kill		);
DECLARE_DO_FUN( do_languages    );
DECLARE_DO_FUN( do_last		);
DECLARE_DO_FUN( do_laws		);
DECLARE_DO_FUN(	do_leave	);
DECLARE_DO_FUN(	do_level	);
DECLARE_DO_FUN(	do_light	);
DECLARE_DO_FUN(	do_list		);
DECLARE_DO_FUN(	do_litterbug	);
DECLARE_DO_FUN( do_loadarea	);
DECLARE_DO_FUN( do_loadup	);
DECLARE_DO_FUN(	do_lock		);
DECLARE_DO_FUN(	do_log		);
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN(	do_low_purge	);
DECLARE_DO_FUN( do_mailroom	);
DECLARE_DO_FUN(	do_make		);
DECLARE_DO_FUN( do_makeadminlist);
DECLARE_DO_FUN(	do_makeboard	);
DECLARE_DO_FUN(	do_makeclan	);
DECLARE_DO_FUN( do_makecouncil 	);
DECLARE_DO_FUN( do_makedeity	);
DECLARE_DO_FUN( do_makeguild    );
DECLARE_DO_FUN( do_makerepair	);
DECLARE_DO_FUN( do_makeshop	);
DECLARE_DO_FUN( do_makewizlist	);
DECLARE_DO_FUN( do_massign	);
DECLARE_DO_FUN(	do_memory	);
DECLARE_DO_FUN( do_mcreate	);
DECLARE_DO_FUN( do_mdelete	);
DECLARE_DO_FUN(	do_mfind	);
DECLARE_DO_FUN(	do_minvoke	);
DECLARE_DO_FUN( do_mistwalk     );
DECLARE_DO_FUN( do_mlist	);
DECLARE_DO_FUN( do_morphcreate  );
DECLARE_DO_FUN( do_morphdestroy );
DECLARE_DO_FUN( do_morphset     );
DECLARE_DO_FUN( do_morphstat    );
DECLARE_DO_FUN( do_mortalize    );
DECLARE_DO_FUN( do_mount	);
DECLARE_DO_FUN( do_mpfind	);
DECLARE_DO_FUN(	do_mset		);
DECLARE_DO_FUN(	do_mstat	);
DECLARE_DO_FUN(	do_murde	);
DECLARE_DO_FUN(	do_murder	);
DECLARE_DO_FUN(	do_muse		);
DECLARE_DO_FUN(	do_music	);
DECLARE_DO_FUN(	do_mwhere	);
DECLARE_DO_FUN( do_name		);
DECLARE_DO_FUN( do_newbiechat   );
DECLARE_DO_FUN( do_newbieset    );
DECLARE_DO_FUN( do_news         );
DECLARE_DO_FUN( do_newscore	);
DECLARE_DO_FUN( do_newzones	);
DECLARE_DO_FUN(	do_noemote	);
DECLARE_DO_FUN( do_noresolve	);
DECLARE_DO_FUN(	do_north	);
DECLARE_DO_FUN( do_northeast	);
DECLARE_DO_FUN( do_northwest	);
DECLARE_DO_FUN(	do_notell	);
DECLARE_DO_FUN( do_notitle      );
DECLARE_DO_FUN(	do_noteroom	);
DECLARE_DO_FUN( do_nuisance	);
DECLARE_DO_FUN( do_oassign	);
DECLARE_DO_FUN( do_ocreate	);
DECLARE_DO_FUN( do_odelete	);
DECLARE_DO_FUN(	do_ofind	);
DECLARE_DO_FUN(	do_ogrub 	);
DECLARE_DO_FUN(	do_oinvoke	);
DECLARE_DO_FUN(	do_oldscore	);
DECLARE_DO_FUN( do_olist	);
DECLARE_DO_FUN( do_opcopy	);
DECLARE_DO_FUN(	do_open		);
DECLARE_DO_FUN( do_opentourney  );
DECLARE_DO_FUN(	do_opfind	);
DECLARE_DO_FUN(	do_order	);
DECLARE_DO_FUN(	do_orders	);
DECLARE_DO_FUN(	do_ordertalk	);
DECLARE_DO_FUN(	do_oset		);
DECLARE_DO_FUN(	do_ostat	);
DECLARE_DO_FUN(	do_ot		);
DECLARE_DO_FUN(	do_outcast	);
DECLARE_DO_FUN(	do_owhere	);
DECLARE_DO_FUN( do_pager	);
DECLARE_DO_FUN(	do_pardon	);
DECLARE_DO_FUN(	do_password	);
DECLARE_DO_FUN( do_pcrename	);
DECLARE_DO_FUN(	do_peace	);
DECLARE_DO_FUN(	do_pick		);
DECLARE_DO_FUN( do_plist	);
DECLARE_DO_FUN( do_poison_weapon);
DECLARE_DO_FUN(	do_pose		);
DECLARE_DO_FUN(	do_practice	);
DECLARE_DO_FUN( do_project	);
DECLARE_DO_FUN( do_prompt	);
DECLARE_DO_FUN( do_pset		);
DECLARE_DO_FUN( do_pstat	);
DECLARE_DO_FUN( do_pull		);
DECLARE_DO_FUN(	do_punch	);
DECLARE_DO_FUN(	do_purge	);
DECLARE_DO_FUN( do_push		);
DECLARE_DO_FUN(	do_put		);
DECLARE_DO_FUN( do_qpset	);
DECLARE_DO_FUN( do_qpstat	);
DECLARE_DO_FUN(	do_quaff	);
DECLARE_DO_FUN(	do_quest	);
DECLARE_DO_FUN(	do_qui		);
DECLARE_DO_FUN(	do_quit		);
DECLARE_DO_FUN( do_racetalk     );
DECLARE_DO_FUN(	do_rank	        );
DECLARE_DO_FUN( do_rap		);
DECLARE_DO_FUN( do_rassign	);
DECLARE_DO_FUN( do_rat		);
DECLARE_DO_FUN( do_rdelete	);
DECLARE_DO_FUN(	do_reboo	);
DECLARE_DO_FUN(	do_reboot	);
DECLARE_DO_FUN( do_recall	);
DECLARE_DO_FUN(	do_recho	);
DECLARE_DO_FUN(	do_recite	);
DECLARE_DO_FUN(	do_redit	);
DECLARE_DO_FUN( do_regoto       );
DECLARE_DO_FUN( do_remains	);
DECLARE_DO_FUN(	do_remove	);
DECLARE_DO_FUN(	do_rent		);
DECLARE_DO_FUN(	do_repair	);
DECLARE_DO_FUN(	do_repairset	);
DECLARE_DO_FUN(	do_repairshops	);
DECLARE_DO_FUN(	do_repairstat	);
DECLARE_DO_FUN( do_repeat	);
DECLARE_DO_FUN(	do_reply	);
DECLARE_DO_FUN(	do_report	);
DECLARE_DO_FUN(	do_rescue	);
DECLARE_DO_FUN( do_reserve	);
DECLARE_DO_FUN( do_reset	);
DECLARE_DO_FUN(	do_rest		);
DECLARE_DO_FUN(	do_restore	);
DECLARE_DO_FUN(	do_restoretime	);
DECLARE_DO_FUN(	do_restrict	);
DECLARE_DO_FUN( do_retell	);
DECLARE_DO_FUN( do_retire       );
DECLARE_DO_FUN( do_retran       );
DECLARE_DO_FUN(	do_return	);
DECLARE_DO_FUN(	do_revert	);
DECLARE_DO_FUN( do_rgrub	);
DECLARE_DO_FUN( do_rip		);
DECLARE_DO_FUN( do_rlist	);
DECLARE_DO_FUN( do_rolldie	);
DECLARE_DO_FUN( do_rpfind	);
DECLARE_DO_FUN( do_rreset	);
DECLARE_DO_FUN(	do_rset		);
DECLARE_DO_FUN(	do_rstat	);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_save		);
DECLARE_DO_FUN( do_savearea	);
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN( do_scan         );
DECLARE_DO_FUN( do_scatter      );
DECLARE_DO_FUN(	do_score	);
DECLARE_DO_FUN( do_scribe       );
DECLARE_DO_FUN( do_search	);
DECLARE_DO_FUN(	do_sedit	);
DECLARE_DO_FUN(	do_sell		);
DECLARE_DO_FUN( do_set_boot_time);
DECLARE_DO_FUN( do_setclan	);
DECLARE_DO_FUN( do_setclass	);
DECLARE_DO_FUN( do_setcouncil   );
DECLARE_DO_FUN( do_setdeity	);
DECLARE_DO_FUN( do_setrace	);
DECLARE_DO_FUN( do_setweather	);
DECLARE_DO_FUN(	do_shops	);
DECLARE_DO_FUN(	do_shopset	);
DECLARE_DO_FUN(	do_shopstat	);
DECLARE_DO_FUN(	do_shout	);
DECLARE_DO_FUN( do_shove  	);
DECLARE_DO_FUN( do_showclass	);
DECLARE_DO_FUN( do_showclan	);
DECLARE_DO_FUN( do_showcouncil	);
DECLARE_DO_FUN( do_showdeity	);
DECLARE_DO_FUN( do_showrace	);
DECLARE_DO_FUN( do_showweather	);	/* FB */
DECLARE_DO_FUN(	do_shutdow	);
DECLARE_DO_FUN(	do_shutdown	);
DECLARE_DO_FUN(	do_silence	);
DECLARE_DO_FUN(	do_sit		);
DECLARE_DO_FUN( do_skin		);
DECLARE_DO_FUN(	do_sla		);
DECLARE_DO_FUN(	do_slay		);
DECLARE_DO_FUN(	do_sleep	);
DECLARE_DO_FUN( do_slice        );
DECLARE_DO_FUN( do_slist        );
DECLARE_DO_FUN(	do_slookup	);
DECLARE_DO_FUN(	do_smoke	);
DECLARE_DO_FUN(	do_sneak	);
DECLARE_DO_FUN(	do_snoop	);
DECLARE_DO_FUN(	do_sober	);
DECLARE_DO_FUN(	do_socials	);
DECLARE_DO_FUN(	do_south	);
DECLARE_DO_FUN(	do_southeast	);
DECLARE_DO_FUN(	do_southwest	);
DECLARE_DO_FUN( do_speak        );
DECLARE_DO_FUN(	do_split	);
DECLARE_DO_FUN(	do_sset		);
DECLARE_DO_FUN(	do_stand	);
DECLARE_DO_FUN(	do_stat		);
DECLARE_DO_FUN(	do_statreport	);
DECLARE_DO_FUN( do_statshield	);
DECLARE_DO_FUN( do_starttourney );
DECLARE_DO_FUN(	do_steal	);
DECLARE_DO_FUN(	do_sting	);
DECLARE_DO_FUN( do_strew	);
DECLARE_DO_FUN( do_strip	);
DECLARE_DO_FUN(	do_stun		);
DECLARE_DO_FUN(	do_style	);
DECLARE_DO_FUN( do_supplicate	);
DECLARE_DO_FUN(	do_switch	);
DECLARE_DO_FUN(	do_showlayers	);
DECLARE_DO_FUN(	do_tail		);
DECLARE_DO_FUN(	do_tamp		);
DECLARE_DO_FUN(	do_tell		);
DECLARE_DO_FUN(	do_think	);
DECLARE_DO_FUN(	do_time		);
DECLARE_DO_FUN(	do_timecmd	);
DECLARE_DO_FUN(	do_title	);
DECLARE_DO_FUN( do_track	);
DECLARE_DO_FUN( do_traffic	);
DECLARE_DO_FUN(	do_transfer	);
DECLARE_DO_FUN(	do_trust	);
DECLARE_DO_FUN(	do_typo		);
DECLARE_DO_FUN( do_unbolt	);
DECLARE_DO_FUN(	do_unfoldarea	);
DECLARE_DO_FUN( do_unhell	);
DECLARE_DO_FUN(	do_unlock	);
DECLARE_DO_FUN( do_unnuisance   );
DECLARE_DO_FUN( do_unsilence    );
DECLARE_DO_FUN(	do_up		);
DECLARE_DO_FUN(	do_users	);
DECLARE_DO_FUN(	do_value	);
DECLARE_DO_FUN( do_version	);
DECLARE_DO_FUN( do_victories	);
DECLARE_DO_FUN(	do_visible	);
DECLARE_DO_FUN( do_vnums	);
DECLARE_DO_FUN( do_vsearch	);
DECLARE_DO_FUN(	do_wake		);
DECLARE_DO_FUN( do_warn         );
DECLARE_DO_FUN( do_wartalk      );
DECLARE_DO_FUN(	do_wear		);
DECLARE_DO_FUN(	do_weather	);
DECLARE_DO_FUN(	do_west		);
DECLARE_DO_FUN(	do_where	);
DECLARE_DO_FUN( do_whisper	);
DECLARE_DO_FUN(	do_who		);
DECLARE_DO_FUN(	do_whois	);
DECLARE_DO_FUN(	do_wimpy	);
DECLARE_DO_FUN(	do_wizhelp	);
DECLARE_DO_FUN( do_wizlist	);
DECLARE_DO_FUN(	do_wizlock	);
DECLARE_DO_FUN( do_worth        ); 
DECLARE_DO_FUN(	do_yell		);
DECLARE_DO_FUN(	do_zap		);
DECLARE_DO_FUN( do_zones	);

/* mob prog stuff */
DECLARE_DO_FUN( do_mp_close_passage );
DECLARE_DO_FUN( do_mp_damage	);
DECLARE_DO_FUN( do_mp_log	);
DECLARE_DO_FUN( do_mp_restore	);
DECLARE_DO_FUN( do_mp_open_passage );
DECLARE_DO_FUN( do_mp_practice	);
DECLARE_DO_FUN( do_mp_slay	);
DECLARE_DO_FUN( do_mpadvance    );
DECLARE_DO_FUN( do_mpasound     );
DECLARE_DO_FUN( do_mpasupress	);
DECLARE_DO_FUN( do_mpat         );
DECLARE_DO_FUN( do_mpcopy	);
DECLARE_DO_FUN( do_mpdream	);
DECLARE_DO_FUN( do_mp_deposit	);
DECLARE_DO_FUN( do_mp_fill_in   );
DECLARE_DO_FUN( do_mp_withdraw	);
DECLARE_DO_FUN( do_mpecho       );
DECLARE_DO_FUN( do_mpechoaround );
DECLARE_DO_FUN( do_mpechoat     );
DECLARE_DO_FUN( do_mpechozone	);
DECLARE_DO_FUN( do_mpedit       );
DECLARE_DO_FUN( do_mrange       );
DECLARE_DO_FUN( do_opedit       );
DECLARE_DO_FUN( do_orange       );
DECLARE_DO_FUN( do_rpedit       );
DECLARE_DO_FUN( do_mpforce      );
DECLARE_DO_FUN( do_mpinvis	);
DECLARE_DO_FUN( do_mpgoto       );
DECLARE_DO_FUN( do_mpjunk       );
DECLARE_DO_FUN( do_mpkill       );
DECLARE_DO_FUN( do_mpmload      );
DECLARE_DO_FUN( do_mpmset	);
DECLARE_DO_FUN( do_mpnothing	);
DECLARE_DO_FUN( do_mpoload      );
DECLARE_DO_FUN( do_mposet	);
DECLARE_DO_FUN( do_mppardon	);
DECLARE_DO_FUN( do_mppeace	);
DECLARE_DO_FUN( do_mppurge      );
DECLARE_DO_FUN( do_mpstat       );
DECLARE_DO_FUN( do_opstat       );
DECLARE_DO_FUN( do_rpstat       );
DECLARE_DO_FUN( do_mptransfer   );
DECLARE_DO_FUN( do_mpmorph	);
DECLARE_DO_FUN( do_mpunmorph	);
DECLARE_DO_FUN( do_mpnuisance   );
DECLARE_DO_FUN( do_mpunnuisance );
DECLARE_DO_FUN( do_mpbodybag    );
DECLARE_DO_FUN( do_mpapply	);
DECLARE_DO_FUN( do_mpapplyb  	);
DECLARE_DO_FUN( do_mppkset	);
DECLARE_DO_FUN( do_mpfavor	);
DECLARE_DO_FUN( do_mpscatter    );
DECLARE_DO_FUN( do_mpdelay      );
DECLARE_DO_FUN( do_mpsound	);
DECLARE_DO_FUN( do_mpsoundaround);
DECLARE_DO_FUN( do_mpsoundat	);
DECLARE_DO_FUN( do_mpmusic	);
DECLARE_DO_FUN( do_mpmusicaround);
DECLARE_DO_FUN( do_mpmusicat	);

/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(	spell_null		);
DECLARE_SPELL_FUN(	spell_notfound		);
DECLARE_SPELL_FUN(	spell_acid_blast	);
DECLARE_SPELL_FUN(	spell_animate_dead	);
DECLARE_SPELL_FUN(	spell_astral_walk	);
DECLARE_SPELL_FUN(	spell_blindness		);
DECLARE_SPELL_FUN(	spell_burning_hands	);
DECLARE_SPELL_FUN(	spell_call_lightning	);
DECLARE_SPELL_FUN(	spell_cause_critical	);
DECLARE_SPELL_FUN(	spell_cause_light	);
DECLARE_SPELL_FUN(	spell_cause_serious	);
DECLARE_SPELL_FUN(	spell_change_sex	);
DECLARE_SPELL_FUN(	spell_charm_person	);
DECLARE_SPELL_FUN(	spell_chill_touch	);
DECLARE_SPELL_FUN(	spell_colour_spray	);
DECLARE_SPELL_FUN(	spell_control_weather	);
DECLARE_SPELL_FUN(	spell_create_food	);
DECLARE_SPELL_FUN(	spell_create_water	);
DECLARE_SPELL_FUN(	spell_cure_blindness	);
DECLARE_SPELL_FUN(	spell_cure_poison	);
DECLARE_SPELL_FUN(	spell_curse		);
DECLARE_SPELL_FUN(	spell_detect_poison	);
DECLARE_SPELL_FUN(	spell_dispel_evil	);
DECLARE_SPELL_FUN(	spell_dispel_magic	);
DECLARE_SPELL_FUN(      spell_disenchant_weapon );
DECLARE_SPELL_FUN(      spell_dream             );
DECLARE_SPELL_FUN(	spell_earthquake	);
DECLARE_SPELL_FUN(	spell_enchant_weapon	);
DECLARE_SPELL_FUN(	spell_energy_drain	);
DECLARE_SPELL_FUN(	spell_faerie_fire	);
DECLARE_SPELL_FUN(	spell_faerie_fog	);
DECLARE_SPELL_FUN(	spell_farsight		);
DECLARE_SPELL_FUN(	spell_fireball		);
DECLARE_SPELL_FUN(	spell_flamestrike	);
DECLARE_SPELL_FUN(	spell_gate		);
DECLARE_SPELL_FUN(      spell_knock             );
DECLARE_SPELL_FUN(	spell_harm		);
DECLARE_SPELL_FUN(	spell_identify		);
DECLARE_SPELL_FUN(	spell_invis		);
DECLARE_SPELL_FUN(	spell_know_alignment	);
DECLARE_SPELL_FUN(	spell_lightning_bolt	);
DECLARE_SPELL_FUN(	spell_locate_object	);
DECLARE_SPELL_FUN(	spell_magic_missile	);
DECLARE_SPELL_FUN(	spell_mist_walk		);
DECLARE_SPELL_FUN(	spell_pass_door		);
DECLARE_SPELL_FUN(	spell_plant_pass	);
DECLARE_SPELL_FUN(	spell_poison		);
DECLARE_SPELL_FUN(	spell_polymorph		);
DECLARE_SPELL_FUN(	spell_possess		);
DECLARE_SPELL_FUN(	spell_recharge		);
DECLARE_SPELL_FUN(	spell_remove_curse	);
DECLARE_SPELL_FUN(	spell_remove_invis	);
DECLARE_SPELL_FUN(	spell_remove_trap	);
DECLARE_SPELL_FUN(	spell_shocking_grasp	);
DECLARE_SPELL_FUN(	spell_sleep		);
DECLARE_SPELL_FUN(	spell_smaug		);
DECLARE_SPELL_FUN(	spell_solar_flight	);
DECLARE_SPELL_FUN(	spell_summon		);
DECLARE_SPELL_FUN(	spell_teleport		);
DECLARE_SPELL_FUN(	spell_ventriloquate	);
DECLARE_SPELL_FUN(	spell_weaken		);
DECLARE_SPELL_FUN(	spell_word_of_recall	);
DECLARE_SPELL_FUN(	spell_acid_breath	);
DECLARE_SPELL_FUN(	spell_fire_breath	);
DECLARE_SPELL_FUN(	spell_frost_breath	);
DECLARE_SPELL_FUN(	spell_gas_breath	);
DECLARE_SPELL_FUN(	spell_lightning_breath	);
DECLARE_SPELL_FUN(	spell_spiral_blast	);
DECLARE_SPELL_FUN(	spell_scorching_surge	);
DECLARE_SPELL_FUN(	spell_helical_flow	);
DECLARE_SPELL_FUN(      spell_transport      	);
DECLARE_SPELL_FUN(      spell_portal            );

DECLARE_SPELL_FUN(      spell_ethereal_fist     );
DECLARE_SPELL_FUN(      spell_spectral_furor    );
DECLARE_SPELL_FUN(      spell_hand_of_chaos     );
DECLARE_SPELL_FUN(      spell_disruption        );
DECLARE_SPELL_FUN(      spell_sonic_resonance   );
DECLARE_SPELL_FUN(      spell_mind_wrack        );
DECLARE_SPELL_FUN(      spell_mind_wrench       );
DECLARE_SPELL_FUN(      spell_revive            );
DECLARE_SPELL_FUN(      spell_sulfurous_spray   );
DECLARE_SPELL_FUN(      spell_caustic_fount     );
DECLARE_SPELL_FUN(      spell_acetum_primus     );
DECLARE_SPELL_FUN(      spell_galvanic_whip     );
DECLARE_SPELL_FUN(      spell_magnetic_thrust   );
DECLARE_SPELL_FUN(      spell_quantum_spike     );
DECLARE_SPELL_FUN(      spell_black_hand        );
DECLARE_SPELL_FUN(      spell_black_fist        );
DECLARE_SPELL_FUN(      spell_black_lightning   );
DECLARE_SPELL_FUN(      spell_midas_touch       );

DECLARE_SPELL_FUN(      spell_bethsaidean_touch	);
DECLARE_SPELL_FUN(	spell_expurgation	);
DECLARE_SPELL_FUN(	spell_sacral_divinity	);

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(apollo)
int	atoi		args( ( const char *string ) );
void *	calloc		args( ( unsigned nelem, size_t size ) );
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(hpux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(interactive)
#endif

#if	defined(linux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(MIPS_OS)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(NeXT)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(sequent)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(sun)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
#if 	defined(SYSV)
size_t 	fread		args( ( void *ptr, size_t size, size_t n,
				FILE *stream ) );
#else
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
#endif
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(ultrix)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 * United States to foreign countries.
 *
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#define PLAYER_DIR	"../player/"	/* Player files			*/
#define BACKUP_DIR	"../player/backup/" /* Backup Player files	*/
#define GOD_DIR		"../gods/"	/* God Info Dir			*/
#define BOARD_DIR	"../boards/"	/* Board data dir		*/
#define CLAN_DIR	"../clans/"	/* Clan data dir		*/
#define COUNCIL_DIR  	"../councils/"  /* Council data dir		*/
#define DEITY_DIR	"../deity/"	/* Deity data dir		*/
#define BUILD_DIR       "../building/"  /* Online building save dir     */
#define SYSTEM_DIR	"../system/"	/* Main system files		*/
#define PROG_DIR	"mudprogs/"	/* MUDProg files		*/
#define CORPSE_DIR	"../corpses/"	/* Corpses			*/
#ifdef WIN32
  #define NULL_FILE	"nul"		/* To reserve one stream        */
#else
  #define NULL_FILE	"/dev/null"	/* To reserve one stream        */
#endif

#define	CLASS_DIR	"../classes/"	/* Classes			*/
#define WATCH_DIR	"../watch/"	/* Imm watch files --Gorog      */
/*
 * The watch directory contains a maximum of one file for each immortal
 * that contains output from "player watches". The name of each file
 * in this directory is the name of the immortal who requested the watch
 */

#define AREA_LIST	"area.lst"	/* List of areas		*/
#define WATCH_LIST      "watch.lst"     /* List of watches              */
#define BAN_LIST        "ban.lst"       /* List of bans                 */
#define RESERVED_LIST	"reserved.lst"	/* List of reserved names	*/
#define CLAN_LIST	"clan.lst"	/* List of clans		*/
#define COUNCIL_LIST	"council.lst"	/* List of councils		*/
#define GUILD_LIST      "guild.lst"     /* List of guilds               */
#define GOD_LIST	"gods.lst"	/* List of gods			*/
#define DEITY_LIST	"deity.lst"	/* List of deities		*/
#define	CLASS_LIST	"class.lst"	/* List of classes		*/
#define	RACE_LIST	"race.lst"	/* List of races		*/

#define MORPH_FILE      "morph.dat"     /* For morph data */
#define BOARD_FILE	"boards.txt"		/* For bulletin boards	 */
#define SHUTDOWN_FILE	"shutdown.txt"		/* For 'shutdown'	 */
#define IMM_HOST_FILE   SYSTEM_DIR "immortal.host" /* For stoping hackers */

#define RIPSCREEN_FILE	SYSTEM_DIR "mudrip.rip"
#define RIPTITLE_FILE	SYSTEM_DIR "mudtitle.rip"
#define ANSITITLE_FILE	SYSTEM_DIR "mudtitle.ans"
#define ASCTITLE_FILE	SYSTEM_DIR "mudtitle.asc"
#define BOOTLOG_FILE	SYSTEM_DIR "boot.txt"	  /* Boot up error file	 */
#define BUG_FILE	SYSTEM_DIR "bugs.txt"	  /* For bug( )          */
#define PBUG_FILE	SYSTEM_DIR "pbugs.txt"	  /* For 'bug' command   */
#define IDEA_FILE	SYSTEM_DIR "ideas.txt"	  /* For 'idea'		 */
#define TYPO_FILE	SYSTEM_DIR "typos.txt"	  /* For 'typo'		 */
#define FIXED_FILE	SYSTEM_DIR "fixed.txt"	  /* For 'fixed' command */
#define LOG_FILE	SYSTEM_DIR "log.txt"	  /* For talking in logged rooms */
#define MOBLOG_FILE	SYSTEM_DIR "moblog.txt"   /* For mplog messages  */
#define PLEVEL_FILE	SYSTEM_DIR "plevel.txt"   /* Char level info */
#define WIZLIST_FILE	SYSTEM_DIR "WIZLIST"	  /* Wizlist		 */
#define WHO_FILE	SYSTEM_DIR "WHO"	  /* Who output file	 */
#define WEBWHO_FILE	SYSTEM_DIR "WEBWHO"	  /* WWW Who output file */
#define REQUEST_PIPE	SYSTEM_DIR "REQUESTS"	  /* Request FIFO	 */
#define SKILL_FILE	SYSTEM_DIR "skills.dat"   /* Skill table	 */
#define HERB_FILE	SYSTEM_DIR "herbs.dat"	  /* Herb table		 */
#define TONGUE_FILE	SYSTEM_DIR "tongues.dat"  /* Tongue tables	 */
#define SOCIAL_FILE	SYSTEM_DIR "socials.dat"  /* Socials		 */
#define COMMAND_FILE	SYSTEM_DIR "commands.dat" /* Commands		 */
#define USAGE_FILE	SYSTEM_DIR "usage.txt"    /* How many people are on 
 						     every half hour - trying to
						     determine best reboot time */
#define ECONOMY_FILE	SYSTEM_DIR "economy.txt"  /* Gold looted, value of
						     used potions/pills  */
#define PROJECTS_FILE	SYSTEM_DIR "projects.txt" /* For projects	 */
#define PLANE_FILE	SYSTEM_DIR "planes.dat"	  /* For planes		 */
#define COLOR_FILE	SYSTEM_DIR "colors.dat"	  /* User-definable color*/
#define TEMP_FILE	SYSTEM_DIR "charsave.tmp" /* More char save protect */
#define CLASSDIR	"../classes/"
#define RACEDIR 	"../races/"

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define BD	BOARD_DATA
#define CL	CLAN_DATA
#define EDD	EXTRA_DESCR_DATA
#define RD	RESET_DATA
#define ED	EXIT_DATA
#define	ST	SOCIALTYPE
#define	CO	COUNCIL_DATA
#define DE	DEITY_DATA
#define SK	SKILLTYPE

/* act_comm.c */
bool	circle_follow	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_follower	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
void	send_rip_screen args( ( CHAR_DATA *ch ) );
void	send_rip_title	args( ( CHAR_DATA *ch ) );
void	send_ansi_title args( ( CHAR_DATA *ch ) );
void	send_ascii_title args( ( CHAR_DATA *ch ) );
void	to_channel	args( ( const char *argument, int channel,
				const char *verb, sh_int level ) );
void  	talk_auction    args( ( char *argument ) );
int	knows_language  args( ( CHAR_DATA *ch, int language,
				CHAR_DATA *cch ) );
bool    can_learn_lang  args( ( CHAR_DATA *ch, int language ) );
int     countlangs      args( ( int languages ) );
char *	translate	args( ( int percent, const char *in, const char *name) );
char *	obj_short	args( ( OBJ_DATA *obj ) );
void    init_profanity_checker	args( ( void ) );

/* act_info.c */
int	get_door	args( ( char *arg ) );
char *  num_punct	args( ( int foo ) );
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
bool	is_ignoring	args( (CHAR_DATA *ch, CHAR_DATA *ign_ch) );
void	show_race_line	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

/* act_move.c */
void	clear_vrooms	args( ( void ) );
ED *	find_door	args( ( CHAR_DATA *ch, char *arg, bool quiet ) );
ED *	get_exit	args( ( ROOM_INDEX_DATA *room, sh_int dir ) );
ED *	get_exit_to	args( ( ROOM_INDEX_DATA *room, sh_int dir, int vnum ) );
ED *	get_exit_num	args( ( ROOM_INDEX_DATA *room, sh_int count ) );
ch_ret	move_char	args( ( CHAR_DATA *ch, EXIT_DATA *pexit, int fall ) );
void	teleport	args( ( CHAR_DATA *ch, sh_int room, int flags ) );
sh_int	encumbrance	args( ( CHAR_DATA *ch, sh_int move ) );
bool	will_fall	args( ( CHAR_DATA *ch, int fall ) );
ch_ret	pullcheck	args( ( CHAR_DATA *ch, int pulse ) );
char *	rev_exit	args( ( sh_int vdir ) );

/* act_obj.c */

obj_ret	damage_obj	args( ( OBJ_DATA *obj ) );
sh_int	get_obj_resistance args( ( OBJ_DATA *obj ) );
void    save_clan_storeroom args( ( CHAR_DATA *ch, CLAN_DATA *clan ) );
void    obj_fall  	args( ( OBJ_DATA *obj, bool through ) );

/* act_wiz.c */
bool create_new_race	args( (int index, char *argument ) );
bool create_new_class	args( (int index, char *argument ) );
RID *	find_location	args( ( CHAR_DATA *ch, char *arg ) );
void	echo_to_all	args( ( sh_int AT_COLOR, char *argument,
				sh_int tar ) );
void   	get_reboot_string args( ( void ) );
struct tm *update_time  args( ( struct tm *old_time ) );
void	free_social	args( ( SOCIALTYPE *social ) );
void	add_social	args( ( SOCIALTYPE *social ) );
void	free_command	args( ( CMDTYPE *command ) );
void	unlink_command	args( ( CMDTYPE *command ) );
void	add_command	args( ( CMDTYPE *command ) );

/* boards.c */
void	load_boards	args( ( void ) );
BD *	get_board	args( ( OBJ_DATA *obj ) );
void	free_note	args( ( NOTE_DATA *pnote ) );

/* build.c */
int     get_cmdflag     args (( char *flag ));
char *	flag_string	args( ( int bitvector, char * const flagarray[] ) );
char *	ext_flag_string	args( ( EXT_BV *bitvector, char * const flagarray[] ) );
int	get_mpflag	args( ( char *flag ) );
int	get_dir		args( ( char *txt  ) );
char *	strip_cr	args( ( char *str  ) );

/* clans.c */
CL *	get_clan	args( ( char *name ) );
void	load_clans	args( ( void ) );
void	save_clan	args( ( CLAN_DATA *clan ) );

CO *	get_council	args( ( char *name ) );
void	load_councils	args( ( void ) );
void 	save_council	args( ( COUNCIL_DATA *council ) );

/* deity.c */
DE *	get_deity	args( ( char *name ) );
void	load_deity	args( ( void ) );
void	save_deity	args( ( DEITY_DATA *deity ) );

/* comm.c */
void	close_socket	args( ( DESCRIPTOR_DATA *dclose, bool force ) );
void	write_to_buffer	args( ( DESCRIPTOR_DATA *d, const char *txt,
				int length ) );
void	write_to_pager	args( ( DESCRIPTOR_DATA *d, const char *txt,
				int length ) );
void	send_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_char_color	args( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_pager	args( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_pager_color	args( ( const char *txt, CHAR_DATA *ch ) );
void	set_char_color  args( ( sh_int AType, CHAR_DATA *ch ) );
void	set_pager_color	args( ( sh_int AType, CHAR_DATA *ch ) );
void	ch_printf	args( ( CHAR_DATA *ch, char *fmt, ... ) );
void	ch_printf_color	args( ( CHAR_DATA *ch, char *fmt, ... ) );
void	pager_printf	args( (CHAR_DATA *ch, char *fmt, ...) );
void	pager_printf_color	args( ( CHAR_DATA *ch, char *fmt, ... ) );
void	act		args( ( sh_int AType, const char *format, CHAR_DATA *ch,
			    const void *arg1, const void *arg2, int type ) );
char *	myobj		args( ( OBJ_DATA *obj ) );
char *	obj_short	args( ( OBJ_DATA *obj ) );

/* reset.c */
RD  *	make_reset	args( ( char letter, int extra, int arg1, int arg2, int arg3 ) );
RD  *	add_reset	args( ( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 ) );
RD  *	place_reset	args( ( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 ) );
void	reset_area	args( ( AREA_DATA * pArea ) );

/* db.c */
void	show_file	args( ( CHAR_DATA *ch, char *filename ) );
char *	str_dup		args( ( char const *str ) );
void	boot_db		args( ( void ) );
void	area_update	args( ( void ) );
void	add_char	args( ( CHAR_DATA *ch ) );
CD *	create_mobile	args( ( MOB_INDEX_DATA *pMobIndex ) );
OD *	create_object	args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void	clear_char	args( ( CHAR_DATA *ch ) );
void	free_char	args( ( CHAR_DATA *ch ) );
char *	get_extra_descr	args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *	get_mob_index	args( ( sh_int vnum ) );
OID *	get_obj_index	args( ( int vnum ) );
RID *	get_room_index	args( ( int vnum ) );
char	fread_letter	args( ( FILE *fp ) );
int	fread_number	args( ( FILE *fp ) );
EXT_BV  fread_bitvector	args( ( FILE *fp ) );
void	fwrite_bitvector args( ( EXT_BV *bits, FILE *fp ) );
char *	print_bitvector	args( ( EXT_BV *bits ) );
char *	fread_string	args( ( FILE *fp ) );
char *	fread_string_nohash args( ( FILE *fp ) );
void	fread_to_eol	args( ( FILE *fp ) );
char *	fread_word	args( ( FILE *fp ) );
char *	fread_line	args( ( FILE *fp ) );
int	number_fuzzy	args( ( int number ) );
int	number_range	args( ( int from, int to ) );
int	number_percent	args( ( void ) );
int	number_door	args( ( void ) );
int	number_bits	args( ( int width ) );
int	number_mm	args( ( void ) );
int	dice		args( ( int number, int size ) );
int	interpolate	args( ( int level, int value_00, int value_32 ) );
void	smash_tilde	args( ( char *str ) );
void	hide_tilde	args( ( char *str ) );
char *	show_tilde	args( ( char *str ) );
bool	str_cmp		args( ( const char *astr, const char *bstr ) );
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
char *	capitalize	args( ( const char *str ) );
char *	strlower	args( ( const char *str ) );
char *	strupper	args( ( const char *str ) );
char *  aoran		args( ( const char *str ) );
void	append_file	args( ( CHAR_DATA *ch, char *file, char *str ) );
void	append_to_file	args( ( char *file, char *str ) );
void	bug		args( ( const char *str, ... ) );
void	log_string_plus	args( ( const char *str, sh_int log_type, sh_int level ) );
RID *	make_room	args( ( int vnum ) );
OID *	make_object	args( ( int vnum, int cvnum, char *name ) );
MID *	make_mobile	args( ( sh_int vnum, sh_int cvnum, char *name ) );
ED  *	make_exit	args( ( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, sh_int door ) );
void	add_help	args( ( HELP_DATA *pHelp ) );
void	fix_area_exits	args( ( AREA_DATA *tarea ) );
void	load_area_file	args( ( AREA_DATA *tarea, char *filename ) );
void	randomize_exits	args( ( ROOM_INDEX_DATA *room, sh_int maxdir ) );
void	make_wizlist	args( ( void ) );
void	tail_chain	args( ( void ) );
bool    delete_room     args( ( ROOM_INDEX_DATA *room ) );
bool    delete_obj      args( ( OBJ_INDEX_DATA *obj ) );
bool    delete_mob      args( ( MOB_INDEX_DATA *mob ) );
/* Functions to add to sorting lists. -- Altrag */
/*void	mob_sort	args( ( MOB_INDEX_DATA *pMob ) );
void	obj_sort	args( ( OBJ_INDEX_DATA *pObj ) );
void	room_sort	args( ( ROOM_INDEX_DATA *pRoom ) );*/
void	sort_area	args( ( AREA_DATA *pArea, bool proto ) );
void    sort_area_by_name  args( (AREA_DATA *pArea) ); /* Fireblade */
void	write_projects  args( ( void ) );

/* build.c */
void	start_editing	args( ( CHAR_DATA *ch, char *data ) );
void	stop_editing	args( ( CHAR_DATA *ch ) );
void	edit_buffer	args( ( CHAR_DATA *ch, char *argument ) );
char *	copy_buffer	args( ( CHAR_DATA *ch ) );
bool	can_rmodify	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room ) );
bool	can_omodify	args( ( CHAR_DATA *ch, OBJ_DATA *obj  ) );
bool	can_mmodify	args( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
bool	can_medit	args( ( CHAR_DATA *ch, MOB_INDEX_DATA *mob ) );
void	free_reset	args( ( AREA_DATA *are, RESET_DATA *res ) );
void	free_area	args( ( AREA_DATA *are ) );
void	assign_area	args( ( CHAR_DATA *ch ) );
EDD *	SetRExtra	args( ( ROOM_INDEX_DATA *room, char *keywords ) );
bool	DelRExtra	args( ( ROOM_INDEX_DATA *room, char *keywords ) );
EDD *	SetOExtra	args( ( OBJ_DATA *obj, char *keywords ) );
bool	DelOExtra	args( ( OBJ_DATA *obj, char *keywords ) );
EDD *	SetOExtraProto	args( ( OBJ_INDEX_DATA *obj, char *keywords ) );
bool	DelOExtraProto	args( ( OBJ_INDEX_DATA *obj, char *keywords ) );
void	fold_area	args( ( AREA_DATA *tarea, char *filename, bool install ) );
int	get_otype	args( ( char *type ) );
int	get_atype	args( ( char *type ) );
int	get_aflag	args( ( char *flag ) );
int	get_oflag	args( ( char *flag ) );
int	get_wflag	args( ( char *flag ) );
void	init_area_weather args(( void ) );
void	save_weatherdata args( ( void ) );

/* fight.c */
int	max_fight	args( ( CHAR_DATA *ch ) );
void	violence_update	args( ( void ) );
ch_ret	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
ch_ret	projectile_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield,
			    OBJ_DATA *projectile, sh_int dist ) );
sh_int	ris_damage	args( ( CHAR_DATA *ch, sh_int dam, int ris ) );
ch_ret	damage		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt ) );
void	update_pos	args( ( CHAR_DATA *victim ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );
void	free_fight	args( ( CHAR_DATA *ch ) );
CD *	who_fighting	args( ( CHAR_DATA *ch ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_attacker	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	stop_hunting	args( ( CHAR_DATA *ch ) );
void	stop_hating	args( ( CHAR_DATA *ch ) );
void	stop_fearing	args( ( CHAR_DATA *ch ) );
void	start_hunting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	start_hating	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	start_fearing	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_hunting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_hating	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_fearing	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool SHOW ) );
bool	legal_loot	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
sh_int	VAMP_AC		args( ( CHAR_DATA *ch ) );
bool    check_illegal_pk args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    raw_kill        args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );   
bool	in_arena	args( ( CHAR_DATA *ch ) );
bool	can_astral	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

/* makeobjs.c */
void	make_corpse	args( ( CHAR_DATA *ch, CHAR_DATA *killer ) );
void	make_blood	args( ( CHAR_DATA *ch ) );
void	make_bloodstain args( ( CHAR_DATA *ch ) );
void	make_scraps	args( ( OBJ_DATA *obj ) );
void	make_fire	args( ( ROOM_INDEX_DATA *in_room, sh_int timer) );
OD *	make_trap	args( ( int v0, int v1, int v2, int v3 ) );
OD *	create_money	args( ( int amount ) );

/* misc.c */
void actiondesc args( ( CHAR_DATA *ch, OBJ_DATA *obj, void *vo ) );
EXT_BV	meb		args( ( int bit ) );
EXT_BV	multimeb	args( ( int bit, ... ) );


/* deity.c */
void adjust_favor	args( ( CHAR_DATA *ch, int field, int mod ) );

/* mud_comm.c */
char *	mprog_type_to_name	args( ( int type ) );

/* mud_prog.c */
#ifdef DUNNO_STRSTR
char *  strstr                  args ( (const char *s1, const char *s2 ) );
#endif

void	mprog_wordlist_check    args ( ( char * arg, CHAR_DATA *mob,
                			CHAR_DATA* actor, OBJ_DATA* object,
					void* vo, int type ) );
void	mprog_percent_check     args ( ( CHAR_DATA *mob, CHAR_DATA* actor,
					OBJ_DATA* object, void* vo,
					int type ) );
void	mprog_act_trigger       args ( ( char* buf, CHAR_DATA* mob,
		                        CHAR_DATA* ch, OBJ_DATA* obj,
					void* vo ) );
void	mprog_bribe_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
		                        int amount ) );
void	mprog_entry_trigger     args ( ( CHAR_DATA* mob ) );
void	mprog_give_trigger      args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
                		        OBJ_DATA* obj ) );
void	mprog_greet_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_fight_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_hitprcnt_trigger  args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_death_trigger     args ( ( CHAR_DATA *killer, CHAR_DATA* mob ) );
void    mprog_random_trigger    args ( ( CHAR_DATA* mob ) );
void    mprog_speech_trigger    args ( ( char* txt, CHAR_DATA* mob ) );
void    mprog_script_trigger    args ( ( CHAR_DATA *mob ) );
void    mprog_hour_trigger      args ( ( CHAR_DATA *mob ) );
void    mprog_time_trigger      args ( ( CHAR_DATA *mob ) );
void    progbug                 args( ( char *str, CHAR_DATA *mob ) );
void	rset_supermob		args( ( ROOM_INDEX_DATA *room) );
void	release_supermob	args( ( ) );

/* planes.c */
PLANE_DATA *	plane_lookup	args( ( const char *name ) );
void		load_planes	args( ( void ) );
void		save_planes	args( ( void ) );
void		check_planes	args( ( PLANE_DATA *p ) );

/* player.c */
void	set_title	args( ( CHAR_DATA *ch, char *title ) );

/* polymorph.c */
void fwrite_morph_data args ( ( CHAR_DATA *ch, FILE *fp ) );
void fread_morph_data args ( ( CHAR_DATA *ch, FILE *fp ) );
void clear_char_morph args ( ( CHAR_MORPH *morph ) );
CHAR_MORPH * make_char_morph args ( ( MORPH_DATA *morph ) );
void free_char_morph args ( ( CHAR_MORPH *morph ) );
CHAR_MORPH * make_char_morph args ( ( MORPH_DATA *morph ) );
char *race_string  args ( ( int bitvector ) );
char *class_string  args ( ( int bitvector ) );
void setup_morph_vnum args ( ( void ) );
void unmorph_all args ( ( MORPH_DATA *morph ) );
MORPH_DATA *get_morph args ( ( char *arg ) );
MORPH_DATA *get_morph_vnum args ( ( int arg ) );
int do_morph_char args ( ( CHAR_DATA *ch, MORPH_DATA *morph ) );
MORPH_DATA *find_morph args ( ( CHAR_DATA *ch, char *target, bool is_cast));
void do_unmorph_char args ( ( CHAR_DATA *ch ) );
void send_morph_message args((CHAR_DATA *ch, MORPH_DATA *morph, bool is_morph));
bool can_morph args ( ( CHAR_DATA *ch, MORPH_DATA *morph, bool is_cast ) );
void do_morph args ( ( CHAR_DATA *ch, MORPH_DATA *morph ) );
void do_unmorph args ( ( CHAR_DATA *ch ) );
void save_morphs args ( ( void ) );
void fwrite_morph args ( ( FILE *fp, MORPH_DATA *morph ) );
void load_morphs args ( ( void ) );
MORPH_DATA *fread_morph args ( ( FILE *fp ) );
void free_morph args ( ( MORPH_DATA *morph ) );
void morph_defaults args ( ( MORPH_DATA *morph ) );
void sort_morphs args ( ( void ) );


/* skills.c */
bool    can_use_skill           args( ( CHAR_DATA *ch, int percent, int gsn));
bool	check_skill		args( ( CHAR_DATA *ch, char *command, char *argument ) );
void	learn_from_success	args( ( CHAR_DATA *ch, int sn ) );
void	learn_from_failure	args( ( CHAR_DATA *ch, int sn ) );
bool	check_parry		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_tumble            args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool 	check_grip		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm			args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
void	trip			args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
bool	mob_fire		args( ( CHAR_DATA *ch, char *name ) );
CD *	scan_for_victim		args( ( CHAR_DATA *ch, EXIT_DATA *pexit,
					 char *name ) );

/* ban.c */
int add_ban args( ( CHAR_DATA *ch, char *arg1, char *arg2,int time,int type ) );
void show_bans args ( ( CHAR_DATA *ch, int type ) );
void save_banlist args ( ( void ) );
void load_banlist args ( ( void ) );
bool check_total_bans args ( ( DESCRIPTOR_DATA *d ) );
bool check_bans args ( ( CHAR_DATA *ch, int type ) );

/* imm_host.c */
bool check_immortal_domain args ( ( CHAR_DATA *ch, char *host ) );
int  load_imm_host args ( ( void ) );
int  fread_imm_host args ( ( FILE *fp, IMMORTAL_HOST *data ) );
void do_write_imm_host args (( void ));
void do_add_imm_host args (( CHAR_DATA *ch, char *argument ));


/* handler.c */
AREA_DATA *  get_area_obj args( ( OBJ_INDEX_DATA * obj));
int	get_exp		args( ( CHAR_DATA *ch ) );
int	get_exp_worth	args( ( CHAR_DATA *ch ) );
int	exp_level	args( ( CHAR_DATA *ch, sh_int level ) );
sh_int	get_trust	args( ( CHAR_DATA *ch ) );
sh_int	get_age		args( ( CHAR_DATA *ch ) );
sh_int	get_curr_str	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_int	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_wis	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_dex	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_con	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_cha	args( ( CHAR_DATA *ch ) );
sh_int  get_curr_lck	args( ( CHAR_DATA *ch ) );
bool	can_take_proto	args( ( CHAR_DATA *ch ) );
int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
bool	is_name		args( ( const char *str, char *namelist ) );
bool	is_name_prefix	args( ( const char *str, char *namelist ) );
bool	nifty_is_name	args( ( char *str, char *namelist ) );
bool	nifty_is_name_prefix args( ( char *str, char *namelist ) );
void	affect_modify	args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_strip	args( ( CHAR_DATA *ch, int sn ) );
bool	is_affected	args( ( CHAR_DATA *ch, int sn ) );
void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
OD *	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
int	apply_ac	args( ( OBJ_DATA *obj, int iWear ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
OD *	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
OD *	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj ) );
void	extract_exit	args( ( ROOM_INDEX_DATA *room, EXIT_DATA *pexit ) );
void	extract_room	args( ( ROOM_INDEX_DATA *room ) );
void	clean_room	args( ( ROOM_INDEX_DATA *room ) );
void	clean_obj	args( ( OBJ_INDEX_DATA *obj ) );
void	clean_mob	args( ( MOB_INDEX_DATA *mob ) );
void	clean_resets	args( ( AREA_DATA *tarea ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
CD *	get_char_room	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_list_rev args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_wear	args( ( CHAR_DATA *ch, char *argument ) );
OD *    get_obj_vnum    args( ( CHAR_DATA *ch, int vnum ) );
OD *	get_obj_here	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
int	get_real_obj_weight args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	room_is_private	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
CD	*room_is_dnd	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *	item_type_name	args( ( OBJ_DATA *obj ) );
char *	affect_loc_name	args( ( int location ) );
char *	affect_bit_name	args( ( EXT_BV *vector ) );
char *	extra_bit_name	args( ( EXT_BV *extra_flags ) );
char *	magic_bit_name	args( ( int magic_flags ) );
char *  pull_type_name	args( ( int pulltype ) );
ch_ret	check_for_trap	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int flag ) );
ch_ret	check_room_for_traps args( ( CHAR_DATA *ch, int flag ) );
bool	is_trapped	args( ( OBJ_DATA *obj ) );
OD *	get_trap	args( ( OBJ_DATA *obj ) );
ch_ret	spring_trap     args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void	name_stamp_stats args( ( CHAR_DATA *ch ) );
void	fix_char	args( ( CHAR_DATA *ch ) );
void	showaffect	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	set_cur_obj	args( ( OBJ_DATA *obj ) );
bool	obj_extracted	args( ( OBJ_DATA *obj ) );
void	queue_extracted_obj	args( ( OBJ_DATA *obj ) );
void	clean_obj_queue	args( ( void ) );
void	set_cur_char	args( ( CHAR_DATA *ch ) );
bool	char_died	args( ( CHAR_DATA *ch ) );
void	queue_extracted_char	args( ( CHAR_DATA *ch, bool extract ) );
void	clean_char_queue	args( ( void ) );
void	add_timer	args( ( CHAR_DATA *ch, sh_int type, sh_int count, DO_FUN *fun, int value ) );
TIMER * get_timerptr	args( ( CHAR_DATA *ch, sh_int type ) );
sh_int	get_timer	args( ( CHAR_DATA *ch, sh_int type ) );
void	extract_timer	args( ( CHAR_DATA *ch, TIMER *timer ) );
void	remove_timer	args( ( CHAR_DATA *ch, sh_int type ) );
bool	in_soft_range	args( ( CHAR_DATA *ch, AREA_DATA *tarea ) );
bool	in_hard_range	args( ( CHAR_DATA *ch, AREA_DATA *tarea ) );
bool	chance  	args( ( CHAR_DATA *ch, sh_int percent ) );
bool 	chance_attrib	args( ( CHAR_DATA *ch, sh_int percent, sh_int attrib ) );
OD *	clone_object	args( ( OBJ_DATA *obj ) );
void	split_obj	args( ( OBJ_DATA *obj, int num ) );
void	separate_obj	args( ( OBJ_DATA *obj ) );
bool	empty_obj	args( ( OBJ_DATA *obj, OBJ_DATA *destobj,
				ROOM_INDEX_DATA *destroom ) );
OD *	find_obj	args( ( CHAR_DATA *ch, char *argument,
				bool carryonly ) );
bool	ms_find_obj	args( ( CHAR_DATA *ch ) );
void	worsen_mental_state args( ( CHAR_DATA *ch, int mod ) );
void	better_mental_state args( ( CHAR_DATA *ch, int mod ) );
void	boost_economy	args( ( AREA_DATA *tarea, int gold ) );
void	lower_economy	args( ( AREA_DATA *tarea, int gold ) );
void	economize_mobgold args( ( CHAR_DATA *mob ) );
bool	economy_has	args( ( AREA_DATA *tarea, int gold ) );
void	add_kill	args( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
int	times_killed	args( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
void	update_aris	args( ( CHAR_DATA *ch) );
AREA_DATA *get_area	args( ( char *name ) ); /* FB */
OD *	get_objtype	args( ( CHAR_DATA *ch, sh_int type ) );

/* interp.c */
bool	check_pos	args( ( CHAR_DATA *ch, sh_int position ) );
void	interpret	args( ( CHAR_DATA *ch, char *argument ) );
bool	is_number	args( ( char *arg ) );
int	number_argument	args( ( char *argument, char *arg ) );
char *	one_argument	args( ( char *argument, char *arg_first ) );
char *	one_argument2	args( ( char *argument, char *arg_first ) );
ST *	find_social	args( ( char *command ) );
CMDTYPE *find_command	args( ( char *command ) );
void	hash_commands	args( ( ) );
void	start_timer	args( ( struct timeval *stime ) );
time_t	end_timer	args( ( struct timeval *stime ) );
void	send_timer	args( ( struct timerset *vtime, CHAR_DATA *ch ) );
void	update_userec	args( ( struct timeval *time_used,
				struct timerset *userec ) );

/* magic.c */
bool	process_spell_components args( ( CHAR_DATA *ch, int sn ) );
int	ch_slookup	args( ( CHAR_DATA *ch, const char *name ) );
int	find_spell	args( ( CHAR_DATA *ch, const char *name, bool know ) );
int	find_skill	args( ( CHAR_DATA *ch, const char *name, bool know ) );
int	find_weapon	args( ( CHAR_DATA *ch, const char *name, bool know ) );
int	find_tongue	args( ( CHAR_DATA *ch, const char *name, bool know ) );
int	skill_lookup	args( ( const char *name ) );
int	herb_lookup	args( ( const char *name ) );
int	personal_lookup	args( ( CHAR_DATA *ch, const char *name ) );
int	slot_lookup	args( ( int slot ) );
int	bsearch_skill	args( ( const char *name, int first, int top ) );
int	bsearch_skill_exact args( ( const char *name, int first, int top ) );
int	bsearch_skill_prefix args( ( const char *name, int first, int top ) );
bool	saves_poison_death	args( ( int level, CHAR_DATA *victim ) );
bool	saves_wand		args( ( int level, CHAR_DATA *victim ) );
bool	saves_para_petri	args( ( int level, CHAR_DATA *victim ) );
bool	saves_breath		args( ( int level, CHAR_DATA *victim ) );
bool	saves_spell_staff	args( ( int level, CHAR_DATA *victim ) );
ch_ret	obj_cast_spell	args( ( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj ) );
int	dice_parse	args( (CHAR_DATA *ch, int level, char *exp) );
SK *	get_skilltype	args( ( int sn ) );
sh_int	get_chain_type	args( ( ch_ret retcode ) );
ch_ret	chain_spells	args( ( int sn, int level, CHAR_DATA *ch, void *vo, sh_int chain ) );

/* request.c */
void	init_request_pipe	args( ( void ) );
void	check_requests		args( ( void ) );

/* save.c */
/* object saving defines for fread/write_obj. -- Altrag */
#define OS_CARRY	0
#define OS_CORPSE	1
void	save_char_obj	args( ( CHAR_DATA *ch ) );
bool	load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name, bool preload ) );
void	set_alarm	args( ( long seconds ) );
void	requip_char	args( ( CHAR_DATA *ch ) );
void    fwrite_obj      args( ( CHAR_DATA *ch,  OBJ_DATA  *obj, FILE *fp, 
				int iNest, sh_int os_type ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp, sh_int os_type ) );
void	de_equip_char	args( ( CHAR_DATA *ch ) );
void	re_equip_char	args( ( CHAR_DATA *ch ) );
void    read_char_mobile args( ( char *argument ) );
void    write_char_mobile args( (  CHAR_DATA *ch, char *argument ) );
CHAR_DATA * fread_mobile args( ( FILE *fp ) );
void    fwrite_mobile	args ( ( FILE *fp, CHAR_DATA *mob ) );

/* shops.c */

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );
char *	lookup_spec	args( ( SPEC_FUN *special ) );

/* tables.c */
int	get_skill	args( ( char *skilltype ) );
char *	spell_name	args( ( SPELL_FUN *spell ) );
char *	skill_name	args( ( DO_FUN *skill ) );
void	load_skill_table args( ( void ) );
void	save_skill_table args( ( void ) );
void	sort_skill_table args( ( void ) );
void	remap_slot_numbers args( ( void ) );
void	load_socials	args( ( void ) );
void	save_socials	args( ( void ) );
void	load_commands	args( ( void ) );
void	save_commands	args( ( void ) );
SPELL_FUN *spell_function args( ( char *name ) );
DO_FUN *skill_function  args( ( char *name ) );
void	write_class_file args( ( int cl ) );
void	save_classes	args( ( void ) );
void	load_classes	args( ( void ) );
void	load_herb_table	args( ( void ) );
void	save_herb_table	args( ( void ) );
void	load_races	args( ( void ) );
void	load_tongues	args( ( void ) );

/* track.c */
void	found_prey	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	hunt_victim	args( ( CHAR_DATA *ch) );

/* update.c */
void	advance_level	args( ( CHAR_DATA *ch ) );
void	gain_exp	args( ( CHAR_DATA *ch, int gain ) );
void	gain_condition	args( ( CHAR_DATA *ch, int iCond, int value ) );
void    check_alignment args( ( CHAR_DATA *ch ) );
void	update_handler	args( ( void ) );
void	reboot_check	args( ( time_t reset ) );
#if 0
void    reboot_check    args( ( char *arg ) );
#endif
void    auction_update  args( ( void ) );
void	remove_portal	args( ( OBJ_DATA *portal ) );
void	weather_update	args( ( void ) );

/* hashstr.c */
char *	str_alloc	args( ( char *str ) );
char *	quick_link	args( ( char *str ) );
int	str_free	args( ( char *str ) );
void	show_hash	args( ( int count ) );
char *	hash_stats	args( ( void ) );
char *	check_hash	args( ( char *str ) );
void	hash_dump	args( ( int hash ) );
void	show_high_hash	args( ( int top ) );

/* newscore.c */
char *  get_class 	args( (CHAR_DATA *ch) );
char *  get_race 	args( (CHAR_DATA *ch) );

#undef	SK
#undef	CO
#undef	ST
#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	BD 
#undef	CL
#undef	EDD
#undef	RD
#undef	ED

/*
 *
 *  New Build Interface Stuff Follows
 *
 */


/*
 *  Data for a menu page
 */
struct	menu_data
{
    char		*sectionNum;
    char		*charChoice;
    int			x;
    int			y;
    char		*outFormat;
    void		*data;
    int			ptrType;
    int			cmdArgs;
    char		*cmdString;
};

DECLARE_DO_FUN( do_redraw_page  );
DECLARE_DO_FUN( do_refresh_page );
DECLARE_DO_FUN( do_pagelen	);
DECLARE_DO_FUN( do_omenu  	);
DECLARE_DO_FUN( do_rmenu  	);
DECLARE_DO_FUN( do_mmenu  	);
DECLARE_DO_FUN( do_clear  	);

extern		MENU_DATA		room_page_a_data[];
extern		MENU_DATA		room_page_b_data[];
extern		MENU_DATA		room_page_c_data[];
extern		MENU_DATA		room_help_page_data[];

extern		MENU_DATA		mob_page_a_data[];
extern		MENU_DATA		mob_page_b_data[];
extern		MENU_DATA		mob_page_c_data[];
extern		MENU_DATA		mob_page_d_data[];
extern		MENU_DATA		mob_page_e_data[];
extern		MENU_DATA		mob_page_f_data[];
extern		MENU_DATA		mob_help_page_data[];

extern		MENU_DATA		obj_page_a_data[];
extern		MENU_DATA		obj_page_b_data[];
extern		MENU_DATA		obj_page_c_data[];
extern		MENU_DATA		obj_page_d_data[];
extern		MENU_DATA		obj_page_e_data[];
extern		MENU_DATA		obj_help_page_data[];

extern		MENU_DATA		control_page_a_data[];
extern		MENU_DATA		control_help_page_data[];

extern	const   char    room_page_a[];
extern	const   char    room_page_b[];
extern	const   char    room_page_c[];
extern	const   char    room_help_page[];

extern	const   char    obj_page_a[];
extern	const   char    obj_page_b[];
extern	const   char    obj_page_c[];
extern	const   char    obj_page_d[];
extern	const   char    obj_page_e[];
extern	const   char    obj_help_page[];

extern	const   char    mob_page_a[];
extern	const   char    mob_page_b[];
extern	const   char    mob_page_c[];
extern	const   char    mob_page_d[];
extern	const   char    mob_page_e[];
extern	const   char    mob_page_f[];
extern	const   char    mob_help_page[];
extern	const   char *  npc_sex[3];
extern	const   char *  ris_strings[];

extern	const   char    control_page_a[];
extern	const   char    control_help_page[];

#define SH_INT 1
#define INT 2
#define CHAR 3
#define STRING 4
#define SPECIAL 5


#define NO_PAGE    0
#define MOB_PAGE_A 1
#define MOB_PAGE_B 2
#define MOB_PAGE_C 3
#define MOB_PAGE_D 4
#define MOB_PAGE_E 5
#define MOB_PAGE_F 17
#define MOB_HELP_PAGE 14
#define ROOM_PAGE_A 6
#define ROOM_PAGE_B 7
#define ROOM_PAGE_C 8
#define ROOM_HELP_PAGE 15
#define OBJ_PAGE_A 9
#define OBJ_PAGE_B 10
#define OBJ_PAGE_C 11
#define OBJ_PAGE_D 12
#define OBJ_PAGE_E 13
#define OBJ_HELP_PAGE 16
#define CONTROL_PAGE_A 18
#define CONTROL_HELP_PAGE 19

#define NO_TYPE   0
#define MOB_TYPE  1
#define OBJ_TYPE  2
#define ROOM_TYPE 3
#define CONTROL_TYPE 4

#define SUB_NORTH DIR_NORTH
#define SUB_EAST  DIR_EAST
#define SUB_SOUTH DIR_SOUTH
#define SUB_WEST  DIR_WEST
#define SUB_UP    DIR_UP
#define SUB_DOWN  DIR_DOWN
#define SUB_NE    DIR_NORTHEAST
#define SUB_NW    DIR_NORTHWEST
#define SUB_SE    DIR_SOUTHEAST
#define SUB_SW    DIR_SOUTHWEST

/*
 * defines for use with this get_affect function
 */

#define RIS_000		BV00
#define RIS_R00		BV01
#define RIS_0I0		BV02
#define RIS_RI0		BV03
#define RIS_00S		BV04
#define RIS_R0S		BV05
#define RIS_0IS		BV06
#define RIS_RIS		BV07

#define GA_AFFECTED	BV09
#define GA_RESISTANT	BV10
#define GA_IMMUNE	BV11
#define GA_SUSCEPTIBLE	BV12
#define GA_RIS          BV30



/*
 *   Map Structures
 */

DECLARE_DO_FUN( do_mapout 	);
DECLARE_DO_FUN( do_lookmap	);

struct  map_data	/* contains per-room data */
{
  int vnum;		/* which map this room belongs to */
  int x;		/* horizontal coordinate */
  int y;		/* vertical coordinate */
  char entry;		/* code that shows up on map */ 
};


struct  map_index_data
{
  MAP_INDEX_DATA  *next;
  int 		  vnum;  		  /* vnum of the map */
  int             map_of_vnums[49][81];   /* room vnums aranged as a map */
};


MAP_INDEX_DATA *get_map_index(int vnum);
void            init_maps();


/*
 * mudprograms stuff
 */
extern	CHAR_DATA *supermob;

void oprog_speech_trigger( char *txt, CHAR_DATA *ch );
void oprog_random_trigger( OBJ_DATA *obj );
void oprog_wear_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
bool oprog_use_trigger( CHAR_DATA *ch, OBJ_DATA *obj, 
                        CHAR_DATA *vict, OBJ_DATA *targ, void *vo );
void oprog_remove_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_sac_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_damage_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_repair_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_drop_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_zap_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
char *oprog_type_to_name( int type );

/*
 * MUD_PROGS START HERE
 * (object stuff)
 */
void oprog_greet_trigger( CHAR_DATA *ch );
void oprog_speech_trigger( char *txt, CHAR_DATA *ch );
void oprog_random_trigger( OBJ_DATA *obj );
void oprog_random_trigger( OBJ_DATA *obj );
void oprog_remove_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_sac_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_get_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_damage_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_repair_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_drop_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_examine_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_zap_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_pull_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_push_trigger( CHAR_DATA *ch, OBJ_DATA *obj );


/* mud prog defines */

#define ERROR_PROG        -1
#define IN_FILE_PROG      -2

typedef enum
{
  ACT_PROG, SPEECH_PROG, RAND_PROG, FIGHT_PROG, DEATH_PROG, HITPRCNT_PROG, 
  ENTRY_PROG, GREET_PROG, ALL_GREET_PROG, GIVE_PROG, BRIBE_PROG, HOUR_PROG, 
  TIME_PROG, WEAR_PROG, REMOVE_PROG, SAC_PROG, LOOK_PROG, EXA_PROG, ZAP_PROG, 
  GET_PROG, DROP_PROG, DAMAGE_PROG, REPAIR_PROG, RANDIW_PROG, SPEECHIW_PROG, 
  PULL_PROG, PUSH_PROG, SLEEP_PROG, REST_PROG, LEAVE_PROG, SCRIPT_PROG,
  USE_PROG
} prog_types;

/*
 * For backwards compatability
 */
#define RDEATH_PROG DEATH_PROG
#define ENTER_PROG  ENTRY_PROG
#define RFIGHT_PROG FIGHT_PROG
#define RGREET_PROG GREET_PROG
#define OGREET_PROG GREET_PROG

void rprog_leave_trigger( CHAR_DATA *ch );
void rprog_enter_trigger( CHAR_DATA *ch );
void rprog_sleep_trigger( CHAR_DATA *ch );
void rprog_rest_trigger( CHAR_DATA *ch );
void rprog_rfight_trigger( CHAR_DATA *ch );
void rprog_death_trigger( CHAR_DATA *killer, CHAR_DATA *ch );
void rprog_speech_trigger( char *txt, CHAR_DATA *ch );
void rprog_random_trigger( CHAR_DATA *ch );
void rprog_time_trigger( CHAR_DATA *ch );
void rprog_hour_trigger( CHAR_DATA *ch );
char *rprog_type_to_name( int type );

#define OPROG_ACT_TRIGGER
#ifdef OPROG_ACT_TRIGGER
void oprog_act_trigger( char *buf, OBJ_DATA *mobj, CHAR_DATA *ch,
			OBJ_DATA *obj, void *vo );
#endif
#define RPROG_ACT_TRIGGER
#ifdef RPROG_ACT_TRIGGER
void rprog_act_trigger( char *buf, ROOM_INDEX_DATA *room, CHAR_DATA *ch,
			OBJ_DATA *obj, void *vo );
#endif


#define GET_ADEPT(ch,sn)    (  skill_table[(sn)]->skill_adept[(ch)->class])
#define LEARNED(ch,sn)	    (IS_NPC(ch) ? 80 : URANGE(0, ch->pcdata->learned[sn], 101))

/* Structure and macros for using long bit vectors */
#define CHAR_SIZE sizeof(char)

typedef char * LONG_VECTOR;

#define LV_CREATE(vector, bit_length)					\
do									\
{									\
	int i;								\
	CREATE(vector, char, 1 + bit_length/CHAR_SIZE);			\
									\
	for(i = 0; i <= bit_length/CHAR_SIZE; i++)			\
		*(vector + i) = 0;					\
}while(0)

#define LV_IS_SET(vector, index)					\
	(*(vector + index/CHAR_SIZE) & (1 << index%CHAR_SIZE))

#define LV_SET_BIT(vector, index)					\
	(*(vector + index/CHAR_SIZE) |= (1 << index%CHAR_SIZE))

#define LV_REMOVE_BIT(vector, index)					\
	(*(vector + index/CHAR_SIZE) &= ~(1 << index%CHAR_SIZE))
	
#define LV_TOGGLE_BIT(vector, index)					\
	(*(vector + index/CHAR_SIZE) ^= (1 << index%CHAR_SIZE))
	
#ifdef WIN32
void gettimeofday(struct timeval *tv, struct timezone *tz);
void kill_timer();

/* directory scanning stuff */

typedef struct dirent
{
    char *	d_name;
};

typedef struct  
{
    HANDLE		hDirectory;
    WIN32_FIND_DATA	Win32FindData;
    struct dirent	dirinfo;
    char		sDirName[MAX_PATH];
} DIR;


DIR *opendir(char * sDirName);
struct dirent *readdir (DIR * dp);
void closedir(DIR * dp);

/* --------------- Stuff for Win32 services ------------------ */
/*

   NJG:

   When "exit" is called to handle an error condition, we really want to
   terminate the game thread, not the whole process.

 */

#define exit(arg) Win32_Exit(arg)
void Win32_Exit(int exit_code);

#endif
