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

#include "Batiments.h"
#include "Channels.h"
#include "Sound.h"
#include "gui/ColorEdit.h"
#include "gui/ShowMap.h"
#include "tools/Video.h" // Pour obtenir SCREEN_WIDTH

/********************************************************************************************
 *                                ECBatiment                                                *
 ********************************************************************************************/

ECBatiment::ECBatiment(ECSpriteBase* b, ECSpriteBase* explos)
	: explosion(explos)
{
	img = new ECSpriteBase(b->path.c_str());

	if(Owner() && Owner()->Color())
		img->ChangeColor(white_color, color_eq[Owner()->Color()]);

	SetImage(img);
}

void ECBatiment::RefreshColor(Color last)
{
	img->ChangeColor(last, Owner() ? color_eq[Owner()->Color()] : white_color);
}

bool ECBatiment::AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*)
{
	if(event_type & ARM_ATTAQ)
	{
		if(Case()->Showed() > 0 && explosion)
		{
			if(!AttaqImg())
			{
				SetAttaqImg(explosion, Image()->X(), Image()->Y());
				AttaqImg()->SetRepeat(false);
				AttaqImg()->SetAnim(true);
				return false;
			}
			if(AttaqImg()->Anim())
			{
				SDL_Delay(20);
				return false;
			}

			SetAttaqImg(0,0,0);
		}
	}
	return true;
}

ECBatiment::~ECBatiment()
{
	if(img)
		delete img;
}

/********************************************************************************************
 *                                         ECRail                                           *
 ********************************************************************************************/
void ECRail::Created()
{
	FindMyImage();
}

void ECRail::Played()
{
	ECEntity::Played();

	FindMyImage();
}

void ECRail::FindMyImage(bool others)
{
	std::vector<ECBEntity*> entities;
	int r = Case()->SearchAroundType(Type(), entities);

	std::vector<ECBEntity*> null; /* On ne s'en sert pas, c'est uniquement l'argument */
	r |= Case()->SearchAroundType(E_CHARFACT, null);

	if(!r)
		SetImage(Resources::Rail_Horiz());

	if((r & C_UP || r & C_DOWN) && !(r & C_LEFT) && !(r & C_RIGHT))
		SetImage(Resources::Rail_Verti());
	else if((r & C_RIGHT || r & C_LEFT) && !(r & C_DOWN) && !(r & C_UP))
		SetImage(Resources::Rail_Horiz());
	else
		SetImage(Resources::Rail_Aiguillage());

	if(others)
		FORit(ECBEntity*, entities, it)
			dynamic_cast<ECRail*>(*it)->FindMyImage(false);
}

/********************************************************************************************
 *                                         ECMine                                           *
 ********************************************************************************************/

ECMine::~ECMine()
{
	delete activeimg;
}

void ECMine::Init()
{
	ECBEntity::Init();

	activeimg = new ECSpriteBase(Resources::Mine_ActFace()->path.c_str());

	if(Owner() && Owner()->Color())
		activeimg->ChangeColor(white_color, color_eq[Owner()->Color()]);
}

void ECMine::RecvData(ECData data)
{
	switch(data.type)
	{
		case DATA_RESTBUILD:
			restBuild = StrToTyp<uint>(data.data);
			if(!restBuild)
			{
				SetImage(activeimg);
				Image()->SetAnim(true);
			}
			break;
	}
}

std::string ECMine::SpecialInfo()
{
	if(!Owner() || !Owner()->IsMe()) return "";
	else if(restBuild) return StringF(_("Active in %d days."), restBuild);
	else return _("Mine actived");
}

/********************************************************************************************
 *                                ECNuclearSearch                                           *
 ********************************************************************************************/

