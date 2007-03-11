/* src/Resources.h - Header of Resources.cpp
 *
 * Copyright (C) 2005-2007 Romain Bignon  <Progs@headfucking.net>
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

#ifndef RESOURCES_H
#define RESOURCES_H

#include <vector>

class ECImage;
class Sound;
class ECSpriteBase;

/** \page Resources_h Resources file's format
 *
 * In Resources.h, there is some macros to add easily a picture or a sprite.
 *
 * For each picture or sprite, add this line with the others in Resources.h :
 * <pre>
 *   R_RESOURCE(type,  name of image, path of image);
 * </pre>
 * For example :
 * <pre>
 *   R_RESOURCE(ECImage,      Titlescreen,  PKGDATADIR_PICS "menu.png");
 *   R_RESOURCE(ECSpriteBase, CaseTerre,    "cases/terre");
 * </pre>
 * And you have to add this line in Resources.cpp :
 * <pre>
 *   R_RESOURCE(type, name of image);
 * </pre>
 *
 * For each types, you have to add a line in Resources.h AND Resources.cpp like this :
 * <pre>
 *   R_TYPE(type);
 * </pre>
 * For example :
 * <pre>
 *   R_TYPE(ECImage);
 * </pre>
 * And in Resources.cpp you have to add a second line like this :
 * <pre>
 *   R_CLEARTYPE(type);
 * </pre>
 * For example :
 * <pre>
 *   R_CLEARTYPE(ECImage);
 * </pre>
 *
 * And it's ok !
 *
 * @author Romain Bignon
 * @date 16/03/2006
 */

#define R_RESOURCE(type, name)     public: \
                                        	static type *name(); \
                                        protected: \
                                        	static type *spr_##name
#define R_TYPE(type) static std::vector<type*> type##_objects

class Resources
{
protected:
	R_TYPE(ECSpriteBase);
	R_TYPE(ECImage);
	R_TYPE(Sound);

public:
	Resources();
	~Resources();
	static void Unload();

	/* Ne pas oublier de rajouter dans Resources.cpp */
	R_RESOURCE(Sound,        DingDong);
	R_RESOURCE(Sound,        SoundResources);
	R_RESOURCE(Sound,        SoundStart);
	R_RESOURCE(Sound,        SoundBegin);
	R_RESOURCE(Sound,        SoundEnd);
	R_RESOURCE(Sound,        SoundMitraillette);
	R_RESOURCE(Sound,        SoundObelisque);
	R_RESOURCE(Sound,        SoundMcDo);

