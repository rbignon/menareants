/* src/Sockets.h- Header of Sockets.cpp
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

#ifndef EC_Sockets_h
#define EC_Sockets_h

#include "Main.h"
#include "Defines.h"
#include "Commands.h"

class EC_ACommand;

class EuroConqApp;

class ECPlayer;

class EC_Client
{
public:
	EC_Client(const char *hostname, unsigned short port);
	EC_Client();

	~EC_Client();

public:

	/** Messages à utiliser dans les reply
	 *
	 * \attention mettre à jour systematiquement msgTab[] dans Sockets.cpp
	 */
	enum msg {
		IAM,         /**< IAM */
		PONG,        /**< POG */
		BYE,         /**< BYE */
		ERROR,       /**< ERR */
		ERRORV,      /**< ERR */
		STAT,        /**< STAT */

		LISTGAME,    /**< LSP */
		JOIN,        /**< JOI */
		CREATE,      /**< JOI $ */
		LEAVE,       /**< LEA */
		MSG,         /**< MSG */
		SET,         /**< SET */

		NONE
	};

	static int read_sock(void *data);

	bool Connect(const char *hostname, unsigned short port);

	int sendrpl(const char *pattern, ...);
	char *rpl(msg t);

	bool IsConnected() const { return connected; }
	void SetConnected() { connected = true; }
	void SetWantDisconnect() { want_disconnect = true; }
	bool WantDisconnect() const { return want_disconnect; }
	std::string GetNick() const { return nick; }
	void set_nick(std::string _nick) { nick = _nick; }

	EuroConqApp *lapp;

	/* Obtient la sturcture player si il fait partit d'un jeu */
	ECPlayer *Player() { return pl; }
	void SetPlayer(ECPlayer *P) { if(!pl) pl = P; }
	void ClrPlayer() { pl = NULL; }

	std::string CantConnect() const { return cantconnect; }
	void SetCantConnect(std::string _msg) { cantconnect = _msg; }

protected:
	int sock;

	std::vector<EC_ACommand*> Commands;
	void parse_message(std::string buf);

	bool connected;
	bool want_disconnect;

	char readQ[MAXBUFFER + 1];
	unsigned int readQi;
	fd_set global_fd_set;

	std::string nick;

	std::string cantconnect;

	ECPlayer *pl;

private:
	void Init();
};

#endif /* EC_Sockets_h */
