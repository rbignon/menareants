/* src/Map.cpp - Map classes
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
#include "Debug.h"
#include "Map.h"
#include "MapEditor.h"
#include "Outils.h"
#include "Resources.h"
#include "Sockets.h"
#include "Units.h"
#include "Sound.h"
#include "gui/ColorEdit.h"
#include "gui/ShowMap.h"
#include "tools/Video.h"
#include <fstream>

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
 *                                   ECMissile                                              *
 ********************************************************************************************/
void ECMissile::SetMissile(ECSpriteBase* c)
{
	if(missile)
		MyFree(missile);
	if(!Entity()->Case()) return;

	missile = new ECSprite(c, Entity()->Image()->Window());
	missile->SetAnim(true);
	if(Entity()->Map())
		Entity()->Map()->ShowMap()->ToRedraw(missile);
}

void ECMissile::SetXY(int x, int y)
{
	missile->set(x, y);
	Entity()->Map()->ShowMap()->ToRedraw(missile);
}

bool ECMissile::AttaqFirst(ECase* c, EC_Client* me)
{
	if(c == Entity()->Case()) return true;

	if(!missile)
	{
		me->LockScreen();
		SetMissile(missile_up);
		Entity()->Map()->ShowMap()->AddAfterDraw(Missile());
		me->UnlockScreen();
		if(Entity()->Case()->Visible())
		{
			dynamic_cast<ECMap*>(Entity()->Case()->Map())->ShowMap()->CenterTo(Entity());
			SetXY(Entity()->Image()->X(), Entity()->Image()->Y());
			SDL_Delay(200);
		}
		return false;
	}
	SetXY(Entity()->Image()->X(), missile->Y() - step);

	if((missile->Y() + (int)missile->GetHeight()) <= 0 || !Entity()->Case()->Visible())
	{
		me->LockScreen();
		if(c->Visible())
			dynamic_cast<ECMap*>(Entity()->Case()->Map())->ShowMap()->CenterTo(c);
		Entity()->Map()->ShowMap()->RemoveAfterDraw(Missile());
		SetMissile(missile_down);
		Entity()->Map()->ShowMap()->AddAfterDraw(Missile());
		SetXY(c->Image()->X(), 0 - missile->GetHeight());
		me->UnlockScreen();
		if(c->Visible())
			Resources::SoundMissile()->Play();
		return true;
	}
	SDL_Delay(20);
	return false;
}

bool ECMissile::AttaqSecond(ECase* c, EC_Client* me)
{
	if(c == Entity()->Case() || !missile) return true;

	SetXY(missile->X(), missile->Y() + step);
	if(missile->Y() >= c->Image()->Y() || !c->Visible())
	{
		me->LockScreen();
		Entity()->Map()->ShowMap()->RemoveAfterDraw(Missile());
		MyFree(missile);
		me->UnlockScreen();
		return true;
	}
	SDL_Delay(20);
	return false;
}

/********************************************************************************************
 *                                 ECEntity                                                 *
 ********************************************************************************************/

ECEntity::ECEntity()
	: Tag(0), image(0), attaq(0), life(0,0,CASE_WIDTH-4,10), selected(false), attaqued_case(0), max_nb(0)
{
	life.SetBackground(false);
}

ECEntity::ECEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case)
	: ECBEntity(_name, _owner, _case), Tag(0), image(0), attaq(0), life(0,0,CASE_WIDTH-4,10), selected(false),
	  attaqued_case(0), max_nb(0)
{
	life.SetBackground(false);
}

ECEntity::~ECEntity()
{
	delete image;
	delete attaq;
}

void ECEntity::Init()
{
	ECBEntity::Init();
	UpdateImages();
}

ECSpriteBase* ECEntity::DeadCase() const
{
	return 0;
}

std::string ECEntity::OwnerNick() const { return (Owner() ? Owner()->GetNick() : _("Neutral")); }

