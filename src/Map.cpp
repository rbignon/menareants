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
#include "Units.h"
#include "Batiments.h"
#include "MapEditor.h"

/********************************************************************************************
 *                               ECEntityList                                               *
 ********************************************************************************************/

ECEntityList EntityList;

ECEntityList::ECEntityList()
{
#define VECTOR
#include "lib/UnitsList.h"
#undef VECTOR
}

ECEntityList::~ECEntityList()
{
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		delete *it;
}

/********************************************************************************************
 *                                   ECMove                                                 *
 ********************************************************************************************/

void ECMove::EstablishDest()
{
	ECBCase* c = first_case;
	if(!c) return;

	for(Vector::const_iterator it = moves.begin(); it != moves.end(); ++it)
		switch(*it)
		{
			case Up: c = c->MoveUp(); break;
			case Down: c = c->MoveDown(); break;
			case Left: c = c->MoveLeft(); break;
			case Right: c = c->MoveRight(); break;
		}
	dest = c;
}

void ECMove::AddMove(E_Move m)
{
	ECBMove::AddMove(m);
	EstablishDest();
}
void ECMove::SetMoves(Vector _moves)
{
	ECBMove::SetMoves(_moves);
	EstablishDest();
}

/********************************************************************************************
 *                                 ECEntity                                                 *
 ********************************************************************************************/

ECEntity::ECEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, e_type _type, uint _Step, uint _nb,
                   uint _visibility)
		: ECBEntity(_name, _owner, _case, _type, _Step, _nb, _visibility), Tag(0), image(0), selected(false), move(this)
{
	//SetShowedCases(true);
}

ECEntity::~ECEntity()
{
	delete image;
}

bool ECEntity::Test(int souris_x, int souris_y)
{
	return (image && ((image->X() <= souris_x) && (souris_x < (int)(image->X()+image->GetWidth()))
	         && (image->Y() <= souris_y) && (souris_y < int(image->Y()+image->GetHeight()))));
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
	bool anim = false;
	if(image)
	{
		anim = image->Anim();
		MyFree(image);
	}
	if(!spr) return;
	image = new ECSprite(spr, app.sdlwindow);
	image->SetAnim(anim);
	if(Case() && dynamic_cast<ECMap*>(acase->Map())->ShowMap())
		image->set(dynamic_cast<ECMap*>(acase->Map())->ShowMap()->X() +(CASE_WIDTH  * acase->X()),
		           dynamic_cast<ECMap*>(acase->Map())->ShowMap()->Y() + (CASE_HEIGHT * acase->Y()));
}

void ECEntity::ChangeCase(ECBCase* newcase)
{
	SetShowedCases(false);
	ECBEntity::ChangeCase(newcase);
	SetShowedCases(true);

	if(dynamic_cast<ECMap*>(acase->Map())->ShowMap())
		image->set(dynamic_cast<ECMap*>(acase->Map())->ShowMap()->X() +(CASE_WIDTH  * acase->X()),
		           dynamic_cast<ECMap*>(acase->Map())->ShowMap()->Y() + (CASE_HEIGHT * acase->Y()));
}

void ECEntity::SetShowedCases(bool show)
{
	if(!Case() || !Owner() || !dynamic_cast<ECPlayer*>(Owner())->IsMe())
		return;

	ECBCase* c = Case()->MoveLeft(Visibility());
	c = c->MoveUp(Visibility());

	for(uint i=0; i <= 2*Visibility(); ++i)
	{
		for(uint j=0; j <= 2*Visibility(); ++j, c = c->MoveRight())
		{
			ECase* cc = dynamic_cast<ECase*>(c);
			if(show)
				cc->SetShowed(cc->Showed() < 0 ? 1 : cc->Showed() + 1);
			else if(cc->Showed() > 0)
				cc->SetShowed(cc->Showed()-1);
			//printf("(%d,%d) = %d\n", cc->X(), cc->Y(), cc->Showed());
		}
		c = c->MoveDown();
		c = c->MoveLeft(2*Visibility()+1);
	}
}

/********************************************************************************************
 *                                ECase                                                     *
 ********************************************************************************************/

ECase::ECase(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id)
	: ECBCase(_map, _x, _y, _flags, _type_id), image(0), selected(0), showed(-1)
{

}

ECase::~ECase()
{
	delete image;
}

bool ECase::Test(int souris_x, int souris_y)
{
	return (image && ((image->X() <= souris_x) && (souris_x < (int)(image->X()+image->GetWidth()))
	         && (image->Y() <= souris_y) && (souris_y < int(image->Y()+image->GetHeight()))));
}

