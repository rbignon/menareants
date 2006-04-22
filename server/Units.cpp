/* server/Units.cpp - Units in server
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

#include "Units.h"
#include "Debug.h"
#include "Channels.h"

bool ECArmy::Return()
{
	/* Si il n'y a pas de last on a rien à faire */
	if(!last) return false;

	/* La récurence de Return() s'arrête jusqu'à un changement d'etat
	 * autre que la position */
	if(last->Case() == acase) return false;

	/* On appel le Return() du last */
	last->Return();

	/* On recule (forcément, car il y a un Last par pas) */
	restStep++;

	ECEntity* lastlast = last;

	/* On prend comme last le last de notre last */
	last = lastlast->Last();

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

void ECArmy::CreateLast()
{
	ECEntity* e = new ECArmy(*this);
	e->SetLock();
	SetLast(e);
	e->Case()->Entities()->Add(e); /* On rajoute cette entité locked à la nouvelle case */
}

bool ECArmy::WantAttaq(uint mx, uint my)
{
	/* On ne peut attaquer que si on est sur la case */
	if(Case()->X() != mx || Case()->Y() != my)
		return false;

	/* Si il n'y a personne à attaquer on n'attaque pas */
	if(Case()->Entities()->size() <= 1) // <=1 car je suis forcément dans la liste vu que je suis sur la case
		return 0;

	std::vector<ECBEntity*> ents = Case()->Entities()->List();
	std::vector<ECBEntity*>::iterator enti;
	for(enti = ents.begin(); enti != ents.end() &&
		(!CanAttaq(*enti) || Like(*enti));
		++enti);

	if(enti == ents.end())
		return false;

	return true;
}

bool ECArmy::WantMove(ECMove::E_Move move)
{
	/* J'ai déjà fait tous mes pas*/
	if(!restStep) return false;

	ECBCase *c = 0;

	switch(move)
	{
		case ECMove::Up: c = Case()->MoveUp(); break;
		case ECMove::Down: c = Case()->MoveDown(); break;
		case ECMove::Left: c = Case()->MoveLeft(); break;
		case ECMove::Right: c = Case()->MoveRight(); break;
	}
	if(!c || c == Case() || last && last->Case() == c) return false;

	if(!(c->Flags() & (C_VILLE|C_TERRE|C_PONT))) return false;

	restStep--;

	/* Si sur la case actuelle il y a une attaque, on ne bouge pas meme si on dit
	 * que si au client.
	 * Note: Si je n'ai encore jamais bougé, je peux quand meme bougé si je suis attaqué (evite l'attaque)
	 */
	if(Last() && Last()->Case() != acase)
	{
		std::vector<ECBEntity*> ents = Case()->Entities()->List();
		for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
			if(ThereIsAttaq(*enti, this))
				return true;
	}

	CreateLast();
	ChangeCase(c);

	return true;
}

void ECArmy::Union(ECEntity* entity)
{
	if(entity->Type() != Type()) throw ECExcept(VIName(entity->Type()) VIName(Type()), "Union avec un autre type !?");

	/* On lock car il fait maintenant partie intégrale de (*enti) */
	entity->SetLock();
	/* On créé un clone de l'ancienne entité. */
	CreateLast();
	/* On met dans le nouvel etat de l'entité le nouveau nombre de soldats */
	SetNb(Nb() + entity->Nb());
	/* Enfin on défini le nombre de pas restants */
	SetRestStep(entity->RestStep());
}

bool ECArmy::Attaq(std::vector<ECEntity*> entities)
{
	uint enemies = 0;
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
			if(this != *it && !(*it)->Locked() && (!(*it)->Like(this) || !Like(*it)) &&
			                                   ((*it)->CanAttaq(this) || CanAttaq(*it)))
				enemies++;

	if(!enemies) return true;

	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(*it != this && !(*it)->Locked() && !Like(*it) && CanAttaq(*it))
		{
			uint killed = rand() % (nb/2+enemies);
			if(killed < nb/(4+enemies)) killed = nb/(4+enemies);
			(*it)->Shooted(killed);
			printf("%s shoot %s de %d\n", LongName().c_str(), (*it)->LongName().c_str(), killed);
		}

	return true;
}
