/* server/InGame.cpp - Commands called in a game.
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

#include "InGame.h"
#include "Channels.h"
#include "Debug.h"
#include "Outils.h"
#include "Commands.h"
#include "Units.h"
#include "Batiments.h"
#include "Main.h"

#define SHOW_EVENT(x) ((x) == ARM_DEPLOY ? "deploy" : (x) == ARM_UNION ? "union" : (x) == ARM_MOVE ? "move" : (x) == ARM_ATTAQ ? "attaq" : (x) == ARM_CREATE ? "create" : (x) == ARM_NUMBER ? "number" : (x) == ARM_CONTAIN ? "contain" : (x) == ARM_UNCONTAIN ? "uncontain" : (x) == ARM_UPGRADE ? "upgrade" : "no")
void EChannel::InitAnims()
{
#ifdef DEBUG
	EventVector evts = Map()->Events();

	for(EventVector::iterator evti = evts.begin(); evti != evts.end(); ++evti)
	{
		if((*evti)->Entity())
			Debug(W_DEBUG|W_ECHO, "=== event - %s %d,%d (%s - %s) ===", SHOW_EVENT((*evti)->Flags()), (*evti)->Case()->X(),
		              (*evti)->Case()->Y(), (*evti)->Entity()->LongName().c_str(), SHOW_EVENT((*evti)->Entity()->EventType()));
		else
			Debug(W_DEBUG|W_ECHO, "=== event - %s %d,%d (no sender) ===", SHOW_EVENT((*evti)->Flags()), (*evti)->Case()->X(),
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
	std::vector<ECEvent*> events = Map()->Events();
	FORit(ECEvent*, events, eventit)
	{
		ECEvent* event = *eventit;
		if(event->Flags() == ARM_CREATE)
		{
			if(event->Entity()->IsHidden())
			{
				if(event->Entity()->Owner() && event->Entity()->Owner()->NbAllies() > 0)
					SendArm(event->Entity()->Owner()->ClientAllies(), event->Entity(), ARM_CREATE, event->Case()->X(), event->Case()->Y());
			}
			else
				SendArm(0, event->Entity(), ARM_CREATE|ARM_HIDE, event->Case()->X(),
				                                                                 event->Case()->Y());

			// false veut dire qu'on est dans un evenement, au moment où la création est propagée
			/* Note: comme je viens de relire ce commentaire sans le comprendre du premier coup, je
			 * me permet de rajouter que la fonction ECEntity::Create() est appellée quand l'unité est créée,
			 * et qu'en l'occurence on met "false" pour spécifier qu'on est dans l'evenement qui
			 * propage la création de l'unité.
			 */
			event->Entity()->Created(false);

			Debug(W_DEBUG|W_ECHO, "= envoie d'un CREATE =");

			std::vector<ECBEntity*> entities = event->Entity()->Case()->Entities()->List();
			FORit(ECBEntity*, entities, e)
				if(!(*e)->IsZombie() && ThereIsAttaq(event->Entity(), *e))
				{
					ECEvent* attaq_event = new ECEvent(ARM_ATTAQ, 0, event->Entity()->Case());

					ShowAnim(attaq_event);

					MyFree(attaq_event);
					break;
				}

			/* On ne cherche pas à supprimer dans la liste d'evenements de l'entité car,
			 * par définition, un ARM_CREATE n'est pas listée à l'intérieur, à cause
			 * de son status particulier.
			 * On supprime donc juste directement dans ECMap et ECPlayer.
			 */
			if(event->Entity()->Owner())
				event->Entity()->Owner()->Events()->Remove(event);
			Map()->RemoveEvent(event, USE_DELETE);
		}
		else if(event->Entity()->Owner() == 0)
			ShowAnim(event);
	}
	playing = first_playing++;
	if(first_playing >= players.size())
		first_playing = 0;
}

void EChannel::NextAnim()
{
	assert(Map());
	assert(State() == EChannel::ANIMING);
	assert(playing < players.size());

	ECEvent* event = 0;
	while(!event && !Map()->Events().empty())
	{
		do
		{
			if(!(event = dynamic_cast<ECPlayer*>(players[playing])->Events()->First()))
				break;

			/* Si l'evenement ne s'est pas produit (l'unité a été détruite ou autre), on passe à l'evenement
			 * suivant pour éviter de faire perdre du temps.
			 */
			if(ShowAnim(event) == false)
				event = 0;
		}
		while(!event);

		playing++;
		if(playing >= players.size())
			playing = 0;
	}
}

