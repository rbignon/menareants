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
#include "Batiments.h"

void EChannel::InitAnims()
{
	dynamic_cast<ECMap*>(map)->SortEvents();
}

#define SHOW_EVENT(x) ((x) == ARM_DEPLOY ? "deploy" : (x) == ARM_UNION ? "union" : (x) == ARM_MOVE ? "move" : (x) == ARM_ATTAQ ? "attaq" : (x) == ARM_CREATE ? "create" : (x) == ARM_NUMBER ? "number" : (x) == ARM_CONTAIN ? "contain" : (x) == ARM_UNCONTAIN ? "uncontain" : "no")
void EChannel::NextAnim()
{
	if(!map) throw ECExcept(VPName(map), "Pas de map");
	if(state != EChannel::ANIMING)
		throw ECExcept(VIName(state), "Appel hors du mode +A(niming)");

	if(dynamic_cast<ECMap*>(map)->Events().empty()) return;

	ECEvent* event = (*dynamic_cast<ECMap*>(map)->Events().begin());

	if(!event)
		throw ECExcept(VPName(event) VIName(dynamic_cast<ECMap*>(map)->Events().size()), "Evenement vide");

#ifdef DEBUG
{
	Debug(W_DEBUG, "event - %s %d,%d", SHOW_EVENT(event->Flags()), event->Case()->X(),
									event->Case()->Y());

	std::vector<ECEntity*> ents = event->Entities()->List();
	for(std::vector<ECEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
	{
		Debug(W_DEBUG, "      - Member %s (%s)", (*enti)->LongName().c_str(), SHOW_EVENT((*enti)->EventType()));
	}

	if(ents.empty())
	{
		dynamic_cast<ECMap*>(map)->RemoveEvent(event, USE_DELETE);
		return;
	}
}
#endif

	switch(event->Flags())
	{
		case ARM_ATTAQ:
		{
			const char S_ATTAQ = 1;
			const char S_REMOVE = 2;
			const char S_END = 3;
			const char T_CONTINUE = 1;
			const char T_STOP = 0;
			const char T_SLEEP = -1;
			char state = S_ATTAQ;

			/* On lock temporairement toutes les entités. Les vainqueurs seront délockés */
			std::vector<ECEntity*> entv = event->Entities()->List();
			for(std::vector<ECEntity*>::iterator it = entv.begin(); it != entv.end();)
			{
				if((*it)->Shadowed())
					it = entv.erase(it);
				else
				{
					(*it)->SetShadowed();
					(*it)->Tag = T_CONTINUE;
					++it;
				}
			}
			/* Si l'evenement ne contenait que des unités locked, on s'en va (genre elle a été supprimée précédemment) */
			if(entv.empty())
			{
				dynamic_cast<ECMap*>(map)->RemoveEvent(event, USE_DELETE);
				return;
			}

			uint flags = ARM_ATTAQ;
			if(!event->Linked().empty())
				flags |= ARM_MOVE;
			SendArm(NULL, entv, flags, event->Case()->X(), event->Case()->Y(), 0, 0, event->Linked());

			std::vector<ECEntity*>::size_type nb_entities = entv.size();
			std::vector<ECEntity*> entities_save = entv;
			for(std::vector<ECEntity*>::iterator it = entv.begin(); state != S_END && !entv.empty();)
			{
				if(it == entv.end())
				{
					it = entv.begin();
					if(state == S_REMOVE && nb_entities > 0) state = S_ATTAQ;
					else state++;
					continue;
				}

				if(state == S_ATTAQ)
				{
					if((*it)->Tag == T_CONTINUE)
						(*it)->Tag = (*it)->Attaq(entv) ? T_CONTINUE : T_STOP;
					++it;
				}
				else if(state == S_REMOVE)
				{
					(*it)->ReleaseShoot();
					Debug(W_DEBUG, "- %s reste %d", (*it)->LongName().c_str(), (*it)->Nb());

					if(!(*it)->Nb() || (*it)->Tag == T_STOP)
					{
						if(!(*it)->Nb())
							it = entv.erase(it);
						else
							(*it)->Tag = T_SLEEP;
						nb_entities--;
					}
					else ++it;
				}
				else ++it;
			}
			entv = entities_save;
			for(std::vector<ECEntity*>::iterator it = entv.begin(); it != entv.end(); ++it)
			{
				if(!(*it)->Nb())
				{
					SendArm(NULL, *it, ARM_REMOVE);
					/* On ne remove pas car il est locked et sera supprimé lors du
						* passage de la boucle principale.
						*/
					//map->RemoveAnEntity(*it, USE_DELETE);
				}
				else
				{
					std::vector<ECBEntity*> fixed = event->Case()->Entities()->Fixed();
					std::vector<ECBEntity*>::iterator fix = fixed.end();
					if(!fixed.empty() && !dynamic_cast<EContainer*>(*it))
						for(fix = fixed.begin(); fix != fixed.end() && (*fix == *it ||
							(*fix)->Type() != (*it)->Type() || (*fix)->Owner() != (*it)->Owner());
								++fix);

					if(fix != fixed.end() && (*it)->Case() == (*fix)->Case())
					{
						ECEntity* gobeur = dynamic_cast<ECEntity*>(*fix);
						gobeur->Union(*it);
						(*it)->Move()->Clear((*it)->Case());
						SendArm(NULL, *it, ARM_MOVE|ARM_REMOVE, event->Case()->X(), event->Case()->Y());
						SendArm(NULL, gobeur, ARM_NUMBER, 0, 0, gobeur->Nb());
						/* Pas besoin de lock, il l'est déjà.
							* (*it)->SetLock();
							*/
					}
					else
					{
						SendArm(NULL, *it, ARM_NUMBER, 0, 0, (*it)->Nb());
						EContainer* contain = dynamic_cast<EContainer*>(*it);
						if(contain && contain->Containing())
							SendArm(NULL, dynamic_cast<ECEntity*>(contain->Containing()), ARM_NUMBER, 0, 0,
							                                                              contain->Containing()->Nb());
						(*it)->Case()->CheckChangingOwner(*it);
						(*it)->SetShadowed(false);
					}
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
				Debug(W_ERR, "Il faudrait qu'il y ait deux entités dans la liste de UNION et il y en a %d", entv.size());
			else
			{
				SendArm(NULL, entv[1], ARM_NUMBER|ARM_HIDE, 0, 0, entv[1]->Nb());
				EContainer* contain_r = dynamic_cast<EContainer*>(event->Entity());
				EContainer* contain = dynamic_cast<EContainer*>(entv[1]);
				if(contain_r && contain_r->Containing() && contain && contain_r->Containing() != contain->Containing())
					SendArm(NULL, dynamic_cast<ECEntity*>(contain_r->Containing()), ARM_REMOVE);
				if(contain && contain->Containing())
				{
					SendArm(NULL, dynamic_cast<ECEntity*>(contain->Containing()), ARM_NUMBER|ARM_HIDE, 0, 0,
					                                                              contain->Containing()->Nb());
					if(contain_r && contain_r->Containing() == contain->Containing())
						SendArm(NULL, dynamic_cast<ECEntity*>(contain->Containing()), ARM_CONTENER);
				}
			}
			break;
		}
		case ARM_DEPLOY:
			SendArm(NULL, event->Entities()->List(), ARM_DEPLOY);
			break;
		case ARM_CREATE:
			SendArm(NULL, event->Entities()->List(), ARM_CREATE|ARM_HIDE|ARM_NOCONCERNED, event->Case()->X(),
			                                         event->Case()->Y(), event->Nb(), event->Type());
			break;
		case ARM_CONTAIN:
		case ARM_UNCONTAIN:
		case ARM_MOVE:
		{
			std::vector<ECEvent*> ev;
			ev.push_back(event);
			SendArm(NULL, event->Entities()->List(), event->Flags(), event->Case()->X(), event->Case()->Y(), 0, 0, ev);

			std::vector<ECEntity*> entv = event->Entities()->List();
			for(std::vector<ECEntity*>::iterator it = entv.begin(); it != entv.end(); ++it)
				(*it)->Case()->CheckChangingOwner(*it);
			break;
		}
		default: Debug(W_WARNING, "L'evenement '%s' n'est pas supporté", SHOW_EVENT(event->Flags())); break;
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
#include "lib/UnitsList.h"
};

ECEntity* CreateAnEntity(uint type, const Entity_ID _name, ECBPlayer* _owner, ECase* _case)
{
	return entities_type[type].func (_name, _owner, _case);
}

/** Modification of an army.
 *
 * Syntax: ARM nom [+nb] [%type] [/nb] [=pos] [*pos] [>] [v] [>] [^] [#] [!] [)nom] [(pos]
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

		if(!entity || entity->Shadowed() || entity->Locked())
			return vDebug(W_DESYNCH, "ARM: Entité introuvable", VPName(entity) VName(parv[1])
			                          VBName(entity ? entity->Shadowed() : false));
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
					return Debug(W_DESYNCH, "ARM: Utilisation de [v<^>] avant un =");
				if(entity)
					return Debug(W_DESYNCH, "ARM: Utilisation de =<pos> sur une entité existante");

				std::string s = parv[i].substr(1);
				x = StrToTyp<uint>(stringtok(s, ","));
				y = StrToTyp<uint>(s);

				flags |= ARM_MOVE;
				break;
			}
			case '*':
			{
				std::string s = parv[i].substr(1);
				x = StrToTyp<uint>(stringtok(s, ","));
				y = StrToTyp<uint>(s);

				if(entity && !(entity->EventType() & ARM_ATTAQ) && entity->WantAttaq(x, y))
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
			case '#':
			{
				if(entity && entity->WantDeploy())
					flags |= ARM_DEPLOY;
				break;
			}
			case '!':
			{
				flags |= ARM_FORCEATTAQ;
				break;
			}
			case ')':
			{
				EContainer* container = dynamic_cast<EContainer*>(cl->Player()->Entities()->Find(parv[i].substr(1).c_str()));
				if(entity && container && container->WantContain(entity, moves))
					flags |= ARM_CONTAIN;
				break;
			}
			case '(':
			{
				std::string s = parv[i].substr(1);
				uint _x = StrToTyp<uint>(stringtok(s, ","));
				uint _y = StrToTyp<uint>(s);
				EContainer* container = entity ? dynamic_cast<EContainer*>(entity) : 0;
				ECEntity* contened = dynamic_cast<ECEntity*>(container->Containing());
				if(container && container->Containing() && container->WantUnContain(_x,_y, moves))
				{
					flags |= ARM_UNCONTAIN;
					entity = contened;
					last_case = container->Case();
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
		if(flags != ARM_CREATE || type == ECBEntity::E_NONE || type >= ECBEntity::E_END)
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
			std::vector<ECBEntity*> ents = entity->Case()->Entities()->List();
			ECEntity* my_friend = 0;
			for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
				if(!dynamic_cast<ECEntity*>(*enti)->Shadowed() && (*enti)->Owner() == entity->Owner() &&
				   (*enti)->Type() == entity->Type())
					my_friend = dynamic_cast<ECEntity*>(*enti);
			if(my_friend)
			{ /* On peut probablement faire une union avec une entité de cette case donc bon au lieu de
			   * se broyer les testicule on va ARM_NUMBER simplement */
				MyFree(entity);
				entity = my_friend;
				flags = ARM_NUMBER;
			}
			else
			{
				Debug(W_DEBUG, "On rajoute un ARM_CREATE");
				cl->Player()->DownMoney(entity->Cost());
				cl->Player()->Stats()->created += entity->Nb();
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
			if(entity->CanBeCreated() && int(entity->Cost()) <= cl->Player()->Money() && entity->AddUnits(entity->InitNb()))
			{
				/*****************************
				*     GESTION DES AJOUTS    *
				*****************************/
				cl->Player()->Stats()->created += entity->InitNb();
				cl->Player()->DownMoney(entity->Cost());
				nb = entity->Nb();
			}
			else
				flags = 0;
		}
		if(flags & ARM_DEPLOY)
		{
			/*****************************
			 *  GESTION DES DÉPLOIEMENTS *
			 *****************************/
			EventVector evts = map->Events();
			bool can_create_event = true;
			for(EventVector::iterator evti = evts.begin(); evti != evts.end();)
			{
				/* Si il y a déjà un evenement, c'est qu'il y a eu déploiement et reploiement
				 * à la suite, donc on revient à la situation initiale et on le supprime
				 */
				if((*evti)->Flags() == ARM_DEPLOY && (*evti)->Entity() == entity)
				{
					map->RemoveEvent(*evti, USE_DELETE);
					evti = evts.erase(evti);
					can_create_event = !can_create_event;
					Debug(W_DEBUG, "Il y avait déjà un evenement de DEPLOY que je supprime");
				}
				else
					++evti;
			}
			if(can_create_event)
			{
				Debug(W_DEBUG, "Création d'un evenement de déploiement");
				ECEvent* event = new ECEvent(ARM_DEPLOY, entity->Case());
				event->Entities()->Add(entity);
				map->AddEvent(event);
			}
		}
		if(flags & ARM_MOVE)
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
				if(last_case && (*evti)->Case() == last_case && (*evti)->Flags() == ARM_MOVE &&
				   (*evti)->Entity() == entity && (last = entity->FindLast(last_case)) &&
				   (!last->Last() || last->Last()->Case() != last->Case()))
				{ /* On a trouvé un déplacement pour cette même entité alors on en profite */
					if(event_found)
						Debug(W_WARNING, "ARM(MOVE).MyMoveFound: Il y avait déjà un event trouvé de MOVE pour cet entité");
					for(ECMove::Vector::const_iterator it = moves.begin(); it != moves.end(); ++it)
						(*evti)->Move()->AddMove(*it);
					event_found = *evti;
					event_found->SetCase(entity->Case());
					event_found->SetFlags(flags);
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
				if(last_case && (*evti)->Case() == last_case && (*evti)->Flags() == ARM_ATTAQ &&
				   (*evti)->Entities()->Find(entity))
				{
					/* On sous traite à la fonction dont le nom fait rêver */
					delete_event = (*evti)->CheckRemoveBecauseOfPartOfAttaqEntity(entity);
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
			/* On ne vérifie pas si last_case != 0, car si c'est le cas c'est une creation, et donc dans ce cas
			 * là on vérifie bien si y a attaque (on peut considérer qu'il était pas sur la meme case avant)
			 */
			if(entity->Case() != last_case)
			{ /* On ne fait ça que si y a eu un changement de case */
				if(!event_found && last_case) // On vérifie bien que c'est pas un create
				{
					Debug(W_DEBUG, "On créé un nouvel event MOVE");
					event_found = new ECEvent(flags, entity->Case());
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
						if(*enti == entity) continue;

						if((entity->CanAttaq(*enti) && !entity->Like(*enti) ||
						    (*enti)->CanAttaq(entity) && !(*enti)->Like(entity))
						  &&
						   (!dynamic_cast<ECEntity*>(*enti)->Shadowed() ||
						    (*enti)->MyStep() - (*enti)->RestStep() == entity->MyStep() - entity->RestStep()))
						{
							if(!attaq_event)
							{
								Debug(W_DEBUG, "On créé un evenement attaque");
								attaq_event = new ECEvent(ARM_ATTAQ, entity->Case());
								map->AddEvent(attaq_event);
							}
							ECEntity* entity2 = dynamic_cast<ECEntity*>(*enti)->FindNext();
							attaq_event->Entities()->Add(entity2);

							ECEvent* last_move_event = map->FindEvent(entity2->Case(), ARM_MOVE,
							                                          entity2);
							if(last_move_event)
							{
								attaq_event->AddLinked(last_move_event);
								map->RemoveEvent(last_move_event);
								Debug(W_DEBUG, "Mouvement d'un MOVE dans l'ATTAQ");
							}
							/* Si jamais on rencontre une entité locked (donc qui avait déjà bougé), et ben
							 * on la fait retourner sur cette case
							 */
							if(dynamic_cast<ECEntity*>(*enti)->Shadowed())
							{
								Debug(W_DEBUG, "Il avait fait un déplacement qu'on annule pour revenir à la case du combat");
								ECBCase* c = entity2->Case();
								entity2->Return(entity->Case());
								ECEvent* evnt = map->FindEvent(c, 0, entity2);
								if(evnt && evnt->CheckRemoveBecauseOfPartOfAttaqEntity(entity2))
								{
									Debug(W_DEBUG, "On jarte un ancien event auquel j'assistais sur mon ancienne case (%d)",
									               SHOW_EVENT(evnt->Flags()));
									map->RemoveEvent(evnt, USE_DELETE);
								}
							}
						}
						/* L'union entre les conteneurs ne se fait que si au moins l'un des deux ne contient rien, ou alors
						 * si les deux unités qu'ils contiennent sont du même type. */
						else if(!union_event && !dynamic_cast<ECEntity*>(*enti)->Shadowed() &&
						        (*enti)->Owner() == entity->Owner() && (*enti)->Type() == entity->Type())
						{
							EContainer* enti_container = dynamic_cast<EContainer*>(*enti);
							EContainer* entity_container = dynamic_cast<EContainer*>(entity);
							if(!enti_container || !entity_container || !enti_container->Containing() ||
							   !entity_container->Containing() ||
							   enti_container->Containing()->Type() == entity_container->Containing()->Type())
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
								flags |= ARM_LOCK;
								if(event_found)
								{
									union_event->AddLinked(event_found);
									map->RemoveEvent(event_found);
								}
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
			/*if(event_found)
				events_sended.push_back(event_found);*/
		}
		// ARM_ATTAQ.(1 + ARM_FORCEATTAQ)
		if(flags & ARM_ATTAQ)
		{
			/*****************************
			 *    GESTION DES ATTAQUES   *
			 *****************************/

			ECBCase* c = (*map)(x,y);

			for(uint turn = 0; !(entity->EventType() & ARM_ATTAQ); ++turn)
			{
				Debug(W_DEBUG, "Recherche à (%d,%d)", c->X(), c->Y());
				ECEvent* attaq_event = map->FindEvent(c, ARM_ATTAQ, entity);
				if(!attaq_event)
				{
					if(flags & ARM_FORCEATTAQ)
					{
						Debug(W_DEBUG, "Création d'un evenement ATTAQ");
						attaq_event = new ECEvent(ARM_ATTAQ, c);
						map->AddEvent(attaq_event);
						attaq_event->Entities()->Add(entity);
						entity->SetEvent(flags);
					}
					std::vector<ECBEntity*> entities = c->Entities()->List();
					ECEntity* next_entity = 0;
					for(std::vector<ECBEntity*>::iterator enti = entities.begin(); enti != entities.end(); ++enti)
					{
						bool can_attaq = false;
						/* La ligne "entity->Move()->Empty()" s'explique simplement par le fait que le client ne voit pas
						* les déplacements faits par les autres. Donc il ne peut faire une attaque sur une case anticipée
						* d'une autre entité. C'est pourquoi si jamais l'unité de cette case a déjà fait un mouvement, c'est
						* que le client qui veut attaquer anticipe une case qui lui semble vide. En tous cas il n'y voit pas
						* cette unité.
						* Si jamais par contre il a cliqué sur la position de cette entité sur sa case d'origine, des
						* dispositions sont là pour ramener l'attaque à la case d'arrivée.
						* Note: si turn>=0, c'est qu'on a nous même recherché une unité qui a bougé, donc ne plus vérifier.
						*/
						if(!dynamic_cast<ECEntity*>(*enti)->Shadowed() && *enti != entity && entity->CanAttaq(*enti) &&
						   !entity->Like(*enti) && (turn || dynamic_cast<ECEntity*>(*enti)->Move()->Empty()))
							can_attaq = true;
						else if(dynamic_cast<ECEntity*>(*enti)->Shadowed() && dynamic_cast<ECEntity*>(*enti)->Move()->Empty())
						{
							Debug(W_DEBUG, "On a trouvé une unité qui a bougé");
							ECEntity* e = dynamic_cast<ECEntity*>(*enti)->FindNext();
							if(entity->WantAttaq(e->Case()->X(), e->Case()->Y()))
								next_entity = e;
						}
	
						if(can_attaq)
						{
							if(!attaq_event)
							{
								Debug(W_DEBUG, "Création d'un evenement ATTAQ");
								attaq_event = new ECEvent(ARM_ATTAQ, c);
								map->AddEvent(attaq_event);
								attaq_event->Entities()->Add(entity);
								entity->SetEvent(flags);
							}
							attaq_event->Entities()->Add(dynamic_cast<ECEntity*>(*enti));
						}
					}
					if(!attaq_event)
					{
						if(next_entity)
						{
							Debug(W_DEBUG, "On change de case !");
							c = next_entity->Case();
							continue;
						}
						Debug(W_DEBUG, "Finalement on ne fait pas d'attaque");
						flags &= ~ARM_ATTAQ;
						break;
					}
				}
				else
				{
					Debug(W_DEBUG, "Il y a déjà une attaque, on ne prévient PAS les attaqués");
					entity->SetEvent(flags);
				}
			}
		}
		recvers.push_back(cl);
		chan->SendArm(recvers, entity, flags, x, y, nb, type, events_sended);

	}
#ifdef DEBUG
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
#endif

#if 0
	BCaseVector casv = map->Cases();
	for(BCaseVector::iterator casi = casv.begin(); casi != casv.end(); ++casi)
	{
		if((*casi)->Entities()->empty())
			continue;
		std::vector<ECBEntity*> entv = (*casi)->Entities()->List();
		Debug(W_DEBUG, "%d,%d: (%s)", (*casi)->X(), (*casi)->Y(),
						(*casi)->Country()->Owner() ? (*casi)->Country()->Owner()->Player()->GetNick() : "*");
		for(std::vector<ECBEntity*>::iterator enti = entv.begin(); enti != entv.end(); ++enti)
			Debug(W_DEBUG, "    [%c] %s (%d)", dynamic_cast<ECEntity*>(*enti)->Shadowed() ? '*' : ' ',
			                                   (*enti)->LongName().c_str(), (*enti)->Nb());
	}
#endif
	return 0;
}