void ECEntity::Select(bool s)
{
	selected = s;
	Map()->ShowMap()->ToRedraw(this);
	if(AttaquedCase())
		Map()->ShowMap()->ToRedraw(Rectanglei(
		        AttaquedCase()->X() < Case()->X() ? AttaquedCase()->Image()->X()+CASE_WIDTH /2 : Image()->X()+CASE_WIDTH /2,
		        AttaquedCase()->Y() < Case()->Y() ? AttaquedCase()->Image()->Y()+CASE_HEIGHT/2 : Image()->Y()+CASE_HEIGHT/2,
		        trajectoire.GetWidth(), trajectoire.GetHeight()));
}

void ECEntity::SetAttaquedCase(ECase* c)
{
	attaqued_case = c;

	if(AttaquedCase() && AttaquedCase() != Case())
	{
		int dx = abs(c->Image()->X() - Image()->X() + 1);
		int dy = abs(c->Image()->Y() - Image()->Y() + 1);
		trajectoire.SetImage(SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, dx, dy,
											32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000));
		DrawLine(trajectoire.Img,        attaqued_case->X() < Case()->X() ? 0 : dx-1,
			                             attaqued_case->Y() < Case()->Y() ? 0 : dy-1,
			                             attaqued_case->X() < Case()->X() ? dx-1 : 0,
			                             attaqued_case->Y() < Case()->Y() ? dy-1 : 0,
			                             trajectoire.MapColor(red_color));
	}
}

void ECEntity::Played()
{
	ECBEntity::Played();
	SetAttaquedCase(0);
	Move()->Clear(Case());
}

bool ECEntity::Test(int souris_x, int souris_y)
{
	return (image && ((image->X() <= souris_x) && (souris_x < (int)(image->X()+image->GetWidth()))
	         && (image->Y() <= souris_y) && (souris_y < int(image->Y()+image->GetHeight()))));
}

bool ECEntity::IsHiddenOnCase()
{
	if(!Channel())
		return false;

	if((Channel()->CurrentEvent() & ARM_ATTAQ) && EventType() & ARM_ATTAQ)
		return false;

	if(Owner() && (Owner()->IsMe() || Owner()->IsAllie(Channel()->GetMe())))
		return false;

	std::vector<ECBEntity*> ents = Case()->Entities()->List();
	bool hidden = false;
	FOR(ECBEntity*, ents, enti)
		if(dynamic_cast<ECEntity*>(enti)->IsHiddenByMe(this))
			hidden = true;
		else if(enti->Owner() && (dynamic_cast<ECPlayer*>(enti->Owner())->IsMe() || enti->Owner()->IsAllie(Channel()->GetMe())))
			return false;

	return hidden;
}

void ECEntity::Draw()
{
	if(image && !Parent() && (!Map()->ShowMap()->HaveBrouillard() || Case()->Visible()))
	{
		if(!IsHiddenOnCase())
			image->draw();
	}
}

void ECEntity::AfterDraw()
{
	if(attaq)
		attaq->draw();

	if(!image || (Map()->ShowMap()->HaveBrouillard() && (!Case() || !Case()->Visible())))
		return;

	if(Selected())
	{
		Resources::Cadre()->Draw(image->X(),image->Y());
		if(Nb() && Map()->Channel()) /* Uniquement si sélectionnée et en jeu (non map editor) */
		{
			life.SetWindow(image->Window());
			life.SetXY(image->X()+2, image->Y()+2);
			life.InitVal(Nb(), 0, max_nb ? max_nb : InitNb());
			float f = float(Nb()) / float(max_nb ? max_nb : InitNb());
			if(f >= 0.5) life.SetValueColor(green_color);
			else if(f >= 0.3) life.SetValueColor(orange_color);
			else life.SetValueColor(red_color);
			life.Draw(Point2i());
		}

	}
	if(Map()->Channel() && Map()->Channel()->State() != EChannel::ANIMING && AttaquedCase() && (EventType() & ARM_ATTAQ))
		trajectoire.Draw(
		         AttaquedCase()->X() < Case()->X() ? AttaquedCase()->Image()->X()+CASE_WIDTH /2
		                                           : Image()->X()+CASE_WIDTH /2,
		         AttaquedCase()->Y() < Case()->Y() ? AttaquedCase()->Image()->Y()+CASE_HEIGHT/2
		                                           : Image()->Y()+CASE_HEIGHT/2);
}

