/* src/Sockets.h- Header of Sockets.cpp
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

#ifndef EC_Sockets_h
#define EC_Sockets_h

#include <ClanLib/Network/socket.h>
#include <ClanLib/Core/System/clanstring.h>
#include "link.h"
#include "Main.h"
#include "Defines.h"

class EC_ACommand;

class EuroConqApp;

class EC_Client
{
public:
	EC_Client(const char *hostname, unsigned short port);

	~EC_Client();

public:

	enum msg {  /* mettre à jour systematiquement msgTab[] dans Sockets.cpp */
		IAM,  /* IAM */
		PONG, /* POG */

		NONE
	};

	void read_sock();
	void err_sock();
	int send(const char *pattern, ...);
	char *rpl(msg t);

	bool IsConnected() { return connected; }
	CL_String get_nick() { return nick; }

	EuroConqApp *app;

protected:
	CL_Socket *sock;

	CL_List<EC_ACommand> Commands;
	void parse_message(CL_String buf);

	bool connected;

	char readQ[MAXBUFFER + 1];
	unsigned int readQi;
	CL_String nick;
};

#endif
