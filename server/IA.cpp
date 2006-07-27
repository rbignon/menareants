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
	if(!Player()) return;

	units.clear();

	std::vector<ECBEntity*> ents = Player()->Entities()->List();

	for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
		units[(*enti)->Type()]++;

	BCountriesVector countries = Player()->MapPlayer()->Countries();
	for(BCountriesVector::iterator cty = countries.begin(); cty != countries.end(); ++cty)
	{
		std::vector<ECBCase*> cases = (*cty)->Cases();
		for(std::vector<ECBCase*>::iterator c = cases.begin(); c != cases.end(); ++c)
		{
			int i = rand()%(ECBEntity::E_END);
			if(i && i < ECBEntity::E_END && units[i] < 2)
			{
				ia_send("ARM - =" + TypToStr((*c)->X()) + "," + TypToStr((*c)->Y()) + " + %" + TypToStr(i));
				break;
			}
		}
	}

	for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
	{
#if 0
#define IA_DEBUG(x) (*Player()->Channel()) << (*enti)->LongName() + " " + x
#else
#define IA_DEBUG(x)
#endif
		if((*enti)->IsBuilding())
		{
			int i = rand()%ECBEntity::E_END;
			if(i)
				ia_send("ARM - =" + TypToStr((*enti)->Case()->X()) + "," + TypToStr((*enti)->Case()->Y()) + " + %" +
				                  TypToStr(i));
		}
		if((*enti)->MyUpgrade() != ECEntity::E_NONE && !(rand()%2))
			ia_send("ARM " + std::string((*enti)->ID()) + " °");
		if((*enti)->MyStep() > 0 || (*enti)->Porty() > 0)
		{
			std::vector<ECBEntity*> es = (*enti)->Case()->Entities()->List();
			if((*enti)->AddUnits(0))
				for(std::vector<ECBEntity*>::iterator e = es.begin(); e != es.end(); ++e)
					if((*e)->CanCreate(*enti) && (*e)->Owner() == Player())
						for(int i = 0; !i ;i = rand()%3)
							ia_send("ARM " + std::string((*enti)->ID()) + " +");

			std::vector<ECBEntity*> all_entities = (*enti)->Map()->Entities()->List();
			ECBEntity* victim = 0;
			uint d = 0;
			for(std::vector<ECBEntity*>::iterator e = all_entities.begin(); e != all_entities.end(); ++e)
				if(!(*enti)->Like(*e) && ((*enti)->CanAttaq(*e) || (*enti)->CanInvest(*e)) &&
				   (!victim || d > (*enti)->Case()->Delta((*e)->Case())) &&
				   ((*e)->Owner() != 0 || !(*e)->IsCity() || !(*enti)->Porty()))
					victim = *e, d = (*enti)->Case()->Delta((*e)->Case());

			if(!victim) continue; // étonnant !

			if((*enti)->Case()->Delta(victim->Case()) <= (*enti)->Porty())
			{
				/* On sait pas si il peut se déployer ou non, alors on écrit ça car on sait jamais, et si il ne peut
				 * pas se déployer c'est pas génant.
				 */
				if(!(*enti)->Deployed())
					ia_send("ARM " + std::string((*enti)->ID()) + " #");
				ia_send("ARM " + std::string((*enti)->ID()) + " *" + TypToStr(victim->Case()->X()) + "," +
																		TypToStr(victim->Case()->Y()));
				continue;
			}
			else if((*enti)->Deployed())
				ia_send("ARM " + std::string((*enti)->ID()) + " #");

			if(!(*enti)->MyStep()) continue; /* Il n'y a pas d'attaque, donc on continue */

			std::string msg;
			ECBCase* c = (*enti)->Case();
			ECBCase* original_case = (*enti)->Case();
			for(uint k = 0; k < (*enti)->MyStep(); k++)
			{
				int i = c->Y() != victim->Case()->Y() && c->X() != victim->Case()->X()
									? rand()%2
									: c->X() != victim->Case()->X()
									? 1
									: 0;

				if(i && c->X() < victim->Case()->X() && (*enti)->WantMove(ECMove::Right, MOVE_SIMULE) &&
						(c->Y() == victim->Case()->Y() ||
							c->Y() < victim->Case()->Y() && (*enti)->WantMove(ECMove::Down, MOVE_SIMULE) ||
							c->Y() > victim->Case()->Y() && (*enti)->WantMove(ECMove::Up, MOVE_SIMULE)))
					msg += " >", c = c->MoveRight();
				else if(i && c->X() > victim->Case()->X() && (*enti)->WantMove(ECMove::Left, MOVE_SIMULE) &&
						(c->Y() == victim->Case()->Y() ||
							c->Y() < victim->Case()->Y() && (*enti)->WantMove(ECMove::Down, MOVE_SIMULE) ||
							c->Y() > victim->Case()->Y() && (*enti)->WantMove(ECMove::Up, MOVE_SIMULE)))
					msg += " <", c = c->MoveLeft();
				else if(c->Y() < victim->Case()->Y() && (*enti)->WantMove(ECMove::Down, MOVE_SIMULE) &&
						(c->X() == victim->Case()->X() ||
							c->X() < victim->Case()->X() && (*enti)->WantMove(ECMove::Right, MOVE_SIMULE) ||
							c->X() > victim->Case()->X() && (*enti)->WantMove(ECMove::Left, MOVE_SIMULE)))
					msg += " v", c = c->MoveDown();
				else if(c->Y() > victim->Case()->Y() && (*enti)->WantMove(ECMove::Up, MOVE_SIMULE) &&
						(c->X() == victim->Case()->X() ||
							c->X() < victim->Case()->X() && (*enti)->WantMove(ECMove::Right, MOVE_SIMULE) ||
							c->X() > victim->Case()->X() && (*enti)->WantMove(ECMove::Left, MOVE_SIMULE)))
					msg += " ^", c = c->MoveUp();
				else
				{
					if(c == victim->Case()) break;
					if(!(*enti)->WantMove(ECMove::Right, MOVE_SIMULE) || !(*enti)->WantMove(ECMove::Left, MOVE_SIMULE))
					{
						uint desesperate = 0;
						int dh = 0, db = 0;
						for((*enti)->SetCase(c);
								!(c->X() < victim->Case()->X() && (*enti)->WantMove(ECMove::Right, MOVE_SIMULE)) &&
								!(c->X() > victim->Case()->X() && (*enti)->WantMove(ECMove::Left, MOVE_SIMULE));
								(*enti)->SetCase((*enti)->Case()->MoveUp()), dh++)
							if(!(*enti)->WantMove(ECMove::Up, MOVE_SIMULE))
							{
								dh = -1;
								break;
							}
						for((*enti)->SetCase(c);
								!(c->X() < victim->Case()->X() && (*enti)->WantMove(ECMove::Right, MOVE_SIMULE)) &&
								!(c->X() > victim->Case()->X() && (*enti)->WantMove(ECMove::Left, MOVE_SIMULE));
								(*enti)->SetCase((*enti)->Case()->MoveDown()), db++)
							if(!(*enti)->WantMove(ECMove::Down, MOVE_SIMULE))
							{
								db = -1;
								break;
							}
						IA_DEBUG("^v : " + TypToStr(dh) + " and " + TypToStr(db) +
									" (" + TypToStr(c->Y()) + " > " + TypToStr(victim->Case()->Y()) + ")");
						desesperate = (db < 0 && dh < 0)
										? 0
										: (db < 0)
										? 2
										: (dh < 0)
											? 1
											: (c->Y() > victim->Case()->Y())
												? 1
												: (c->Y() < victim->Case()->Y())
												? 2
												: (db < dh)
													? 1
													: 2;
						(*enti)->SetCase(c);

						if(desesperate == 1 && (*enti)->WantMove(ECMove::Down, MOVE_SIMULE))
							msg += " v", c = c->MoveDown();
						else if(desesperate == 2) msg += " ^", c = c->MoveUp();
						else if(desesperate == 0)
						{
							if((*enti)->WantMove(ECMove::Right, MOVE_SIMULE)) msg += " >", c = c->MoveUp();
							else msg += " <", c = c->MoveRight();
						}
					}
					else if(!(*enti)->WantMove(ECMove::Up, MOVE_SIMULE) || !(*enti)->WantMove(ECMove::Down, MOVE_SIMULE))
					{
						uint desesperate = 0;
						int dl = 0, dr = 0;
						for((*enti)->SetCase(c);
								!(c->Y() < victim->Case()->Y() && (*enti)->WantMove(ECMove::Down, MOVE_SIMULE)) &&
								!(c->Y() > victim->Case()->Y() && (*enti)->WantMove(ECMove::Up, MOVE_SIMULE));
								(*enti)->SetCase((*enti)->Case()->MoveLeft()), dl++)
							if(!(*enti)->WantMove(ECMove::Left, MOVE_SIMULE))
							{
								dl = -1;
								break;
							}
						for((*enti)->SetCase(c);
								!(c->Y() < victim->Case()->Y() && (*enti)->WantMove(ECMove::Down, MOVE_SIMULE)) &&
								!(c->Y() > victim->Case()->Y() && (*enti)->WantMove(ECMove::Up, MOVE_SIMULE));
								(*enti)->SetCase((*enti)->Case()->MoveRight()), dr++)
							if(!(*enti)->WantMove(ECMove::Right, MOVE_SIMULE))
							{
								dr = -1;
								break;
							}
						IA_DEBUG("<> : " + TypToStr(dl) + " and " + TypToStr(dr) +
									" (" + TypToStr(c->X()) + " > " + TypToStr(victim->Case()->X()) + ")");
						desesperate = (dl < 0 && dr < 0)
										? 0
										: (dl < 0)
										? 2
										: (dr < 0)
											? 1
											: (c->X() > victim->Case()->X())
												? 1
												: (c->X() < victim->Case()->X())
												? 2
												: (dl < dr)
													? 1
													: 2;
						(*enti)->SetCase(c);

						if(desesperate == 1 && (*enti)->WantMove(ECMove::Left, MOVE_SIMULE))
							msg += " <", c = c->MoveLeft();
						else if(desesperate == 2) msg += " >", c = c->MoveRight();
						else if(desesperate == 0)
						{
							if((*enti)->WantMove(ECMove::Up, MOVE_SIMULE)) msg += " ^", c = c->MoveUp();
							else msg += " v", c = c->MoveRight();
						}
					}
				}
				(*enti)->SetCase(c);
			}
			(*enti)->SetCase(original_case);
			if(!msg.empty())
				ia_send("ARM " + std::string((*enti)->ID()) + msg);
		}
	}
}

