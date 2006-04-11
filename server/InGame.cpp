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
	dynamic_cast<ECMap*>(map)->SortEvents();
}

#define SHOW_EVENT(x) ((x) == ARM_UNION ? "union" : (x) == ARM_MOVE ? "move" : (x) == ARM_ATTAQ ? "attaq" : (x) == ARM_CREATE ? "create" : (x) == ARM_NUMBER ? "number" : "no")
void EChannel::NextAnim()
{
	if(!map) throw ECExcept(VPName(map), "Pas de map");
	if(state != EChannel::ANIMING)
		throw ECExcept(VIName(state), "Appel hors du mode +A(niming)");

	if(dynamic_cast<ECMap*>(map)->Events().empty()) return;

	ECEvent* event = (*dynamic_cast<ECMap*>(map)->Events().begin());

	Debug(W_DEBUG, "event - %s %d,%d", SHOW_EVENT(event->Flags()), event->Case()->X(),
									event->Case()->Y());
	std::vector<ECEntity*> ents = event->Entities()->List();
	for(std::vector<ECEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
	{
		if(!(*enti)) Debug(W_DEBUG, "bizare, nul ? %p", *enti);
		else
			Debug(W_DEBUG, "      - Member %s (%s)", (*enti)->LongName().c_str(), SHOW_EVENT((*enti)->EventType()));
	}

	if(!event)
		throw ECExcept(VPName(event) VIName(dynamic_cast<ECMap*>(map)->Events().size()), "Evenement vide");

	switch(event->Flags())
	{
		case ARM_ATTAQ:
		{
			uint flags = ARM_ATTAQ;
			if(!event->Linked().empty())
				flags |= ARM_MOVE;
			SendArm(NULL, event->Entities()->List(), flags, event->Case()->X(), event->Case()->Y(), 0, 0, event->Linked());

			std::vector<ECEntity*> entv = event->Entities()->List();
			const char S_ATTAQ = 1;
			const char S_REMOVE = 2;
			const char S_WINERS = 3;
			const char S_END = 4;
			const char T_CONTINUE = 1;
			const char T_STOP = 0;
			char state = S_ATTAQ;
			for(std::vector<ECEntity*>::iterator it = entv.begin(); state != S_END;)
			{
				if(state == S_ATTAQ)
				{
					(*it)->Tag = (*it)->Attaq(entv) ? T_CONTINUE : T_STOP;
					++it;
				}
				else if(state == S_REMOVE || state == S_WINERS)
				{
					if(state == S_REMOVE)
						(*it)->ReleaseShoot(),
						Debug(W_DEBUG, "- %s reste %d", (*it)->LongName().c_str(), (*it)->Nb());

					if(!(*it)->Nb())
					{
						SendArm(NULL, *it, ARM_REMOVE);
						event->Entities()->Remove(*it);
						map->RemoveAnEntity(*it, USE_DELETE);
						it = entv.erase(it);
					}
					else if(state == S_WINERS || (*it)->Tag == T_STOP)
					{
						if(event->Case()->Entities()->Fixed() && (*it)->Last() &&
						   (*it)->Last()->Case() != event->Case())
						{
							/// \todo Gérer le déplacement quand il y a plusieurs entitées sur la case
						}
						SendArm(NULL, *it, ARM_NUMBER, (*it)->Case()->X(), (*it)->Case()->Y(), (*it)->Nb());

						(*it)->Case()->CheckChangingOwner(*it);

						it = entv.erase(it);
					}
					else ++it;
				}
				else ++it;

				if(it == entv.end())
				{
					it = entv.begin();
					if(state == S_REMOVE && !ECEntity::AreFriends(entv)) state = S_ATTAQ;
					else state++;
				}
			}
			break;
		}
		case ARM_UNION:
		{
			SendArm(NULL, event->Entity(), ARM_MOVE|ARM_REMOVE, event->Case()->X(), event->Case()->Y(), 0, 0,
			        event->Linked());
			std::vector<ECEntity*> entv = event->Entities()->List();
			if(entv.size() != 2)
				Debug(W_ERR, "Il faudraut qu'il y ait deux entités dans la liste de UNION et il y en a %d", entv.size());
			else
				SendArm(NULL, entv[1], ARM_NUMBER|ARM_HIDE, 0, 0, entv[1]->Nb());
			break;
		}
		case ARM_CREATE:
			SendArm(NULL, event->Entities()->List(), ARM_CREATE|ARM_HIDE|ARM_NOCONCERNED, event->Case()->X(),
			                                         event->Case()->Y(), event->Nb(), event->Type());
			break;
		case ARM_MOVE:
		{
			std::vector<ECEvent*> ev;
			ev.push_back(event);
			SendArm(NULL, event->Entities()->List(), ARM_MOVE, event->Case()->X(), event->Case()->Y(), 0, 0, ev);

			std::vector<ECEntity*> entv = event->Entities()->List();
			for(std::vector<ECEntity*>::iterator it = entv.begin(); it != entv.end(); ++it)
				(*it)->Case()->CheckChangingOwner(*it);
			break;
		}
		default: break;
	}
	dynamic_cast<ECMap*>(map)->RemoveEvent(event, USE_DELETE);
}

template<typename T>
static ECEntity* CreateEntity(const Entity_ID _name, ECBPlayer* _owner, ECase* _case)
{
	return new T(_name, _owner, _case);
}

static struct
{
	ECEntity* (*func) (const Entity_ID _name, ECBPlayer* _owner, ECase* _case);
} entities_type[] = {
	/* E_ARMY */{ CreateEntity<ECArmy> },
	/* E_END  */{ NULL }
};


/** Modification of an army.
 *
 * Syntax: ARM nom [+nb] [%type] [/nb] [=pos] [*pos] [>] [v] [>] [^]
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
	ECMove::Vector moves;
	ECase* last_case = (entity ? entity->Case() : 0);
	for(uint i = 2; i<parv.size(); i++)
	{
		switch(parv[i][0])
		{
			case '^':
				if(entity && entity->WantMove(ECMove::Up))
				{
					flags |= ARM_MOVE;
					moves.push_back(ECMove::Up);
				}
				break;
			case '>':
				if(entity && entity->WantMove(ECMove::Right))
				{
					flags |= ARM_MOVE;
					moves.push_back(ECMove::Right);
				}
				break;
			case 'v':
				if(entity && entity->WantMove(ECMove::Down))
				{
					flags |= ARM_MOVE;
					moves.push_back(ECMove::Down);
				}
				break;
			case '<':
				if(entity && entity->WantMove(ECMove::Left))
				{
					flags |= ARM_MOVE;
					moves.push_back(ECMove::Left);
				}
				break;
			case '=':
			{
				if(!moves.empty())
					return Debug(W_DESYNCH, "Utilisation de [v<^>] avant un =");
				if(entity)
					return Debug(W_DESYNCH, "Utilisation de =<pos> sur une entité existante");

				std::string s = parv[i].substr(1);
				x = StrToTyp<uint>(stringtok(s, ","));
				y = StrToTyp<uint>(s);

				if(!entity)
					flags |= ARM_MOVE;
				break;
			}
			case '*':
			{
				std::string s = parv[i].substr(1);
				x = StrToTyp<uint>(stringtok(s, ","));
				y = StrToTyp<uint>(s);

				if(entity && entity->WantAttaq(x, y))
					flags |= ARM_ATTAQ;
				break;
			}
			case '+':
			{
				flags |= ARM_NUMBER;
				break;
			}
			case '%':
			{
				if(is_num(parv[i].substr(1).c_str()))
				{
					type = StrToTyp<uint>(parv[i].substr(1));
					flags |= ARM_TYPE;
				}
				break;
			}
			case '/':
			default: Debug(W_DESYNCH, "ARM: Flag %c non supporté (%s)", parv[i][0], parv[i].c_str());
		}
	}
	if((flags & ARM_MOVE) && (flags & ARM_ATTAQ))
		return Debug(W_DESYNCH, "ARM: Essaye d'attaquer et de se déplacer");
	if(!entity)
	{
		/******************************
		 *    GESTION DES CRÉATIONS   *
		 ******************************/
		if(flags != ARM_CREATE || type >= ECBEntity::E_END)
			return Debug(W_DESYNCH, "ARM: Création d'une entité incorrecte");

		const char *e_name = chan->FindEntityName(cl->Player());
		entity = entities_type[type].func(e_name, cl->Player(), (*(chan->Map()))(x,y));
		if(!entity->CanBeCreated() || int(entity->Cost()) > cl->Player()->Money())
		{
			MyFree(entity);
			flags = 0;
		}
		else
		{
			if(entity->Case()->Entities()->Sames(entity))
			{ /* On peut probablement faire une union avec une entité de cette case donc bon au lieu de
			   * se broyer les testicule on va ARM_NUMBER simplement */
				std::vector<ECBEntity*> ents = entity->Case()->Entities()->List();
				ECEntity* my_friend = 0;
				for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
					if(!(*enti)->Locked() && (*enti)->Owner() == entity->Owner() && (*enti)->Type() == entity->Type())
						my_friend = dynamic_cast<ECEntity*>(*enti);
				if(!my_friend)
					throw ECExcept(VIName(entity->Case()->Entities()->Sames(entity)) VIName(ents.size()),
					               "Il y a comme un problème là");

				MyFree(entity);
				entity = my_friend;
				flags = ARM_NUMBER;
			}
			else
			{
				Debug(W_DEBUG, "On rajoute un ARM_CREATE");
				cl->Player()->DownMoney(entity->Cost());
				ECEvent* event = new ECEvent(ARM_CREATE, entity->Case());
				event->Entities()->Add(entity);
				event->SetNb(entity->Nb());
				event->SetType(entity->Type());
				chan->Map()->AddAnEntity(entity);
				map->AddEvent(event);
				nb = event->Nb();
				entity->SetEvent(flags);
			}
		}
	}
	if(flags)
	{
		std::nrvector<TClient*> recvers;
		std::vector<ECEvent*> events_sended;
		if(flags == ARM_NUMBER)
		{
			if(entity->CanBeCreated() && int(entity->Cost()) <= cl->Player()->Money())
			{
				/*****************************
				*     GESTION DES AJOUTS    *
				*****************************/
				entity->AddUnits(entity->InitNb());
	
				cl->Player()->DownMoney(entity->Cost());
				nb = entity->Nb();
			}
			else
				flags = 0;
		}
		if(flags == ARM_MOVE)
		{
			/*****************************
			 *  GESTION DES DÉPLACEMENTS *
			 *****************************/
			entity->SetEvent(flags);

			EventVector evts = map->Events();
			ECEvent* event_found = 0;
			ECEvent* attaq_event = 0;
			ECEvent* union_event = 0;
			for(EventVector::iterator evti = evts.begin(); evti != evts.end();)
			{
				bool delete_event = false;
				Debug(W_DEBUG, "%p(%d,%d) vs %p(%d,%d)", (*evti)->Case(), (*evti)->Case()->X(), (*evti)->Case()->Y(),
				                                         entity->Case(), entity->Case()->X(), entity->Case()->Y());
				/* Si evenement meme case, déplacement, entité c'est moi, et que c'est bien un déplacement continue.
				 * - On vérifie donc last_case, c'est à dire la case de départ, soit la case d'arrivée du mouvement.
				 * - On vérifie que c'est bien un movuement
				 * - On vérifie que c'est bien *mon* entité qui fait ce mouvement
				 * - On vérifie que l'entité "last" en position de la case d'origine n'a soit pas de last
				 *   (impossible ? mais on évite les segfaults), soit le last de ce last n'est pas sur la
				 *   meme case que ce last, donc que le déplacement est continue.
				 */
				ECEntity* last = 0;
				if((*evti)->Case() == last_case && (*evti)->Flags() == ARM_MOVE && (*evti)->Entity() == entity &&
				   (last = entity->FindLast(last_case)) && (!last->Last() || last->Last()->Case() != last->Case()))
				{ /* On a trouvé un déplacement pour cette même entité alors on en profite */
					if(event_found)
						Debug(W_WARNING, "ARM(MOVE).MyMoveFound: Il y avait déjà un event trouvé de MOVE pour cet entité");
					for(ECMove::Vector::const_iterator it = moves.begin(); it != moves.end(); ++it)
						(*evti)->Move()->AddMove(*it);
					event_found = *evti;
					event_found->SetCase(entity->Case());
					Debug(W_DEBUG, "On a trouvé un déplacement continue au mien");
				}

				if(entity->Case() == last_case)
				{
					++evti;
					continue;
				}

				/* On quitte l'ancienne case et on regarde si il y avait pas une attaque. Si c'est le cas,
				 * on supprime mon entité
				 */
				if((*evti)->Case() == entity->Last()->Case() && (*evti)->Flags() == ARM_ATTAQ &&
				   (*evti)->Entities()->Find(entity))
				{
					Debug(W_DEBUG, "Retrait d'attaquants");
					(*evti)->Entities()->Remove(entity);
					std::vector<ECEntity*> ents = (*evti)->Entities()->List();
					delete_event = true;
					const char T_ATTAQ_STEP = 0;
					const char T_MOVE_STEP = 1;
					const char T_END_STEP = 2;
					for(char step = T_ATTAQ_STEP; step != T_END_STEP; ++step)
						for(std::vector<ECEntity*>::iterator enti = ents.begin(); enti != ents.end();)
						{
							if((*enti)->Locked())
							{
								++enti;
								continue;
							}

							/* Pour cette entité, on cherche si il peut encore attaquer quelqu'un d'autre, sinon il jarte */
							std::vector<ECEntity*>::iterator enti2;
							for(enti2 = ents.begin();
							    enti2 != ents.end() && (*enti2 == *enti || (*enti2)->Locked() ||
							      (!(*enti)->CanAttaq(*enti2) || (*enti)->Like(*enti2)) &&
							      (step != T_MOVE_STEP || !(*enti2)->CanAttaq(*enti) || (*enti2)->Like(*enti)));
							    ++enti2);
							if(enti2 == ents.end())
							{
								if(step == T_ATTAQ_STEP && (*enti)->EventType() == ARM_ATTAQ)
								{ /* C'était un attaquant, on lui fait faire un Return */
									Debug(W_DEBUG, "il y a un attaquant qui se trouve fort sodomisé.");
									if((*enti)->Return() && (*enti)->Owner())
										chan->SendArm(dynamic_cast<ECPlayer*>((*enti)->Owner())->Client(), *enti,
										              ARM_RETURN, (*enti)->Case()->X(), (*enti)->Case()->Y());

									/* On supprime l'evenement de mouvement linké */
									std::vector<ECEvent*> evs = (*evti)->Linked();
									std::vector<ECEvent*>::iterator ev;
									for(ev = evs.begin(); ev != evs.end() && (*ev)->Entity() != *enti; ++ev);
									if(ev != evs.end())
										(*evti)->RemoveLinked(*ev, USE_DELETE);
								}
								else if(step == T_MOVE_STEP && (*enti)->EventType() == ARM_MOVE)
								{ /* C'était un mouvement innocent, on remet juste son evenement en lieu commun */
									std::vector<ECEvent*> evs = (*evti)->Linked();
									std::vector<ECEvent*>::iterator ev;
									for(ev = evs.begin(); ev != evs.end() && (*ev)->Entity() != *enti; ++ev);
									if(ev != evs.end())
									{
										(*evti)->RemoveLinked(*ev);
										map->AddEvent(*ev);
									}
								}
								else
								{
									++enti;
									continue;
								}
								(*evti)->Entities()->Remove(*enti);
								enti = ents.erase(enti);
							}
							else
							{
								/* Cet evenement réuni encore des unités qui peuvent se battre */
								delete_event = false;
								++enti;
							}
						}
				}

				/* On regarde si, sur la nouvelle case, il y a un evenement d'attaque.
				 * Note: il n'y a pas besoin de vérifier tout le parcourt, en effet, c'est à chaque
				 *       appel de ECEntity::WantMove() qu'il est censé le faire
				 */
				if((*evti)->Case() == entity->Case() && (*evti)->Flags() == ARM_ATTAQ && !union_event && !attaq_event)
				{
					std::vector<ECEntity*> ev_entities = (*evti)->Entities()->List();
					std::vector<ECEntity*>::iterator et;
					for(et = ev_entities.begin(); et != ev_entities.end() &&
					    (!entity->CanAttaq(*et) || entity->Like(*et) && (!(*et)->CanAttaq(entity) || (*et)->Like(entity)));
					    ++et);
					if(et != ev_entities.end())
					{
						if(attaq_event)
							Debug(W_WARNING, "ARM(MOVE).ATTAQ_FOUND: On a déjà trouvé un event d'attaque sur cette case.");
						attaq_event = *evti;
						Debug(W_DEBUG, "On a trouvé un evenement d'attaque");
					}
				}
				if(delete_event)
				{
					map->RemoveEvent(*evti, USE_DELETE);
					evti = evts.erase(evti);
				}
				else
					++evti;
			}
			if(entity->Case() != last_case)
			{ /* On ne fait ça que si y a eu un changement de case */
				if(!event_found)
				{
					Debug(W_DEBUG, "On créé un nouvel event MOVE");
					event_found = new ECEvent(ARM_MOVE, entity->Case());
					event_found->Entities()->Add(entity);
					event_found->Move()->SetMoves(moves);
					event_found->Move()->SetEntity(entity);
					event_found->Move()->SetFirstCase(last_case);
					map->AddEvent(event_found);
				}

				/* On regarde sur la case si il y a des entités à attaquer, ou à union */
				std::vector<ECBEntity*> ents = entity->Case()->Entities()->List();
				if(!attaq_event && !union_event)
					for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
					{
						if((*enti)->Locked() || *enti == entity) continue;

						if(entity->CanAttaq(*enti) && !entity->Like(*enti) ||
						(*enti)->CanAttaq(entity) && !(*enti)->Like(entity))
						{
							if(!attaq_event)
							{
								Debug(W_DEBUG, "On créé un evenement attaque");
								attaq_event = new ECEvent(ARM_ATTAQ, entity->Case());
								map->AddEvent(attaq_event);
							}
							attaq_event->Entities()->Add(dynamic_cast<ECEntity*>(*enti));
							ECEvent* last_move_event = map->FindEvent(entity->Case(), ARM_MOVE,
							                                          dynamic_cast<ECEntity*>(*enti));
							if(last_move_event)
							{
								attaq_event->AddLinked(last_move_event);
								map->RemoveEvent(last_move_event);
							}
						}
						if(!union_event && (*enti)->Owner() == entity->Owner() &&
						   (*enti)->Type() == entity->Type())
						{
							Debug(W_DEBUG, "On créé un evenement union");
							dynamic_cast<ECEntity*>(*enti)->Union(entity);
							union_event = new ECEvent(ARM_UNION, entity->Case());
							map->AddEvent(union_event);
							/* L'ordre est important dans l'evenement union :
							 * 1) Celui qui va disparaitre
							 * 2) Celui qui absorbe
							 */
							union_event->Entities()->Add(entity);
							union_event->Entities()->Add(dynamic_cast<ECEntity*>(*enti));
							if(event_found)
							{
								union_event->AddLinked(event_found);
								map->RemoveEvent(event_found);
							}
						}
					}
				if(attaq_event && !union_event)
				{
					Debug(W_DEBUG, "On se met dans un evenement attaq");
					attaq_event->Entities()->Add(entity);
					if(event_found)
					{ /* On link dans attaq_event et on l'enlève de la boucle principale.
					* En effet, on affichera les déplacements en meme temps que l'attaque
					*/
						attaq_event->AddLinked(event_found);
						map->RemoveEvent(event_found);
					}
					else
						Debug(W_WARNING, "ARM(MOVE).ADD_IN_ATTAQ: Il n'y a pas d'evenement de mouvement");
				}
			}
			if(event_found)
				events_sended.push_back(event_found);
		}
		if(flags == ARM_ATTAQ)
		{
			/*****************************
			 *    GESTION DES ATTAQUES   *
			 *****************************/
			entity->SetEvent(flags);

			ECEvent* attaq_event = map->FindEvent((*map)(x,y), ARM_ATTAQ, entity);
			if(!attaq_event)
			{
				attaq_event = new ECEvent(ARM_ATTAQ, (*map)(x,y));
				map->AddEvent(attaq_event);
				attaq_event->Entities()->Add(entity);
				std::vector<ECBEntity*> entities = (*map)(x,y)->Entities()->List();
				for(std::vector<ECBEntity*>::const_iterator enti = entities.begin(); enti != entities.end(); ++enti)
					if(*enti != entity && (entity->CanAttaq(*enti) && !entity->Like(*enti) ||
					   (*enti)->CanAttaq(entity) && !(*enti)->Like(entity)))
					{
						attaq_event->Entities()->Add(entity);
						if((*enti)->Owner())
							recvers.push_back(dynamic_cast<ECPlayer*>((*enti)->Owner())->Client());
					}
			}
			else
			{
				std::vector<ECEntity*> entities = attaq_event->Entities()->List();
				for(std::vector<ECEntity*>::const_iterator enti = entities.begin(); enti != entities.end(); ++enti)
					if((*enti)->Owner())
						recvers.push_back(dynamic_cast<ECPlayer*>((*enti)->Owner())->Client());
			}
		}
		recvers.push_back(cl);
		chan->SendArm(recvers, entity, flags, x, y, nb, type, events_sended);

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
			  Debug(W_DEBUG, "      - Member %s (%s)", (*enti)->LongName().c_str(), SHOW_EVENT((*enti)->EventType()));
		}
	}

	BCaseVector casv = map->Cases();
	for(BCaseVector::iterator casi = casv.begin(); casi != casv.end(); ++casi)
	{
		if((*casi)->Entities()->empty() && (!(*casi)->Country()->Owner() || !((*casi)->Flags() & C_VILLE)))
			continue;
		std::vector<ECBEntity*> entv = (*casi)->Entities()->List();
		Debug(W_DEBUG, "%d,%d: (%s)", (*casi)->X(), (*casi)->Y(), (*casi)->Country()->Owner() ? (*casi)->Country()->Owner()->Player()->GetNick() : "*");
		for(std::vector<ECBEntity*>::iterator enti = entv.begin(); enti != entv.end(); ++enti)
			Debug(W_DEBUG, "    [%c] %s (%d)", (*enti)->Locked() ? '*' : ' ', (*enti)->LongName().c_str(), (*enti)->Nb());
	}
	return 0;
}
