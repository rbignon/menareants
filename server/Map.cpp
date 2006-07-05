/* server/Map.cpp - Map classes
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

#include "Defines.h"
#include "Map.h"
#include "Debug.h"
#include "Channels.h"
#include "Main.h"
#include "Units.h"
#include <fstream>
#include <algorithm>

MapVector MapList;

/********************************************************************************************
 *                                 ECMap                                                    *
 ********************************************************************************************/
 
bool LoadMaps()
{
	std::ifstream fp(MAP_FILE);

	if(!fp)
	{
		Debug(W_ERR|W_ECHO, "Unable to load map list files: %s", MAP_FILE);
		return false;
	}

	std::string ligne;
	uint nbmaps = 0;

	while(std::getline(fp, ligne))
	{
		if(ligne[0] == '#' || ligne[0] == '\0') continue;
		ECMap *map = 0;
		try
		{
			map = new ECMap(std::string(PKGDATADIR + ligne), nbmaps);
			map->Init();
		}
		catch(TECExcept &e)
		{
			delete map;
			Debug(W_ERR|W_ECHO, "Unable to load this map file : %s", ligne.c_str());
			vDebug(W_ERR|W_ECHO, e.Message(), e.Vars());
			continue;
		}
		nbmaps++;
		MapList.push_back(map);
	}
	Debug(W_ECHO|W_NOLOG, "%d maps loaded !", nbmaps);
	return true;
}

ECMap::~ECMap()
{
	for(EventVector::iterator it = map_events.begin(); it != map_events.end(); ++it)
		delete *it;
}

EventVector::iterator ECMap::RemoveEvent(ECEvent* p, bool use_delete)
{
	for(EventVector::iterator it = map_events.begin(); it != map_events.end();)
	{
		if (*it == p)
		{
			if(use_delete)
				delete p;
			it = map_events.erase(it);
			return it;
		}
		else
			++it;
	}
	return EventVector::iterator(0);
}

ECEvent* ECMap::FindEvent(ECase* c, uint f, ECEntity* e)
{
	for(EventVector::iterator it = map_events.begin(); it != map_events.end(); ++it)
		if((!c || (*it)->Case() == c) && (!f || (*it)->Flags() == f) && (!e || (*it)->Entities()->Find(e)))
			return *it;
	return 0;
}

struct SortEventsFunction
{
	bool operator ()(ECEvent* const& a1, ECEvent* const& a2) const
	{
		return (*a2) < (*a1);
	}
};
void ECMap::SortEvents()
{
	std::sort(map_events.begin(), map_events.end(), SortEventsFunction());
}

/********************************************************************************************
 *                                 ECountry                                                 *
 ********************************************************************************************/

bool ECountry::ChangeOwner(ECBMapPlayer* mp)
{
	ECBMapPlayer* last_owner = owner;
	if(ECBCountry::ChangeOwner(mp))
	{
		for(BCaseVector::iterator c = cases.begin(); c != cases.end(); ++c)
		{
			std::vector<ECBEntity*> ents = (*c)->Entities()->List();
			for(std::vector<ECBEntity*>::iterator it = ents.begin(); it != ents.end(); ++it)
				if((*it)->IsCountryMaker())
				{
					EChannel* c = dynamic_cast<EChannel*>(Map()->Channel());
					ECEntity* et = dynamic_cast<ECEntity*>(*it);
					c->SendArm(NULL, et, ARM_REMOVE);
					et->SetOwner(owner ? owner->Player() : 0);
					if(last_owner && last_owner->Player())
						last_owner->Player()->Entities()->Remove(et);
					else
						c->Map()->Neutres()->Remove(et);
					if(owner && owner->Player())
						owner->Player()->Entities()->Add(et);
					else
						c->Map()->Neutres()->Add(et);
					et->SetID(c->FindEntityName(owner ? dynamic_cast<ECPlayer*>(owner->Player()) : 0));
					c->SendArm(NULL, et, ARM_CREATE|ARM_HIDE, (et)->Case()->X(), (et)->Case()->Y());
				}
		}
		if(owner && owner->Player())
		{
			EChannel* chan = dynamic_cast<EChannel*>(owner->Player()->Channel());
			chan->sendto_players(0, app.rpl(ECServer::SET), owner->Player()->GetNick(),
									std::string(std::string("+@ ") + ident).c_str());
		}
		else if(last_owner && last_owner->Player())
		{
			EChannel* chan = dynamic_cast<EChannel*>(last_owner->Player()->Channel());
			chan->sendto_players(0, app.rpl(ECServer::SET), last_owner->Player()->GetNick(),
									std::string(std::string("-@ ") + ident).c_str());
		}
		return true;
	}
	return false;
}

