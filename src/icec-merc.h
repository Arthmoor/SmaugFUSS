/*
 * IMC2 - an inter-mud communications protocol
 *
 * icec-merc.h: IMC-channel-extensions (ICE) client defs for Merc
 *
 * Copyright (C) 1997 Oliver Jowett <oliver@jowett.manawatu.planet.co.nz>
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

#ifndef ICEC_MERC_H
#define ICEC_MERC_H

DECLARE_DO_FUN(do_icecommand);
DECLARE_DO_FUN(do_iceconfig);
DECLARE_DO_FUN(do_icelist);
DECLARE_DO_FUN(do_icechannels);

bool icec_command_hook(CHAR_DATA *ch, const char *command, const char *argument);

#endif
