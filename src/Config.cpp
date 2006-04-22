/* src/Config.cpp - Configuration
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

#include <string>
#include <fstream>
#include <iostream>

#include "Outils.h"
#include "Config.h"
#include "Resources.h"
#include "Channels.h"
#include "lib/Colors.h"
#include "gui/Form.h"
#include "gui/ListBox.h"
#include "gui/Label.h"
#include "gui/BouttonText.h"
#include "gui/Edit.h"
#include "gui/ComboBox.h"
#include "gui/ColorEdit.h"
#include "gui/MessageBox.h"
#include "gui/Memo.h"
#include "Main.h"

Config::Config(std::string _filename)
{
	filename = _filename;
	want_quit_config = false;
	set_defaults(false);
}

bool Config::set_defaults(bool want_save)
{
	hostname = "127.0.0.1";
	port = 5461;
	nick = "anonyme";
	color = 0;
	nation = 0;
	server_list.clear();
	server_list.push_back("127.0.0.1:5461");
	server_list.push_back("game.coderz.info:5461");
	server_list.push_back("192.168.0.2:5461");
	if(want_save)
		save();
	return true;
}

bool Config::load()
{
	std::ifstream fp(filename.c_str());

	if(!fp)
	{
		app.FirstRun();
		return set_defaults();
	}

	std::string ligne;

	server_list.clear();

	while(std::getline(fp, ligne))
	{
		std::string key = stringtok(ligne, " ");

		if(key == "SERVER") hostname = ligne;
		else if(key == "PORT" && is_num(ligne.c_str())) port = atoi(ligne.c_str());
		else if(key == "COLOR" && is_num(ligne.c_str())) color = atoi(ligne.c_str());
		else if(key == "NATION" && is_num(ligne.c_str())) nation = atoi(ligne.c_str());
		else if(key == "NICK") nick = ligne;
		else if(key == "SERVERLIST") server_list.push_back(ligne);
		else
		{
			std::cout << "Fichier incorrect" << std::endl;
			return set_defaults();
		}
	}
	if(hostname.empty() || port < 1 || port > 65535 || color >= COLOR_MAX || nation >= ECPlayer::N_MAX)
	{
		std::cout << "Lecture de la configuration invalide." << std::endl;
		return set_defaults();
	}

	return true;
}

bool Config::save() const
{
    std::ofstream fp(filename.c_str());
    if (!fp)
    {
        std::cerr << "Impossible de créer le fichier de configuration" << std::endl;
        return 0;
    }

    fp << "SERVER " << hostname << std::endl;
    fp << "PORT " << port << std::endl;
    fp << "NICK " << nick << std::endl;
    fp << "COLOR " << color << std::endl;
    fp << "NATION " << nation << std::endl;
    for(std::vector<std::string>::const_iterator it = server_list.begin(); it != server_list.end(); ++it)
    	fp << "SERVERLIST " << *it << std::endl;

	return true;
}

/********************************************************************************************
 *                                    TConfigForm                                           *
 ********************************************************************************************/

class TConfigForm : public TForm
{
/* Constructeur/Destructeur */
public:

	TConfigForm(SDL_Surface*);
	~TConfigForm();

/* Composants */
public:

	TButtonText*    OkButton;
	TButtonText*    CancelButton;
	TListBox*       ServerList;
	TEdit*          Nick;
	TColorEdit*     Color;
	TComboBox*      Nation;
	TLabel*         Title;
	TLabel*         Info;
	TLabel*         NickInfo;
	TLabel*         NationInfo;
	TMemo*          Hints;

/* Evenements */
public:

};

