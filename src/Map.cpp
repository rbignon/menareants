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

ECMap::ECMap(std::vector<std::string> _map_file)
	: ECBMap(_map_file)
{
	preview = 0;
	CreatePreview();
}

ECMap::ECMap(std::string filename)
	: ECBMap(filename)
{
	preview = 0;
	CreatePreview();
}

ECMap::~ECMap()
{
	if(preview) delete preview;
}

/** \note Preview use two screen pixels for one pixel in map */
void ECMap::CreatePreview()
{
	if(!initialised) return;

	if(preview) delete preview;

	SDL_Surface *surf = CreateRGBASurface(x*4, y*4, SDL_SWSURFACE|SDL_SRCALPHA);

	/* Dessine la preview */
	uint xx = 0, yy = 0;
	for(uint _y = 0; _y < y; ++_y, yy+=2, xx=0)
		for(uint _x = 0; _x < x; ++_x, xx+=2)
		{
			ECase *c = map[ _y * x + _x ];
			SDL_Color *color = 0;
			switch(c->TypeID())
			{
				case 'v':
				case 'V':
					color = &red_color;
					break;
				case 't':
					color = &brown_color;
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

			putpixel(surf, xx, yy, SDL_MapRGB(surf->format, color->r, color->g, color->b));
			putpixel(surf, xx+1, yy, SDL_MapRGB(surf->format, color->r, color->g, color->b));
			putpixel(surf, xx, yy+1, SDL_MapRGB(surf->format, color->r, color->g, color->b));
			putpixel(surf, xx+1, yy+1, SDL_MapRGB(surf->format, color->r, color->g, color->b));
		}

	/* Numero du player */
	for(std::vector<ECMapPlayer*>::iterator it = map_players.begin(); it != map_players.end(); ++it)
	{
		bool found = false;
		std::vector<ECountry*> coun = (*it)->Countries();
		for(std::vector<ECountry*>::iterator ci = coun.begin(); ci != coun.end() && !found; ++ci)
		{
			std::vector<ECase*> cas = (*ci)->Cases();
			for(std::vector<ECase*>::iterator casi = cas.begin(); casi != cas.end() && !found; ++casi)
			{
				if((*casi)->Flags() & (C_TERRE|C_VILLE|C_CAPITALE) && ((casi+1) == cas.end() || (casi+2) == cas.end() ||
				   ((*(casi+2))->X() == (*casi)->X()+2 && (*(casi+2))->Flags() & (C_TERRE|C_VILLE|C_CAPITALE))))
				{
					SDL_Surface *txtsurf = TTF_RenderText_Blended(&(app.Font()->tiny.GetTTF()),
					                                 TypToStr((*it)->Num()).c_str(), white_color);
					SDL_Rect dst_rect;
					dst_rect.x = (*casi)->X()*2+1;
					dst_rect.y = (*casi)->Y()*2+1;
					dst_rect.h = txtsurf->h;
					dst_rect.w = txtsurf->w;
					
					SDL_BlitSurface(txtsurf,NULL,surf, &dst_rect);
					found = true;
				}
			}
		}
	}
	preview = new ECImage(surf);
}
