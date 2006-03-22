/* src/Map.cpp - Map classes
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
 
#include "Map.h"
#include "Main.h"
#include "Outils.h"
#include "tools/Font.h"
#include "Debug.h"
#include "gui/ColorEdit.h"
#include "gui/ShowMap.h"
#include "Map.h"
#include "Resources.h"
#include "Channels.h"

/********************************************************************************************
 *                                 ECEntity                                                 *
 ********************************************************************************************/

ECEntity::ECEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, e_type _type, uint _Step, uint _nb)
		: ECBEntity(_name, _owner, _case, _type, _Step, _nb), Tag(0), image(0), selected(false), new_case(0)
{}

ECEntity::~ECEntity()
{
	delete image;
}

bool ECEntity::Test(int souris_x, int souris_y)
{
	return (image && ((image->X() <= souris_x) && (souris_x <= (int)(image->X()+image->GetWidth()))
	         && (image->Y() <= souris_y) && (souris_y <= int(image->Y()+image->GetHeight()))));
}

void ECEntity::Draw()
{
	if(image)
	{
		image->draw();
		if(selected)
			Resources::Cadre()->Draw(image->X(),image->Y());
	}
}

void ECEntity::SetImage(ECSpriteBase* spr)
{
	if(image) MyFree(image);
	if(!spr) return;
	image = new ECSprite(spr, app.sdlwindow);
	image->SetAnim(false);
	if(dynamic_cast<ECMap*>(acase->Map())->ShowMap())
		image->set(dynamic_cast<ECMap*>(acase->Map())->ShowMap()->X() +(CASE_WIDTH  * acase->X()),
		           dynamic_cast<ECMap*>(acase->Map())->ShowMap()->Y() + (CASE_HEIGHT * acase->Y()));
}

void ECEntity::ChangeCase(ECBCase* newcase)
{
	ECBEntity::ChangeCase(newcase);
	if(dynamic_cast<ECMap*>(acase->Map())->ShowMap())
		image->set(dynamic_cast<ECMap*>(acase->Map())->ShowMap()->X() +(CASE_WIDTH  * acase->X()),
		           dynamic_cast<ECMap*>(acase->Map())->ShowMap()->Y() + (CASE_HEIGHT * acase->Y()));
}

/********************************************************************************************
 *                                ECase                                                     *
 ********************************************************************************************/

ECase::ECase(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id)
	: ECBCase(_map, _x, _y, _flags, _type_id)
{
	image = 0;
}

ECase::~ECase()
{
	delete image;
}

bool ECase::Test(int souris_x, int souris_y)
{
	return (image && ((image->X() <= souris_x) && (souris_x <= (int)(image->X()+image->GetWidth()))
	         && (image->Y() <= souris_y) && (souris_y <= int(image->Y()+image->GetHeight()))));
}

void ECase::Draw()
{
	if(image)
		image->draw();
}

void ECase::SetImage(ECSpriteBase* spr)
{
	if(image) delete image;
	if(!spr) return;
	image = new ECSprite(spr, app.sdlwindow);
	if(dynamic_cast<ECMap*>(map)->ShowMap())
		image->set(dynamic_cast<ECMap*>(map)->ShowMap()->X() +(CASE_WIDTH  * x),
		           dynamic_cast<ECMap*>(map)->ShowMap()->Y() + (CASE_HEIGHT * x));
}

/********************************************************************************************
 *                                ECMap                                                     *
 ********************************************************************************************/

ECMap::ECMap(std::vector<std::string> _map_file)
	: ECBMap(_map_file), showmap(0)
{
	preview = 0;
}

ECMap::ECMap(std::string filename)
	: ECBMap(filename), showmap(0)
{
	preview = 0;
}

ECMap::~ECMap()
{
	if(preview) delete preview;
}

static struct
{
	char c;
	ECSpriteBase* (*spr) ();
} case_img[] = {
	{ 't', Resources::CaseTerre },
	{ 'm', Resources::CaseMer },
	{ 'v', Resources::CaseVille },
	{ 'V', Resources::CaseCapitale },
	{ 'a', Resources::CaseBordSud },
	{ 'b', Resources::CaseBordNord },
	{ 'c', Resources::CaseBordEst },
	{ 'd', Resources::CaseBordOuest },
	{ 'e', Resources::CaseBordSudEst },
	{ 'f', Resources::CaseBordSudOuest },
	{ 'g', Resources::CaseBordNordOuest },
	{ 'h', Resources::CaseBordNordEst },
	{ 'i', Resources::CaseCoinNordOuest },
	{ 'j', Resources::CaseCoinNordEst },
	{ 'k', Resources::CaseCoinSudEst },
	{ 'l', Resources::CaseCoinSudOuest }
};

void ECMap::SetCaseAttr(ECBCase* c, char id)
{
	if(!c) return;

	for(uint j=0; j < (sizeof case_img / sizeof *case_img); j++)
		if(case_img[j].c == id)
		{
			dynamic_cast<ECase*>(c)->SetImage(case_img[j].spr());
			return;
		}

	throw ECExcept(VIName(c->X()) VIName(c->Y()) VCName(id), "Le terrain graphique est introuvable");
}

template<typename T>
static ECBCase* CreateCase(ECBMap *map, uint x, uint y, uint flags, char type_id)
{
	return new T(map, x, y, flags, type_id);
}

