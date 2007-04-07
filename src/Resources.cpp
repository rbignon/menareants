/* src/Resources.cpp - Struct which define all resources.
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

#include <SDL.h>
#include <SDL_image.h>

#include "Resources.h"
#include "Defines.h"
#include "Sound.h"
#include "tools/Images.h"
#include "Units.h"

#define R_CLEARTYPE(type) for(std::vector<type*>::iterator it = type##_objects.begin(); it != type##_objects.end(); ++it) \
                          	delete *it; \
                          type##_objects.clear()
#define R_RESOURCE(type, name, url) type* Resources::name() { \
												if(!spr_##name) \
												{ \
													spr_##name = new type(url); \
													type##_objects.push_back(spr_##name); \
												} \
												return spr_##name; \
                                        	}\
                            type* Resources::spr_##name = NULL
#define R_RESOURCE_P(type, name, url, p) type* Resources::name() { \
												if(!spr_##name) \
												{ \
													spr_##name = new type(url, p); \
													type##_objects.push_back(spr_##name); \
												} \
												return spr_##name; \
                                        	}\
                            type* Resources::spr_##name = NULL

/* Ne pas oublier de rajouter dans Resources.h */

/* SONS */
	R_RESOURCE(Sound,        DingDong,         "dingdong.wav");
	R_RESOURCE(Sound,        SoundResources,   "resources.wav");
	R_RESOURCE(Sound,        SoundStart,       "start.wav");
	R_RESOURCE(Sound,        SoundBegin,       "begin.wav");
	R_RESOURCE(Sound,        SoundEnd,         "end.wav");
	R_RESOURCE(Sound,        SoundMitraillette,"bruits/machinegun.wav");
	R_RESOURCE(Sound,        SoundObelisque,   "bruits/laserfire.wav");
	R_RESOURCE(Sound,        SoundMcDo,        "units/Gerbale.ogg");
	R_RESOURCE(Sound,        SoundProut,       "units/prout.ogg");
	R_RESOURCE(Sound,        SoundMissile,     "units/missile.ogg");

