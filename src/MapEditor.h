/* src/MapEditor.h - Header of MapEditor.cpp
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

#ifndef EC_MAPEDITOR_H
#define EC_MAPEDITOR_H

#include "Map.h"
#include "Channels.h"
#include "gui/Form.h"
#include "gui/ListBox.h"
#include "gui/BouttonText.h"
#include "gui/ShowMap.h"
#include "gui/ChildForm.h"
#include "gui/Image.h"
#include "gui/Edit.h"
#include "gui/ComboBox.h"
#include "gui/Memo.h"
#include "gui/Label.h"

class TCheckBox;
class TSpinEdit;
class TBarreLatIcons;

/********************************************************************************************
 *                                  EMap                                                    *
 ********************************************************************************************/

class EMapPlayer : public ECMapPlayer, public ECPlayer
{
/* Constructeurs */
public:
	EMapPlayer(char _id, uint _num)
		: ECMapPlayer(_id, _num), ECPlayer("",0,false,false,false,false)
	{
		SetMapPlayer(this);
		SetPlayer(this);
	}

	virtual bool IsIA() const { return true; }
	virtual const char* GetNick() const { return ""; }

};

class EMap : public ECMap
{
/* Constructeurs */
public:
	EMap(std::string filename, uint x, uint y, std::string date);
	EMap(std::string f)
		: ECMap(f), can_save(false)
	{}

/* Methodes */
public:
	void Save();

	void AddCountry(const char* id);
	bool RemoveCountry(const char* id);
	ECBCountry* GetCountry(const char* id);

	void AddPlayer();
	bool RemovePlayer(const char id);
	ECBMapPlayer* GetPlayer(const char id);

	bool CanSave() const { return can_save; }
	void SetCanSave(bool b = true) { can_save = b; }

/* variables privées */
private:

	bool can_save;
	std::vector<std::string> other_lines;

	virtual ECBMapPlayer* CreateMapPlayer(char _id, uint _num)
	{
		EMapPlayer* mp = new EMapPlayer(_id, _num);
		mp->SetColor(_id%COLOR_MAX);
		return mp;
	}
	virtual void VirtualAddUnit(std::string line);
	virtual void SeeMapLine(std::string line);
};

/********************************************************************************************
 *                                  TMapEditor                                              *
 ********************************************************************************************/

class TBarreCaseIcons : public TChildForm
{
public:
	TBarreCaseIcons(int _x, int _y)
		: TChildForm(_x,_y, 0, 0), Next(0), Last(0), first(0)
	{}

	void Init();

	void SetList();

	TButton* Next;
	TButton* Last;

private:

	static void SelectCase(TObject* o, void* e);

	virtual bool Clic(const Point2i&, int button);

	uint first;
};

class TBarreCase : public TChildForm
{
public:
	TBarreCase();

/* Composants */
public:

	TBarreCaseIcons* Icons;
	TListBox*        Country;

/* Attributs */
public:

	std::vector<ECase*> Cases() const { return cases; }
	bool Empty() const { return cases.empty(); }
	void AddCase(ECase* c, bool update = true) { c->Select(true); cases.push_back(c); if(update) Update(c); }
	bool RemoveCase(ECase* c, bool update = true);

/* Methodes */
public:
	void Update(ECase* = 0);

	ECase* ChangeCaseType(ECase*, case_img_t*);
	void CheckAroundCase(ECase* c);

	void Init();

	void UnSelect(bool update = true);
	bool Select() const { return select; }

private:
	std::vector<ECase*> cases;

	static void ChangeOwner(TObject*, void*);

	bool select;
};

class TBarreEntity : public TChildForm
{
public:
	TBarreEntity();

/* Composants */
public:

	TImage*         Icon;
	TLabel*         Name;
	TEdit*          Nb;
	TListBox*       Owner;
	TButtonText*    RemoveButton;

/* Attributs */
public:

	ECEntity* Entity() const { return entity; }
	void SetEntity(ECEntity* e);

/* Methodes */
public:
	void Update() { if(entity) SetEntity(entity); }

	void Init();

	void UnSelect();
	bool Select() const { return select; }

private:
	ECEntity* entity;

	bool select;
};

class TEditBarreLat : public TChildForm
{
public:
	TEditBarreLat();

	void Init();

/* Composants */
public:
	TImage*             Radar;
	TButtonText*        QuitButton;
	TButtonText*        OptionsButton;
	TButtonText*        SaveButton;
	TImage*             ScreenPos;
	TBarreLatIcons*     Icons;

/* Evenements */
public:
	static void RadarClick(TObject*, const Point2i&);

protected:
	static void SelectUnit(TObject* o, void* e);
};

class TMapEditor : public TForm
{
/* Constructeurs */
public:
	TMapEditor(ECImage* w, EMap* m);

/* Methodes */
public:
	static bool Editor(const char *path, TForm* form = 0);
	void ShowBarreLat(bool show = true);
	void ShowBarreAct(bool show = true, ECase* c = 0);

/* Composants */
public:

	TMap*          Map;
	TEditBarreLat* BarreLat;
	TBarreEntity*  BarreEntity;
	TBarreCase*    BarreCase;

/* Evenements */
private:

	void OnKeyUp(SDL_keysym);
	void OnKeyDown(SDL_keysym key);
	void OnClic(const Point2i&, int, bool&);
	void OnClicUp(const Point2i& mouse, int button);
	void BeforeDraw();

	EMap* map;
};

/********************************************************************************************
 *                                  TOptionsMap                                             *
 ********************************************************************************************/
class TOptionsMap : public TForm
{
/* Constructeur */
public:
	TOptionsMap(ECImage* w, EMap* map);

/* Composants */
public:

	TButtonText* OkButton;
	TLabel*      PlayersLabel;
	TListBox*    Players;
	TButtonText* AddPlayerButton;
	TButtonText* DelPlayerButton;
	TLabel*      CountriesLabel;
	TListBox*    Countries;
	TEdit*       AddCountryEdit;
	TButtonText* AddCountryButton;
	TButtonText* DelCountryButton;
	TLabel*      CountryPlayerLabel;
	TComboBox*   CountryPlayer;

	TLabel*      NameLabel;
	TEdit*       Name;

	TSpinEdit*   MinPlayers;
	TSpinEdit*   MaxPlayers;

	TLabel*      CityLabel;
	TEdit*       City;

	TCheckBox*   Mission;

	TMemo*       Hints;

/* Methodes */
public:

	void Refresh();

	static void Options(TObject*, void* m);

/* Evenements */
private:

	void OnClic(const Point2i&, int, bool&);

/* Variables privées */
private:
	EMap* map;
};


/********************************************************************************************
 *                                  TLoadMapFile                                            *
 ********************************************************************************************/

class TLoadMapFile : public TForm
{
/* Constructeur/Destructeur */
public:
	TLoadMapFile(ECImage* w);

	void Refresh();

/* Composants */
public:

	TListBox*    MapsList;
	TButtonText* RetourButton;
	TButtonText* NewButton;
	TButtonText* LoadButton;

/* Evenements */
private:
	void OnClic(const Point2i& mouse, int button, bool&);
};

#endif /* EC_MAPEDITOR_H */
