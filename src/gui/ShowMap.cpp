/* src/gui/ShowMap.cpp - This is a component to show the map
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

#include "ShowMap.h"
#include "Defines.h"
#include "Resources.h"
#include "Channels.h"
#include "Debug.h"
#include "tools/Font.h"
#include "Form.h"
#include "Units.h"
#include "gui/ColorEdit.h"
#include <SDL_gfxPrimitives.h>

#undef SCREEN_HEIGHT
#define SCREEN_HEIGHT (Window()->GetHeight())
#undef SCREEN_WIDTH
#define SCREEN_WIDTH (Window()->GetWidth())

void TMap::Init()
{
	/* L'appel à cette fonction va permettre de placer correctement toutes les unités (voir la fonction en question) */
	SetPosition(X(), Y(), true);

	/* On définie la taille de la carte graphiquement */
	size.y = CASE_HEIGHT * map->Height();
	size.x = CASE_WIDTH  * map->Width();

	// Si en +SENDING on a envoyé une attaque, la non présence des images à l'écran faussera le dessin créé
	std::vector<ECBEntity*> ents = Map()->Entities()->List();
	for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
		dynamic_cast<ECEntity*>(*enti)->SetAttaquedCase(dynamic_cast<ECEntity*>(*enti)->AttaquedCase());

	/* On dit que ce composant est toujours redessiné */
	SetAlwaysRedraw();
	SetMustRedraw();
}

void TMap::LockScreen() const
{
	if(mutex)
		SDL_LockMutex(mutex);
}

void TMap::UnlockScreen() const
{
	if(mutex)
		SDL_UnlockMutex(mutex);
}

void TMap::RemoveAfterDraw(ECSprite* s)
{
	for(std::vector<ECSprite*>::iterator it = after_draw.begin(); it != after_draw.end();)
	{
		if(*it == s)
		{
			after_draw.erase(it);
			return;
		}
		else
			++it;
	}
}

ECase* TMap::Pixel2Case(const Point2i& pixel)
{
	uint x_ = (pixel.x - X()) / CASE_WIDTH;
	uint y_ = (pixel.y - Y()) / CASE_HEIGHT;
	if(x_ >= Map()->Width())  x_ = Map()->Width()  - 1;
	if(y_ >= Map()->Height()) y_ = Map()->Height() - 1;
	return dynamic_cast<ECase*>((*Map())(x_, y_));
}

std::vector<ECase*> TMap::Rect2Case(const Rectanglei& rect)
{
	std::vector<ECase*> cases;

	ECBCase* c = Pixel2Case(rect.GetPosition());
	uint nb_x = (rect.X() % CASE_WIDTH  + rect.Width()) / CASE_WIDTH + 1;
	uint nb_y = (rect.Y() % CASE_HEIGHT + rect.Height()) / CASE_HEIGHT + 1;

	//printf("%d,%d,%d,%d\n", c->X(), c->Y(), nb_x, nb_y);

	for(uint i=0; i <= nb_y; ++i)
	{
		uint j=0;
		for(; j <= nb_x; ++j)
		{
			ECase* cc = dynamic_cast<ECase*>(c);

			cases.push_back(cc);

			if(c->X() == Map()->Width()-1)
				break;
			c = c->MoveRight();
		}
		if(c->Y() == Map()->Height()-1)
			break;
		c = c->MoveDown();
		c = c->MoveLeft(j);
	}
	return cases;
}

void TMap::ToRedraw(ECSprite* img)
{
	ToRedraw(Rectanglei(img->X(), img->Y(), img->GetWidth(), img->GetHeight()));
}

void TMap::ToRedraw(const Point2i& pixel)
{
	ToRedraw(Pixel2Case(pixel));
}

void TMap::ToRedraw(const Rectanglei& rect)
{
	LockScreen();
	std::vector<ECase*> cases = Rect2Case(rect);
	for(std::vector<ECase*>::iterator it = cases.begin(); it != cases.end(); ++it)
		(*it)->SetMustRedraw();
	UnlockScreen();
}

