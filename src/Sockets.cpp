/* src/Sockets.cpp - Gestion des sockets.
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

#include "Sockets.h"
#include "Menu.h"
#include "Main.h"
#include <arpa/inet.h>
#include <errno.h>

bool EuroConqApp::connect_to_server(bool entered)
{
	if(entered)
	{ /* IN */
		try
		{
			((EuroConqApp *)app)->client = new EC_Client(((EuroConqApp *)app)->conf->hostname, ((EuroConqApp *)app)->conf->port);
		}
		catch (CL_Error err)
		{
			Menu menu_err( "Impossible de se connecter :\n" + err.message, ((EuroConqApp *)app));
			menu_err.add_item("Retour", 0, 0);
			menu_err.execute();
			delete ((EuroConqApp *)app)->client;
			return false;
		}
	}
	else
	{ /* OUT */
		delete ((EuroConqApp *)app)->client;
	}

	return true;
}

EC_Client::EC_Client(const char *hostname, unsigned short port)
{
	connected = false;

	struct sockaddr_in fsocket = {0};
	int socki = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	fsocket.sin_family = AF_INET;
	fsocket.sin_addr.s_addr = inet_addr(hostname);
	fsocket.sin_port = htons(port);

	if(connect(socki, (struct sockaddr *) &fsocket, sizeof fsocket) < 0)
		throw CL_Error(strerror(errno));

	sock = new CL_Socket(socki);

	sock->sig_read_triggered().connect(this,&EC_Client::read_sock);
	//sock->sig_write_triggered().connect(this,&EC_Client::conn);
	sock->sig_exception_triggered().connect(this,&EC_Client::err_sock);
}

EC_Client::~EC_Client()
{
	delete sock;
}

void EC_Client::read_sock()
{
	char buf[128];
	memset((void*)&buf,0,127);

	sock->recv(buf,127);

	if(!connected) connected = true;

	std::cout << "Command Received on read: " << buf << std::endl;
	sock->sig_read_triggered().connect(this,&EC_Client::read_sock);
	//sock->sig_write_triggered().connect(this,&EC_Client::write_sock);
	sock->sig_exception_triggered().connect(this,&EC_Client::err_sock);
	//process_cmd(buf);
}

void EC_Client::err_sock()
{
	std::cout << "Erreur !" << std::endl;

	connected = false;
}