bool ECEntity::CanWalkTo(ECase* c, bool &move, bool &invest)
{
	/* = -1 : on ne peut investir en cas de présence d'une unité enemie. N'arrive pas si c'est sur un conteneur
	 * =  0 : n'a trouvé aucune unité à investir ou me contenant, ni aucune unité enemie
	 * =  1 : peut investir
	 * =  2 : peut me contenir
	 */
	int can_invest = 0;
	std::vector<ECBEntity*> ents = c->Entities()->List();

	move = invest = false;

	FOR(ECBEntity*, ents, enti)
	{
		EContainer* container = 0;
		if((container = dynamic_cast<EContainer*>(enti)) && container->CanContain(this) && this->Owner() &&
		   this->Owner()->IsMe() && container->Owner() && dynamic_cast<ECPlayer*>(container->Owner())->IsMe())
			can_invest = 2;
		if(can_invest >= 0 && this->CanInvest(enti))
			can_invest = 1;
		if(!enti->Like(this) && enti->CanAttaq(this) && can_invest != 2)
		{
			/* chercher l'interet de ce truc là.
				* -> Je suppose que c'est pour dire que dans le cas où y a une unité enemie, on ne parle pas
				*    d'entrer dans le batiment mais de se battre, donc on montre pas encore la fleche quoi
				*/
			can_invest = -1;
		}
	}

	if(can_invest >= 2 || (can_invest >= 1 && this->CanWalkOn(c)))
		return (invest = true);
	else if(this->CanWalkOn(c))
		return (move = true);
	else
		return false;
}

void ECEntity::SetNb(uint n)
{
	ECBEntity::SetNb(n);
	if(max_nb < n) max_nb = n;
}

ECPlayer* ECEntity::Owner() const
{
	return dynamic_cast<ECPlayer*>(ECBEntity::Owner());
}

ECase* ECEntity::Case() const
{
	return dynamic_cast<ECase*>(ECBEntity::Case());
}

ECMap* ECEntity::Map() const
{
	return dynamic_cast<ECMap*>(ECBEntity::Map());
}

EChannel* ECEntity::Channel() const
{
	if(!Map()) return 0;
	if(!Map()->Channel()) return 0;

	return dynamic_cast<EChannel*>(Map()->Channel());
}

void ECEntity::ImageSetXY(int x, int y)
{
	if(!image)
		return;

	image->set(x,y);

	if(Map() && Map()->ShowMap())
		Map()->ShowMap()->ToRedraw(this);
}

