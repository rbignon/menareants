/* server/IA.cpp - Functions about Artificial Intelligence
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
 *
 * $Id$
 */

#include "IA.h"
#include "Commands.h"
#include "Main.h"
#include "Channels.h"
#include "Debug.h"
#include "Units.h"

/********************************************************************************************
 *                         METHODES D'INTELLIGENCE ARTIFICIELLE                             *
 ********************************************************************************************/
class TIA::UseTransportBoat : public TIA::Strategy
{
public:
	UseTransportBoat(TIA* i) : Strategy(i) {}
	virtual bool Exec()
	{
		ECBoat* boat = 0;
		ECEntity* unit = 0;
		std::vector<ECBEntity*> entities;

		entities = Entities()->List();
		for(std::vector<ECBEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
			if((*it)->IsInfantry())
			{
				unit = dynamic_cast<ECEntity*>(*it);
				break;
			}

		if(!unit)
			return false;

		if((entities = Entities()->Find(ECEntity::E_BOAT)).empty())
		{ /* On a pas de bateau encore d'assigné, on en cherche un */

			entities = IA()->Player()->Entities()->Find(ECEntity::E_BOAT);
			if(!entities.empty())
			{ /* On prend dans nos bateaux le plus proche */
				uint d = 0;
				for(std::vector<ECBEntity*>::iterator e = entities.begin(); e != entities.end(); ++e)
						if(!IA()->recruted[*e] && !dynamic_cast<ECBoat*>(*e)->Containing() &&
						   (!boat && unit->Case()->Delta((*e)->Case()) < 20 || d > unit->Case()->Delta((*e)->Case())))
							boat = dynamic_cast<ECBoat*>(*e),
							d = boat->Case()->Delta((*e)->Case());

				if(boat)
					AddEntity(boat);
				// Sinon, si jamais il n'y a que des bateaux recrutés ou qui contiennent déjà quelqu'un,
				// on en construit un autre
			}
			if(!boat)
			{ /* Le joueur n'a pas encore de bateau, on cherche un chantier naval pour en construire un */
				entities = IA()->Player()->Entities()->Find(ECEntity::E_SHIPYARD);
				uint d = 0;
				ECBEntity* shipyard = 0;
				for(std::vector<ECBEntity*>::iterator e = entities.begin(); e != entities.end(); ++e)
					if(unit->Case()->Delta((*e)->Case()) < 8 && (!shipyard || d > unit->Case()->Delta((*e)->Case())))
						shipyard = *e, d = unit->Case()->Delta((*e)->Case());
				if(!shipyard)
				{ /* Le joueur n'a pas de chantier naval, on en construit un */
					ECBCase* cc = 0;
					d = 0;
					BCountriesVector countries = IA()->Player()->MapPlayer()->Countries();
					for(BCountriesVector::iterator cty = countries.begin(); cty != countries.end(); ++cty)
					{
						std::vector<ECBCase*> cases = (*cty)->Cases();
						for(std::vector<ECBCase*>::iterator c = cases.begin(); c != cases.end(); ++c)
							if(((*c)->Flags() & C_MER) && (!cc || d > unit->Case()->Delta(*c)))
								cc = *c, d = unit->Case()->Delta(*c);
					}
					// Semblerait qu'il n'y ait pas de mer dans nos countries
					if(!cc || d > 8)
						{ FDebug(W_WARNING, "Pas de mer ou trop loins"); return false; }

					/* On créé un ShipYard, et aussi un bateau sur la même case dans le cas où le shipyard a
					 * bien été construit */
					IA()->ia_send(ECPacket(MSG_ARM, ECArgs("-", "=" + TypToStr(cc->X()) + "," + TypToStr(cc->Y()),
					                                       "+", "%" + TypToStr(ECEntity::E_SHIPYARD))));
					IA()->ia_send(ECPacket(MSG_ARM, ECArgs("-", "=" + TypToStr(cc->X()) + "," + TypToStr(cc->Y()),
					                                       "+", "%" + TypToStr(ECEntity::E_BOAT))));
					return true; /* On attend le prochain tour pour faire quelque chose avec */
				}
				else
				{ /* On construit un bateau à partir du chantier naval le plus proche */
					IA()->ia_send(ECPacket(MSG_ARM, ECArgs("-", "=" + TypToStr(shipyard->Case()->X()) + "," + TypToStr(shipyard->Case()->Y()),
					                                       "+", "%" + TypToStr(ECEntity::E_BOAT))));
					return true; /* On attends le prochain tour pour s'en servir */
				}
			}
		}
		else boat = dynamic_cast<ECBoat*>(entities.front());


		if(!boat)
			{ FDebug(W_WARNING, "Pas de bateau !?"); return false; }

		if(!boat->Containing())
		{ /* Le code quand le bateau et l'armée cherchent à se rapprocher */
			/* Si le bateau ne peut contenir l'unité, on triche discretement en ajoutant le nombre nécessaire
			 * au bateau pour avoir la capacité de contenir cette armée
			 */
			if(!boat->CanContain(unit))
				boat->SetNb(unit->Nb()/100 + 10);

			if(!boat->CanContain(unit))
			{
				if(boat->Containing())
					Debug(W_WARNING, "IA::UseTransportBoat: Le bateau contient déjà quelqu'un !?");
				else
					Debug(W_WARNING, "IA::UseTransportBoat: Le bateau a été augmenté à %d mais ne peut toujours pas contenir"
									" les %d hommes", boat->Nb(), unit->Nb());
				return false;
			}
			if(unit->DestCase()->Delta(boat->DestCase()) == 1)
				IA()->ia_send(ECPacket(MSG_ARM, ECArgs(unit->ID(), std::string(")") + boat->ID())));
			else
			{
				bool in_boat = false;
				for(uint i = 0; i < boat->MyStep(); ++i)
				{
					IA()->WantMoveTo(boat, unit->Case(), 1, true);
					if(unit->DestCase()->Delta(boat->DestCase()) == 1)
					{
						IA()->ia_send(ECPacket(MSG_ARM, ECArgs(unit->ID(), std::string(")") + boat->ID())));
						in_boat = true;
						break;
					}
				}
				if(!in_boat)
				{
					for(uint i = 0; i < unit->MyStep(); ++i)
					{
						IA()->WantMoveTo(unit, boat->DestCase(), 0, true);
						if(unit->DestCase()->Delta(boat->DestCase()) == 1)
						{
							IA()->ia_send(ECPacket(MSG_ARM, ECArgs(unit->ID(), std::string(")") + boat->ID())));
							break;
						}
					}
				}
			}
		}
		else
		{ /* Le code du bateau qui se déplace vers la cible */
			std::vector<ECBEntity*> all_entities = unit->Map()->Entities()->List();
			ECBEntity* victim = 0;
			uint d = 0;
			for(std::vector<ECBEntity*>::iterator e = all_entities.begin(); e != all_entities.end(); ++e)
				if(!(*e)->IsHidden() && !unit->Like(*e) &&
				   (unit->CanAttaq(*e) || unit->CanInvest(*e)) && !(*e)->IsTerrain() &&
				   (!victim || d > boat->Case()->Delta((*e)->Case())))
					victim = *e, d = boat->Case()->Delta((*e)->Case());

			if(!victim) return true;

			for(uint i = 0; i < boat->MyStep(); ++i)
			{
				if(boat->RestStep() > 0 && boat->DestCase() == boat->SearchProximCase(victim->Case()))
				{
					unit->SetRestStep(unit->MyStep());

					ECBMove::E_Move last_move = boat->Move()->Empty() ? (ECBMove::E_Move)-1 : boat->Move()->Moves().back();
					if(last_move != ECBMove::Right && boat->DestCase()->X()-1 >= victim->Case()->X() &&
					   !boat->WantMove(ECMove::Left, MOVE_SIMULE) &&
					   boat->DestCase()->X() > 0 && boat->DestCase()->MoveLeft()->Flags() & C_TERRE)
						IA()->ia_send(ECPacket(MSG_ARM, ECArgs(boat->ID(), "(" + TypToStr(boat->DestCase()->X()-1) + "," + TypToStr(boat->DestCase()->Y()))));
					else if(last_move != ECBMove::Left && boat->DestCase()->X()+1 <= victim->Case()->X() &&
					   !boat->WantMove(ECMove::Right, MOVE_SIMULE) &&
					   boat->DestCase()->X() < boat->Map()->Width()-1 && boat->DestCase()->MoveRight()->Flags() & C_TERRE)
						IA()->ia_send(ECPacket(MSG_ARM, ECArgs(boat->ID(), "(" + TypToStr(boat->DestCase()->X()+1) + "," + TypToStr(boat->DestCase()->Y()))));
					else if(last_move != ECBMove::Down && boat->DestCase()->Y()-1 >= victim->Case()->Y() &&
					   !boat->WantMove(ECMove::Up, MOVE_SIMULE) &&
					   boat->DestCase()->Y() > 0 && boat->DestCase()->MoveUp()->Flags() & C_TERRE)
						IA()->ia_send(ECPacket(MSG_ARM, ECArgs(boat->ID(), "(" + TypToStr(boat->DestCase()->X()) + "," + TypToStr(boat->DestCase()->Y()-1))));
					else if(last_move != ECBMove::Up && boat->DestCase()->Y()+1 <= victim->Case()->Y() &&
					   !boat->WantMove(ECMove::Down, MOVE_SIMULE) &&
					   boat->DestCase()->Y() < boat->Map()->Height()-1 && boat->DestCase()->MoveDown()->Flags() & C_TERRE)
						IA()->ia_send(ECPacket(MSG_ARM, ECArgs(boat->ID(), "(" + TypToStr(boat->DestCase()->X()) + "," + TypToStr(boat->DestCase()->Y()+1))));
					else
					{
						FDebug(W_WARNING, "On est bien sur la case d'arrivée du bateau mais on peut pas débarquer !");
						IA()->WantMoveTo(boat, victim->Case(), 1, true);
						continue;
					}

					/*  HACK On file une petite aide à l'IA pour qu'elle puisse avoir de l'avenir en terre conquise */
					if(unit->Type() == ECEntity::E_ARMY && unit->Nb() <= 1000)
						unit->SetNb(1000 + rand()%10 * 100);
					return false;
				}
				IA()->WantMoveTo(boat, victim->Case(), 1, true);
			}
		}

		return true;
	} /* Exec() */

};

#if 0
#define IA_DEBUG(x) (*Player()->Channel()) << (*enti)->LongName() + " " + x
#else
#define IA_DEBUG(x)
#endif
void TIA::WantMoveTo(ECBEntity* enti, ECBCase* dest, uint nb_cases, bool proxim)
{
	std::string msg;
	std::stack<ECBMove::E_Move> moves;
	if(!nb_cases || nb_cases > enti->RestStep())
		nb_cases = enti->RestStep();

	if(!enti->FindFastPath(dest, moves, enti->DestCase()) && (!proxim || !enti->FindFastPath(enti->SearchProximCase(dest), moves, enti->DestCase())))
	{
		if(enti->IsInfantry() && !recruted[enti])
			UseStrategy(new UseTransportBoat(this), enti);
		return;
	}

	ECArgs args;
	args += enti->ID();
	for(uint i = 0; moves.empty() == false && i < nb_cases; ++i)
	{
		ECBMove::E_Move m = moves.top();
		moves.pop();
		switch(m)
		{
			case ECBMove::Up: args += "^"; break;
			case ECBMove::Down: args += "v"; break;
			case ECBMove::Left: args += "<"; break;
			case ECBMove::Right: args += ">"; break;
		}
	}
	ia_send(ECPacket(MSG_ARM, args));
}

void TIA::FirstMovements()
{
	if(!Player() || Player()->Lost()) return;

	units.clear();

	std::vector<ECBEntity*> ents = Player()->Entities()->List();

	for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
		units[(*enti)->Type()]++;

	for(std::vector<Strategy*>::iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
		if(!(*strat)->Exec())
		{ /* Si il return false, c'est qu'il a envie de se faire supprimer */
			std::vector<Strategy*>::iterator s = strat - 1;
			RemoveStrategy(*strat, USE_DELETE);
			strat = s;
		}

	ents = Player()->Channel()->Map()->Entities()->List();
	for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
	{
		if((*enti)->IsZombie() || (*enti)->Locked() || !(*enti)->Owner() ||
		   !(*enti)->Owner()->IsAllie(Player()) && Player() != (*enti)->Owner())
			continue;
		if((*enti)->IsBuilding())
		{
			int t = 0;
			switch((*enti)->Type())
			{
				case ECEntity::E_CASERNE:
					switch(rand()%15)
					{
						case 0:
						case 1:
						case 2:
						case 3:
						case 4: t = ECEntity::E_ARMY; break;
						case 5: t = ECEntity::E_ENGINER; break;
						case 6: t = ECEntity::E_TOURIST; break;
						case 7: t = ECEntity::E_MCDO; break;
					}
					break;
				case ECEntity::E_CHARFACT:
					switch(rand()%7)
					{
						case 0:
						case 1:
						case 2:
						case 3:
						case 4:
						case 5: t = ECEntity::E_CHAR; break;
						case 6: t = ECEntity::E_MISSILAUNCHER; break;
					}
					break;
				default:
					t = rand()%ECEntity::E_END;
					break;
			}
			if(t)
			{
				ia_send(ECPacket(MSG_ARM, ECArgs("-", "=" + TypToStr((*enti)->Case()->X()) + "," + TypToStr((*enti)->Case()->Y()),
				                                 "+", "%" + TypToStr(t))));
				/* HACK pour que les armées ne soient pas de 100 */
				if(t == ECEntity::E_ARMY)
					for(int j = 12; j >= 0; --j)
						ia_send(ECPacket(MSG_ARM, ECArgs("-", "=" + TypToStr((*enti)->Case()->X()) + "," + TypToStr((*enti)->Case()->Y()),
				                                                 "+", "%" + TypToStr(t))));
			}
		}
		if((*enti)->Owner() != Player())
			continue;

		if((*enti)->MyUpgrade() != ECEntity::E_NONE && !(rand()%2))
			ia_send(ECPacket(MSG_ARM, ECArgs((*enti)->ID(), "U")));
		if((*enti)->MyStep() > 0 || (*enti)->Porty() > 0)
		{
			if((*enti)->AddUnits(0))
			{
				std::vector<ECBEntity*> es = (*enti)->Case()->Entities()->List();
				for(std::vector<ECBEntity*>::iterator e = es.begin(); e != es.end(); ++e)
					if((*e)->CanCreate(*enti))
						for(int i = 0; !i ;i = rand()% ((*enti)->Nb() < 1000 ? 2 : 3))
							ia_send(ECPacket(MSG_ARM, ECArgs((*enti)->ID(), "+")));
			}

			if(recruted[*enti]) continue; /* Il est recruté, donc tout ce qu'on peut lui faire faire là sera fait par la
			                               * fonction de l'action */

			std::vector<ECBEntity*> all_entities = (*enti)->Map()->Entities()->List();
			ECBEntity* victim = 0;
			uint d = 0;
			for(std::vector<ECBEntity*>::iterator e = all_entities.begin(); e != all_entities.end(); ++e)
				if(!(*e)->IsHidden() && !(*e)->IsTerrain() && !(*enti)->Like(*e) &&
				   ((*enti)->CanAttaq(*e) || (*enti)->CanInvest(*e) && !(*enti)->Deployed()) &&
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
					ia_send(ECPacket(MSG_ARM, ECArgs((*enti)->ID(), "#")));
				ia_send(ECPacket(MSG_ARM, ECArgs((*enti)->ID(), "*" + TypToStr(victim->Case()->X()) + "," +
				                                                      TypToStr(victim->Case()->Y()))));
				continue;
			}
			else if((*enti)->Deployed())
				ia_send(ECPacket(MSG_ARM, ECArgs((*enti)->ID(), "#")));

			if(!(*enti)->MyStep()) continue; /* Il n'y a pas d'attaque, donc on continue */

			WantMoveTo(*enti, victim->Case());
		}
	}

	ia_send(ECPacket(MSG_SET, "+!"));
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
		if(have_ia)
		{
			if(!Player()->IsAllie(*it))
				ia_send(ECPacket(MSG_SET, ECArgs("+a", (*it)->Nick())));
		}
		else if(Player()->IsAllie(*it))
			ia_send(ECPacket(MSG_SET, ECArgs("-a", (*it)->Nick())));
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

	std::vector<ECPacket> to_send;

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
						to_send.push_back(ECPacket(MSG_SET, ECArgs("-a", sender->Nick())));
					else if(add && !is_allie)
						to_send.push_back(ECPacket(MSG_SET, ECArgs("+a", sender->Nick())));
					break;
				}

				BPlayerVector pls = me->Player()->Allies();
				bool ia_allies = true;
				for(BPlayerVector::iterator pl = pls.begin(); pl != pls.end(); ++pl)
				{
					if(add && (*pl)->IsIA())
						to_send.push_back(ECPacket(MSG_SET, ECArgs("-a", (*pl)->Nick())));
					else if(!add && !(*pl)->IsIA() && *pl != sender) ia_allies = false;
				}
				if(!add && ia_allies)
					want_make_allies = true;
				else if(add)
				{
					pls = sender->Allies();
					FORit(ECBPlayer*, pls, pl)
						if(*pl != me->Player())
							to_send.push_back(ECPacket(MSG_SET, ECArgs("+a", (*pl)->Nick())));
				}

				to_send.push_back(ECPacket(MSG_SET, ECArgs(std::string(add ? "+" : "-") + "a", sender->Nick())));
				break;
			}
			case 'm':
				if(add && !me->Player()->Ready())
				    me->ia_send(ECPacket(MSG_SET, "+!"));
				break;
			case '!':
				/* Si on met -!, on remet l'IA prete */
				if(!add && !me->Player()->Ready() && !me->Player()->Lost() && me->Player()->Channel()->State() != EChannel::PLAYING)
				{
					for(std::vector<ECPlayer*>::iterator it=players.begin(); it != players.end(); ++it)
						if(*it == me->Player())
						{
							me->ia_send(ECPacket(MSG_SET, "+!"));
							break;
						}
				}

