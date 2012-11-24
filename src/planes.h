/* Copy this into MUD.H once planes.c is in working order and ready to
   be linked.. -- Altrag */

typedef struct plane_data PLANE_DATA;
typedef struct afswap_data AFSWAP_DATA;
typedef struct snswap_data SNSWAP_DATA;

struct weather_data
{
   int mmhg;
   int change;
   int sky;
   int sunlight;
   int temp;
};

struct plane_data
{
   PLANE_DATA *next;
   PLANE_DATA *prev;
   AFSWAP_DATA *first_afswap;
   AFSWAP_DATA *last_afswap;
   SNSWAP_DATA *first_snswap;
   SNSWAP_DATA *last_snswap;
   char *name;
   int stronger;
   int weaker;
   int nullified;
   int reverse;
   int reflected;
   short month_ofs;
   short mintemp;
   short maxtemp;
   short climate;
   short gravity;
   TIME_INFO_DATA time_info;
   WEATHER_DATA weather_data;
};

#define CLIMATE_ARCTIC       0   /* VERY cold */
#define CLIMATE_SUBARCTIC    1   /* cold, some rainfall */
#define CLIMATE_COASTAL      2   /* cold, lots of rainfall */
#define CLIMATE_TROPICAL     4   /* warm, lots of rainfall */
#define CLIMATE_HUMID        3   /* warm, some rainfall */
#define CLIMATE_ARID         5   /* warm, dry */
#define CLIMATE_DESERT       6   /* VERY hot */

#define GRAVITY_NORMAL       0
#define GRAVITY_ZERO         1
#define GRAVITY_REVERSE      2

struct snswap_data
{
   SNSWAP_DATA *next;
   SNSWAP_DATA *prev;
   short old_sn;
   short new_sn;
   short swap_chance;
};

struct afswap_data
{
   AFSWAP_DATA *next;
   AFSWAP_DATA *prev;
   int old_af;
   int new_af;
   short swap_chance;
};