void Config::WantOk(TObject* OkButton, void* configinst)
{
	TConfigForm* form = static_cast<TConfigForm*>(OkButton->Parent());
	Config* conf = static_cast<Config*>(configinst);

	if(form->Nick->GetString().empty())
	{
		TMessageBox("Veuillez rentrer un pseudo.", BT_OK, form).Show();
		return;
	}

	conf->nick = form->Nick->GetString();

	std::string p = form->ServerList->ReadValue(form->ServerList->GetSelectedItem());
	std::string h = stringtok(p, ":");
	int pp = StrToTyp<int>(p);
	if(pp > 1 && pp < 65535)
	{
		conf->port = pp;
		conf->hostname = h;
	}

	conf->color = form->Color->Value();
	conf->nation = form->Nation->GetSelectedItem();

	conf->save();
	conf->want_quit_config = true;
}

void Config::WantCancel(TObject*, void* configinst)
{
	Config* conf = (Config*)configinst;
	conf->want_quit_config = true;
}

void Config::Configuration(bool first)
{
	TConfigForm*     ConfigForm = new TConfigForm(app.sdlwindow);

	ConfigForm->CancelButton->SetOnClick(Config::WantCancel, this);
	ConfigForm->OkButton->SetOnClick(Config::WantOk, this);
	ConfigForm->Nick->SetString(nick);
	ConfigForm->Color->SetValue(color);
	ConfigForm->Nation->Select(nation);
	if(first)
	{
		ConfigForm->Info->Show();
		ConfigForm->CancelButton->SetEnabled(false);
	}

	bool found = false;
	for(std::vector<std::string>::const_iterator it = server_list.begin(); it != server_list.end(); ++it)
	{
		std::string p = *it;
		std::string h = stringtok(p, ":");

		ConfigForm->ServerList->AddItem((h == hostname && p == TypToStr(port)), *it, *it, black_color, true);
		if(h == hostname && p == TypToStr(port)) found = true;
	}
	if(!found)
		ConfigForm->ServerList->AddItem(true, hostname + ":" + TypToStr(port), hostname, black_color, true);

	want_quit_config = false;
	do
	{
		ConfigForm->Actions();
		ConfigForm->Update();
	} while(!want_quit_config);
}

TConfigForm::TConfigForm(SDL_Surface *w)
	: TForm(w)
{
	Title = AddComponent(new TLabel(300,120,"Configuration", white_color, &app.Font()->big));

	Info = AddComponent(new TLabel(160, 160,"C'est votre premier lancement, veuillez configurer votre jeu.",
	                                        white_color, &app.Font()->normal));
	Info->Hide();

	OkButton = AddComponent(new TButtonText(600,400, 150,50, "OK", &app.Font()->normal));
	CancelButton = AddComponent(new TButtonText(600,450, 150,50, "Annuler", &app.Font()->normal));

	ServerList = AddComponent(new TListBox(&app.Font()->sm, 50, 200, 220, 300));

	NickInfo = AddComponent(new TLabel(300,200,"Pseudo :", white_color, &app.Font()->normal));
	Nick = AddComponent(new TEdit(&app.Font()->sm, 300,225,200, NICKLEN, NICK_CHARS));
	Nick->SetHint("Votre pseudo permettra de vous distinguer des autres joueurs");

	Color = AddComponent(new TColorEdit(&app.Font()->sm, "Couleur par défaut", 300, 250, 200));
	Color->SetHint("Couleur par default dans chaques parties");

	NationInfo = AddComponent(new TLabel(300, 275,"Nation par défaut :", white_color, &app.Font()->normal));
	Nation = AddComponent(new TComboBox(&app.Font()->sm, 300, 300, 200));
	for(uint i = 0; i < ECPlayer::N_MAX; ++i)
	{
		uint j = Nation->AddItem(false, std::string(nations_str[i].name), "");
		Nation->SetItemHint(j, nations_str[i].infos);
	}
		
    Hints = AddComponent(new TMemo(&app.Font()->sm, 550, 200, 200, 100));
    SetHint(Hints);

	SetBackground(Resources::Titlescreen());
}

TConfigForm::~TConfigForm()
{
	delete Hints;
	delete NationInfo;
	delete Nation;
	delete Color;
	delete Nick;
	delete NickInfo;
	delete ServerList;
	delete CancelButton;
	delete OkButton;
	delete Info;
	delete Title;
}