/********************************************************************************************
 *                                 ECEvent                                                  *
 ********************************************************************************************/

bool ECEvent::RemoveLinked(ECEvent* p, bool use_delete)
{
	for(EventVector::iterator it = linked.begin(); it != linked.end();)
	{
		if (*it == p)
		{
			if(use_delete)
				delete p;
			it = linked.erase(it);
			return true;
		}
		else
			++it;
	}
	return false;
}

bool ECEvent::operator<(const ECEvent& e) const
{
	const char CREATE = 12;
	const char REPLOY = 10;
	const char CONTAIN= 9;
	const char MOVE   = 8;
	const char UNCONTAIN= 7;
	const char UNION  = 5;
	const char DEPLOY = 3;
	const char ATTAQ  = 1;
	char me = 0, him = 0;
	switch(flags)
	{
		case ARM_CREATE: me = CREATE; break;
		case ARM_UNION: me = UNION; break;
		case ARM_DEPLOY: me = Entity()->Deployed() ? DEPLOY : REPLOY; break;
		case ARM_CONTAIN: me = CONTAIN;
		case ARM_UNCONTAIN: me = UNCONTAIN;
		case ARM_ATTAQ: me = ATTAQ; break;
		case ARM_MOVE: me = MOVE; break;
		default: me = 0; break;
	}
	switch(e.Flags())
	{
		case ARM_CREATE: him = CREATE; break;
		case ARM_UNION: him = UNION; break;
		case ARM_ATTAQ: him = ATTAQ; break;
		case ARM_DEPLOY: him = e.Entity()->Deployed() ? DEPLOY : REPLOY; break;
		case ARM_CONTAIN: him = CONTAIN;
		case ARM_UNCONTAIN: him = UNCONTAIN;
		case ARM_MOVE: him = MOVE; break;
		default: him = 0; break;
	}
	if(me == CONTAIN && him == MOVE && Entity()->Parent() && Entity()->Parent() == dynamic_cast<EContainer*>(e.Entity()))
	{
		if(dynamic_cast<EContainer*>(e.Entity())->Containing())
			return false;
		else
			return true;
	}
	if(me == UNCONTAIN && him == MOVE && dynamic_cast<EContainer*>(e.Entity()))
	{
		if(dynamic_cast<EContainer*>(e.Entity())->Containing())
			return true;
		else
			return false;
	}
	if(him == CONTAIN && me == MOVE && e.Entity()->Parent() && e.Entity()->Parent() == dynamic_cast<EContainer*>(Entity()))
	{
		if(dynamic_cast<EContainer*>(Entity())->Containing())
			return true;
		else
			return false;
	}
	if(him == UNCONTAIN && me == MOVE && dynamic_cast<EContainer*>(Entity()))
	{
		if(dynamic_cast<EContainer*>(Entity())->Containing())
			return false;
		else
			return true;
	}
	
	return (me < him);
}

