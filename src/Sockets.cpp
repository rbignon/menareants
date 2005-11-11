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

#include "Commands.h"
#include "Sockets.h"
#include "Menu.h"
#include "Main.h"
#include "array.h"
#include <arpa/inet.h>
#include <errno.h>
#include <cstdarg>

/* Messages à envoyer */
const char* msgTab[] = {
	"IAM %s " APP_SMALLNAME " " APP_PVERSION, /* IAM - Présentation */
	"POG",                                    /* POG - PONG */
     0
};

char *EC_Client::rpl(EC_Client::msg t)
{
  if(t<0 || t>EC_Client::NONE)
    //throw WRWExcept(VIName(t) VIName(EC_Client::NONE), "sort de la table", 0);
    throw CL_Error("Sort de la table"); /* TODO: utiliser une exception à nous */
  return (char *)msgTab[t];
}

int EC_Client::send(const char *pattern, ...)
{
	if(!sock) return -1;

	static char buf[MAXBUFFER + 20];
	va_list vl;
	int len;

	va_start(vl, pattern);
	len = vsnprintf(buf, sizeof buf -2, pattern, vl);
	if(len < 0) len = sizeof buf-2;

	buf[len++] = '\r';
	buf[len++] = '\n';
	buf[len] = 0;
	va_end(vl);

	sock->send(buf);
	std::cout << "S - " << buf << std::endl;

	return 0;
}

void EC_Client::parse_message(CL_String buf)
{
	char s[MAXBUFFER + 20];
	CL_String cmdname;
	int i, len = buf.get_length();

	cout << "R - " << buf << endl;

	CL_Array<CL_String> parv;
	for(i=0; i <= len; )
	{
		bool slash;
		int j;
		while(buf[i] == ' ') i++;
		for(slash=false,j=0; (i<=len && (buf[i] != ' ' || slash)); i++)
			if(buf[i] == '\\' && (buf[i+1] == ' ' || buf[i+1] == '\\') && !slash)
				slash = true;
			else
				s[j++]=buf[i], slash=false;
		s[j]='\0';
		if(s[0] != ':' || parv.get_num_items())
		{
			if(!parv.get_num_items())
				cmdname = s;
			parv.add(new CL_String(s));
		}
	}

	std::cout << "Com: " << cmdname << std::endl;
	for(i=0;i<parv.get_num_items();i++)
		std::cout << "parv[" << i << "]=" << *(parv[i]) << std::endl;

	EC_ACommand *cmd = NULL;
	for(i=0;i<Commands.get_num_items() && !cmd;i++)
		if(Commands[i]->CmdName == cmdname)
		{
			cmd = Commands[i];
			break;
		}

	if(!cmd || parv.get_num_items() < cmd->args)
		/* ATTENTION: la commande est inconnu ou donnée incorrectement, donc on exit
		 *            mais on ne quit pas (?). À voir, quand il y aurra un système
		 *            de debugage, si on le fait remarquer à l'user. Au pire je pense
		 *            qu'il faudrait au moins en informer le serveur. Ceci permettrait
		 *            de noter les bugs qu'il voit et les écrire dans un fichier.
		 */
		{ printf("erreur de pars ! %s\n", cmd ? "arg" : "cmd introuvable"); return; }

	try
	{
		cmd->Exec(this, parv);
	}
	catch(...)
	{
		std::cout << "Erreur du parsage !!" << std::endl;
	}

	return;
}

void EC_Client::read_sock()
{
	char buf[MAXBUFFER + 1];
	register char *ptr;
	int r;
	memset((void*)&buf,0,MAXBUFFER);

	r = sock->recv(buf,MAXBUFFER);

	if(!connected) connected = true;

	ptr = buf;
	buf[r] = 0;

	while(*ptr)
	{
		if(*ptr == '\n')
		{
			readQ[readQi-1] = 0; /* pour le \r devant le \n */
			parse_message(readQ);
			readQi = 0;
		}
		else readQ[readQi++] = *ptr;
		++ptr;
	}

	sock->sig_read_triggered().connect(this,&EC_Client::read_sock);
	sock->sig_write_triggered().connect(this,&EC_Client::read_sock);
	sock->sig_exception_triggered().connect(this,&EC_Client::err_sock);
}

void EC_Client::err_sock()
{
	std::cout << "Erreur !" << std::endl;

	connected = false;
}

bool EuroConqApp::connect_to_server(bool entered)
{
	if(entered)
	{ /* IN */
		try
		{
			((EuroConqApp *)app)->client = new EC_Client(((EuroConqApp *)app)->conf->hostname, ((EuroConqApp *)app)->conf->port);
			((EuroConqApp *)app)->client->app = ((EuroConqApp *)app);
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
	/* Initialisation des variables */
	connected = false;
	readQi = 0;

	/* Ajout des commandes */
	Commands.add(new HELCommand("HEL", 0,	1));
	Commands.add(new PIGCommand("PIG", 0,	0));

	/* Création du socket*/
	struct sockaddr_in fsocket = {0};
	int socki = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	fsocket.sin_family = AF_INET;
	fsocket.sin_addr.s_addr = inet_addr(hostname);
	fsocket.sin_port = htons(port);

	/* Connexion */
	if(connect(socki, (struct sockaddr *) &fsocket, sizeof fsocket) < 0)
		throw CL_Error(strerror(errno));

	/* Création d'un CL_Socket qui sert juste d'interface avec le socket existant */
	sock = new CL_Socket(socki);

	/* Timers */
	sock->sig_read_triggered().connect(this,&EC_Client::read_sock);
	//sock->sig_write_triggered().connect(this,&EC_Client::read_sock);
	sock->sig_exception_triggered().connect(this,&EC_Client::err_sock);
}

EC_Client::~EC_Client()
{
	delete sock;
	for(int i=0;i<Commands.get_num_items();i++)
		delete Commands[i];
	Commands.clear();
}
