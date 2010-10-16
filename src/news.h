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
**         Crimson Blade Codebase (CbC)             **
**     (c) 2000-2002 John Bellone (Noplex)          **
**           Coders: Noplex, Krowe                  **
**        http://www.crimsonblade.org               **
*****************************************************/

/*
 * File: news.h
 * Name: Extended News (v2.81)
 * Author: John 'Noplex' Bellone (john.bellone@flipsidesoftware.com)
 * Terms:
 * If this file is to be re-disributed; you must send an email
 * to the author. All headers above the #include calls must be
 * kept intact.
 * Description:
 * This is the extended news module; it allows for news to be
 * posted in note-like format; and bringing you into a editbuffer
 * instead of one-line posts (like Elder Chronicles). It also
 * allows support for online HTML output for news to be automatically
 * generated and included via a PHP; SSL; or a TXT include.
 */

#define NEWS_FILE "news.dat"
#define NEWS_INCLUDE_FILE "news.txt"
#define NEWS_TOP "\r\n"
#define NEWS_HEADER "\r\n"
#define NEWS_HEADER_ALL "&g( &W#&g)                          (&WSubject&g)\r\n"
#define NEWS_HEADER_READ "&g( &W#&g)                          (&WSubject&g)\r\n"
#define NEWS_VIEW               15
#define NEWS_MAX_TYPES          10

DECLARE_DO_FUN( do_editnews );

typedef struct news_data NEWS;
struct news_data
{
   NEWS *next;
   NEWS *prev;
   const char *title;
   const char *name;
   const char *post;
   const char *date;
   int number;
   int type;
};

typedef struct news_type NEWS_TYPE;
struct news_type
{
   NEWS *first_news;
   NEWS *last_news;
   NEWS_TYPE *next;
   NEWS_TYPE *prev;
   const char *header;
   const char *cmd_name;
   const char *name;
   int vnum;
   short level;
};

extern NEWS_TYPE *first_news_type;
extern NEWS_TYPE *last_news_type;

/* news.c */
NEWS *grab_news( NEWS_TYPE * type, const char *str );
NEWS_TYPE *figure_type( const char *str );
void display_news( CHAR_DATA * ch, NEWS * news, NEWS_TYPE * type );
void renumber_news( void );
void save_news( void );
void load_news( void );
void fread_news( NEWS * news, FILE * fp );
char *stamp_time( void );
void write_html_news( void );
void snarf_news( FILE * fpWrite );
void display_news_type( CHAR_DATA * ch, NEWS_TYPE * type, char *argument );
void fread_news_type( NEWS_TYPE * type, FILE * fp );
bool news_cmd_hook( CHAR_DATA * ch, char *cmd, char *argument );
void link_news_to_type( NEWS * news );
