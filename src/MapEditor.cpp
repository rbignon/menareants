/* src/MapEditor.cpp - This is a map editor
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
 *
 * $Id$
 */

#include "Main.h"
#include "Config.h"
#include "Resources.h"
#include "Map.h"
#include "Outils.h"
#include "Defines.h"
#include "gui/MessageBox.h"
#include "gui/ColorEdit.h" // Pour color_eq[]
#include "gui/CheckBox.h"
#include "Debug.h"
#include "tools/Maths.h"
#include "tools/Video.h"
#include "tools/Font.h"
#include "MapEditor.h"
#include "Units.h"
#include "Batiments.h"
#include "InGame.h"
#include <fstream>

/********************************************************************************************
 *                                          EMap                                            *
 ********************************************************************************************/

template<typename T>
static ECEntity* CreateEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb, ECBMap* map)
{
	T* t = new T(_name, _owner, _case);
	t->SetNb(_nb);
	t->SetMap(map);
	t->Init();
	return t;
}

static struct
{
	ECEntity* (*create) (const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb, ECBMap* map);
} entities_type[] = {
#include "lib/UnitsList.h"
};

void EMap::SeeMapLine(std::string line)
{
	if(line[0] == '_')
		other_lines.push_back(line);
}

void EMap::VirtualAddUnit(std::string line)
{
	std::string type = stringtok(line, " ");
	std::string owner = stringtok(line, " ");
	std::string acaca = stringtok(line, " ");
	uint x, y;
	x = StrToTyp<uint>(stringtok(acaca, ","));
	y = StrToTyp<uint>(acaca);
	std::string number = line;
	if(type.empty() || owner.empty() || acaca.empty() || number.empty())
		vDebug(W_ERR, "La d�claration d'une unit� sur la map est invalide.",
						VName(type) VName(owner) VName(acaca) VName(number));

	if (x >= Width() || y >= Height())
		throw ECExcept(VIName(Width()) VIName(Height()) VIName(x) VIName(y), "Access � un element hors du tableau");

	ECBPlayer* pl = 0;
	if(owner[0] != '*')
	{
		BMapPlayersVector::iterator it;
		for(it = map_players.begin(); it != map_players.end() && (*it)->ID() != owner[0]; ++it);

		if(it != map_players.end())
		{
			EMapPlayer* mp = dynamic_cast<EMapPlayer*>(*it);
			pl = dynamic_cast<ECBPlayer*>(mp);
		}
	}

	ECEntity* entity = entities_type[StrToTyp<uint>(type)].create ("**", pl, map[ y * Width() + x ], StrToTyp<uint>(number),
	                                                               this);


	AddAnEntity(entity);
}

void EMap::AddPlayer()
{
	char num = 'A';

	bool unchecked = false;
	do
	{
		BMapPlayersVector::iterator it;
		for(it = map_players.begin(); it != map_players.end() && (*it)->ID() != num; it++);

		if(it == map_players.end())
			unchecked = true;
		else
		{
			if(num < 'z') num++;
			else if(num >= 'z') break;
		}
	} while(!unchecked);

	if(!unchecked)
		throw ECExcept(TypToStr(num), "Impossible de trouver l'ID d'un player");

	map_players.push_back(CreateMapPlayer(num, map_players.size()));
}

ECBMapPlayer* EMap::GetPlayer(const char id)
{
	BMapPlayersVector::iterator it;
	for(it = map_players.begin(); it != map_players.end() && (*it)->ID() != id; ++it);
	if(it != map_players.end())
		return *it;

	return 0;
}

bool EMap::RemovePlayer(const char id)
{
	BMapPlayersVector::iterator it;
	for(it = map_players.begin(); it != map_players.end() && (*it)->ID() != id; ++it);
	if(it == map_players.end())
		return false;

	BCountriesVector cnts = (*it)->Countries();
	for(BCountriesVector::iterator cnt = cnts.begin(); cnt != cnts.end(); ++cnt)
		(*cnt)->SetOwner(0);

	std::vector<ECBEntity*> ents = dynamic_cast<EMapPlayer*>(*it)->Entities()->List();
	for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
		(*enti)->SetOwner(0);

	delete *it;
	map_players.erase(it);
	return true;
}

ECBCountry* EMap::GetCountry(const char* id)
{
	BCountriesVector::iterator it;
	for(it = map_countries.begin(); it != map_countries.end() && strcmp((*it)->ID(), id); ++it);
	if(it != map_countries.end())
		return *it;

	return 0;
}

bool EMap::RemoveCountry(const char* id)
{
	BCountriesVector::iterator it;
	for(it = map_countries.begin(); it != map_countries.end() && strcmp((*it)->ID(), id); ++it);
	if(it == map_countries.end())
		return false;

	if(!(*it)->Cases().empty())
		return false;

	if((*it)->Owner())
		(*it)->Owner()->RemoveCountry(*it);

	delete *it;
	map_countries.erase(it);
	return true;
}

void EMap::AddCountry(const char* id)
{
	map_countries.push_back(CreateCountry(this, id));
}

EMap::EMap(std::string _filename, uint _x, uint _y, std::string d)
	: ECMap(std::vector<std::string>(0)), can_save(false)
{
	chan = 0;
	min = 0;
	x = _x;
	y = _y;
	max = 0;
	city_money = 0;
	filename = _filename;
	mission = false;

	date.SetDate(d);

	ECBCountry* country = CreateCountry(this, "AA");
	map_countries.push_back(country);

	for(_y = 0; _y < y; ++_y)
		for(_x = 0; _x < x; ++_x)
		{
			ECBCase* c = CreateCase(_x, _y, 't');
			SetCaseAttr(c, 't');
			c->SetCountry(country);
			country->AddCase(c);
			map.push_back(c);
		}

	map_infos.push_back("Map cr��e par " + Config::GetInstance()->nick);
	initialised = true;
}

