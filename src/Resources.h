/* src/Resources.h - Header of Resources.cpp
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
 * $Id$
 */

#ifndef RESOURCES_H
#define RESOURCES_H

#include <vector>
#include "Defines.h"
#include "tools/Images.h"

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

#define R_RESOURCE(type, name, url)     public: \
                                        	static type *name() { \
												if(!spr_##name) \
												{ \
													spr_##name = new type(url); \
													type##_objects.push_back(spr_##name); \
												} \
												return spr_##name; \
                                        	} \
                                        protected: \
                                        	static type *spr_##name
#define R_RESOURCE_P(type, name, url, p) public: \
                                        	static type *name() { \
												if(!spr_##name) \
												{ \
													spr_##name = new type(url, p); \
													type##_objects.push_back(spr_##name); \
												} \
												return spr_##name; \
                                        	} \
                                        protected: \
                                        	static type *spr_##name
#define R_TYPE(type) static std::vector<type*> type##_objects

class Resources
{
protected:
	R_TYPE(ECSpriteBase);
	R_TYPE(ECImage);

public:
	Resources();
	~Resources();
	static void Unload();

	/* Ne pas oublier de rajouter dans Resources.cpp */
	R_RESOURCE(ECSpriteBase, Intro,            "intro");
	R_RESOURCE(ECImage,      Titlescreen,      PKGDATADIR_ANIMS "intro/17.png");
	R_RESOURCE(ECImage,      Loadscreen,       PKGDATADIR_PICS "loading.png");
	R_RESOURCE(ECImage,      BarreLat,         PKGDATADIR_PICS "barrelat.png");
	R_RESOURCE(ECImage,      BarreAct,         PKGDATADIR_PICS "barreact.png");
	R_RESOURCE(ECImage,      Cadre,            PKGDATADIR_PICS "cadre.png");
	R_RESOURCE(ECImage,      Case,             PKGDATADIR_PICS "case.png");
	R_RESOURCE(ECImage,      FogTop,           PKGDATADIR_PICS "fog-top.png");
	R_RESOURCE(ECImage,      FogBottom,        PKGDATADIR_PICS "fog-bottom.png");
	R_RESOURCE(ECImage,      FogRight,         PKGDATADIR_PICS "fog-right.png");
	R_RESOURCE(ECImage,      FogLeft,          PKGDATADIR_PICS "fog-left.png");
	R_RESOURCE(ECImage,      FogTopLeft,       PKGDATADIR_PICS "fog-topleft.png");
	R_RESOURCE(ECImage,      FogTopRight,      PKGDATADIR_PICS "fog-topright.png");
	R_RESOURCE(ECImage,      FogBottomLeft,    PKGDATADIR_PICS "fog-bottomleft.png");
	R_RESOURCE(ECImage,      FogBottomRight,   PKGDATADIR_PICS "fog-bottomright.png");
	R_RESOURCE(ECImage,      GoodHashure,      PKGDATADIR_PICS "goodhashure.png");
	R_RESOURCE(ECImage,      BadHashure,       PKGDATADIR_PICS "badhashure.png");
	R_RESOURCE(ECImage,      HelpScreen,       PKGDATADIR_PICS "help.png");
	R_RESOURCE_P(ECImage,    FlecheVert,       PKGDATADIR_PICS "fleches/fleche_vert.png", true);
	R_RESOURCE_P(ECImage,    FlecheHoriz,      PKGDATADIR_PICS "fleches/fleche_horiz.png", true);
	R_RESOURCE_P(ECImage,    FlecheDroiteHaut, PKGDATADIR_PICS "fleches/fleche_droitehaut.png", true);
	R_RESOURCE_P(ECImage,    FlecheDroiteBas,  PKGDATADIR_PICS "fleches/fleche_droitebas.png", true);
	R_RESOURCE_P(ECImage,    FlecheGaucheHaut, PKGDATADIR_PICS "fleches/fleche_gauchehaut.png", true);
	R_RESOURCE_P(ECImage,    FlecheGaucheBas,  PKGDATADIR_PICS "fleches/fleche_gauchebas.png", true);
	R_RESOURCE_P(ECImage,    FlecheVersDroite, PKGDATADIR_PICS "fleches/fleche_vers_droite.png", true);
	R_RESOURCE_P(ECImage,    FlecheVersGauche, PKGDATADIR_PICS "fleches/fleche_vers_gauche.png", true);
	R_RESOURCE_P(ECImage,    FlecheVersHaut,   PKGDATADIR_PICS "fleches/fleche_vers_haut.png", true);
	R_RESOURCE_P(ECImage,    FlecheVersBas,    PKGDATADIR_PICS "fleches/fleche_vers_bas.png", true);
	R_RESOURCE_P(ECImage,    FlecheAttaqDroite,PKGDATADIR_PICS "fleches/fleche_attaq_droite.png", true);
	R_RESOURCE_P(ECImage,    FlecheAttaqGauche,PKGDATADIR_PICS "fleches/fleche_attaq_gauche.png", true);
	R_RESOURCE_P(ECImage,    FlecheAttaqHaut,  PKGDATADIR_PICS "fleches/fleche_attaq_haut.png", true);
	R_RESOURCE_P(ECImage,    FlecheAttaqBas,   PKGDATADIR_PICS "fleches/fleche_attaq_bas.png", true);
	R_RESOURCE_P(ECImage,    PointerSelect,    PKGDATADIR_PICS "cursors/select.png", true);
	R_RESOURCE_P(ECImage,    PointerAttaq,     PKGDATADIR_PICS "cursors/attaq.png", true);
	R_RESOURCE_P(ECImage,    PointerMaintainedAttaq, PKGDATADIR_PICS "cursors/maintened_attaq.png", true);
	R_RESOURCE_P(ECImage,    PointerCantAttaq, PKGDATADIR_PICS "cursors/cantattaq.png", true);
	R_RESOURCE_P(ECImage,    PointerInvest,    PKGDATADIR_PICS "cursors/invest.png", true);
	R_RESOURCE_P(ECImage,    PointerTop,       PKGDATADIR_PICS "cursors/top.png", true);
	R_RESOURCE_P(ECImage,    PointerBottom,    PKGDATADIR_PICS "cursors/bottom.png", true);
	R_RESOURCE_P(ECImage,    PointerLeft,      PKGDATADIR_PICS "cursors/left.png", true);
	R_RESOURCE_P(ECImage,    PointerRight,     PKGDATADIR_PICS "cursors/right.png", true);
	R_RESOURCE_P(ECImage,    PointerRadar,     PKGDATADIR_PICS "cursors/radar.png", true);
	R_RESOURCE(ECSpriteBase, UpButton,         "upbutton");
	R_RESOURCE(ECSpriteBase, DownButton,       "downbutton");
	R_RESOURCE(ECSpriteBase, NormalButton,     "normalbutton");
	R_RESOURCE(ECSpriteBase, LitleButton,      "litlebutton");
	R_RESOURCE(ECSpriteBase, PlayButton,       "playbutton");
	R_RESOURCE(ECSpriteBase, OptionsButton,    "optionsbutton");
	R_RESOURCE(ECSpriteBase, CreditsButton,    "creditsbutton");
	R_RESOURCE(ECSpriteBase, QuitterButton,    "quitterbutton");
	R_RESOURCE(ECSpriteBase, CheckBox,         "checkbox");
	R_RESOURCE(ECSpriteBase, CaseMer,          "cases/mer");
	R_RESOURCE(ECSpriteBase, CaseTerre,        "cases/terre");
	R_RESOURCE(ECSpriteBase, CaseBordNord,     "cases/bordnord");
	R_RESOURCE(ECSpriteBase, CaseBordSud,      "cases/bordsud");
	R_RESOURCE(ECSpriteBase, CaseBordEst,      "cases/bordest");
	R_RESOURCE(ECSpriteBase, CaseBordOuest,    "cases/bordouest");
	R_RESOURCE(ECSpriteBase, CaseBordNordOuest,"cases/bordno");
	R_RESOURCE(ECSpriteBase, CaseBordNordEst,  "cases/bordne");
	R_RESOURCE(ECSpriteBase, CaseBordSudOuest, "cases/bordso");
	R_RESOURCE(ECSpriteBase, CaseBordSudEst,   "cases/bordse");
	R_RESOURCE(ECSpriteBase, CaseCoinSudEst,   "cases/coinse");
	R_RESOURCE(ECSpriteBase, CaseCoinSudOuest, "cases/coinso");
	R_RESOURCE(ECSpriteBase, CaseCoinNordEst,  "cases/coinno");
	R_RESOURCE(ECSpriteBase, CaseCoinNordOuest,"cases/coinne");
	R_RESOURCE(ECSpriteBase, CasePontHorizontal,"cases/ponthorizontal");
	R_RESOURCE(ECSpriteBase, CasePontVertical, "cases/pontvertical");
	R_RESOURCE(ECSpriteBase, CasePontHaut,     "cases/ponthaut");
	R_RESOURCE(ECSpriteBase, CasePontBas,      "cases/pontbas");
	R_RESOURCE(ECSpriteBase, CasePontGauche,   "cases/pontgauche");
	R_RESOURCE(ECSpriteBase, CasePontDroite,   "cases/pontdroite");
	R_RESOURCE(ECSpriteBase, CaseTerreDead,    "cases/terredead");
	R_RESOURCE(ECSpriteBase, CaseCityNODead,    "cases/citynodead");
	R_RESOURCE(ECSpriteBase, CaseCityNEDead,    "cases/citynedead");
	R_RESOURCE(ECSpriteBase, CaseCitySODead,    "cases/citysodead");
	R_RESOURCE(ECSpriteBase, CaseCitySEDead,    "cases/citysedead");
	R_RESOURCE(ECSpriteBase, Brouillard,       "brouillard");
	R_RESOURCE(ECImage,      Army_Icon,        PKGDATADIR_PICS "units/army_icon.png");
	R_RESOURCE(ECSpriteBase, Army_Face,        "units/army/face");
	R_RESOURCE(ECSpriteBase, Army_Left,        "units/army/left");
	R_RESOURCE(ECSpriteBase, Army_Right,       "units/army/right");
	R_RESOURCE(ECSpriteBase, Army_Dos,         "units/army/dos");
	R_RESOURCE(ECImage,      Caserne_Icon,     PKGDATADIR_PICS "units/caserne_icon.png");
	R_RESOURCE(ECSpriteBase, Caserne_Face,     "units/caserne");
	R_RESOURCE(ECImage,      Char_Icon,        PKGDATADIR_PICS "units/char_icon.png");
	R_RESOURCE(ECSpriteBase, Char_Face,        "units/char/face");
	R_RESOURCE(ECSpriteBase, Char_Left,        "units/char/left");
	R_RESOURCE(ECSpriteBase, Char_Right,       "units/char/right");
	R_RESOURCE(ECSpriteBase, Char_Dos,         "units/char/dos");
	R_RESOURCE(ECImage,      CharFact_Icon,    PKGDATADIR_PICS "units/charfact_icon.png");
	R_RESOURCE(ECSpriteBase, CharFact_Face,    "units/charfact");
	R_RESOURCE(ECImage,      MissiLauncher_Icon, PKGDATADIR_PICS "units/missilauncher_icon.png");
	R_RESOURCE(ECSpriteBase, MissiLauncher_Face, "units/missilauncher/face");
	R_RESOURCE(ECSpriteBase, MissiLauncher_Left, "units/missilauncher/left");
	R_RESOURCE(ECSpriteBase, MissiLauncher_Right,"units/missilauncher/right");
	R_RESOURCE(ECSpriteBase, MissiLauncher_Dos,  "units/missilauncher/dos");
	R_RESOURCE(ECSpriteBase, MissiLauncher_Deployed,"units/missilauncher/deployed");
	R_RESOURCE(ECSpriteBase, MissiLauncher_Reployed,"units/missilauncher/reployed");
	R_RESOURCE(ECSpriteBase, MissiLauncher_Missile_Up,  "units/missilauncher/missile_up");
	R_RESOURCE(ECSpriteBase, MissiLauncher_Missile_Down,"units/missilauncher/missile_down");
	R_RESOURCE(ECImage,      City_Icon,          PKGDATADIR_PICS "units/city_icon.png");
	R_RESOURCE(ECSpriteBase, City_Face,          "cases/ville");
	R_RESOURCE(ECImage,      Capitale_Icon,      PKGDATADIR_PICS "units/capitale_icon.png");
	R_RESOURCE(ECSpriteBase, Capitale_Face,      "cases/capitale");
	R_RESOURCE(ECImage,      Shipyard_Icon,      PKGDATADIR_PICS "units/shipyard_icon.png");
	R_RESOURCE(ECSpriteBase, Shipyard_Face,      "units/shipyard");
	R_RESOURCE(ECImage,      Boat_Icon,        PKGDATADIR_PICS "units/boat_icon.png");
	R_RESOURCE(ECSpriteBase, Boat_Face,        "units/boat/face");
	R_RESOURCE(ECSpriteBase, Boat_Left,        "units/boat/left");
	R_RESOURCE(ECSpriteBase, Boat_Right,       "units/boat/right");
	R_RESOURCE(ECSpriteBase, Boat_Dos,         "units/boat/dos");
	R_RESOURCE(ECImage,      NuclearSearch_Icon,PKGDATADIR_PICS "units/nuclearsearch_icon.png");
	R_RESOURCE(ECSpriteBase, NuclearSearch_Face,"units/nuclearsearch");
	R_RESOURCE(ECImage,      Silo_Icon,         PKGDATADIR_PICS "units/silo_icon.png");
	R_RESOURCE(ECSpriteBase, Silo_Face,         "units/silo");
	R_RESOURCE(ECSpriteBase, Silo_Missile_Up,   "units/silo/missile_up");
	R_RESOURCE(ECSpriteBase, Silo_Missile_Down, "units/silo/missile_down");
	R_RESOURCE(ECImage,      Enginer_Icon,      PKGDATADIR_PICS "units/enginer_icon.png");
	R_RESOURCE(ECSpriteBase, Enginer_Face,      "units/enginer/face");
	R_RESOURCE(ECSpriteBase, Enginer_Left,      "units/enginer/left");
	R_RESOURCE(ECSpriteBase, Enginer_Right,     "units/enginer/right");
	R_RESOURCE(ECSpriteBase, Enginer_Dos,       "units/enginer/dos");
	R_RESOURCE(ECImage,      DefenseTower_Icon, PKGDATADIR_PICS "units/defensetower_icon.png");
	R_RESOURCE(ECSpriteBase, DefenseTower_Face, "units/defensetower");
	R_RESOURCE(ECImage,      Tourist_Icon,      PKGDATADIR_PICS "units/tourist_icon.png");
	R_RESOURCE(ECSpriteBase, Tourist_Face,      "units/tourist/face");
	R_RESOURCE(ECSpriteBase, Tourist_Left,      "units/tourist/left");
	R_RESOURCE(ECSpriteBase, Tourist_Right,     "units/tourist/right");
	R_RESOURCE(ECSpriteBase, Tourist_Dos,       "units/tourist/dos");
	R_RESOURCE(ECImage,      Mine_Icon,         PKGDATADIR_PICS "units/mine_icon.png");
	R_RESOURCE(ECSpriteBase, Mine_DesactFace,   "units/mine/desactived");
	R_RESOURCE(ECSpriteBase, Mine_ActFace,      "units/mine/actived");
	R_RESOURCE(ECImage,      Obelisk_Icon,      PKGDATADIR_PICS "units/obelisk_icon.png");
	R_RESOURCE(ECSpriteBase, Obelisk_Face,      "units/obelisk");
};

#undef R_RESOURCE
#undef R_TYPE
#endif
