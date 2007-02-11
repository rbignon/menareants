/* src/Login.cpp - Login commands
 *
 * Copyright (C) 2005-2007 Romain Bignon  <Progs@headfucking.net>
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

#include "Channels.h"
#include "Commands.h"
#include "Server.h"
#include "Outils.h"
#include "Main.h"
#include "Debug.h"
#include <string>
#include <fstream>

/** Administration commands.
 *
 * Syntax: ADMIN REHASH
 *         ADMIN LOGIN [pass]
 *         ADMIN KILL [nick]
 */
int ADMINCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(parv[1] == "LOGIN")
	{
		if(parv.size() < 3 || parv[2].empty() || parv[2] == " " || parv[2] != app.GetConf()->AdminPass())
		{
			Debug(W_WARNING, "ADMIN LOGIN: Identification échouée de %s", cl->GetNick());
			return cl->sendrpl(app.rpl(ECServer::ERR));
		}
		cl->SetFlag(ECD_ADMIN);
		return cl->sendrpl(app.rpl(ECServer::ADMIN));
	}

	if(!cl->HasFlag(ECD_ADMIN))
	{
		Debug(W_WARNING, "ADMIN %s: Utilisation par un non admin %s", parv[1].c_str(), cl->GetNick());
		return cl->sendrpl(app.rpl(ECServer::ERR));
	}

	if(parv[1] == "REHASH")
	{
		if(!app.GetConf()->load())
		{
			Debug(W_WARNING, "ADMIN REHASH: Impossible de charger la configuration");
			return cl->sendrpl(app.rpl(ECServer::ERR));
		}
		else
			app.MSet("+PG", TypToStr(app.GetConf()->MaxConnexions()) + " " + TypToStr(app.GetConf()->MaxGames()));
		return 0;
	}

	if(parv[1] == "KILL")
	{
		TClient* victim = 0;
		if(parv.size() < 3 || !(victim = app.FindClient(parv[2].c_str())))
			return cl->sendrpl(app.rpl(ECServer::ERR));

		return victim->exit(app.rpl(ECServer::BYE));
	}

	cl->sendrpl(app.rpl(ECServer::ERR));

	return 0;
}

/** Send motd to client.
 *
 * @param cl client struct
 */
static void send_motd(TClient *cl)
{
	std::vector<std::string> motd = app.GetConf()->Motd();

	for(std::vector<std::string>::iterator it = motd.begin(); it != motd.end(); ++it)
		cl->sendrpl(app.rpl(ECServer::MOTD), FormatStr(*it).c_str());

	cl->sendrpl(app.rpl(ECServer::ENDOFMOTD));
	return;
}

/** Check if nickname is correct.
 *
 * @param nick original nickname.
 * @return new nickname (or NULL if nickname is absolutly incorrect)
 */
static char *correct_nick(const char *nick)
{
	static char newnick[NICKLEN + 1];
	uint i = 0;

	while(*nick && i < NICKLEN)
	{
		if(*nick == ' ') newnick[i++] = '_';
		else if(strchr(NICK_CHARS, *nick)) newnick[i++] = *nick;
		*nick++;
	}
	newnick[i] = '\0';
	return newnick;
}

static void send_stats(TClient *cl)
{
	cl->sendrpl(app.rpl(ECServer::STAT), app.NBco, app.NBtot, app.NBchan, app.NBwchan, app.NBachan, app.NBtotchan,
	                                     app.Uptime());
}

/** Client send me several informations about him.
 *
 * Syntax: IAM nick prog version
 */
int IAMCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(IsAuth(cl))
		return vDebug(W_DESYNCH, "IAM: User déjà logué", VSName(cl->GetNick()));

	/** \note il est normal de vérifier ici seulement le nombre d'arguments: ça permet un
	 *       meilleur controle par rapport à la version
	 */
	if(parv.size() < 4 || parv[2] != CLIENT_SMALLNAME)
		return cl->exit(app.rpl(ECServer::MAJ), '0');

	int pversion = 0;
	pversion = StrToTyp<int>(parv[3]);

	if(parv[3] != APP_PVERSION)
		return cl->exit(app.rpl(ECServer::MAJ), pversion < atol(APP_PVERSION) ? '-' : '+');

	char *nick = correct_nick(parv[1].c_str());

	if(*nick == '\0') return cl->exit(app.rpl(ECServer::ERR));

	if(app.FindClient(nick))
		return cl->exit(app.rpl(ECServer::USED));

	cl->SetNick(nick);
	SetAuth(cl);

	Debug(W_CONNS, ">> Connexion de %s@%s", cl->GetNick(), cl->GetIp());

	app.NBco++;
	app.NBtot++;
	app.MSet("+p " + TypToStr(app.NBco));

	cl->sendrpl(app.rpl(ECServer::AIM), cl->GetNick());

	send_motd(cl);
	send_stats(cl);

	for(ChannelVector::const_iterator chan = ChanList.begin(); chan != ChanList.end(); ++chan)
		if((*chan)->State() == EChannel::PINGING)
		{
			BPlayerVector pls = (*chan)->Players();
			for(BPlayerVector::const_iterator pl = pls.begin(); pl != pls.end(); ++pl)
				if((*pl)->CanRejoin() && (*pl)->Nick() == cl->Nick())
				{
					cl->sendrpl(app.rpl(ECServer::REJOIN), FormatStr((*chan)->Name()).c_str());
					break;
				}
		}

	return 0;
}

/** Request stats of server.
 *
 * Syntax: STAT
 */
int STATCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	send_stats(cl);
	return 0;
}

/** Received a PING message from client.
 *
 * Syntax: PIG
 */
int PIGCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	return cl->sendrpl(app.rpl(ECServer::PONG));
}

/** Received a PONG message from client.
 *
 * Syntax: POG
 */
int POGCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	DelPing(cl);

	return 0;

}