bool EChannel::ShowAnim(ECEvent* event)
{
#ifdef DEBUG
	{
		if(event->Entity())
			Debug(W_DEBUG|W_ECHO, "=== event - %s %d,%d (%s - %s) ===", SHOW_EVENT(event->Flags()), event->Case()->X(),
		              event->Case()->Y(), event->Entity()->LongName().c_str(), SHOW_EVENT(event->Entity()->EventType()));
		else
			Debug(W_DEBUG|W_ECHO, "=== event - %s %d,%d (no sender) ===", SHOW_EVENT(event->Flags()), event->Case()->X(),
		              event->Case()->Y());

		std::vector<ECEntity*> ents = event->Entities()->List();
		for(std::vector<ECEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
		{
			Debug(W_DEBUG|W_ECHO, "      - Member %s (%s)", (*enti)->LongName().c_str(), SHOW_EVENT((*enti)->EventType()));
		}
	}
#endif
	bool ret = true;

	/* On supprime l'evenement chez toutes les entités qui l'ont à priori */
	/* On le fait au début car après il se peut que des unités changent d'owner, etc */
	if(event->Entity())
	{
		event->Entity()->Events()->Remove(event);
		if(event->Entity()->Owner())
			event->Entity()->Owner()->Events()->Remove(event);
	}
	std::vector<ECEntity*> ents = event->Entities()->List();
	FORit(ECEntity*, ents, it)
	{
		(*it)->Events()->Remove(event);
		if((*it)->Owner())
			(*it)->Owner()->Events()->Remove(event);
	}

	if(event->Entity() && event->Entity()->IsZombie())
		ret = false;
	else
		switch(event->Flags())
		{
			case ARM_ATTAQ:
			{
				/* Si il y a event->Entity(), ça veut dire que c'est quelqu'un qui a fait une attaque
				 * explicite.
				 * Et si la liste des entités n'est pas vide, ça veut dire qu'il y a des cibles précises,
				 * et on va chercher à voir qui on attaque (dans le cas où elle a bougée).
				 */
				if(event->Entities()->Empty() == false && event->Entity())
				{
					std::vector<ECEntity*> ents = event->Entities()->List();
					ECEntity* victim = 0;
					FOR(ECEntity*, ents, e)
					{
						if(!e->Case() || !event->Entity()->CanAttaq(e) || e->IsZombie() || e->Parent() ||
						   event->Entity()->Case()->Delta(e->Case()) > event->Entity()->Porty())
							continue;
						if(!victim || e->Case() != event->Case())
							victim = e;
					}
					if(victim)
						event->SetCase(victim->Case());
					else
					{
						ret = false;
						break;
					}
				}

				enum
				{
					S_ATTAQ,
					S_REMOVE,
					S_END
				};
				enum
				{
					T_SLEEP = -1,
					T_STOP = 0,
					T_CONTINUE = 1
				};
				std::vector<ECEntity*> entv;
				if(event->Entity())
				{
					entv.push_back(event->Entity());
					event->Entity()->Tag = T_CONTINUE;
				}
				std::map<ECEntity*, int> ents_init_nb;
				/* On rends zombie temporairement toutes les entités. Les vainqueurs seront délockés */
				{
					std::vector<ECBEntity*> case_entities = event->Entity() ?
					                                                event->Entity()->GetAttaquedEntities(event->Case())
					                                              : event->Case()->Entities()->List();
					for(std::vector<ECBEntity*>::iterator it = case_entities.begin(); it != case_entities.end(); ++it)
						if(!(*it)->IsZombie() && !(*it)->Locked())
						{
							/* Si l'unité est hidden, il faut bien montrer son existance lors d'un combat, donc on créé l'unité.
							Les alliés du propriétaire ont déjà vu la création de l'unité, et vont donc recevoir une nouvelle
							ligne, mais theoriquement ils devraient l'ignorer. */
							ECEntity *e = dynamic_cast<ECEntity*>(*it);
							if(e->IsHidden())
								SendArm(0, e, ARM_CREATE|ARM_HIDE, e->Case()->X(), e->Case()->Y());

							e->SetZombie();
							e->Tag = T_CONTINUE;
							ents_init_nb[e] = e->RealNb();

							if(e != event->Entity())
								entv.push_back(e);
						}
				}
				/* Si l'evenement ne contenait que des unités zombied, on s'en va (genre elle a été supprimée précédemment) */
				if(entv.empty())
				{
					ret = false;
					break;
				}

				uint flags = ARM_ATTAQ;
				if(!event->Linked().empty())
					flags |= ARM_MOVE;

				/* Si jamais il n'y a plus que des amis on envoie les mouvements puis on se barre, sauf si quelqu'un a
				* vraiment envie de se taper meme contre personne
				*/
				if(!event->Entity() && ECEntity::AreFriends(entv))
					flags &= ~ARM_ATTAQ;

				if(event->Entity())
					SendArm(NULL, event->Entity(), flags, event->Case()->X(), event->Case()->Y(), 0, event->Linked());
				else
					SendArm(NULL, entv, flags, event->Case()->X(), event->Case()->Y(), 0, event->Linked());

				if(!(flags & ARM_ATTAQ))
				{
					// Il n'y a que des amis (voir trois lignes plus haut) donc on se barre après avoir envoyé les moves
					for(std::vector<ECEntity*>::iterator it = entv.begin(); it != entv.end(); ++it)
						(*it)->SetZombie(false);
					break;
				}

				std::nrvector<TClient*> receivers = ECEntity::EntitiesToClients(entv);

				int state = S_ATTAQ;
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
							(*it)->Tag = (*it)->Attaq(entv, event) ? T_CONTINUE : T_STOP;
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
							{
								(*it)->Tag = T_SLEEP;
								++it;
							}
							// Il est déjà arrivé, allez savoir pourquoi, que nb_entities soit décrémenté en dessous de 0.
							if(nb_entities > 0)
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
						/* On ne remove pas car il est shadowed et sera supprimé lors du
							* passage de la boucle principale.
							*/
						//map->RemoveAnEntity(*it, USE_DELETE);
					}
					else
					{
						if(!event->Entity() && (*it)->Owner())
							for(std::map<ECEntity*,int>::iterator en = ents_init_nb.begin(); en != ents_init_nb.end(); ++en)
							{
								if((*it)->Like(en->first) || !en->second)
									continue;
								(*it)->Owner()->Stats()->score += en->second;
								if((*it)->Case() != en->first->Case())
									continue;
								(*it)->Owner()->Stats()->score += abs(en->second - ents_init_nb[*it]);
							}
						std::vector<ECBEntity*> fixed = (*it)->Case()->Entities()->List();
						std::vector<ECBEntity*>::iterator fix = fixed.end();
						if(!fixed.empty())
							for(fix = fixed.begin();
							        fix != fixed.end() && (*fix == *it || (*fix)->IsZombie() || (*fix)->Locked() || (*fix)->Level() != (*it)->Level() ||
							        !(*fix)->Move()->Empty() || (*fix)->Type() != (*it)->Type() || (*fix)->Owner() != (*it)->Owner() ||
							        dynamic_cast<EContainer*>(*fix) && dynamic_cast<EContainer*>(*it) &&
							        dynamic_cast<EContainer*>(*fix)->Containing() && dynamic_cast<EContainer*>(*it)->Containing() &&
							        dynamic_cast<EContainer*>(*fix)->Containing()->Type() != dynamic_cast<EContainer*>(*it)->Type());
							    ++fix);

						if(fix != fixed.end() && (*it)->Move()->Empty())
						{
							ECEntity* gobeur = dynamic_cast<ECEntity*>(*fix);
							gobeur->Union(*it);
							(*it)->Move()->Clear((*it)->Case());
							SendArm(NULL, *it, ARM_INVEST|ARM_REMOVE, event->Case()->X(), event->Case()->Y());
							SendArm(receivers, gobeur, ARM_NUMBER);
							Debug(W_DEBUG, "Reste %d à %s", gobeur->Nb(), gobeur->LongName().c_str());
							/* Pas besoin de mettre shadow, il l'est déjà.
								* (*it)->SetZombie();
								*/
						}
						else
						{
							SendArm(receivers, *it, ARM_NUMBER);
							Debug(W_DEBUG, "Reste %d à %s", (*it)->Nb(), (*it)->LongName().c_str());
							EContainer* contain = dynamic_cast<EContainer*>(*it);
							if(contain && contain->Containing())
							{
								SendArm(receivers, dynamic_cast<ECEntity*>(contain->Containing()), ARM_NUMBER);
								Debug(W_DEBUG, "Reste %d à %s", contain->Containing()->Nb(), contain->Containing()->LongName().c_str());
							}
							dynamic_cast<ECase*>((*it)->Case())->CheckInvests(*it);
							(*it)->SetZombie(false);
						}
					}
				}
				break;
			}
			case ARM_DEPLOY:
				event->Entity()->SetDeployed(!event->Entity()->Deployed());
				event->Entity()->DelEvent(ARM_DEPLOY);
				SendArm(NULL, event->Entity(), ARM_DEPLOY);
				break;
			case ARM_CONTAIN:
			case ARM_UNCONTAIN:
			case ARM_MOVE:
			{
				if(event->Entity() == 0)
				{
					Debug(W_WARNING, "Nobody to do this movement");
					ret = false;
					break;
				}
				ECEntity* entity = event->Entity();
				if(event->Flags() == ARM_CONTAIN)
				{
					EContainer* container;
					if(event->Entities()->Empty() || !(container = dynamic_cast<EContainer*>(event->Entities()->First())))
					{
						Debug(W_WARNING, "ARM CONTAIN: There isn't any container in event's entities !");
						break;
					}
					/* Si le conteneur est un zombie, c'est qu'il a été supprimé et donc on ne pénètre pas à l'interieur */
					if(container->IsZombie())
					{
						event->Move()->Return(entity->Case());
						entity->Move()->Return(entity->Case());
						ret = false;
						break;
					}
				}
				if(event->Flags() == ARM_UNCONTAIN)
				{
					EContainer* container = dynamic_cast<EContainer*>(entity->Parent());
					if(!container)
					{
						Debug(W_WARNING, "ARM UNCONTAIN %s: Entity isn't in a container !", entity->LongName().c_str());
						ret = false;
						break;
					}
					if(container->IsZombie())
					{ /* Le container a été supprimé, donc on ne bouge pas car on est forcément supprimé également.
					   * Il est à noter qu'en theorie si le container est un zombie, moi aussi
					   * et donc on aurait pas du arriver ici.
					   */
						Debug(W_WARNING, "ARM UNCONTAIN %s: My parent is a zombie, but not me !", entity->LongName().c_str());
						ret = false;
						break;
					}
					container->UnContain();
				}
				ECBMove::Vector moves = event->Move()->Moves();
				ECBCase* c = entity->Case();
				bool end = false;
				int score_to_add = 1;
				FORit(ECBMove::E_Move, moves, m)
				{
					switch(*m)
					{
						case ECBMove::Up: c = c->MoveUp(); break;
						case ECBMove::Down: c = c->MoveDown(); break;
						case ECBMove::Left: c = c->MoveLeft(); break;
						case ECBMove::Right: c = c->MoveRight(); break;
					}
					score_to_add *= 2;
					entity->ChangeCase(c);
					std::vector<ECBEntity*> entities = c->Entities()->List();
					bool attaq = false, invest = false;
					FORit(ECBEntity*, entities, e)
					{
						if((*e)->IsZombie() || (*e)->Parent()) continue;
						if(ThereIsAttaq(entity, *e))
						{
							attaq = true;
							break;
						}
						if(entity->CanInvest(*e))
							invest = true;
					}
					if(attaq)
					{
						ECEvent* attaq_event = new ECEvent(ARM_ATTAQ, 0, c);
						attaq_event->AddLinked(event);

						ShowAnim(attaq_event);

						MyFree(attaq_event);

						if(entity->IsZombie())
						{
							end = true;
							break;
						}

						event->Move()->GoTo(c);
						entity->Move()->GoTo(c);

						dynamic_cast<ECase*>(c)->CheckInvests(entity);

						if(entity->IsZombie())
						{
							end = true;
							break;
						}
					}
					else if(invest)
					{
						ECEvent* invest_event = new ECEvent(event->Flags(), entity, c);
						invest_event->Move()->SetEntity(entity);
						invest_event->Move()->SetFirstCase(event->Move()->FirstCase());
						invest_event->Move()->SetMoves(event->Move()->Moves());
						invest_event->Move()->Return(c);

						SendArm(NULL, entity, event->Flags(), event->Case()->X(), event->Case()->Y(), 0, ToVec(invest_event));

						MyFree(invest_event);

						event->Move()->GoTo(c);
						entity->Move()->GoTo(c);

						dynamic_cast<ECase*>(c)->CheckInvests(entity);

						if(entity->IsZombie())
						{
							end = true;
							break;
						}
					}
				}
				if(entity->Owner()) entity->Owner()->Stats()->score += score_to_add;
				if(end)
					break;

				if(!event->Move()->Empty())
				{
					std::vector<ECEvent*> ev;
					ev.push_back(event);

					if(event->Flags() == ARM_CONTAIN)
					{
						EContainer* container;
						if(event->Entities()->Empty() || !(container = dynamic_cast<EContainer*>(event->Entities()->First())))
						{
							Debug(W_WARNING, "ARM CONTAIN: There isn't any container in event's entities !");
							break;
						}
						/* Si le conteneur est un zombie, c'est qu'il a été supprimé et donc on ne pénètre pas à l'interieur */
						if(container->IsZombie())
						{
							event->Move()->Return(entity->Case());
							entity->Move()->Return(entity->Case());
							event->SetFlags(ARM_MOVE);
						}
						else
						{
							event->Entity()->ChangeCase(event->Move()->Dest());
							container->Contain(entity);
						}
					}
					else
						event->Entity()->ChangeCase(event->Move()->Dest());

					SendArm(NULL, entity, event->Flags(), event->Case()->X(), event->Case()->Y(), 0, ev);

					event->Entity()->Move()->GoTo(entity->Case());
					dynamic_cast<ECase*>(entity->Case())->CheckInvests(entity);
				}

				if(!entity->IsZombie() && entity->Move()->Empty())
				{
					std::vector<ECBEntity*> fixed = entity->Case()->Entities()->List();
					std::vector<ECBEntity*>::iterator fix = fixed.end();
					if(!fixed.empty())
						for(fix = fixed.begin();
							fix != fixed.end() && (*fix == entity || (*fix)->IsZombie() || (*fix)->Locked() || (*fix)->Level() != entity->Level() ||
							!(*fix)->Move()->Empty() || (*fix)->Type() != entity->Type() || (*fix)->Owner() != entity->Owner() ||
							dynamic_cast<EContainer*>(*fix) && dynamic_cast<EContainer*>(entity) &&
							dynamic_cast<EContainer*>(*fix)->Containing() && dynamic_cast<EContainer*>(entity)->Containing() &&
							dynamic_cast<EContainer*>(*fix)->Containing()->Type() != dynamic_cast<EContainer*>(entity)->Type() );
							++fix);

					if(fix != fixed.end())
					{
						ECEntity* gobeur = dynamic_cast<ECEntity*>(*fix);
						gobeur->Union(entity);
						entity->Move()->Clear(entity->Case());
						SendArm(NULL, entity, ARM_INVEST|ARM_REMOVE, entity->Case()->X(), entity->Case()->Y());
						if(entity->Owner())
						{
							SendArm(entity->Owner()->Client(), gobeur, ARM_NUMBER);
							if(entity->Owner()->NbAllies() > 0)
								SendArm(entity->Owner()->ClientAllies(), gobeur, ARM_NUMBER);
						}
						entity->SetZombie();
					}
				}

				break;
			}
			case ARM_UPGRADE:
			{
				if(event->Entity() == 0)
				{
					Debug(W_WARNING, "Nobody to do an upgrade");
					ret = false;
					break;
				}
				ECEntity* entity = event->Entity();

				SendArm(NULL, entity, ARM_REMOVE|ARM_UPGRADE);

				ECEntity* upgrade = CreateAnEntity(entity->MyUpgrade(), entity->ID(), entity->Owner(), entity->Case());

				/* On remet le niveau de vie au plus haut */
				upgrade->SetNb(upgrade->InitNb());

				/* Elle se fera supprimer */
				entity->SetZombie();
				Map()->AddAnEntity(upgrade);

				SendArm(NULL, upgrade, ARM_CREATE|ARM_HIDE, upgrade->Case()->X(), upgrade->Case()->Y());
				break;
			}
			default: Debug(W_WARNING, "Event '%s' isn't supported", SHOW_EVENT(event->Flags())); break;
		}

	Map()->RemoveEvent(event, USE_DELETE);

	Debug(W_DEBUG|W_ECHO, "=== End of event ===");


	return ret;
}