void ECase::Draw()
{
	if(image)
	{
		image->draw();
		if(selected)
			Resources::Cadre()->Draw(image->X(),image->Y());
	}
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
	: ECBMap(_map_file), showmap(0), brouillard(false)
{
	preview = 0;
}

ECMap::ECMap(std::string filename)
	: ECBMap(filename), showmap(0), brouillard(false)
{
	preview = 0;
}

ECMap::~ECMap()
{
	if(preview) delete preview;
}

struct case_img_t case_img[] = {
	{ 't', Resources::CaseTerre,           't' },
	{ 'm', Resources::CaseMer,             'm' },
	{ 'a', Resources::CaseBordSud,         'm' },
	{ 'b', Resources::CaseBordNord,        'm' },
	{ 'c', Resources::CaseBordEst,         'm' },
	{ 'd', Resources::CaseBordOuest,       'm' },
	{ 'e', Resources::CaseBordSudEst,      'm' },
	{ 'f', Resources::CaseBordSudOuest,    'm' },
	{ 'g', Resources::CaseBordNordOuest,   'm' },
	{ 'h', Resources::CaseBordNordEst,     'm' },
	{ 'i', Resources::CaseCoinNordOuest,   'm' },
	{ 'j', Resources::CaseCoinNordEst,     'm' },
	{ 'k', Resources::CaseCoinSudEst,      'm' },
	{ 'l', Resources::CaseCoinSudOuest,    'm' },
	{ 'n', Resources::CasePontHorizontal,  'p' },
	{ 'o', Resources::CasePontVertical,    'p' },
	{ 'p', Resources::CasePontGauche,      'p' },
	{ 'q', Resources::CasePontDroite,      'p' },
	{ 'r', Resources::CasePontHaut,        'p' },
	{ 's', Resources::CasePontBas,         'p' },
};

void TBarreCaseIcons::GoLast(TObject* o, void* e)
{
	TBarreCaseIcons* parent = dynamic_cast<TBarreCaseIcons*>(o->Parent());

	assert(parent);

	if(parent->first > 0) --parent->first;

	parent->SetList();
}

void TBarreCaseIcons::GoNext(TObject* o, void* e)
{
	TBarreCaseIcons* parent = dynamic_cast<TBarreCaseIcons*>(o->Parent());

	assert(parent);

	if(parent->first < (ASIZE(case_img)-1)) ++parent->first;

	parent->SetList();
}

void TBarreCaseIcons::SetList()
{
	Clear();

	Init();

	int _x = X();
	uint _h = 0;
	TBarreCase* parent = dynamic_cast<TBarreCase*>(Parent());
	for(uint _i = first; _i < ASIZE(case_img); ++_i)
	{
		TImage* i = AddComponent(new TImage(_x, 0, case_img[_i].spr()->First(), false));
		_x += i->Width() + 1;
		if(i->Height() > _h) _h = i->Height();
		i->SetOnClick(TBarreCaseIcons::SelectCase, &case_img[_i]);
		if(_x + (2 * i->Width()) >= parent->Width()) break;
	}
	SetWidth(parent->Width()-X());
	SetHeight(_h);
}

void ECMap::SetCaseAttr(ECBCase* c, char id)
{
	if(!c) return;

	for(uint j=0; j < (sizeof case_img / sizeof *case_img); j++)
		if(case_img[j].c == id)
		{
			dynamic_cast<ECase*>(c)->SetImage(case_img[j].spr());
			dynamic_cast<ECase*>(c)->SetImgID(id);
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
void ECMap::CreatePreview(uint width, uint height, bool ingame)
{
	if(!initialised) return;

	uint size_x = width/x, size_y = height/y;

	SDL_Surface *surf = CreateRGBASurface(x*size_x, y*size_y, SDL_SWSURFACE|SDL_SRCALPHA);

	SLOCK(surf);
	/* Dessine la preview */
	uint xx = 0, yy = 0;
	for(uint _y = 0; _y < y; ++_y, yy+=size_y, xx=0)
		for(uint _x = 0; _x < x; ++_x, xx+=size_x)
		{
			ECBCase* cc = map[ _y * x + _x ];
			ECase *c = dynamic_cast<ECase*>(cc);
			if(!c || Brouillard() && c->Showed() < 0) continue;
			SDL_Color *color = 0;
			switch(c->TypeID())
			{
				case 'v':
				case 'V':
					color = &red_color;
					break;
				case 't':
					color = (c->Country() && c->Country()->Owner()) ?
					          c->Country()->Owner()->Player() ?
					            color_eq[c->Country()->Owner()->Player()->Color()]
					          : color_eq[c->Country()->Owner()->ID()%COLOR_MAX]
					        : &brown_color;
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

			if(!(c->Flags() & C_MER))
			{
				if(_y > 0 && map[(_y-1) * x + _x]->Country() != c->Country() && !(map[(_y-1) * x + _x]->Flags() & C_MER))
					marge |= MARGE_TOP;
				if(_x > 0 && map[_y * x + _x-1]->Country() != c->Country() && !(map[_y * x + _x-1]->Flags() & C_MER))
					marge |= MARGE_LEFT;
				if(size_x > 5 && size_y > 5)
				{
					if(_y < (y-1) && map[(_y+1) * x + _x]->Country() != c->Country() && !(map[(_y+1) * x+_x]->Flags() & C_MER))
						marge |= MARGE_BOTTOM;
					if(_x < (x-1) && map[_y * x + _x+1]->Country() != c->Country() && !(map[_y * x + _x+1]->Flags() & C_MER))
						marge |= MARGE_RIGHT;
				}
			}

			for(uint _yy = yy; _yy < yy+size_y; _yy++)
				for(uint _xx = xx; _xx < xx+size_x; _xx++)
				{
					SDL_Color *col = (marge & MARGE_TOP && _yy == yy ||
					                  marge & MARGE_LEFT && _xx == xx ||
					                  marge & MARGE_BOTTOM && _yy == yy+size_y-1 ||
					                  marge & MARGE_RIGHT && _xx == xx+size_x-1) ? marge_color : color;
					putpixel(surf, _xx, _yy, SDL_MapRGB(surf->format,
					              (Brouillard() && c->Showed()<=0) ? (col->r>60) ? col->r - 60 : 0 : col->r,
					              (Brouillard() && c->Showed()<=0) ? (col->g>60) ? col->g - 60 : 0 : col->g,
					              (Brouillard() && c->Showed()<=0) ? (col->b>60) ? col->b - 60 : 0 : col->b));
				}
		}
	/* Position des unités */
	if(ingame)
	{
		std::vector<ECBEntity*> ents = entities.List();
		for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
		{
			if(!(*enti)->Owner() || Brouillard() && dynamic_cast<ECase*>((*enti)->Case())->Showed() <= 0) continue;
			SDL_Color *col = color_eq[(*enti)->Owner()->Color()];
			yy = (*enti)->Case()->Y() * size_y;
			xx = (*enti)->Case()->X() * size_x;
			for(uint _yy = yy; _yy < yy+size_y; _yy++)
				for(uint _xx = xx; _xx < xx+size_x; _xx++)
					putpixel(surf, _xx, _yy, SDL_MapRGB(surf->format, col->r > 100 ? col->r - 30 : col->r + 30,
					                                                  col->g > 100 ? col->g - 30 : col->g + 30,
					                                                  col->b > 100 ? col->b - 30 : col->b + 30));
		}
	}

	/* Numero du player */
	if(!ingame)
	{
		for(std::vector<ECMapPlayer*>::iterator it = map_players.begin(); it != map_players.end(); ++it)
		{
			uint begin_x = Width(), begin_y = Height(), max_x = 0, max_y = 0;
			std::vector<ECountry*> coun = (*it)->Countries();
			for(std::vector<ECountry*>::iterator ci = coun.begin(); ci != coun.end(); ++ci)
			{
				std::vector<ECBCase*> cas = (*ci)->Cases();
				for(std::vector<ECBCase*>::iterator casi = cas.begin(); casi != cas.end(); ++casi)
				{
					if((*casi)->Flags() & (C_TERRE))
					{
						if(begin_x > (*casi)->X()) begin_x = (*casi)->X();
						if(begin_y > (*casi)->Y()) begin_y = (*casi)->Y();
						if(max_x < (*casi)->X()) max_x = (*casi)->X();
						if(max_y < (*casi)->Y()) max_y = (*casi)->Y();
					}
				}
			}
			SDL_Surface *txtsurf = TTF_RenderText_Blended(&(app.Font()->sm.GetTTF()),
			                                  TypToStr((*it)->Num()).c_str(), white_color);
			SDL_Rect dst_rect;
			dst_rect.x = (begin_x+max_x)/2 * size_x;
			dst_rect.y = (begin_y+max_y)/2 * size_y;
			dst_rect.h = txtsurf->h;
			dst_rect.w = txtsurf->w;

			SDL_BlitSurface(txtsurf,NULL,surf, &dst_rect);
			SDL_FreeSurface(txtsurf);
		}
	}

	SUNLOCK(surf);

	if(preview)
		preview->SetImage(surf);
	else
		preview = new ECImage(surf);
}