void EMap::Save()
{
	std::vector<std::string> file;
	ECMap::Save(file);

	std::ofstream fp(filename.c_str());

	FORit(std::string, file, it)
		fp << *it << std::endl;
}

/********************************************************************************************
 *                                  TMapEditor                                              *
 ********************************************************************************************/

bool TMapEditor::Editor(const char *path, TForm* form)
{
	std::string name;
	bool create = false;

	EMap* map = NULL;

	if(!path)
	{
		create = true;
		{
			TMessageBox mb("Entrez le nom du fichier de la carte",
							HAVE_EDIT|BT_OK|BT_CANCEL, form);
			mb.Edit()->SetAvailChars(MAPFILE_CHARS);
			mb.Edit()->SetMaxLen(20);
			if(mb.Show() == BT_OK)
				name = mb.EditText();
			if(name.empty()) return true;
		}

		uint x = 0, y = 0;
		std::string date;
		name = MenAreAntsApp::GetInstance()->GetPath() + name + ".map";

		{
			TMessageBox mb("Combien de cases en abscisse\n(horizontales) ?", HAVE_EDIT|BT_OK|BT_CANCEL, form);
			mb.Edit()->SetAvailChars("0123456789");
			mb.Edit()->SetMaxLen(2);
			if(mb.Show() == BT_OK)
				x = StrToTyp<uint>(mb.EditText());
			else return true;
		}
		{
			TMessageBox mb("Combien de cases en ordonn�e\n(vertical) ?", HAVE_EDIT|BT_OK|BT_CANCEL, form);
			mb.Edit()->SetAvailChars("0123456789");
			mb.Edit()->SetMaxLen(2);
			if(mb.Show() == BT_OK)
				y = StrToTyp<uint>(mb.EditText());
			else return true;
		}
		{
			TMessageBox mb("Quelle est la date en d�but de partie ?\n('xx xx xxxx' ou 'xx/xx/xxxx')",
			               HAVE_EDIT|BT_OK|BT_CANCEL, form);
			mb.Edit()->SetAvailChars("0123456789/ ");
			mb.Edit()->SetMaxLen(10);
			if(mb.Show() == BT_OK)
				date = mb.EditText();
			else return true;
		}
		try
		{
			map = new EMap(name, x, y, date);
		}
		catch(const TECExcept &e)
		{
			vDebug(W_ERR, e.Message(), e.Vars());
			MyFree(map);
			return false;
		}
	}
	else
	{
		name = path;

		try
		{
			TMessageBox("Chargement de la map en cours...", 0, form).Draw();
			map = new EMap(path);
			map->Init();
			map->SetCanSave();
		}
		catch(const TECExcept &e)
		{
			vDebug(W_ERR, e.Message(), e.Vars());
			MyFree(map);
			return false;
		}

	}

	BMapPlayersVector mps = map->MapPlayers();
	for(BMapPlayersVector::iterator it = mps.begin(); it != mps.end(); ++it)
	{
		EMapPlayer *mp = dynamic_cast<EMapPlayer*>(*it);
		mp->SetPlayer(mp);
		mp->SetMapPlayer(mp);
	}

	TMapEditor* MapEditor = 0;
	try
	{
		MapEditor = new TMapEditor(Video::GetInstance()->Window(), map);

		MapEditor->Run();
	}
	catch(const TECExcept &e)
	{
		MyFree(MapEditor);
		MyFree(map);
		throw;
	}

	MyFree(MapEditor);
	MyFree(map);

	return true;
}

void TMapEditor::BeforeDraw()
{
	BarreLat->ScreenPos->SetXY(
	         BarreLat->Radar->X() -
	         ((int)BarreLat->Radar->Width()/(int)map->Width() *
	         Map->X()/CASE_WIDTH),
	         BarreLat->Radar->Y() -
	         ((int)BarreLat->Radar->Height()/(int)map->Height() *
	         Map->Y()/CASE_HEIGHT));
	//Window()->Fill(0);
	SetMustRedraw();
}

void TMapEditor::OnKeyUp(SDL_keysym key)
{
	switch(key.sym)
	{
		case SDLK_DELETE:
		{
			ECEntity* entity = 0;
			if((entity = BarreEntity->Entity()))
			{
				BarreEntity->UnSelect();
				map->RemoveAnEntity(entity);
				map->CreatePreview(120,120,P_FRONTMER|P_ENTITIES);
				break;
			}
			break;
		}
		default: break;
	}
}

