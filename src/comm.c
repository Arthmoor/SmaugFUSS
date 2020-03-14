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
 *                      Low-level communication module                      *
 ****************************************************************************/

#include <stdio.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "mud.h"
#include "mccp.h"
#include "mssp.h"
#include "sha256.h"

/*
 * Socket and TCP/IP stuff.
 */
#ifdef WIN32
#include <io.h>
#include <winsock2.h>
#undef EINTR
#undef EMFILE
#define EINTR WSAEINTR
#define EMFILE WSAEMFILE
#define EWOULDBLOCK WSAEWOULDBLOCK
#define MAXHOSTNAMELEN 32

#define  TELOPT_ECHO        '\x01'
#define  GA                 '\xF9'
#define  SB                 '\xFA'
#define  WILL               '\xFB'
#define  WONT               '\xFC'
#define  DO                 '\xFD'
#define  DONT               '\xFE'
#define  IAC                '\xFF'
void bailout( void );
void shutdown_checkpoint( void );
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>
#endif

#ifdef IMC
void imc_delete_info( void );
void free_imcdata( bool complete );
#endif
void dispose_ban( BAN_DATA * ban, int type );
void close_all_areas( void );
void free_commands( void );
void free_prog_actlists( void );
void free_boards( void );
void free_skills( void );
void free_socials( void );
void free_teleports( void );
void free_tongues( void );
void free_helps( void );
void free_watchlist( void );
void free_projects( void );
void free_bans( void );
void free_clans( void );
void free_morphs( void );
void free_deities( void );
void free_all_planes( void );
void free_councils( void );
void free_specfuns( void );
void free_all_reserved( void );

const unsigned char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const unsigned char echo_on_str[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const unsigned char go_ahead_str[] = { IAC, GA, '\0' };

void save_sysdata( SYSTEM_DATA sys );

/*
 * Global variables.
 */
IMMORTAL_HOST *immortal_host_start; /* Start of Immortal legal domains */
IMMORTAL_HOST *immortal_host_end;   /* End of Immortal legal domains */
DESCRIPTOR_DATA *first_descriptor;  /* First descriptor     */
DESCRIPTOR_DATA *last_descriptor;   /* Last descriptor      */
DESCRIPTOR_DATA *d_next;   /* Next descriptor in loop */
int num_descriptors;
bool mud_down; /* Shutdown       */
bool service_shut_down; /* Shutdown by operator closing down service */
time_t boot_time;
HOUR_MIN_SEC set_boot_time_struct;
HOUR_MIN_SEC *set_boot_time;
struct tm *new_boot_time;
struct tm new_boot_struct;
char str_boot_time[MAX_INPUT_LENGTH];
char lastplayercmd[MAX_INPUT_LENGTH * 2];
time_t current_time; /* Time of this pulse      */
int control;   /* Controlling descriptor  */
int newdesc;   /* New descriptor    */
fd_set in_set; /* Set of desc's for reading  */
fd_set out_set;   /* Set of desc's for writing  */
fd_set exc_set;   /* Set of desc's with errors  */
int maxdesc;
const char *alarm_section = "(unknown)";
bool winter_freeze = FALSE;

/*
 * OS-dependent local functions.
 */
void game_loop( void );
int init_socket( int mudport );
void new_descriptor( int new_desc );
bool read_from_descriptor( DESCRIPTOR_DATA * d );
bool write_to_descriptor( DESCRIPTOR_DATA * d, const char *txt, int length );

/*
 * Other local functions (OS-independent).
 */
bool check_parse_name( const char *name, bool newchar );
short check_reconnect( DESCRIPTOR_DATA * d, const char *name, bool fConn );
short check_playing( DESCRIPTOR_DATA * d, const char *name, bool kick );
int main( int argc, char **argv );
void nanny( DESCRIPTOR_DATA * d, char *argument );
bool flush_buffer( DESCRIPTOR_DATA * d, bool fPrompt );
void read_from_buffer( DESCRIPTOR_DATA * d );
void stop_idling( CHAR_DATA * ch );
void free_desc( DESCRIPTOR_DATA * d );
void display_prompt( DESCRIPTOR_DATA * d );
void set_pager_input( DESCRIPTOR_DATA * d, char *argument );
bool pager_output( DESCRIPTOR_DATA * d );
void mail_count( CHAR_DATA * ch );

int port;

/*
 * Clean all memory on exit to help find leaks
 * Yeah I know, one big ugly function -Druid
 * Added to AFKMud by Samson on 5-8-03.
 */
void cleanup_memory( void )
{
   int hash, loopa;
   CHAR_DATA *character;
   OBJ_DATA *object;
   DESCRIPTOR_DATA *desc, *desc_next;

#ifdef IMC
   fprintf( stdout, "%s", "IMC2 Data.\n" );
   free_imcdata( TRUE );
   imc_delete_info(  );
#endif

   fprintf( stdout, "%s", "Project Data.\n" );
   free_projects(  );

   fprintf( stdout, "%s", "Reserved Names.\n" );
   free_all_reserved(  );

   fprintf( stdout, "%s", "Ban Data.\n" );
   free_bans(  );

   fprintf( stdout, "%s", "Morph Data.\n" );
   free_morphs(  );

   /*
    * Commands 
    */
   fprintf( stdout, "%s", "Commands.\n" );
   free_commands(  );

   /*
    * Deities 
    */
   fprintf( stdout, "%s", "Deities.\n" );
   free_deities(  );

   /*
    * Clans 
    */
   fprintf( stdout, "%s", "Clans.\n" );
   free_clans(  );

   /*
    * Councils
    */
   fprintf( stdout, "%s", "Councils.\n" );
   free_councils(  );

   /*
    * socials 
    */
   fprintf( stdout, "%s", "Socials.\n" );
   free_socials(  );

   /*
    * Watches 
    */
   fprintf( stdout, "%s", "Watches.\n" );
   free_watchlist(  );

   /*
    * Helps 
    */
   fprintf( stdout, "%s", "Helps.\n" );
   free_helps(  );

   /*
    * Languages 
    */
   fprintf( stdout, "%s", "Languages.\n" );
   free_tongues(  );

   /*
    * Boards 
    */
   fprintf( stdout, "%s", "Boards.\n" );
   free_boards(  );

   /*
    * Whack supermob 
    */
   fprintf( stdout, "%s", "Whacking supermob.\n" );
   if( supermob )
   {
      char_from_room( supermob );
      UNLINK( supermob, first_char, last_char, next, prev );
      free_char( supermob );
   }

   /*
    * Free Objects 
    */
   clean_obj_queue(  );
   fprintf( stdout, "%s", "Objects.\n" );
   while( ( object = last_object ) != NULL )
      extract_obj( object );
   clean_obj_queue(  );

   /*
    * Free Characters 
    */
   clean_char_queue(  );
   fprintf( stdout, "%s", "Characters.\n" );
   while( ( character = last_char ) != NULL )
      extract_char( character, TRUE );
   clean_char_queue(  );

   /*
    * Descriptors 
    */
   fprintf( stdout, "%s", "Descriptors.\n" );
   for( desc = first_descriptor; desc; desc = desc_next )
   {
      desc_next = desc->next;
      UNLINK( desc, first_descriptor, last_descriptor, next, prev );
      free_desc( desc );
   }

   /*
    * Liquids 
    */
   fprintf( stdout, "%s", "Liquid Table.\n" );
   free_liquiddata(  );

   /*
    * Races 
    */
   fprintf( stdout, "%s", "Races.\n" );
   for( hash = 0; hash < MAX_RACE; hash++ )
   {
      for( loopa = 0; loopa < MAX_WHERE_NAME; loopa++ )
         DISPOSE( race_table[hash]->where_name[loopa] );
      DISPOSE( race_table[hash] );
   }

   /*
    * Classes 
    */
   fprintf( stdout, "%s", "Classes.\n" );
   for( hash = 0; hash < MAX_CLASS; hash++ )
   {
      STRFREE( class_table[hash]->who_name );
      DISPOSE( class_table[hash] );
   }

   /*
    * Teleport lists 
    */
   fprintf( stdout, "%s", "Teleport Data.\n" );
   free_teleports(  );

   /*
    * Areas - this includes killing off the hash tables and such 
    */
   fprintf( stdout, "%s", "Area Data Tables.\n" );
   close_all_areas(  );

   /*
    * Planes
    */
   fprintf( stdout, "%s", "Planes.\n" );
   free_all_planes(  );

   /*
    * Get rid of auction pointer  MUST BE AFTER OBJECTS DESTROYED 
    */
   fprintf( stdout, "%s", "Auction.\n" );
   DISPOSE( auction );

   // MSSP Info
   fprintf( stdout, "%s", "MSSP Info.\n" );
   free_mssp_info();

   /*
    * System Data 
    */
   fprintf( stdout, "%s", "System data.\n" );
   if( sysdata.time_of_max )
      DISPOSE( sysdata.time_of_max );
   if( sysdata.mud_name )
      DISPOSE( sysdata.mud_name );
   if( sysdata.guild_overseer )
      STRFREE( sysdata.guild_overseer );
   if( sysdata.guild_advisor )
      STRFREE( sysdata.guild_advisor );

   /*
    * Title table 
    */
   fprintf( stdout, "%s", "Title table.\n" );
   for( hash = 0; hash < MAX_CLASS; hash++ )
   {
      for( loopa = 0; loopa < MAX_LEVEL + 1; loopa++ )
      {
         DISPOSE( title_table[hash][loopa][0] );
         DISPOSE( title_table[hash][loopa][1] );
      }
   }

   /*
    * Skills 
    */
   fprintf( stdout, "%s", "Skills and Herbs.\n" );
   free_skills(  );

   /*
    * Prog Act lists 
    */
   fprintf( stdout, "%s", "Mudprog act lists.\n" );
   free_prog_actlists(  );

   fprintf( stdout, "%s", "Specfun lists.\n" );
   free_specfuns(  );

   /*
    * Some freaking globals 
    */
   fprintf( stdout, "%s", "Globals.\n" );
   DISPOSE( ranged_target_name );

   fprintf( stdout, "%s", "Checking string hash for leftovers.\n" );
   {
      for( hash = 0; hash < 1024; hash++ )
         hash_dump( hash );
   }

   fprintf( stdout, "%s", "Cleanup complete, exiting.\n" );
   return;
}  /* cleanup memory */

#ifdef WIN32
int mainthread( int argc, char **argv )
#else
int main( int argc, char **argv )
#endif
{
   struct timeval now_time;
   bool fCopyOver = FALSE;
#ifdef IMC
   int imcsocket = -1;
#endif

   DONT_UPPER = FALSE;
   num_descriptors = 0;
   first_descriptor = NULL;
   last_descriptor = NULL;
   sysdata.NO_NAME_RESOLVING = TRUE;
   sysdata.WAIT_FOR_AUTH = TRUE;

   /*
    * Init time.
    */
   gettimeofday( &now_time, NULL );
   current_time = ( time_t ) now_time.tv_sec;
   boot_time = time( 0 );  /*  <-- I think this is what you wanted */
   mudstrlcpy( str_boot_time, ctime( &current_time ), MAX_INPUT_LENGTH );

   /*
    * Init boot time.
    */
   set_boot_time = &set_boot_time_struct;
   set_boot_time->manual = 0;

   new_boot_time = update_time( localtime( &current_time ) );
   /*
    * Copies *new_boot_time to new_boot_struct, and then points
    * new_boot_time to new_boot_struct again. -- Alty 
    */
   new_boot_struct = *new_boot_time;
   new_boot_time = &new_boot_struct;
   new_boot_time->tm_mday += 1;
   if( new_boot_time->tm_hour > 12 )
      new_boot_time->tm_mday += 1;
   new_boot_time->tm_sec = 0;
   new_boot_time->tm_min = 0;
   new_boot_time->tm_hour = 6;

   /*
    * Update new_boot_time (due to day increment) 
    */
   new_boot_time = update_time( new_boot_time );
   new_boot_struct = *new_boot_time;
   new_boot_time = &new_boot_struct;
   /*
    * Bug fix submitted by Gabe Yoder 
    */
   new_boot_time_t = mktime( new_boot_time );
   reboot_check( mktime( new_boot_time ) );
   /*
    * Set reboot time string for do_time 
    */
   get_reboot_string(  );

   /*
    * Get the port number.
    */
   port = 4000;
   if( argc > 1 )
   {
      if( !is_number( argv[1] ) )
      {
         fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
         exit( 1 );
      }
      else if( ( port = atoi( argv[1] ) ) <= 1024 )
      {
         fprintf( stderr, "Port number must be above 1024.\n" );
         exit( 1 );
      }
      if( argv[2] && argv[2][0] )
      {
         fCopyOver = TRUE;
         control = atoi( argv[3] );
#ifdef IMC
         imcsocket = atoi( argv[4] );
#endif
      }
      else
         fCopyOver = FALSE;
   }

   /*
    * Run the game.
    */
#ifdef WIN32
   {
      /*
       * Initialise Windows sockets library 
       */

      unsigned short wVersionRequested = MAKEWORD( 1, 1 );
      WSADATA wsadata;
      int err;

      /*
       * Need to include library: wsock32.lib for Windows Sockets 
       */
      err = WSAStartup( wVersionRequested, &wsadata );
      if( err )
      {
         fprintf( stderr, "Error %i on WSAStartup\n", err );
         exit( 1 );
      }

      /*
       * standard termination signals 
       */
      signal( SIGINT, ( void * )bailout );
      signal( SIGTERM, ( void * )bailout );
   }
#endif /* WIN32 */

   log_string( "Booting Database" );
   boot_db( fCopyOver );
   log_string( "Initializing socket" );
   if( !fCopyOver )  /* We have already the port if copyover'ed */
      control = init_socket( port );

#ifdef IMC
   /*
    * Initialize and connect to IMC2 
    */
   imc_startup( FALSE, imcsocket, fCopyOver );
#endif

   log_printf( "%s ready on port %d.", sysdata.mud_name, port );

   if( fCopyOver )
   {
      log_string( "Initiating hotboot recovery." );
      hotboot_recover(  );
   }

   game_loop(  );

   close( control );

#ifdef IMC
   imc_shutdown( FALSE );
#endif

#ifdef WIN32
   /*
    * Shut down Windows sockets 
    */

   WSACleanup(  );   /* clean up */
   kill_timer(  );   /* stop timer thread */
#endif


   /*
    * That's all, folks.
    */
   log_string( "Normal termination of game." );

   log_string( "Cleaning up Memory." );
   cleanup_memory(  );

   exit( 0 );
}

int init_socket( int mudport )
{
   struct sockaddr_in sa;
   int x = 1;
   int fd;

   if( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
   {
      perror( "Init_socket: socket" );
      exit( 1 );
   }

   if( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, ( void * )&x, sizeof( x ) ) < 0 )
   {
      perror( "Init_socket: SO_REUSEADDR" );
      close( fd );
      exit( 1 );
   }

#if defined(SO_DONTLINGER) && !defined(SYSV)
   {
      struct linger ld;

      ld.l_onoff = 1;
      ld.l_linger = 1000;

      if( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER, ( void * )&ld, sizeof( ld ) ) < 0 )
      {
         perror( "Init_socket: SO_DONTLINGER" );
         close( fd );
         exit( 1 );
      }
   }
#endif

   memset( &sa, '\0', sizeof( sa ) );
   sa.sin_family = AF_INET;
   sa.sin_port = htons( mudport );

   if( bind( fd, ( struct sockaddr * )&sa, sizeof( sa ) ) == -1 )
   {
      perror( "Init_socket: bind" );
      close( fd );
      exit( 1 );
   }

   if( listen( fd, 50 ) < 0 )
   {
      perror( "Init_socket: listen" );
      close( fd );
      exit( 1 );
   }

   return fd;
}