/* IMAGES*/
	R_RESOURCE(ECSpriteBase, Intro,            "intro");
	R_RESOURCE(ECImage,      Titlescreen,      PKGDATADIR_PICS "background.png");
	R_RESOURCE_P(ECImage,    Title,            PKGDATADIR_PICS "title.png", true);
	R_RESOURCE_P(ECImage,    TitleMini,        PKGDATADIR_PICS "title_mini.png", true);
	R_RESOURCE(ECImage,      Loadscreen,       PKGDATADIR_PICS "loading.png");
	R_RESOURCE(ECImage,      BarreLat,         PKGDATADIR_PICS "barrelat.png");
	R_RESOURCE(ECImage,      BarreAct,         PKGDATADIR_PICS "barreact.png");
	R_RESOURCE(ECImage,      Cadre,            PKGDATADIR_PICS "cadre.png");
	R_RESOURCE(ECImage,      Case,             PKGDATADIR_PICS "case.png");
	R_RESOURCE_P(ECImage,    FogTop,           PKGDATADIR_PICS "fog-top.png", true);
	R_RESOURCE_P(ECImage,    FogBottom,        PKGDATADIR_PICS "fog-bottom.png", true);
	R_RESOURCE_P(ECImage,    FogRight,         PKGDATADIR_PICS "fog-right.png", true);
	R_RESOURCE_P(ECImage,    FogLeft,          PKGDATADIR_PICS "fog-left.png", true);
	R_RESOURCE_P(ECImage,    FogTopLeft,       PKGDATADIR_PICS "fog-topleft.png", true);
	R_RESOURCE_P(ECImage,    FogTopRight,      PKGDATADIR_PICS "fog-topright.png", true);
	R_RESOURCE_P(ECImage,    FogBottomLeft,    PKGDATADIR_PICS "fog-bottomleft.png", true);
	R_RESOURCE_P(ECImage,    FogBottomRight,   PKGDATADIR_PICS "fog-bottomright.png", true);
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
	R_RESOURCE_P(ECImage,    PointerStandard,  PKGDATADIR_PICS "cursors/standard.png", true);
	R_RESOURCE_P(ECImage,    PointerAttaq,     PKGDATADIR_PICS "cursors/attaq.png", true);
	R_RESOURCE_P(ECImage,    PointerMaintainedAttaq, PKGDATADIR_PICS "cursors/maintened_attaq.png", true);
	R_RESOURCE_P(ECImage,    PointerCantAttaq, PKGDATADIR_PICS "cursors/cantattaq.png", true);
	R_RESOURCE_P(ECImage,    PointerInvest,    PKGDATADIR_PICS "cursors/invest.png", true);
	R_RESOURCE_P(ECImage,    PointerLeft,      PKGDATADIR_PICS "cursors/left.png", true);
	R_RESOURCE_P(ECImage,    PointerRadar,     PKGDATADIR_PICS "cursors/radar.png", true);
	R_RESOURCE_P(ECImage,    PointerAddBP,     PKGDATADIR_PICS "cursors/add_bp.png", true);
	R_RESOURCE_P(ECImage,    PointerRemBP,     PKGDATADIR_PICS "cursors/rem_bp.png", true);
	R_RESOURCE_P(ECImage,    MoveMapPoint,     PKGDATADIR_PICS "move_map.png", true);
	R_RESOURCE(ECSpriteBase, Balise,           "balise");
	R_RESOURCE(ECSpriteBase, UpButton,         "upbutton");
	R_RESOURCE(ECSpriteBase, DownButton,       "downbutton");
	R_RESOURCE(ECSpriteBase, NormalButton,     "normalbutton");
	R_RESOURCE(ECSpriteBase, LitleButton,      "litlebutton");
	R_RESOURCE(ECSpriteBase, MissionButton,    "missionbutton");
	R_RESOURCE(ECSpriteBase, EscarmoucheButton,"escarmouchebutton");
	R_RESOURCE(ECSpriteBase, PlayButton,       "playbutton");
	R_RESOURCE(ECSpriteBase, OptionsButton,    "optionsbutton");
	R_RESOURCE(ECSpriteBase, MapEditorButton,  "mapeditorbutton");
	R_RESOURCE(ECSpriteBase, CreditsButton,    "creditsbutton");
	R_RESOURCE(ECSpriteBase, QuitButton,       "quitbutton");
	R_RESOURCE(ECSpriteBase, AccountButton,    "buttons/account");
	R_RESOURCE(ECSpriteBase, BackButton,       "buttons/back");
	R_RESOURCE(ECSpriteBase, CancelButton,     "buttons/cancel");
	R_RESOURCE(ECSpriteBase, MBackButton,      "buttons/mback");
	R_RESOURCE(ECSpriteBase, MCancelButton,    "buttons/mcancel");
	R_RESOURCE(ECSpriteBase, MOkButton,        "buttons/mok");
	R_RESOURCE(ECSpriteBase, OkButton,         "buttons/ok");
	R_RESOURCE(ECSpriteBase, RefreshButton,    "buttons/refresh");
	R_RESOURCE(ECSpriteBase, SaveButton,       "buttons/save");
	R_RESOURCE(ECSpriteBase, ScoresButton,     "buttons/scores");
	R_RESOURCE(ECSpriteBase, RadioButton,      "buttons/radio");
	R_RESOURCE(ECSpriteBase, AddButton,        "buttons/add");
	R_RESOURCE(ECSpriteBase, RemoveButton,     "buttons/remove");
	R_RESOURCE(ECSpriteBase, RefreshConfButton,"buttons/refreshconf");
	R_RESOURCE(ECSpriteBase, AIButton,         "buttons/AI");
	R_RESOURCE(ECSpriteBase, HostButton,       "buttons/host");
	R_RESOURCE(ECSpriteBase, CreateButton,     "buttons/create");
	R_RESOURCE(ECSpriteBase, JoinButton,       "buttons/join");
	R_RESOURCE(ECSpriteBase, KillButton,       "buttons/kill");
	R_RESOURCE(ECSpriteBase, MPlayButton,      "buttons/mplay");
	R_RESOURCE(ECSpriteBase, ReadyButton,      "buttons/ready");
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
	R_RESOURCE(ECSpriteBase, CaseCityNODead,   "cases/citynodead");
	R_RESOURCE(ECSpriteBase, CaseCityNEDead,   "cases/citynedead");
	R_RESOURCE(ECSpriteBase, CaseCitySODead,   "cases/citysodead");
	R_RESOURCE(ECSpriteBase, CaseCitySEDead,   "cases/citysedead");
	R_RESOURCE(ECSpriteBase, CaseMontain,      "cases/montagne");
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
	R_RESOURCE_P(ECImage,    DefenseTower_Missile, PKGDATADIR_ANIMS "units/defensetower/missile.png", true);
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
	R_RESOURCE(ECImage,      McDo_Icon,         PKGDATADIR_PICS "units/mcdo_icon.png");
	R_RESOURCE(ECSpriteBase, McDo_Face,         "units/mcdo/face");
	R_RESOURCE(ECSpriteBase, McDo_Left,         "units/mcdo/left");
	R_RESOURCE(ECSpriteBase, McDo_Right,        "units/mcdo/right");
	R_RESOURCE(ECSpriteBase, McDo_Dos,          "units/mcdo/dos");
	R_RESOURCE(ECSpriteBase, McDo_Caserne,      "units/mcdo/caserne");
	R_RESOURCE(ECSpriteBase, McDo_Jouano,       "units/mcdo/jouano");
	R_RESOURCE(ECImage,      Trees_Icon,        PKGDATADIR_PICS "units/trees_icon.png");
	R_RESOURCE(ECSpriteBase, Trees_Face,        "units/trees");
	R_RESOURCE(ECImage,      Megalopole_Icon,   PKGDATADIR_PICS "units/megalopole_icon.png");
	R_RESOURCE(ECSpriteBase, Megalopole_Face,   "units/megalopole");
	R_RESOURCE(ECImage,      Rail_Icon,         PKGDATADIR_PICS "units/rail_icon.png");
	R_RESOURCE(ECSpriteBase, Rail_Horiz,        "units/rail/horiz");
	R_RESOURCE(ECSpriteBase, Rail_Verti,        "units/rail/verti");
	R_RESOURCE(ECSpriteBase, Rail_Aiguillage,   "units/rail/aiguillage");
	R_RESOURCE(ECImage,      Train_Icon,        PKGDATADIR_PICS "units/train_icon.png");
	R_RESOURCE(ECSpriteBase, Train_Face,        "units/train/face");
	R_RESOURCE(ECSpriteBase, Train_Left,        "units/train/left");
	R_RESOURCE(ECSpriteBase, Train_Right,       "units/train/right");
	R_RESOURCE(ECSpriteBase, Train_Dos,         "units/train/dos");
	R_RESOURCE(ECImage,      Jouano_Icon,       PKGDATADIR_PICS "units/jouano_icon.png");
	R_RESOURCE(ECSpriteBase, Jouano_Face,       "units/jouano/face");
	R_RESOURCE(ECSpriteBase, Jouano_Fog,        "units/jouano/fog");
	R_RESOURCE(ECImage,      Plane_Icon,        PKGDATADIR_PICS "units/plane_icon.png");
	R_RESOURCE(ECSpriteBase, Plane_Face,        "units/plane/face");
	R_RESOURCE(ECSpriteBase, Plane_Left,        "units/plane/left");
	R_RESOURCE(ECSpriteBase, Plane_Right,       "units/plane/right");
	R_RESOURCE(ECSpriteBase, Plane_Dos,         "units/plane/dos");
	R_RESOURCE(ECSpriteBase, Plane_Deployed,    "units/plane/deployed");
	R_RESOURCE(ECSpriteBase, Plane_Reployed,    "units/plane/reployed");
	R_RESOURCE(ECImage,      BarbedWire_Icon,   PKGDATADIR_PICS "units/barbedwire_icon.png");
	R_RESOURCE(ECSpriteBase, BarbedWire_Horiz,        "units/barbedwire/horiz");
	R_RESOURCE(ECSpriteBase, BarbedWire_Verti,        "units/barbedwire/verti");
	R_RESOURCE(ECSpriteBase, BarbedWire_Aiguillage,   "units/barbedwire/aiguillage");
	R_RESOURCE(ECSpriteBase, BarbedWire_TNord,        "units/barbedwire/Tnord");
	R_RESOURCE(ECSpriteBase, BarbedWire_TSud,         "units/barbedwire/Tsud");
	R_RESOURCE(ECSpriteBase, BarbedWire_TEst,         "units/barbedwire/Test");
	R_RESOURCE(ECSpriteBase, BarbedWire_TOuest,       "units/barbedwire/Touest");
	R_RESOURCE(ECSpriteBase, BarbedWire_Left,         "units/barbedwire/left");
	R_RESOURCE(ECSpriteBase, BarbedWire_Right,        "units/barbedwire/right");
	R_RESOURCE(ECSpriteBase, BarbedWire_Up,           "units/barbedwire/up");
	R_RESOURCE(ECSpriteBase, BarbedWire_Down,         "units/barbedwire/down");
	R_RESOURCE(ECSpriteBase, BarbedWire_NordOuest,    "units/barbedwire/nordouest");
	R_RESOURCE(ECSpriteBase, BarbedWire_NordEst,      "units/barbedwire/nordest");
	R_RESOURCE(ECSpriteBase, BarbedWire_SudOuest,     "units/barbedwire/sudouest");
	R_RESOURCE(ECSpriteBase, BarbedWire_SudEst,       "units/barbedwire/sudest");
#undef R_RESOURCE
#undef R_RESOURCE_P
#define R_TYPE(type) std::vector<type*> Resources::type##_objects
	R_TYPE(ECImage);
	R_TYPE(ECSpriteBase);
	R_TYPE(Sound);
#undef R_TYPE

Resources::Resources()
{
}

Resources::~Resources()
{

}

void Resources::Init()
{
	ECJouano::Anim.Init();
}

void Resources::Unload()
{
	R_CLEARTYPE(ECImage);
	R_CLEARTYPE(ECSpriteBase);
	R_CLEARTYPE(Sound);
}

#undef R_CLEARTYPE