/********************************************************************************************
 *                        COMMANDES RECUES PAR LES IA DU SERVEUR                            *
 ********************************************************************************************/

/** :nicks LEA */
int TIA::LEACommand (std::vector<ECPlayer*> players, TIA *me, std::vector<std::string> parv)
{
	if(parv[0] == me->Nick())
	{
		Debug(W_DEBUG, "%s s'en va", me->GetNick());
		app.delclient(me);
		return 0;
	}
	return 0;
}

void TIA::MakeAllies()
{
	//std::vector<std::string> to_send;

	std::vector<ECBPlayer*> pls = Player()->Channel()->Players();
	for(std::vector<ECBPlayer*>::iterator it = pls.begin(); it != pls.end(); ++it)
	{
		if(*it == Player() || !(*it)->IsIA()) continue;
		bool have_ia = true;
		BPlayerVector allies = (*it)->Allies();
		for(BPlayerVector::iterator pl = allies.begin(); pl != allies.end(); ++pl)
			if(!(*pl)->IsIA())
			{
				have_ia = false;
				break;
			}
		if(have_ia && !Player()->IsAllie(*it))
			ia_send("SET +a " + std::string((*it)->GetNick()));
	}

	/*for(std::vector<std::string>::iterator it = to_send.begin(); it != to_send.end(); ++it)
		ia_send(*it);*/
}

