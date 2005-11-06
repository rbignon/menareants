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
#include "Main.h"

bool EuroConqApp::connect_to_server()
{
	try
	{
		((EuroConqApp *)app)->client = new EC_Client(CL_String("127.0.0.1"), 6667);
	}
	catch (CL_Error err)
	{
		return false;
	}

	return true;
}

EC_Client::EC_Client(const char *hostname, unsigned short port)
{
	connected = false;
	CL_IPAddress ip;
	ip.set_port(port);
	ip.set_address(hostname);

	sock = new CL_Socket(CL_Socket::tcp);

	sock->connect(ip);
	sock->sig_read_triggered().connect(this,&EC_Client::conn);
	sock->sig_write_triggered().connect(this,&EC_Client::conn);
	sock->sig_exception_triggered().connect(this,&EC_Client::conn);
}

EC_Client::~EC_Client()
{
	delete sock;
}

void EC_Client::conn()
{
	std::cout << "Connected !" << std::endl;
	sock->sig_read_triggered().connect(this,&EC_Client::read_sock);
	sock->sig_write_triggered().connect(this,&EC_Client::write_sock);
	sock->sig_exception_triggered().connect(this,&EC_Client::err_sock);

	connected = true;
}

void EC_Client::write_sock()
{
	char buf[128];
	memset(buf,0,128);
	sock->recv(buf,127);
	std::cout << "Command Received on write: " << buf << std::endl;
	//process_cmd(buf);

	sock->sig_read_triggered().connect(this,&EC_Client::read_sock);
	sock->sig_write_triggered().connect(this,&EC_Client::write_sock);
	sock->sig_exception_triggered().connect(this,&EC_Client::err_sock);
}

void EC_Client::read_sock()
{
	char buf[128];
	memset((void*)&buf,0,127);
	sock->recv(buf,127);
	sock->sig_read_triggered().connect(this,&EC_Client::read_sock);
	sock->sig_write_triggered().connect(this,&EC_Client::write_sock);
	sock->sig_exception_triggered().connect(this,&EC_Client::err_sock);

	std::cout << "Command Received on read: " << buf << std::endl;
	//process_cmd(buf);
}

void EC_Client::err_sock()
{
	std::cout << "Erreur !" << std::endl;

	connected = false;
}
