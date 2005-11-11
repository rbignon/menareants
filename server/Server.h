/* server/Server.h - Header of Server.cpp
 *
 * Copyright (C) 2005 Romain Bignon  <Progs@headfucking.net>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * $Id$
 */

#ifndef ECD_SERVER_H
#define ECD_SERVER_H

#include "Defines.h"
#include <string>

/* - Client */
#define SetFlush(x) 	(x)->flag |= ECD_FLUSH
#define SetPing(x)		(x)->flag |= ECD_PING
#define SetAuth(x)		(x)->flag |= ECD_AUTH
#define DelFlush(x) 	(x)->flag &= ~ECD_FLUSH
#define DelPing(x)		(x)->flag &= ~ECD_PING
#define IsFlush(x)		((x)->flag & ECD_FLUSH)
#define IsPing(x)		((x)->flag & ECD_PING)
#define IsAuth(x)		((x)->flag & ECD_AUTH)

class TClient
{
public:
	char nick[NICKLEN + 1];
	int fd;
	unsigned int flag;	/* divers infos */
#define ECD_AUTH 	0x01
#define ECD_FREE 	0x02
#define ECD_FLUSH	0x04
#define ECD_PING	0x08
	char ip[16];
	size_t buflen;
	size_t recvlen;
	time_t lastread;
	char QBuf[ECD_SENDSIZE+1];
	char RecvBuf[ECD_RECVSIZE+1];

public:
	int sendrpl(const char *pattern, ...);
	int exit(const char *, ...);
	inline int dequeue();
};

#endif