/** :nicks SET modes [params] */
int TIA::SETCommand (std::vector<ECPlayer*> players, TIA *me, std::vector<std::string> parv)
{
	ECPlayer *sender = 0;
	if(players.size()) sender = players[0];

	bool add = true, want_make_allies = false;

	std::vector<std::string> to_send;

	for(uint i=0, j=2; i<parv[1].size(); i++)
		switch(parv[1][i])
		{
			case '+': add = true; break;
			case '-': add = false; break;
			case 'S':
				if(!add) /* On vient pour la première fois en Playing */
				{
					/* On s'allie avec toutes les IA */
					want_make_allies = true;
				}
				break;
			case 'P':
				if(add)
					me->FirstMovements();
				break;
			case 'a':
			{
				if(sender == me->Player() || strcasecmp(me->Player()->GetNick(), parv[j++].c_str()))
					break;

				if(sender->IsIA())
				{
					bool is_allie = me->Player()->IsAllie(sender);
					if(!add && is_allie)
						to_send.push_back("SET -a " + std::string(sender->GetNick()));
					else if(add && !is_allie)
						to_send.push_back("SET +a " + std::string(sender->GetNick()));
					break;
				}

				BPlayerVector pls = me->Player()->Allies();
				bool ia_allies = true;
				for(BPlayerVector::iterator pl = pls.begin(); pl != pls.end(); ++pl)
				{
					if(add && (*pl)->IsIA())
						to_send.push_back("SET -a " + std::string((*pl)->GetNick()));
					else if(!add && !(*pl)->IsIA() && *pl != sender) ia_allies = false;
				}
				if(!add && ia_allies)
					want_make_allies = true;

				to_send.push_back("SET " + std::string(add ? "+" : "-") + "a " + sender->GetNick());
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

	for(std::vector<std::string>::iterator it = to_send.begin(); it != to_send.end(); ++it)
		me->ia_send(*it);

	if(want_make_allies)
		me->MakeAllies();
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
	IA->SetNick(IA_CHAR + parv[1]);
	SetAuth(IA);

	/* A partir de là le serveur le gere comme un client normal */
	IA->ia_send("JOI " + FormatStr(chan->GetName()));
	if(chan->Map())
		IA->ia_send("SET +!");

	return 0;
}