	R_RESOURCE(ECSpriteBase, Intro);
	R_RESOURCE(ECImage,      Titlescreen);
	R_RESOURCE(ECImage,      Title);
	R_RESOURCE(ECImage,      TitleMini);
	R_RESOURCE(ECImage,      Loadscreen);
	R_RESOURCE(ECImage,      BarreLat);
	R_RESOURCE(ECImage,      BarreAct);
	R_RESOURCE(ECImage,      Cadre);
	R_RESOURCE(ECImage,      Case);
	R_RESOURCE(ECImage,      FogTop);
	R_RESOURCE(ECImage,      FogBottom);
	R_RESOURCE(ECImage,      FogRight);
	R_RESOURCE(ECImage,      FogLeft);
	R_RESOURCE(ECImage,      FogTopLeft);
	R_RESOURCE(ECImage,      FogTopRight);
	R_RESOURCE(ECImage,      FogBottomLeft);
	R_RESOURCE(ECImage,      FogBottomRight);
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
	R_RESOURCE(ECImage,      PointerStandard);
	R_RESOURCE(ECImage,      PointerSelect);
	R_RESOURCE(ECImage,      PointerAttaq);
	R_RESOURCE(ECImage,      PointerMaintainedAttaq);
	R_RESOURCE(ECImage,      PointerCantAttaq);
	R_RESOURCE(ECImage,      PointerInvest);
	R_RESOURCE(ECImage,      PointerLeft);
	R_RESOURCE(ECImage,      PointerRadar);
	R_RESOURCE(ECImage,      PointerAddBP);
	R_RESOURCE(ECImage,      PointerRemBP);
	R_RESOURCE(ECImage,      MoveMapPoint);
	R_RESOURCE(ECSpriteBase, Balise);
	R_RESOURCE(ECSpriteBase, UpButton);
	R_RESOURCE(ECSpriteBase, DownButton);
	R_RESOURCE(ECSpriteBase, NormalButton);
	R_RESOURCE(ECSpriteBase, LitleButton);
	R_RESOURCE(ECSpriteBase, MissionButton);
	R_RESOURCE(ECSpriteBase, EscarmoucheButton);
	R_RESOURCE(ECSpriteBase, PlayButton);
	R_RESOURCE(ECSpriteBase, OptionsButton);
	R_RESOURCE(ECSpriteBase, MapEditorButton);
	R_RESOURCE(ECSpriteBase, CreditsButton);
	R_RESOURCE(ECSpriteBase, QuitButton);
	R_RESOURCE(ECSpriteBase, CheckBox);
	R_RESOURCE(ECSpriteBase, CaseMer);
	R_RESOURCE(ECSpriteBase, CaseTerre);
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
	R_RESOURCE(ECSpriteBase, CaseTerreDead);
	R_RESOURCE(ECSpriteBase, CaseCityNODead);
	R_RESOURCE(ECSpriteBase, CaseCityNEDead);
	R_RESOURCE(ECSpriteBase, CaseCitySODead);
	R_RESOURCE(ECSpriteBase, CaseCitySEDead);
	R_RESOURCE(ECSpriteBase, CaseMontain);
	R_RESOURCE(ECSpriteBase, Brouillard);
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
	R_RESOURCE(ECImage,      Enginer_Icon);
	R_RESOURCE(ECSpriteBase, Enginer_Face);
	R_RESOURCE(ECSpriteBase, Enginer_Left);
	R_RESOURCE(ECSpriteBase, Enginer_Right);
	R_RESOURCE(ECSpriteBase, Enginer_Dos);
	R_RESOURCE(ECImage,      DefenseTower_Icon);
	R_RESOURCE(ECSpriteBase, DefenseTower_Face);
	R_RESOURCE(ECImage,      DefenseTower_Missile);
	R_RESOURCE(ECImage,      Tourist_Icon);
	R_RESOURCE(ECSpriteBase, Tourist_Face);
	R_RESOURCE(ECSpriteBase, Tourist_Left);
	R_RESOURCE(ECSpriteBase, Tourist_Right);
	R_RESOURCE(ECSpriteBase, Tourist_Dos);
	R_RESOURCE(ECImage,      Mine_Icon);
	R_RESOURCE(ECSpriteBase, Mine_DesactFace);
	R_RESOURCE(ECSpriteBase, Mine_ActFace);
	R_RESOURCE(ECImage,      Obelisk_Icon);
	R_RESOURCE(ECSpriteBase, Obelisk_Face);
	R_RESOURCE(ECImage,      McDo_Icon);
	R_RESOURCE(ECSpriteBase, McDo_Face);
	R_RESOURCE(ECSpriteBase, McDo_Left);
	R_RESOURCE(ECSpriteBase, McDo_Right);
	R_RESOURCE(ECSpriteBase, McDo_Dos);
	R_RESOURCE(ECSpriteBase, McDo_Caserne);
	R_RESOURCE(ECSpriteBase, McDo_Jouano);
	R_RESOURCE(ECImage,      Trees_Icon);
	R_RESOURCE(ECSpriteBase, Trees_Face);
	R_RESOURCE(ECImage,      Megalopole_Icon);
	R_RESOURCE(ECSpriteBase, Megalopole_Face);
	R_RESOURCE(ECImage,      Rail_Icon);
	R_RESOURCE(ECSpriteBase, Rail_Horiz);
	R_RESOURCE(ECSpriteBase, Rail_Verti);
	R_RESOURCE(ECSpriteBase, Rail_Aiguillage);
	R_RESOURCE(ECImage,      Train_Icon);
	R_RESOURCE(ECSpriteBase, Train_Face);
	R_RESOURCE(ECSpriteBase, Train_Left);
	R_RESOURCE(ECSpriteBase, Train_Right);
	R_RESOURCE(ECSpriteBase, Train_Dos);
	R_RESOURCE(ECImage,      Jouano_Icon);
	R_RESOURCE(ECSpriteBase, Jouano_Face);
	R_RESOURCE(ECImage,      Plane_Icon);
	R_RESOURCE(ECSpriteBase, Plane_Face);
	R_RESOURCE(ECSpriteBase, Plane_Left);
	R_RESOURCE(ECSpriteBase, Plane_Right);
	R_RESOURCE(ECSpriteBase, Plane_Dos);
	R_RESOURCE(ECSpriteBase, Plane_Deployed);
	R_RESOURCE(ECSpriteBase, Plane_Reployed);
	R_RESOURCE(ECImage,      BarbedWire_Icon);
	R_RESOURCE(ECSpriteBase, BarbedWire_Horiz);
	R_RESOURCE(ECSpriteBase, BarbedWire_Verti);
	R_RESOURCE(ECSpriteBase, BarbedWire_Aiguillage);
	R_RESOURCE(ECSpriteBase, BarbedWire_TNord);
	R_RESOURCE(ECSpriteBase, BarbedWire_TSud);
	R_RESOURCE(ECSpriteBase, BarbedWire_TEst);
	R_RESOURCE(ECSpriteBase, BarbedWire_TOuest);
	R_RESOURCE(ECSpriteBase, BarbedWire_Left);
	R_RESOURCE(ECSpriteBase, BarbedWire_Right);
	R_RESOURCE(ECSpriteBase, BarbedWire_Up);
	R_RESOURCE(ECSpriteBase, BarbedWire_Down);
	R_RESOURCE(ECSpriteBase, BarbedWire_NordOuest);
	R_RESOURCE(ECSpriteBase, BarbedWire_NordEst);
	R_RESOURCE(ECSpriteBase, BarbedWire_SudOuest);
	R_RESOURCE(ECSpriteBase, BarbedWire_SudEst);
};

#undef R_RESOURCE
#undef R_TYPE
#endif