void TMapEditor::OnClic(const Point2i& mouse, int button, bool&)
{
	if(Map->CreateEntity())
	{
		ECase *acase = 0;
		ECEntity* entity = Map->CreateEntity();

		if(button == MBUTTON_RIGHT)
			Map->SetCreateEntity(0);
		else if(!BarreLat->Test(mouse, button) &&
		        !BarreEntity->Test(mouse, button) &&
		        !BarreCase->Test(mouse, button) &&
		        (acase = Map->TestCase(mouse)))
		{
			ECEntity* et = entities_type[entity->Type()].create ("**", 0, acase, entity->InitNb(), map);
			map->AddAnEntity(et);

			BarreCase->UnSelect();
			Map->SetCreateEntity(0);
			BarreEntity->SetEntity(et);
			Map->SetPosition(Map->X(), Map->Y(), true);
			map->CreatePreview(120,120,P_FRONTMER|P_ENTITIES);
		}
		return;
	}
	if(BarreLat->SaveButton->Test(mouse, button))
	{
		if(!map->CanSave())
		{
			TMessageBox("Vous ne pouvez pas sauvegarder tant que vous n'avez pas configur� la carte.",
					BT_OK, this).Show();
			return;
		}
		try
		{
			map->Save();
			TMessageBox("Sauvegarde effectu�e", BT_OK, this).Show();
		}
		catch(const TECExcept &e)
		{
			TMessageBox(std::string("Impossible de sauvegarder la carte :\n\n") + e.Message(),
					BT_OK, this).Show();
		}
	}
	if(BarreLat->QuitButton->Test(mouse, button))
	{
		uint result = 0;
		if(map->CanSave())
		{
			TMessageBox mb("Voulez-vous sauvegarder la carte ?", BT_YES|BT_NO|BT_CANCEL, this);
			result = mb.Show();
		}
		else
		{
			TMessageBox mb("Vous n'avez pas configur� la carte, vous ne pouvez donc pas la sauvegarder.\n"
					"Voulez-vous quand m�me fermer sans sauvegarder ?", BT_OK|BT_CANCEL,
					this);
			result = mb.Show();
		}
		if(result != BT_CANCEL)
		{
			try
			{
				if(result == BT_YES)
					map->Save();
				want_quit = true;
			}
			catch(const TECExcept &e)
			{
				TMessageBox(std::string("Impossible de sauvegarder la carte :\n\n") +
						e.Message(), BT_OK, this).Show();
			}
		}
		return;
	}
	ECEntity* entity = 0;

	if((entity = BarreEntity->Entity()) &&
	   BarreEntity->RemoveButton->Test(mouse, button))
	{
		BarreEntity->UnSelect();
		map->RemoveAnEntity(entity);
		map->CreatePreview(120,120,P_FRONTMER|P_ENTITIES);
		return;
	}

	if(BarreEntity->Test(mouse, button) ||
	   BarreCase->Test(mouse, button) ||
	   BarreLat->Test(mouse, button))
		return;

	if(button == MBUTTON_LEFT)
	{
		ECase* c = 0;
		if(!IsPressed(SDLK_LCTRL) && !IsPressed(SDLK_LSHIFT) &&
		   (entity = Map->TestEntity(mouse)))
		{
			BarreCase->UnSelect();
			BarreEntity->SetEntity(entity);
		}
		else if((c = Map->TestCase(mouse)))
		{
			BarreEntity->UnSelect();
			if(!BarreCase->RemoveCase(c, false) ||
			   BarreCase->Cases().size() > 1 && !IsPressed(SDLK_LCTRL))
			{
				if(IsPressed(SDLK_LSHIFT) && !BarreCase->Empty())
				{
					ECase* first = BarreCase->Cases().front();
					ECBCase* next = first;
					uint t_x = abs(first->X() - c->X());
					uint t_y = abs(first->Y() - c->Y());
					BarreCase->UnSelect(false);
					BarreCase->AddCase(first, false);
					if(c->Y() < first->Y())
						next = next->MoveUp(t_y);
					if(c->X() < first->X())
						next = next->MoveLeft(t_x);

					for(uint i=0; i <= t_y; ++i)
					{
						uint j=0;
						for(; j <= t_x; ++j)
						{
							if(next != first)
								BarreCase->AddCase(dynamic_cast<ECase*>(next), false);
							if(next->X() == next->Map()->Width()-1)
								break;
							next = next->MoveRight();
						}
						if(next->Y() == next->Map()->Height()-1)
							break;
						next = next->MoveDown();
						next = next->MoveLeft(j);
					}
					BarreCase->Update(c);
				}
				else
				{
					if(!IsPressed(SDLK_LCTRL))
						BarreCase->UnSelect(false);
					BarreCase->AddCase(c);
				}
			}
			else if(BarreCase->Empty())
				BarreCase->UnSelect();
		}
	}
	else if(button == MBUTTON_RIGHT)
	{
		BarreEntity->UnSelect();
		BarreCase->UnSelect();
	}
}

void TMapEditor::ShowBarreAct(bool show, ECase* c)
{
	assert(Map && (BarreEntity || BarreCase));

	int h = BarreEntity ? BarreEntity->Height() : BarreCase->Height();

	if(show)
		Map->SetContraintes(Map->Xmin(), int(SCREEN_HEIGHT - Map->Height()) - h);
	else
		Map->SetContraintes(Map->Xmin(), SCREEN_HEIGHT - int(Map->Height()));

	Map->SetXY(Map->X(),
	           (c && (c->Image()->Y() + c->Image()->GetHeight()) >= (SCREEN_HEIGHT - h)) ?
	               Map->Y() - c->Image()->GetHeight()
	             : Map->Y());
}

void TMapEditor::ShowBarreLat(bool show)
{
	assert(Map && BarreLat);

	if(show)
		Map->SetContraintes(int(SCREEN_WIDTH - Map->Width()) - int(BarreLat->Width()), Map->Ymin());
	else
		Map->SetContraintes(SCREEN_WIDTH - int(Map->Width()), Map->Ymin());
	Map->SetXY(Map->X(), Map->Y());
}

TMapEditor::TMapEditor(ECImage* w, EMap *m)
	: TForm(w), map(m)
{
	Map = AddComponent(new TMap(m));
	m->SetShowMap(Map);
	Map->SetBrouillard(false);
	Map->SetContraintes(SCREEN_WIDTH - int(Map->Width()), SCREEN_HEIGHT - int(Map->Height()));

	BarreEntity = AddComponent(new TBarreEntity);
	BarreEntity->Hide();
	BarreCase = AddComponent(new TBarreCase);
	BarreCase->Hide();
	BarreLat = AddComponent(new TEditBarreLat);

	ShowBarreLat();
	SetFocusOrder(false);
}

/********************************************************************************************
 *                                TBarreCaseIcons                                           *
 ********************************************************************************************/