void ECEntity::SetAttaqImg(ECSpriteBase* spr, int x, int y)
{
	bool anim = false;
	if(attaq)
	{
		anim = attaq->Anim();
		MyFree(attaq);
	}

	if(!spr) return;

	attaq = new ECSprite(spr, Video::GetInstance()->Window());
	attaq->SetAnim(anim);

	if(Map() && Map()->ShowMap())
	{
		attaq->set(x,y);
		Map()->ShowMap()->ToRedraw(attaq);
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
	if(Case())
		dynamic_cast<ECase*>(Case())->SetMustRedraw();

	if(!spr) return;
	image = new ECSprite(spr, Video::GetInstance()->Window());
	image->SetAnim(anim);
	if(Case() && Map() && Map()->ShowMap())
		ImageSetXY(Map()->ShowMap()->X() +(CASE_WIDTH  * Case()->X()),
		           Map()->ShowMap()->Y() + (CASE_HEIGHT * Case()->Y()));
}

void ECEntity::SetOwner(ECBPlayer* player)
{
	images.clear();

	ECBEntity::SetOwner(player);

	UpdateImages(); // Call the virtual method to update image list.
}

void ECEntity::PutImage(imgs_t i, ECSpriteBase* b)
{
	if (Owner())
	{
		if(Owner()->HasSprite(Type(), i))
			b = Owner()->GetSprite(Type(), i);
		else
		{
			b = new ECSpriteBase(b->path.c_str());
			Owner()->SetSprite(Type(), i, b);
		}
	}
	images.insert(ImgList::value_type(i, b));

	if(Owner() && Owner()->Color())
		images[i]->ChangeColor(white_color, color_eq[Owner()->Color()]);

	if(images.size() == 1) SetImage(b);
}

void ECEntity::ChangeCase(ECBCase* newcase)
{
	if(newcase == Case()) return;

	SetShowedCases(false);
	ECBEntity::ChangeCase(newcase);
	SetShowedCases(true);

	if(Case() && Map() && Map()->ShowMap())
		ImageSetXY(Map()->ShowMap()->X() +(CASE_WIDTH  * Case()->X()),
		           Map()->ShowMap()->Y() + (CASE_HEIGHT * Case()->Y()));
}

void ECEntity::SetShowedCases(bool show, bool forced)
{
	if(!Case() || !Owner() || !Visibility() ||
	   (!forced && (Parent() || (!dynamic_cast<ECPlayer*>(Owner())->IsMe() &&
	                             !Owner()->IsAllie(dynamic_cast<EChannel*>(Owner()->Channel())->GetMe())))))
		return;

	ECBCase* c = Case()->MoveLeft(Visibility());
	c = c->MoveUp(Visibility());

	for(uint i=0; i <= 2*Visibility(); ++i)
	{
		uint j=0;
		for(; j <= 2*Visibility(); ++j)
		{
			ECase* cc = dynamic_cast<ECase*>(c);
			if(show)
				cc->SetShowed((cc->Showed() < 0) ? 1 : (cc->Showed() + 1));
			else if(cc->Showed() > 0)
				cc->SetShowed(cc->Showed()-1);

			cc->SetMustRedraw();
			if(c->X() == Case()->X())
				j = Visibility();

			if(c->X() == c->Map()->Width()-1)
				break;
			c = c->MoveRight();
		}
		if(c->Y() == Case()->Y())
			i = Visibility();

		if(c->Y() == c->Map()->Height()-1)
			break;
		c = c->MoveDown();
		c = c->MoveLeft(j);
	}
}

/********************************************************************************************
 *                                ECase                                                     *
 ********************************************************************************************/

ECase::ECase(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id)
	: image(0), selected(0), showed(-1), must_redraw(true)
{

}

ECase::~ECase()
{
	delete image;
}

int ECase::Showed() const
{
	return showed;
}

bool ECase::Visible() const
{
	return (showed > 0 || !dynamic_cast<ECMap*>(Map())->Brouillard());
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
		if(Showed() == 0 && dynamic_cast<ECMap*>(Map())->ShowMap()->HaveBrouillard())
			image->First()->Shadow()->Draw(image->X(), image->Y());
		else
			image->draw();
		if(selected)
			Resources::Cadre()->Draw(image->X(),image->Y());
	}
}

void ECase::SetImage(ECSpriteBase* spr)
{
	if(image) delete image;
	SetMustRedraw();
	if(!spr)
		return;

	image = new ECSprite(spr, Video::GetInstance()->Window());

	if(dynamic_cast<ECMap*>(map)->ShowMap())
		image->set(dynamic_cast<ECMap*>(map)->ShowMap()->X() +(CASE_WIDTH   * X()),
		           dynamic_cast<ECMap*>(map)->ShowMap()->Y() + (CASE_HEIGHT * Y()));
	SetMustRedraw();
}

ECBContainer* ECase::GetContainer(ECEntity* entity)
{
	EContainer* container = NULL;

	std::vector<ECBEntity*> ents = this->Entities()->List();
	for(std::vector<ECBEntity*>::iterator it = ents.begin(); it != ents.end(); ++it)
		if(*it && !(*it)->Locked() && (container = dynamic_cast<EContainer*>(*it)) &&
		   container->CanContain(entity) && container->Owner() == entity->Owner() &&
		   container->Move()->Empty())
			break;
		else
			container = 0;
	return container;
}

/********************************************************************************************
 *                                ECMap                                                     *
 ********************************************************************************************/

ECMap::ECMap(std::vector<std::string> _map_file)
	: ECBMap(_map_file), showmap(0), brouillard(false), nb_days(0)
{

}

ECMap::ECMap(std::string filename)
	: ECBMap(filename), showmap(0), brouillard(false), nb_days(0)
{

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
	{ 'M', Resources::CaseMontain,         'M' }
};

