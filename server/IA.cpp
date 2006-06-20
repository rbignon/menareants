/* server/IA.cpp - Functions about Artificial Intelligence
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
 *
 * $Id$
 */

#include "IA.h"
#include "Commands.h"
#include "Main.h"
#include "Channels.h"
#include "Debug.h"

/********************************************************************************************
 *                         METHODES D'INTELLIGENCE ARTIFICIELLE                             *
 ********************************************************************************************/


void TIA::FirstMovements()
{
	std::vector<ECBEntity*> ents = Player()->Entities()->List();
	for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
	{
		std::string msg;
		if((*enti)->IsBuilding())
		{
			int i = rand()%ECBEntity::E_END;
			if(i)
				ia_send("ARM - =" + TypToStr((*enti)->Case()->X()) + "," + TypToStr((*enti)->Case()->Y()) + " + %" +
				                  TypToStr(i));
		}
		else
		{
			std::vector<ECBEntity*> es = (*enti)->Case()->Entities()->List();
			for(std::vector<ECBEntity*>::iterator e = es.begin(); e != es.end(); ++e)
			{
				if((*e)->CanCreate(*enti) && (*e)->Owner() == Player())
				{
					for(int i = rand()%2; !i ;i = rand()%3)
						ia_send("ARM " + std::string((*enti)->ID()) + " +");
				}
			}
			for(int k = 0; k < 2; k++)
			{
				int i = rand()%4;
				switch(i)
				{
					case 0: msg += " <"; break;
					case 1: msg += " >"; break;
					case 2: msg += " ^"; break;
					case 3: msg += " v"; break;
				}
			}
		}
		if(msg.empty()) continue;
		ia_send("ARM " + std::string((*enti)->ID()) + msg);
	}
}

/********************************************************************************************
 *                        COMMANDES RECUES PAR LES IA DU SERVEUR                            *
 ********************************************************************************************/

/** :nicks LEA */
int TIA::LEACommand (std::vector<ECPlayer*> players, TIA *me, std::vector<std::string> parv)
{
	for(std::vector<ECPlayer*>::iterator it = players.begin(); it != players.end(); ++it)
		if(*it == me->Player())
		{
			Debug(W_DEBUG, "%s s'en va", me->GetNick());
			app.delclient(me);
			return 0;
		}
	return 0;
}

/** :nicks SET modes [params] */
int TIA::SETCommand (std::vector<ECPlayer*> players, TIA *me, std::vector<std::string> parv)
{
	ECPlayer *sender = 0;
	if(players.size()) sender = players[0];

	bool add = true;

	for(uint i=0, j=2; i<parv[1].size(); i++)
		switch(parv[1][i])
		{
			case '+': add = true; break;
			case '-': add = false; break;
			case 'P':
				if(add)
					me->FirstMovements();
				break;
			case 'a':
			{
				if(strcasecmp(me->Player()->GetNick(), parv[j++].c_str())) break;

				me->ia_send("SET " + std::string(add ? "+" : "-") + "a " + sender->GetNick());
				break;
			}
			case 'm':
				if(add && !me->Player()->Ready())
				    me->ia_send("SET +!");
				break;
			case '!':
			    /* Si on met -!, on remet l'IA prete */
			    if(!add && !me->Player()->Ready() && me->Player()->Channel()->State() != EChannel::PLAYING)
			    {
					for(std::vector<ECPlayer*>::iterator it=players.begin(); it != players.end(); ++it)
					    if(*it == me->Player())
					    {
							me->ia_send("SET +!");
							break;
						}
				}
				else if(add && !me->Player()->Ready() && me->Player()->Channel()->State() == EChannel::PLAYING)
				{
					BPlayerVector pls = me->Player()->Channel()->Players();
					uint counter = 0, humans = 0;
					for(BPlayerVector::const_iterator it = pls.begin(); it != pls.end(); ++it)
						if(!(*it)->IsIA())
						{
							humans++;
							if((*it)->Ready())
								counter++;
						}
					if(counter == humans)
						me->ia_send("SET +!");
				}

				break;
		}
	return 0;
}

/********************************************************************************************
 *                                FONCTIONS DE LA CLASSE TIA                                *
 ********************************************************************************************/

void TIA::recv_msgs()
{
	for(std::vector<std::string>::iterator it = msgs.begin(); it != msgs.end();)
	{
		std::string s = *it;
		msgs.erase(it);
		recv_one_msg(s);
		it = msgs.begin();
	}
}

int TIA::ia_recv(std::string msg)
{
	if(Locked())
		msgs.push_back(msg);
	else
	{
		if(!msgs.empty())
			recv_msgs();
		return recv_one_msg(msg);
	}
	return 0;
}