void TBarreCaseIcons::Init()
{
	TChildForm* form = dynamic_cast<TChildForm*>(Parent());

	if(!Last)
		Last = AddComponent(new TButton (form->Width()-X()-10, 5 ,5,10));
	else
		Last->SetX(X() + form->Width()-X()-10);

	if(!Next)
		Next = AddComponent(new TButton (form->Width()-X()-10, 40,5,10));
	else
		Next->SetX(X() + form->Width()-X()-10);

	Last->SetImage (new ECSprite(Resources::UpButton(), Window()));
	Next->SetImage (new ECSprite(Resources::DownButton(), Window()));

	Next->SetOnClick(TBarreCaseIcons::GoNext, 0);
	Last->SetOnClick(TBarreCaseIcons::GoLast, 0);
}

void TBarreCaseIcons::SelectCase(TObject* o, void* e)
{
	assert(o);
	assert(o->Parent());
	assert(o->Parent()->Parent());
	assert(o->Parent()->Parent()->Parent());

	/*  TBarreCase             Parent()
	 *  `- TBarreLatIcons      Parent()
	 *     `- TImage           o
	 */
	TBarreCase *bc = dynamic_cast<TBarreCase*>(o->Parent()->Parent());
	TMapEditor *me = dynamic_cast<TMapEditor*>(bc->Parent());

	case_img_t* ci = static_cast<case_img_t*>(e);
	std::vector<ECase*> cases = bc->Cases();
	for(std::vector<ECase*>::iterator it = cases.begin(); it != cases.end(); ++it)
	{
		int x = (*it)->X(), y = (*it)->Y();
		std::vector<ECBEntity*> entities = (*it)->Entities()->List();
		ECBCountry* country = (*it)->Country();
		country->RemoveCase(*it);
		delete *it;
		*it = dynamic_cast<ECase*>(me->Map->Map()->CreateCase(x,y, ci->type));
		me->Map->Map()->SetCaseAttr(*it, ci->c);
		(*it)->Image()->set(me->Map->X()+(CASE_WIDTH  * (*it)->X()),
		                    me->Map->Y()+(CASE_HEIGHT * (*it)->Y()));
		(*it)->SetCountry(country);
		country->AddCase(*it);
		for(std::vector<ECBEntity*>::iterator enti = entities.begin(); entities.end() != enti; ++enti)
			(*enti)->SetCase(*it);
		(*me->Map->Map())(x,y) = *it;
		(*it)->Select();
	}

	me->Map->Map()->CreatePreview(120,120,P_FRONTMER|P_ENTITIES);
}



/********************************************************************************************
 *                                TBarreCase                                                *
 ********************************************************************************************/

bool TBarreCase::RemoveCase(ECase* c, bool update)
{
	for(std::vector<ECase*>::iterator it = cases.begin(); it != cases.end(); ++it)
		if(*it == c)
		{
			cases.erase(it);
			c->Select(false);
			if(update)
				Update();
			return true;
		}
	return false;
}

void TBarreCase::UnSelect(bool update)
{
	if(!select) return;
	for(std::vector<ECase*>::iterator it = cases.begin(); it != cases.end(); ++it)
		(*it)->Select(false);
	cases.clear();

	if(update)
		Update();
}

void TBarreCase::ChangeOwner(TObject* o, void*)
{
	TBarreCase* bc = dynamic_cast<TBarreCase*>(o->Parent());

	if(bc->Country->Selected() < 0)
		return;

	const char* id =  bc->Country->SelectedItem()->Value().c_str();

	ECBCountry* country = 0;

	BCountriesVector cts = dynamic_cast<TMapEditor*>(bc->Parent())->Map->Map()->Countries();
	for(BCountriesVector::iterator it = cts.begin(); it != cts.end(); ++it)
		if(!strcmp((*it)->ID(), id))
		{
			country = *it;
			break;
		}

	if(!country)
	{
		Debug(W_WARNING, "La country %s n'a pas �t� trouv�e", id);
		return;
	}

	for(std::vector<ECase*>::iterator it = bc->cases.begin(); it != bc->cases.end(); ++it)
	{
		if((*it)->Country())
			(*it)->Country()->RemoveCase(*it);
		(*it)->SetCountry(country);
		country->AddCase(*it);
	}

	dynamic_cast<TMapEditor*>(bc->Parent())->Map->Map()->CreatePreview(120,120,P_FRONTMER|P_ENTITIES);
}

void TBarreCase::Update(ECase* c)
{
	if(!cases.empty())
	{
		select = true;

		ECBCountry* country = 0;
		for(std::vector<ECase*>::iterator it = cases.begin(); it != cases.end(); ++it)
			if(country && country != (*it)->Country())
			{
				country = 0;
				break;
			}
			else if(!country)
				country = (*it)->Country();

		Country->ClearItems();
		BCountriesVector cts = dynamic_cast<TMapEditor*>(Parent())->Map->Map()->Countries();
		for(BCountriesVector::reverse_iterator it = cts.rbegin(); it != cts.rend(); ++it)
		{
			TListBoxItem* i = Country->AddItem(country && *it == country, (*it)->ID(), (*it)->ID());
			if(*it == country) Country->ScrollTo(i);
		}

		Show();
		dynamic_cast<TMapEditor*>(Parent())->ShowBarreAct(true, c);
	}
	else
	{
		select = false;

		dynamic_cast<TMapEditor*>(Parent())->ShowBarreAct(false);

		Hide();
	}
}

TBarreCase::TBarreCase()
	: TChildForm(0, SCREEN_HEIGHT-110, SCREEN_WIDTH-200, 110)
{

}