ECase* TBarreCase::ChangeCaseType(ECase* c, case_img_t* type)
{
	TMapEditor *me = dynamic_cast<TMapEditor*>(Parent());

	int x = c->X(), y = c->Y();
	std::vector<ECBEntity*> entities = c->Entities()->List();
	ECBCountry* country = c->Country();

	country->RemoveCase(c);

	bool selected = c->Selected();
	delete c;
	c = dynamic_cast<ECase*>(me->Map->Map()->CreateCase(x,y, type->type));
	me->Map->Map()->SetCaseAttr(c, type->c);
	c->Select(selected);

	c->Image()->set(me->Map->X()+(CASE_WIDTH * c->X()), me->Map->Y()+(CASE_HEIGHT * c->Y()));
	c->SetCountry(country);
	country->AddCase(c);

	for(std::vector<ECBEntity*>::iterator enti = entities.begin(); entities.end() != enti; ++enti)
	{
		(*enti)->SetCase(c);
		c->Entities()->Add(*enti);
	}

	(*me->Map->Map())(x,y) = c;

	return c;
}

void TBarreCase::CheckAroundCase(ECase* c)
{
#ifdef DEBUG
#define INTELLIGENT_EDITOR
#endif
#ifdef INTELLIGENT_EDITOR
	if(c->Selected() && (c->ImgID() == 'm' || c->ImgID() == 't'))
	{
		ECBCase *acase = c->MoveUp()->MoveLeft();

		for(uint i=0; i <= 2; ++i)
		{
			uint j=0;
			for(; j <= 2; ++j)
			{
				if(!dynamic_cast<ECase*>(acase)->Selected())
				{
					/*
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
					*/
					char t = 0;

					if(c->ImgID() == 'm')
					{
						if(acase->MoveRight()->TypeID() == 't' && acase->MoveDown()->TypeID() == 't' && acase->MoveRight()->MoveDown()->TypeID() == 't')
							t = 'k';
						else if(acase->MoveLeft()->TypeID() == 't' && acase->MoveDown()->TypeID() == 't' && acase->MoveLeft()->MoveDown()->TypeID() == 't')
							t = 'l';
						else if(acase->MoveRight()->TypeID() == 't' && acase->MoveUp()->TypeID() == 't' && acase->MoveRight()->MoveUp()->TypeID() == 't')
							t = 'i';
						else if(acase->MoveLeft()->TypeID() == 't' && acase->MoveUp()->TypeID() == 't' && acase->MoveLeft()->MoveUp()->TypeID() == 't')
							t = 'j';
						else if(acase->MoveRight()->TypeID() == 't')
							t = 'd';
						else if(acase->MoveLeft()->TypeID() == 't')
							t = 'c';
						else if(acase->MoveUp()->TypeID() == 't')
							t = 'a';
						else if(acase->MoveUp()->TypeID() == 't')
							t = 'b';
						else if(acase->MoveUp()->MoveLeft()->TypeID() == 't')
							t = 'e';
						else if(acase->MoveUp()->MoveRight()->TypeID() == 't')
							t = 'f';
						else if(acase->MoveDown()->MoveLeft()->TypeID() == 't')
							t = 'h';
						else if(acase->MoveDown()->MoveRight()->TypeID() == 't')
							t = 'g';
					}

					if(t)
					{

						case_img_t* type;
						for(uint i = 0; i < ASIZE(case_img); ++i)
							if(case_img[i].c == t)
							{
								type = &case_img[i];
								break;
							}
						acase = ChangeCaseType(dynamic_cast<ECase*>(acase), type);
					}
				}

				if(c->X() == acase->X())
					j = 1;

				if(c->X() == acase->Map()->Width()-1)
					break;
				acase = acase->MoveRight();
			}
			if(acase->Y() == c->Y())
				i = 1;

			if(acase->Y() == acase->Map()->Height()-1)
				break;
			acase = acase->MoveDown();
			acase = acase->MoveLeft(j);
		}
	}
#endif
}

