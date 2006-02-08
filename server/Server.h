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
#define SetFlush(x) 	(x)->SetFlag(ECD_FLUSH)
#define SetPing(x)		(x)->SetFlag(ECD_PING)
#define SetAuth(x)		(x)->SetFlag(ECD_AUTH)
#define DelFlush(x) 	(x)->DelFlag(ECD_FLUSH)
#define DelPing(x)		(x)->DelFlag(ECD_PING)
#define IsFlush(x)		((x)->HasFlag(ECD_FLUSH))
#define IsPing(x)		((x)->HasFlag(ECD_PING))
#define IsAuth(x)		((x)->HasFlag(ECD_AUTH))

class ECPlayer;

/** TClient class.
 * This is class of a client.
 */
class TClient
{
/* Constructeurs/Deconstructeurs */
public:

/* Methodes */
public:

	/** Parse a messages. */
	int parse_this();

	/** Send a reply to client.
	 * @return systematicaly 0.
	 */
	int sendrpl(const char *pattern, ...);

	/** Send an unformated message. */
	int sendbuf(const char* buf, int len);

	/** Close connexion with client and send a formated message. */
	int exit(const char *, ...);

	/** Free client class. */
	void Free();

	/** Initialization of client. */
	void Init(int fd, const char *ip);

/* Attributs */
public:

	/** Get nickname of client. */
	char* GetNick() { return nick; }

	/** Set client's nickname. */
	void SetNick(char* _nick) { strncpy(nick, _nick, NICKLEN); }

	/** Get IP. */
	char* GetIp() { return ip; }

	/** Get flags. */
	unsigned int GetFlags() { return flag; }

	/** Set a flag. */
	void SetFlag(unsigned int f) { flag |= f; }

	/** Remove a flag. */
	void DelFlag(unsigned int f) { flag &= ~f; }

	/** Check if client has \a f flag. */
	bool HasFlag(unsigned int f) { return (flag & f); }

	/** Get last read time. */
	time_t GetLastRead() { return lastread; }

	/** Get client's sock. */
	int GetFd() { return fd; }

	/** Get player's struct if client is in a game. */
	ECPlayer *Player() { return pl; }

	/** Set player struct */
	void SetPlayer(ECPlayer *P) { if(!pl) pl = P; }

	/** Remove player struct */
	void ClrPlayer() { pl = NULL; }

/* Variables et fonctions privées */
protected:
	char nick[NICKLEN + 1];
	size_t buflen;
	size_t recvlen;
	time_t lastread;
	char QBuf[ECD_SENDSIZE+1];
	char RecvBuf[ECD_RECVSIZE+1];
	int fd;
	unsigned int flag;	/* divers infos */
#define ECD_AUTH 	0x01
#define ECD_FREE 	0x02
#define ECD_FLUSH	0x04
#define ECD_PING	0x08
	char ip[16];
	ECPlayer *pl;

	inline int dequeue();

public:

	/** Parse a message to call a function's command. */
	int parsemsg();
};

#endif