void TBarreCase::Init()
{
	Icons = AddComponent(new TBarreCaseIcons(100,10));
	Icons->SetList();

	Country = AddComponent(new TListBox(Rectanglei(2, 15, 100, Height()-40)));
	Country->SetOnClick(TBarreCase::ChangeOwner, 0);

	SetBackground(Resources::BarreAct());
}

/********************************************************************************************
 *                                TBarreEntity                                              *
 ********************************************************************************************/

void TBarreEntity::UnSelect()
{
	if(!select) return;
	if(entity)
		SetEntity(0);
}

void TBarreEntity::SetEntity(ECEntity* e)
{
	if(entity)
	{
		if(Nb->Text().empty())
		{
			TMessageBox mb("Veuillez mettre un nombre dans l'unit�", BT_OK, dynamic_cast<TForm*>(Parent()));
			mb.Show();
			return;
		}
		if(Owner->Selected() < 0)
		{
			TMessageBox mb ("Veuillez s�lectionner un propri�taire", BT_OK, dynamic_cast<TForm*>(Parent()));
			mb.Show();
			return;
		}
		entity->SetNb(StrToTyp<uint>(Nb->Text()));

		bool dont_check = false;
		Color last_color = white_color;
		if(entity->Owner())
		{
			if(dynamic_cast<EMapPlayer*>(entity->Owner())->ID() == Owner->SelectedItem()->Value()[0])
				dont_check = true;
			else
			{
				entity->Owner()->Entities()->Remove(entity);
				last_color = color_eq[entity->Owner()->Color()];
			}
		}
		else
		{
			if(Owner->SelectedItem()->Value()[0] == '*')
				dont_check = true;
			else
				entity->Case()->Map()->Neutres()->Remove(entity);
		}

		if(!dont_check)
		{
			if(Owner->SelectedItem()->Value()[0] == '*')
			{
				entity->SetOwner(0);
				entity->Case()->Map()->Neutres()->Add(entity);
			}
			else
			{
				BMapPlayersVector mps = dynamic_cast<TMapEditor*>(Parent())->Map->Map()->MapPlayers();
				for(BMapPlayersVector::iterator it = mps.begin(); it != mps.end(); ++it)
					if((*it)->ID() == Owner->SelectedItem()->Value()[0])
					{
						entity->SetOwner(dynamic_cast<EMapPlayer*>(*it));
						dynamic_cast<EMapPlayer*>(*it)->Entities()->Add(entity);
						break;
					}
			}
			entity->RefreshColor(last_color);
			dynamic_cast<EMap*>(entity->Case()->Map())->CreatePreview(120,120,P_FRONTMER|P_ENTITIES);
		}
	}

	if(e)
	{
		select = true;
		Name->SetCaption(e->Name());
		Nb->SetString(TypToStr(e->Nb()));
		Icon->SetImage(e->Icon(), false);

		Owner->ClearItems();
		Owner->AddItem(!e->Owner(), "Neutre", "*");
		BMapPlayersVector mps = dynamic_cast<TMapEditor*>(Parent())->Map->Map()->MapPlayers();
		for(BMapPlayersVector::iterator it = mps.begin(); it != mps.end(); ++it)
			Owner->AddItem(e->Owner() == dynamic_cast<EMapPlayer*>(*it), TypToStr((*it)->ID()), TypToStr((*it)->ID()));

		if(entity)
			entity->Select(false);
		e->Select();

		if(!entity)
		{
			Show();
			dynamic_cast<TMapEditor*>(Parent())->ShowBarreAct(true);
		}
		entity = e;
	}
	else
	{
		select = false;
		if(entity)
			entity->Select(false);
		if(entity)
		{
			dynamic_cast<TMapEditor*>(Parent())->ShowBarreAct(false);

			Hide();

		}
		entity = 0;
	}
}

TBarreEntity::TBarreEntity()
	: TChildForm(0, SCREEN_HEIGHT-110, SCREEN_WIDTH-200, 110), entity(0)
{

}

void TBarreEntity::Init()
{
	Name = AddComponent(new TLabel(60,15, "", black_color, Font::GetInstance(Font::Big)));

	RemoveButton = AddComponent(new TButtonText(Width()-150,15,100,30, "Supprimer", Font::GetInstance(Font::Small)));
	RemoveButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));

	Owner = AddComponent(new TListBox(Rectanglei(RemoveButton->X()-105, 15, 100, Height()-40)));

	Icon = AddComponent(new TImage(5,15));

	Nb = AddComponent(new TEdit(Font::GetInstance(Font::Small), 5,65,100));
	Nb->SetAvailChars("0123456789");

	SetBackground(Resources::BarreAct());
}

/********************************************************************************************
 *                                TEditBarreLatIcons                                        *
 ********************************************************************************************/

void TEditBarreLat::SelectUnit(TObject* o, void* e)
{
	assert(o);
	assert(o->Parent());
	assert(o->Parent()->Parent());
	assert(o->Parent()->Parent()->Parent());

	/* TMapEditor                Parent()
	 * `- TEditBarreLat          Parent()
	 *    `- TBarreLatIcons      Parent()
	 *       `- TImage           o
	 */
	TMapEditor *editor = static_cast<TMapEditor*>(o->Parent()->Parent()->Parent());
	if(!editor)
		return;

	editor->Map->SetCreateEntity(static_cast<ECEntity*>(e));
}

void TEditBarreLat::RadarClick(TObject* m, const Point2i& mouse)
{
	TImage* map = dynamic_cast<TImage*>(m);
	TMapEditor* editor = dynamic_cast<TMapEditor*>(m->Parent()->Parent());

	if(!map)
		throw ECExcept(VPName(map), "Appel incorrect");

	int size_x = map->Width() / editor->Map->Map()->Width();
	int size_y = map->Height() / editor->Map->Map()->Height();
	int _x = BorneInt((mouse.X() - map->X()) / size_x, 0, editor->Map->Map()->Width()-1);
	int _y = BorneInt((mouse.Y() - map->Y()) / size_y, 0, editor->Map->Map()->Height()-1);

	if(editor->Map->Enabled())
		editor->Map->CenterTo(dynamic_cast<ECase*>((*editor->Map->Map())(_x,_y)));

	map->DelFocus();
}

