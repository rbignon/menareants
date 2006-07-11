/* src/Resources.cpp - Struct which define all resources.
 *
 * Copyright (C) 2005 Romain Bignon  <Progs@headfucking.net>
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
 * $Id$
 */

#include <SDL.h>
#include <SDL_image.h>

#include "Resources.h"

#define R_CLEARTYPE(type) for(std::vector<type*>::iterator it = type##_objects.begin(); it != type##_objects.end(); ++it) \
                          	delete *it; \
                          type##_objects.clear()
#define R_RESOURCE(type, name) type* Resources::spr_##name = NULL
	R_RESOURCE(ECSpriteBase, Intro);
	R_RESOURCE(ECImage,      Titlescreen);
	R_RESOURCE(ECImage,      Loadscreen);
	R_RESOURCE(ECImage,      BarreLat);
	R_RESOURCE(ECImage,      BarreAct);
	R_RESOURCE(ECImage,      Cadre);
	R_RESOURCE(ECImage,      Case);
	R_RESOURCE(ECImage,      GoodHashure);
	R_RESOURCE(ECImage,      BadHashure);
	R_RESOURCE(ECImage,      HelpScreen);
	R_RESOURCE(ECImage,      FlecheVert);
	R_RESOURCE(ECImage,      FlecheHoriz);
	R_RESOURCE(ECImage,      FlecheDroiteHaut);
	R_RESOURCE(ECImage,      FlecheDroiteBas);
	R_RESOURCE(ECImage,      FlecheGaucheHaut);
	R_RESOURCE(ECImage,      FlecheGaucheBas);
	R_RESOURCE(ECImage,      FlecheVersDroite);
	R_RESOURCE(ECImage,      FlecheVersGauche);
	R_RESOURCE(ECImage,      FlecheVersHaut);
	R_RESOURCE(ECImage,      FlecheVersBas);
	R_RESOURCE(ECImage,      FlecheAttaqDroite);
	R_RESOURCE(ECImage,      FlecheAttaqGauche);
	R_RESOURCE(ECImage,      FlecheAttaqHaut);
	R_RESOURCE(ECImage,      FlecheAttaqBas);
	R_RESOURCE(ECSpriteBase, UpButton);
	R_RESOURCE(ECSpriteBase, DownButton);
	R_RESOURCE(ECSpriteBase, NormalButton);
	R_RESOURCE(ECSpriteBase, LitleButton);
	R_RESOURCE(ECSpriteBase, PlayButton);
	R_RESOURCE(ECSpriteBase, OptionsButton);
	R_RESOURCE(ECSpriteBase, CreditsButton);
	R_RESOURCE(ECSpriteBase, QuitterButton);
	R_RESOURCE(ECSpriteBase, CheckBox);
	R_RESOURCE(ECSpriteBase, CaseMer);
	R_RESOURCE(ECSpriteBase, CaseTerre);
	R_RESOURCE(ECSpriteBase, CaseVille);
	R_RESOURCE(ECSpriteBase, CaseCapitale);
	R_RESOURCE(ECSpriteBase, CaseBordNord);
	R_RESOURCE(ECSpriteBase, CaseBordSud);
	R_RESOURCE(ECSpriteBase, CaseBordEst);
	R_RESOURCE(ECSpriteBase, CaseBordOuest);
	R_RESOURCE(ECSpriteBase, CaseBordNordOuest);
	R_RESOURCE(ECSpriteBase, CaseBordNordEst);
	R_RESOURCE(ECSpriteBase, CaseBordSudOuest);
	R_RESOURCE(ECSpriteBase, CaseBordSudEst);
	R_RESOURCE(ECSpriteBase, CaseCoinSudEst);
	R_RESOURCE(ECSpriteBase, CaseCoinSudOuest);
	R_RESOURCE(ECSpriteBase, CaseCoinNordEst);
	R_RESOURCE(ECSpriteBase, CaseCoinNordOuest);
	R_RESOURCE(ECSpriteBase, CasePontHorizontal);
	R_RESOURCE(ECSpriteBase, CasePontVertical);
	R_RESOURCE(ECSpriteBase, CasePontHaut);
	R_RESOURCE(ECSpriteBase, CasePontBas);
	R_RESOURCE(ECSpriteBase, CasePontGauche);
	R_RESOURCE(ECSpriteBase, CasePontDroite);
	R_RESOURCE(ECImage,      Army_Icon);
	R_RESOURCE(ECSpriteBase, Army_Face);
	R_RESOURCE(ECSpriteBase, Army_Left);
	R_RESOURCE(ECSpriteBase, Army_Right);
	R_RESOURCE(ECSpriteBase, Army_Dos);
	R_RESOURCE(ECImage,      Caserne_Icon);
	R_RESOURCE(ECSpriteBase, Caserne_Face);
	R_RESOURCE(ECImage,      Char_Icon);
	R_RESOURCE(ECSpriteBase, Char_Face);
	R_RESOURCE(ECSpriteBase, Char_Left);
	R_RESOURCE(ECSpriteBase, Char_Right);
	R_RESOURCE(ECSpriteBase, Char_Dos);
	R_RESOURCE(ECImage,      CharFact_Icon);
	R_RESOURCE(ECSpriteBase, CharFact_Face);
	R_RESOURCE(ECImage,      MissiLauncher_Icon);
	R_RESOURCE(ECSpriteBase, MissiLauncher_Face);
	R_RESOURCE(ECSpriteBase, MissiLauncher_Left);
	R_RESOURCE(ECSpriteBase, MissiLauncher_Right);
	R_RESOURCE(ECSpriteBase, MissiLauncher_Dos);
	R_RESOURCE(ECSpriteBase, MissiLauncher_Deployed);
	R_RESOURCE(ECSpriteBase, MissiLauncher_Reployed);
	R_RESOURCE(ECSpriteBase, MissiLauncher_Missile_Up);
	R_RESOURCE(ECSpriteBase, MissiLauncher_Missile_Down);
	R_RESOURCE(ECImage,      City_Icon);
	R_RESOURCE(ECSpriteBase, City_Face);
	R_RESOURCE(ECImage,      Capitale_Icon);
	R_RESOURCE(ECSpriteBase, Capitale_Face);
	R_RESOURCE(ECImage,      Shipyard_Icon);
	R_RESOURCE(ECSpriteBase, Shipyard_Face);
	R_RESOURCE(ECImage,      Boat_Icon);
	R_RESOURCE(ECSpriteBase, Boat_Face);
	R_RESOURCE(ECSpriteBase, Boat_Left);
	R_RESOURCE(ECSpriteBase, Boat_Right);
	R_RESOURCE(ECSpriteBase, Boat_Dos);
	R_RESOURCE(ECImage,      NuclearSearch_Icon);
	R_RESOURCE(ECSpriteBase, NuclearSearch_Face);
	R_RESOURCE(ECImage,      Silo_Icon);
	R_RESOURCE(ECSpriteBase, Silo_Face);
	R_RESOURCE(ECSpriteBase, Silo_Missile_Up);
	R_RESOURCE(ECSpriteBase, Silo_Missile_Down);
#undef R_RESOURCE
#define R_TYPE(type) std::vector<type*> Resources::type##_objects
	R_TYPE(ECImage);
	R_TYPE(ECSpriteBase);
#undef R_TYPE

Resources::Resources()
{
}

Resources::~Resources()
{

}

void Resources::Unload()
{
	R_CLEARTYPE(ECImage);
	R_CLEARTYPE(ECSpriteBase);
}

#undef R_CLEARTYPE
