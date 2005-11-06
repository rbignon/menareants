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

class EC_Client
{
public:
	EC_Client(const char *hostname, unsigned short port);

	~EC_Client();

public:

	bool connected;

	void conn();
	void read_sock();
	void write_sock();
	void err_sock();

protected:
	CL_Socket *sock;
};

#endif