TEditBarreLat::TEditBarreLat()
	: TChildForm(SCREEN_WIDTH-200, 0, 200, SCREEN_HEIGHT)
{

}

void TEditBarreLat::Init()
{
	QuitButton = AddComponent(new TButtonText(30,240,100,30, "Fermer", Font::GetInstance(Font::Small)));
	QuitButton->SetImage(new ECSprite(Resources::LitleButton(), Video::GetInstance()->Window()));
	QuitButton->SetX(X() + Width()/2 - QuitButton->Width()/2);
	OptionsButton = AddComponent(new TButtonText(30,270,100,30, "Configuration", Font::GetInstance(Font::Small)));
	OptionsButton->SetImage(new ECSprite(Resources::LitleButton(), Video::GetInstance()->Window()));
	OptionsButton->SetX(X() + Width()/2 - OptionsButton->Width()/2);
	SaveButton = AddComponent(new TButtonText(30,300,100,30, "Sauvegarder", Font::GetInstance(Font::Small)));
	SaveButton->SetImage(new ECSprite(Resources::LitleButton(), Video::GetInstance()->Window()));
	SaveButton->SetX(X() + Width()/2 - SaveButton->Width()/2);

	TMapEditor* editor = dynamic_cast<TMapEditor*>(Parent());

	OptionsButton->SetOnClick(TOptionsMap::Options, (void*)editor->Map->Map());

	Icons = AddComponent(new TBarreLatIcons(20, 340));
	Icons->SetMaxHeight(SCREEN_HEIGHT - Icons->Y());
	Icons->SetList(EntityList.List(), TEditBarreLat::SelectUnit);
	Icons->SetX(X() + Width()/2 - Icons->Width()/2);

	editor->Map->Map()->CreatePreview(120,120, P_FRONTMER|P_ENTITIES);
	int _x = 40 + 60 - editor->Map->Map()->Preview()->GetWidth() / 2 ;
	int _y = 55 + 60 - editor->Map->Map()->Preview()->GetHeight() / 2 ;
	Radar = AddComponent(new TImage(_x, _y, false));
	Radar->SetImage(editor->Map->Map()->Preview(), false);
	Radar->SetOnClickPos(TEditBarreLat::RadarClick);

	ScreenPos = AddComponent(new TImage(0,0));
	SDL_Surface *surf = SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, Width(), Height(),
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
	DrawRect(surf, 0, 0,
	          editor->Map->Map()->Preview()->GetWidth()/editor->Map->Map()->Width()  * (SCREEN_WIDTH-Width()) / CASE_WIDTH,
	          editor->Map->Map()->Preview()->GetHeight()/editor->Map->Map()->Height() * SCREEN_HEIGHT / CASE_HEIGHT,
	         SDL_MapRGB(surf->format, 0xff,0xfc,0x00));
	ScreenPos->SetImage(new ECImage(surf));
	ScreenPos->SetEnabled(false);

	SetBackground(Resources::BarreLat());
}

/********************************************************************************************
 *                                  TOptionsMap                                             *
 ********************************************************************************************/

void TOptionsMap::Options(TObject*, void* m)
{
	EMap* map = static_cast<EMap*>(m);
	TOptionsMap* OptionsMap = 0;
	try
	{
		OptionsMap = new TOptionsMap(Video::GetInstance()->Window(), map);
		OptionsMap->Refresh();

		/* Mettre les valeurs qui ne seront enregistr�es qu'� la fermeture */
		OptionsMap->Name->SetString(map->Name());
		OptionsMap->MinPlayers->SetValue(map->MinPlayers());
		OptionsMap->MaxPlayers->SetValue(map->MaxPlayers());
		OptionsMap->City->SetString(TypToStr(map->CityMoney()));

		OptionsMap->Run();
	}
	catch(TECExcept &e)
	{
		MyFree(OptionsMap);
		throw;
	}

	map->Name() = OptionsMap->Name->Text();
	map->MaxPlayers() = OptionsMap->MaxPlayers->Value();
	map->MinPlayers() = OptionsMap->MinPlayers->Value();
	map->CityMoney()  = StrToTyp<int>(OptionsMap->City->Text());
	map->IsMission()  = OptionsMap->Mission->Checked();

	MyFree(OptionsMap);
	map->CreatePreview(120,120, P_FRONTMER|P_ENTITIES);
	map->SetCanSave();
}

