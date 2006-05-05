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
	next = this;
	ECEntity* e = new EChar(*this);
	next = 0;
	e->SetLock();
	SetLast(e);
	e->Case()->Entities()->Add(e); /* On rajoute cette entit� locked � la nouvelle case */
}

/********************************************************************************************
 *                               ECArmy                                                     *
 ********************************************************************************************/

void ECArmy::CreateLast()
{
	next = this;
	ECEntity* e = new ECArmy(*this);
	next = 0;
	e->SetLock();
	SetLast(e);
	e->Case()->Entities()->Add(e); /* On rajoute cette entit� locked � la nouvelle case */
}

/********************************************************************************************
 *                               ECUnit                                                     *
 ********************************************************************************************/

bool ECUnit::WantAttaq(uint mx, uint my)
{
	/* On ne peut attaquer que si on est sur la case */
	if(Case()->X() != mx || Case()->Y() != my)
		return false;

	/* Si il n'y a personne � attaquer on n'attaque pas */
	if(Case()->Entities()->size() <= 1) // <=1 car je suis forc�ment dans la liste vu que je suis sur la case
		return 0;

	std::vector<ECBEntity*> ents = Case()->Entities()->List();
	std::vector<ECBEntity*>::iterator enti;
	for(enti = ents.begin(); enti != ents.end() &&
		((*enti)->Locked() || !CanAttaq(*enti) || Like(*enti));
		++enti);

	if(enti == ents.end())
		return false;

	return true;
}

bool ECUnit::WantMove(ECBMove::E_Move move)
{
	/* J'ai d�j� fait tous mes pas*/
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

	if(!(c->Flags() & case_flags)) return false;

	restStep--;

	/* Si sur la case actuelle il y a une attaque, on ne bouge pas meme si on dit
	 * que si au client.
	 */
	std::vector<ECBEntity*> ents = Case()->Entities()->List();
	for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
		if((*enti)->MyStep() - (*enti)->RestStep() == this->MyStep() - this->RestStep() &&
		   ThereIsAttaq(*enti, this))
			return true;

	CreateLast();
	ChangeCase(c);

	return true;
}
