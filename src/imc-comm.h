/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc-comm.h: command interface definitions
 *
 * Copyright (C) 1996 Oliver Jowett <oliver@sa-search.massey.ac.nz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, write to the
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef IMC_COMM_H
#define IMC_COMM_H

#include "imc.h"

/** Semi-configurable bits **/

/* time between keepalive broadcasts (seconds) */
#define IMC_KEEPALIVE_TIME 300

/* time before dropping a mud off rlist (seconds) */
#define IMC_KEEPALIVE_TIMEOUT 350

/** End of configurable bits **/


#define IMC_IGNORE_EXACT  0    /* match exactly */
#define IMC_IGNORE_PREFIX 1    /* ignore prefixes when matching */
#define IMC_IGNORE_SUFFIX 2    /* ignore suffixes when matching */

#define IMC_IGNORE  1          /* ignore sender */
#define IMC_NOTRUST 2          /* don't trust sender's levels */

typedef struct _imc_ignore_data
{
  char *name;
  int type;                       /* exact, prefix, suffix */
  int flags;                      /* ignore, notrust */
  struct _imc_ignore_data *next;
} imc_ignore_data;

extern imc_ignore_data *imc_ignore_list;
extern char *imc_prefix;

#define IMC_INVIS 1
#define IMC_HIDDEN 2

typedef struct
{
  char name[IMC_NAME_LENGTH];	/* name of character */
  int invis;			/* invisibility state */
  int see;			/* invis-detection state */
  int level;			/* trust level */
  int wizi;			/* wizi level */
  int sex;			/* 0=male, 1=female, 2=other :) */
} imc_char_data;

/* Function prototypes declared/called */

void imc_send_chat(const imc_char_data *from, int channel,
		   const char *argument, const char *to);
void imc_send_emote(const imc_char_data *from, int channel,
		    const char *argument, const char *to);
void imc_send_tell(const imc_char_data *from, const char *to,
		   const char *argument, int isreply);
void imc_send_who(const imc_char_data *from, const char *to,
		  const char *type);
void imc_send_whoreply(const char *to, const char *data);
void imc_send_beep(const imc_char_data *from, const char *to);
void imc_send_keepalive(void);
void imc_send_ping(const char *to, int time_s, int time_u);
void imc_send_pingreply(const char *to, int time_s, int time_u);

void imc_recv_chat(const imc_char_data *from, int channel,
		   const char *argument);
void imc_recv_emote(const imc_char_data *from, int channel,
		    const char *argument);
void imc_recv_tell(const imc_char_data *from, const char *to,
		   const char *argument, int isreply);
void imc_recv_whoreply(const char *to, const char *data);
void imc_recv_who(const imc_char_data *from, const char *type);
void imc_recv_beep(const imc_char_data *from, const char *to);
void imc_recv_keepalive(const char *from, const char *version);
void imc_recv_ping(const char *from, int time_s, int time_u);
void imc_recv_pingreply(const char *from, int time_s, int time_u);

imc_ignore_data *imc_getignore(const char *who);
int imc_isignored(const char *who);
void imc_addignore(const char *who, int flags);
int imc_delignore(const char *who);
imc_ignore_data *imc_newignore(void);
void imc_freeignore(imc_ignore_data *ign);

const char *imc_ignore(const char *what);
const char *imc_sockets(void);
const char *imc_list(int level);
const char *imc_getstats(void);
int imc_command(const char *argument);

int imc_saveconfig(void);
int imc_readconfig(void);

int imc_saveignores(void);
int imc_readignores(void);

void imc_reloadconfig(void);

int imc_startup(const char *name, int port, const char *config);
void imc_idle(void);
void imc_shutdown(void);

#endif