template<typename T>
static ECEntity* CreateEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case)
{
	T* entity = new T(_name, _owner, _case);
	entity->SetNb(entity->InitNb());
	entity->Init();
	return entity;
}

static struct
{
	ECEntity* (*func) (const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case);
} entities_type[] = {
#include "lib/UnitsList.h"
};

ECEntity* CreateAnEntity(uint type, const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case)
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
		return Debug(W_DESYNCH, "ARM: Player %s is ready!", cl->GetNick());

	if(cl->Player()->Lost())
		return Debug(W_DESYNCH, "ARM: Player %s has lost!", cl->GetNick());

	EChannel* chan = cl->Player()->Channel();
	ECMap *map = dynamic_cast<ECMap*>(chan->Map());
	ECEntity* entity = 0;
	uint flags = 0;

	if(parv[1][0] != '-') /* Si parv[1][0] == '-' alors on créé une nouvelle entity */
	{
		entity = dynamic_cast<ECEntity*>(cl->Player()->Entities()->Find(parv[1].c_str()));

		if(!entity)
			return vDebug(W_DESYNCH, "ARM: Unable to find entity", VPName(entity) VName(parv[0]) VName(parv[1]));

		if(entity->IsZombie() || entity->Locked())
			return vDebug(W_DESYNCH, "ARM: We can't move this entity!", VPName(entity) VName(parv[0]) VName(parv[1])
			                          VBName(entity->IsZombie()) VIName(entity->Type()) VBName(entity->Locked())
			                          VPName(entity->Parent()));
	}

	uint y = 0, x = 0, type = 0, nb = 0;
	ECMove::Vector moves;
	ECBCase* last_case = (entity ? entity->DestCase() : 0);
	EContainer* container = 0;
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
			case 'C':
				flags |= ARM_RETURN;
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
				container = dynamic_cast<EContainer*>(cl->Player()->Entities()->Find(parv[i].substr(1).c_str()));
				if(entity && container && container->WantContain(entity, moves))
					flags |= ARM_CONTAIN;
				break;
			}
			case '(':
			{
				std::string s = parv[i].substr(1);
				uint _x = StrToTyp<uint>(stringtok(s, ","));
				uint _y = StrToTyp<uint>(s);
				container = entity ? dynamic_cast<EContainer*>(entity) : 0;
				if(!container)
					break;
				ECEntity* contened = dynamic_cast<ECEntity*>(container->Containing());
				if(container && container->Containing() && container->WantUnContain(_x,_y, moves))
				{
					flags |= ARM_UNCONTAIN;
					entity = contened;
					last_case = container->DestCase();
				}
				break;
			}
			case 'U':
			{
				if(entity && entity->MyUpgrade() > 0)
					flags |= ARM_UPGRADE;
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
			ECEntity *my_friend = 0, *creator = 0;
			for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
			{
				if(dynamic_cast<ECEntity*>(*enti)->IsZombie() || (*enti)->Owner() != entity->Owner() ||
				   !(*enti)->Move()->Empty())
					continue;
				if((*enti)->CanCreate(entity) && (!creator || (*enti)->IsBuilding()))
					creator = dynamic_cast<ECEntity*>(*enti);
				if((*enti)->Type() == entity->Type())
					my_friend = dynamic_cast<ECEntity*>(*enti);
			}
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
				ECEvent* event = new ECEvent(ARM_CREATE, entity, entity->Case());
				event->SetNb(entity->Nb());
				event->SetType(entity->Type());
				chan->Map()->AddAnEntity(entity);
				map->AddEvent(event);
				if(entity->Owner())
					entity->Owner()->Events()->Add(event);
				nb = event->Nb();
				entity->AddEvent(flags);
				if(creator)
					creator->Create(entity);
			}
		}
	}
	if(flags)
	{
		std::nrvector<TClient*> recvers;
		std::vector<ECEvent*> events_sended;
		if(flags == ARM_RETURN)
		{
			/*****************************
			*     GESTION DES RETOURS    *
			******************************/
			if(entity->Case() == 0)
				flags = 0;
			else
			{
				entity->CancelEvents();
				x = entity->Case()->X();
				y = entity->Case()->Y();
			}
		}
		if(flags == ARM_NUMBER)
		{
			if(entity->CanBeCreated() && int(entity->Cost()) <= cl->Player()->Money() && entity->AddUnits(entity->InitNb()))
			{
				/*****************************
				*     GESTION DES AJOUTS     *
				******************************/
				cl->Player()->Stats()->created += entity->InitNb();
				cl->Player()->DownMoney(entity->Cost());
				nb = entity->Nb();
			}
			else
				flags = 0;
		}
		if(flags & ARM_UPGRADE)
		{
			/*****************************
			 *    GESTION DES UPGRADES   *
			 *****************************/
			ECEntity* upgrade = CreateAnEntity(entity->MyUpgrade(), entity->ID(), entity->Owner(), entity->Case());
			if(int(upgrade->Cost()) > cl->Player()->Money())
				flags = 0;
			else
			{
				cl->Player()->Stats()->created += upgrade->InitNb();
				cl->Player()->DownMoney(upgrade->Cost());
				Debug(W_DEBUG, "Création d'un evenement d'UPGRADE");
				ECEvent* event = new ECEvent(ARM_UPGRADE, entity, entity->DestCase());
				entity->Lock();
				map->AddEvent(event);
				entity->AddEvent(flags);
				entity->Events()->Add(event);
				if(entity->Owner())
					entity->Owner()->Events()->Add(event);
				flags |= ARM_LOCK;
			}
			/* On supprime l'entité qui a été générée pour vérifier le prix.
			 * On la recréera lors de l'animation si elle est bien construite */
			MyFree(upgrade);
		}
		if(flags & ARM_DEPLOY)
		{
			/*****************************
			 *  GESTION DES DÉPLOIEMENTS *
			 *****************************/

			EventVector evts = entity->Events()->List();
			bool can_create_event = true;
			for(EventVector::iterator evti = evts.begin(); evti != evts.end(); ++evti)
				if((*evti)->Flags() == ARM_DEPLOY)
				{
					can_create_event = false;
					flags = 0;
					break;
				}
			if(can_create_event)
			{
				Debug(W_DEBUG, "Création d'un evenement de déploiement");
				ECEvent* event = new ECEvent(ARM_DEPLOY, entity, entity->DestCase());
				entity->Events()->Add(event);
				map->AddEvent(event);
				if(entity->Owner())
					entity->Owner()->Events()->Add(event);
				entity->AddEvent(flags);
			}
		}
		if(flags & ARM_MOVE && flags != ARM_CREATE)
		{
			/*****************************
			 *  GESTION DES DÉPLACEMENTS *
			 *****************************/
			entity->AddEvent(flags);

			ECEvent* event = entity->Events()->Last();
			if(event && event->Flags() == ARM_MOVE && event->Case() == last_case)
			{ /* On a trouvé un déplacement pour cette même entité alors on en profite */
				for(ECMove::Vector::const_iterator it = moves.begin(); it != moves.end(); ++it)
					event->Move()->AddMove(*it);
				event->SetCase(entity->Move()->Dest());
				event->SetFlags(flags);
				Debug(W_DEBUG, "On a trouvé un déplacement continue au mien");
			}
			else
			{
				Debug(W_DEBUG, "On créé un nouvel event MOVE");
				event = new ECEvent(flags, entity, entity->DestCase());
				event->Move()->SetMoves(moves);
				event->Move()->SetEntity(entity);
				event->Move()->SetFirstCase(last_case);
				map->AddEvent(event);
				entity->AddEvent(flags);
				if(entity->Owner())
					entity->Owner()->Events()->Add(event);
				entity->Events()->Add(event);

			}
			if(container)
			{
				/* On ajoute l'evenement au conteneur histoire qu'il y ait quelque chose
				 * séparant son dernier mouvement et un éventuel nouveau mouvement.
				 */
				event->Entities()->Add(container);
				container->Events()->Add(event);
				flags &= ~ARM_CONTENER; // On ne dit pas au client qu'on va débarquer, car SendArm() cherche Parent(), or on n'est pas encore
				                        // réellement contenu.
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
			std::vector<ECBEntity*> entities = c->Entities()->List();

			Debug(W_DEBUG, "Création d'un evenement ATTAQ");
			ECEvent* event = new ECEvent(ARM_ATTAQ, entity, c);

			/* Si jamais on cherche pas à forcer la case, on recherche sur cette case une unité mobile
			 * que l'on aimerait toucher coute que coute, et que donc, même si lors des animations
			 * elle change de case, on attaquera.
			 * Si il n'y a pas d'unité mobile sur la case, ben ça sera sur cette case qu'on tirera.
			 */
			if(!(flags & ARM_FORCEATTAQ))
				for(std::vector<ECBEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
					if(entity->CanAttaq(*it) && !entity->Like(*it))
					{
						event->Entities()->Add(dynamic_cast<ECEntity*>(*it));
						break;
					}

			map->AddEvent(event);
			if(entity->Owner())
				entity->Owner()->Events()->Add(event);
			entity->AddEvent(flags);
			entity->Events()->Add(event);
		}
		std::vector<ECBPlayer*> allies = cl->Player()->Allies();
		FORit(ECBPlayer*, allies, a)
			if(dynamic_cast<ECPlayer*>(*a)->Client())
				recvers.push_back(dynamic_cast<ECPlayer*>(*a)->Client());

		recvers.push_back(cl);
		chan->SendArm(recvers, entity, flags, x, y, 0, events_sended);

		if(flags == ARM_CREATE)
			entity->Created(true); // callback, et true veut dire que ça a été créé tout de suite

	}

	return 0;
}

/** Add a breakpoint on the map.
 *
 * Syntax: BP [+|-]x,y [message]
 */
int BPCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(!cl->Player() || cl->Player()->Channel()->State() != EChannel::PLAYING)
		return vDebug(W_DESYNCH, "BP: Le joueur n'est pas dans une partie, ou alors la partie n'est pas +P(laying)",
		              VPName(cl->Player()));

	EChannel* chan = cl->Player()->Channel();
	ECMap *map = dynamic_cast<ECMap*>(chan->Map());
	char ch = parv[1][0];
	std::string s = parv[1].substr(1);
	uint x = StrToTyp<uint>(stringtok(s, ","));
	uint y = StrToTyp<uint>(s);

	ECBCase *c;
	try
	{
		c = (*map)(x,y);
	}
	catch(TECExcept &e)
	{
		Debug(W_DESYNCH, "BP: %s a voulu faire un breakpoint hors de la map (%d,%d)", cl->GetNick(), x, y);
		return cl->sendrpl(ERR_UNKNOWN);
	}

	std::string message = parv.size() > 2 ? parv[2] : "";

	switch(ch)
	{
		case '+':
		{
			std::vector<ECPlayer::BreakPoint> bp = cl->Player()->BreakPoints();
			for(std::vector<ECPlayer::BreakPoint>::iterator it = bp.begin(); it != bp.end(); ++it)
				if(it->c == c)
					return cl->sendrpl(ERR_UNKNOWN);

			cl->Player()->AddBreakPoint(ECPlayer::BreakPoint(c, message));

			break;
		}
		case '-':
		{
			if(!cl->Player()->RemoveBreakPoint(c))
				return cl->sendrpl(ERR_UNKNOWN);

			break;
		}
		default: return vDebug(W_WARNING, "BP: Invalide identifiant", VCName(ch) VName(parv[1]));
	}

	std::vector<TClient*> allies = cl->Player()->ClientAllies();
	for(std::vector<TClient*>::iterator it = allies.begin(); it != allies.end(); ++it)
		cl->sendrpl(cl->Nick(), MSG_BREAKPOINT, ECArgs(ch + TypToStr(c->X()) + "," + TypToStr(c->Y()), message));

	return 0;
}
