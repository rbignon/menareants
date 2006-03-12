/* server/InGame.cpp - Commands called in a game.
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

#include "InGame.h"
#include "Channels.h"
#include "Debug.h"
#include "Outils.h"
#include "Commands.h"
#include "Units.h"

void EChannel::InitAnims()
{

}

#define SHOW_EVENT(x) ((x) == ARM_UNION ? "union" : (x) == ARM_MOVE ? "move" : (x) == ARM_ATTAQ ? "attaq" : (x) == ARM_CREATE ? "create" : (x) == ARM_NUMBER ? "number" : "no")
void EChannel::NextAnim()
{
	if(!map) throw ECExcept(VPName(map), "Pas de map");
	if(state != EChannel::ANIMING)
		throw ECExcept(VIName(state), "Appel hors du mode +A(niming)");

	if(dynamic_cast<ECMap*>(map)->Events().empty()) return;

	ECEvent* event = (*dynamic_cast<ECMap*>(map)->Events().begin());

	if(!event)
		throw ECExcept(VPName(event) VIName(dynamic_cast<ECMap*>(map)->Events().size()), "Evenement vide");

	switch(event->Flags())
	{
		case ARM_ATTAQ:
			Debug(W_DEBUG, "C'est une attaque");
			SendArm(NULL, event->Entities()->List(), ARM_ATTAQ, event->Case()->X(), event->Case()->Y());
			break;
		case ARM_UNION:
			SendArm(NULL, event->Entities()->List(), ARM_MOVE|ARM_REMOVE, event->Case()->X(), event->Case()->Y());
			break;
		case ARM_NUMBER:
			SendArm(NULL, event->Entities()->List(), ARM_NUMBER|ARM_HIDE, 0, 0, event->Nb());
			break;
		case ARM_CREATE:
			SendArm(NULL, event->Entities()->List(), ARM_CREATE|ARM_HIDE, event->Case()->X(), event->Case()->Y(),
			                                                                       event->Nb(), event->Type());
			break;
		case ARM_MOVE:
			SendArm(NULL, event->Entities()->List(), ARM_MOVE, event->Case()->X(), event->Case()->Y());
			break;
		default: break;
	}
	Debug(W_DEBUG, "event - %s %d,%d", SHOW_EVENT(event->Flags()), event->Case()->X(),
									event->Case()->Y());
	std::vector<ECEntity*> ents = event->Entities()->List();
	for(std::vector<ECEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
	{
		if(!(*enti)) Debug(W_DEBUG, "bizare, nul ? %p", *enti);
		else
			Debug(W_DEBUG, "      - Member %s!%s (%s)", (*enti)->Owner() ? (*enti)->Owner()->GetNick() : "*",
							(*enti)->ID(), SHOW_EVENT((*enti)->EventType()));
	}
	dynamic_cast<ECMap*>(map)->RemoveEvent(event, USE_DELETE);
}

template<typename T>
static ECEntity* CreateEntity(const Entity_ID _name, ECBPlayer* _owner, ECase* _case, uint _nb)
{
	return new T(_name, _owner, _case, _nb);
}

static struct
{
	ECEntity* (*func) (const Entity_ID _name, ECBPlayer* _owner, ECase* _case, uint _nb);
} entities_type[] = {
	/* E_ARMY */{ CreateEntity<ECArmy> },
	/* E_END  */{ NULL }
};


/** Modification of an army.
 *
 * Syntax: ARM nom [+nb] [%type] [/nb] [>pos] [*pos]
 */
int ARMCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(!cl->Player() || cl->Player()->Channel()->State() != EChannel::PLAYING)
		return vDebug(W_DESYNCH, "ARM: Le joueur n'est pas dans une partie, ou alors la partie n'est pas +P(laying)",
		              VPName(cl->Player()));

	if(cl->Player()->Ready())
		return Debug(W_DESYNCH, "ARM: Le joueur essaye de ARM alors qu'il est pret !");

	EChannel* chan = cl->Player()->Channel();
	ECMap *map = dynamic_cast<ECMap*>(chan->Map());
	ECEntity* entity = 0;
	uint flags = 0;

	if(parv[1] != "-") /* Si parv[1] == "-" alors on créé une nouvelle entity */
	{
		entity = dynamic_cast<ECEntity*>(cl->Player()->Entities()->Find(parv[1].c_str()));

		if(!entity)
			return vDebug(W_DESYNCH, "Entité introuvable", VPName(entity) VName(parv[1]));
	}

	uint y = 0, x = 0, type = 0, nb = 0;
	ECBCase* last_case = 0;
	for(uint i = 2; i<parv.size(); i++)
	{
		switch(parv[i][0])
		{
			case '>':
			{
				if(flags & ARM_ATTAQ || flags & ARM_MOVE)
					return vDebug(W_DESYNCH, "ARM: Utilisation simultanée de ATTAQ et MOVE", VHName(flags));

				std::string s = parv[i].substr(1);
				x = StrToTyp<uint>(stringtok(s, ","));
				y = StrToTyp<uint>(s);

				if(!entity || (last_case = entity->Move(x, y)))
					flags |= ARM_MOVE;
				break;
			}
			case '*':
			{
				if(flags & ARM_ATTAQ || flags & ARM_MOVE)
					return vDebug(W_DESYNCH, "ARM: Utilisation simultanée de ATTAQ et MOVE", VHName(flags));

				std::string s = parv[i].substr(1);
				x = StrToTyp<uint>(stringtok(s, ","));
				y = StrToTyp<uint>(s);

				if(entity && (last_case = entity->Attaq(x, y)))
					flags |= ARM_ATTAQ;
				break;
			}
			case '+':
			{
				if(!entity)
				{
					nb = StrToTyp<uint>(parv[i].substr(1));
					flags |= ARM_NUMBER;
				}
				break;
			}
			case '/':
			case '%':
			{
				type = StrToTyp<uint>(parv[i].substr(1));
				flags |= ARM_TYPE;
				break;
			}
			default: Debug(W_DESYNCH, "ARM: Flag %c non supporté (%s)", parv[i][0], parv[i].c_str());
		}
	}
	if(!entity)
	{ /* Création d'une entité */
		/******************************
		 *    GESTION DES CRÉATIONS   *
		 ******************************/
		if(flags != ARM_CREATE || type >= ECBEntity::E_END)
			return Debug(W_DESYNCH, "ARM: Création d'une entité incorrecte");

		const char *e_name = chan->FindEntityName(cl->Player());
		entity = entities_type[type].func(e_name, cl->Player(), (*(chan->Map()))(x,y), nb);
		if(!entity->CanBeCreated())
		{
			MyFree(entity);
			flags = 0;
		}
		else
		{
			/** \todo: mettre ici concernant l'argent */
			ECEvent* event = 0;

			if(entity->Case()->Entities()->Sames(entity))
			{ /* On peut probablement faire une union avec une entité de cette case donc bon au lieu de
			   * se broyer les testicule on va ARM_NUMBER simplement */
				EventVector evts = map->Events();
				bool event_found = false;
				for(EventVector::iterator evti = evts.begin(); evti != evts.end(); ++evti)
				{
					ECEntity* et = *((*evti)->Entities()->List().begin());
					if((*evti)->Case() == entity->Case() && (*evti)->Flags() == ARM_NUMBER &&
					   et->Owner() == entity->Owner() && et->Type() == entity->Type())
					{ /* On a trouvé un ARM_NUMBER qu'on investit */
						(*evti)->SetNb((*evti)->Nb() + entity->Nb());
						MyFree(entity);
						entity = et;
						entity->AddUnits(nb);
						event = *evti;
						Debug(W_DEBUG, "On a trouvé un ancien ARM_NUMBER qu'on investit");

						event_found = true;
						break;
					}
				}
				if(!event_found)
				{ /* On créé un nouvel evenement */
					event = new ECEvent(ARM_NUMBER, entity->Case());
					std::vector<ECBEntity*> ents = entity->Case()->Entities()->List();
					ECEntity* my_friend = 0;
					for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
						if(!(*enti)->Locked() && (*enti)->Owner() == entity->Owner() && (*enti)->Type() == entity->Type())
							my_friend = dynamic_cast<ECEntity*>(*enti);
					if(!my_friend)
						throw ECExcept(VIName(entity->Case()->Entities()->Sames(entity)) VIName(ents.size()),
									"Il y a comme un problème là");
	
					Debug(W_DEBUG, "On rajoute directement les unités dans cette armée.");
	
					my_friend->AddUnits(entity->Nb());
	
					MyFree(entity);
					entity = my_friend;
	
					event->SetNb(entity->Nb());
					event->Entities()->Add(entity);
					map->AddEvent(event);
				}
				flags = ARM_NUMBER;
			}
			else
			{
				Debug(W_DEBUG, "On rajoute un ARM_CREATE");
				event = new ECEvent(ARM_CREATE, entity->Case());
				event->Entities()->Add(entity);
				event->SetNb(entity->Nb());
				event->SetType(entity->Type());
				chan->Map()->AddAnEntity(entity);
				map->AddEvent(event);
			}
			nb = event->Nb();
			entity->SetEvent(flags);
		}
	}
	if(flags)
	{
		std::vector<TClient*> recvers;
		if(flags == ARM_ATTAQ || flags & ARM_MOVE)
		{
			/******************************
			 *  GESTION DES DÉPLACEMENTS  *
			 *    ET DES ATTAQUES         *
			 ******************************/
			entity->SetEvent(flags);

			EventVector evts = map->Events();
			bool event_found = false;
			for(EventVector::iterator evti = evts.begin(); evti != evts.end();)
			{
				Debug(W_DEBUG, "%p(%d,%d) vs %p(%d,%d)", (*evti)->Case(), (*evti)->Case()->X(), (*evti)->Case()->Y(),
				                                         entity->Case(), entity->Case()->X(), entity->Case()->Y());

				if((*evti)->Case() == entity->Case() && ((*evti)->Flags() == ARM_MOVE || (*evti)->Flags() == ARM_ATTAQ) ||
				                                         (*evti)->Flags() == ARM_UNION)
				{ /* Si il existe déjà un evenement sur cette case, on rajoute notre entité là. */
					bool attaq = true;
					if(flags & ARM_MOVE)
					{ /* On cherche si c'est pour scinder avec une autre armée ou si c'est une attaque */
						attaq = false;
						Debug(W_DEBUG, "Même case & MOVE, on cherche à voir si y a split ou attaque");
						std::vector<ECEntity*> ents = (*evti)->Entities()->List();
						for(ECList<ECEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
						{
							if((*enti)->Locked() || *enti == entity) continue;

							if((*enti)->Owner() == entity->Owner() && (*enti)->Type() == entity->Type())
							{ /* On va unir les deux armées */
								if((*evti)->Flags() == ARM_UNION)
								{ /* Cet evenement est une union donc on se propose de rajouter
								   * notre unité ici.
								   */
									(*enti)->SetEvent(flags == ARM_CREATE ? ARM_CREATE : ARM_UNION);
									(*evti)->Entities()->Add(entity);
									Debug(W_DEBUG, "On Union dans cet event");
								}
								else
								{ /* L'evenement actuel n'est pas une union, donc on se propose de créer un
								   * nouvel evenement rien que pour ça
								   */
									ECEvent* event = new ECEvent(ARM_UNION, entity->Case());
									event->Entities()->Add(entity);
									map->AddEvent(event);
									event_found = true;
									Debug(W_DEBUG, "On Union dans un nouvel event");
								}

								(*enti)->Union(entity); /* entity union dans *enti */
								event_found = true;
							}
							/* Si jamais une des deux entités peut attaquer l'autre, on dit que c'est bien
							 * une attaque
							 */
							else if(entity->CanAttaq(*enti) || (*enti)->CanAttaq(entity))
								attaq = true;
						}
					}
					if(attaq && !event_found && (*evti)->Flags() & (ARM_MOVE|ARM_ATTAQ))
					{ /* Ce mouvement se transforme en attaque */
						std::vector<ECEntity*> etlist = (*evti)->Entities()->List();
						for(ECList<ECEntity*>::iterator it = etlist.begin(); it != etlist.end(); ++it)
							if((*it) && !(*it)->Locked() && (*it)->Owner())
								recvers.push_back(dynamic_cast<ECPlayer*>((*it)->Owner())->Client());
						(*evti)->Entities()->Add(entity);
						(*evti)->SetFlags(ARM_ATTAQ);
						event_found = true;
						Debug(W_DEBUG, "on trouve un evenement en cette case, on attaque");
					}
				}
				if((*evti)->Case() == last_case)
				{
					/* On vérifie si, sur sa case d'origine, il n'y avait pas des attaques anticipées. Si tel
					 * est le cas, on informe aux attaquants que l'attaqué a bougé et on les remet donc au
					 * point d'origine...
					 */
					std::vector<ECEntity*> ents = (*evti)->Entities()->List();
					bool no_body = true, someone_want_attaq = false;
					for(std::vector<ECEntity*>::iterator enti = ents.begin(); enti != ents.end();)
						if((*enti)->EventType() & ARM_ATTAQ)
						{
							Debug(W_DEBUG, "il y a un attaquant qui se trouve fort sodomisé.");
							if((*enti)->Return())
								chan->SendArm(dynamic_cast<ECPlayer*>((*enti)->Owner())->Client(), *enti, ARM_RETURN);

							ECList<ECEntity*>::iterator it = enti;
							++it;
							(*evti)->Entities()->Remove(*enti);
							enti = it;
						}
						/* J'avais déjà fait un deplacement (ici donc), et là j'en refais un donc on m'enlève d'ici */
						else if(((*evti)->Flags() == ARM_ATTAQ || (*evti)->Flags() == ARM_MOVE) && *enti == entity)
						{
							Debug(W_DEBUG, "ça disparait !");
							ECList<ECEntity*>::iterator it = enti;
							++it;
							(*evti)->Entities()->Remove(*enti);
							enti = it;
						}
						else
						{
							if((*enti)->EventType()) no_body = false;
							if((*enti)->EventType() & ARM_ATTAQ) someone_want_attaq = true;
							++enti;
						}
					if(!someone_want_attaq && (*evti)->Flags() == ARM_ATTAQ)
						(*evti)->SetFlags(ARM_MOVE);
					if((*evti)->Entities()->empty() || no_body)
					{
						EventVector::iterator it = evti;
						++it;
						map->RemoveEvent(*evti, USE_DELETE);
						evti = it;
						Debug(W_DEBUG, "plus personne. (%d)", map->Events().size());
					}
					else ++evti;
				}
				else ++evti;
			}
			if(!event_found && flags != ARM_CREATE)
			{
				/* Aucun évenement n'a été trouvé, et donc on créé un. */
				ECEvent* event;

				if(!entity->Case()->Entities()->Sames(entity) && !entity->Case()->Entities()->Enemies(entity))
					event = new ECEvent(ARM_MOVE, entity->Case()),
					Debug(W_DEBUG, "On créé l'evenement MOVE (%d)", map->Events().size());
				else
				{
					Debug(W_DEBUG, "On créé l'evenement ATTAQ (%d)", map->Events().size());
					event = new ECEvent(ARM_ATTAQ, entity->Case());
					std::vector<ECBEntity*> ents = entity->Case()->Entities()->List();
					ECEntity* my_friend = 0;
					for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
					{
						if((*enti)->Locked() || *enti == entity) continue;
						if((*enti)->Owner() == entity->Owner() && (*enti)->Type() == entity->Type())
							// On va faire une union, pas besoin de le mettre dans l'evenement
							my_friend = dynamic_cast<ECEntity*>(*enti);
						else
						{
							if((*enti)->Owner())
								recvers.push_back(dynamic_cast<ECPlayer*>((*enti)->Owner())->Client());
							event->Entities()->Add(dynamic_cast<ECEntity*>(*enti));
						}
					}
					if(my_friend)
					{ /* Sur la case il y a des unités amies */
						event->SetFlags(flags == ARM_CREATE ? ARM_CREATE : ARM_UNION);
						Debug(W_DEBUG, "On Union dans cet event (%d)", map->Events().size());
						my_friend->Union(entity); /* entity s'unie dans my_friend */
					}
				}
				event->Entities()->Add(entity);
				map->AddEvent(event);
			}
		}

		recvers.push_back(cl);
		chan->SendArm(recvers, entity, flags, x, y, nb, type);
	}
	EventVector evts = map->Events();

	for(EventVector::iterator evti = evts.begin(); evti != evts.end(); ++evti)
	{
		Debug(W_DEBUG, "event - %s %d,%d", SHOW_EVENT((*evti)->Flags()), (*evti)->Case()->X(),
		                             (*evti)->Case()->Y());
		std::vector<ECEntity*> ents = (*evti)->Entities()->List();
		for(std::vector<ECEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
		{
			if(!(*enti)) Debug(W_DEBUG, "bizare, nul ? %p", *enti);
			else
			  Debug(W_DEBUG, "      - Member %s!%s (%s)", (*enti)->Owner() ? (*enti)->Owner()->GetNick() : "*",
			                  (*enti)->ID(), SHOW_EVENT((*enti)->EventType()));
		}
	}

	BCaseVector casv = map->Cases();
	for(BCaseVector::iterator casi = casv.begin(); casi != casv.end(); ++casi)
	{
		if((*casi)->Entities()->empty()) continue;
		std::vector<ECBEntity*> entv = (*casi)->Entities()->List();
		Debug(W_DEBUG, "%d,%d:", (*casi)->X(), (*casi)->Y());
		for(std::vector<ECBEntity*>::iterator enti = entv.begin(); enti != entv.end(); ++enti)
			Debug(W_DEBUG, "    [%c] %s!%s (%d)", (*enti)->Locked() ? '*' : ' ',
			                            (*enti)->Owner() ? (*enti)->Owner()->GetNick() : "*", (*enti)->ID(),
			                            (*enti)->Nb());
	}
	return 0;
}