void TMap::ToRedraw(TComponent* c)
{
	assert(c);
	ToRedraw(*c);
}

void TMap::ToRedraw(ECEntity* e)
{
	assert(e);
	ToRedraw(Rectanglei(e->Image()->X(), e->Image()->Y(), e->Image()->GetWidth(), e->Image()->GetHeight()));
}

void TMap::ToRedraw(ECase* c)
{
	assert(c);
	LockScreen();
	c->SetMustRedraw();
	UnlockScreen();
}

void TMap::SetMustRedraw(bool b)
{
	LockScreen();
	must_redraw = b;
	UnlockScreen();
}

void TMap::SetSchema(bool s)
{
	schema = s;
	SetMustRedraw();
}

void TMap::SetXY(int _x, int _y)
{
	SetPosition(_x, _y);
}

void TMap::MoveTo(int _x, int _y)
{
	SetPosition(- _x, - _y);
}

void TMap::CenterTo(int _x, int _y)
{
	MoveTo(_x - SCREEN_WIDTH/2, _y - SCREEN_HEIGHT/2);
}

void TMap::ScrollTo(int x2, int y2)
{
	int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py, x1, y1;

	x1 = - X() + SCREEN_WIDTH/2;
	y1 = - Y() + SCREEN_HEIGHT/2;

	dx = x2 - x1;               /* the horizontal distance of the line */
	dy = y2 - y1;               /* the vertical distance of the line */
	dxabs = abs(dx);
	dyabs = abs(dy);
	sdx = sgn(dx);
	sdy = sgn(dy);
	x = dyabs >> 1;
	y = dxabs >> 1;
	px = x1;
	py = y1;

	if(dxabs >= dyabs)          /* the line is more horizontal than vertical */
	{
		for(i = 0; i < dxabs; i++)
		{
			y += dyabs;
			if(y >= dxabs)
			{
				y -= dxabs;
				py += sdy;
			}
			px += sdx;
			if(px < 0 || px%50)
				continue;
			CenterTo(px, py);
			SDL_Delay(20);
		}
	}
	else                        /* the line is more vertical than horizontal */
	{
		for(i = 0; i < dyabs; i++)
		{
			x += dxabs;
			if(x >= dyabs)
			{
				x -= dyabs;
				px += sdx;
			}
			py += sdy;
			if(py < 0 || py%50)
				continue;
			CenterTo(px, py);
			SDL_Delay(20);
		}
	}
}

void TMap::SetPosition(int _x, int _y, bool force)
{
	if(!map) return;

	/* On major ou minor _x si jamais ça souhaite placer la map inconvenablement */
	if(_x > 0) _x = 0;
	if(Xmin() <= 0 && _x < Xmin()) _x = Xmin();
	if(Xmin() > 0) _x = X();

	if(_y > 0) _y = 0;
	if(Ymin() <= 0 && _y < Ymin()) _y = Ymin();
	if(Ymin() > 0) _y = Y();

	if(!force && X() == _x && Y() == _y) return;

	position.x = _x;
	position.y = _y;

	/* On redéfinie la position à l'écran de toutes les cases */
	BCaseVector cases = map->Cases();
	for(BCaseVector::iterator casi = cases.begin(); casi != cases.end(); ++casi)
		if(*casi)
			dynamic_cast<ECase*>(*casi)->Image()->set(X()+(CASE_WIDTH  * (*casi)->X()),
		                                                  Y()+(CASE_HEIGHT * (*casi)->Y()));

	/* On redéfini à l'écran la position de toutes les unités */
	std::vector<ECBEntity*> entities = map->Entities()->List();
	for(std::vector<ECBEntity*>::iterator enti = entities.begin(); enti != entities.end(); ++enti)
		if((*enti)->Case())
			dynamic_cast<ECEntity*>(*enti)->Image()->set(X()+(CASE_WIDTH  * (*enti)->Case()->X()),
		                                                     Y()+(CASE_HEIGHT * (*enti)->Case()->Y()));

	SetMustRedraw();
}