void TOptionsMap::OnClic(const Point2i& mouse, int button, bool&)
{
	if(OkButton->Test(mouse, button))
	{
		if(Name->Text().empty())
			TMessageBox("Veuillez donner un nom � la carte !", BT_OK, this).Show();
		else if(Players->Size() < 2)
			TMessageBox("Il doit y avoir au moins deux joueur", BT_OK, this).Show();
		else if(Countries->Empty())
			TMessageBox("Il doit y avoir au moins une country", BT_OK, this).Show();
		else if(City->Empty())
			TMessageBox("Vous devez mettre l'argent des villes",
					BT_OK, this).Show();
		else if(MinPlayers->Value() < 2)
			TMessageBox("Le nombre minimal de joueurs doit �tre d'au moins deux joueurs", BT_OK,
					this).Show();
		else if(MaxPlayers->Value() < MinPlayers->Value())
			TMessageBox("Le nombre maximal de joueurs doit �tre �gal ou sup�rieur au nombre de "
					"joueurs minimal", BT_OK, this).Show();
		else if(StrToTyp<int>(City->Text()) < 1)
			TMessageBox("Il faut qu'il y ait de l'argent pour chaque ville par tour",
					BT_OK, this).Show();
		else
		{
			want_quit = true;
			return;
		}
	}

	const char* id = 0;
	char m_id = 0;
	if(AddPlayerButton->Test(mouse, button))
	{
		map->AddPlayer();
		Refresh();
	}
	else if(Players->Selected() < 0 || !(m_id = Players->SelectedItem()->Value()[0]))
		DelPlayerButton->SetEnabled(false);
	else if(Players->Test(mouse, button))
		DelPlayerButton->SetEnabled(true);
	else if(DelPlayerButton->Test(mouse, button))
	{
		map->RemovePlayer(m_id);
		Refresh();
	}

	if(AddCountryButton->Test(mouse, button))
	{
		std::string c_id = AddCountryEdit->Text();
		if(c_id.empty() || c_id.size() != 2)
		{
			TMessageBox mb("L'identifiant doit faire 2 caract�res", BT_OK, this);
			mb.Show();
		}
		else if(map->GetCountry(c_id.c_str()))
		{
			TMessageBox mb("L'identifiant est d�j� utilis�", BT_OK, this);
			mb.Show();
		}
		else
		{
			map->AddCountry(c_id.c_str());
			AddCountryEdit->ClearString();
			Refresh();
		}
	}
	else if(Countries->Selected() < 0 || !(id = Countries->SelectedItem()->Value().c_str()))
	{
		CountryPlayer->SetEnabled(false);
		DelCountryButton->SetEnabled(false);
	}
	else if(Countries->Test(mouse, button))
	{
		ECBCountry* country = 0;

		BCountriesVector cts = map->Countries();
		for(BCountriesVector::iterator it = cts.begin(); it != cts.end(); ++it)
			if(!strcmp((*it)->ID(), id))
			{
				country = *it;
				break;
			}
		if(!country) return;

		DelCountryButton->SetEnabled(country->Cases().empty());

		CountryPlayer->SetEnabled();
		CountryPlayer->ClearItems();
		CountryPlayer->AddItem(true, "Neutre", "*");
		BMapPlayersVector mps = map->MapPlayers();
		for(BMapPlayersVector::iterator it = mps.begin(); it != mps.end(); ++it)
			CountryPlayer->AddItem(*it == country->Owner(), TypToStr((*it)->ID()),
			                       TypToStr((*it)->ID()));
	}
	else if(DelCountryButton->Test(mouse, button))
	{
		map->RemoveCountry(id);
		Refresh();
	}
	else if(CountryPlayer->Selected() != -1)
	{
		const char map_id = CountryPlayer->SelectedItem()->Value()[0];
		ECBMapPlayer* mp = map_id == '*' ? 0 : map->GetPlayer(map_id);
		if(!mp && map_id != '*') return;

		ECBCountry* country = map->GetCountry(id);
		if(!country) return;

		country->ChangeOwner(mp);
	}
}

void TOptionsMap::Refresh()
{
	Countries->ClearItems();
	BCountriesVector cts = map->Countries();
	for(BCountriesVector::reverse_iterator it = cts.rbegin(); it != cts.rend(); ++it)
		Countries->AddItem(false, (*it)->ID(), (*it)->ID(), (*it)->Cases().empty() ? green_color : black_color);

	Players->ClearItems();
	CountryPlayer->ClearItems();
	BMapPlayersVector mps = map->MapPlayers();
	for(BMapPlayersVector::iterator it = mps.begin(); it != mps.end(); ++it)
	{
		Players->AddItem(false, TypToStr((*it)->ID()), TypToStr((*it)->ID()));
		CountryPlayer->AddItem(false, TypToStr((*it)->ID()), TypToStr((*it)->ID()));
	}

	DelCountryButton->SetEnabled(false);
	DelPlayerButton->SetEnabled(false);

	MinPlayers->SetMax(map->MapPlayers().size());
	MaxPlayers->SetMax(map->MapPlayers().size());

	Mission->Check(map->IsMission());
}

