/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc-mail.h: mailer declarations
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

#ifndef _IMC_MAIL_H
#define _IMC_MAIL_H

#include "imc.h"

/* Defs originally in imc-mail.c */

typedef struct _imc_mail
{
  char *from;			/* 'from' line */
  char *to;			/* 'to' line */
  char *text;			/* text of the mail */
  char *date;			/* 'date' line */
  char *subject;		/* 'subject' line */
  char *id;			/* mail ID (should be unique among mails) */
  time_t received;		/* when it was received (into the queue) */
  int usage;			/* number of references to this mail */
  struct _imc_mail *next;
} imc_mail;

extern imc_mail *imc_ml_head;

typedef struct _imc_qnode
{
  imc_mail *data;
  char *tomud;
  struct _imc_qnode *next;
} imc_qnode;

extern imc_qnode *imc_mq_head, *imc_mq_tail;

typedef struct _imc_mailid
{
  char *id;			/* message-id */
  time_t received;		/* when received */
  struct _imc_mailid *next;
} imc_mailid;

extern imc_mailid *imc_idlist;

void imc_recv_mailok(const char *from, const char *id);
void imc_recv_mailrej(const char *from, const char *id, const char *reason);
void imc_recv_mail(const char *from, const char *to, const char *date,
		   const char *subject, const char *id, const char *text);
void imc_send_mail(const char *from, const char *to, const char *date,
		   const char *subject, const char *text);

char *imc_mail_arrived(const char *from, const char *to, const char *date,
		       const char *subject, const char *text);

void imc_mail_startup(void);
void imc_mail_idle(void);
void imc_mail_shutdown(void);

char *imc_mail_showqueue(void);

#include "imc-comm.h"

#endif
