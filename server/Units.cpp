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

/********************************************************************************************
 *                               EChar                                                      *
 ********************************************************************************************/

void EChar::CreateLast()
{
	ECEntity* e = new EChar(*this);
	e->SetLock();
	SetLast(e);
	e->Case()->Entities()->Add(e); /* On rajoute cette entité locked à la nouvelle case */
}

bool EChar::WantAttaq(uint mx, uint my)
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

bool EChar::WantMove(ECBMove::E_Move move)
{
	/* J'ai déjà fait tous mes pas*/
	if(!restStep) return false;

	ECBCase *c = 0;

	switch(move)
	{
		case ECBMove::Up: c = Case()->MoveUp(); break;
		case ECBMove::Down: c = Case()->MoveDown(); break;
		case ECBMove::Left: c = Case()->MoveLeft(); break;
		case ECBMove::Right: c = Case()->MoveRight(); break;
	}
	if(!c || c == Case() || last && last->Case() == c) return false;

	if(!(c->Flags() & (C_VILLE|C_TERRE))) return false;

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

/********************************************************************************************
 *                               ECArmy                                                     *
 ********************************************************************************************/

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

bool ECArmy::WantMove(ECBMove::E_Move move)
{
	/* J'ai déjà fait tous mes pas*/
	if(!restStep) return false;

	ECBCase *c = 0;

	switch(move)
	{
		case ECBMove::Up: c = Case()->MoveUp(); break;
		case ECBMove::Down: c = Case()->MoveDown(); break;
		case ECBMove::Left: c = Case()->MoveLeft(); break;
		case ECBMove::Right: c = Case()->MoveRight(); break;
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