TOptionsMap::TOptionsMap(ECImage* w, EMap* m)
	: TForm(w)
{
	map = m;

	OkButton = AddComponent(new TButtonText(600,500,150,50, "OK", Font::GetInstance(Font::Normal)));

	PlayersLabel = AddComponent(new TLabel(100, 130, "Joueurs :", white_color, Font::GetInstance(Font::Normal)));
	Players = AddComponent(new TListBox(Rectanglei(100,150,150,200)));
	Players->SetNoItemHint();
	Players->SetHint("Ce sont les diff�rents joueurs susceptibles d'�tre jou�s.\nChaque territoire et unit� peut �tre li� � "
	                 " un joueur.");

	AddPlayerButton = AddComponent(new TButtonText(255, 150, 100, 30, "Ajouter", Font::GetInstance(Font::Normal)));
	AddPlayerButton->SetImage(new ECSprite(Resources::LitleButton(), Video::GetInstance()->Window()));

	DelPlayerButton = AddComponent(new TButtonText(255, 180, 100, 30, "Supprimer", Font::GetInstance(Font::Normal)));
	DelPlayerButton->SetImage(new ECSprite(Resources::LitleButton(), Video::GetInstance()->Window()));

	CountriesLabel = AddComponent(new TLabel(400, 130, "Territoires :", white_color, Font::GetInstance(Font::Normal)));
	Countries = AddComponent(new TListBox(Rectanglei(400,150,150,200)));
	Countries->SetNoItemHint();
	Countries->SetHint("Liste des diff�rents territoires. Vous pouvez assigner chaque case � un territoire.\n"
	                   "Vous ne pouvez supprimer que les territoires en vert, qui ne sont assign�s � aucune case.");

	AddCountryEdit = AddComponent(new TEdit(Font::GetInstance(Font::Small), 400,360,150, 2, COUNTRY_CHARS));

	AddCountryButton = AddComponent(new TButtonText(555, 350, 100, 30, "Ajouter", Font::GetInstance(Font::Normal)));
	AddCountryButton->SetImage(new ECSprite(Resources::LitleButton(), Video::GetInstance()->Window()));

	DelCountryButton = AddComponent(new TButtonText(555, 320, 100, 30, "Supprimer", Font::GetInstance(Font::Normal)));
	DelCountryButton->SetImage(new ECSprite(Resources::LitleButton(), Video::GetInstance()->Window()));

	CountryPlayerLabel = AddComponent(new TLabel(560, 150, "Appartient � :", white_color, Font::GetInstance(Font::Small)));
	CountryPlayer = AddComponent(new TComboBox(Font::GetInstance(Font::Small), 560 + CountryPlayerLabel->Width(), 150, 70));
	CountryPlayer->SetEnabled(false);
	CountryPlayer->SetNoItemHint();
	CountryPlayer->SetHint("Propri�taire du territoire s�lectionn�.\n"
	                       "Un territoire neutre n'appartiendra � personne au d�but de la partie.");

	NameLabel = AddComponent(new TLabel(50,380, "Nom de la carte", white_color, Font::GetInstance(Font::Normal)));
	Name = AddComponent(new TEdit(Font::GetInstance(Font::Small), 50, 400, 150, 50, EDIT_CHARS));

	                                                                           // label    x  y  width min
	                                                                                        // max                step def
	MinPlayers = AddComponent(new TSpinEdit(Font::GetInstance(Font::Small),"Min players", 50,420,150,1,
	                                                                                      map->MapPlayers().size(),1,1));
	MaxPlayers = AddComponent(new TSpinEdit(Font::GetInstance(Font::Small),"Max players", 50, 440, 150, 1,
	                                                                                      map->MapPlayers().size(), 1, 1));

	CityLabel = AddComponent(new TLabel(50,460, "Argent par villes", white_color, Font::GetInstance(Font::Normal)));
	City = AddComponent(new TEdit(Font::GetInstance(Font::Small), 50, 480, 150, 5, "0123456789"));

	Mission = AddComponent(new TCheckBox(Font::GetInstance(Font::Normal), 50, 500, "Mission", white_color));
	Mission->SetHint("Si actif, cette carte est une mission et le joueur sera le premier dans la liste.\n"
	                 "Cet editeur ne permet pas de configurer les param�tres sp�ciaux des missions");

	Hints = AddComponent(new TMemo(Font::GetInstance(Font::Small), 300, 480, 290, 100));
	SetHint(Hints);

	SetBackground(Resources::Titlescreen());
}

/********************************************************************************************
 *                                  TLoadMapFile                                            *
 ********************************************************************************************/

void MenAreAntsApp::MapEditor()
{
	TLoadMapFile* LoadMapFile = 0;
	try
	{
		LoadMapFile = new TLoadMapFile(Video::GetInstance()->Window());
		LoadMapFile->SetMutex(mutex);

		LoadMapFile->Run();

	}
	catch(TECExcept &e)
	{
		MyFree(LoadMapFile);
		throw;
	}
	MyFree(LoadMapFile);
}

void TLoadMapFile::OnClic(const Point2i& mouse, int button, bool&)
{
	if(MapsList->Test(mouse, button))
	{
		if(MapsList->Selected() >= 0 && MapsList->SelectedItem()->Enabled())
			LoadButton->SetEnabled(true);
		else
			LoadButton->SetEnabled(false);
	}
	else if(NewButton->Test(mouse, button))
	{
		if(!TMapEditor::Editor(NULL, this))
		{
			TMessageBox mb("Impossible de cr�er la carte.\n"
					"Son nom est peut �tre d�j� utilis�, ou alors les informations "
					"que vous avez fournis sont invalides.",
					BT_OK, this);
			mb.Show();
		}
		Refresh();
	}
	else if(LoadButton->Test(mouse, button))
	{
		if(MapsList->Selected() >= 0 && !TMapEditor::Editor(MapsList->SelectedItem()->Value().c_str()))
		{
			TMessageBox mb("Impossible d'ouvrir la map " + MapsList->SelectedItem()->Value() +
			               ".\nVeuillez reessayer", BT_OK, this);
			mb.Show();
		}
		Refresh();
		LoadButton->SetEnabled(false);
	}
	else if(RetourButton->Test(mouse, button))
		want_quit = true;
}

void TLoadMapFile::Refresh()
{
	MapsList->ClearItems();

	std::vector<std::string> file_list = GetFileList(MenAreAntsApp::GetInstance()->GetPath(), "map");

	for(std::vector<std::string>::const_iterator it = file_list.begin(); it != file_list.end(); ++it)
		MapsList->AddItem(false, it->substr(0, it->size() - 4), MenAreAntsApp::GetInstance()->GetPath() + *it,
			                             black_color, true);
}

TLoadMapFile::TLoadMapFile(ECImage* w)
	: TForm(w)
{

	MapsList = AddComponent(new TListBox(Rectanglei(300,200,200,300)));
	Refresh();

	NewButton = AddComponent(new TButtonText(550,250,150,50, "Nouveau", Font::GetInstance(Font::Normal)));
	LoadButton = AddComponent(new TButtonText(550,300,150,50, "Charger", Font::GetInstance(Font::Normal)));
	LoadButton->SetEnabled(false);
	RetourButton = AddComponent(new TButtonText(550,350,150,50, "Retour", Font::GetInstance(Font::Normal)));

	SetBackground(Resources::Titlescreen());
}
