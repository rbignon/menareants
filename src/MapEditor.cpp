/* src/MapEditor.cpp - This is a map editor
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

#if 0
#include <windows.h>
#endif
#include "Main.h"
#include "Resources.h"
#include "Map.h"
#include "Outils.h"
#include "Defines.h"
#include "gui/MessageBox.h"
#include "gui/ColorEdit.h" // Pour color_eq[]
#include "Debug.h"
#include "tools/Maths.h"
#include "MapEditor.h"
#include "Units.h"
#include "Batiments.h"
#include <dirent.h>
#include <fstream>

/********************************************************************************************
 *                                          EMap                                            *
 ********************************************************************************************/

template<typename T>
static ECEntity* CreateEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb)
{
	return new T(_name, _owner, _case, _nb);
}

static struct
{
	ECEntity* (*create) (const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb);
} entities_type[] = {
#include "lib/UnitsList.h"
};

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
		vDebug(W_ERR, "La déclaration d'une unité sur la map est invalide.",
						VName(type) VName(owner) VName(acaca) VName(number));

	if (x >= Width() || y >= Height())
		throw ECExcept(VIName(Width()) VIName(Height()) VIName(x) VIName(y), "Access à un element hors du tableau");

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

	ECEntity* entity = entities_type[StrToTyp<uint>(type)].create ("**", pl, map[ y * Width() + x ], StrToTyp<uint>(number));
	

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
	date = new ECDate(d);
	begin_money = 0;
	city_money = 0;
	filename = _filename;

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

	map_infos.push_back("Map créée par " + app.getconf()->nick);
	initialised = true;
}

