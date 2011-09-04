/* server/Batiments.cpp - Buildings in server
 *
 * Copyright (C) 2005-2011 Romain Bignon  <romain@menareants.org>
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
#include <stdlib.h>

/********************************************************************************************
 *                                        ECavern                                           *
 ********************************************************************************************/
bool ECavern::Contain(ECBEntity* entity)
{
	if (Containing())
		return EContainer::Contain(entity);
	else
		return ECBCavern::Contain(entity);
}

void ECavern::ReleaseShoot()
{
	EContainer::ReleaseShoot();
	/*if (!Containing())
		BroadcastContaining(NULL);*/
}

/********************************************************************************************
 *                                        ECObelisk                                         *
 ********************************************************************************************/

bool ECObelisk::WantAttaq(uint mx, uint my, bool force)
{
	if(!force && (EventType() & ARM_ATTAQ))
		return false;

	/* On n'attaque pas sur notre case */
	if(DestCase()->X() == mx && DestCase()->Y() == my)
		return false;

	uint d = 0;
	for(uint x=DestCase()->X(); x != mx; d++) x < mx ? ++x : --x;
	for(uint y=DestCase()->Y(); y != my; d++) y < my ? ++y : --y;

	if(d > Porty())
		return false;

	return true;
}

bool ECObelisk::Attaq(std::vector<ECEntity*> entities, ECEvent* event)
{
	/* C'est une attaque contre moi (probablement sur la meme case). */
	if(!(EventType() & ARM_ATTAQ) || event->Case() == Case())
		return ECEntity::Attaq(entities, event);

	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(*it != this && !Like(*it) && CanAttaq(*it) && (*it)->Case() != Case() && (*it)->Nb())
		{
			uint dx = 0, dy = 0;
			for(uint x=Case()->X(); x != (*it)->Case()->X(); dx++) x < (*it)->Case()->X() ? ++x : --x;
			for(uint y=Case()->Y(); y != (*it)->Case()->Y(); dy++) y < (*it)->Case()->Y() ? ++y : --y;
			uint d = dx + dy;
			float coef = 0;
			switch(d)
			{
				case 0: coef = 0.0f; break;
				case 1: coef = 2.0f; break;
				case 2: coef = 1.8f; break;
				case 3: coef = 1.4f; break;
				case 4: coef = 0.9f; break;
				case 5: coef = 0.9f; break;
				default: coef = 0;
			}
			coef = float(Nb())/float(InitNb());
			uint killed = 0;
			if((*it)->IsBuilding())                          killed = uint(600 * coef);
			else if((*it)->IsInfantry())                     killed = uint(500 * coef);
			else if((*it)->IsVehicule() || (*it)->IsNaval()) killed = uint(700 * coef);
			else if((*it)->IsPlane())                        killed = uint(1000* coef);
			else
			{
				FDebug(W_WARNING, "Shoot d'un type non supporté");
				continue;
			}
			if(!killed) continue;

			if(Owner())
				Channel()->send_info(Owner(), EChannel::I_SHOOT, ECArgs(LongName(), (*it)->LongName(), TypToStr(killed)));
			if((*it)->Owner())
				Channel()->send_info((*it)->Owner(), EChannel::I_SHOOT, ECArgs(LongName(), (*it)->LongName(), TypToStr(killed)));
			Shoot(*it, killed);
		}

	return false;
}

/********************************************************************************************
 *                                        ECDefenseTower                                    *
 ********************************************************************************************/

bool ECDefenseTower::WantAttaq(uint mx, uint my, bool force)
{
	if(!force && (EventType() & ARM_ATTAQ))
		return false;

	/* On n'attaque pas sur notre case */
	if(DestCase()->X() == mx && DestCase()->Y() == my)
		return false;

	uint d = 0;
	for(uint x=DestCase()->X(); x != mx; d++) x < mx ? ++x : --x;
	for(uint y=DestCase()->Y(); y != my; d++) y < my ? ++y : --y;

	if(d > Porty())
		return false;

	return true;
}