/*
static void SegVio()
{
  CHAR_DATA *ch;

  log_string( "SEGMENTATION VIOLATION" );
  log_string( lastplayercmd );
  for ( ch = first_char; ch; ch = ch->next )
  {
    log_printf( "%cPC: %-20s room: %d", IS_NPC(ch) ? 'N' : ' ', ch->name, ch->in_room->vnum );
  }
  exit(0);
}
*/

int child_exit_status = 0;

static void clean_up_child_process( int signal_number )
{
   /* Clean up the child process. */
   int status;

   wait( &status );

   /* Store its exit status in a global variable. */
   child_exit_status = status;
}

/*
 * LAG alarm!							-Thoric
 */
void caught_alarm( int signum )
{
   char buf[MAX_STRING_LENGTH];

   bug( "ALARM CLOCK!  In section %s", alarm_section );
   mudstrlcpy( buf, "Alas, the hideous malevalent entity known only as 'Lag' rises once more!\r\n", MAX_STRING_LENGTH );
   echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
   if( newdesc )
   {
      FD_CLR( newdesc, &in_set );
      FD_CLR( newdesc, &out_set );
      FD_CLR( newdesc, &exc_set );
      log_string( "clearing newdesc" );
   }
}

bool check_bad_desc( int desc )
{
   if( FD_ISSET( desc, &exc_set ) )
   {
      FD_CLR( desc, &in_set );
      FD_CLR( desc, &out_set );
      log_string( "Bad FD caught and disposed." );
      return TRUE;
   }
   return FALSE;
}

/*
 * Determine whether this player is to be watched  --Gorog
 */
bool chk_watch( short player_level, const char *player_name, const char *player_site )
{
   WATCH_DATA *pw;
/*
    log_printf( "chk_watch entry: plev=%d pname=%s psite=%s", player_level, player_name, player_site);
*/
   if( !first_watch )
      return FALSE;

   for( pw = first_watch; pw; pw = pw->next )
   {
      if( pw->target_name )
      {
         if( !str_cmp( pw->target_name, player_name ) && player_level < pw->imm_level )
            return TRUE;
      }
      else if( pw->player_site )
      {
         if( !str_prefix( pw->player_site, player_site ) && player_level < pw->imm_level )
            return TRUE;
      }
   }
   return FALSE;
}

