/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc-envy.h: Envy2 interface definitions
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

#ifndef IMC_SMAUG_H
#define IMC_SMAUG_H

/* Stick your mud ad in this #define :) */
#define IMC_MUD_INFO \
"Realms of Despair:\n\r" \
"  realms.game.org 4000\n\r" \
"  http://www.game.org\n\r"

/* uncomment this if your mud supports color */
/* #define IMC_COLOR */

/* #define this to whatever your color escapes start with */
#define IMC_COLORCHAR '{'

#define IMC_RCHAT   0x01
#define IMC_RIMM    0x02
#define IMC_RINFO   0x04
#define IMC_RCODE   0x08

#define IMC_RBEEP   0x2000
#define IMC_RTELL   0x4000

#define IMC_LEVEL_RTELL 5
#define IMC_LEVEL_RBEEP 20

#if 0 /* Envy2 does not have the equivalent of Wiznet */
#define WIZ_IMC       (Y)
#define WIZ_IMCDEBUG  (Z)
#endif

DECLARE_DO_FUN(do_rchat);
DECLARE_DO_FUN(do_rimm);
DECLARE_DO_FUN(do_rinfo);
DECLARE_DO_FUN(do_rcode);
DECLARE_DO_FUN(do_rtell);
DECLARE_DO_FUN(do_rreply);
DECLARE_DO_FUN(do_rwho);
DECLARE_DO_FUN(do_rquery);
DECLARE_DO_FUN(do_rbeep);

DECLARE_DO_FUN(do_imclist);
DECLARE_DO_FUN(do_rsockets);
DECLARE_DO_FUN(do_imc);
DECLARE_DO_FUN(do_rignore);
DECLARE_DO_FUN(do_rconnect);
DECLARE_DO_FUN(do_rdisconnect);

DECLARE_DO_FUN(do_istats);
DECLARE_DO_FUN(do_mailqueue);

DECLARE_DO_FUN(do_rchannels);
DECLARE_DO_FUN(do_rchanset);

/* for memory marking (leak checking in conjunction with SAM) */
void imc_markmemory(void (*markfn)(void *));

/* string checking (with SAM+hacks) */
void imc_markstrings(void (*markfn)(char *));

const char *color_itom(const char *);
const char *color_mtoi(const char *);

#endif