ECEntity* TMap::TestEntity(const Point2i& mouse)
{
	//if(!Focused()) return 0;

	ECase* c = TestCase(mouse);

	if(c->Entities()->Empty() || (Map()->Channel() && !ECMap::CanSelect(c))) return 0;

	ECEntity* e = dynamic_cast<ECEntity*>(c->Entities()->First());

	c->Entities()->Remove(e);
	c->Entities()->Add(e);

	return (Map()->Channel() && (!e->CanBeSelected() || e->IsHiddenOnCase())) ? TestEntity(mouse) : e;
}

ECase* TMap::TestCase(const Point2i& mouse)
{
	//if(!Focused()) return 0;

#if 0
	BCaseVector cases = map->Cases();
	for(BCaseVector::iterator casi = cases.begin(); casi != cases.end(); ++casi)
		if(*casi && dynamic_cast<ECase*>(*casi)->Test(mouse_x, mouse_y))
			return dynamic_cast<ECase*>(*casi);
	return 0;
#endif
	return Pixel2Case(mouse);
}

void TMap::DrawFog(ECase* c)
{
	// Fog borders: if adjacents cases are fogged and this one is not, then we add fog borders!
	if(c->Showed() < 0 || !HaveBrouillard())
		return;

	/* La fonction dessine juste les contours du brouillard noir. */

	ECase* up = dynamic_cast<ECase*>(c->MoveUp());
	ECase* down = dynamic_cast<ECase*>(c->MoveDown());
	ECase* left = dynamic_cast<ECase*>(c->MoveLeft());
	ECase* right = dynamic_cast<ECase*>(c->MoveRight());

	// Side borders
	if(up->Showed() < 0)
		Resources::FogTop()->Draw(c->Image()->X(), c->Image()->Y());
	if(down->Showed() < 0)
		Resources::FogBottom()->Draw(c->Image()->X(), c->Image()->Y() + CASE_HEIGHT - Resources::FogBottom()->GetHeight());
	if(right->Showed() < 0)
		Resources::FogRight()->Draw(c->Image()->X() + CASE_WIDTH - Resources::FogRight()->GetWidth(), c->Image()->Y());
	if(left->Showed() < 0)
		Resources::FogLeft()->Draw(c->Image()->X(), c->Image()->Y());
	// Corner borders
	if(up->Showed() >= 0
	   && left->Showed() >= 0
	   && (dynamic_cast<ECase*>(left->MoveUp()))->Showed() < 0)
		Resources::FogTopLeft()->Draw(c->Image()->X(), c->Image()->Y());
	if(up->Showed() >= 0
	   && right->Showed() >= 0
	   && (dynamic_cast<ECase*>(right->MoveUp()))->Showed() < 0)
		Resources::FogTopRight()->Draw(c->Image()->X() + CASE_WIDTH - Resources::FogTopRight()->GetWidth(), c->Image()->Y());
	if(down->Showed() >= 0
	   && left->Showed() >= 0
	   && (dynamic_cast<ECase*>(left->MoveDown()))->Showed() < 0)
		Resources::FogBottomLeft()->Draw(c->Image()->X(),
		                                 c->Image()->Y() + CASE_HEIGHT - Resources::FogBottomLeft()->GetHeight());
	if(down->Showed() >= 0
	   && right->Showed() >= 0
	   && (dynamic_cast<ECase*>(right->MoveDown()))->Showed() < 0)
		Resources::FogBottomRight()->Draw(c->Image()->X() + CASE_WIDTH - Resources::FogBottomRight()->GetWidth(),
		                                  c->Image()->Y() + CASE_HEIGHT - Resources::FogBottomRight()->GetHeight());
}

