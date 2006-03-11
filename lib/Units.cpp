/* lib/Units.cpp - Units in game
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

bool ECBArmy::Return()
{
	if(!last) return false;

	acase->Entities()->Remove(this);
	acase = last->Case();
	acase->Entities()->Add(this);

	last->Case()->Entities()->Remove(last);
	MyFree(last);

	return true;
}

void ECBArmy::CreateLast()
{
	if(!last)
	{
		ECBEntity* e = new ECBArmy(*this);
		e->SetLock();
		SetLast(e);
		e->Case()->Entities()->Add(e); /* On rajoute cette entité locked à la nouvelle case */
	}
}

ECBCase* ECBArmy::Attaq(uint mx, uint my)
{
	ECBCase* c;
	if((c = CheckMove(mx, my)))
	{
		if(c->Entities()->empty())
			return 0;

		std::vector<ECBEntity*> ents = c->Entities()->List();
		std::vector<ECBEntity*>::iterator enti;
		for(enti = ents.begin(); enti != ents.end() &&
		          (!CanAttaq(*enti) || (*enti)->Owner() == owner);
		    ++enti);

		if(enti == ents.end())
			return 0;

		ECBCase* last_c = acase;
		CreateLast();
		ChangeCase(c); /* On se positionne sur la nouvelle case */
		return last_c;
	}
	return 0;
}

ECBCase* ECBArmy::CheckMove(uint mx, uint my)
{
	if(mx == acase->X() && my == acase->Y() ||
	   mx >= acase->Map()->Width() || my >= acase->Map()->Height() ||
	   int(acase->X() - mx) > (int)restStep || int(mx - acase->X()) > (int)restStep ||
	   int(acase->Y() - my) > (int)restStep || int(my - acase->Y()) > (int)restStep)
		return 0;

	uint nb_mov = 0;
	ECBCase *c = acase;

	for(;c->X() != mx && (c->Flags() & (C_VILLE|C_TERRE|C_PONT)) && nb_mov < restStep;nb_mov++)
		if(c->X() < mx) c = c->MoveRight();
		else c = c->MoveLeft();
	if(c->X() != mx) return 0;

	for(;c->Y() != my && (c->Flags() & (C_VILLE|C_TERRE|C_PONT)) && nb_mov < restStep;nb_mov++)
		if(c->Y() < my) c = c->MoveDown();
		else c = c->MoveUp();
	if(c->Y() != my) return 0;

	if(last && c == last->Case()) return 0;
	if(!(c->Flags() & (C_VILLE|C_TERRE|C_PONT))) return 0;

	restStep -= nb_mov;

	return c;
}

ECBCase* ECBArmy::Move(uint mx, uint my)
{
	ECBCase *c;

	if(!(c = CheckMove(mx, my))) return 0;

	ECBCase* last_c = acase;
	CreateLast();
	ChangeCase(c);

	return last_c;
}

void ECBArmy::Union(ECBEntity* entity)
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
