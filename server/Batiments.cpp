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
 *                                        ECDefenseTower                                    *
 ********************************************************************************************/

bool ECDefenseTower::WantAttaq(uint mx, uint my, bool force)
{
	if(!force && (EventType() & ARM_ATTAQ))
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

	return true;
}

bool ECDefenseTower::Attaq(std::vector<ECEntity*> entities, ECEvent* event)
{
	/* C'est une attaque contre moi (probablement sur la meme case). */
	if(!(EventType() & ARM_ATTAQ))
		return ECEntity::Attaq(entities, event);

	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(*it != this && !Like(*it) && CanAttaq(*it) && (*it)->Case() != Case())
		{
			uint dx = 0, dy = 0;
			for(uint x=Case()->X(); x != (*it)->Case()->X(); dx++) x < (*it)->Case()->X() ? ++x : --x;
			for(uint y=Case()->Y(); y != (*it)->Case()->Y(); dy++) y < (*it)->Case()->Y() ? ++y : --y;
			uint d = dx + dy;
			float coef = 0;
			switch(d)
			{
				case 0: coef = 0.0f; break;
				case 1: coef = 0.8f; break;
				case 2: coef = 2.0f; break;
				case 3: coef = 1.4f; break;
				case 4: coef = 0.9f; break;
				default: coef = 0;
			}
			uint killed = 0;
			if((*it)->IsInfantry())                          killed = uint(300 * coef);
			else if((*it)->IsVehicule() || (*it)->IsNaval()) killed = uint(400 * coef);
			else
			{
				FDebug(W_WARNING, "Shoot d'un type non support�");
				continue;
			}
			if(!killed) continue;

			Shoot(*it, killed);
			(*Channel()) << "La tour de d�fense " + LongName() + " d�gomme " + (*it)->Qual() + " " +
							(*it)->LongName() + " de " + TypToStr(killed);
		}


	return false;
}



/********************************************************************************************
 *                                        ECMine                                            *
 ********************************************************************************************/

void ECMine::Played()
{
	if(restBuild)
	{
		restBuild--;
		if(Owner()->Client())
		{
			std::vector<TClient*> sdrs = Owner()->ClientAllies();
			sdrs.push_back(Owner()->Client());
			Channel()->SendArm(sdrs, this, ARM_DATA, 0,0, ECData(DATA_RESTBUILD, TypToStr(restBuild)));
		}
	}
	ECEntity::Played();
}

bool ECMine::Attaq(std::vector<ECEntity*> entities, ECEvent* event)
{
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		if(*it == this) continue;
		Shoot(*it, 998);
	}
	/* Auto destruction */
	Shoot(this, Nb());

	return false;
}

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

