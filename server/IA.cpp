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
		{ /* On a pas de bateau encore d'assign�, on en cherche un */

			entities = IA()->Player()->Entities()->Find(ECEntity::E_BOAT);
			if(!entities.empty())
			{ /* On prend dans nos bateaux le plus proche */
				uint d = 0;
				for(std::vector<ECBEntity*>::iterator e = entities.begin(); e != entities.end(); ++e)
						if(!IA()->recruted[*e] && !dynamic_cast<ECBoat*>(*e)->Containing() &&
						   (!boat || d > unit->Case()->Delta((*e)->Case())))
							boat = dynamic_cast<ECBoat*>(*e),
							d = boat->Case()->Delta((*e)->Case());

				if(boat)
					AddEntity(boat);
				// Sinon, si jamais il n'y a que des bateaux recrut�s ou qui contiennent d�j� quelqu'un,
				// on en construit un autre
			}
			if(!boat)
			{ /* Le joueur n'a pas encore de bateau, on cherche un chantier naval pour en construire un */
				entities = IA()->Player()->Entities()->Find(ECEntity::E_SHIPYARD);
				ECBCase* cc = 0;
				uint d = 0;
				if(entities.empty())
				{ /* Le joueur n'a pas de chantier naval, on en construit un */
					BCountriesVector countries = IA()->Player()->MapPlayer()->Countries();
					for(BCountriesVector::iterator cty = countries.begin(); cty != countries.end(); ++cty)
					{
						std::vector<ECBCase*> cases = (*cty)->Cases();
						for(std::vector<ECBCase*>::iterator c = cases.begin(); c != cases.end(); ++c)
							if((*c)->Flags() & C_MER && (!cc || d > unit->Case()->Delta(cc)))
								cc = *c, d = unit->Case()->Delta(cc);
					}
					if(!cc)
						return false; // Semblerait qu'il n'y ait pas de mer dans nos countries

					/* On cr�� un ShipYard, et aussi un bateau sur la m�me case dans le cas o� le shipyard a
					 * bien �t� construit */
					IA()->ia_send("ARM - =" + TypToStr(cc->X()) + "," + TypToStr(cc->Y()) +
					              " + %" + TypToStr(ECEntity::E_SHIPYARD));
					IA()->ia_send("ARM - =" + TypToStr(cc->X()) + "," + TypToStr(cc->Y()) +
					              " + %" + TypToStr(ECEntity::E_BOAT));
					return true; /* On attend le prochain tour pour faire quelque chose avec */
				}
				else
				{ /* On construit un bateau � partir du chantier naval le plus proche */
					ECBEntity* shipyard = 0;
					uint d = 0;
					for(std::vector<ECBEntity*>::iterator e = entities.begin(); e != entities.end(); ++e)
						if(!shipyard || d > unit->Case()->Delta((*e)->Case()))
							shipyard = *e, d = unit->Case()->Delta((*e)->Case());

					if(!shipyard)
						return false; // Probl�me

					IA()->ia_send("ARM - =" + TypToStr(shipyard->Case()->X()) + "," + TypToStr(shipyard->Case()->Y()) +
					              " + %" + TypToStr(ECEntity::E_BOAT));
					return true; /* On attends le prochain tour pour s'en servir */
				}
			}
		}
		else boat = dynamic_cast<ECBoat*>(entities.front());


		if(!boat)
			return false; // probl�me

		/* Si le bateau ne peut contenir l'unit�, on triche discretement en ajoutant le nombre n�cessaire
		 * au bateau pour avoir la capacit� de contenir cette arm�e
		 */
		if(!boat->CanContain(unit))
			boat->SetNb(unit->Nb()/50 + 10);

		if(!boat->CanContain(unit))
		{
			Debug(W_WARNING, "IA::UseTransportBoat: Le bateau a �t� augment� � %d mais ne peut toujours pas contenir"
			                 " les %d hommes", boat->Nb(), unit->Nb());
			return false;
		}

		if(!boat->Containing())
		{ /* Le code quand le bateau et l'arm�e cherchent � se rapprocher */
			if(unit->Case()->Delta(boat->Case()) == 1)
				IA()->ia_send("ARM " + std::string(unit->ID()) + " )" + boat->ID());
			else
			{
				bool in_boat = false;
				for(uint i = 0; i < boat->MyStep(); ++i)
				{
					IA()->WantMoveTo(boat, unit->Case(), 1);
					if(unit->Case()->Delta(boat->Case()) == 1)
					{
						IA()->ia_send("ARM " + std::string(unit->ID()) + " )" + boat->ID());
						in_boat = true;
						break;
					}
				}
				if(!in_boat)
				{
					for(uint i = 0; i < unit->MyStep(); ++i)
					{
						IA()->WantMoveTo(unit, boat->Case());
						if(unit->Case()->Delta(boat->Case()) == 1)
						{
							IA()->ia_send("ARM " + std::string(unit->ID()) + " )" + boat->ID());
							break;
						}
					}
				}
			}
		}
		else
		{ /* Le code du bateau qui se d�place vers la cible */
			std::vector<ECBEntity*> all_entities = unit->Map()->Entities()->List();
			ECBEntity* victim = 0;
			uint d = 0;
			for(std::vector<ECBEntity*>::iterator e = all_entities.begin(); e != all_entities.end(); ++e)
				if(!(*e)->IsHidden() && !unit->Like(*e) &&
				   (unit->CanAttaq(*e) || unit->CanInvest(*e)) &&
				   (!victim || d > boat->Case()->Delta((*e)->Case())))
					victim = *e, d = boat->Case()->Delta((*e)->Case());

			if(!victim) return true;

			for(bool first = true; ; first = !first)
			{
				if(!first)
				{
					IA()->WantMoveTo(boat, victim->Case(), d < 8 ? boat->MyStep()-1 : 0);
				}
				if(boat->Case()->Delta(victim->Case()) < 6 && boat->RestStep() > 0)
				{
					unit->SetMyStep(unit->MyStep());
					ECBCase* last_case = boat->Last() ? boat->Last()->Case() : 0;
					if(boat->Case()->MoveLeft() != last_case && !boat->WantMove(ECMove::Left, MOVE_SIMULE) &&
					   boat->Case()->X() > 0 && boat->Case()->MoveLeft()->Flags() & C_TERRE)
						IA()->ia_send("ARM " + std::string(boat->ID()) +
									" (" + TypToStr(boat->Case()->X()-1) + "," + TypToStr(boat->Case()->Y()));
					else if(boat->Case()->MoveRight() != last_case && !boat->WantMove(ECMove::Right, MOVE_SIMULE) &&
					        boat->Case()->X() < boat->Map()->Width()-1 && boat->Case()->MoveRight()->Flags() & C_TERRE)
						IA()->ia_send("ARM " + std::string(boat->ID()) +
									" (" + TypToStr(boat->Case()->X()+1) + "," + TypToStr(boat->Case()->Y()));
					else if(boat->Case()->MoveUp() != last_case && !boat->WantMove(ECMove::Up, MOVE_SIMULE) &&
					        boat->Case()->Y() > 0 && boat->Case()->MoveUp()->Flags() & C_TERRE)
						IA()->ia_send("ARM " + std::string(boat->ID()) +
									" (" + TypToStr(boat->Case()->X()) + "," + TypToStr(boat->Case()->Y()-1));
					else if(boat->Case()->MoveDown() != last_case && !boat->WantMove(ECMove::Down, MOVE_SIMULE) &&
					        boat->Case()->Y() < boat->Map()->Height()-1 && boat->Case()->MoveDown()->Flags() & C_TERRE)
						IA()->ia_send("ARM " + std::string(boat->ID()) +
									" (" + TypToStr(boat->Case()->X()) + "," + TypToStr(boat->Case()->Y()+1));
					else
					{
						IA()->WantMoveTo(boat, victim->Case(), 1);
						if(!first)
							return true;
						else
							continue;
					}

					/* On file une petite aide � l'IA pour qu'elle puisse avoir de l'avenir en terre conquise */
					unit->SetNb(unit->Nb() + 1000);
					return false;
				}
				if(!first)
					break;
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
void TIA::WantMoveTo(ECBEntity* enti, ECBCase* dest, uint nb_cases)
{
	std::string msg;
	ECBCase* c = enti->Case();
	ECBCase* original_case = enti->Case();
	if(!nb_cases || nb_cases > enti->MyStep())
		nb_cases = enti->RestStep();
	for(uint k = 0; k < nb_cases && c != dest; k++)
	{
		int i = c->Y() != dest->Y() && c->X() != dest->X()
							? rand()%2
							: c->X() != dest->X()
							? 1
							: 0;

		if(i && c->X() < dest->X() && enti->WantMove(ECMove::Right, MOVE_SIMULE) &&
				(c->Y() == dest->Y() ||
					c->Y() < dest->Y() && enti->WantMove(ECMove::Down, MOVE_SIMULE) ||
					c->Y() > dest->Y() && enti->WantMove(ECMove::Up, MOVE_SIMULE)))
			msg += " >", c = c->MoveRight();
		else if(i && c->X() > dest->X() && enti->WantMove(ECMove::Left, MOVE_SIMULE) &&
				(c->Y() == dest->Y() ||
					c->Y() < dest->Y() && enti->WantMove(ECMove::Down, MOVE_SIMULE) ||
					c->Y() > dest->Y() && enti->WantMove(ECMove::Up, MOVE_SIMULE)))
			msg += " <", c = c->MoveLeft();
		else if(c->Y() < dest->Y() && enti->WantMove(ECMove::Down, MOVE_SIMULE) &&
				(c->X() == dest->X() ||
					c->X() < dest->X() && enti->WantMove(ECMove::Right, MOVE_SIMULE) ||
					c->X() > dest->X() && enti->WantMove(ECMove::Left, MOVE_SIMULE)))
			msg += " v", c = c->MoveDown();
		else if(c->Y() > dest->Y() && enti->WantMove(ECMove::Up, MOVE_SIMULE) &&
				(c->X() == dest->X() ||
					c->X() < dest->X() && enti->WantMove(ECMove::Right, MOVE_SIMULE) ||
					c->X() > dest->X() && enti->WantMove(ECMove::Left, MOVE_SIMULE)))
			msg += " ^", c = c->MoveUp();
		else
		{
			if(!enti->WantMove(ECMove::Right, MOVE_SIMULE) || !enti->WantMove(ECMove::Left, MOVE_SIMULE))
			{
				uint desesperate = 0;
				int dh = 0, db = 0;
				for(enti->SetCase(c);
						!(c->X() < dest->X() && enti->WantMove(ECMove::Right, MOVE_SIMULE)) &&
						!(c->X() > dest->X() && enti->WantMove(ECMove::Left, MOVE_SIMULE));
						enti->SetCase(enti->Case()->MoveUp()), dh++)
					if(!enti->WantMove(ECMove::Up, MOVE_SIMULE))
					{
						dh = -1;
						break;
					}
				for(enti->SetCase(c);
						!(c->X() < dest->X() && enti->WantMove(ECMove::Right, MOVE_SIMULE)) &&
						!(c->X() > dest->X() && enti->WantMove(ECMove::Left, MOVE_SIMULE));
						enti->SetCase(enti->Case()->MoveDown()), db++)
					if(!enti->WantMove(ECMove::Down, MOVE_SIMULE))
					{
						db = -1;
						break;
					}
				IA_DEBUG("^v : " + TypToStr(dh) + " and " + TypToStr(db) +
							" (" + TypToStr(c->Y()) + " > " + TypToStr(dest->Y()) + ")");
				desesperate = (db < 0 && dh < 0 && !recruted[enti])
								? 0
								: (db < 0 && !recruted[enti])
								? 2
								: (dh < 0 && !recruted[enti])
									? 1
									: (c->Y() > dest->Y())
										? 1
										: (c->Y() < dest->Y())
										? 2
										: (db < dh)
											? 1
											: 2;
				enti->SetCase(c);

				if(desesperate == 1 && enti->WantMove(ECMove::Down, MOVE_SIMULE))
					msg += " v", c = c->MoveDown();
				else if(desesperate == 2) msg += " ^", c = c->MoveUp();
				else if(desesperate == 0)
				{
					if(enti->IsInfantry())
						UseStrategy(new UseTransportBoat(this), enti);
					else if(enti->WantMove(ECMove::Right, MOVE_SIMULE)) msg += " >", c = c->MoveRight();
					else msg += " <", c = c->MoveLeft();
				}
			}
			else if(!enti->WantMove(ECMove::Up, MOVE_SIMULE) || !enti->WantMove(ECMove::Down, MOVE_SIMULE))
			{
				uint desesperate = 0;
				int dl = 0, dr = 0;
				for(enti->SetCase(c);
						!(c->Y() < dest->Y() && enti->WantMove(ECMove::Down, MOVE_SIMULE)) &&
						!(c->Y() > dest->Y() && enti->WantMove(ECMove::Up, MOVE_SIMULE));
						enti->SetCase(enti->Case()->MoveLeft()), dl++)
					if(!enti->WantMove(ECMove::Left, MOVE_SIMULE))
					{
						dl = -1;
						break;
					}
				for(enti->SetCase(c);
						!(c->Y() < dest->Y() && enti->WantMove(ECMove::Down, MOVE_SIMULE)) &&
						!(c->Y() > dest->Y() && enti->WantMove(ECMove::Up, MOVE_SIMULE));
						enti->SetCase(enti->Case()->MoveRight()), dr++)
					if(!enti->WantMove(ECMove::Right, MOVE_SIMULE))
					{
						dr = -1;
						break;
					}
				IA_DEBUG("<> : " + TypToStr(dl) + " and " + TypToStr(dr) +
							" (" + TypToStr(c->X()) + " > " + TypToStr(dest->X()) + ")");
				desesperate = (dl < 0 && dr < 0 && !recruted[enti])
								? 0
								: (dl < 0 && !recruted[enti])
								? 2
								: (dr < 0 && !recruted[enti])
									? 1
									: (c->X() > dest->X())
										? 1
										: (c->X() < dest->X())
										? 2
										: (dl < dr)
											? 1
											: 2;
				enti->SetCase(c);

				if(desesperate == 1 && enti->WantMove(ECMove::Left, MOVE_SIMULE))
					msg += " <", c = c->MoveLeft();
				else if(desesperate == 2) msg += " >", c = c->MoveRight();
				else if(desesperate == 0)
				{
					if(enti->IsInfantry())
						UseStrategy(new UseTransportBoat(this), enti);
					else if(enti->WantMove(ECMove::Up, MOVE_SIMULE)) msg += " ^", c = c->MoveUp();
					else msg += " v", c = c->MoveDown();
				}
			}
		}
		enti->SetCase(c);
	}
	enti->SetCase(original_case);
	if(!msg.empty())
		ia_send("ARM " + std::string(enti->ID()) + msg);
}

void TIA::FirstMovements()
{
	if(!Player()) return;

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
		if((*enti)->IsBuilding())
		{
			int i = rand()%ECBEntity::E_END;
			if(i)
				ia_send("ARM - =" + TypToStr((*enti)->Case()->X()) + "," + TypToStr((*enti)->Case()->Y()) + " + %" +
				                  TypToStr(i));
		}
		if((*enti)->MyUpgrade() != ECEntity::E_NONE && !(rand()%2))
			ia_send("ARM " + std::string((*enti)->ID()) + " �");
		if((*enti)->MyStep() > 0 || (*enti)->Porty() > 0)
		{
			if((*enti)->AddUnits(0))
			{
				std::vector<ECBEntity*> es = (*enti)->Case()->Entities()->List();
				for(std::vector<ECBEntity*>::iterator e = es.begin(); e != es.end(); ++e)
					if((*e)->CanCreate(*enti) && (*e)->Owner() == Player())
						for(int i = 0; !i ;i = rand()%3)
							ia_send("ARM " + std::string((*enti)->ID()) + " +");
			}

			if(recruted[*enti]) continue; /* Il est recrut�, donc tout ce qu'on peut lui faire faire l� sera fait par la
			                               * fonction de l'action */

			std::vector<ECBEntity*> all_entities = (*enti)->Map()->Entities()->List();
			ECBEntity* victim = 0;
			uint d = 0;
			for(std::vector<ECBEntity*>::iterator e = all_entities.begin(); e != all_entities.end(); ++e)
				if(!(*e)->IsHidden() && !(*enti)->Like(*e) &&
				   ((*enti)->CanAttaq(*e) || (*enti)->CanInvest(*e)) &&
				   (!victim || d > (*enti)->Case()->Delta((*e)->Case())) &&
				   ((*e)->Owner() != 0 || !(*e)->IsCity() || !(*enti)->Porty()))
					victim = *e, d = (*enti)->Case()->Delta((*e)->Case());

			if(!victim) continue; // �tonnant !

			if((*enti)->Case()->Delta(victim->Case()) <= (*enti)->Porty())
			{
				/* On sait pas si il peut se d�ployer ou non, alors on �crit �a car on sait jamais, et si il ne peut
				 * pas se d�ployer c'est pas g�nant.
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

			WantMoveTo(*enti, victim->Case());
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
	else
		me->CheckIfIReady();
	return 0;
}

void TIA::CheckIfIReady()
{
	if(!Player()->Ready() && Player()->Channel()->State() == EChannel::PLAYING)
	{
		BPlayerVector pls = Player()->Channel()->Players();
		uint counter = 0, humans = 0;
		for(BPlayerVector::const_iterator it = pls.begin(); it != pls.end(); ++it)
			if(!(*it)->IsIA())
			{
				humans++;
				if((*it)->Ready())
					counter++;
			}
		if(counter == humans)
			ia_send("SET +!");
	}
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
				if(!add) /* On vient pour la premi�re fois en Playing */
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
				else if(add)
					me->CheckIfIReady();

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

void TIA::RemoveEntity(ECBEntity* e)
{
	for (std::vector<Strategy*>::iterator it = strategies.begin(); it != strategies.end(); ++it)
		(*it)->Entities()->Remove(e);
}

void TIA::UseStrategy(Strategy* s, ECBEntity* e)
{
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

	if(!chan->Joinable() || chan->GetLimite() && chan->NbPlayers() >= chan->GetLimite())
	{
		app.delclient(this);
		return false;
	}

	SetPlayer(new ECPlayer(this, chan, false, false));
	chan->sendto_players(0, app.rpl(ECServer::JOIN), GetNick(), FormatStr(chan->Name()).c_str(), "");

	/* �a me l'envoie � moi m�me */
	sendrpl(app.rpl(ECServer::SET), app.GetConf()->ServerName().c_str(), chan->ModesStr().c_str());

	return true;
}

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
		Debug(W_ECHO, "S(%s@%s) - %s", this->GetNick(), this->GetIp(), msg.c_str());
#endif

	std::string cmdname;
	std::vector<std::string> parv;

	SplitBuf(msg, &parv, &cmdname);

	cmds_t* cmd = 0;
	for(uint i = 0; i < ASIZE(cmds) && !cmd; ++i)
		if(!strcmp(cmds[i].cmd, cmdname.c_str()))
			cmd = &cmds[i];

	/* Ce n'est pas forc�ment grave que la commande ne soit pas reconnue, dans le sens
	 * o� on ne supporte pas tout ce qui est envoy� par le serveur (les joins etc on s'en
	 * fou par exemple)
	 */
	if(!cmd) return 0;
	if((parv.size()-1) < cmd->args)
	    return vDebug(W_WARNING, "TIA::ia_recv(): R�ception d'un message anormal", VName(msg));

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
			/* Il est tout � fait possible que le player ne soit pas trouv�,
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

	IA->Join(chan);

	/* A partir de l� le serveur le gere comme un client normal */

	return 0;
}