int TIA::recv_one_msg(std::string msg)
{
	static struct cmds_t {
		char cmd[COMLEN + 1];
		int (*func) (std::vector<ECPlayer*> players, TIA *me, std::vector<std::string> parv);
		uint args;
	} cmds[] = {
		{"SET",                TIA::SETCommand,             1},
		{"LEA",                TIA::LEACommand,             0}
	};

	while(msg[msg.size()-1] == '\n' || msg[msg.size()-1] == '\r')
		msg = msg.substr(0, msg.size()-2);

#ifdef DEBUG
	if(strncmp(msg.c_str(), "PIG", 3) && strncmp(msg.c_str(), "POG", 3))
		printf("S(%s@%s) - %s\n", this->GetNick(), this->GetIp(), msg.c_str());
#endif

	std::string cmdname;
	std::vector<std::string> parv;

	SplitBuf(msg, &parv, &cmdname);

	cmds_t* cmd = 0;
	for(uint i = 0; i < ASIZE(cmds) && !cmd; ++i)
		if(!strcmp(cmds[i].cmd, cmdname.c_str()))
			cmd = &cmds[i];

	/* Ce n'est pas forcément grave que la commande ne soit pas reconnue, dans le sens
	 * où on ne supporte pas tout ce qui est envoyé par le serveur (les joins etc on s'en
	 * fou par exemple)
	 */
	if(!cmd) return 0;
	if((parv.size()-1) < cmd->args)
	    return vDebug(W_WARNING, "TIA::ia_recv(): Réception d'un message anormal", VName(msg));

	std::vector<ECPlayer*> players;
	if(Player() && !parv[0].empty())
	{
		std::string line = parv[0];
		while(!line.empty())
		{
			std::string tmp;
			tmp = stringtok(line, ",");
			if(tmp.find('!')) tmp = stringtok(tmp, "!");
			ECPlayer* tmpl = Player()->Channel()->GetPlayer(tmp.c_str());
			if(tmpl) players.push_back(tmpl);
			/* Il est tout à fait possible que le player ne soit pas trouvé,
			   genre si c'est un join... */
		}
	}
	
	try
	{
	    cmd->func(players, this, parv);
	}
	catch(TECExcept &e)
	{
        vDebug(W_ERR, e.Message(), e.Vars());
	}
	return 0;
}

/********************************************************************************************
 *                  COMMANDES RECUES PAR LE SERVEUR POUR LES IA                             *
 ********************************************************************************************/

/** Create an Artificial Intelligence instance
 *
 * Syntax: JIA nickname
 */
int JIACommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(!cl->Player() || cl->IsIA())
	{
	    vDebug(cl->IsIA() ? W_WARNING : W_DESYNCH, "JIA: Appel incorrect", VPName(cl->Player()) VBName(cl->IsIA()));
	    return cl->sendrpl(app.rpl(ECServer::ERR));
	}
	if(!cl->Player()->IsPriv())
	{
	    Debug(W_DESYNCH, "JIA: L'appelant n'est ni owner ni op");
	    return cl->sendrpl(app.rpl(ECServer::ERR));
	}
	EChannel* chan = cl->Player()->Channel();

	if(!chan->Joinable())
	{
		vDebug(W_WARNING, "JIA: Le client essaye de faire joindre une IA dans un salon en jeu", VSName(chan->GetName())
		                  VSName(cl->GetNick()) VIName(chan->State()));
		return cl->sendrpl(app.rpl(ECServer::ERR));
	}
	if(chan->GetLimite() && chan->NbPlayers() >= chan->GetLimite())
		return cl->sendrpl(app.rpl(ECServer::ERR));

	for(std::string::iterator c = parv[1].begin(); c != parv[1].end(); ++c)
	{
		if(!strchr(NICK_CHARS, *c))
			return cl->sendrpl(app.rpl(ECServer::ERR));
		else if(*c == ' ') *c = '_';
	}

	if(app.FindClient((IA_CHAR + parv[1]).c_str()))
		return cl->sendrpl(app.rpl(ECServer::IANICKUSED), parv[1].c_str());
		
	TIA* IA = dynamic_cast<TIA*>(app.addclient(-1, ""));
	IA->SetNick((IA_CHAR + parv[1]).c_str());
	SetAuth(IA);

	/* A partir de là le serveur le gere comme un client normal */
	IA->ia_send("JOI " + FormatStr(chan->GetName()));
	if(chan->Map())
	    IA->ia_send("SET +!");

	return 0;
}