bool ECEvent::CheckRemoveBecauseOfPartOfAttaqEntity(ECEntity* entity)
{
	/* Mauvais appel */
	if(flags != ARM_ATTAQ) return false;

	EChannel* chan = dynamic_cast<EChannel*>(entity->Owner()->Channel());
	ECMap* map = dynamic_cast<ECMap*>(chan->Map());

	bool delete_event = true;
	Debug(W_DEBUG, "Retrait d'attaquants");
	Entities()->Remove(entity);
	std::vector<ECEntity*> ents = Entities()->List();

	/* La première étape est de virer tous les attaquants qui n'ont plus rien à attaquer, et de passer ceux qui souhaitent
	 * maintenir leur attaque ET qui tiennent encore l'unité qui se retire dans dans leur portée, de changer l'orientation
	 * de leur attaque et de passer l'attaque sur une autre case.
	 * Par la suite, on recherche les déplacements faits vers la case de l'attaque et qui ne cherchent aucunement
	 * à se faire sodomiser l'anus par un bétail. Si jamais après le retrait de cette entité il n'y a plus d'attaque entre
	 * lui et les autres, on le retire de la liste des attaquants et on met son mouvement, s'il existe, dans la liste globale.
	 */
	const char T_ATTAQ_STEP = 0;
	const char T_MOVE_STEP = 1;
	const char T_END_STEP = 2;
	for(char step = T_ATTAQ_STEP; step != T_END_STEP; ++step)
		for(std::vector<ECEntity*>::iterator enti = ents.begin(); enti != ents.end();)
		{
			if((*enti)->Shadowed())
			{
				++enti;
				continue;
			}

			if(step == T_ATTAQ_STEP && (*enti)->EventType() & ARM_ATTAQ && !((*enti)->EventType() & ARM_FORCEATTAQ) &&
			   entity->Case() != Case() && (*enti)->WantAttaq(entity->Case()->X(), entity->Case()->Y()))
			{
				Debug(W_DEBUG, "On déokace notre évenement");
				ECEvent* attaq_event = dynamic_cast<ECMap*>(Case()->Map())->FindEvent(entity->Case(), ARM_ATTAQ);
				if(!attaq_event)
				{
					Debug(W_DEBUG, "Création d'un evenement ATTAQ issue d'un déplacement de cible");
					attaq_event = new ECEvent(ARM_ATTAQ, entity->Case());
					dynamic_cast<ECMap*>(Case()->Map())->AddEvent(attaq_event);
					std::vector<ECBEntity*> entities = entity->Case()->Entities()->List();
					for(std::vector<ECBEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
						if(!dynamic_cast<ECEntity*>(*it)->Shadowed() && *it != *enti && (*enti)->CanAttaq(*it) &&
						   !(*enti)->Like(*it))
							attaq_event->Entities()->Add(dynamic_cast<ECEntity*>(*it));
				}
				attaq_event->Entities()->Add(*enti);

				std::vector<ECEvent*>::iterator ev;
				for(ev = this->linked.begin(); ev != this->linked.end() && (*ev)->Entity() != *enti; ++ev);
				if(ev != this->linked.end())
				{
					attaq_event->AddLinked(*ev);
					ev = this->linked.erase(ev);
				}

				this->Entities()->Remove(*enti);
				enti = ents.erase(enti);
				continue;
			}

			/* Pour cette entité, on cherche si il peut encore attaquer quelqu'un d'autre, sinon il jarte */
			std::vector<ECEntity*>::iterator enti2;
			for(enti2 = ents.begin();
				enti2 != ents.end() && (*enti2 == *enti || (*enti2)->Shadowed() ||
					(!(*enti)->CanAttaq(*enti2) || (*enti)->Like(*enti2)) &&
					(step != T_MOVE_STEP || !(*enti2)->CanAttaq(*enti) || (*enti2)->Like(*enti)));
				++enti2);

			/* Il n'y a personne qu'il veut/peut attaquer et personne qui veut/peut l'attaquer */
			if(enti2 == ents.end())
			{
				if(step == T_ATTAQ_STEP && (*enti)->EventType() & ARM_ATTAQ && !((*enti)->EventType() & ARM_FORCEATTAQ))
				{ /* C'était un attaquant, on lui fait faire un Return */
					Debug(W_DEBUG, "il y a un attaquant qui se trouve fort sodomisé.");
					if((*enti)->Return() && (*enti)->Owner())
						chan->SendArm(dynamic_cast<ECPlayer*>((*enti)->Owner())->Client(), *enti,
										ARM_RETURN, (*enti)->Case()->X(), (*enti)->Case()->Y());

					/* On supprime l'evenement de mouvement linké */
					std::vector<ECEvent*>::iterator ev;
					for(ev = this->linked.begin(); ev != this->linked.end() && (*ev)->Entity() != *enti; ++ev);
					if(ev != this->linked.end())
					{
						delete *ev;
						ev = this->linked.erase(ev);
					}
				}
				else if(step == T_MOVE_STEP && (*enti)->EventType() == ARM_MOVE)
				{ /* C'était un mouvement innocent, on remet juste son evenement en lieu commun */
					std::vector<ECEvent*>::iterator ev;
					for(ev = this->linked.begin(); ev != this->linked.end() && (*ev)->Entity() != *enti; ++ev);
					if(ev != this->linked.end())
					{
						map->AddEvent(*ev);
						ev = this->linked.erase(ev);
					}
				}
				else
				{
					++enti;
					continue;
				}
				this->Entities()->Remove(*enti);
				enti = ents.erase(enti);
			}
			else
			{
				/* Cet evenement réuni encore des unités qui peuvent se battre */
				delete_event = false;
				++enti;
			}
		}
#ifdef DEBUG
	if(delete_event)
		Debug(W_DEBUG, "On supprime bien l'evenement");
	else
		Debug(W_DEBUG, "On ne supprime pas, doit y avoir d'autres unités combatantes");
#endif
	return delete_event;
}

/********************************************************************************************
 *                                 ECEntity                                                 *
 ********************************************************************************************/

bool ECEntity::AreFriends(std::vector<ECEntity*> list)
{
	for(std::vector<ECEntity*>::iterator it = list.begin(); it != list.end(); ++it)
		for(std::vector<ECEntity*>::iterator ti = list.begin(); ti != list.end(); ++ti)
			if(!(*ti)->Like(*it) && !(*it)->Like(*ti))
				return false;
	return true;
}

void ECEntity::Played()
{
	ECBEntity::Played();
	RemoveLast();
	Move()->Clear(Case());
}

ECEntity::~ECEntity()
{
	RemoveLast();
}

void ECEntity::Shoot(ECEntity* e, uint k)
{
	if(Owner())
		dynamic_cast<ECPlayer*>(Owner())->Stats()->shooted += k;

	Debug(W_DEBUG, "%s shoot %s de %d", LongName().c_str(), e->LongName().c_str(), k);
	e->Shooted(k);
}

void ECEntity::ReleaseShoot()
{
	nb -= (shooted > nb ? nb : shooted);
	if(Owner())
		Owner()->Stats()->killed += shooted;
	shooted = 0;
}

ECPlayer* ECEntity::Owner() const
{
	return dynamic_cast<ECPlayer*>(ECBEntity::Owner());
}

EChannel* ECEntity::Channel() const
{
	assert(Map());
	assert(Map()->Channel());
	return dynamic_cast<EChannel*>(Map()->Channel());
}

void ECEntity::RemoveLast()
{
	if(!last) return;

	//last->RemoveLast();
	last->Case()->Entities()->Remove(last);
	last->Case()->Map()->Entities()->Remove(last);

	// Le destructeur va appeler le RemoveLast() de l'entité last
	MyFree(last);
}

ECEntity* ECEntity::FindLast(ECBCase* c)
{
	for(ECEntity* a_l = this; a_l; a_l = a_l->Last())
		if(a_l->Case() == c)
			return a_l;
	return 0;
}

ECEntity* ECEntity::FindNext()
{
	ECEntity* a_l = this;
	for(; a_l->Next(); a_l = a_l->Next());
	return a_l;
}

void ECEntity::Union(ECEntity* entity)
{
	if(entity->Type() != Type()) throw ECExcept(VIName(entity->Type()) VIName(Type()), "Union avec un autre type !?");

	/* On lock car il fait maintenant partie intégrale de (*enti) */
	entity->SetShadowed();
	/* On créé un clone de l'ancienne entité. */
	CreateLast();
	/* On met dans le nouvel etat de l'entité le nouveau nombre de soldats */
	SetNb(Nb() + entity->Nb());
	/* Enfin on défini le nombre de pas restants */
	SetRestStep(RestStep() > entity->RestStep() ? RestStep() - entity->RestStep() : 0);
}

bool ECEntity::Return(ECBCase* c)
{
	/* Si il n'y a pas de last on a rien à faire */
	if(!last) return false;

	/* La récurence de Return() s'arrête jusqu'à un changement d'etat
	 * autre que la position, ou si, dans le cas d'une case spécifiée,
	 * on a atteint le last qui est sur cette case.
	 */
	if(!c && last->Case() == Case() || c == Case()) return false;

	/* On appel le Return() du last */
	last->Return(c);

	ECEntity* lastlast = last;

	if(last)
	{
		move = *(last->Move());
	}

	/* On prend comme last le last de notre last */
	last = lastlast->Last();
	lastlast->last = 0;

	/* On récupère les pas */
	if(last && last->Case() != acase)
		restStep = !last ? myStep : (last->RestStep()) ? last->RestStep() - 1 : 0;

	if(last)
		last->next = this;

	/* On s'enlève de la case où on est actuellement et on se met sur l'ancienne case */
	acase->Entities()->Remove(this);

	acase = lastlast->Case();
	acase->Entities()->Add(this);

	/* On enlève le last de la case */
	lastlast->Case()->Entities()->Remove(lastlast);

	/* On supprime le last */
	MyFree(lastlast);

	return true;
}

bool ECEntity::Attaq(std::vector<ECEntity*> entities, ECEvent*)
{
	uint enemies = 0;
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(this != *it && (*it)->Case() == Case() && !Like(*it) && (*it)->Nb() &&
		   (CanAttaq(*it) || Parent() && Parent()->CanAttaq(*it)))
			enemies++;

	if(!enemies) return false;

	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(*it != this && (*it)->Case() == Case() && !Like(*it) && CanAttaq(*it))
		{
			uint killed = rand() % (nb/2+enemies);
			if(killed < nb/(4+enemies)) killed = nb/(4+enemies);
			Shoot(*it, killed);
		}

	return true;
}

std::nrvector<TClient*> ECEntity::EntitiesToClients(std::vector<ECEntity*> entities)
{
	std::nrvector<TClient*> players;
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if((*it)->Owner() && (*it)->Owner()->Client())
			players.push_back((*it)->Owner()->Client());
	return players;
}