bool ECDefenseTower::Attaq(std::vector<ECEntity*> entities, ECEvent* event)
{
	/* C'est une attaque contre moi (probablement sur la meme case). */
	if(!(EventType() & ARM_ATTAQ) || event->Case() == Case())
		return false;

	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		if(*it != this && !Like(*it) && CanAttaq(*it) && (*it)->Case() != Case() && (*it)->Nb() && (*it)->Level() <= L_GROUND)
		{
			uint dx = 0, dy = 0;
			for(uint x=Case()->X(); x != (*it)->Case()->X(); dx++) x < (*it)->Case()->X() ? ++x : --x;
			for(uint y=Case()->Y(); y != (*it)->Case()->Y(); dy++) y < (*it)->Case()->Y() ? ++y : --y;
			uint d = dx + dy;
			float coef = 0;
			switch(d)
			{
				case 0: coef = 0.0f; break;
				case 1: coef = 1.5f; break;
				case 2: coef = 2.0f; break;
				case 3: coef = 1.4f; break;
				case 4: coef = 0.9f; break;
				default: coef = 0;
			}
			coef = float(Nb())/float(InitNb());
			uint killed = 0;
			if((*it)->IsInfantry())                          killed = uint(300 * coef);
			else if((*it)->IsVehicule() || (*it)->IsNaval()) killed = uint(400 * coef);
			else if((*it)->IsPlane())                        killed = uint(600 * coef);
			else
			{
				FDebug(W_WARNING, "Shoot d'un type non supporté");
				continue;
			}
			if(!killed) continue;

			if(Owner())
				Channel()->send_info(Owner(), EChannel::I_SHOOT, ECArgs(LongName(), (*it)->LongName(), TypToStr(killed)));
			if((*it)->Owner())
				Channel()->send_info((*it)->Owner(), EChannel::I_SHOOT, ECArgs(LongName(), (*it)->LongName(), TypToStr(killed)));
			Shoot(*it, killed);
		}


	return false;
}

/********************************************************************************************
 *                                        ECMine                                            *
 ********************************************************************************************/

void ECMine::Resynch(ECPlayer* pl)
{
	Channel()->SendArm(pl->Client(), this, ARM_DATA, 0,0, ECData(DATA_RESTBUILD, TypToStr(restBuild)));
}

void ECMine::Played()
{
	if(restBuild)
	{
		restBuild--;
		if(Owner() && Owner()->Client())
		{
			std::vector<TClient*> sdrs = Owner()->ClientAllies();
			sdrs.push_back(Owner()->Client());
			Channel()->SendArm(sdrs, this, ARM_DATA, 0,0, ECData(DATA_RESTBUILD, TypToStr(restBuild)));
		}
	}
}

bool ECMine::Attaq(std::vector<ECEntity*> entities, ECEvent* event)
{
	if(event->Case() != Case())
		return false;

	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		if(*it == this || (*it)->Level() > L_GROUND) continue;
		int a = rand()%2, i = rand()%100;
		Shoot(*it, a ? (1000 + i) : (1000 - i));
	}
	/* Auto destruction */
	Shoot(this, Nb());

	return false;
}

/********************************************************************************************
 *                               ECGulag                                                    *
 ********************************************************************************************/
void ECGulag::Resynch(ECPlayer* pl)
{
	Channel()->SendArm(pl->Client(), this, ARM_DATA, 0, 0, ECData(DATA_NB_PRISONERS, TypToStr(nb_prisoners)));
}

void ECGulag::Played()
{
	Channel()->SendArm(NULL, this, ARM_DATA, 0, 0, ECData(DATA_NB_PRISONERS, TypToStr(nb_prisoners)));
	if (Owner() && TurnMoney(Owner()) > 0)
		Channel()->send_info(Owner(), EChannel::I_GULAG_GAIN, ECArgs(LongName(), TypToStr(TurnMoney(Owner()))));
}

/********************************************************************************************
 *                               ECNuclearSearch                                            *
 ********************************************************************************************/

void ECNuclearSearch::Resynch(ECPlayer* pl)
{
	Channel()->SendArm(pl->Client(), this, ARM_DATA, 0,0, ECData(DATA_NBMISSILES, TypToStr(missiles)));
	if(Owner() && (Owner() == pl || Owner()->IsAllie(pl)))
		Channel()->SendArm(pl->Client(), this, ARM_DATA, 0,0, ECData(DATA_RESTBUILD, TypToStr(restBuild)));
}

