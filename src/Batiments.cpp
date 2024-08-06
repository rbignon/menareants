/* src/Batiments.cpp - Code for buildings.
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
#include "Sound.h"
#include "gui/ColorEdit.h"
#include "gui/ShowMap.h"
#include "tools/Video.h" // Pour obtenir SCREEN_WIDTH
#include "tools/Font.h"

/********************************************************************************************
 *                                ECBatiment                                                *
 ********************************************************************************************/

ECBatiment::ECBatiment(ECSpriteBase* explos)
	: explosion(explos)
{
}

bool ECBatiment::AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*)
{
	if(EventType() & ARM_ATTAQ)
	{
		if(Case()->Visible() && explosion)
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

/********************************************************************************************
 *                                ECEiffelTower                                             *
 ********************************************************************************************/
void ECEiffelTower::Init()
{
	ECEntity::Init();
	if(Owner() && Owner()->IsMe())
	{
		Map()->SetBrouillard(false);
		Map()->ShowMap()->SetBrouillard(false);
	}
}

ECEiffelTower::~ECEiffelTower()
{
	if(Owner() && Owner()->IsMe())
	{
		Map()->SetBrouillard(true);
		Map()->ShowMap()->SetBrouillard(true);
		std::vector<ECBCase*> cases = Map()->Cases();
		FORit(ECBCase*, cases, c)
		{
			ECase* acase = dynamic_cast<ECase*>(*c);
			if(acase->Showed() == 0)
				acase->SetShowed(-1);
		}
	}
}


/********************************************************************************************
 *                                ECRadar                                                   *
 ********************************************************************************************/
void ECRadar::Init()
{
	ECEntity::Init();
	if(Owner() && Owner()->IsMe())
	{
		std::vector<ECBCase*> cases = Map()->Cases();
		FORit(ECBCase*, cases, c)
		{
			ECase* acase = dynamic_cast<ECase*>(*c);
			if(acase->Showed() < 0)
				acase->SetShowed(0);
		}
	}
}

ECRadar::~ECRadar()
{
	if(Owner() && Owner()->IsMe())
	{
		std::vector<ECBCase*> cases = Map()->Cases();
		FORit(ECBCase*, cases, c)
		{
			ECase* acase = dynamic_cast<ECase*>(*c);
			if(acase->Showed() == 0)
				acase->SetShowed(-1);
		}
	}
}

/********************************************************************************************
 *                                         ECBarbedWire                                     *
 ********************************************************************************************/
void ECBarbedWire::Created()
{
	FindMyImage();
}

void ECBarbedWire::Played()
{
	FindMyImage();
}

void ECBarbedWire::FindMyImage(bool others)
{
	std::vector<ECBEntity*> entities;
	int r = Case()->SearchAroundType(Type(), entities);

	if(!r)
		SetImage(Resources::BarbedWire_Horiz());

	if((r & C_UP))
	{
		if((r & C_DOWN))
		{
			if((r & C_LEFT) && (r & C_RIGHT))
				SetImage(Resources::BarbedWire_Aiguillage());
			else if((r & C_LEFT))
				SetImage(Resources::BarbedWire_TOuest());
			else if((r & C_RIGHT))
				SetImage(Resources::BarbedWire_TEst());
			else
				SetImage(Resources::BarbedWire_Verti());
		}
		else
			if((r & C_LEFT) && (r & C_RIGHT))
				SetImage(Resources::BarbedWire_TNord());
			else if((r & C_LEFT))
				SetImage(Resources::BarbedWire_SudEst());
			else if((r & C_RIGHT))
				SetImage(Resources::BarbedWire_SudOuest());
			else
				SetImage(Resources::BarbedWire_Down());
	}
	else if(r & C_DOWN)
	{
		if((r & C_LEFT) && (r & C_RIGHT))
			SetImage(Resources::BarbedWire_TSud());
		else if(r & C_LEFT)
			SetImage(Resources::BarbedWire_NordEst());
		else if(r & C_RIGHT)
			SetImage(Resources::BarbedWire_NordOuest());
		else
			SetImage(Resources::BarbedWire_Up());
	}
	else if(r & C_LEFT && r & C_RIGHT)
		SetImage(Resources::BarbedWire_Horiz());
	else if(r & C_LEFT)
		SetImage(Resources::BarbedWire_Right());
	else if(r & C_RIGHT)
		SetImage(Resources::BarbedWire_Left());
	else
		SetImage(Resources::BarbedWire_Horiz());

	/*if((r & C_UP || r & C_DOWN) && !(r & C_LEFT) && !(r & C_RIGHT))
		SetImage(Resources::BarbedWire_Verti());
	else if((r & C_RIGHT || r & C_LEFT) && !(r & C_DOWN) && !(r & C_UP))
		SetImage(Resources::BarbedWire_Horiz());
	else if(
	else
		SetImage(Resources::BarbedWire_Aiguillage());*/

	if(others)
		FORit(ECBEntity*, entities, it)
			dynamic_cast<ECBarbedWire*>(*it)->FindMyImage(false);
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
	ECEntity::Init();

	activeimg = new ECSpriteBase(Resources::Mine_ActFace()->path.c_str());

	if(Owner() && Owner()->Color())
		activeimg->Gray2Color(color_eq[Owner()->Color()]);
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
 *                                ECavern                                                   *
 ********************************************************************************************/
ECavern::~ECavern()
{
	/* Prevent uncontain unit from other caverns */
	SetContaining(NULL);
}

std::string ECavern::SpecialInfo()
{
	if(Containing()) return _(" - Contain:");
	else if(Owner() && Owner()->IsMe()) return StringF(_(" - Capacity: %d"),100 * Nb());
	else return "";
}

/********************************************************************************************
 *                                ECGulag                                                   *
 ********************************************************************************************/
void ECGulag::Created()
{
	FindMyImage();
}

void ECGulag::Played()
{
	FindMyImage();
}

void ECGulag::FindMyImage()
{
	if (NbPrisoners() == 0)
		SetImage(GetSprite(I_Down));
	else if (NbPrisoners() < MaxPrisoners())
		SetImage(GetSprite(I_Right));
	else
		SetImage(GetSprite(I_Up));
}

void ECGulag::RecvData(ECData data)
{
	switch(data.type)
	{
		case DATA_NB_PRISONERS:
		{
			nb_prisoners = static_cast<ECEntity::e_type>(StrToTyp<uint>(data.data));
			break;
		}
	}
}

std::string ECGulag::SpecialInfo()
{
	if (NbPrisoners() == 0)
		return StringF(_("This camp is empty."));
	else if (NbPrisoners() < MaxPrisoners())
		return StringF(_("There are %d prisoners (%d remaining places)"), NbPrisoners(), MaxPrisoners()-NbPrisoners());
	else
		return StringF(_("This camp is full (%d prisoners)"), NbPrisoners());
}

/********************************************************************************************
 *                                ECNuclearSearch                                           *
 ********************************************************************************************/

void ECNuclearSearch::Init()
{
	ECBNuclearSearch::Init();
	ECEntity::Init();
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
	switch(EventType())
	{
		case ARM_ATTAQ:
			return missile.AttaqFirst(c, me);
		default:
			return true;
	}
}

bool ECSilo::MakeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(EventType())
	{
		case ARM_ATTAQ:
			return missile.AttaqSecond(c, me);
		default:
			return true;
	}
}

bool ECSilo::AfterEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(EventType())
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
 *                                 ECountryMaker                                            *
 ********************************************************************************************/

ECountryMaker::ECountryMaker()
{
	SetLabel();
}

void ECountryMaker::ChangeCase(ECBCase* c)
{
	ECBatiment::ChangeCase(c);
	SetLabel();
}

void ECountryMaker::SetLabel()
{
	if (!Case())
		return;
	const std::string& name = Case()->Country()->Name();
	if (!name.empty())
	{
		ECImage text = Font::GetInstance(Font::Normal)->CreateSurface(name, white_color);
		label.NewSurface(text.GetSize(), SDL_HWSURFACE, false);
		label.Fill(brown_color);
		label.LineColor(0, label.GetWidth()-1, 0, 0, white_color);
		label.LineColor(0, 0, 0, label.GetHeight()-1, white_color);
		label.LineColor(0, label.GetWidth()-1, label.GetHeight()-1, label.GetHeight()-1, black_color);
		label.LineColor(label.GetWidth()-1, label.GetWidth()-1, 0, label.GetHeight()-1, black_color);
		label.Blit(text);
	}
}

void ECountryMaker::AfterDraw()
{
	if (!label.IsNull() && (!Map()->ShowMap()->HaveBrouillard() || Case()->Visible()))
		label.Draw(Case()->Image()->X()+CASE_WIDTH/2-label.GetWidth()/2,
		           Case()->Image()->Y());
	ECEntity::AfterDraw();
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

		if(cible->Visible() && t - t0 < duration/2 &&
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
	switch(EventType())
	{
		case ARM_ATTAQ:
			if(c != Case() && (c->Visible() || Case()->Visible()))
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
	switch(EventType())
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
	switch(EventType())
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
		img.Draw(victim->X() < Case()->X() ? victim->Image()->X()
		                                   : Image()->X(),
		         victim->Y() < Case()->Y() ? victim->Image()->Y()
		                                   : Image()->Y());
}

bool ECObelisk::BeforeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me)
{
	switch(EventType())
	{
		case ARM_ATTAQ:
			if(!Case()->Visible() || c == Case()) return true;

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
	switch(EventType())
	{
		case ARM_ATTAQ:
		{
			if(c == Case()) return true;

			int dx = CASE_WIDTH * (abs((int)c->X() - (int)Case()->X()) + 1);
			int dy = CASE_HEIGHT * (abs((int)c->Y() - (int)Case()->Y()) + 1);
			img.SetImage(SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, dx, dy,
											32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
			DrawLargeLine(img.Img, c->X() < Case()->X() ? (dx - CASE_WIDTH + 53) : 53,
			                       c->Y() < Case()->Y() ? (dy - CASE_HEIGHT + 2) : 2,
			                       c->X() < Case()->X() ? CASE_WIDTH/2 : (dx - CASE_WIDTH/2),
			                       c->Y() < Case()->Y() ? CASE_HEIGHT-30 : (dy - 30),
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
	switch(EventType())
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
