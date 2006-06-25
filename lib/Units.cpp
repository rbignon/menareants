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

ECBContainer::~ECBContainer()
{

}

bool ECBContainer::Contain(ECBEntity* entity)
{
	if(!entity || Containing())
		return false;

	entity->ChangeCase(0);
	entity->SetCase(Case());

	SetContaining(entity);
	entity->Lock();
	entity->SetParent(this);

	return true;
}

bool ECBContainer::UnContain()
{
	if(!Containing())
		return false;

	Containing()->Unlock();
	Case()->Entities()->Add(Containing());
	Containing()->SetParent(0);
	Containing()->SetCase(0);
	Containing()->ChangeCase(Case());

	SetContaining(0);

	return true;
}
