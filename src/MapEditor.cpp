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

	map_infos.push_back("Map créée par " + Config::GetInstance()->nick);
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

	if(IsMission())
		fp << "MISSION" << std::endl;

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
	fp << "CITY " << CityMoney() << std::endl;
	fp << "MIN " << MinPlayers() << std::endl;
	fp << "MAX " << MaxPlayers() << std::endl;
	fp << "DATE " << Date()->String() << std::endl;
	for(std::vector<std::string>:: const_iterator it = map_infos.begin(); it != map_infos.end(); ++it)
		fp << "INFO " << *it << std::endl;

	for(std::vector<std::string>::const_iterator it = other_lines.begin(); it != other_lines.end(); ++it)
		fp << *it << std::endl;

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
			TMessageBox mb("Combien de cases en ordonnée\n(vertical) ?", HAVE_EDIT|BT_OK|BT_CANCEL, form);
			mb.Edit()->SetAvailChars("0123456789");
			mb.Edit()->SetMaxLen(2);
			if(mb.Show() == BT_OK)
				y = StrToTyp<uint>(mb.EditText());
			else return true;
		}
		{
			TMessageBox mb("Quelle est la date en début de partie ?\n('xx xx xxxx' ou 'xx/xx/xxxx')",
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
		bool eob = false;
		MapEditor = new TMapEditor(Video::GetInstance()->Window(), map);

		SDL_Event event;
		do
		{
			while( SDL_PollEvent( &event) )
			{
				MapEditor->Actions(event);
				switch(event.type)
				{
					case SDL_KEYUP:
					{
						switch(event.key.keysym.sym)
						{
							case SDLK_DELETE:
							{
								ECEntity* entity = 0;
								if((entity = MapEditor->BarreEntity->Entity()))
								{
									MapEditor->BarreEntity->UnSelect();
									map->RemoveAnEntity(entity);
									map->CreatePreview(120,120,P_FRONTMER|P_ENTITIES);
									break;
								}
								break;
							}
							default: break;
						}
					}
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
								ECEntity* et = entities_type[entity->Type()].create ("**", 0, acase, entity->InitNb(), map);
								map->AddAnEntity(et);

								MapEditor->BarreCase->UnSelect();
								MapEditor->Map->SetCreateEntity(0);
								MapEditor->BarreEntity->SetEntity(et);
								MapEditor->Map->SetPosition(MapEditor->Map->X(), MapEditor->Map->Y(), true);
								map->CreatePreview(120,120,P_FRONTMER|P_ENTITIES);
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
								               "Voulez-vous quand même fermer sans sauvegarder ?", BT_OK|BT_CANCEL,
								                MapEditor);
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
							map->CreatePreview(120,120,P_FRONTMER|P_ENTITIES);
							break;
						}

						if(MapEditor->BarreEntity->Test(event.button.x, event.button.y) ||
						   MapEditor->BarreCase->Test(event.button.x, event.button.y) ||
						   MapEditor->BarreLat->Test(event.button.x, event.button.y))
							break;

						if(event.button.button == MBUTTON_LEFT)
						{
							ECase* c = 0;
							if(!MapEditor->IsPressed(SDLK_LCTRL) && !MapEditor->IsPressed(SDLK_LSHIFT) &&
							   (entity = MapEditor->Map->TestEntity(event.button.x, event.button.y)))
							{
								MapEditor->BarreCase->UnSelect();
								MapEditor->BarreEntity->SetEntity(entity);
							}
							else if((c = MapEditor->Map->TestCase(event.button.x, event.button.y)))
							{
								MapEditor->BarreEntity->UnSelect();
								if(!MapEditor->BarreCase->RemoveCase(c, false) ||
								   MapEditor->BarreCase->Cases().size() > 1 && !MapEditor->IsPressed(SDLK_LCTRL))
								{
									if(MapEditor->IsPressed(SDLK_LSHIFT) && !MapEditor->BarreCase->Empty())
									{
										ECase* first = MapEditor->BarreCase->Cases().front();
										ECBCase* next = first;
										uint t_x = abs(first->X() - c->X());
										uint t_y = abs(first->Y() - c->Y());
										MapEditor->BarreCase->UnSelect(false);
										MapEditor->BarreCase->AddCase(first, false);
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
													MapEditor->BarreCase->AddCase(dynamic_cast<ECase*>(next), false);
												if(next->X() == next->Map()->Width()-1)
													break;
												next = next->MoveRight();
											}
											if(next->Y() == next->Map()->Height()-1)
												break;
											next = next->MoveDown();
											next = next->MoveLeft(j);
										}
										MapEditor->BarreCase->Update(c);
									}
									else
									{
										if(!MapEditor->IsPressed(SDLK_LCTRL))
											MapEditor->BarreCase->UnSelect(false);
										MapEditor->BarreCase->AddCase(c);
									}
								}
								else if(MapEditor->BarreCase->Empty())
									MapEditor->BarreCase->UnSelect();
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
			Video::GetInstance()->Window()->Fill(0);
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

TMapEditor::TMapEditor(ECImage* w, ECMap *m)
	: TForm(w)
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
 *                                TBarreCaseIcons                                        *
 ********************************************************************************************/

void TBarreCaseIcons::Init()
{
	TChildForm* form = dynamic_cast<TChildForm*>(Parent());

	if(!Last)
		Last = AddComponent(new TButton (form->Width()-X()-10, 5 ,5,10));
	else
		Last->SetX(X() + form->Width()-X()-10);

	if(!Next)
		Next = AddComponent(new TButton (form->Width()-X()-10, 80,5,10));
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

	if(bc->Country->GetSelectedItem() < 0)
		return;

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
	: TChildForm(0, SCREEN_HEIGHT-110, SCREEN_WIDTH-200, 110)
{

}

void TBarreCase::Init()
{
	Icons = AddComponent(new TBarreCaseIcons(100,10));
	Icons->SetList();

	Country = AddComponent(new TListBox(Font::GetInstance(Font::Small), 2, 15, 100, Height()-40));
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
		Color last_color = white_color;
		if(entity->Owner())
		{
			if(dynamic_cast<EMapPlayer*>(entity->Owner())->ID() == Owner->ReadValue(Owner->GetSelectedItem())[0])
				dont_check = true;
			else
			{
				entity->Owner()->Entities()->Remove(entity);
				last_color = color_eq[entity->Owner()->Color()];
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

	Owner = AddComponent(new TListBox(Font::GetInstance(Font::Small), RemoveButton->X()-105, 15, 100, Height()-15));

	Icon = AddComponent(new TImage(5,15));

	Nb = AddComponent(new TEdit(Font::GetInstance(Font::Small), 5,65,100));
	Nb->SetAvailChars("0123456789");

	SetBackground(Resources::BarreAct());
}

/********************************************************************************************
 *                                TEditBarreLatIcons                                        *
 ********************************************************************************************/

/********************************************************************************************
 *                               TBarreLat                                                  *
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
	SDL_Surface *surf = SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, w, h,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
	DrawRect(surf, 0, 0,
	          editor->Map->Map()->Preview()->GetWidth()/editor->Map->Map()->Width()  * (SCREEN_WIDTH-w) / CASE_WIDTH,
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
		OptionsMap = new TOptionsMap(Video::GetInstance()->Window(), map);
		OptionsMap->Refresh();

		/* Mettre les valeurs qui ne seront enregistrées qu'à la fermeture */
		OptionsMap->Name->SetString(map->Name());
		OptionsMap->MinPlayers->SetValue(map->MinPlayers());
		OptionsMap->MaxPlayers->SetValue(map->MaxPlayers());
		OptionsMap->City->SetString(TypToStr(map->CityMoney()));

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
							else if(OptionsMap->City->Empty())
								TMessageBox("Vous devez mettre l'argent des villes",
								               BT_OK, OptionsMap).Show();
							else if(OptionsMap->MinPlayers->Value() < 2)
								TMessageBox("Le nombre minimal de joueurs doit être d'au moins deux joueurs", BT_OK,
								            OptionsMap).Show();
							else if(OptionsMap->MaxPlayers->Value() < OptionsMap->MinPlayers->Value())
								TMessageBox("Le nombre maximal de joueurs doit être égal ou supérieur au nombre de "
								            "joueurs minimal", BT_OK, OptionsMap).Show();
							else if(StrToTyp<int>(OptionsMap->City->Text()) < 1)
								TMessageBox("Il faut qu'il y ait de l'argent pour chaque ville par tour",
								            BT_OK, OptionsMap).Show();
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
	map->CityMoney()  = StrToTyp<int>(OptionsMap->City->Text());
	map->IsMission()  = OptionsMap->Mission->Checked();
	
	MyFree(OptionsMap);
	map->CreatePreview(120,120, P_FRONTMER|P_ENTITIES);
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

	Mission->Check(map->IsMission());
}

TOptionsMap::TOptionsMap(ECImage* w, EMap* m)
	: TForm(w)
{
	map = m;

	OkButton = AddComponent(new TButtonText(600,500,150,50, "OK", Font::GetInstance(Font::Normal)));

	PlayersLabel = AddComponent(new TLabel(100, 130, "Joueurs :", white_color, Font::GetInstance(Font::Normal)));
	Players = AddComponent(new TListBox(Font::GetInstance(Font::Small), 100,150,150,200));
	Players->SetNoItemHint();
	Players->SetHint("Ce sont les différents joueurs susceptibles d'être joués.\nChaque territoire et unité peut être lié à "
	                 " un joueur.");

	AddPlayerButton = AddComponent(new TButtonText(255, 150, 100, 30, "Ajouter", Font::GetInstance(Font::Normal)));
	AddPlayerButton->SetImage(new ECSprite(Resources::LitleButton(), Video::GetInstance()->Window()));

	DelPlayerButton = AddComponent(new TButtonText(255, 180, 100, 30, "Supprimer", Font::GetInstance(Font::Normal)));
	DelPlayerButton->SetImage(new ECSprite(Resources::LitleButton(), Video::GetInstance()->Window()));

	CountriesLabel = AddComponent(new TLabel(400, 130, "Territoires :", white_color, Font::GetInstance(Font::Normal)));
	Countries = AddComponent(new TListBox(Font::GetInstance(Font::Small), 400,150,150,200));
	Countries->SetNoItemHint();
	Countries->SetHint("Liste des différents territoires. Vous pouvez assigner chaque case à un territoire.\n"
	                   "Vous ne pouvez supprimer que les territoires en vert, qui ne sont assignés à aucune case.");

	AddCountryEdit = AddComponent(new TEdit(Font::GetInstance(Font::Small), 400,360,150, 2, COUNTRY_CHARS));

	AddCountryButton = AddComponent(new TButtonText(555, 350, 100, 30, "Ajouter", Font::GetInstance(Font::Normal)));
	AddCountryButton->SetImage(new ECSprite(Resources::LitleButton(), Video::GetInstance()->Window()));

	DelCountryButton = AddComponent(new TButtonText(555, 320, 100, 30, "Supprimer", Font::GetInstance(Font::Normal)));
	DelCountryButton->SetImage(new ECSprite(Resources::LitleButton(), Video::GetInstance()->Window()));

	CountryPlayerLabel = AddComponent(new TLabel(560, 150, "Appartient à :", white_color, Font::GetInstance(Font::Small)));
	CountryPlayer = AddComponent(new TComboBox(Font::GetInstance(Font::Small), 560 + CountryPlayerLabel->Width(), 150, 70));
	CountryPlayer->SetEnabled(false);
	CountryPlayer->SetNoItemHint();
	CountryPlayer->SetHint("Propriétaire du territoire sélectionné.\n"
	                       "Un territoire neutre n'appartiendra à personne au début de la partie.");

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
	                 "Cet editeur ne permet pas de configurer les paramètres spéciaux des missions");

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
		SDL_Event event;
		bool eob = false;
		LoadMapFile = new TLoadMapFile(Video::GetInstance()->Window());
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
							if(LoadMapFile->MapsList->GetSelectedItem() >= 0 &&
							   !TMapEditor::Editor(LoadMapFile->MapsList->ReadValue(
										LoadMapFile->MapsList->GetSelectedItem()).c_str()))
							{
								TMessageBox mb(std::string("Impossible d'ouvrir la map " +
												LoadMapFile->MapsList->ReadValue(
														LoadMapFile->MapsList->GetSelectedItem())
												+ ".\nVeuillez reessayer").c_str(), BT_OK, LoadMapFile);
								mb.Show();
							}
							LoadMapFile->Refresh();
							LoadMapFile->LoadButton->SetEnabled(false);
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

	std::vector<std::string> file_list = GetFileList(MenAreAntsApp::GetInstance()->GetPath(), "map");

	for(std::vector<std::string>::const_iterator it = file_list.begin(); it != file_list.end(); ++it)
		MapsList->AddItem(false, it->substr(0, it->size() - 4), MenAreAntsApp::GetInstance()->GetPath() + *it,
			                             black_color, true);
}

TLoadMapFile::TLoadMapFile(ECImage* w)
	: TForm(w)
{

	MapsList = AddComponent(new TListBox(Font::GetInstance(Font::Small), 300,200,200,300));
	Refresh();

	NewButton = AddComponent(new TButtonText(550,250,150,50, "Nouveau", Font::GetInstance(Font::Normal)));
	LoadButton = AddComponent(new TButtonText(550,300,150,50, "Charger", Font::GetInstance(Font::Normal)));
	LoadButton->SetEnabled(false);
	RetourButton = AddComponent(new TButtonText(550,350,150,50, "Retour", Font::GetInstance(Font::Normal)));

	SetBackground(Resources::Titlescreen());
}