void ECNuclearSearch::Played()
{
	restBuild--;
	if(!restBuild)
	{
		restBuild = NUCLEARSEARCH_BUILDTIME;
		missiles++;
		Channel()->SendArm(0, this, ARM_DATA, 0,0, ECData(DATA_NBMISSILES, TypToStr(missiles)));
	}
	if(Owner() && Owner()->Client())
		Channel()->SendArm(Owner()->Client(), this, ARM_DATA, 0,0, ECData(DATA_RESTBUILD, TypToStr(restBuild)));
}

void ECNuclearSearch::RemoveOneMissile()
{
	Missiles()--;
	if(Owner() && Owner()->Client())
		Channel()->SendArm(Owner()->Client(), this, ARM_DATA, 0,0, ECData(DATA_NBMISSILES, TypToStr(Missiles())));
}

/********************************************************************************************
 *                                        ECSilo                                            *
 ********************************************************************************************/

void ECSilo::Resynch(ECPlayer* pl)
{
	if(Owner() && (Owner() == pl || Owner()->IsAllie(pl)))
		Channel()->SendArm(pl->Client(), this, ARM_DATA, 0,0, ECData(DATA_RESTBUILD, TypToStr(restBuild)));
}

void ECSilo::Played()
{
	if(restBuild)
	{
		restBuild--;
		if(Owner() && Owner()->Client())
		{
			std::vector<TClient*> sdrs = Owner()->ClientAllies();
			sdrs.push_back(Owner()->Client());
			Channel()->SendArm(sdrs, this, ARM_DATA, 0,0, ECData(DATA_RESTBUILD, TypToStr(restBuild)));
		}
	}
}

bool ECSilo::WantAttaq(uint mx, uint my, bool force)
{
	if(!force && (!NuclearSearch() || !NuclearSearch()->Missiles()))
		return false;

	/* On n'attaque pas sur notre case */
	if(DestCase()->X() == mx && DestCase()->Y() == my)
		return false;

	uint d = 0;
	for(uint x=DestCase()->X(); x != mx; d++) x < mx ? ++x : --x;
	for(uint y=DestCase()->Y(); y != my; d++) y < my ? ++y : --y;

	/* On ne tire que dans un rayon de SILO_PORTY (10?) cases. */
	if(d > Porty())
		return false;

	if(!force)
		dynamic_cast<ECNuclearSearch*>(NuclearSearch())->RemoveOneMissile();

	return true;
}

std::vector<ECBEntity*> ECSilo::GetAttaquedEntities(ECBCase* c) const
{
	std::vector<ECBEntity*> entities;
	ECBCase* cc = c->MoveLeft(SILO_IMPACT)->MoveUp(SILO_IMPACT);
	for(uint i=0; i <= 2*SILO_IMPACT; ++i)
	{
		uint j=0;
		for(; j <= 2*SILO_IMPACT; ++j)
		{
			std::vector<ECBEntity*> ents = cc->Entities()->List();
			entities.insert(entities.end(), ents.begin(), ents.end());

			if(cc->X() == cc->Map()->Width()-1)
				break;
			cc = cc->MoveRight();
		}
		if(cc->Y() == cc->Map()->Height()-1)
			break;
		cc = cc->MoveDown();
		cc = cc->MoveLeft(j);
	}
	return entities;
}

bool ECSilo::Attaq(std::vector<ECEntity*> entities, ECEvent* event)
{
	/* C'est une attaque contre moi (probablement sur la meme case). */
	if(!(EventType() & ARM_ATTAQ) || event->Case() == Case())
		return ECEntity::Attaq(entities, event);

	ECBCase* c = event->Case();
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		if(*it == this) continue;

		if((*it)->Case() == c)
		{
			if((*it)->IsCountryMaker())
				Shoot(*it, 1002);
			else if((*it)->Type() == E_NUCLEARSEARCH)
				Shoot(*it, 30);
			else
				Shoot(*it, (*it)->RealNb());
		}
		else
		{
			if((*it)->IsCountryMaker())
				continue;
			if((*it)->IsBuilding())
				Shoot(*it, (*it)->InitNb()/2+21);
			else
				Shoot(*it, (*it)->RealNb());
		}
	}

	return false;
}