				break;
		}

	for(std::vector<ECPacket>::iterator it = to_send.begin(); it != to_send.end(); ++it)
		me->ia_send(*it);

	if(want_make_allies)
		me->MakeAllies();
	return 0;
}

/********************************************************************************************
 *                                FONCTIONS DE LA CLASSE TIA                                *
 ********************************************************************************************/

void TIA::RemoveEntity(ECBEntity* e)
{
	for (std::vector<Strategy*>::iterator it = strategies.begin(); it != strategies.end(); ++it)
		(*it)->RemoveEntity(e);
}

void TIA::UseStrategy(Strategy* s, ECBEntity* e)
{
	if(!s->Exec()) return;
	AddStrategy(s);
	s->AddEntity(e);
}

bool TIA::RemoveStrategy(Strategy* _s, bool use_delete)
{
	for (std::vector<Strategy*>::iterator it = strategies.begin(); it != strategies.end(); )
	{
		if (*it == _s)
		{
			if(use_delete)
				delete _s;
			it = strategies.erase(it);
			return true;
		}
		else
			++it;
	}
	return false;
}

bool TIA::Join(EChannel* chan)
{
	assert(chan);
	assert(!Player());

	if(!chan->Joinable() || chan->Limite() && chan->NbPlayers() >= chan->Limite())
	{
		app.delclient(this);
		return false;
	}

	SetPlayer(new ECPlayer(this, chan, false, false));
	chan->sendto_players(0, GetNick(), MSG_JOIN, chan->Name());

	/* Ça me l'envoie à moi même */
	sendrpl(ECPacket(app.GetConf()->ServerName(), MSG_SET, ECArgs(chan->ModesStr()).DontSplit()));

	return true;
}