void TMap::DrawCountries(ECase* c)
{
	const int LINE_WIDTH = 10;
	if(!c->Visible())
		return;

	if(map->Channel() && (!c->Country()->Owner() || !c->Country()->Owner()->Player()))
		return;

	/* !WARNING! Le code qui suit risque de faire mal aux yeux. Prière de faire bien attention en lisant ces lignes de
	 *           ne pas tomber dans un coma.
	 *           Merci.
	 */

	if((map->Channel() && (!c->MoveUp()->Country()->Owner() || !c->MoveUp()->Country()->Owner()->Player()
	                       || c->MoveUp()->Country()->Owner()->Player() != c->Country()->Owner()->Player()))
	   || (!map->Channel() && c->MoveUp()->Country() != c->Country()))
		Window()->BoxColor(Rectanglei(c->Image()->X(), c->Image()->Y(), CASE_WIDTH, LINE_WIDTH),
		                   c->Country()->Owner() && c->Country()->Owner()->Player() ? color_eq[c->Country()->Owner()->Player()->Color()] : white_color);

	if((map->Channel() && (!c->MoveDown()->Country()->Owner() || !c->MoveDown()->Country()->Owner()->Player()
	                       || c->MoveDown()->Country()->Owner()->Player() != c->Country()->Owner()->Player()))
	   || (!map->Channel() && c->MoveDown()->Country() != c->Country()))
		Window()->BoxColor(Rectanglei(c->Image()->X(), c->Image()->Y() + CASE_HEIGHT - LINE_WIDTH, CASE_WIDTH, LINE_WIDTH),
		                   c->Country()->Owner() && c->Country()->Owner()->Player() ? color_eq[c->Country()->Owner()->Player()->Color()] : white_color);

	if((map->Channel() && (!c->MoveLeft()->Country()->Owner() || !c->MoveLeft()->Country()->Owner()->Player()
	                       || c->MoveLeft()->Country()->Owner()->Player() != c->Country()->Owner()->Player()))
	   || (!map->Channel() && c->MoveLeft()->Country() != c->Country()))
		Window()->BoxColor(Rectanglei(c->Image()->X(), c->Image()->Y(), LINE_WIDTH, CASE_HEIGHT),
		                   c->Country()->Owner() && c->Country()->Owner()->Player() ? color_eq[c->Country()->Owner()->Player()->Color()] : white_color);

	if((map->Channel() && (!c->MoveRight()->Country()->Owner() || !c->MoveRight()->Country()->Owner()->Player()
	                       || c->MoveRight()->Country()->Owner()->Player() != c->Country()->Owner()->Player()))
	   || (!map->Channel() && c->MoveRight()->Country() != c->Country()))
		Window()->BoxColor(Rectanglei(c->Image()->X() + CASE_WIDTH - LINE_WIDTH, c->Image()->Y(), LINE_WIDTH, CASE_HEIGHT),
		                   c->Country()->Owner() && c->Country()->Owner()->Player() ? color_eq[c->Country()->Owner()->Player()->Color()] : white_color);

	if((map->Channel() && (!c->MoveUp()->MoveRight()->Country()->Owner() || !c->MoveUp()->MoveRight()->Country()->Owner()->Player()
	                       || c->MoveUp()->MoveRight()->Country()->Owner()->Player() != c->Country()->Owner()->Player()))
	   || (!map->Channel() && c->MoveUp()->MoveRight()->Country() != c->Country()))
		Window()->BoxColor(Rectanglei(c->Image()->X() + CASE_WIDTH - LINE_WIDTH, c->Image()->Y(), LINE_WIDTH, LINE_WIDTH),
		                   c->Country()->Owner() && c->Country()->Owner()->Player() ? color_eq[c->Country()->Owner()->Player()->Color()] : white_color);

	if((map->Channel() && (!c->MoveUp()->MoveLeft()->Country()->Owner() || !c->MoveUp()->MoveLeft()->Country()->Owner()->Player()
	                       || c->MoveUp()->MoveLeft()->Country()->Owner()->Player() != c->Country()->Owner()->Player()))
	   || (!map->Channel() && c->MoveUp()->MoveLeft()->Country() != c->Country()))
		Window()->BoxColor(Rectanglei(c->Image()->X(), c->Image()->Y(), LINE_WIDTH, LINE_WIDTH),
		                   c->Country()->Owner() && c->Country()->Owner()->Player() ? color_eq[c->Country()->Owner()->Player()->Color()] : white_color);

	if((map->Channel() && (!c->MoveDown()->MoveRight()->Country()->Owner() || !c->MoveDown()->MoveRight()->Country()->Owner()->Player()
	                       || c->MoveDown()->MoveRight()->Country()->Owner()->Player() != c->Country()->Owner()->Player()))
	   || (!map->Channel() && c->MoveDown()->MoveRight()->Country() != c->Country()))
		Window()->BoxColor(Rectanglei(c->Image()->X() + CASE_WIDTH - LINE_WIDTH, c->Image()->Y() + CASE_HEIGHT - LINE_WIDTH, LINE_WIDTH, LINE_WIDTH),
		                   c->Country()->Owner() && c->Country()->Owner()->Player() ? color_eq[c->Country()->Owner()->Player()->Color()] : white_color);

	if((map->Channel() && (!c->MoveDown()->MoveLeft()->Country()->Owner() || !c->MoveDown()->MoveLeft()->Country()->Owner()->Player()
	                       || c->MoveDown()->MoveLeft()->Country()->Owner()->Player() != c->Country()->Owner()->Player()))
	   || (!map->Channel() && c->MoveDown()->MoveLeft()->Country() != c->Country()))
		Window()->BoxColor(Rectanglei(c->Image()->X(), c->Image()->Y() + CASE_HEIGHT - LINE_WIDTH, LINE_WIDTH, LINE_WIDTH),
		                   c->Country()->Owner() && c->Country()->Owner()->Player() ? color_eq[c->Country()->Owner()->Player()->Color()] : white_color);
}

