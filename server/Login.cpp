/* src/Login.cpp - Login commands
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
#include "Server.h"
#include "Outils.h"
#include "Main.h"
#include <string>

char *correct_nick(const char *nick)
{
	static char newnick[NICKLEN + 1];
	int i = 0;

	while(*nick && i < NICKLEN)
	{
		if(*nick == '@') newnick[i++] = 'a';
		else if(*nick == ' ') newnick[i++] = '_';
		else if(isalnum(*nick) || *nick == '-' || *nick == '_') newnick[i++] = *nick;
		*nick++;
	}
	newnick[i] = '\0';
	return newnick;
}

/* IAM <nick> <prog> <version> */
int IAMCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(IsAuth(cl)) return cl->exit(app.rpl(ECServer::ERR));
	if(parv[2] != CLIENT_SMALLNAME)
		return cl->exit(app.rpl(ECServer::MAJ), '0');

	int pversion = 0;
	StrToTyp(parv[3], pversion);

	if(parv[3] != APP_PVERSION)
		return cl->exit(app.rpl(ECServer::MAJ), pversion < atol(APP_PVERSION) ? '-' : '+');

	char *nick = correct_nick(parv[1].c_str());

	if(*nick == '\0') return cl->exit(app.rpl(ECServer::ERR));

	for(unsigned int i=0; i < app.GetHighSock(); i++)
		if(!strcasecmp(app.Clients[i].GetNick(), nick))
			return cl->exit(app.rpl(ECServer::USED));

	cl->SetNick(nick);
	SetAuth(cl);

	cl->sendrpl(app.rpl(ECServer::AIM), cl->GetNick());
	return 0;
}

/* PIG */
int PIGCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	return cl->sendrpl(app.rpl(ECServer::PONG));
}

/* POG */
int POGCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(IsPing(cl)) DelPing(cl);

	return 0;

}
