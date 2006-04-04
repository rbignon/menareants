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
#include "lib/Colors.h"
#include "gui/Form.h"
#include "gui/ListBox.h"
#include "gui/Label.h"
#include "gui/BouttonText.h"
#include "gui/Edit.h"
#include "gui/ColorEdit.h"

Config::Config(std::string _filename)
{
	filename = _filename;
	want_quit_config = false;
}

bool Config::set_defaults()
{
	hostname = "127.0.0.1";
	port = 5461;
	nick = "anonyme";
	color = 0;
	server_list.push_back("127.0.0.1:5461");
	server_list.push_back("game.coderz.info:5461");
	save();
	return true;
}

bool Config::load()
{
	std::ifstream fp(filename.c_str());

	if(!fp)
		return set_defaults();

	std::string ligne;

	server_list.clear();

	while(std::getline(fp, ligne))
	{
		std::string key = stringtok(ligne, " ");

		if(key == "SERVER") hostname = ligne;
		else if(key == "PORT" && is_num(ligne.c_str())) port = atoi(ligne.c_str());
		else if(key == "COLOR" && is_num(ligne.c_str())) color = atoi(ligne.c_str());
		else if(key == "NICK") nick = ligne;
		else if(key == "SERVERLIST") server_list.push_back(ligne);
		else
		{
			std::cout << "Fichier incorrect" << std::endl;
			return set_defaults();
		}
	}
	if(hostname.empty() || port < 1 || port > 65535 || color > COLOR_MAX)
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

	TConfigForm();
	~TConfigForm();

/* Composants */
public:

	TButtonText*    OkButton;
	TButtonText*    CancelButton;
	TListBox*       ServerList;
	TEdit*          Nick;
	TColorEdit*     Color;
	TLabel*         Title;
	TLabel*         NickInfo;

/* Evenements */
public:

};

void Config::WantOk(void* forminst, void* configinst)
{
	TConfigForm* form = static_cast<TConfigForm*>(forminst);
	Config* conf = static_cast<Config*>(configinst);

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

	conf->save();
	conf->want_quit_config = true;
}

void Config::WantCancel(void*, void* configinst)
{
	Config* conf = (Config*)configinst;
	conf->want_quit_config = true;
}

void Config::Configuration()
{
	TConfigForm*     ConfigForm = new TConfigForm;

	ConfigForm->CancelButton->SetClickedFunc(Config::WantCancel, this);
	ConfigForm->OkButton->SetClickedFunc(Config::WantOk, this);
	ConfigForm->Nick->SetString(nick);
	ConfigForm->Color->SetValue(color);

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

TConfigForm::TConfigForm()
	: TForm()
{
	Title = AddComponent(new TLabel(300,120,"Configuration", white_color, &app.Font()->big));

	OkButton = AddComponent(new TButtonText(600,400, 150,50, "OK"));
	CancelButton = AddComponent(new TButtonText(600,450, 150,50, "Annuler"));

	ServerList = AddComponent(new TListBox(50, 200, 220, 300));

	NickInfo = AddComponent(new TLabel(300,200,"Pseudo :", white_color, &app.Font()->normal));
	Nick = AddComponent(new TEdit(300,225,200, NICKLEN));

	Color = AddComponent(new TColorEdit("Couleur par default", 300, 250, 200));
	
	SetBackground(Resources::Titlescreen());
}

TConfigForm::~TConfigForm()
{
	delete Color;
	delete Nick;
	delete NickInfo;
	delete ServerList;
	delete CancelButton;
	delete OkButton;
	delete Title;
}