void ECNuclearSearch::Init()
{
	ECBNuclearSearch::Init();
	Image()->SetAnim(true);
	if(Owner() && Channel() && !Owner()->IsMe())
		Channel()->Print(StringF(_("WARNING!! %s has got nuclear technologie, with his nuclear search"), Owner()->GetNick()), 0x008);
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
				if(Owner()->IsMe())
					Channel()->Print(_("You have got a new missile in your stock!"), 0x008);
				else
					Channel()->Print(StringF(_("%s has a new missile in stock!"), Owner()->GetNick()), 0x008);
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
	return (Owner() && Owner()->IsMe())
	           ? StringF(_("%d missile(s) - New missile in %d day(s)"), missiles, restBuild)
	           : "";
}

/********************************************************************************************
 *                                         ECSilo                                           *
 ********************************************************************************************/

void ECSilo::RecvData(ECData data)
{
	switch(data.type)
	{
		case DATA_RESTBUILD:
			restBuild = StrToTyp<uint>(data.data);
			break;
	}
}

std::string ECSilo::SpecialInfo()
{
	if(!Owner() || !Owner()->IsMe())
		return "";
	else if(!NuclearSearch())
		return _("Not linked with a NuclearSearch Center.");
	else if(restBuild)
		return StringF(_("This silo is ready in %d day(s)."), restBuild);
	else if(!NuclearSearch()->Missiles())
		return _("You haven't got any missile.");
	else
		return StringF(ngettext("There is %d missile", "There are %d missiles", NuclearSearch()->Missiles()), NuclearSearch()->Missiles());
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
			return true;
		}
		default:
			return true;
	}
}

/********************************************************************************************
 *                                 ECMegalopole                                             *
 ********************************************************************************************/

std::string ECMegalopole::SpecialInfo()
{
	if(!Owner() || !Owner()->IsMe())
		return "";
	else
		return StringF(_("$%d income each turn"), TurnMoney(Owner()));
}

/********************************************************************************************
 *                                 ECapitale                                                *
 ********************************************************************************************/

std::string ECapitale::SpecialInfo()
{
	if(!Owner() || !Owner()->IsMe())
		return "";
	else
		return StringF(_("$%d income each turn"), TurnMoney(Owner()));
}

/********************************************************************************************
 *                                 ECity                                                    *
 ********************************************************************************************/

std::string ECity::SpecialInfo()
{
	if(!Owner() || !Owner()->IsMe())
		return "";
	else
		return StringF(_("$%d income each turn"), TurnMoney(Owner()));
}

/********************************************************************************************
 *                                 ECDefenseTower                                           *
 ********************************************************************************************/
void ECDefenseTower::AfterDraw()
{
	ECEntity::AfterDraw();

	if(!cible) return;

	// Coordonnées du point de départ
	float ax = Case()->Image()->X() + 36;
	float ay = Case()->Image()->Y() + 2;
	// Coordonnées du point d'arrivé
	float bx = (int)(cible->Image()->X() + CASE_WIDTH/2 - Resources::DefenseTower_Missile()->GetWidth()/2);
	float by = (int)(cible->Image()->Y() + CASE_HEIGHT/2 - Resources::DefenseTower_Missile()->GetHeight()/2);
	// Point de passage le plus haut du missile (là où il commence à retomber)
	float ty;
	float tx = ((ax + bx) / 2);

	ty = 0;

	if(!miss)
		miss = SDL_GetTicks();

	// Position du missile :
	const unsigned int duration = 2000; // Durée du mouvement du missile
	unsigned int t = SDL_GetTicks(); // Temps actuel en millisecondes
	unsigned int t0 = miss; // Temps au moement du lancement du missile

	if( ax == tx ) tx++; // Evite une division par 0
	if( tx == bx ) bx++; // Evite une division par 0

	float x = ((bx - ax) * (t - t0) / (float)duration) + ax;

	// Calcule des coeffs de l'ï¿œuation
	float A, B, C;

	if( t - t0 < duration / 2)
	{
		A = (ay - ty) / ((ax - tx)*(ax - tx));
		B = - 2 * A * tx;
		C = ay - A*ax*ax - B*ax;
	}
	else
	{
		A = (by - ty) / ((bx - tx)*(bx - tx));
		B = - 2 * A * tx;
		C = by - A*bx*bx - B*bx;
	}

	// Position du missile :
	if( t - t0 <= duration)
	{
		//float x = ((bx - ax) * (t - t0) / (float)duration) + (float)ax;
		float y = A*x*x + B*x + C;

		Resources::DefenseTower_Missile()->Draw((int)x, (int)y);

		if(cible->Showed() > 0 && t - t0 < duration/2 &&
		   (cible->Image()->X() < 0 || cible->Image()->X()+CASE_WIDTH > (int)SCREEN_WIDTH))
			Map()->ShowMap()->CenterTo(cible);

		// On fait assez large
		Map()->ShowMap()->ToRedraw(Rectanglei((int)x-CASE_WIDTH, (int)y-CASE_HEIGHT, 3*CASE_WIDTH, 3*CASE_HEIGHT));
	}
	else
	{
		// Explosion
		miss = 0;
		cible = 0;
	}
}

