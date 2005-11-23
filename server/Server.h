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

class TClient
{
/* Constructeurs/Deconstructeurs */
public:

/* Methodes */
public:

	/* Parsage du dernier message */
	int parse_this();

	/* Envoyer un message formaté au client */
	int sendrpl(const char *pattern, ...);

	/* Envoie un message (sans formatage) */
	int sendbuf(const char* buf, int len);

	/* Fermer le client (message formaté) */
	int exit(const char *, ...);

	/* Met le client en etat "Libre" */
	void Free();

	/* Initialisation d'un client */
	void Init(int fd, const char *ip);

/* Attributs */
public:

	/* Obtient le pseudo */
	char* GetNick() { return nick; }

	/* Paramètre le nick */
	void SetNick(char* _nick) { strncpy(nick, _nick, NICKLEN); }

	/* Obtient l'ip */
	char* GetIp() { return ip; }

	/* Obtient les flags */
	unsigned int GetFlags() { return flag; }

	/* Paramètre un flag */
	void SetFlag(unsigned int f) { flag |= f; }

	/* Retire un flag */
	void DelFlag(unsigned int f) { flag &= ~f; }

	/* A le flag ? */
	bool HasFlag(unsigned int f) { return (flag & f); }

	/* Dernière lecture */
	time_t GetLastRead() { return lastread; }

	/* Obtient le sock */
	int GetFd() { return fd; }

	/* Obtient la sturcture player si il fait partit d'un jeu */
	ECPlayer *Player() { return pl; }

	void SetPlayer(ECPlayer *P) { if(!pl) pl = P; }

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
	int parsemsg();
};

#endif