bool TBarreCaseIcons::Clic (const Point2i& mouse, int button)
{
	if(Mouse(mouse) == false) return false;

	if (Next->Visible() || Last->Visible())
	{
		if (button == SDL_BUTTON_WHEELDOWN || Next->Test(mouse))
		{
			// bottom button
			if(first < (ASIZE(case_img)-2)) first += 2;
			else if(first < (ASIZE(case_img)-1)) ++first;
			Init();
			return true;
		}


		if (button == SDL_BUTTON_WHEELUP || Last->Test(mouse))
		{
			// top button
			if(first > 1) first -= 2;
			else first = 0;
			Init();
			return true;
		}
	}

	return TChildForm::Clic(mouse, button);
}


void TBarreCaseIcons::SetList()
{
	Clear();
	Next = 0;
	Last = 0;
	Init();

	int _x = X(), _y = 0;
	int _h = 0;
	TBarreCase* parent = dynamic_cast<TBarreCase*>(Parent());
	bool up = true;
	for(uint _i = first; _i < ASIZE(case_img); ++_i, up = !up)
	{
		ECImage* img = new ECImage;
		img->NewSurface(Point2i(CASE_WIDTH, CASE_HEIGHT), case_img[_i].spr()->First()->Img->flags, false);
		img->Blit(case_img[_i].spr()->First());
		img->Zoom(double(50) / double(img->GetWidth()), double(50) / double(img->GetHeight()), true);

		TImage* i = AddComponent(new TImage(_x, _y, img));

		if(i->Height() > _h) _h = i->Height();
		i->SetOnClick(TBarreCaseIcons::SelectCase, &case_img[_i]);

		if(up)
			_y += _h + 1;
		else
		{
			_x += i->Width() + 1;
			_y = 0;
		}

		if(_x + (3 * i->Width()) + 10 >= parent->Width()) break;
	}
	SetWidth(parent->Width()-X());
	SetHeight(_h * 2 + 1);
}

