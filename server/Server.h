/* server/Server.h - Header of Server.cpp
 *
 * Copyright (C) 2005-2006 Romain Bignon  <Progs@headfucking.net>
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
#include "lib/Messages.h"
#include <string>
#include <ctime>

/* - Client */
#define SetFlush(x)		(x)->SetFlag(ECD_FLUSH)
#define SetPing(x)		(x)->SetFlag(ECD_PING)
#define SetAuth(x)		(x)->SetFlag(ECD_AUTH)
#define DelFlush(x)		(x)->DelFlag(ECD_FLUSH)
#define DelPing(x)		(x)->DelFlag(ECD_PING)
#define IsFlush(x)		((x)->HasFlag(ECD_FLUSH))
#define IsPing(x)		((x)->HasFlag(ECD_PING))
#define IsAuth(x)		((x)->HasFlag(ECD_AUTH))

class ECPlayer;
class ECBPlayer;
class TIA;
class ECBEntity;
class ECEntity;

class TClient
{
/* Constructeur/Destructeur */
public:
    TClient(int _fd, const char* _ip)
        : nick("~"), lastread(0), fd(_fd), flag(0), ip(_ip), pl(0)
    {}

    virtual ~TClient() {}

/* Méthodes */
public:

	/** Send a reply to client.
	 * @return systematicaly 0.
	 */
	virtual int sendrpl(const ECPacket&) = 0;
	int sendrpl(const std::string& s, const ECMessage&, ECArgs = ECArgs());
	int sendrpl(const ECMessage&, ECArgs = ECArgs());
	int sendrpl(const std::vector<ECBPlayer*>&, const ECMessage&, ECArgs = ECArgs());
	int sendrpl(const std::vector<ECPlayer*>&, const ECMessage&, ECArgs = ECArgs());
	int sendrpl(const ECBPlayer*, const ECMessage&, ECArgs = ECArgs());
	int sendrpl(const std::vector<ECEntity*>&, const ECMessage&, ECArgs = ECArgs());
	virtual int sendrpl(const ECError&, ECArgs = ECArgs()) { return 0; }

	/** Close connexion with client and send a formated message. */
	virtual int exit(const ECMessage&, ECArgs = ECArgs());
	virtual int exit(const ECError&, ECArgs = ECArgs());

	int parsemsg(std::string buf);
	int parsemsg(const ECPacket&);
	int parsemsg(const ECMessage& cmdname, const std::vector<std::string>& parv);

	virtual void Free();

	virtual void RemoveEntity(ECBEntity* e) {}

/* Attributs */
public:

	/** Get nickname of client. */
	const char* GetNick() const { return nick.c_str(); }
	std::string Nick() const { return nick; }

	/** Set client's nickname. */
	void SetNick(std::string _nick) { nick = _nick; }

	/** Get IP. */
	const char* GetIp() const { return ip.c_str(); }

	/** Get flags. */
	unsigned int GetFlags() const { return flag; }

	/** Set a flag. */
	void SetFlag(unsigned int f) { flag |= f; }

	/** Remove a flag. */
	void DelFlag(unsigned int f) { flag &= ~f; }

	/** Check if client has \a f flag. */
	bool HasFlag(unsigned int f) const { return (flag & f); }

	/** Get last read time. */
	time_t GetLastRead() const { return lastread; }

	/** Get client's sock. */
	int GetFd() const { return fd; }

	/** Get player's struct if client is in a game. */
	ECPlayer *Player() const { return pl; }

	/** Set player struct */
	void SetPlayer(ECPlayer *P) { if(!pl) pl = P; }

	/** Remove player struct */
	void ClrPlayer() { pl = NULL; }

	bool IsHuman() const { return (fd >= 0); }
	bool IsIA() const { return (fd < 0); }

	void SetLastRead(time_t t) { lastread = t; }
	time_t LastRead() const { return lastread; }

	virtual void Lock() {}
	virtual void UnLock() {}
	virtual bool Locked() const { return false; }

	void SetCookie(std::string c) { cookie = c; }
	std::string Cookie() const { return cookie; }

/* Variables privées */
private:
	std::string nick;
	time_t lastread;
	int fd;
	unsigned int flag;	/* divers infos */
#define ECD_AUTH 	0x01
#define ECD_FREE 	0x02
#define ECD_FLUSH	0x04
#define ECD_PING	0x08
#define ECD_ADMIN	0x10
	std::string ip;
	ECPlayer *pl;
	std::string cookie;
};

/** TClient class.
 * This is class of a client.
 */
class TRealClient : public TClient
{
/* Constructeurs/Deconstructeurs */
public:

	TRealClient(int _fd, const char* _ip)
	    : TClient(_fd, _ip), recvlen(0), sendlen(0)
	{}

/* Methodes */
public:

	/** Parse a messages. */
	int parse_this();

	/** Send an unformated message. */
	int sendbuf(std::string);

	virtual int sendrpl(const ECError&, ECArgs = ECArgs());
	virtual int sendrpl(const ECPacket&);

	int flush();

	/** Close connexion with client and send a formated message. */
	int exit(const ECMessage&, ECArgs = ECArgs());
	int exit(const ECError&, ECArgs = ECArgs());

/* Attributs */
public:


/* Variables privées */
private:
    char RecvBuf[ECD_RECVSIZE+1];
    size_t recvlen;
    char SendBuf[ECD_SENDSIZE+1];
    size_t sendlen;
};

#endif
