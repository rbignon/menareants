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
	R_RESOURCE(ECImage,      Titlescreen,      PKGDATADIR_PICS "menu.png");
	R_RESOURCE(ECImage,      Menuscreen,       PKGDATADIR_PICS "menu_vide.png");
	R_RESOURCE(ECImage,      Loadscreen,       PKGDATADIR_PICS "loading.png");
	R_RESOURCE(ECImage,      BarreLat,         PKGDATADIR_PICS "barrelat.png");
	R_RESOURCE(ECImage,      Cadre,            PKGDATADIR_PICS "cadre.png");
	R_RESOURCE(ECImage,      FlecheVert,       PKGDATADIR_PICS "fleches/fleche_vert.png");
	R_RESOURCE(ECImage,      FlecheHoriz,      PKGDATADIR_PICS "fleches/fleche_horiz.png");
	R_RESOURCE(ECImage,      FlecheDroiteHaut, PKGDATADIR_PICS "fleches/fleche_droitehaut.png");
	R_RESOURCE(ECImage,      FlecheDroiteBas,  PKGDATADIR_PICS "fleches/fleche_droitebas.png");
	R_RESOURCE(ECImage,      FlecheGaucheHaut, PKGDATADIR_PICS "fleches/fleche_gauchehaut.png");
	R_RESOURCE(ECImage,      FlecheGaucheBas,  PKGDATADIR_PICS "fleches/fleche_gauchebas.png");
	R_RESOURCE(ECImage,      FlecheVersDroite, PKGDATADIR_PICS "fleches/fleche_vers_droite.png");
	R_RESOURCE(ECImage,      FlecheVersGauche, PKGDATADIR_PICS "fleches/fleche_vers_gauche.png");
	R_RESOURCE(ECImage,      FlecheVersHaut,   PKGDATADIR_PICS "fleches/fleche_vers_haut.png");
	R_RESOURCE(ECImage,      FlecheVersBas,    PKGDATADIR_PICS "fleches/fleche_vers_bas.png");
	R_RESOURCE(ECImage,      FlecheAttaqDroite,PKGDATADIR_PICS "fleches/fleche_attaq_droite.png");
	R_RESOURCE(ECImage,      FlecheAttaqGauche,PKGDATADIR_PICS "fleches/fleche_attaq_gauche.png");
	R_RESOURCE(ECImage,      FlecheAttaqHaut,  PKGDATADIR_PICS "fleches/fleche_attaq_haut.png");
	R_RESOURCE(ECImage,      FlecheAttaqBas,   PKGDATADIR_PICS "fleches/fleche_attaq_bas.png");
	R_RESOURCE(ECSpriteBase, UpButton,         "upbutton");
	R_RESOURCE(ECSpriteBase, DownButton,       "downbutton");
	R_RESOURCE(ECSpriteBase, NormalButton,     "normalbutton");
	R_RESOURCE(ECSpriteBase, LitleButton,      "litlebutton");
	R_RESOURCE(ECSpriteBase, CaseMer,          "cases/mer");
	R_RESOURCE(ECSpriteBase, CaseTerre,        "cases/terre");
	R_RESOURCE(ECSpriteBase, CaseVille,        "cases/ville");
	R_RESOURCE(ECSpriteBase, CaseCapitale,     "cases/capitale");
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
	R_RESOURCE(ECSpriteBase, Army_Face,        "units/army/face");
	R_RESOURCE(ECSpriteBase, Army_Left,        "units/army/left");
	R_RESOURCE(ECSpriteBase, Army_Right,       "units/army/right");
	R_RESOURCE(ECSpriteBase, Army_Dos,         "units/army/dos");
};

#undef R_RESOURCE
#undef R_TYPE
#endif