void EMap::Save()
{
	if(map_players.empty())
		throw ECExcept("", "Veuillez créer des joueurs !");

	std::ofstream fp(filename.c_str());
	if (!fp)
		throw ECExcept(VName(filename), "Impossible de créer le fichier map.");

	fp << "# Veuillez ne pas éditer le fichier map à la main." << std::endl;
	fp << "# Utilisez plutôt l'éditeur de carte intégré au jeu." << std::endl;

	fp << "NAME " << name << std::endl;

	for(BMapPlayersVector::const_iterator it = map_players.begin(); it != map_players.end(); ++it)
		fp << "PLAYER " << (*it)->ID() << std::endl;

	fp << "X " << x << std::endl;
	fp << "Y " << y << std::endl;

	fp << "MAP" << std::endl;

	for(uint _y = 0; _y < y; ++_y)
	{
		for(uint _x = 0; _x < x; ++_x)
		{
			ECase *c = dynamic_cast<ECase*>(map[ _y * x + _x ]);
			if(!c)
				throw ECExcept(VPName(c), "Veuillez ne pas enregistrer une carte non achevée");
			fp << c->TypeID() << c->Country()->ID();
			fp << (c->Country()->Owner() ? c->Country()->Owner()->ID() : '*') << c->ImgID();
		}
		fp << std::endl;
	}

	fp << "EOM" << std::endl;
	fp << "BEGIN " << BeginMoney() << std::endl;
	fp << "CITY " << CityMoney() << std::endl;
	fp << "MIN " << MinPlayers() << std::endl;
	fp << "MAX " << MaxPlayers() << std::endl;
	fp << "DATE " << Date()->String() << std::endl;
	for(std::vector<std::string>:: const_iterator it = map_infos.begin(); it != map_infos.end(); ++it)
		fp << "INFO " << *it << std::endl;

	for(BMapPlayersVector::const_iterator it = map_players.begin(); it != map_players.end(); ++it)
	{
		std::vector<ECBEntity*> sts = dynamic_cast<EMapPlayer*>(*it)->Entities()->List();
		for(std::vector<ECBEntity*>::iterator st = sts.begin(); st != sts.end(); ++st)
			fp << "UNIT " << (*st)->Type() << " " << dynamic_cast<EMapPlayer*>((*st)->Owner())->ID()
			   << " " << (*st)->Case()->X() << "," << (*st)->Case()->Y()
			   << " " << (*st)->Nb()  << std::endl;
	}
	std::vector<ECBEntity*> n = neutres.List();
	for(std::vector<ECBEntity*>::iterator st = n.begin(); st != n.end(); ++st)
		fp << "UNIT " << (*st)->Type() << " " << "*"
		   << " " << (*st)->Case()->X() << "," << (*st)->Case()->Y()
		   << " " << (*st)->Nb()  << std::endl;
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
							HAVE_EDIT|BT_OK, form);
			mb.Edit()->SetAvailChars(MAPFILE_CHARS);
			mb.Edit()->SetMaxLen(20);
			if(mb.Show() == BT_OK)
				name = mb.EditText();
			if(name.empty()) return true;
		}

		uint x = 0, y = 0;
		std::string date;
		name = app.GetPath() + name + ".map";

		{
			TMessageBox mb("Combien de cases en abscisse\n(horizontales) ?", HAVE_EDIT|BT_OK, form);
			mb.Edit()->SetAvailChars("0123456789");
			mb.Edit()->SetMaxLen(2);
			if(mb.Show() == BT_OK)
				x = StrToTyp<uint>(mb.EditText());
		}
		{
			TMessageBox mb("Combien de cases en ordonnée\n(vertical) ?", HAVE_EDIT|BT_OK, form);
			mb.Edit()->SetAvailChars("0123456789");
			mb.Edit()->SetMaxLen(2);
			if(mb.Show() == BT_OK)
				y = StrToTyp<uint>(mb.EditText());
		}
		{
			TMessageBox mb("Quelle est la date en début de partie ?\n('xx xx xxxx' ou 'xx/xx/xxxx')", HAVE_EDIT|BT_OK, form);
			mb.Edit()->SetAvailChars("0123456789/ ");
			mb.Edit()->SetMaxLen(10);
			if(mb.Show() == BT_OK)
				date = mb.EditText();
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
		bool eob = false;
		bool ctrl = false;
		MapEditor = new TMapEditor(app.sdlwindow, map);

		SDL_Event event;
		do
		{
			while( SDL_PollEvent( &event) )
			{
				MapEditor->Actions(event);
				switch(event.type)
				{
					case SDL_KEYUP:
						switch (event.key.keysym.sym)
						{
							case SDLK_LCTRL:
								ctrl = false;
								break;
							default: break;
						}
						break;
					case SDL_KEYDOWN:
						switch(event.key.keysym.sym)
						{
							case SDLK_LCTRL:
								ctrl = true;
								break;
							default: break;
						}
						break;
					case SDL_MOUSEBUTTONDOWN:
					{
						if(MapEditor->Map->CreateEntity())
						{
							ECase *acase = 0;
							ECEntity* entity = MapEditor->Map->CreateEntity();

							if(event.button.button == MBUTTON_RIGHT)
								MapEditor->Map->SetCreateEntity(0);
							else if(!MapEditor->BarreLat->Test(event.button.x, event.button.y) &&
						            !MapEditor->BarreEntity->Test(event.button.x, event.button.y) &&
						            !MapEditor->BarreCase->Test(event.button.x, event.button.y) &&
						            (acase = MapEditor->Map->TestCase(event.button.x, event.button.y)))
							{
								ECEntity* et = entities_type[entity->Type()].create ("**", 0, acase, entity->InitNb());
								map->AddAnEntity(et);

								MapEditor->BarreCase->UnSelect();
								MapEditor->Map->SetCreateEntity(0);
								MapEditor->BarreEntity->SetEntity(et);
							}
							break;
						}
						if(MapEditor->BarreLat->SaveButton->Test(event.button.x, event.button.y))
						{
							if(!map->CanSave())
							{
								TMessageBox("Vous ne pouvez pas sauvegarder tant que vous n'avez pas configuré la carte.",
								              BT_OK, MapEditor).Show();
								break;
							}
							try
							{
								map->Save();
								TMessageBox("Sauvegarde effectuée", BT_OK, MapEditor).Show();
							}
							catch(const TECExcept &e)
							{
								TMessageBox m((std::string("Impossible de sauvegarder la carte :\n\n") +
								               e.Message()).c_str(), BT_OK, MapEditor);
								m.Show();
							}
						}
						if(MapEditor->BarreLat->QuitButton->Test(event.button.x, event.button.y))
						{
							uint result = 0;
							if(map->CanSave())
							{
								TMessageBox mb("Voulez-vous sauvegarder la carte ?", BT_YES|BT_NO|BT_CANCEL, MapEditor);
								result = mb.Show();
							}
							else
							{
								TMessageBox mb("Vous n'avez pas configuré la carte, vous ne pouvez donc pas la sauvegarder.\n"
								               "Voulez-vous quand même fermer sans sauvegarder ?", BT_OK|BT_CANCEL, MapEditor);
								result = mb.Show();
							}
							if(result != BT_CANCEL)
							{
								try
								{
									if(result == BT_YES)
										map->Save();
									eob = true;
								}
								catch(const TECExcept &e)
								{
									TMessageBox m((std::string("Impossible de sauvegarder la carte :\n\n") +
									               e.Message()).c_str(), BT_OK, MapEditor);
									m.Show();
								}
							}
							break;
						}
						ECEntity* entity = 0;

						if((entity = MapEditor->BarreEntity->Entity()) &&
						   MapEditor->BarreEntity->RemoveButton->Test(event.button.x, event.button.y))
						{
							MapEditor->BarreEntity->UnSelect();
							map->RemoveAnEntity(entity);
							map->CreatePreview(120,120,true);
							break;
						}

						if(MapEditor->BarreEntity->Test(event.button.x, event.button.y) ||
						   MapEditor->BarreCase->Test(event.button.x, event.button.y) ||
						   MapEditor->BarreLat->Test(event.button.x, event.button.y))
							break;

						if(event.button.button == MBUTTON_LEFT)
						{
							ECase* c = 0;
							if(!ctrl && (entity = MapEditor->Map->TestEntity(event.button.x, event.button.y)))
							{
								MapEditor->BarreCase->UnSelect();
								MapEditor->BarreEntity->SetEntity(entity);
							}
							else if((c = MapEditor->Map->TestCase(event.button.x, event.button.y)))
							{
								MapEditor->BarreEntity->UnSelect();
								if(!MapEditor->BarreCase->RemoveCase(c, false))
								{
									if(!ctrl)
										MapEditor->BarreCase->UnSelect(false);
									MapEditor->BarreCase->AddCase(c);
								}
							}
						}
						else if(event.button.button == MBUTTON_RIGHT)
						{
							MapEditor->BarreEntity->UnSelect();
							MapEditor->BarreCase->UnSelect();
						}
					}
					default: break;
				}
			}
			MapEditor->BarreLat->ScreenPos->SetXY(
			            MapEditor->BarreLat->Radar->X() -
			              ((int)MapEditor->BarreLat->Radar->Width()/(int)map->Width() *
			               MapEditor->Map->X()/CASE_WIDTH),
			            MapEditor->BarreLat->Radar->Y() -
			              ((int)MapEditor->BarreLat->Radar->Height()/(int)map->Height() *
			              MapEditor->Map->Y()/CASE_HEIGHT));
			SDL_FillRect(app.sdlwindow, NULL, 0);
			MapEditor->Update();
		} while(!eob);
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

TMapEditor::TMapEditor(SDL_Surface* w, ECMap *m)
	: TForm(w)
{
	Map = AddComponent(new TMap(m));
	m->SetShowMap(Map);
	Map->SetBrouillard(false);
	Map->SetContraintes(SCREEN_WIDTH - int(Map->Width()), SCREEN_HEIGHT - int(Map->Height()));

	BarreLat = AddComponent(new TEditBarreLat);
	BarreEntity = AddComponent(new TBarreEntity);
	BarreEntity->Hide();
	BarreCase = AddComponent(new TBarreCase);
	BarreCase->Hide();

	ShowBarreLat();
	SetFocusOrder(false);
}

TMapEditor::~TMapEditor()
{
	delete BarreCase;
	delete BarreEntity;
	delete BarreLat;
	delete Map;
}

/********************************************************************************************
 *                                TBarreCaseIcons                                        *
 ********************************************************************************************/

void TBarreCaseIcons::Init()
{
  TChildForm* form = dynamic_cast<TChildForm*>(Parent());

  Last = AddComponent(new TButton (form->Width()-X()-10, 5 ,5,10));
  Next = AddComponent(new TButton (form->Width()-X()-10, 40,5,10));

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

	me->Map->Map()->CreatePreview(120,120,true);
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

	if(bc->Country->GetSelectedItem() < 0)
	{
		Debug(W_DEBUG, "Aucun item sélectionné");
		return;
	}

	const char* id =  bc->Country->ReadValue(bc->Country->GetSelectedItem()).c_str();

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
		Debug(W_WARNING, "La country %s n'a pas été trouvée", id);
		return;
	}

	for(std::vector<ECase*>::iterator it = bc->cases.begin(); it != bc->cases.end(); ++it)
		(*it)->SetCountry(country);

	dynamic_cast<TMapEditor*>(bc->Parent())->Map->Map()->CreatePreview(120,120,true);
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
		for(BCountriesVector::iterator it = cts.begin(); it != cts.end(); ++it)
		{
			uint i = Country->AddItem(country && *it == country, (*it)->ID(), (*it)->ID());
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
	: TChildForm(0, SCREEN_HEIGHT-100, SCREEN_WIDTH-150, 100)
{

}

void TBarreCase::Init()
{
	Icons = AddComponent(new TBarreCaseIcons(100,5));
	Icons->SetList();

	Country = AddComponent(new TListBox(&app.Font()->sm, 2, 5, 100, Height()-25));
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
			TMessageBox mb("Veuillez mettre un nombre dans l'unité", BT_OK, dynamic_cast<TForm*>(Parent()));
			mb.Show();
			return;
		}
		if(Owner->GetSelectedItem() < 0)
		{
			TMessageBox mb ("Veuillez sélectionner un propriétaire", BT_OK, dynamic_cast<TForm*>(Parent()));
			mb.Show();
			return;
		}
		entity->SetNb(StrToTyp<uint>(Nb->Text()));

		bool dont_check = false;
		SDL_Color last_color = white_color;
		if(entity->Owner())
		{
			if(dynamic_cast<EMapPlayer*>(entity->Owner())->ID() == Owner->ReadValue(Owner->GetSelectedItem())[0])
				dont_check = true;
			else
			{
				entity->Owner()->Entities()->Remove(entity);
				last_color = *color_eq[entity->Owner()->Color()];
			}
		}
		else
		{
			if(Owner->ReadValue(Owner->GetSelectedItem())[0] == '*')
				dont_check = true;
			else
				entity->Case()->Map()->Neutres()->Remove(entity);
		}

		if(!dont_check)
		{
			if(Owner->ReadValue(Owner->GetSelectedItem())[0] == '*')
			{
				entity->SetOwner(0);
				entity->Case()->Map()->Neutres()->Add(entity);
			}
			else
			{
				BMapPlayersVector mps = dynamic_cast<TMapEditor*>(Parent())->Map->Map()->MapPlayers();
				for(BMapPlayersVector::iterator it = mps.begin(); it != mps.end(); ++it)
					if((*it)->ID() == Owner->ReadValue(Owner->GetSelectedItem())[0])
					{
						entity->SetOwner(dynamic_cast<EMapPlayer*>(*it));
						dynamic_cast<EMapPlayer*>(*it)->Entities()->Add(entity);
						break;
					}
			}
			entity->RefreshColor(last_color);
			dynamic_cast<EMap*>(entity->Case()->Map())->CreatePreview(120,120,true);
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
	: TChildForm(0, SCREEN_HEIGHT-100, SCREEN_WIDTH-150, 100), entity(0)
{

}

void TBarreEntity::Init()
{
	Name = AddComponent(new TLabel(60,5, "", black_color, &app.Font()->big));

	RemoveButton = AddComponent(new TButtonText(Width()-150,5,100,30, "Supprimer", &app.Font()->sm));
	RemoveButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));

	Owner = AddComponent(new TListBox(&app.Font()->sm, RemoveButton->X()-105, 5, 100, Height()-15));

	Icon = AddComponent(new TImage(5,5));

	Nb = AddComponent(new TEdit(&app.Font()->sm, 5,55,100));
	Nb->SetAvailChars("0123456789");

	SetBackground(Resources::BarreAct());
}

/********************************************************************************************
 *                                TEditBarreLatIcons                                        *
 ********************************************************************************************/

void TEditBarreLatIcons::SelectUnit(TObject* o, void* e)
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

void TEditBarreLatIcons::SetList(std::vector<ECEntity*> list)
{
	Clear();

	int _x = 0, _y = 0;
	uint _h = 0, _w = 0;
	bool left = true;
	for(std::vector<ECEntity*>::iterator it = list.begin(); it != list.end(); ++it, left = !left)
	{
		TImage* i = AddComponent(new TImage(_x, _y, (*it)->Icon(), false));

		if(i->Width() > _w) _w = i->Width();
		if(i->Height() > _h) _h = i->Height();

		if(left)
			_x += _w;
		else
		{
			_x = 0;
			_y += _h;
		}
		i->SetOnClick(TEditBarreLatIcons::SelectUnit, (void*)*it);
	}
	if(!left) // c'est à dire on était à droite
		_y += _h;
	SetWidth(2*_w);
	SetHeight(_y);
}

/********************************************************************************************
 *                               TBarreLat                                                  *
 ********************************************************************************************/

void TEditBarreLat::RadarClick(TObject* m, int x, int y)
{
	TImage* map = dynamic_cast<TImage*>(m);
	TMapEditor* editor = dynamic_cast<TMapEditor*>(m->Parent()->Parent());

	if(!map)
		throw ECExcept(VPName(map), "Appel incorrect");

	int size_x = map->Width() / editor->Map->Map()->Width();
	int size_y = map->Height() / editor->Map->Map()->Height();
	int _x = BorneInt((x - map->X()) / size_x, 0, editor->Map->Map()->Width()-1);
	int _y = BorneInt((y - map->Y()) / size_y, 0, editor->Map->Map()->Height()-1);

	if(editor->Map->Enabled())
		editor->Map->CenterTo(dynamic_cast<ECase*>((*editor->Map->Map())(_x,_y)));

	map->DelFocus();
}

TEditBarreLat::TEditBarreLat()
	: TChildForm(SCREEN_WIDTH-150, 0, 150, SCREEN_HEIGHT)
{

}

void TEditBarreLat::Init()
{
	QuitButton = AddComponent(new TButtonText(30,200,100,30, "Fermer", &app.Font()->sm));
	QuitButton->SetImage(new ECSprite(Resources::LitleButton(), app.sdlwindow));
	OptionsButton = AddComponent(new TButtonText(30,230,100,30, "Configuration", &app.Font()->sm));
	OptionsButton->SetImage(new ECSprite(Resources::LitleButton(), app.sdlwindow));
	SaveButton = AddComponent(new TButtonText(30,260,100,30, "Sauvegarder", &app.Font()->sm));
	SaveButton->SetImage(new ECSprite(Resources::LitleButton(), app.sdlwindow));

	TMapEditor* editor = dynamic_cast<TMapEditor*>(Parent());

	OptionsButton->SetOnClick(TOptionsMap::Options, (void*)editor->Map->Map());

	Icons = AddComponent(new TEditBarreLatIcons(20, 300));
	Icons->SetList(EntityList.List());

	editor->Map->Map()->CreatePreview(120,120, true);
	int _x = 15 + 60 - editor->Map->Map()->Preview()->GetWidth() / 2 ;
	int _y = 55 + 60 - editor->Map->Map()->Preview()->GetHeight() / 2 ;
	Radar = AddComponent(new TImage(_x, _y, false));
	Radar->SetImage(editor->Map->Map()->Preview(), false);
	Radar->SetOnClickPos(TEditBarreLat::RadarClick);

	ScreenPos = AddComponent(new TImage(0,0));
	SDL_Surface *surf = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, w, h,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
	DrawRect(surf, 0, 0, editor->Map->Map()->Preview()->GetWidth()/editor->Map->Map()->Width()  * (SCREEN_WIDTH-w) / CASE_WIDTH,
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
		SDL_Event event;
		bool eob = false;
		OptionsMap = new TOptionsMap(app.sdlwindow, map);
		OptionsMap->Refresh();

		/* Mettre les valeurs qui ne seront enregistrées qu'à la fermeture */
		OptionsMap->Name->SetString(map->Name());
		OptionsMap->MinPlayers->SetValue(map->MinPlayers());
		OptionsMap->MaxPlayers->SetValue(map->MaxPlayers());
		OptionsMap->City->SetString(TypToStr(map->CityMoney()));
		OptionsMap->Begin->SetString(TypToStr(map->BeginMoney()));

		do
		{
			while( SDL_PollEvent( &event) )
			{
				OptionsMap->Actions(event);
				switch(event.type)
				{
					case SDL_MOUSEBUTTONDOWN:
					{
						if(OptionsMap->OkButton->Test(event.button.x, event.button.y))
						{
							if(OptionsMap->Name->Text().empty())
								TMessageBox("Veuillez donner un nom à la carte !", BT_OK, OptionsMap).Show();
							else if(OptionsMap->Players->Size() < 2)
								TMessageBox("Il doit y avoir au moins deux joueur", BT_OK, OptionsMap).Show();
							else if(OptionsMap->Countries->Empty())
								TMessageBox("Il doit y avoir au moins une country", BT_OK, OptionsMap).Show();
							else if(OptionsMap->Begin->Empty() || OptionsMap->City->Empty())
								TMessageBox("Vous devez mettre l'argent de début de partie et l'argent des villes",
								               BT_OK, OptionsMap).Show();
							else if(OptionsMap->MinPlayers->Value() < 2)
								TMessageBox("Le nombre minimal de joueurs doit être d'au moins deux joueurs", BT_OK,
								            OptionsMap).Show();
							else if(OptionsMap->MaxPlayers->Value() < OptionsMap->MinPlayers->Value())
								TMessageBox("Le nombre maximal de joueurs doit être égal ou supérieur au nombre de joueurs "
								            "minimal", BT_OK, OptionsMap).Show();
							else
								eob = true;
						}

						const char* id = 0;
						char m_id = 0;
						if(OptionsMap->AddPlayerButton->Test(event.button.x, event.button.y))
						{
							map->AddPlayer();
							OptionsMap->Refresh();
						}
						else if(OptionsMap->Players->GetSelectedItem() < 0 ||
						  !(m_id = OptionsMap->Players->ReadValue(OptionsMap->Players->GetSelectedItem())[0]))
							OptionsMap->DelPlayerButton->SetEnabled(false);
						else if(OptionsMap->Players->Test(event.button.x, event.button.y))
							OptionsMap->DelPlayerButton->SetEnabled(true);
						else if(OptionsMap->DelPlayerButton->Test(event.button.x, event.button.y))
						{
							map->RemovePlayer(m_id);
							OptionsMap->Refresh();
						}
						
						if(OptionsMap->AddCountryButton->Test(event.button.x, event.button.y))
						{
							std::string c_id = OptionsMap->AddCountryEdit->Text();
							if(c_id.empty() || c_id.size() != 2)
							{
								TMessageBox mb("L'identifiant doit faire 2 caractères", BT_OK, OptionsMap);
								mb.Show();
							}
							if(map->GetCountry(c_id.c_str()))
							{
								TMessageBox mb("L'identifiant est déjà utilisé", BT_OK, OptionsMap);
								mb.Show();
							}
							else
							{
								map->AddCountry(c_id.c_str());
								OptionsMap->AddCountryEdit->ClearString();
								OptionsMap->Refresh();
							}
						}
						else if(OptionsMap->Countries->GetSelectedItem() < 0 ||
						  !(id = OptionsMap->Countries->ReadValue(OptionsMap->Countries->GetSelectedItem()).c_str()))
						{
							OptionsMap->CountryPlayer->SetEnabled(false);
							OptionsMap->DelCountryButton->SetEnabled(false);
						}
						else if(OptionsMap->Countries->Test(event.button.x, event.button.y))
						{
							ECBCountry* country = 0;

							BCountriesVector cts = map->Countries();
							for(BCountriesVector::iterator it = cts.begin(); it != cts.end(); ++it)
								if(!strcmp((*it)->ID(), id))
								{
									country = *it;
									break;
								}
							if(!country) break;

							OptionsMap->DelCountryButton->SetEnabled(country->Cases().empty());

							OptionsMap->CountryPlayer->SetEnabled();
							OptionsMap->CountryPlayer->ClearItems();
							OptionsMap->CountryPlayer->AddItem(true, "Neutre", "*");
							BMapPlayersVector mps = map->MapPlayers();
							for(BMapPlayersVector::iterator it = mps.begin(); it != mps.end(); ++it)
								OptionsMap->CountryPlayer->AddItem(*it == country->Owner(), TypToStr((*it)->ID()),
								                                   TypToStr((*it)->ID()));
						}
						else if(OptionsMap->DelCountryButton->Test(event.button.x, event.button.y))
						{
							map->RemoveCountry(id);
							OptionsMap->Refresh();
						}
						else if(OptionsMap->CountryPlayer->GetSelectedItem() != -1)
						{
							const char map_id = OptionsMap->CountryPlayer->ReadValue(
							                               OptionsMap->CountryPlayer->GetSelectedItem())[0];
							ECBMapPlayer* mp = map_id == '*' ? 0 : map->GetPlayer(map_id);
							if(!mp && map_id != '*') break;

							ECBCountry* country = map->GetCountry(id);
							if(!country) break;

							country->ChangeOwner(mp);
						}
						break;
					}
					default:
						break;
				}
			}
			OptionsMap->Update();
		} while(!eob);
	}
	catch(TECExcept &e)
	{
		MyFree(OptionsMap);
		throw;
	}

	map->Name() = OptionsMap->Name->Text();
	map->MaxPlayers() = OptionsMap->MaxPlayers->Value();
	map->MinPlayers() = OptionsMap->MinPlayers->Value();
	map->BeginMoney() = StrToTyp<int>(OptionsMap->Begin->Text());
	map->CityMoney()  = StrToTyp<int>(OptionsMap->City->Text());
	
	MyFree(OptionsMap);
	map->CreatePreview(120,120, true);
	map->SetCanSave();
}

void TOptionsMap::Refresh()
{
	Countries->ClearItems();
	BCountriesVector cts = map->Countries();
	for(BCountriesVector::iterator it = cts.begin(); it != cts.end(); ++it)
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
}

TOptionsMap::TOptionsMap(SDL_Surface* w, EMap* m)
	: TForm(w)
{
	map = m;

	OkButton = AddComponent(new TButtonText(600,500,150,50, "OK", &app.Font()->normal));

	PlayersLabel = AddComponent(new TLabel(100, 130, "Joueurs :", white_color, &app.Font()->normal));
	Players = AddComponent(new TListBox(&app.Font()->sm, 100,150,150,200));
	Players->SetNoItemHint();
	Players->SetHint("Ce sont les différents joueurs susceptibles d'être joués.\nChaque territoire et unité peut être lié à "
	                 " un joueur.");

	AddPlayerButton = AddComponent(new TButtonText(255, 150, 100, 30, "Ajouter", &app.Font()->normal));
	AddPlayerButton->SetImage(new ECSprite(Resources::LitleButton(), app.sdlwindow));

	DelPlayerButton = AddComponent(new TButtonText(255, 180, 100, 30, "Supprimer", &app.Font()->normal));
	DelPlayerButton->SetImage(new ECSprite(Resources::LitleButton(), app.sdlwindow));

	CountriesLabel = AddComponent(new TLabel(400, 130, "Territoires :", white_color, &app.Font()->normal));
	Countries = AddComponent(new TListBox(&app.Font()->sm, 400,150,150,200));
	Countries->SetNoItemHint();
	Countries->SetHint("Liste des différents territoires. Vous pouvez assigner chaque case à un territoire.\n"
	                   "Vous ne pouvez supprimer que les territoires en vert, qui ne sont assignés à aucune case.");

	AddCountryEdit = AddComponent(new TEdit(&app.Font()->sm, 400,360,150, 2, COUNTRY_CHARS));

	AddCountryButton = AddComponent(new TButtonText(555, 350, 100, 30, "Ajouter", &app.Font()->normal));
	AddCountryButton->SetImage(new ECSprite(Resources::LitleButton(), app.sdlwindow));

	DelCountryButton = AddComponent(new TButtonText(555, 320, 100, 30, "Supprimer", &app.Font()->normal));
	DelCountryButton->SetImage(new ECSprite(Resources::LitleButton(), app.sdlwindow));

	CountryPlayerLabel = AddComponent(new TLabel(560, 150, "Appartient à :", white_color, &app.Font()->sm));
	CountryPlayer = AddComponent(new TComboBox(&app.Font()->sm, 560 + CountryPlayerLabel->Width(), 150, 70));
	CountryPlayer->SetEnabled(false);
	CountryPlayer->SetNoItemHint();
	CountryPlayer->SetHint("Propriétaire du territoire sélectionné.\n"
	                       "Un territoire neutre n'appartiendra à personne au début de la partie.");

	NameLabel = AddComponent(new TLabel(50,380, "Nom de la carte", white_color, &app.Font()->normal));
	Name = AddComponent(new TEdit(&app.Font()->sm, 50, 400, 150, 50, EDIT_CHARS));

	                                                            // label    x    y    w  min   max                 step  def
	MinPlayers = AddComponent(new TSpinEdit(&app.Font()->sm, "Min players", 50, 420, 150, 1, map->MapPlayers().size(), 1, 1));
	MaxPlayers = AddComponent(new TSpinEdit(&app.Font()->sm, "Max players", 50, 440, 150, 1, map->MapPlayers().size(), 1, 1));

	CityLabel = AddComponent(new TLabel(50,460, "Argent par villes", white_color, &app.Font()->normal));
	City = AddComponent(new TEdit(&app.Font()->sm, 50, 480, 150, 5, "0123456789"));

	BeginLabel = AddComponent(new TLabel(50,500, "Argent au début de la partie", white_color, &app.Font()->normal));
	Begin = AddComponent(new TEdit(&app.Font()->sm, 50, 520, 150, 5, "0123456789"));

	Hints = AddComponent(new TMemo(&app.Font()->sm, 300, 480, 290, 100));
	SetHint(Hints);

	SetBackground(Resources::Titlescreen());
}

TOptionsMap::~TOptionsMap()
{
	delete Hints;
	delete Begin;
	delete BeginLabel;
	delete City;
	delete CityLabel;
	delete MaxPlayers;
	delete MinPlayers;
	delete Name;
	delete NameLabel;
	delete CountryPlayer;
	delete CountryPlayerLabel;
	delete DelCountryButton;
	delete AddCountryButton;
	delete AddCountryEdit;
	delete Countries;
	delete CountriesLabel;
	delete DelPlayerButton;
	delete AddPlayerButton;
	delete Players;
	delete PlayersLabel;
	delete OkButton;
}

/********************************************************************************************
 *                                  TLoadMapFile                                            *
 ********************************************************************************************/

void MenAreAntsApp::MapEditor()
{
	TLoadMapFile* LoadMapFile = 0;
	try
	{
		SDL_Event event;
		bool eob = false;
		LoadMapFile = new TLoadMapFile(sdlwindow);
		LoadMapFile->SetMutex(mutex);
		do
		{
			while( SDL_PollEvent( &event) )
			{
				LoadMapFile->Actions(event);
				switch(event.type)
				{
					case SDL_MOUSEBUTTONDOWN:
						if(LoadMapFile->MapsList->Test(event.button.x, event.button.y))
						{
							if(LoadMapFile->MapsList->GetSelectedItem() >= 0 &&
							   LoadMapFile->MapsList->EnabledItem(LoadMapFile->MapsList->GetSelectedItem()))
								LoadMapFile->LoadButton->SetEnabled(true);
							else
								LoadMapFile->LoadButton->SetEnabled(false);
						}
						else if(LoadMapFile->NewButton->Test(event.button.x, event.button.y))
						{
							if(!TMapEditor::Editor(NULL, LoadMapFile))
							{
								TMessageBox mb(std::string("Impossible de créer la carte.\n"
													"Son nom est peut être déjà utilisé, ou alors les informations "
													"que vous avez fournis sont invalides.").c_str(),
													BT_OK, LoadMapFile);
								mb.Show();
							}
							LoadMapFile->Refresh();
						}
						else if(LoadMapFile->LoadButton->Test(event.button.x, event.button.y))
						{
							if(!TMapEditor::Editor(LoadMapFile->MapsList->ReadValue(
										LoadMapFile->MapsList->GetSelectedItem()).c_str()))
							{
								TMessageBox mb(std::string("Impossible d'ouvrir la map " +
												LoadMapFile->MapsList->ReadValue(
														LoadMapFile->MapsList->GetSelectedItem())
												+ ".\nVeuillez reessayer").c_str(), BT_OK, LoadMapFile);
								mb.Show();
							}
							LoadMapFile->Refresh();
						}
						else if(LoadMapFile->RetourButton->Test(event.button.x, event.button.y))
							eob = true;
						break;
					default:
						break;
				}
			}
			LoadMapFile->Update();
		} while(!eob);
	}
	catch(TECExcept &e)
	{
		MyFree(LoadMapFile);
		throw;
	}
	MyFree(LoadMapFile);
}

void TLoadMapFile::Refresh()
{
	MapsList->ClearItems();
#if 0
	WIN32_FIND_DATA File;
	HANDLE hSearch;
	BOOL re;
	
	hSearch=FindFirstFile("*.map", &File);
	if(hSearch ==  INVALID_HANDLE_VALUE)
		return;
	
	re=TRUE;
	do
	{
		/* Traitement */
		re = FindNextFile(hSearch, &File);
	} while(re);
	
	FindClose(hSearch);
#else
	struct dirent *lecture;
	DIR *rep;
	rep = opendir(app.GetPath().c_str());
	while ((lecture = readdir(rep)))
	{
		std::string s = lecture->d_name;
		if(s.rfind(".map") != s.size() - 4) continue;
		if(!s.empty())
			MapsList->AddItem(false, s.substr(0, s.size() - 4), app.GetPath() + s,
			                             black_color, true);
	}
	closedir(rep);
#endif
}

TLoadMapFile::TLoadMapFile(SDL_Surface* w)
	: TForm(w)
{

	MapsList = AddComponent(new TListBox(&app.Font()->sm, 300,200,200,300));
	Refresh();

	NewButton = AddComponent(new TButtonText(550,250,150,50, "Nouveau", &app.Font()->normal));
	LoadButton = AddComponent(new TButtonText(550,300,150,50, "Charger", &app.Font()->normal));
	LoadButton->SetEnabled(false);
	RetourButton = AddComponent(new TButtonText(550,350,150,50, "Retour", &app.Font()->normal));

	SetBackground(Resources::Titlescreen());
}

TLoadMapFile::~TLoadMapFile()
{
	delete RetourButton;
	delete LoadButton;
	delete NewButton;
	delete MapsList;
}