void TIA::recv_msgs()
{
	for(std::vector<ECPacket>::iterator it = msgs.begin(); it != msgs.end();)
	{
		ECPacket s = *it;
		msgs.erase(it);
		recv_one_msg(s);
		it = msgs.begin();
	}
}

int TIA::ia_recv(const ECPacket& msg)
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

int TIA::recv_one_msg(const ECPacket& msg)
{
	static struct cmds_t {
		ECMessage cmd;
		int (*func) (std::vector<ECPlayer*> players, TIA *me, std::vector<std::string> parv);
		uint args;
	} cmds[] = {
		{MSG_SET,                TIA::SETCommand,             1},
		{MSG_LEAVE,              TIA::LEACommand,             0}
	};

	cmds_t* cmd = 0;
	for(uint i = 0; i < ASIZE(cmds) && !cmd; ++i)
		if(cmds[i].cmd == msg.Command())
			cmd = &cmds[i];

	/* Ce n'est pas forcément grave que la commande ne soit pas reconnue, dans le sens
	 * où on ne supporte pas tout ce qui est envoyé par le serveur (les joins etc on s'en
	 * fou par exemple)
	 */
	if(!cmd) return 0;
	if(msg.Args().Size() < cmd->args)
	    return Debug(W_WARNING, "TIA::ia_recv(): Réception d'un message anormal");

	std::vector<ECPlayer*> players;

	std::vector<std::string> parv, temp = msg.Args().List();
	parv.push_back(msg.From());
	parv.insert(parv.end(), temp.begin(), temp.end());

	if(Player() && !parv[0].empty())
	{
		std::string line = parv[0];
		while(!line.empty())
		{
			std::string tmp;
			tmp = stringtok(line, ",");
			if(tmp.find('!') != std::string::npos) tmp = stringtok(tmp, "!");
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
	if(!cl->Player() || cl->IsIA() || cl->Player()->Channel()->IsMission())
	{
		vDebug(cl->IsIA() ? W_WARNING : W_DESYNCH, "JIA: Appel incorrect", VPName(cl->Player()) VBName(cl->IsIA())
		                                           VName(cl->Player()->Channel()->Name()));
		return cl->sendrpl(ERR_UNKNOWN);
	}
	if(!cl->Player()->IsPriv())
	{
	    Debug(W_DESYNCH, "JIA: L'appelant n'est ni owner ni op");
	    return cl->sendrpl(ERR_UNKNOWN);
	}
	EChannel* chan = cl->Player()->Channel();

	if(!chan->Joinable())
	{
		vDebug(W_WARNING, "JIA: Le client essaye de faire joindre une IA dans un salon en jeu", VSName(chan->GetName())
		                  VSName(cl->GetNick()) VIName(chan->State()));
		return cl->sendrpl(ERR_UNKNOWN);
	}
	if(chan->NbPlayers() >= chan->Limite())
		return cl->sendrpl(ERR_IA_CANT_JOIN, parv[1]);

	for(std::string::iterator c = parv[1].begin(); c != parv[1].end(); ++c)
	{
		if(*c == ' ') *c = '_';
		else if(!strchr(NICK_CHARS, *c))
			return cl->sendrpl(ERR_IA_CANT_JOIN, parv[1]);
	}

	if(app.FindClient((IA_CHAR + parv[1]).c_str()))
		return cl->sendrpl(ERR_IA_CANT_JOIN, parv[1]);

	TIA* IA = dynamic_cast<TIA*>(app.addclient(-1, ""));
	IA->SetNick(IA_CHAR + parv[1]);
	SetAuth(IA);

	IA->Join(chan);

	/* A partir de là le serveur le gere comme un client normal */

	return 0;
}