void TMap::Draw(const Point2i& mouse)
{
	if(!map) return;

	if(Enabled())
	{
		/* Si la carte est active, on peut scroller dessus.*/
		int xx = X(), yy = Y();

		if(move_map)
		{ /* Le joueur avait fait un clic molette appuyé et du coup on bouge par rapport à ce point de départ */
			xx += move_point.x - mouse.x;
			yy += move_point.y - mouse.y;
		}
		else
		{
			/* Changement de position automatique */
			if(mouse.x >= 0 && mouse.x < 20)
				xx += 30 - mouse.x;
			if(mouse.y >= 0 && mouse.y < 20)
				yy += 30 - mouse.y;
			if(mouse.x > int(SCREEN_WIDTH-20) && mouse.x <= int(SCREEN_WIDTH))
				xx -= 30 - (SCREEN_WIDTH - mouse.x);
			if(mouse.y > int(SCREEN_HEIGHT-20) && mouse.y <= int(SCREEN_HEIGHT))
				yy -= 30 - (SCREEN_HEIGHT - mouse.y);
		}

		if(xx != X() || yy != Y())
			SetXY(xx, yy);
	}
	else move_map = false;

	/* Boucle sur les cases pour les dessiner */
	BCaseVector cases = map->Cases();
	for(BCaseVector::iterator casi = cases.begin(); casi != cases.end(); ++casi)
	{
		ECase* c = dynamic_cast<ECase*>(*casi);
		if(!c) continue;

		/* Si le dessin de la case sort de l'écran, on passe */
		if(c->Image()->X() > X()+Width() ||
		   c->Image()->X()+c->Image()->GetWidth() < X() ||
		   c->Image()->Y() > Y()+Height() ||
		   c->Image()->Y()+c->Image()->GetHeight() < Y())
			continue;

		/* Uniquement si on doit redessiner la case, ou que l'image de la case est animée, ou autres excuses */
		if(MustRedraw() || c->MustRedraw() || c->Image()->Anim() || CreateEntity() || SelectedEntity())
		{
			/* Si brouillard et que le Showed de la case est à -1, donc que la case est noir, on dessine un rectangle noir */
			if(HaveBrouillard() && c->Showed() < 0)
			{
				SDL_Rect r_back = {c->Image()->X(),c->Image()->Y(),CASE_WIDTH,CASE_HEIGHT};
				Window()->FillRect(r_back, 0);
				continue;
			}

			c->Draw();

			/* Trucs dessinés si il y a une unité sélectionnée */
			if(SelectedEntity() && SelectedEntity()->Owner() && SelectedEntity()->Owner()->IsMe() && SelectedEntity()->Owner()->Ready() == false)
			{
				bool move, invest;

				/* Si l'unité sélectionnée est un conteneur et contient quelqu'un, on colorie la case si c'est une case de destination */
				if(dynamic_cast<TForm*>(Parent()) && dynamic_cast<TForm*>(Parent())->IsPressed(SDLK_SPACE) &&
				   dynamic_cast<EContainer*>(SelectedEntity()) &&
				   dynamic_cast<EContainer*>(SelectedEntity())->Containing())
				{
					if(((SelectedEntity()->DestCase()->X() == c->X() && (SelectedEntity()->DestCase()->Y() == c->Y()+1 || SelectedEntity()->DestCase()->Y() == c->Y()-1)) ||
					    (SelectedEntity()->DestCase()->Y() == c->Y() && (SelectedEntity()->DestCase()->X() == c->X()+1 || SelectedEntity()->DestCase()->X() == c->X()-1)))
					   && dynamic_cast<EContainer*>(SelectedEntity())->Containing()->CanWalkOn(c)
					   && SelectedEntity()->Move()->Size() < SelectedEntity()->MyStep())
					{
						ECImage background;
						SDL_Rect r_back = {0,0,CASE_WIDTH,CASE_HEIGHT};
						background.SetImage(SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, CASE_WIDTH, CASE_HEIGHT,
											32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
						background.FillRect(r_back, background.MapRGBA(0x89, 0x74, 0xff, 255*3/10));
						SDL_Rect r_back2 = {c->Image()->X(),c->Image()->Y(),CASE_WIDTH,CASE_HEIGHT};
						Window()->Blit(background, &r_back2);
					}
				}

				/* Si l'unité sélectionnée peut bouger sur la case, on la colorie */
				if(SelectedEntity()->Case() != c && SelectedEntity()->Case()->Delta(c) <= SelectedEntity()->MyStep() &&
				   (!SelectedEntity()->Deployed() ^ !!(SelectedEntity()->EventType() & ARM_DEPLOY)))
				{
					if(SelectedEntity()->CanWalkTo(c, move, invest))
					{
						ECImage background;
						SDL_Rect r_back = {0,0,CASE_WIDTH,CASE_HEIGHT};
						background.SetImage(SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, CASE_WIDTH, CASE_HEIGHT,
											32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
						background.FillRect(r_back, background.MapRGBA(255, 249, 126, 255*3/10));
						SDL_Rect r_back2 = {c->Image()->X(),c->Image()->Y(),CASE_WIDTH,CASE_HEIGHT};
						Window()->Blit(background, &r_back2);
					}
				}

				/* Si l'unité sélectionnée peut tirer sur cette case,on la colorie */
				if(SelectedEntity()->WantAttaq(0,0) && !(SelectedEntity()->EventType() & ARM_ATTAQ) &&
				   c->Delta(SelectedEntity()->Case()) <= SelectedEntity()->Porty())
				{
					ECImage background;
					SDL_Rect r_back = {0,0,CASE_WIDTH,CASE_HEIGHT};
					background.SetImage(SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, CASE_WIDTH, CASE_HEIGHT,
										32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
					background.FillRect(r_back, background.MapRGBA(255, 0, 0, 255*3/10));
					SDL_Rect r_back2 = {c->Image()->X(),c->Image()->Y(),CASE_WIDTH,CASE_HEIGHT};
					Window()->Blit(background, &r_back2);
				}
			}

			/* CreateEntity() pointe vers une entité qu'on veut créer. On dessine donc sur la case les hashures blanches ou rouges */
			if(CreateEntity() && c->Test(mouse.x, mouse.y))
					((!CreateEntity()->Owner() || CreateEntity()->CanBeCreated(c)) ? Resources::GoodHashure()
					                                                               : Resources::BadHashure())
					                     ->Draw(c->Image()->X(), c->Image()->Y());

			/* En mode schema on dessine le quadrillage et le contour des territoires */
			if(schema)
			{
				Resources::Case()->Draw(c->Image()->X(), c->Image()->Y());
				DrawCountries(c);
			}

			if(map->Channel()) // Si il n'y a pas de channel, c'est l'éditeur de map et on n'utilise pas ça dans ce cas.
				c->SetMustRedraw(false);

			DrawFog(c);
		}

		/* Si il y a des unités sur cette case, on boucle et on dessine les bâtiments */
		if(!c->Entities()->Empty())
		{
			std::vector<ECBEntity*> ents = c->Entities()->List();
			FOR(ECBEntity*, ents, entity)
				if(entity && entity->IsBuilding() && !dynamic_cast<ECEntity*>(entity)->OnTop())
					dynamic_cast<ECEntity*>(entity)->Draw();
		}
	}

	/* Maintenant on va dessiner les autres unités */
	std::vector<ECBEntity*> entities = map->Entities()->List();
	for(std::vector<ECBEntity*>::iterator enti = entities.begin(); enti != entities.end();)
		if(!(*enti))
		{
			map->RemoveAnEntity(*enti);
			enti = entities.erase(enti);
		}
		else
		{
			ECEntity* entity = dynamic_cast<ECEntity*>(*enti);
			if(entity &&
			   entity->Image()->X() <= Window()->GetWidth() &&
			   entity->Image()->X()+entity->Image()->GetWidth() >= 0 &&
			   entity->Image()->Y() <= Window()->GetHeight() &&
			   entity->Image()->Y()+entity->Image()->GetHeight() >= 0)
			{
				if(!entity->IsBuilding() && !entity->OnTop())
					entity->Draw();
				if(entity->Case() && entity->Image() && (entity->Image()->Anim() || entity->Image()->SpriteBase()->Alpha()))
					entity->Case()->SetMustRedraw();

			}
			++enti;
		}

	/* Troisième boucle pour maintenant dessiner les unités qui sont au dessus des autres (arbres, etc) */
	FORit(ECBEntity*, entities, enti)
	{
		ECEntity* e = dynamic_cast<ECEntity*>(*enti);
		if(e->OnTop())
		{
			if(e &&
			   e->Image()->X() <= Window()->GetWidth() &&
			   e->Image()->X()+e->Image()->GetWidth() >= 0 &&
			   e->Image()->Y() <= Window()->GetHeight() &&
			   e->Image()->Y()+e->Image()->GetHeight() >= 0)
			{
				e->Draw();
			}
		}
		e->AfterDraw();
	}

	/* On dessine les after_draw qui sont des images stockées par les unités pour les dessiner après */
	FOR(ECSprite*, after_draw, it)
		if(it)
			it->draw();

#if 0
	for(BCaseVector::iterator casi = cases.begin(); casi != cases.end(); ++casi)
		if(dynamic_cast<ECase*>(*casi)->Showed() == 0)
		{
			SDL_Rect dest;
			dest.x = dynamic_cast<ECase*>(*casi)->Image()->X();
			dest.y = dynamic_cast<ECase*>(*casi)->Image()->Y();
			SDL_BlitSurface(brouillard, NULL, Window(), &dest);
		}
#endif

	if(map->Channel())
	{
		BPlayerVector pls = map->Channel()->Players();

		/* Dessin des balises */
		FORit(ECBPlayer*, pls, pl)
		{
			std::vector<ECPlayer::BreakPoint> bps = dynamic_cast<ECPlayer*>(*pl)->BreakPoints();
			FORit(ECPlayer::BreakPoint, bps, bp)
				if(bp->sprite)
				{
					bp->sprite->set(bp->c->Image()->X() + CASE_WIDTH/2  - bp->sprite->GetWidth()/2,
					                bp->c->Image()->Y() + CASE_HEIGHT/2 - bp->sprite->GetHeight()/2);
					bp->sprite->draw();
					bp->text.Draw(bp->c->Image()->X() + CASE_WIDTH/2 - bp->text.GetWidth()/2,
					              bp->c->Image()->Y() + CASE_HEIGHT - bp->text.GetHeight());
					bp->c->SetMustRedraw();
				}
		}

		/* En mode PLAYING, on dessine toutes les fleches */
		if(map->Channel()->State() == EChannel::PLAYING)
			for(std::vector<ECBEntity*>::iterator enti = entities.begin(); enti != entities.end(); ++enti)
			{
				ECEntity* entity = dynamic_cast<ECEntity*>(*enti);
				if(!entity->Move()->Empty())
				{
					ECase* c = dynamic_cast<ECase*>((*enti)->Case());
					if(c != entity->Move()->FirstCase())
						FDebug(W_WARNING|W_SEND, "La case de l'entité et le départ du mouvement ne sont pas identiques");

					ECMove::Vector moves = entity->Move()->Moves();
					ECase* next_c = 0;
					int last_move = -1;
					for(ECMove::Vector::iterator move = moves.begin(); move != moves.end(); ++move)
					{
						switch(*move)
						{
							case ECMove::Up: next_c = dynamic_cast<ECase*>(c->MoveUp()); break;
							case ECMove::Down: next_c = dynamic_cast<ECase*>(c->MoveDown()); break;
							case ECMove::Left: next_c = dynamic_cast<ECase*>(c->MoveLeft()); break;
							case ECMove::Right: next_c = dynamic_cast<ECase*>(c->MoveRight()); break;
						}
						if(c->Y() == next_c->Y())
						{
							switch(last_move)
							{
								case ECMove::Up:
									(c->X() < next_c->X() ? Resources::FlecheGaucheBas()
														: Resources::FlecheDroiteBas())
														->Draw(c->Image()->X(), c->Image()->Y());
									break;
								case ECMove::Down:
									(c->X() < next_c->X() ? Resources::FlecheGaucheHaut()
														: Resources::FlecheDroiteHaut())
														->Draw(c->Image()->X(), c->Image()->Y());
									break;
								case ECMove::Left:
								case ECMove::Right:
								default:
									Resources::FlecheHoriz()->Draw(c->Image()->X(), c->Image()->Y());
									break;
							}
						}
						if(c->X() == next_c->X())
						{
							switch(last_move)
							{
								case ECMove::Left:
									(c->Y() < next_c->Y() ? Resources::FlecheGaucheBas()
														: Resources::FlecheGaucheHaut())
														->Draw(c->Image()->X(), c->Image()->Y());
									break;
								case ECMove::Right:
									(c->Y() < next_c->Y() ? Resources::FlecheDroiteBas()
														: Resources::FlecheDroiteHaut())
														->Draw(c->Image()->X(), c->Image()->Y());
									break;
								case ECMove::Up:
								case ECMove::Down:
								default:
									Resources::FlecheVert()->Draw(c->Image()->X(), c->Image()->Y());
									break;
							}
						}
						c->SetMustRedraw();
						last_move = *move;
						c = next_c;
					}
					switch(last_move)
					{
						case ECMove::Right:
							(entity->EventType() == ARM_ATTAQ && c == entity->Move()->Dest() ? Resources::FlecheAttaqDroite()
									: Resources::FlecheVersDroite())->Draw(c->Image()->X(), c->Image()->Y());
							break;
						case ECMove::Left:
							(entity->EventType() == ARM_ATTAQ && c == entity->Move()->Dest() ? Resources::FlecheAttaqGauche()
									: Resources::FlecheVersGauche())->Draw(c->Image()->X(), c->Image()->Y());
							break;
						case ECMove::Up:
							(entity->EventType() == ARM_ATTAQ && c == entity->Move()->Dest() ? Resources::FlecheAttaqHaut()
									: Resources::FlecheVersHaut())->Draw(c->Image()->X(), c->Image()->Y());
							break;
						case ECMove::Down:
							(entity->EventType() == ARM_ATTAQ && c == entity->Move()->Dest() ? Resources::FlecheAttaqBas()
									: Resources::FlecheVersBas())->Draw(c->Image()->X(), c->Image()->Y());
							break;
					}
					c->SetMustRedraw();

				}
			}
	}

	if(move_map)
	{
		ECImage* surf = Resources::MoveMapPoint();
		surf->Draw(move_point.x - surf->GetWidth()/2, move_point.y - surf->GetHeight()/2);
	}
	SetMustRedraw(false);
}