void ECMap::SetCaseAttr(ECBCase* c, char id)
{
	if(!c) return;

	for(uint j=0; j < (sizeof case_img / sizeof *case_img); j++)
		if(case_img[j].c == id)
		{
			dynamic_cast<ECase*>(c)->SetImage(case_img[j].spr());
			c->SetImgID(id);
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
	{ 'm', CreateCase<ECMer>,      C_MER              },
	{ 't', CreateCase<ECTerre>,    C_TERRE            },
	{ 'p', CreateCase<ECPont>,     C_PONT             },
	{ 'M', CreateCase<ECMontain>,  C_MONTAIN          }
};

ECBCase* ECMap::CreateCase(uint _x, uint _y, char type_id)
{
	for(uint j=0; j < (sizeof case_type / sizeof *case_type); j++)
		if(case_type[j].c == type_id)
			return case_type[j].func (this, _x, _y, case_type[j].flags, case_type[j].c);

	return 0;
}

bool ECMap::CanSelect(ECBCase* c)
{
	std::vector<ECBEntity*> es = c->Entities()->List();
	bool continuer = false;
	FOR(ECBEntity*, es, enti)
	{
		ECEntity* entii = dynamic_cast<ECEntity*>(enti);
		if(entii->CanBeSelected() && !entii->IsHiddenOnCase())
		{
			continuer = true;
			break;
		}
	}
	return continuer;
}

/** \note Preview use two screen pixels for one pixel in map */
void ECMap::CreatePreview(uint width, uint height, int flags)
{
	if(!initialised) return;

	uint size_x = width/x, size_y = height/y;
	if(size_x > size_y) size_x = size_y;
	else size_y = size_x;
	pixel_size = size_y;

	SDL_Surface *surf = CreateRGBASurface(x*size_x, y*size_y, SDL_HWSURFACE|SDL_SRCALPHA);

	SLOCK(surf);
	/* Dessine la preview */
	uint xx = 0, yy = 0;
	for(uint _y = 0; _y < y; ++_y, yy+=size_y, xx=0)
		for(uint _x = 0; _x < x; ++_x, xx+=size_x)
		{
			ECBCase* cc = map[ _y * x + _x ];
			ECase *c = dynamic_cast<ECase*>(cc);
			if(!c || (Brouillard() && c->Showed() < 0)) continue;
			Color color;
			switch(c->TypeID())
			{
				case 'v':
				case 'V':
					color = red_color;
					break;
				case 't':
					color = (c->Country() && c->Country()->Owner()) ?
					          c->Country()->Owner()->Player() ?
					            color_eq[c->Country()->Owner()->Player()->Color()]
					          : color_eq[c->Country()->Owner()->ID()%COLOR_MAX]
					        : brown_color;
					break;
				case 'p':
					color = gray_color;
					break;
				case 'm':
					color = blue_color;
					break;
				case 'M':
					color = black_color;
					break;
			}

			Color marge_color =  white_color;
			const short MARGE_LEFT = 0x01;
			const short MARGE_RIGHT = 0x02;
			const short MARGE_TOP = 0x04;
			const short MARGE_BOTTOM = 0x08;
			unsigned short marge = 0;

			if(!(c->Flags() & C_MER) || (flags & P_FRONTMER))
			{
				if(_y > 0 && map[(_y-1) * x + _x]->Country() != c->Country() &&
				   ((flags & P_FRONTMER) || !(map[(_y-1) * x + _x]->Flags() & C_MER)))
					marge |= MARGE_TOP;
				if(_x > 0 && map[_y * x + _x-1]->Country() != c->Country() &&
				   ((flags & P_FRONTMER) || !(map[_y * x + _x-1]->Flags() & C_MER)))
					marge |= MARGE_LEFT;
				if(size_x > 5 && size_y > 5)
				{
					if(_y < (y-1) && map[(_y+1) * x + _x]->Country() != c->Country() &&
					   ((flags & P_FRONTMER) || !(map[(_y+1) * x+_x]->Flags() & C_MER)))
						marge |= MARGE_BOTTOM;
					if(_x < (x-1) && map[_y * x + _x+1]->Country() != c->Country() &&
					   ((flags & P_FRONTMER) || !(map[_y * x + _x+1]->Flags() & C_MER)))
						marge |= MARGE_RIGHT;
				}
			}

			for(uint _yy = yy; _yy < yy+size_y; _yy++)
				for(uint _xx = xx; _xx < xx+size_x; _xx++)
				{
					Color col = ((marge & MARGE_TOP && _yy == yy) ||
					             (marge & MARGE_LEFT && _xx == xx) ||
					             (marge & MARGE_BOTTOM && _yy == yy+size_y-1) ||
					             (marge & MARGE_RIGHT && _xx == xx+size_x-1)) ? marge_color : color;
					putpixel(surf, _xx, _yy, SDL_MapRGB(surf->format,
					         (Brouillard() && c->Showed()<=0) ? (col.GetRed()>60) ? col.GetRed() - 60 : 0 : col.GetRed(),
					         (Brouillard() && c->Showed()<=0) ? (col.GetGreen()>60) ? col.GetGreen() - 60 : 0 : col.GetGreen(),
					         (Brouillard() && c->Showed()<=0) ? (col.GetBlue()>60) ? col.GetBlue() - 60 : 0 : col.GetBlue()));
				}
		}
	/* Position des unités */
	if(flags & P_ENTITIES)
	{
		std::vector<ECBEntity*> ents = entities.List();
		for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
		{
			if(!(*enti)->Case() || (*enti)->Parent() ||
			   (Brouillard() && dynamic_cast<ECase*>((*enti)->Case())->Showed() <= 0) ||
			   dynamic_cast<ECEntity*>(*enti)->IsHiddenOnCase() ||
			   !dynamic_cast<ECEntity*>(*enti)->CanBeSelected())
				continue;
			Color col = (*enti)->Owner() ? color_eq[(*enti)->Owner()->Color()] : white_color;
			yy = (*enti)->Case()->Y() * size_y;
			xx = (*enti)->Case()->X() * size_x;
			for(uint _yy = yy; _yy < yy+size_y; _yy++)
				for(uint _xx = xx; _xx < xx+size_x; _xx++)
					putpixel(surf, _xx, _yy, SDL_MapRGB(surf->format,
					                                          (col.GetRed()>  100) ? col.GetRed() - 30 : col.GetRed() + 30,
					                                          (col.GetGreen()>100) ? col.GetGreen() - 30 : col.GetGreen() + 30,
					                                          (col.GetBlue()> 100) ? col.GetBlue() - 30 : col.GetBlue() + 30));
		}
	}

	SUNLOCK(surf);

	preview.SetImage(surf);
}