bool ECDefenseTower::BeforeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
			if(c != Case() && (c->Showed() > 0 || Case()->Showed() > 0))
			{
				Map()->ShowMap()->CenterTo(this);
				cible = c;
			}
		default:
			return true;
	}
}

bool ECDefenseTower::MakeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
			if(cible)
			{
				SDL_Delay(20);
				return false;
			}
			return true;
		default:
			return true;
	}
}

bool ECDefenseTower::AfterEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
			return true;
		default:
			return true;
	}
}

/********************************************************************************************
 *                                 ECObelisk                                                *
 ********************************************************************************************/
void ECObelisk::AfterDraw()
{
	ECEntity::AfterDraw();
	if(victim)
		img.Draw(victim->X() < Case()->X() ? victim->Image()->X()+53
				                           : Image()->X()+53,
				 victim->Y() < Case()->Y() ? victim->Image()->Y()+2
				                           : Image()->Y()+2);
}

bool ECObelisk::BeforeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
			if(Case()->Showed() <= 0 || c == Case()) return true;

			Image()->SetRepeat(false);
			Image()->SetAnim(true);
			while(Image()->Anim()) SDL_Delay(20);

			return true;

		default:
			return true;
	}
}

bool ECObelisk::MakeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
		{
			if(c == Case()) return true;

			int dx = abs(c->Image()->X() - Image()->X() + 3);
			int dy = abs(c->Image()->Y() - Image()->Y() + 3);
			img.SetImage(SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, dx, dy,
											32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
			DrawLargeLine(img.Img,       c->X() < Case()->X() ? 0 : dx-1,
			                             c->Y() < Case()->Y() ? 0 : dy-1,
			                             c->X() < Case()->X() ? dx-1 : 0,
			                             c->Y() < Case()->Y() ? dy-1 : 0,
			                             img.MapColor(red_color));
			Resources::SoundObelisque()->Play();
			victim = c;
			SDL_Delay(1500);
			victim = 0;
			Map()->ShowMap()->ToRedraw(Rectanglei(c->X() < Case()->X() ? c->Image()->X()+CASE_WIDTH /2
				                                            : Image()->X()+CASE_WIDTH /2,
				                       c->Y() < Case()->Y() ? c->Image()->Y()+CASE_HEIGHT/2
				                                            : Image()->Y()+CASE_HEIGHT/2,
				                       img.GetWidth(), img.GetHeight()));
			img.SetImage(0);
			return true;
		}
		default:
			return true;
	}
}

bool ECObelisk::AfterEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(event_type)
	{
		case ARM_ATTAQ:
			if(c == Case()) return true;

			Image()->SetAnim(false);
			Image()->SetFrame(0);
			return true;
		default:
			return true;
	}
}