static struct
{
	char c;
	ECBCase* (*func) (ECBMap *map, uint x, uint y, uint flgs, char type_id);
	uint flags;
} case_type[] = {
	{ 'v', CreateCase<ECVille>, C_VILLE            },
	{ 'V', CreateCase<ECVille>, C_VILLE|C_CAPITALE },
	{ 'm', CreateCase<ECMer>,   C_MER              },
	{ 't', CreateCase<ECTerre>, C_TERRE            },
	{ 'p', CreateCase<ECPont>,  C_PONT             }
};

ECBCase* ECMap::CreateCase(uint _x, uint _y, char type_id)
{
	for(uint j=0; j < (sizeof case_type / sizeof *case_type); j++)
		if(case_type[j].c == type_id)
			return case_type[j].func (this, _x, _y, case_type[j].flags, case_type[j].c);

	return 0;
}

/** \note Preview use two screen pixels for one pixel in map */
void ECMap::CreatePreview(uint width, uint height)
{
	if(!initialised) return;

	uint size_x = width/x, size_y = height/y;

	SDL_Surface *surf = CreateRGBASurface(x*size_x, y*size_y, SDL_SWSURFACE|SDL_SRCALPHA);

	/* Dessine la preview */
	uint xx = 0, yy = 0;
	for(uint _y = 0; _y < y; ++_y, yy+=size_y, xx=0)
		for(uint _x = 0; _x < x; ++_x, xx+=size_x)
		{
			ECase *c = dynamic_cast<ECase*>(map[ _y * x + _x ]);
			SDL_Color *color = 0;
			switch(c->TypeID())
			{
				case 'v':
				case 'V':
					color = &red_color;
					break;
				case 't':
					color = (c->Country()->Owner() && c->Country()->Owner()->Player()) ?
					        color_eq[c->Country()->Owner()->Player()->Color()] : &brown_color;
					break;
				case 'p':
					color = &gray_color;
					break;
				case 'm':
					color = &blue_color;
					break;
			}
			if(!color)
				throw ECExcept(VPName(c->TypeID()) VIName(_x) VIName(_y), "Terrain inconnu");

			SDL_Color *marge_color =  &white_color;
			const short MARGE_LEFT = 0x01;
			const short MARGE_RIGHT = 0x02;
			const short MARGE_TOP = 0x04;
			const short MARGE_BOTTOM = 0x08;
			unsigned short marge = 0;

			if(_y > 0 && map[(_y-1) * x + _x]->Country() != c->Country())
				marge |= MARGE_TOP;
			if(_x > 0 && map[_y * x + _x-1]->Country() != c->Country())
				marge |= MARGE_LEFT;
			if(size_x > 5 && size_y > 5)
			{
				if(_y < (y-1) && map[(_y+1) * x + _x]->Country() != c->Country())
					marge |= MARGE_BOTTOM;
				if(_x < (x-1) && map[_y * x + _x+1]->Country() != c->Country())
					marge |= MARGE_RIGHT;
			}

			for(uint _yy = yy; _yy < yy+size_y; _yy++)
				for(uint _xx = xx; _xx < xx+size_x; _xx++)
				{
					SDL_Color *col = (marge & MARGE_TOP && _yy == yy ||
					                  marge & MARGE_LEFT && _xx == xx ||
					                  marge & MARGE_BOTTOM && _yy == yy+size_y-1 ||
					                  marge & MARGE_RIGHT && _xx == xx+size_x-1) ? marge_color : color;
					putpixel(surf, _xx, _yy, SDL_MapRGB(surf->format, col->r, col->g, col->b));
				}
			/*putpixel(surf, xx+1, yy, SDL_MapRGB(surf->format, color->r, color->g, color->b));
			putpixel(surf, xx, yy+1, SDL_MapRGB(surf->format, color->r, color->g, color->b));
			putpixel(surf, xx+1, yy+1, SDL_MapRGB(surf->format, color->r, color->g, color->b));
			*/
		}

	/* Numero du player */
	for(std::vector<ECMapPlayer*>::iterator it = map_players.begin(); it != map_players.end(); ++it)
	{
		bool found = false;
		std::vector<ECountry*> coun = (*it)->Countries();
		for(std::vector<ECountry*>::iterator ci = coun.begin(); ci != coun.end() && !found; ++ci)
		{
			std::vector<ECBCase*> cas = (*ci)->Cases();
			for(std::vector<ECBCase*>::iterator casi = cas.begin(); casi != cas.end() && !found; ++casi)
			{
				if((*casi)->Flags() & (C_VILLE|C_CAPITALE)/* && ((casi+1) == cas.end() || (casi+2) == cas.end() ||
				   ((*(casi+2))->X() == (*casi)->X()+2 && (*(casi+2))->Flags() & (C_TERRE|C_VILLE|C_CAPITALE)))*/)
				{
					SDL_Surface *txtsurf = TTF_RenderText_Blended(&(app.Font()->small.GetTTF()),
					                                 TypToStr((*it)->Num()).c_str(), white_color);
					SDL_Rect dst_rect;
					dst_rect.x = (*casi)->X()*size_x+1;
					dst_rect.y = (*casi)->Y()*size_y+1;
					dst_rect.h = txtsurf->h;
					dst_rect.w = txtsurf->w;
					
					SDL_BlitSurface(txtsurf,NULL,surf, &dst_rect);
					found = true;
				}
			}
		}
	}
	if(preview) delete preview;
	preview = new ECImage(surf);
}