void accept_new( int ctrl )
{
   static struct timeval null_time;
   DESCRIPTOR_DATA *d;

   /*
    * Poll all active descriptors.
    */
   FD_ZERO( &in_set );
   FD_ZERO( &out_set );
   FD_ZERO( &exc_set );
   FD_SET( ctrl, &in_set );
   maxdesc = ctrl;
   newdesc = 0;
   for( d = first_descriptor; d; d = d->next )
   {
      maxdesc = UMAX( maxdesc, d->descriptor );
      FD_SET( d->descriptor, &in_set );
      FD_SET( d->descriptor, &out_set );
      FD_SET( d->descriptor, &exc_set );
      if( d->ifd != -1 && d->ipid != -1 )
      {
         maxdesc = UMAX( maxdesc, d->ifd );
         FD_SET( d->ifd, &in_set );
      }
      if( d == last_descriptor )
         break;
   }

   if( select( maxdesc + 1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
   {
      perror( "accept_new: select: poll" );
      exit( 1 );
   }

   if( FD_ISSET( ctrl, &exc_set ) )
   {
      bug( "Exception raise on controlling descriptor %d", ctrl );
      FD_CLR( ctrl, &in_set );
      FD_CLR( ctrl, &out_set );
   }
   else if( FD_ISSET( ctrl, &in_set ) )
   {
      newdesc = ctrl;
      new_descriptor( newdesc );
   }
}

void game_loop( void )
{
   struct timeval last_time;
   char cmdline[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;

#ifndef WIN32
   signal( SIGPIPE, SIG_IGN );
   signal( SIGALRM, caught_alarm );
   signal( SIGCHLD, clean_up_child_process );
#endif

   /*
    * signal( SIGSEGV, SegVio ); 
    */
   gettimeofday( &last_time, NULL );
   current_time = ( time_t ) last_time.tv_sec;

   /*
    * Main loop 
    */
   while( !mud_down )
   {
      accept_new( control );

      /*
       * Kick out descriptors with raised exceptions
       * or have been idle, then check for input.
       */
      for( d = first_descriptor; d; d = d_next )
      {
         if( d == d->next )
         {
            bug( "descriptor_loop: loop found & fixed" );
            d->next = NULL;
         }
         d_next = d->next;

         d->idle++;  /* make it so a descriptor can idle out */
         if( FD_ISSET( d->descriptor, &exc_set ) )
         {
            FD_CLR( d->descriptor, &in_set );
            FD_CLR( d->descriptor, &out_set );
            if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
               save_char_obj( d->character );
            d->outtop = 0;
            close_socket( d, TRUE );
            continue;
         }
         else if( ( !d->character && d->idle > 360 )  /* 2 mins */
                  || ( d->connected != CON_PLAYING && d->idle > 1200 )  /* 5 mins */
                  || d->idle > 28800 ) /* 2 hrs  */
         {
            write_to_descriptor( d, "Idle timeout... disconnecting.\r\n", 0 );
            d->outtop = 0;
            close_socket( d, TRUE );
            continue;
         }
         else
         {
            d->fcommand = FALSE;

            if( FD_ISSET( d->descriptor, &in_set ) )
            {
               d->idle = 0;
               if( d->character )
                  d->character->timer = 0;
               if( !read_from_descriptor( d ) )
               {
                  FD_CLR( d->descriptor, &out_set );
                  if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
                     save_char_obj( d->character );
                  d->outtop = 0;
                  close_socket( d, FALSE );
                  continue;
               }
            }

            /*
             * check for input from the dns 
             */
            if( ( d->connected == CON_PLAYING || d->character != NULL ) && d->ifd != -1 && FD_ISSET( d->ifd, &in_set ) )
               process_dns( d );

            if( d->character && d->character->wait > 0 )
            {
               --d->character->wait;
               continue;
            }

            read_from_buffer( d );
            if( d->incomm[0] != '\0' )
            {
               d->fcommand = TRUE;
               stop_idling( d->character );

               mudstrlcpy( cmdline, d->incomm, MAX_INPUT_LENGTH );
               d->incomm[0] = '\0';

               if( d->character )
                  set_cur_char( d->character );

               if( d->pagepoint )
                  set_pager_input( d, cmdline );
               else
                  switch ( d->connected )
                  {
                     default:
                        nanny( d, cmdline );
                        break;
                     case CON_PLAYING:
                        interpret( d->character, cmdline );
                        break;
                     case CON_EDITING:
                        edit_buffer( d->character, cmdline );
                        break;
                  }
            }
         }
         if( d == last_descriptor )
            break;
      }

#ifdef IMC
      imc_loop(  );
#endif

      /*
       * Autonomous game motion.
       */
      update_handler(  );

      /*
       * Output.
       */
      for( d = first_descriptor; d; d = d_next )
      {
         d_next = d->next;

         if( ( d->fcommand || d->outtop > 0 ) && FD_ISSET( d->descriptor, &out_set ) )
         {
            if( d->pagepoint )
            {
               if( !pager_output( d ) )
               {
                  if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
                     save_char_obj( d->character );
                  d->outtop = 0;
                  close_socket( d, FALSE );
               }
            }
            else if( !flush_buffer( d, TRUE ) )
            {
               if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
                  save_char_obj( d->character );
               d->outtop = 0;
               close_socket( d, FALSE );
            }
         }
         if( d == last_descriptor )
            break;
      }

      /*
       * Synchronize to a clock.
       * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
       * Careful here of signed versus unsigned arithmetic.
       */
      {
         struct timeval now_time;
         long secDelta;
         long usecDelta;

         gettimeofday( &now_time, NULL );
         usecDelta = ( ( int )last_time.tv_usec ) - ( ( int )now_time.tv_usec ) + 1000000 / PULSE_PER_SECOND;
         secDelta = ( ( int )last_time.tv_sec ) - ( ( int )now_time.tv_sec );
         while( usecDelta < 0 )
         {
            usecDelta += 1000000;
            secDelta -= 1;
         }

         while( usecDelta >= 1000000 )
         {
            usecDelta -= 1000000;
            secDelta += 1;
         }

         if( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
         {
            struct timeval stall_time;

            stall_time.tv_usec = usecDelta;
            stall_time.tv_sec = secDelta;
#ifdef WIN32
            Sleep( ( stall_time.tv_sec * 1000L ) + ( stall_time.tv_usec / 1000L ) );
#else
            if( select( 0, NULL, NULL, NULL, &stall_time ) < 0 && errno != EINTR )
            {
               perror( "game_loop: select: stall" );
               exit( 1 );
            }
#endif
         }
      }

      gettimeofday( &last_time, NULL );
      current_time = ( time_t ) last_time.tv_sec;
   }
   /*
    * Save morphs so can sort later. --Shaddai 
    */
   if( sysdata.morph_opt )
      save_morphs(  );

   fflush( stderr ); /* make sure strerr is flushed */
   return;
}

void new_descriptor( int new_desc )
{
   char buf[MAX_STRING_LENGTH];
   char log_buf[MAX_STRING_LENGTH];
   DESCRIPTOR_DATA *dnew;
   struct sockaddr_in sock;
   int desc;
#ifndef WIN32
   socklen_t size;
#else
   unsigned int size;
   unsigned long arg = 1;
#endif

   size = sizeof( sock );
   if( check_bad_desc( new_desc ) )
   {
      set_alarm( 0 );
      return;
   }
   set_alarm( 20 );
   alarm_section = "new_descriptor::accept";
   if( ( desc = accept( new_desc, ( struct sockaddr * )&sock, &size ) ) < 0 )
   {
      perror( "New_descriptor: accept" );
      log_printf_plus( LOG_COMM, sysdata.log_level, "%s", "[*****] BUG: New_descriptor: accept" );
      set_alarm( 0 );
      return;
   }
   if( check_bad_desc( new_desc ) )
   {
      set_alarm( 0 );
      return;
   }
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

   set_alarm( 20 );
   alarm_section = "new_descriptor: after accept";

#ifdef WIN32
   if( ioctlsocket( desc, FIONBIO, &arg ) == -1 )
#else
   if( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
#endif
   {
      perror( "New_descriptor: fcntl: FNDELAY" );
      set_alarm( 0 );
      return;
   }
   if( check_bad_desc( new_desc ) )
      return;

   CREATE( dnew, DESCRIPTOR_DATA, 1 );
   dnew->next = NULL;
   dnew->descriptor = desc;
   dnew->connected = CON_GET_NAME;
   dnew->outsize = 2000;
   dnew->idle = 0;
   dnew->lines = 0;
   dnew->scrlen = 24;
   dnew->port = ntohs( sock.sin_port );
   dnew->newstate = 0;
   dnew->prevcolor = 0x07;
   dnew->ifd = -1;   /* Descriptor pipes, used for DNS resolution and such */
   dnew->ipid = -1;
   dnew->can_compress = FALSE;
   CREATE( dnew->mccp, MCCP, 1 );

   CREATE( dnew->outbuf, char, dnew->outsize );

   mudstrlcpy( log_buf, inet_ntoa( sock.sin_addr ), MAX_STRING_LENGTH );
   dnew->host = STRALLOC( log_buf );
   if( !sysdata.NO_NAME_RESOLVING )
   {
      mudstrlcpy( buf, in_dns_cache( log_buf ), MAX_STRING_LENGTH );

      if( buf[0] == '\0' )
         resolve_dns( dnew, sock.sin_addr.s_addr );
      else
      {
         STRFREE( dnew->host );
         dnew->host = STRALLOC( buf );
      }
   }

   if( check_total_bans( dnew ) )
   {
      write_to_descriptor( dnew, "Your site has been banned from this Mud.\r\n", 0 );
      free_desc( dnew );
      set_alarm( 0 );
      return;
   }

   /*
    * Init descriptor data.
    */

   if( !last_descriptor && first_descriptor )
   {
      DESCRIPTOR_DATA *d;

      bug( "New_descriptor: last_desc is NULL, but first_desc is not! ...fixing" );
      for( d = first_descriptor; d; d = d->next )
         if( !d->next )
            last_descriptor = d;
   }

   LINK( dnew, first_descriptor, last_descriptor, next, prev );

   /*
    * MCCP Compression 
    */
   write_to_buffer( dnew, (const char *)will_compress2_str, 0 );

   /*
    * Send the greeting.
    */
   {
      extern char *help_greeting;
      if( help_greeting[0] == '.' )
         send_to_desc_color( help_greeting + 1, dnew );
      else
         send_to_desc_color( help_greeting, dnew );
   }

   if( ++num_descriptors > sysdata.maxplayers )
      sysdata.maxplayers = num_descriptors;
   if( sysdata.maxplayers > sysdata.alltimemax )
   {
      if( sysdata.time_of_max )
         DISPOSE( sysdata.time_of_max );
      snprintf( buf, MAX_STRING_LENGTH, "%24.24s", ctime( &current_time ) );
      sysdata.time_of_max = str_dup( buf );
      sysdata.alltimemax = sysdata.maxplayers;
      snprintf( log_buf, MAX_STRING_LENGTH, "Broke all-time maximum player record: %d", sysdata.alltimemax );
      log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
      to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
      save_sysdata( sysdata );
   }
   set_alarm( 0 );
   return;
}

void free_desc( DESCRIPTOR_DATA * d )
{
   compressEnd( d );
   close( d->descriptor );
   STRFREE( d->host );
   DISPOSE( d->outbuf );
   if( d->pagebuf )
      DISPOSE( d->pagebuf );
   DISPOSE( d->mccp );
   DISPOSE( d );
   return;
}

void close_socket( DESCRIPTOR_DATA * dclose, bool force )
{
   CHAR_DATA *ch;
   DESCRIPTOR_DATA *d;
   bool DoNotUnlink = FALSE;

   if( dclose->ipid != -1 )
   {
      int status;

      kill( dclose->ipid, SIGKILL );
      waitpid( dclose->ipid, &status, 0 );
   }
   if( dclose->ifd != -1 )
      close( dclose->ifd );

   /*
    * flush outbuf 
    */
   if( !force && dclose->outtop > 0 )
      flush_buffer( dclose, FALSE );

   /*
    * say bye to whoever's snooping this descriptor 
    */
   if( dclose->snoop_by )
      write_to_buffer( dclose->snoop_by, "Your victim has left the game.\r\n", 0 );

   /*
    * stop snooping everyone else 
    */
   for( d = first_descriptor; d; d = d->next )
      if( d->snoop_by == dclose )
         d->snoop_by = NULL;

   /*
    * Check for switched people who go link-dead. -- Altrag 
    */
   if( dclose->original )
   {
      if( ( ch = dclose->character ) != NULL )
         do_return( ch, "" );
      else
      {
         bug( "Close_socket: dclose->original without character %s",
              ( dclose->original->name ? dclose->original->name : "unknown" ) );
         dclose->character = dclose->original;
         dclose->original = NULL;
      }
   }

   ch = dclose->character;

   /*
    * sanity check :( 
    */
   if( !dclose->prev && dclose != first_descriptor )
   {
      DESCRIPTOR_DATA *dp, *dn;
      bug( "Close_socket: %s desc:%p != first_desc:%p and desc->prev = NULL!",
           ch ? ch->name : d->host, ( void * )dclose, ( void * )first_descriptor );
      dp = NULL;
      for( d = first_descriptor; d; d = dn )
      {
         dn = d->next;
         if( d == dclose )
         {
            bug( "%s: %s desc:%p found, prev should be:%p, fixing.", __func__, ch ? ch->name : d->host, ( void * )dclose,
                 ( void * )dp );
            dclose->prev = dp;
            break;
         }
         dp = d;
      }
      if( !dclose->prev )
      {
         bug( "%s: %s desc:%p could not be found!.", __func__, ch ? ch->name : dclose->host, ( void * )dclose );
         DoNotUnlink = TRUE;
      }
   }

   if( !dclose->next && dclose != last_descriptor )
   {
      DESCRIPTOR_DATA *dp, *dn;
      bug( "%s: %s desc:%p != last_desc:%p and desc->next = NULL!", __func__,
           ch ? ch->name : d->host, ( void * )dclose, ( void * )last_descriptor );
      dn = NULL;
      for( d = last_descriptor; d; d = dp )
      {
         dp = d->prev;
         if( d == dclose )
         {
            bug( "%s: %s desc:%p found, next should be:%p, fixing.", __func__, ch ? ch->name : d->host, ( void * )dclose,
                 ( void * )dn );
            dclose->next = dn;
            break;
         }
         dn = d;
      }
      if( !dclose->next )
      {
         bug( "%s: %s desc:%p could not be found!.", __func__, ch ? ch->name : dclose->host, ( void * )dclose );
         DoNotUnlink = TRUE;
      }
   }

   if( dclose->character )
   {
      log_printf_plus( LOG_COMM, UMAX( sysdata.log_level, ch->level ), "Closing link to %s. (INRoom %d)", ch->pcdata->filename, ( ch->in_room ? ch->in_room->vnum : -1 ) );

      if( dclose->connected == CON_EDITING )
      {
         if( ch->last_cmd )
            ch->last_cmd( ch, "" );
         else
            stop_editing( ch );
         dclose->connected = CON_PLAYING;
      }

      if( dclose->connected == CON_PLAYING || dclose->connected == CON_EDITING || dclose->connected == CON_DELETE )
      {
         act( AT_ACTION, "$n has lost $s link.", ch, NULL, NULL, TO_CANSEE );
         ch->desc = NULL;
      }
      else
      {
         /*
          * clear descriptor pointer to get rid of bug message in log 
          */
         dclose->character->desc = NULL;
         free_char( dclose->character );
      }
   }

   if( !DoNotUnlink )
   {
      /*
       * make sure loop doesn't get messed up 
       */
      if( d_next == dclose )
         d_next = d_next->next;
      UNLINK( dclose, first_descriptor, last_descriptor, next, prev );
   }

   compressEnd( dclose );

   if( dclose->descriptor == maxdesc )
      --maxdesc;

   free_desc( dclose );
   --num_descriptors;
   return;
}

bool read_from_descriptor( DESCRIPTOR_DATA * d )
{
   unsigned int iStart;
   int iErr;

   /*
    * Hold horses if pending command already. 
    */
   if( d->incomm[0] != '\0' )
      return TRUE;

   /*
    * Check for overflow. 
    */
   iStart = strlen( d->inbuf );
   if( iStart >= sizeof( d->inbuf ) - 10 )
   {
      log_printf( "%s input overflow!", d->host );
      write_to_descriptor( d, "\r\n*** PUT A LID ON IT!!! ***\r\nYou cannot enter the same command more than 20 consecutive times!\r\n", 0 );
      return FALSE;
   }

   for( ;; )
   {
      int nRead;

      nRead = recv( d->descriptor, d->inbuf + iStart, sizeof( d->inbuf ) - 10 - iStart, 0 );
#ifdef WIN32
      iErr = WSAGetLastError(  );
#else
      iErr = errno;
#endif
      if( nRead > 0 )
      {
         iStart += nRead;
         if( d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r' )
            break;
      }
      else if( nRead == 0 )
      {
         log_string_plus( "EOF encountered on read.", LOG_COMM, sysdata.log_level );
         return FALSE;
      }
      else if( iErr == EWOULDBLOCK )
         break;
      else
      {
         perror( "Read_from_descriptor" );
         return FALSE;
      }
   }

   d->inbuf[iStart] = '\0';
   return TRUE;
}

/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA * d )
{
   int i, j, k, iac = 0;

   /*
    * Hold horses if pending command already.
    */
   if( d->incomm[0] != '\0' )
      return;

   /*
    * Look for at least one new line.
    */
   for( i = 0; i < MAX_INBUF_SIZE && d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
   {
      if( d->inbuf[i] == '\0' )
         return;
   }

   /*
    * Canonical input processing.
    */
   for( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
   {
      if( k >= 254 )
      {
         write_to_descriptor( d, "Line too long.\r\n", 0 );

         /*
          * skip the rest of the line 
          */
         /*
          * for ( ; d->inbuf[i] != '\0' || i>= MAX_INBUF_SIZE ; i++ )
          * {
          * if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
          * break;
          * }
          */
         d->inbuf[i] = '\n';
         d->inbuf[i + 1] = '\0';
         break;
      }

      if( d->inbuf[i] == ( signed char )IAC )
         iac = 1;
      else if( iac == 1
               && ( d->inbuf[i] == ( signed char )DO || d->inbuf[i] == ( signed char )DONT
                    || d->inbuf[i] == ( signed char )WILL ) )
         iac = 2;
      else if( iac == 2 )
      {
         iac = 0;
         if( d->inbuf[i] == ( signed char )TELOPT_COMPRESS2 )
         {
            if( d->inbuf[i - 1] == ( signed char )DO )
               compressStart( d );
            else if( d->inbuf[i - 1] == ( signed char )DONT )
               compressEnd( d );
         }
      }
      else if( d->inbuf[i] == '\b' && k > 0 )
         --k;
      else if( isascii( d->inbuf[i] ) && isprint( d->inbuf[i] ) )
         d->incomm[k++] = d->inbuf[i];
   }

   /*
    * Finish off the line.
    */
   if( k == 0 )
      d->incomm[k++] = ' ';
   d->incomm[k] = '\0';

   /*
    * Deal with bozos with #repeat 1000 ...
    */
   if( k > 1 || d->incomm[0] == '!' )
   {
      if( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
      {
         d->repeat = 0;
      }
      else
      {
         if( ++d->repeat >= 20 )
         {
/*		log_printf( "%s input spamming!", d->host );
*/
            write_to_descriptor( d, "\r\n*** PUT A LID ON IT!!! ***\r\nYou cannot enter the same command more than 20 consecutive times!\r\n", 0 );
            mudstrlcpy( d->incomm, "quit", MAX_INPUT_LENGTH );
         }
      }
   }

   /*
    * Do '!' substitution.
    */
   if( d->incomm[0] == '!' )
      mudstrlcpy( d->incomm, d->inlast, MAX_INPUT_LENGTH );
   else
      mudstrlcpy( d->inlast, d->incomm, MAX_INPUT_LENGTH );

   /*
    * Shift the input buffer.
    */
   while( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
      i++;
   for( j = 0; ( d->inbuf[j] = d->inbuf[i + j] ) != '\0'; j++ )
      ;
   return;
}

/*
 * Low level output function.
 */
bool flush_buffer( DESCRIPTOR_DATA * d, bool fPrompt )
{
   char buf[MAX_INPUT_LENGTH];

   /*
    * If buffer has more than 4K inside, spit out .5K at a time   -Thoric
    */
   if( !mud_down && d->outtop > 4096 )
   {
      memcpy( buf, d->outbuf, 512 );
      d->outtop -= 512;
      memmove( d->outbuf, d->outbuf + 512, d->outtop );

      if( d->snoop_by )
      {
         char snoopbuf[MAX_INPUT_LENGTH];

         buf[512] = '\0';
         if( d->character && d->character->name )
         {
            if( d->original && d->original->name )
               snprintf( snoopbuf, MAX_INPUT_LENGTH, "%s (%s)", d->character->name, d->original->name );
            else
               snprintf( snoopbuf, MAX_INPUT_LENGTH, "%s", d->character->name );
            write_to_buffer( d->snoop_by, snoopbuf, 0 );
         }
         write_to_buffer( d->snoop_by, "% ", 2 );
         write_to_buffer( d->snoop_by, buf, 0 );
      }

      if( !write_to_descriptor( d, buf, 512 ) )
      {
         d->outtop = 0;
         return FALSE;
      }
      return TRUE;
   }

   /*
    * Bust a prompt.
    */
   if( fPrompt && !mud_down && d->connected == CON_PLAYING )
   {
      CHAR_DATA *ch;

      ch = d->original ? d->original : d->character;
      if( xIS_SET( ch->act, PLR_BLANK ) )
         write_to_buffer( d, "\r\n", 2 );

      if( xIS_SET( ch->act, PLR_PROMPT ) )
         display_prompt( d );

      if( ch && !IS_NPC( ch ) && xIS_SET( ch->act, PLR_ANSI ) )
      {
         write_to_buffer( d, ANSI_RESET, 0 );
         d->prevcolor = 0x08;
      }

      if( xIS_SET( ch->act, PLR_TELNET_GA ) )
         write_to_buffer( d, (const char *)go_ahead_str, 0 );
   }

   /*
    * Short-circuit if nothing to write.
    */
   if( d->outtop == 0 )
      return TRUE;

   /*
    * Snoop-o-rama.
    */
   if( d->snoop_by )
   {
      /*
       * without check, 'force mortal quit' while snooped caused crash, -h 
       */
      if( d->character && d->character->name )
      {
         /*
          * Show original snooped names. -- Altrag 
          */
         if( d->original && d->original->name )
            snprintf( buf, MAX_INPUT_LENGTH, "%s (%s)", d->character->name, d->original->name );
         else
            snprintf( buf, MAX_INPUT_LENGTH, "%s", d->character->name );
         write_to_buffer( d->snoop_by, buf, 0 );
      }
      write_to_buffer( d->snoop_by, "% ", 2 );
      write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
   }

   /*
    * OS-dependent output.
    */
   if( !write_to_descriptor( d, d->outbuf, d->outtop ) )
   {
      d->outtop = 0;
      return FALSE;
   }
   else
   {
      d->outtop = 0;
      return TRUE;
   }
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA * d, const char *txt, size_t length )
{
   if( !d )
   {
      bug( "%s: NULL descriptor", __func__ );
      return;
   }

   if( MPSilent )
      return;

   /*
    * Normally a bug... but can happen if loadup is used.
    */
   if( !d->outbuf )
      return;

   /*
    * Find length in case caller didn't.
    */
   if( length <= 0 )
      length = strlen( txt );

/* Uncomment if debugging or something
    if ( length != strlen(txt) )
    {
	bug( "Write_to_buffer: length(%d) != strlen(txt)!", length );
	length = strlen(txt);
    }
*/

   /*
    * Initial \r\n if needed.
    */
   if( d->outtop == 0 && !d->fcommand )
   {
      d->outbuf[0] = '\r';
      d->outbuf[1] = '\n';
      d->outtop = 2;
   }

   /*
    * Expand the buffer as needed.
    */
   while( d->outtop + length >= d->outsize )
   {
      if( d->outsize > 32000 )
      {
         /*
          * empty buffer 
          */
         d->outtop = 0;
         /*
          * Bugfix by Samson - moved bug() call up 
          */
         bug( "Buffer overflow. Closing (%s).", d->character ? d->character->name : "???" );
         close_socket( d, TRUE );
         return;
      }
      d->outsize *= 2;
      RECREATE( d->outbuf, char, d->outsize );
   }

   /*
    * Copy.
    */
   strncpy( d->outbuf + d->outtop, txt, length );
   d->outtop += length;
   d->outbuf[d->outtop] = '\0';
   return;
}

/*
* This is the MCCP version. Use write_to_descriptor_old to send non-compressed text.
* Updated to run with the block checks by Orion... if it doesn't work, blame
* him.;P -Orion
*/
bool write_to_descriptor( DESCRIPTOR_DATA * d, const char *txt, int length )
{
   int iStart = 0;
   int nWrite = 0;
   int nBlock;
   int iErr;
   int len;

   if( length <= 0 )
      length = strlen( txt );

   if( d && d->mccp->out_compress )
   {
      d->mccp->out_compress->next_in = ( unsigned char * )txt;
      d->mccp->out_compress->avail_in = length;

      while( d->mccp->out_compress->avail_in )
      {
         d->mccp->out_compress->avail_out =
            COMPRESS_BUF_SIZE - ( d->mccp->out_compress->next_out - d->mccp->out_compress_buf );

         if( d->mccp->out_compress->avail_out )
         {
            int status = deflate( d->mccp->out_compress, Z_SYNC_FLUSH );

            if( status != Z_OK )
               return FALSE;
         }

         len = d->mccp->out_compress->next_out - d->mccp->out_compress_buf;
         if( len > 0 )
         {
            for( iStart = 0; iStart < len; iStart += nWrite )
            {
               nBlock = UMIN( len - iStart, 4096 );
               nWrite = send( d->descriptor, d->mccp->out_compress_buf + iStart, nBlock, 0 );
               if( nWrite == -1 )
               {
                  iErr = errno;
                  if( iErr == EWOULDBLOCK )
                  {
                     /*
                      * This is a SPAMMY little bug error. I would suggest
                      * not using it, but I've included it in case. -Orion
                      *
                      perror( "Write_to_descriptor: Send is blocking" );
                      */
                     nWrite = 0;
                     continue;
                  }
                  else
                  {
                     perror( "Write_to_descriptor" );
                     return FALSE;
                  }
               }

               if( !nWrite )
                  break;
            }

            if( !iStart )
               break;

            if( iStart < len )
               memmove( d->mccp->out_compress_buf, d->mccp->out_compress_buf + iStart, len - iStart );

            d->mccp->out_compress->next_out = d->mccp->out_compress_buf + len - iStart;
         }
      }
      return TRUE;
   }

   for( iStart = 0; iStart < length; iStart += nWrite )
   {
      nBlock = UMIN( length - iStart, 4096 );
      nWrite = send( d->descriptor, txt + iStart, nBlock, 0 );
      if( nWrite == -1 )
      {
         iErr = errno;
         if( iErr == EWOULDBLOCK )
         {
            /*
             * This is a SPAMMY little bug error. I would suggest
             * not using it, but I've included it in case. -Orion
             *
             perror( "Write_to_descriptor: Send is blocking" );
             */
            nWrite = 0;
            continue;
         }
         else
         {
            perror( "Write_to_descriptor" );
            return FALSE;
         }
      }
   }
   return TRUE;
}

/*
 *
 * Added block checking to prevent random booting of the descriptor. Thanks go
 * out to Rustry for his suggestions. -Orion
 */
bool write_to_descriptor_old( int desc, const char *txt, int length )
{
   int iStart = 0;
   int nWrite = 0;
   int nBlock = 0;
   int iErr = 0;

   if( length <= 0 )
      length = strlen( txt );

   for( iStart = 0; iStart < length; iStart += nWrite )
   {
      nBlock = UMIN( length - iStart, 4096 );
      nWrite = send( desc, txt + iStart, nBlock, 0 );

      if( nWrite == -1 )
      {
         iErr = errno;
         if( iErr == EWOULDBLOCK )
         {
            /*
             * This is a SPAMMY little bug error. I would suggest
             * not using it, but I've included it in case. -Orion
             *
             perror( "Write_to_descriptor: Send is blocking" );
             */
            nWrite = 0;
            continue;
         }
         else
         {
            perror( "Write_to_descriptor" );
            return FALSE;
         }
      }
   }
   return TRUE;
}

void show_title( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *ch;

   ch = d->character;

   if( !IS_SET( ch->pcdata->flags, PCFLAG_NOINTRO ) )
   {
      if( xIS_SET( ch->act, PLR_RIP ) )
         send_rip_title( ch );
      else if( xIS_SET( ch->act, PLR_ANSI ) )
         send_ansi_title( ch );
      else
         send_ascii_title( ch );
   }
   else
   {
      write_to_buffer( d, "Press enter...\r\n", 0 );
   }
   d->connected = CON_PRESS_ENTER;
}

void nanny_get_name( DESCRIPTOR_DATA * d, char *argument )
{
   CHAR_DATA *ch;
   bool fOld;
   short chk;
   char buf[MAX_STRING_LENGTH];

   ch = d->character;

   if( argument[0] == '\0' )
   {
      close_socket( d, FALSE );
      return;
   }

   argument[0] = UPPER( argument[0] );

   if( !str_cmp( argument, "MSSP-REQUEST" ) )
   {
      send_mssp_data( d );
      //Uncomment below if you want to know when an MSSP request occurs
      //log_printf( "IP: %s requested MSSP data!", d->host );
      close_socket( d, FALSE );
      return;
   }

   /*
    * Old players can keep their characters. -- Alty
    */
   if( !check_parse_name( argument, ( d->newstate != 0 ) ) )
   {
      write_to_buffer( d, "Illegal name, try another.\r\nName: ", 0 );
      return;
   }

   if( !str_cmp( argument, "New" ) )
   {
      if( d->newstate == 0 )
      {
         /*
          * New player
          */
         /*
          * Don't allow new players if DENY_NEW_PLAYERS is true
          */
         if( sysdata.DENY_NEW_PLAYERS == TRUE )
         {
            write_to_buffer( d, "The mud is currently preparing for a reboot.\r\n", 0 );
            write_to_buffer( d, "New players are not accepted during this time.\r\n", 0 );
            write_to_buffer( d, "Please try again in a few minutes.\r\n", 0 );
            close_socket( d, FALSE );
         }
         write_to_buffer( d, "\r\nChoosing a name is one of the most important parts of this game...\r\n"
                          "Make sure to pick a name appropriate to the character you are going\r\n"
                          "to role play, and be sure that it suits a medieval theme.\r\n"
                          "If the name you select is not acceptable, you will be asked to choose\r\n"
                          "another one.\r\n\r\nPlease choose a name for your character: ", 0 );
         d->newstate++;
         d->connected = CON_GET_NAME;
         return;
      }
      else
      {
         write_to_buffer( d, "Illegal name, try another.\r\nName: ", 0 );
         return;
      }
   }

   if( check_playing( d, argument, FALSE ) == BERR )
   {
      write_to_buffer( d, "Name: ", 0 );
      return;
   }

   fOld = load_char_obj( d, argument, TRUE, FALSE );
   if( !d->character )
   {
      char cbuf[MAX_STRING_LENGTH];

      log_printf( "Bad player file %s@%s.", argument, d->host );
      snprintf( cbuf, MAX_STRING_LENGTH, "Your playerfile is corrupt... Please notify %s\r\n", sysdata.admin_email );
      write_to_buffer( d, cbuf, 0 );
      close_socket( d, FALSE );
      return;
   }

   ch = d->character;
   if( check_bans( ch, BAN_SITE ) )
   {
      write_to_buffer( d, "Your site has been banned from this Mud.\r\n", 0 );
      close_socket( d, FALSE );
      return;
   }

   if( fOld )
   {
      if( check_bans( ch, BAN_CLASS ) )
      {
         write_to_buffer( d, "Your class has been banned from this Mud.\r\n", 0 );
         close_socket( d, FALSE );
         return;
      }
      if( check_bans( ch, BAN_RACE ) )
      {
         write_to_buffer( d, "Your race has been banned from this Mud.\r\n", 0 );
         close_socket( d, FALSE );
         return;
      }
   }

   if( xIS_SET( ch->act, PLR_DENY ) )
   {
      log_printf_plus( LOG_COMM, sysdata.log_level, "Denying access to %s@%s.", argument, d->host );
      if( d->newstate != 0 )
      {
         write_to_buffer( d, "That name is already taken.  Please choose another: ", 0 );
         d->connected = CON_GET_NAME;
         d->character->desc = NULL;
         free_char( d->character ); /* Big Memory Leak before --Shaddai */
         d->character = NULL;
         return;
      }
      write_to_buffer( d, "You are denied access.\r\n", 0 );
      close_socket( d, FALSE );
      return;
   }

   /*
    *  Make sure the immortal host is from the correct place.
    *  Shaddai
    */
   if( IS_IMMORTAL( ch ) && sysdata.check_imm_host && !check_immortal_domain( ch, d->host ) )
   {
      log_printf_plus( LOG_COMM, sysdata.log_level, "%s's char being hacked from %s.", argument, d->host );
      write_to_buffer( d, "This hacking attempt has been logged.\r\n", 0 );
      close_socket( d, FALSE );
      return;
   }

   chk = check_reconnect( d, argument, FALSE );
   if( chk == BERR )
      return;

   if( chk )
   {
      fOld = TRUE;
   }
   else
   {
      if( sysdata.wizlock && !IS_IMMORTAL( ch ) )
      {
         write_to_buffer( d, "The game is wizlocked. Only immortals can connect now.\r\n", 0 );
         write_to_buffer( d, "Please try back later.\r\n", 0 );
         close_socket( d, FALSE );
         return;
      }
   }

   if( fOld )
   {
      if( d->newstate != 0 )
      {
         write_to_buffer( d, "That name is already taken. Please choose another: ", 0 );
         d->connected = CON_GET_NAME;
         d->character->desc = NULL;
         free_char( d->character ); /* Big Memory Leak before --Shaddai */
         d->character = NULL;
         return;
      }

      /*
       * Old player
       */
      write_to_buffer( d, "Password: ", 0 );
      write_to_buffer( d, (const char *)echo_off_str, 0 );
      d->connected = CON_GET_OLD_PASSWORD;
      return;
   }
   else
   {
      /*
       * if ( !check_parse_name( argument ) )
       * {
       * write_to_buffer( d, "hat name is reserved, please try another.\r\nName: ", 0 );
       * return;
       * }
       */
      if( d->newstate == 0 )
      {
         /*
          * No such player
          */
         write_to_buffer( d, "\r\nNo such player exists.\r\nPlease check your spelling, or type new to start a new player.\r\n\r\nName: ", 0 );
         d->connected = CON_GET_NAME;
         d->character->desc = NULL;
         free_char( d->character ); /* Big Memory Leak before --Shaddai */
         d->character = NULL;
         return;
      }

      snprintf( buf, MAX_STRING_LENGTH, "Did I get that right, %s (Y/N)? ", argument );
      write_to_buffer( d, buf, 0 );
      d->connected = CON_CONFIRM_NEW_NAME;
      return;
   }
}

void nanny_get_old_password( DESCRIPTOR_DATA * d, char *argument )
{
   CHAR_DATA *ch;
   char buf[MAX_STRING_LENGTH];
   bool fOld;
   short chk;

   ch = d->character;
   write_to_buffer( d, "\r\n", 2 );

   if( str_cmp( sha256_crypt( argument ), ch->pcdata->pwd ) )
   {
      write_to_buffer( d, "Wrong password, disconnecting.\r\n", 0 );
      /*
       * clear descriptor pointer to get rid of bug message in log
       */
      d->character->desc = NULL;
      close_socket( d, FALSE );
      return;
   }

   write_to_buffer( d, (const char *)echo_on_str, 0 );

   if( check_playing( d, ch->pcdata->filename, TRUE ) )
      return;

   chk = check_reconnect( d, ch->pcdata->filename, TRUE );
   if( chk == BERR )
   {
      if( d->character && d->character->desc )
         d->character->desc = NULL;
      close_socket( d, FALSE );
      return;
   }
   if( chk == TRUE )
      return;

   mudstrlcpy( buf, ch->pcdata->filename, MAX_STRING_LENGTH );
   d->character->desc = NULL;
   free_char( d->character );
   d->character = NULL;
   fOld = load_char_obj( d, buf, FALSE, FALSE );
   if( !fOld )
      bug( "%s: failed to load_char_obj for %s.", __func__, buf );

   if( !d->character )
   {
      char cbuf[MAX_STRING_LENGTH];

      log_printf( "Bad player file %s@%s.", argument, d->host );
      snprintf( cbuf, MAX_STRING_LENGTH, "Your playerfile is corrupt... Please notify %s\r\n", sysdata.admin_email );
      write_to_buffer( d, cbuf, 0 );
      close_socket( d, FALSE );
      return;
   }

   ch = d->character;
   if( ch->position == POS_FIGHTING
       || ch->position == POS_EVASIVE
       || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE || ch->position == POS_BERSERK )
      ch->position = POS_STANDING;

   log_printf_plus( LOG_COMM, UMAX( sysdata.log_level, ch->level ), "%s (%s) has connected.", ch->name, d->host );
   show_title( d );
}

void nanny_confirm_new_name( DESCRIPTOR_DATA * d, char *argument )
{
   CHAR_DATA *ch;
   char buf[MAX_STRING_LENGTH];

   ch = d->character;

   switch ( *argument )
   {
      case 'y':
      case 'Y':
         snprintf( buf, MAX_STRING_LENGTH,
                   "\r\nMake sure to use a password that won't be easily guessed by someone else."
                   "\r\nPick a good password for %s: %s", ch->name, echo_off_str );
         write_to_buffer( d, buf, 0 );
         d->connected = CON_GET_NEW_PASSWORD;
         break;

      case 'n':
      case 'N':
         write_to_buffer( d, "Ok, what IS it, then? ", 0 );
         /*
          * clear descriptor pointer to get rid of bug message in log
          */
         d->character->desc = NULL;
         free_char( d->character );
         d->character = NULL;
         d->connected = CON_GET_NAME;
         break;

      default:
         write_to_buffer( d, "Please type Yes or No. ", 0 );
         break;
   }
}

void nanny_get_new_password( DESCRIPTOR_DATA * d, char *argument )
{
   CHAR_DATA *ch;
   char *pwdnew;

   ch = d->character;
   write_to_buffer( d, "\r\n", 2 );

   if( strlen( argument ) < 5 )
   {
      write_to_buffer( d, "Password must be at least five characters long.\r\nPassword: ", 0 );
      return;
   }

   if( argument[0] == '!' )
   {
      send_to_char( "New password cannot begin with the '!' character.", ch );
      return;
   }
   pwdnew = sha256_crypt( argument );  /* SHA-256 Encryption */
   DISPOSE( ch->pcdata->pwd );
   ch->pcdata->pwd = str_dup( pwdnew );
   write_to_buffer( d, "\r\nPlease retype the password to confirm: ", 0 );
   d->connected = CON_CONFIRM_NEW_PASSWORD;
}

void nanny_confirm_new_password( DESCRIPTOR_DATA * d, char *argument )
{
   CHAR_DATA *ch;

   ch = d->character;
   write_to_buffer( d, "\r\n", 2 );

   if( str_cmp( sha256_crypt( argument ), ch->pcdata->pwd ) )
   {
      write_to_buffer( d, "Passwords don't match.\r\nRetype password: ", 0 );
      d->connected = CON_GET_NEW_PASSWORD;
      return;
   }

   write_to_buffer( d, (const char *)echo_on_str, 0 );
   write_to_buffer( d, "\r\nWhat is your sex (M/F/N)? ", 0 );
   d->connected = CON_GET_NEW_SEX;
}

void nanny_get_new_sex( DESCRIPTOR_DATA * d, char *argument )
{
   CHAR_DATA *ch;
   char buf[MAX_STRING_LENGTH];
   int iClass;

   ch = d->character;

   switch ( argument[0] )
   {
      case 'm':
      case 'M':
         ch->sex = SEX_MALE;
         break;
      case 'f':
      case 'F':
         ch->sex = SEX_FEMALE;
         break;
      case 'n':
      case 'N':
         ch->sex = SEX_NEUTRAL;
         break;
      default:
         write_to_buffer( d, "That's not a sex.\r\nWhat IS your sex? ", 0 );
         return;
   }

   write_to_buffer( d, "\r\nSelect a class, or type help [class] to learn more about that class.\r\n[", 0 );
   buf[0] = '\0';

   for( iClass = 0; iClass < MAX_PC_CLASS; iClass++ )
   {
      if( class_table[iClass]->who_name && class_table[iClass]->who_name[0] != '\0' )
      {
         if( iClass > 0 )
         {
            if( strlen( buf ) + strlen( class_table[iClass]->who_name ) > 77 )
            {
               mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
               write_to_buffer( d, buf, 0 );
               buf[0] = '\0';
            }
            else
               mudstrlcat( buf, " ", MAX_STRING_LENGTH );
         }
         mudstrlcat( buf, class_table[iClass]->who_name, MAX_STRING_LENGTH );
      }
   }
   mudstrlcat( buf, "]\r\n: ", MAX_STRING_LENGTH );
   write_to_buffer( d, buf, 0 );
   d->connected = CON_GET_NEW_CLASS;
}

void nanny_get_new_class( DESCRIPTOR_DATA * d, const char *argument )
{
   CHAR_DATA *ch;
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_STRING_LENGTH];
   int iClass, iRace;

   ch = d->character;
   argument = one_argument( argument, arg );

   if( !str_cmp( arg, "help" ) )
   {

      for( iClass = 0; iClass < MAX_PC_CLASS; iClass++ )
      {
         if( class_table[iClass]->who_name && class_table[iClass]->who_name[0] != '\0' )
         {
            if( toupper( argument[0] ) == toupper( class_table[iClass]->who_name[0] )
                && !str_prefix( argument, class_table[iClass]->who_name ) )
            {
               do_help( ch, argument );
               write_to_buffer( d, "Please choose a class: ", 0 );
               return;
            }
         }
      }
      write_to_buffer( d, "No such help topic.  Please choose a class: ", 0 );
      return;
   }

   for( iClass = 0; iClass < MAX_PC_CLASS; iClass++ )
   {
      if( class_table[iClass]->who_name && class_table[iClass]->who_name[0] != '\0' )
      {
         if( toupper( arg[0] ) == toupper( class_table[iClass]->who_name[0] )
             && !str_prefix( arg, class_table[iClass]->who_name ) )
         {
            ch->Class = iClass;
            break;
         }
      }
   }

   if( iClass == MAX_PC_CLASS
       || !class_table[iClass]->who_name
       || class_table[iClass]->who_name[0] == '\0' || !str_cmp( class_table[iClass]->who_name, "unused" ) )
   {
      write_to_buffer( d, "That's not a class.\r\nWhat IS your class? ", 0 );
      return;
   }


   if( check_bans( ch, BAN_CLASS ) )
   {
      write_to_buffer( d, "That class is not currently avaiable.\r\nWhat IS your class? ", 0 );
      return;
   }

   write_to_buffer( d, "\r\nYou may choose from the following races, or type help [race] to learn more:\r\n[", 0 );
   buf[0] = '\0';
   for( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
   {
      if( iRace != RACE_VAMPIRE
          && race_table[iRace]->race_name && race_table[iRace]->race_name[0] != '\0'
          && !IS_SET( race_table[iRace]->class_restriction, 1 << ch->Class )
          && str_cmp( race_table[iRace]->race_name, "unused" ) )
      {
         if( iRace > 0 )
         {
            if( strlen( buf ) + strlen( race_table[iRace]->race_name ) > 77 )
            {
               mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
               write_to_buffer( d, buf, 0 );
               buf[0] = '\0';
            }
            else
               mudstrlcat( buf, " ", MAX_STRING_LENGTH );
         }
         mudstrlcat( buf, race_table[iRace]->race_name, MAX_STRING_LENGTH );
      }
   }
   mudstrlcat( buf, "]\r\n: ", MAX_STRING_LENGTH );
   write_to_buffer( d, buf, 0 );
   d->connected = CON_GET_NEW_RACE;
}

void nanny_get_new_race( DESCRIPTOR_DATA * d, const char *argument )
{
   CHAR_DATA *ch;
   char arg[MAX_STRING_LENGTH];
   int iRace;

   ch = d->character;
   argument = one_argument( argument, arg );
   if( !str_cmp( arg, "help" ) )
   {
      for( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
      {
         if( toupper( argument[0] ) == toupper( race_table[iRace]->race_name[0] )
             && !str_prefix( argument, race_table[iRace]->race_name ) )
         {
            do_help( ch, argument );
            write_to_buffer( d, "Please choose a race: ", 0 );
            return;
         }
      }
      write_to_buffer( d, "No help on that topic.  Please choose a race: ", 0 );
      return;
   }


   for( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
   {
      if( toupper( arg[0] ) == toupper( race_table[iRace]->race_name[0] )
          && !str_prefix( arg, race_table[iRace]->race_name ) )
      {
         ch->race = iRace;
         break;
      }
   }

   if( iRace == MAX_PC_RACE
       || !race_table[iRace]->race_name || race_table[iRace]->race_name[0] == '\0'
       || iRace == RACE_VAMPIRE
       || IS_SET( race_table[iRace]->class_restriction, 1 << ch->Class )
       || !str_cmp( race_table[iRace]->race_name, "unused" ) )
   {
      write_to_buffer( d, "That's not a race.\r\nWhat IS your race? ", 0 );
      return;
   }

   if( check_bans( ch, BAN_RACE ) )
   {
      write_to_buffer( d, "That race is not currently available.\r\nWhat is your race? ", 0 );
      return;
   }

   write_to_buffer( d, "\r\nWould you like RIP, ANSI or no graphic/color support, (R/A/N)? ", 0 );
   d->connected = CON_GET_WANT_RIPANSI;
}

void nanny_get_want_ripansi( DESCRIPTOR_DATA * d, const char *argument )
{
   CHAR_DATA *ch;
   char log_buf[MAX_STRING_LENGTH];

   ch = d->character;

   switch ( argument[0] )
   {
      case 'r':
      case 'R':
         xSET_BIT( ch->act, PLR_RIP );
         xSET_BIT( ch->act, PLR_ANSI );
         break;
      case 'a':
      case 'A':
         xSET_BIT( ch->act, PLR_ANSI );
         break;
      case 'n':
      case 'N':
         break;
      default:
         write_to_buffer( d, "Invalid selection.\r\nRIP, ANSI or NONE? ", 0 );
         return;
   }
   snprintf( log_buf, MAX_STRING_LENGTH, "%s@%s new %s %s.", ch->name, d->host,
             race_table[ch->race]->race_name, class_table[ch->Class]->who_name );
   log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
   to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
   write_to_buffer( d, "Press [ENTER] ", 0 );
   show_title( d );
   ch->level = 0;
   ch->position = POS_STANDING;
   d->connected = CON_PRESS_ENTER;
   set_pager_color( AT_PLAIN, ch );
}

void nanny_press_enter( DESCRIPTOR_DATA * d, const char *argument )
{
   CHAR_DATA *ch;

   ch = d->character;

   if( chk_watch( get_trust( ch ), ch->name, d->host ) ) /*  --Gorog */
      SET_BIT( ch->pcdata->flags, PCFLAG_WATCH );
   else
      REMOVE_BIT( ch->pcdata->flags, PCFLAG_WATCH );

   set_pager_color( AT_PLAIN, ch );
   if( xIS_SET( ch->act, PLR_RIP ) )
      send_rip_screen( ch );
   if( xIS_SET( ch->act, PLR_ANSI ) )
      send_to_pager( "\033[2J", ch );
   else
      send_to_pager( "\014", ch );
   if( IS_IMMORTAL( ch ) )
      do_help( ch, "imotd" );
   if( ch->level == LEVEL_AVATAR )
      do_help( ch, "amotd" );
   if( ch->level < LEVEL_AVATAR && ch->level > 0 )
      do_help( ch, "motd" );
   if( ch->level == 0 )
      do_help( ch, "nmotd" );
   send_to_pager( "\r\nPress [ENTER] ", ch );
   d->connected = CON_READ_MOTD;
}

void nanny_read_motd( DESCRIPTOR_DATA * d, const char *argument )
{
   CHAR_DATA *ch;
   char buf[MAX_STRING_LENGTH];
   char motdbuf[MAX_STRING_LENGTH];
   ch = d->character;

   snprintf( motdbuf, MAX_STRING_LENGTH, "\r\nWelcome to %s...\r\n", sysdata.mud_name );
   write_to_buffer( d, motdbuf, 0 );

   add_char( ch );
   d->connected = CON_PLAYING;

   if( ch->level == 0 )
   {
      OBJ_DATA *obj;
      int iLang, uLang;

      ch->pcdata->clan = NULL;
      switch ( class_table[ch->Class]->attr_prime )
      {
         case APPLY_STR:
            ch->perm_str = 16;
            break;
         case APPLY_INT:
            ch->perm_int = 16;
            break;
         case APPLY_WIS:
            ch->perm_wis = 16;
            break;
         case APPLY_DEX:
            ch->perm_dex = 16;
            break;
         case APPLY_CON:
            ch->perm_con = 16;
            break;
         case APPLY_CHA:
            ch->perm_cha = 16;
            break;
         case APPLY_LCK:
            ch->perm_lck = 16;
            break;
      }

      ch->perm_str += race_table[ch->race]->str_plus;
      ch->perm_int += race_table[ch->race]->int_plus;
      ch->perm_wis += race_table[ch->race]->wis_plus;
      ch->perm_dex += race_table[ch->race]->dex_plus;
      ch->perm_con += race_table[ch->race]->con_plus;
      ch->perm_cha += race_table[ch->race]->cha_plus;
      ch->affected_by = race_table[ch->race]->affected;
      ch->perm_lck += race_table[ch->race]->lck_plus;

      ch->armor += race_table[ch->race]->ac_plus;
      ch->alignment += race_table[ch->race]->alignment;
      ch->attacks = race_table[ch->race]->attacks;
      ch->defenses = race_table[ch->race]->defenses;
      ch->saving_poison_death = race_table[ch->race]->saving_poison_death;
      ch->saving_wand = race_table[ch->race]->saving_wand;
      ch->saving_para_petri = race_table[ch->race]->saving_para_petri;
      ch->saving_breath = race_table[ch->race]->saving_breath;
      ch->saving_spell_staff = race_table[ch->race]->saving_spell_staff;

      ch->height =
         number_range( ( int )( race_table[ch->race]->height * .9 ), ( int )( race_table[ch->race]->height * 1.1 ) );
      ch->weight =
         number_range( ( int )( race_table[ch->race]->weight * .9 ), ( int )( race_table[ch->race]->weight * 1.1 ) );

      if( ch->Class == CLASS_PALADIN )
         ch->alignment = 1000;

      if( ( iLang = skill_lookup( "common" ) ) < 0 )
         bug( "%s", "Nanny: cannot find common language." );
      else
         ch->pcdata->learned[iLang] = 100;

      /*
       * Give them their racial languages 
       */
      if( race_table[ch->race] )
      {
         for( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
         {
            if( IS_SET( race_table[ch->race]->language, 1 << iLang ) )
            {
               if( ( uLang = skill_lookup( lang_names[iLang] ) ) < 0 )
                  bug( "%s: cannot find racial language [%s].", __func__, lang_names[iLang] );
               else
                  ch->pcdata->learned[uLang] = 100;
            }
         }
      }

      /*
       * ch->resist           += race_table[ch->race]->resist;    drats
       */
      /*
       * ch->susceptible     += race_table[ch->race]->suscept;    drats
       */

      reset_colors( ch );
      name_stamp_stats( ch );

      ch->level = 1;
      ch->exp = 0;
      ch->max_hit += race_table[ch->race]->hit;
      ch->max_mana += race_table[ch->race]->mana;
      ch->hit = UMAX( 1, ch->max_hit );
      ch->mana = UMAX( 1, ch->max_mana );
      ch->move = ch->max_move;
      ch->gold = 0;
      /*
       * Set player birthday to current mud day, -17 years - Samson 10-25-99
       */
      ch->pcdata->day = time_info.day;
      ch->pcdata->month = time_info.month;
      ch->pcdata->year = time_info.year - 17;
      ch->pcdata->age = 17;
      ch->pcdata->age_bonus = 0;
      snprintf( buf, MAX_STRING_LENGTH, "the %s", title_table[ch->Class][ch->level][ch->sex == SEX_FEMALE ? 1 : 0] );
      set_title( ch, buf );

      /*
       * Added by Narn.  Start new characters with autoexit and autgold
       * already turned on.  Very few people don't use those.
       */
      xSET_BIT( ch->act, PLR_AUTOGOLD );
      xSET_BIT( ch->act, PLR_AUTOEXIT );

      /*
       * Added by Brittany, Nov 24/96.  The object is the adventurer's guide
       * to the realms of despair, part of Academy.are.
       */
      {
         OBJ_INDEX_DATA *obj_ind = get_obj_index( 10333 );
         if( obj_ind != NULL )
         {
            obj = create_object( obj_ind, 0 );
            obj_to_char( obj, ch );
            equip_char( ch, obj, WEAR_HOLD );
         }
      }
      if( !sysdata.WAIT_FOR_AUTH )
         char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
      else
      {
         char_to_room( ch, get_room_index( ROOM_AUTH_START ) );
         ch->pcdata->auth_state = 0;
         SET_BIT( ch->pcdata->flags, PCFLAG_UNAUTHED );
      }
   }
   else if( !IS_IMMORTAL( ch ) && ch->pcdata->release_date > 0 && ch->pcdata->release_date > current_time )
   {
      if( ch->in_room->vnum == 6 || ch->in_room->vnum == 8 || ch->in_room->vnum == 1206 )
         char_to_room( ch, ch->in_room );
      else
         char_to_room( ch, get_room_index( 8 ) );
   }
   else if( ch->in_room && ( IS_IMMORTAL( ch ) || !xIS_SET( ch->in_room->room_flags, ROOM_PROTOTYPE ) ) )
   {
      char_to_room( ch, ch->in_room );
   }
   else if( IS_IMMORTAL( ch ) )
   {
      char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
   }
   else
   {
      char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
   }

   if( get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
      remove_timer( ch, TIMER_SHOVEDRAG );

   if( get_timer( ch, TIMER_PKILLED ) > 0 )
      remove_timer( ch, TIMER_PKILLED );

   /* Login trigger by Edmond */
   rprog_login_trigger( ch );
   mprog_login_trigger( ch );

   act( AT_ACTION, "$n has entered the game.", ch, NULL, NULL, TO_CANSEE );
   if( ch->pcdata->pet )
   {
      act( AT_ACTION, "$n returns to $s master from the Void.", ch->pcdata->pet, NULL, ch, TO_NOTVICT );
      act( AT_ACTION, "$N returns with you to the realms.", ch, NULL, ch->pcdata->pet, TO_CHAR );
   }
   do_look( ch, "auto" );
   mail_count( ch );
   check_loginmsg( ch );

   if( !ch->was_in_room && ch->in_room == get_room_index( ROOM_VNUM_TEMPLE ) )
      ch->was_in_room = get_room_index( ROOM_VNUM_TEMPLE );
   else if( ch->was_in_room == get_room_index( ROOM_VNUM_TEMPLE ) )
      ch->was_in_room = get_room_index( ROOM_VNUM_TEMPLE );
   else if( !ch->was_in_room )
      ch->was_in_room = ch->in_room;
}

void nanny_delete_char( DESCRIPTOR_DATA * d, const char *argument )
{
   CHAR_DATA *ch = d->character;

   if( !ch )
   {
      bug( "%s: NULL ch!", __func__ );
      d->connected = CON_PLAYING;
      return;
   }

   write_to_buffer( d, "\r\n", 2 );

   if( str_cmp( sha256_crypt( argument ), ch->pcdata->pwd ) )
   {
      write_to_buffer( d, "Wrong password entered, deletion cancelled.\r\n", 0 );
      write_to_buffer( d, (const char *)echo_on_str, 0 );
      d->connected = CON_PLAYING;
      return;
   }
   else
   {
      write_to_buffer( d, "\r\nYou've deleted your character!!!\r\n", 0 );
      log_printf( "Player: %s has deleted.", capitalize( ch->name ) );
      do_destroy( ch, ch->name );
      return;
   }
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA * d, char *argument )
{
   while( isspace( *argument ) )
      argument++;

   switch ( d->connected )
   {
      default:
         bug( "%s: bad d->connected %d.", __func__, d->connected );
         close_socket( d, TRUE );
         return;

      case CON_GET_NAME:
         nanny_get_name( d, argument );
         break;

      case CON_GET_OLD_PASSWORD:
         nanny_get_old_password( d, argument );
         break;

      case CON_CONFIRM_NEW_NAME:
         nanny_confirm_new_name( d, argument );
         break;

      case CON_GET_NEW_PASSWORD:
         nanny_get_new_password( d, argument );
         break;

      case CON_CONFIRM_NEW_PASSWORD:
         nanny_confirm_new_password( d, argument );
         break;

      case CON_GET_NEW_SEX:
         nanny_get_new_sex( d, argument );
         break;

      case CON_GET_NEW_CLASS:
         nanny_get_new_class( d, argument );
         break;

      case CON_GET_NEW_RACE:
         nanny_get_new_race( d, argument );
         break;

      case CON_GET_WANT_RIPANSI:
         nanny_get_want_ripansi( d, argument );
         break;

      case CON_PRESS_ENTER:
         nanny_press_enter( d, argument );
         break;

      case CON_READ_MOTD:
         nanny_read_motd( d, argument );
         break;

      case CON_DELETE:
         nanny_delete_char( d, argument );
         break;
   }
   return;
}

bool is_reserved_name( const char *name )
{
   RESERVE_DATA *res;

   for( res = first_reserved; res; res = res->next )
      if( ( *res->name == '*' && !str_infix( res->name + 1, name ) ) || !str_cmp( res->name, name ) )
         return TRUE;
   return FALSE;
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name( const char *name, bool newchar )
{
   /*
    * Names checking should really only be done on new characters, otherwise
    * we could end up with people who can't access their characters.  Would
    * have also provided for that new area havoc mentioned below, while still
    * disallowing current area mobnames.  I personally think that if we can
    * have more than one mob with the same keyword, then may as well have
    * players too though, so I don't mind that removal.  -- Alty
    */

   if( is_reserved_name( name ) && newchar )
      return FALSE;

   /*
    * Length restrictions.
    */
   if( strlen( name ) < 3 )
      return FALSE;

   if( strlen( name ) > 12 )
      return FALSE;

   /*
    * Alphanumerics only.
    * Lock out IllIll twits.
    */
   {
      const char *pc;
      bool fIll;

      fIll = TRUE;
      for( pc = name; *pc != '\0'; pc++ )
      {
         if( !isalpha( *pc ) )
            return FALSE;
         if( LOWER( *pc ) != 'i' && LOWER( *pc ) != 'l' )
            fIll = FALSE;
      }

      if( fIll )
         return FALSE;
   }

   /*
    * Code that followed here used to prevent players from naming
    * themselves after mobs... this caused much havoc when new areas
    * would go in...
    */
   return TRUE;
}

/*
 * Look for link-dead player to reconnect.
 */
short check_reconnect( DESCRIPTOR_DATA * d, const char *name, bool fConn )
{
   CHAR_DATA *ch;

   for( ch = first_char; ch; ch = ch->next )
   {
      if( !IS_NPC( ch ) && ( !fConn || !ch->desc ) && ch->pcdata->filename && !str_cmp( name, ch->pcdata->filename ) )
      {
         if( fConn && ch->switched )
         {
            write_to_buffer( d, "Already playing.\r\nName: ", 0 );
            d->connected = CON_GET_NAME;
            if( d->character )
            {
               /*
                * clear descriptor pointer to get rid of bug message in log 
                */
               d->character->desc = NULL;
               free_char( d->character );
               d->character = NULL;
            }
            return BERR;
         }
         if( fConn == FALSE )
         {
            DISPOSE( d->character->pcdata->pwd );
            d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
         }
         else
         {
            /*
             * clear descriptor pointer to get rid of bug message in log 
             */
            d->character->desc = NULL;
            free_char( d->character );
            d->character = ch;
            ch->desc = d;
            ch->timer = 0;
            send_to_char( "Reconnecting.\r\n", ch );
            if( d->host )
               ch->pcdata->recent_site = STRALLOC( d->host );
            rprog_login_trigger( ch );
            mprog_login_trigger( ch );
            act( AT_ACTION, "$n has reconnected.", ch, NULL, NULL, TO_CANSEE );
            log_printf_plus( LOG_COMM, UMAX( sysdata.log_level, ch->level ), "%s (%s) reconnected.", ch->name, d->host );
            d->connected = CON_PLAYING;
            do_look( ch, "auto" );
            check_loginmsg( ch );
         }
         return TRUE;
      }
   }
   return FALSE;
}

/*
 * Check if already playing.
 */
short check_playing( DESCRIPTOR_DATA * d, const char *name, bool kick )
{
   CHAR_DATA *ch;
   DESCRIPTOR_DATA *dold;
   int cstate;

   for( dold = first_descriptor; dold; dold = dold->next )
   {
      if( dold != d
          && ( dold->character || dold->original )
          && !str_cmp( name, dold->original ? dold->original->pcdata->filename : dold->character->pcdata->filename ) )
      {
         cstate = dold->connected;
         ch = dold->original ? dold->original : dold->character;
         if( !ch->name || ( cstate != CON_PLAYING && cstate != CON_EDITING && cstate != CON_DELETE ) )
         {
            write_to_buffer( d, "Already connected - try again.\r\n", 0 );
            log_printf_plus( LOG_COMM, sysdata.log_level, "%s already connected.", ch->pcdata->filename );
            return BERR;
         }
         if( !kick )
            return TRUE;
         write_to_buffer( d, "Already playing... Kicking off old connection.\r\n", 0 );
         write_to_buffer( dold, "Kicking off old connection... bye!\r\n", 0 );
         close_socket( dold, FALSE );
         /*
          * clear descriptor pointer to get rid of bug message in log 
          */
         d->character->desc = NULL;
         free_char( d->character );
         d->character = ch;
         ch->desc = d;
         ch->timer = 0;
         if( ch->switched )
            do_return( ch->switched, "" );
         ch->switched = NULL;
         send_to_char( "Reconnecting.\r\n", ch );
         if( d->host )
            ch->pcdata->recent_site = STRALLOC( d->host );
         rprog_login_trigger( ch );
         mprog_login_trigger( ch );
         do_look( ch, "auto" );
         check_loginmsg( ch );
         act( AT_ACTION, "$n has reconnected, kicking off old link.", ch, NULL, NULL, TO_CANSEE );
         log_printf_plus( LOG_COMM, UMAX( sysdata.log_level, ch->level ), "%s@%s reconnected, kicking off old link.",
                          ch->pcdata->filename, d->host );
         d->connected = cstate;
         return TRUE;
      }
   }
   return FALSE;
}

void stop_idling( CHAR_DATA * ch )
{
   ROOM_INDEX_DATA *was_in_room;

   if( !ch || !ch->desc || ch->desc->connected != CON_PLAYING || !IS_IDLE( ch ) )
      return;

   ch->timer = 0;
   was_in_room = ch->was_in_room;
   char_from_room( ch );
   char_to_room( ch, was_in_room );
   ch->was_in_room = ch->in_room;
   /*
    * ch->was_in_room  = NULL;
    */
   REMOVE_BIT( ch->pcdata->flags, PCFLAG_IDLE );

   /* Void triggers by Edmond */
   rprog_void_trigger( ch );
   mprog_void_trigger( ch );

   act( AT_ACTION, "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
   return;
}

/*
 * Function to strip off the "a" or "an" or "the" or "some" from an object's
 * short description for the purpose of using it in a sentence sent to
 * the owner of the object.  (Ie: an object with the short description
 * "a long dark blade" would return "long dark blade" for use in a sentence
 * like "Your long dark blade".  The object name isn't always appropriate
 * since it contains keywords that may not look proper.		-Thoric
 */
const char *myobj( OBJ_DATA * obj )
{
   if( !str_prefix( "a ", obj->short_descr ) )
      return obj->short_descr + 2;
   if( !str_prefix( "an ", obj->short_descr ) )
      return obj->short_descr + 3;
   if( !str_prefix( "the ", obj->short_descr ) )
      return obj->short_descr + 4;
   if( !str_prefix( "some ", obj->short_descr ) )
      return obj->short_descr + 5;
   return obj->short_descr;
}

const char *obj_short( OBJ_DATA * obj )
{
   static char buf[MAX_STRING_LENGTH];

   if( obj->count > 1 )
   {
      snprintf( buf, MAX_STRING_LENGTH, "%s (%d)", obj->short_descr, obj->count );
      return buf;
   }
   return obj->short_descr;
}

#define NAME(ch)        ( IS_NPC(ch) ? ch->short_descr : ch->name )

const char *MORPHNAME( CHAR_DATA * ch )
{
   if( ch->morph && ch->morph->morph && ch->morph->morph->short_desc != NULL )
      return ch->morph->morph->short_desc;
   else
      return NAME( ch );
}

char *act_string( const char *format, CHAR_DATA * to, CHAR_DATA * ch, const void *arg1, const void *arg2, int flags )
{
   static const char *const he_she[] = { "it", "he", "she" };
   static const char *const him_her[] = { "it", "him", "her" };
   static const char *const his_her[] = { "its", "his", "her" };
   static char buf[MAX_STRING_LENGTH];
   char fname[MAX_INPUT_LENGTH];
   char *point = buf;
   char temp[MSL];
   const char *str = format;
   const char *i;
   bool should_upper = false;
   CHAR_DATA *vch = ( CHAR_DATA * ) arg2;
   OBJ_DATA *obj1 = ( OBJ_DATA * ) arg1;
   OBJ_DATA *obj2 = ( OBJ_DATA * ) arg2;

   if( str[0] == '$' )
      DONT_UPPER = false;

   while( *str != '\0' )
   {
      if( *str == '.' || *str == '?' || *str == '!' )
         should_upper = true;
      else if( should_upper == true && !isspace( *str ) && *str != '$' )
         should_upper = false;

      if( *str != '$' )
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      if( !arg2 && *str >= 'A' && *str <= 'Z' )
      {
         bug( "Act: missing arg2 for code %c:", *str );
         bug( "%s", format );
         i = " <@@@> ";
      }
      else
      {
         switch ( *str )
         {
            default:
               bug( "Act: bad code %c.", *str );
               i = " <@@@> ";
               break;

            case 'd':
               if( !arg2 || ( ( char * )arg2 )[0] == '\0' )
                  i = "door";
               else
               {
                  one_argument( ( char * )arg2, fname );
                  i = fname;
               }
               break;

            case 'e':
               if( ch->sex > 2 || ch->sex < 0 )
               {
                  bug( "act_string: player %s has sex set at %d!", ch->name, ch->sex );
                  i = should_upper ? "It" : "it";
               }
               else
                  i = should_upper ?
                   !can_see( to, ch ) ? "It" : capitalize( he_she[URANGE( 0, ch->sex, 2 )] ) :
                   !can_see( to, ch ) ? "it" : he_she[URANGE( 0, ch->sex, 2 )];
               break;

            case 'E':
               if( vch->sex > 2 || vch->sex < 0 )
               {
                  bug( "act_string: player %s has sex set at %d!", vch->name, vch->sex );
                  i = should_upper ? "It" : "it";
               }
               else
                  i = should_upper ?
                   !can_see( to, vch ) ? "It" : capitalize( he_she[URANGE( 0, vch->sex, 2 )] ) :
                   !can_see( to, vch ) ? "it" : he_she[URANGE( 0, vch->sex, 2 )];
               break;

            case 'm':
               if( ch->sex > 2 || ch->sex < 0 )
               {
                  bug( "act_string: player %s has sex set at %d!", ch->name, ch->sex );
                  i = should_upper ? "It" : "it";
               }
               else
                  i = should_upper ?
                   !can_see( to, ch ) ? "It" : capitalize( him_her[URANGE( 0, ch->sex, 2 )] ) :
                   !can_see( to, ch ) ? "it" : him_her[URANGE( 0, ch->sex, 2 )];
               break;

            case 'M':
               if( vch->sex > 2 || vch->sex < 0 )
               {
                  bug( "act_string: player %s has sex set at %d!", vch->name, vch->sex );
                  i = should_upper ? "It" : "it";
               }
               else
                  i = should_upper ?
                   !can_see( to, vch ) ? "It" : capitalize( him_her[URANGE( 0, vch->sex, 2 )] ) :
                   !can_see( to, vch ) ? "it" : him_her[URANGE( 0, vch->sex, 2 )];
               break;

            case 'n':
               if( !can_see( to, ch ) )
                  i = "Someone";
               else
               {
                  snprintf( temp, sizeof( temp ), "%s", ( to ? PERS( ch, to ) : NAME( ch ) ) );
                  i = temp;
               }
               break;

            case 'N':
               if( !can_see( to, vch ) )
                  i = "Someone";
               else
               {
                  snprintf( temp, sizeof( temp ), "%s", ( to ? PERS( vch, to ) : NAME( vch ) ) );
                  i = temp;
               }
               break;

            case 'p':
               if( !to || can_see_obj( to, obj1 ) )
               {
                  /*
                   * Prevents act programs from triggering off note shorts 
                   */
                  if( ( !to || IS_NPC( to ) ) && ( obj1->item_type == ITEM_PAPER ) )
                     i = obj1->pIndexData->short_descr;
                  else
                     i = obj_short( obj1 );
               }
               else
                  i = "something";
               break;

            case 'P':
               if( !to || can_see_obj( to, obj2 ) )
               {
                  /*
                   * Prevents act programs from triggering off note shorts 
                   */
                  if( ( !to || IS_NPC( to ) ) && ( obj2->item_type == ITEM_PAPER ) )
                     i = obj2->pIndexData->short_descr;
                  else
                     i = obj_short( obj2 );
               }
               else
                  i = "something";
               break;

            case 'q':
               i = ( to == ch ) ? "" : "s";
               break;

            case 'Q':
               i = ( to == ch ) ? "your" : his_her[URANGE( 0, ch->sex, 2 )];
               break;

            case 's':
               if( ch->sex > 2 || ch->sex < 0 )
               {
                  bug( "act_string: player %s has sex set at %d!", ch->name, ch->sex );
                  i = should_upper ? "It" : "it";
               }
               else
                  i = should_upper ? 
                   !can_see( to, ch ) ? "It" : capitalize( his_her[URANGE( 0, ch->sex, 2 )] ) :
                   !can_see( to, ch ) ? "it" : his_her[URANGE( 0, ch->sex, 2 )];
               break;

            case 'S':
               if( vch->sex > 2 || vch->sex < 0 )
               {
                  bug( "act_string: player %s has sex set at %d!", vch->name, vch->sex );
                  i = should_upper ? "It" : "it";
               }
               else
                  i = should_upper ? 
                   !can_see( to, vch ) ? "It" : capitalize( his_her[URANGE( 0, vch->sex, 2 )] ) :
                   !can_see( to, vch ) ? "it" : his_her[URANGE( 0, vch->sex, 2 )];
               break;

            case 't':
               /* bug fix - Edmond  i = (char *) arg1;         break; */
               if ( arg1 )
                  i = (char *) arg1;
               else
               {
                  bug( "Act: Bad variable $t" );
                  i = " <@@@> ";
               }
               break;

            case 'T':
               /* same bug fix as above -  i = (char *) arg2;          break; */
               if ( arg2 )
                  i = (char *) arg2;
               else
               {
                  bug( "Act: Bad variable $T" );
                  i = " <@@@> ";
               }
               break;
         }
      }
      ++str;
      while( ( *point = *i ) != '\0' )
         ++point, ++i;
   }
   mudstrlcpy( point, "\r\n", MSL );

   if( !DONT_UPPER )
   {
      bool bUppercase = true;     //Always uppercase first letter
      char *astr = buf;
      for( char c = *astr; c; c= *++astr )
      {
          if( c == '&' )
          {
               //Color Code
               c = *++astr;     //Read Color Code
               if( c == '[' )
               {
                    //Extended color code, skip until ']'
                    do { c = *++astr; } while ( c && c != ']' );
               }

               if( !c )
                   break;
          }
          else if( bUppercase && isalpha( c ) )
          {
               *astr = toupper(c);
               bUppercase = false;
          }
     }
   }
   return buf;
}
#undef NAME

void act( short AType, const char *format, CHAR_DATA * ch, const void *arg1, const void *arg2, int type )
{
   const char *txt;
   const char *str;
   CHAR_DATA *to;
   CHAR_DATA *vch = ( CHAR_DATA * ) arg2;
#define ACTF_NONE 0
#define ACTF_TXT  BV00
#define ACTF_CH   BV01
#define ACTF_OBJ  BV02
   OBJ_DATA *obj1 = ( OBJ_DATA * ) arg1;
   OBJ_DATA *obj2 = ( OBJ_DATA * ) arg2;
   int flags1 = ACTF_NONE, flags2 = ACTF_NONE;

   /*
    * Discard null and zero-length messages.
    */
   if( !format || format[0] == '\0' )
      return;

   if( !ch )
   {
      bug( "%s: null ch. (%s)", __func__, format );
      return;
   }

   // Do some proper type checking here..  Sort of.  We base it on the $* params.
   // This is kinda lame really, but I suppose in some weird sense it beats having
   // to pass like 8 different NULL parameters every time we need to call act()..
   for( str = format; *str; ++str )
   {
      if( *str == '$' )
      {
         if( !*++str )
            break;
         switch ( *str )
         {
            default:
               bug( "%s: bad code %c for format %s.", __func__, *str, format );
               break;

            case 't':
               flags1 |= ACTF_TXT;
               obj1 = NULL;
               break;

            case 'T':
            case 'd':
               flags2 |= ACTF_TXT;
               vch = NULL;
               obj2 = NULL;
               break;

            case 'n':
            case 'e':
            case 'm':
            case 's':
            case 'q':
               break;

            case 'N':
            case 'E':
            case 'M':
            case 'S':
            case 'Q':
               flags2 |= ACTF_CH;
               obj2 = NULL;
               break;

            case 'p':
               flags1 |= ACTF_OBJ;
               break;

            case 'P':
               flags2 |= ACTF_OBJ;
               vch = NULL;
               break;
         }
      }
   }

   if( flags1 != ACTF_NONE && flags1 != ACTF_TXT && flags1 != ACTF_CH && flags1 != ACTF_OBJ )
   {
      bug( "%s: arg1 has more than one type in format %s. Setting all NULL.", __func__, format );
      obj1 = NULL;
   }

   if( flags2 != ACTF_NONE && flags2 != ACTF_TXT && flags2 != ACTF_CH && flags2 != ACTF_OBJ )
   {
      bug( "%s: arg2 has more than one type in format %s. Setting all NULL.", __func__, format );
      vch = NULL;
      obj2 = NULL;
   }

   if( !ch->in_room )
      to = NULL;
   else if( type == TO_CHAR )
      to = ch;
   else
      to = ch->in_room->first_person;

   /*
    * ACT_SECRETIVE handling
    */
   if( IS_NPC( ch ) && xIS_SET( ch->act, ACT_SECRETIVE ) && type != TO_CHAR )
      return;

   if( type == TO_VICT )
   {
      if( !vch )
      {
         bug( "%s", "Act: null vch with TO_VICT." );
         bug( "%s (%s)", ch->name, format );
         return;
      }
      if( !vch->in_room )
      {
         bug( "%s", "Act: vch in NULL room!" );
         bug( "%s -> %s (%s)", ch->name, vch->name, format );
         return;
      }
      to = vch;
   }

   if( MOBtrigger && type != TO_CHAR && type != TO_VICT && to )
   {
      OBJ_DATA *to_obj;

      txt = act_string( format, NULL, ch, arg1, arg2, STRING_IMM );
      if( HAS_PROG( to->in_room, ACT_PROG ) )
         rprog_act_trigger( txt, to->in_room, ch, obj1, vch, obj2 );
      for( to_obj = to->in_room->first_content; to_obj; to_obj = to_obj->next_content )
         if( HAS_PROG( to_obj->pIndexData, ACT_PROG ) )
            oprog_act_trigger( txt, to_obj, ch, obj1, vch, obj2 );
   }

   /*
    * Anyone feel like telling me the point of looping through the whole
    * room when we're only sending to one char anyways..? -- Alty 
    */
   for( ; to; to = ( type == TO_CHAR || type == TO_VICT ) ? NULL : to->next_in_room )
   {
      if( ( !to->desc && ( IS_NPC( to ) && !HAS_PROG( to->pIndexData, ACT_PROG ) ) ) || !IS_AWAKE( to ) )
         continue;

      if( type == TO_CHAR && to != ch )
         continue;
      if( type == TO_VICT && ( to != vch || to == ch ) )
         continue;
      if( type == TO_ROOM && to == ch )
         continue;
      if( type == TO_NOTVICT && ( to == ch || to == vch ) )
         continue;
      if( type == TO_CANSEE && ( to == ch
         || ( !IS_NPC(ch) && (xIS_SET(ch->act, PLR_WIZINVIS)
         && ( get_trust(to) < ( ch->pcdata ? ch->pcdata->wizinvis : 0 ) ) ) ) ) )
         continue;

      if( IS_IMMORTAL( to ) )
         txt = act_string( format, to, ch, arg1, arg2, STRING_IMM );
      else
         txt = act_string( format, to, ch, arg1, arg2, STRING_NONE );

      if( to->desc )
      {
         set_char_color( AType, to );
         send_to_char( txt, to );
      }

      if( MOBtrigger )
      {
         /*
          * Note: use original string, not string with ANSI. -- Alty 
          */
         mprog_act_trigger( txt, to, ch, obj1, vch, obj2 );
      }
   }
   MOBtrigger = TRUE;
   return;
}

void do_name( CHAR_DATA* ch, const char* argument)
{
   char ucase_argument[MAX_STRING_LENGTH];
   char fname[1024];
   struct stat fst;
   CHAR_DATA *tmp;

   if( !NOT_AUTHED( ch ) || ch->pcdata->auth_state != 2 )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   mudstrlcpy( ucase_argument, argument, MAX_STRING_LENGTH );
   ucase_argument[0] = UPPER( argument[0] );

   if( !check_parse_name( ucase_argument, TRUE ) )
   {
      send_to_char( "That name is reserved, please try another.\r\n", ch );
      return;
   }

   if( check_playing( ch->desc, argument, FALSE ) == BERR )
      return;

   if( !str_cmp( ch->name, ucase_argument ) )
   {
      send_to_char( "That's already your name!\r\n", ch );
      return;
   }

   for( tmp = first_char; tmp; tmp = tmp->next )
   {
      if( !str_cmp( argument, tmp->name ) )
         break;
   }

   if( tmp )
   {
      send_to_char( "That name is already taken.  Please choose another.\r\n", ch );
      return;
   }

   snprintf( fname, 1024, "%s%c/%s", PLAYER_DIR, tolower( ucase_argument[0] ), capitalize( ucase_argument ) );
   if( stat( fname, &fst ) != -1 )
   {
      send_to_char( "That name is already taken.  Please choose another.\r\n", ch );
      return;
   }

   STRFREE( ch->name );
   ch->name = STRALLOC( ucase_argument );
   STRFREE( ch->pcdata->filename );
   ch->pcdata->filename = STRALLOC( ucase_argument );
   send_to_char( "Your name has been changed.  Please apply again.\r\n", ch );
   ch->pcdata->auth_state = 1;
   return;
}

/* Alternate Self delete command provided by Waldemar Thiel (Swiv) */
/* Allows characters to delete themselves - Installed 1-18-98 by Samson */
void do_delet( CHAR_DATA *ch, const char *argument )
{
   send_to_char( "If you want to DELETE, spell it out.\r\n", ch );
   return;
}

void do_delete( CHAR_DATA *ch, const char *argument )
{
   if( IS_NPC(ch) )
   {
      send_to_char ( "Yeah, right. Mobs can't delete themselves.\r\n", ch );
      return;
   }

   if( ch->fighting != NULL )
   {
      send_to_char( "Wait until the fight is over before deleting yourself.\r\n", ch );
      return;
   }

   /* Reimbursement warning added to code by Samson 1-18-98 */
   set_char_color( AT_YELLOW, ch );
   send_to_char( "Remember, this decision is IRREVERSABLE. There are no reimbursements!\r\n", ch );

   /* Immortals warning added to code by Samson 1-18-98 */
   if( IS_IMMORTAL(ch) )
   {
      ch_printf( ch, "Consider this carefuly %s, if you delete, you will not\r\nbe reinstated as an immortal!\r\n", ch->name );
      send_to_char( "Any area data you have will also be lost if you proceed.\r\n", ch );
   }

   set_char_color( AT_RED, ch );
   send_to_char( "\r\nType your password if you wish to delete your character.\r\n", ch );
   send_to_char( "[DELETE] Password: ", ch );
   write_to_buffer( ch->desc, (const char *)echo_off_str, 0 );
   ch->desc->connected = CON_DELETE;
   return;
}

char *default_fprompt( CHAR_DATA * ch )
{
   static char buf[60];

   mudstrlcpy( buf, "&w<&Y%hhp ", 60 );
   if( IS_VAMPIRE( ch ) )
      mudstrlcat( buf, "&R%bbp", 60 );
   else
      mudstrlcat( buf, "&C%mm", 60 );
   mudstrlcat( buf, " &G%vmv&w> ", 60 );
   if( IS_NPC( ch ) || IS_IMMORTAL( ch ) )
      mudstrlcat( buf, "%i%R", 60 );
   return buf;
}

char *default_prompt( CHAR_DATA * ch )
{
   static char buf[60];

   mudstrlcpy( buf, "&w<&Y%hhp ", 60 );
   if( IS_VAMPIRE( ch ) )
      mudstrlcat( buf, "&R%bbp", 60 );
   else
      mudstrlcat( buf, "&C%mm", 60 );
   mudstrlcat( buf, " &G%vmv&w> ", 60 );
   if( IS_NPC( ch ) || IS_IMMORTAL( ch ) )
      mudstrlcat( buf, "%i%R", 60 );
   return buf;
}

int getcolor( char clr )
{
   static const char colors[17] = "xrgObpcwzRGYBPCW";
   int r;

   for( r = 0; r < 16; r++ )
      if( clr == colors[r] )
         return r;
   return -1;
}

void display_prompt( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *ch = d->character;
   CHAR_DATA *och = ( d->original ? d->original : d->character );
   CHAR_DATA *victim;
   bool ansi = ( !IS_NPC( och ) && xIS_SET( och->act, PLR_ANSI ) );
   const char *prompt;
   const char *helpstart = "<Type HELP START>";
   char buf[MAX_STRING_LENGTH];
   char *pbuf = buf;
   unsigned int pstat;
   int percent;

   if( !ch )
   {
      bug( "%s: NULL ch", __func__ );
      return;
   }

   if( !IS_NPC( ch ) && !IS_SET( ch->pcdata->flags, PCFLAG_HELPSTART ) )
      prompt = helpstart;
   else if( !IS_NPC( ch ) && ch->substate != SUB_NONE && ch->pcdata->subprompt && ch->pcdata->subprompt[0] != '\0' )
      prompt = ch->pcdata->subprompt;
   else if( IS_NPC( ch ) || ( !ch->fighting && ( !ch->pcdata->prompt || !*ch->pcdata->prompt ) ) )
      prompt = default_prompt( ch );
   else if( ch->fighting )
   {
      if( !ch->pcdata->fprompt || !*ch->pcdata->fprompt )
         prompt = default_fprompt( ch );
      else
         prompt = ch->pcdata->fprompt;
   }
   else
      prompt = ch->pcdata->prompt;

   if( ansi )
   {
      mudstrlcpy( pbuf, ANSI_RESET, MAX_STRING_LENGTH );
      d->prevcolor = 0x08;
      pbuf += 4;
   }

   /*
    * Clear out old color stuff 
    */
   for( ; *prompt; prompt++ )
   {
      /*
       * '%' = prompt commands
       * Note: foreground changes will revert background to 0 (black)
       */
      if( *prompt != '%' )
      {
         *( pbuf++ ) = *prompt;
         continue;
      }
      ++prompt;

      if( !*prompt )
         break;

      if( *prompt == *( prompt - 1 ) )
      {
         *( pbuf++ ) = *prompt;
         continue;
      }

      switch ( *( prompt - 1 ) )
      {
         default:
            bug( "%s: bad command char '%c'.", __func__, *( prompt - 1 ) );
            break;

         case '%':
            *pbuf = '\0';
            pstat = 0x80000000;

            switch ( *prompt )
            {
               case '%':
                  *pbuf++ = '%';
                  *pbuf = '\0';
                  break;

               case 'a':
                  if( ch->level >= 10 )
                     pstat = ch->alignment;
                  else if( IS_GOOD( ch ) )
                     mudstrlcpy( pbuf, "good", MAX_STRING_LENGTH );
                  else if( IS_EVIL( ch ) )
                     mudstrlcpy( pbuf, "evil", MAX_STRING_LENGTH );
                  else
                     mudstrlcpy( pbuf, "neutral", MAX_STRING_LENGTH );
                  break;

               case 'A':
                  snprintf( pbuf, MAX_STRING_LENGTH, "%s%s%s", IS_AFFECTED( ch, AFF_INVISIBLE ) ? "I" : "",
                            IS_AFFECTED( ch, AFF_HIDE ) ? "H" : "", IS_AFFECTED( ch, AFF_SNEAK ) ? "S" : "" );
                  break;

               case 'C':  /* Tank */
                  if( !ch->fighting || ( victim = ch->fighting->who ) == NULL )
                     mudstrlcpy( pbuf, "N/A", MAX_STRING_LENGTH );
                  else if( !victim->fighting || ( victim = victim->fighting->who ) == NULL )
                     mudstrlcpy( pbuf, "N/A", MAX_STRING_LENGTH );
                  else
                  {
                     if( victim->max_hit > 0 )
                        percent = ( 100 * victim->hit ) / victim->max_hit;
                     else
                        percent = -1;
                     if( percent >= 100 )
                        mudstrlcpy( pbuf, "perfect health", MAX_STRING_LENGTH );
                     else if( percent >= 90 )
                        mudstrlcpy( pbuf, "slightly scratched", MAX_STRING_LENGTH );
                     else if( percent >= 80 )
                        mudstrlcpy( pbuf, "few bruises", MAX_STRING_LENGTH );
                     else if( percent >= 70 )
                        mudstrlcpy( pbuf, "some cuts", MAX_STRING_LENGTH );
                     else if( percent >= 60 )
                        mudstrlcpy( pbuf, "several wounds", MAX_STRING_LENGTH );
                     else if( percent >= 50 )
                        mudstrlcpy( pbuf, "nasty wounds", MAX_STRING_LENGTH );
                     else if( percent >= 40 )
                        mudstrlcpy( pbuf, "bleeding freely", MAX_STRING_LENGTH );
                     else if( percent >= 30 )
                        mudstrlcpy( pbuf, "covered in blood", MAX_STRING_LENGTH );
                     else if( percent >= 20 )
                        mudstrlcpy( pbuf, "leaking guts", MAX_STRING_LENGTH );
                     else if( percent >= 10 )
                        mudstrlcpy( pbuf, "almost dead", MAX_STRING_LENGTH );
                     else
                        mudstrlcpy( pbuf, "DYING", MAX_STRING_LENGTH );
                  }
                  break;

               case 'c':
                  if( !ch->fighting || ( victim = ch->fighting->who ) == NULL )
                     mudstrlcpy( pbuf, "N/A", MAX_STRING_LENGTH );
                  else
                  {
                     if( victim->max_hit > 0 )
                        percent = ( 100 * victim->hit ) / victim->max_hit;
                     else
                        percent = -1;
                     if( percent >= 100 )
                        mudstrlcpy( pbuf, "perfect health", MAX_STRING_LENGTH );
                     else if( percent >= 90 )
                        mudstrlcpy( pbuf, "slightly scratched", MAX_STRING_LENGTH );
                     else if( percent >= 80 )
                        mudstrlcpy( pbuf, "few bruises", MAX_STRING_LENGTH );
                     else if( percent >= 70 )
                        mudstrlcpy( pbuf, "some cuts", MAX_STRING_LENGTH );
                     else if( percent >= 60 )
                        mudstrlcpy( pbuf, "several wounds", MAX_STRING_LENGTH );
                     else if( percent >= 50 )
                        mudstrlcpy( pbuf, "nasty wounds", MAX_STRING_LENGTH );
                     else if( percent >= 40 )
                        mudstrlcpy( pbuf, "bleeding freely", MAX_STRING_LENGTH );
                     else if( percent >= 30 )
                        mudstrlcpy( pbuf, "covered in blood", MAX_STRING_LENGTH );
                     else if( percent >= 20 )
                        mudstrlcpy( pbuf, "leaking guts", MAX_STRING_LENGTH );
                     else if( percent >= 10 )
                        mudstrlcpy( pbuf, "almost dead", MAX_STRING_LENGTH );
                     else
                        mudstrlcpy( pbuf, "DYING", MAX_STRING_LENGTH );
                  }
                  break;

               case 'h':
                  pstat = ch->hit;
                  break;

               case 'H':
                  pstat = ch->max_hit;
                  break;

               case 'm':
                  if( IS_VAMPIRE( ch ) )
                     pstat = 0;
                  else
                     pstat = ch->mana;
                  break;

               case 'M':
                  if( IS_VAMPIRE( ch ) )
                     pstat = 0;
                  else
                     pstat = ch->max_mana;
                  break;

               case 'N':  /* Tank */
                  if( !ch->fighting || ( victim = ch->fighting->who ) == NULL )
                     mudstrlcpy( pbuf, "N/A", MAX_STRING_LENGTH );
                  else if( !victim->fighting || ( victim = victim->fighting->who ) == NULL )
                     mudstrlcpy( pbuf, "N/A", MAX_STRING_LENGTH );
                  else
                  {
                     if( ch == victim )
                        mudstrlcpy( pbuf, "You", MAX_STRING_LENGTH );
                     else if( IS_NPC( victim ) )
                        mudstrlcpy( pbuf, victim->short_descr, MAX_STRING_LENGTH );
                     else
                        mudstrlcpy( pbuf, victim->name, MAX_STRING_LENGTH );
                     pbuf[0] = UPPER( pbuf[0] );
                  }
                  break;

               case 'n':
                  if( !ch->fighting || ( victim = ch->fighting->who ) == NULL )
                     mudstrlcpy( pbuf, "N/A", MAX_STRING_LENGTH );
                  else
                  {
                     if( ch == victim )
                        mudstrlcpy( pbuf, "You", MAX_STRING_LENGTH );
                     else if( IS_NPC( victim ) )
                        mudstrlcpy( pbuf, victim->short_descr, MAX_STRING_LENGTH );
                     else
                        mudstrlcpy( pbuf, victim->name, MAX_STRING_LENGTH );
                     pbuf[0] = UPPER( pbuf[0] );
                  }
                  break;

               case 'T':
                  if( time_info.hour < 5 )
                     mudstrlcpy( pbuf, "night", MAX_STRING_LENGTH );
                  else if( time_info.hour < 6 )
                     mudstrlcpy( pbuf, "dawn", MAX_STRING_LENGTH );
                  else if( time_info.hour < 19 )
                     mudstrlcpy( pbuf, "day", MAX_STRING_LENGTH );
                  else if( time_info.hour < 21 )
                     mudstrlcpy( pbuf, "dusk", MAX_STRING_LENGTH );
                  else
                     mudstrlcpy( pbuf, "night", MAX_STRING_LENGTH );
                  break;

               case 'b':
                  if( IS_VAMPIRE( ch ) )
                     pstat = ch->pcdata->condition[COND_BLOODTHIRST];
                  else
                     pstat = 0;
                  break;

               case 'B':
                  if( IS_VAMPIRE( ch ) )
                     pstat = ch->level + 10;
                  else
                     pstat = 0;
                  break;

               case 'u':
                  pstat = num_descriptors;
                  break;

               case 'U':
                  pstat = sysdata.maxplayers;
                  break;

               case 'v':
                  pstat = ch->move;
                  break;

               case 'V':
                  pstat = ch->max_move;
                  break;

               case 'g':
                  pstat = ch->gold;
                  break;

               case 'r':
                  if( IS_IMMORTAL( och ) )
                     pstat = ch->in_room->vnum;
                  break;

               case 'F':
                  if( IS_IMMORTAL( och ) )
                     snprintf( pbuf, MAX_STRING_LENGTH, "%s", ext_flag_string( &ch->in_room->room_flags, r_flags ) );
                  break;

               case 'R':
                  if( xIS_SET( och->act, PLR_ROOMVNUM ) )
                     snprintf( pbuf, MAX_STRING_LENGTH, "<#%d> ", ch->in_room->vnum );
                  break;

               case 'D': /*display DND status*/
                  if( IS_IMMORTAL(ch) )
                  {
                     if( IS_SET( ch->pcdata->flags, PCFLAG_DND ) )
                        mudstrlcpy( pbuf, "DND", MAX_STRING_LENGTH );
                  }
                  break;

               case 'x':
                  pstat = ch->exp;
                  break;

               case 'X':
                  pstat = exp_level( ch, ch->level + 1 ) - ch->exp;
                  break;

               case 'w':
                  pstat = ch->carry_weight;
                  break;

               case 'W':
                  pstat = can_carry_w(ch);
                  break;

               case 'o':  /* display name of object on auction */
                  if( auction->item )
                     mudstrlcpy( pbuf, auction->item->name, MAX_STRING_LENGTH );
                  break;

               case 'S':
                  if( ch->style == STYLE_BERSERK )
                     mudstrlcpy( pbuf, "B", MAX_STRING_LENGTH );
                  else if( ch->style == STYLE_AGGRESSIVE )
                     mudstrlcpy( pbuf, "A", MAX_STRING_LENGTH );
                  else if( ch->style == STYLE_DEFENSIVE )
                     mudstrlcpy( pbuf, "D", MAX_STRING_LENGTH );
                  else if( ch->style == STYLE_EVASIVE )
                     mudstrlcpy( pbuf, "E", MAX_STRING_LENGTH );
                  else
                     mudstrlcpy( pbuf, "S", MAX_STRING_LENGTH );
                  break;

               case 'i':
                  if( ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_WIZINVIS ) ) ||
                      ( IS_NPC( ch ) && xIS_SET( ch->act, ACT_MOBINVIS ) ) )
                     snprintf( pbuf, MAX_STRING_LENGTH, "(Invis %d) ",
                               ( IS_NPC( ch ) ? ch->mobinvis : ch->pcdata->wizinvis ) );
                  else if( IS_AFFECTED( ch, AFF_INVISIBLE ) )
                     mudstrlcpy( pbuf, "(Invis) ", MAX_STRING_LENGTH );
                  break;

               case 'I':
                  pstat = ( IS_NPC( ch ) ? ( xIS_SET( ch->act, ACT_MOBINVIS ) ? ch->mobinvis : 0 )
                            : ( xIS_SET( ch->act, PLR_WIZINVIS ) ? ch->pcdata->wizinvis : 0 ) );
                  break;
            }
            if( pstat != 0x80000000 )
               snprintf( pbuf, MAX_STRING_LENGTH - strlen (buf), "%u", pstat );
            pbuf += strlen( pbuf );
            break;
      }
   }
   *pbuf = '\0';
   send_to_char( buf, ch );
   return;
}

void set_pager_input( DESCRIPTOR_DATA * d, char *argument )
{
   while( isspace( *argument ) )
      argument++;
   d->pagecmd = *argument;
   return;
}

bool pager_output( DESCRIPTOR_DATA * d )
{
   const char *last;
   CHAR_DATA *ch;
   int pclines;
   int lines;
   bool ret;

   if( !d || !d->pagepoint || d->pagecmd == -1 )
      return TRUE;
   ch = d->original ? d->original : d->character;
   pclines = UMAX( ch->pcdata->pagerlen, 5 ) - 1;
   switch ( LOWER( d->pagecmd ) )
   {
      default:
         lines = 0;
         break;
      case 'b':
         lines = -1 - ( pclines * 2 );
         break;
      case 'r':
         lines = -1 - pclines;
         break;
      case 'n':
         lines = 0;
         pclines = 0x7FFFFFFF;   /* As many lines as possible */
         break;
      case 'q':
         d->pagetop = 0;
         d->pagepoint = NULL;
         flush_buffer( d, TRUE );
         DISPOSE( d->pagebuf );
         d->pagesize = MAX_STRING_LENGTH;
         return TRUE;
   }
   while( lines < 0 && --d->pagepoint >= d->pagebuf )
      if( *d->pagepoint == '\n' )
         ++lines;
   if( d->pagepoint < d->pagebuf )
      d->pagepoint = d->pagebuf;
   if( *d->pagepoint == '\r' && *( ++d->pagepoint ) == '\n' )
      ++d->pagepoint;
   for( lines = 0, last = d->pagepoint; lines < pclines; ++last )
      if( !*last )
         break;
      else if( *last == '\n' )
         ++lines;
   if( *last == '\r' )
      ++last;
   if( last != d->pagepoint )
   {
      if( !write_to_descriptor( d, d->pagepoint, ( last - d->pagepoint ) ) )
         return FALSE;
      d->pagepoint = last;
   }
   while( isspace( *last ) )
      ++last;
   if( !*last )
   {
      d->pagetop = 0;
      d->pagepoint = NULL;
      flush_buffer( d, TRUE );
      DISPOSE( d->pagebuf );
      d->pagesize = MAX_STRING_LENGTH;
      return TRUE;
   }
   d->pagecmd = -1;
   if( xIS_SET( ch->act, PLR_ANSI ) )
      if( write_to_descriptor( d, ANSI_LBLUE, 0 ) == FALSE )
         return FALSE;
   if( ( ret = write_to_descriptor( d, "(C)ontinue, (N)on-stop, (R)efresh, (B)ack, (Q)uit: [C] ", 0 ) ) == FALSE )
      return FALSE;
   if( xIS_SET( ch->act, PLR_ANSI ) )
   {
      char buf[32];

      snprintf( buf, 32, "%s", color_str( d->pagecolor, ch ) );
      ret = write_to_descriptor( d, buf, 0 );
   }
   return ret;
}

#ifdef WIN32

void shutdown_mud( char *reason );

void bailout( void )
{
   echo_to_all( AT_IMMORT, "MUD shutting down by system operator NOW!!", ECHOTAR_ALL );
   shutdown_mud( "MUD shutdown by system operator" );
   log_string( "MUD shutdown by system operator" );
   Sleep( 5000 ); /* give "echo_to_all" time to display */
   mud_down = TRUE;  /* This will cause game_loop to exit */
   service_shut_down = TRUE;  /* This will cause characters to be saved */
   fflush( stderr );
   return;
}

#endif
