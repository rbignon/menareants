/* src/Batiments.cpp - Code for buildings.
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

#include "Channels.h"
#include "Batiments.h"
#include "gui/ColorEdit.h"

/********************************************************************************************
 *                                ECBatiment                                                *
 ********************************************************************************************/

ECBatiment::ECBatiment(ECSpriteBase* b)
{
	img = new ECSpriteBase(b->path.c_str());

	if(Owner() && Owner()->Color())
		img->ChangeColor(white_color, *color_eq[Owner()->Color()]);

	SetImage(img);
}

void ECBatiment::RefreshColor(SDL_Color last)
{
	img->ChangeColor(last, Owner() ? *color_eq[Owner()->Color()] : white_color);
}

ECBatiment::~ECBatiment()
{
	if(img)
		delete img;
}

/********************************************************************************************
 *                                ECNuclearSearch                                           *
 ********************************************************************************************/

void ECNuclearSearch::Init()
{
	ECBNuclearSearch::Init();
	SDL_Delay(1000);
}

void ECNuclearSearch::RecvData(ECData data)
{
	switch(data.type)
	{
		case DATA_NBMISSILES:
		{
			uint new_missile_nb = StrToTyp<uint>(data.data);
			if(Owner() && missiles < new_missile_nb)
			{
				// 0x008 = I_Shit
				if(Owner() == Channel()->GetMe())
					Channel()->Print("Vous avez un nouveau missile en stock !", 0x008);
				else
					Channel()->Print(std::string(Owner()->GetNick()) + " a un nouveau missile en stock !", 0x008);
			}
			missiles = new_missile_nb;
			break;
		}
		case DATA_RESTBUILD:
			restBuild = StrToTyp<uint>(data.data);
			break;
	}
}

std::string ECNuclearSearch::SpecialInfo()
{
	return TypToStr(missiles) + " missile(s) - Nouveau missile dans " + TypToStr(restBuild) + " jour(s)";
}

/********************************************************************************************
 *                                         ECSilo                                           *
 ********************************************************************************************/

void ECSilo::Draw()
{
	ECEntity::Draw();
	missile.Draw();
}

bool ECSilo::BeforeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
			return missile.AttaqFirst(c, me);
		default:
			return true;
	}
}

bool ECSilo::MakeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
			return missile.AttaqSecond(c, me);
		default:
			return true;
	}
}

bool ECSilo::AfterEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
		{
			if(c == Case()) return true;

			if(c->Flags() & (C_TERRE))
				c->Image()->SetFrame(1);

			SDL_Delay(800);
			return true;
		}
		default:
			return true;
	}
}
