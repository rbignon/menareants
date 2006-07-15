/* server/Batiments.cpp - Buildings in server
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

#include "Batiments.h"
#include "Channels.h"
#include "Debug.h"

/********************************************************************************************
 *                               ECNuclearSearch                                            *
 ********************************************************************************************/

void ECNuclearSearch::Played()
{
	restBuild--;
	if(!restBuild)
	{
		restBuild = NUCLEARSEARCH_BUILDTIME;
		missiles++;
		Channel()->SendArm(0, this, ARM_DATA, 0,0, ECData(DATA_NBMISSILES, TypToStr(missiles)));
	}
	if(Owner()->Client())
		Channel()->SendArm(Owner()->Client(), this, ARM_DATA, 0,0, ECData(DATA_RESTBUILD, TypToStr(restBuild)));
	ECEntity::Played();
}

void ECNuclearSearch::RemoveOneMissile()
{
	Missiles()--;
	if(Owner()->Client())
		Channel()->SendArm(Owner()->Client(), this, ARM_DATA, 0,0, ECData(DATA_NBMISSILES, TypToStr(Missiles())));
}

/********************************************************************************************
 *                                        ECSilo                                            *
 ********************************************************************************************/

bool ECSilo::WantAttaq(uint mx, uint my, bool force)
{
	if(!force && (!NuclearSearch() || !NuclearSearch()->Missiles()))
		return false;

	/* On n'attaque pas sur notre case */
	if(Case()->X() == mx && Case()->Y() == my)
		return false;

	uint d = 0;
	for(uint x=Case()->X(); x != mx; d++) x < mx ? ++x : --x;
	for(uint y=Case()->Y(); y != my; d++) y < my ? ++y : --y;

	/* On ne tire que dans un rayon de SILO_PORTY (10?) cases. */
	if(d > Porty())
		return false;

	if(!force)
		dynamic_cast<ECNuclearSearch*>(NuclearSearch())->RemoveOneMissile();

	return true;
}

bool ECSilo::Attaq(std::vector<ECEntity*> entities, ECEvent* event)
{
	/* C'est une attaque contre moi (probablement sur la meme case). */
	if(!(EventType() & ARM_ATTAQ))
		return ECEntity::Attaq(entities, event);

	ECBCase* c = event->Case();
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		if(*it == this) continue;
		if((*it)->IsCountryMaker())
			Shoot(*it, 1002);
		else if((*it)->Type() == E_NUCLEARSEARCH)
			Shoot(*it, 30);
		else
			Shoot(*it, (*it)->RealNb());
	}

	ECBCase* this_c = c;
	c = c->MoveLeft(SILO_IMPACT)->MoveUp(SILO_IMPACT);

	std::nrvector<TClient*> receivers = ECEntity::EntitiesToClients(entities);

	for(uint i=0; i <= 2*SILO_IMPACT; ++i)
	{
		uint j=0;
		for(; j <= 2*SILO_IMPACT; ++j)
		{
			if(c != this_c)
			{
				std::vector<ECBEntity*> ents = c->Entities()->List();
				for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
				{
					if((*enti)->IsCountryMaker() || (*enti)->Shadowed() || (*enti)->Type() == E_NUCLEARSEARCH)
						continue;
					ECEntity* entity = dynamic_cast<ECEntity*>(*enti);
					if(entity->IsBuilding())
						Shoot(entity, entity->InitNb()/2+21);
					else
						Shoot(entity, entity->RealNb());

					entity->ReleaseShoot();
					if(!entity->Nb())
					{
						entity->SetShadowed();
						Channel()->SendArm(0, entity, ARM_REMOVE);
					}
					else
					{
						if(entity->Owner() && entity->Owner()->Client())
							receivers.push_back(entity->Owner()->Client());
						Channel()->SendArm(receivers, entity, ARM_NUMBER);
					}
				}
			}

			if(c->X() == c->Map()->Width()-1)
				break;
			c = c->MoveRight();
		}
		if(c->Y() == c->Map()->Height()-1)
			break;
		c = c->MoveDown();
		c = c->MoveLeft(j);
	}

	return false;
}

