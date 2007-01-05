/* src/Config.cpp - Configuration
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
 * $Id$
 */

#include <string>
#include <fstream>
#include <iostream>

#include "Channels.h"
#include "Config.h"
#include "Outils.h"
#include "Main.h"
#include "Resources.h"
#include "Sound.h"
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
#include "gui/CheckBox.h"
#include "tools/Video.h"

struct resolutions_t
{
	uint w;
	uint h;
} resolutions[] = {
	{ 800, 600 },
	{ 1024, 768 },
	{ 1152, 864 },
	{ 1280, 1024 },
	{ 1680, 1050 }
};

Config* Config::singleton = NULL;

Config* Config::GetInstance()
{
	if (singleton == NULL)
		singleton = new Config();

	return singleton;
}

Config::Config()
{
	want_quit_config = false;
	set_defaults(false);
}

bool Config::set_defaults(bool want_save)
{
	hostname = "game.coderz.info";
	port = 5460;
#ifdef WIN32
	nick = getenv("USERNAME");
#else
	nick = getenv("USER");
#endif
	color = 0;
	nation = 0;
	screen_width = 1024;
	screen_height = 768;
	ttf_file = PKGDATADIR_FONTS "larabieb.ttf";
#ifdef WIN32
	fullscreen = true;
#else
	fullscreen = false;
#endif
	server_list.clear();
	server_list.push_back("game.coderz.info:5460");
	music = true;
	effect = true;
	if(want_save)
		save();
	return true;
}

bool Config::load()
{
	std::ifstream fp(filename.c_str());
	int version = 0;

	if(!fp)
	{
		MenAreAntsApp::GetInstance()->FirstRun();
		return set_defaults();
	}

	std::string ligne;

	server_list.clear();

	while(std::getline(fp, ligne))
	{
		if(ligne[0] == '#') continue;

		std::string key = stringtok(ligne, " ");

		if(key == "VERSION") version = StrToTyp<int>(ligne);
		else if(key == "SERVER") hostname = ligne;
		else if(key == "PORT" && is_num(ligne.c_str())) port = StrToTyp<int>(ligne);
		else if(key == "COLOR" && is_num(ligne.c_str())) color = StrToTyp<uint>(ligne);
		else if(key == "NATION" && is_num(ligne.c_str())) nation = StrToTyp<uint>(ligne);
		else if(key == "NICK") nick = ligne;
		else if(key == "SWIDTH") screen_width = StrToTyp<uint>(ligne);
		else if(key == "SHEIGHT") screen_height = StrToTyp<uint>(ligne);
		else if(key == "SERVERLIST" && version >= 2) server_list.push_back(ligne);
		else if(key == "TTF" && version >= 2) ttf_file = ligne;
		else if(key == "FULLSCREEN") fullscreen = (ligne == "1" || ligne == "true");
		else if(key == "MUSIC") music = (ligne == "1" || ligne == "true");
		else if(key == "EFFECT") effect = (ligne == "1" || ligne == "true");
		else
		{
			std::cerr << "Fichier incorrect: " << std::endl;
			std::cerr << ": " << key << " " << ligne << std::endl;
			MenAreAntsApp::GetInstance()->FirstRun();
			return set_defaults();
		}
	}
	if(hostname.empty() || ttf_file.empty() || port < 1 || port > 65535 || color >= COLOR_MAX ||
	   nation >= ECPlayer::N_MAX || nick.empty())
	{
		std::cerr << "Lecture de la configuration invalide." << std::endl;
		MenAreAntsApp::GetInstance()->FirstRun();
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

    fp << "VERSION " << CLIENT_CONFVERSION << std::endl;
    fp << "SERVER " << hostname << std::endl;
    fp << "PORT " << port << std::endl;
    fp << "NICK " << nick << std::endl;
    fp << "COLOR " << color << std::endl;
    fp << "NATION " << nation << std::endl;
    fp << "SWIDTH " << screen_width << std::endl;
    fp << "SHEIGHT " << screen_height << std::endl;
    fp << "TTF " << ttf_file << std::endl;
    fp << "FULLSCREEN " << fullscreen << std::endl;
    fp << "MUSIC " << music << std::endl;
    fp << "EFFECT " << effect << std::endl;
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

	TConfigForm(ECImage*);

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
	TEdit*          NewServer;
	TButtonText*    AddServerButton;
	TButtonText*    DelServerButton;
	TLabel*         ResolutionInfo;
	TComboBox*      Resolution;
	TCheckBox*      FullScreen;
	TCheckBox*      Music;
	TCheckBox*      Effect;

/* Evenements */
public:

	void SetRelativePositions();

};

void Config::WantAddServer(TObject* OkButton, void* configinst)
{
	TConfigForm* form = static_cast<TConfigForm*>(OkButton->Parent());
	Config* conf = static_cast<Config*>(configinst);

	std::string port = form->NewServer->Text();
	if(port.empty())
	{
		TMessageBox("Entrez un serveur ! (sous la forme \"host:port\").", BT_OK, form).Show();
		return;
	}
	std::string host = stringtok(port, ":");
	if(port.empty())
		port = "5461";
	else if(!is_num(port.c_str()))
	{
		TMessageBox("Le port doit être une valeur numerique.", BT_OK, form).Show();
		return;
	}
	host = host + ":" + port;
	conf->server_list.push_back(host);
	form->ServerList->AddItem(true, host, host, black_color, true);
	return;
}

void Config::WantDelServer(TObject* OkButton, void* configinst)
{
	TConfigForm* form = static_cast<TConfigForm*>(OkButton->Parent());
	Config* conf = static_cast<Config*>(configinst);

	if(form->ServerList->GetSelectedItem() < 0)
		return;

	std::string text = form->ServerList->ReadValue(form->ServerList->GetSelectedItem());
	form->ServerList->RemoveItem(form->ServerList->GetSelectedItem());

	for(std::vector<std::string>::iterator it = conf->server_list.begin(); it != conf->server_list.end(); ++it)
		if((*it) == text)
		{
			conf->server_list.erase(it);
			break;
		}
}

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

	if(form->ServerList->GetSelectedItem() < 0)
	{
		TMessageBox("Veuillez sélectionner un serveur.", BT_OK, form).Show();
		return;
	}

	std::string p = form->ServerList->ReadValue(form->ServerList->GetSelectedItem());
	std::string h = stringtok(p, ":");
	int pp = h.empty() ? MSERV_DEFPORT : StrToTyp<int>(p);
	if(pp > 1 && pp < 65535)
	{
		conf->port = pp;
		conf->hostname = h;
	}

	conf->color = form->Color->Value();
	conf->nation = form->Nation->GetSelectedItem();

	conf->fullscreen = form->FullScreen->Checked();
	conf->music = form->Music->Checked();
	conf->effect = form->Effect->Checked();

	conf->save();
	conf->want_quit_config = true;
}

void Config::WantCancel(TObject*, void* configinst)
{
	Config* conf = static_cast<Config*>(configinst);
	conf->want_quit_config = true;
}

void Config::SetFullScreen(TObject* obj, void* configinst)
{
	TCheckBox* CheckBox = dynamic_cast<TCheckBox*>(obj);
	Config* conf = static_cast<Config*>(configinst);
	if(!CheckBox) return;

	Video::GetInstance()->SetConfig(conf->screen_width, conf->screen_height, CheckBox->Checked());
	conf->fullscreen = CheckBox->Checked();
}

void Config::SetMusic(TObject* obj, void* configinst)
{
	TCheckBox* CheckBox = dynamic_cast<TCheckBox*>(obj);
	Config* conf = static_cast<Config*>(configinst);
	if(!CheckBox) return;

	conf->music = CheckBox->Checked();
	if(CheckBox->Checked())
		Sound::PlayMusic();
	else
		Sound::StopMusic();
}

void Config::ChangeResolution(TListBox* listbox)
{
	int i = listbox->GetSelectedItem ();
	Config* conf = Config::GetInstance();
	conf->screen_width = resolutions[i].w;
	conf->screen_height = resolutions[i].h;
	Video::GetInstance()->SetConfig(conf->screen_width, conf->screen_height, conf->fullscreen);
	TConfigForm* cform = dynamic_cast<TConfigForm*>(listbox->Parent());
	if(!cform) return;

	cform->SetBackground(Resources::Titlescreen());
	cform->SetRelativePositions();
}

void Config::Configuration(bool first)
{
	TConfigForm*     ConfigForm = new TConfigForm(Video::GetInstance()->Window());

	ConfigForm->CancelButton->SetOnClick(Config::WantCancel, this);
	ConfigForm->OkButton->SetOnClick(Config::WantOk, this);
	ConfigForm->AddServerButton->SetOnClick(Config::WantAddServer, this);
	ConfigForm->DelServerButton->SetOnClick(Config::WantDelServer, this);
	ConfigForm->Nick->SetString(nick);
	ConfigForm->Color->SetValue(color);
	ConfigForm->Nation->Select(nation);
	ConfigForm->Music->Check(music);
	ConfigForm->Music->SetOnClick(Config::SetMusic, this);
	ConfigForm->Effect->Check(effect);
	ConfigForm->FullScreen->Check(fullscreen);
	ConfigForm->FullScreen->SetOnClick(Config::SetFullScreen, this);
	ConfigForm->Resolution->SetOnChange(Config::ChangeResolution);
	for(uint i = 0; i < ASIZE(resolutions); ++i)
		if(screen_width == resolutions[i].w && screen_height == resolutions[i].h)
		{
			ConfigForm->Resolution->Select(i);
			break;
		}
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

	delete ConfigForm;
}

void TConfigForm::SetRelativePositions()
{
	// Va remettre le label automatiquement au milieu
	Title->Init();
	Info->Init();

	Hints->SetX(Window()->GetWidth() - Hints->Width() - 20);
	OkButton->SetXY(Hints->X() + Hints->Width()/2 - OkButton->Width()/2,
	                Hints->Y() + Hints->Height() + 50);
	CancelButton->SetXY(Hints->X() + Hints->Width()/2 - CancelButton->Width()/2,
	                    OkButton->Y() + OkButton->Height());

	int x = Window()->GetWidth()/2 - 100;
	NickInfo->SetX(x);
	Nick->SetX(x);
	Color->SetX(x);
	NationInfo->SetX(x);
	Nation->SetXY(x, Nation->RealY());
	ResolutionInfo->SetX(x);
	Resolution->SetXY(x, Resolution->RealY());
	FullScreen->SetX(x);
	Music->SetX(x);
	Effect->SetX(x);
}

TConfigForm::TConfigForm(ECImage *w)
	: TForm(w)
{
	Title = AddComponent(new TLabel(120,"Configuration", white_color, Font::GetInstance(Font::Big)));

	Info = AddComponent(new TLabel(160,"C'est votre premier lancement, veuillez configurer votre jeu.",
	                                        white_color, Font::GetInstance(Font::Normal)));
	Info->Hide();

	OkButton = AddComponent(new TButtonText(600,400, 150,50, "OK", Font::GetInstance(Font::Normal)));
	CancelButton = AddComponent(new TButtonText(600,450, 150,50, "Annuler", Font::GetInstance(Font::Normal)));

	ServerList = AddComponent(new TListBox(Font::GetInstance(Font::Small), 50, 200, 220, 300));
	ServerList->SetHint("Sélectionnez le serveur auquel vous souhaitez vous connecter.");

	NewServer = AddComponent(new TEdit(Font::GetInstance(Font::Small), 50,510,220));
	NewServer->SetHint("Serveur à ajouter sous la forme \"host[:port]\"");

	DelServerButton = AddComponent(new TButtonText(280,460,100,30, "Supprimer", Font::GetInstance(Font::Small)));
	DelServerButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	DelServerButton->SetHint("Supprimer le serveur sélectionné dans la liste.");
	AddServerButton = AddComponent(new TButtonText(280,500,100,30, "Ajouter", Font::GetInstance(Font::Small)));
	AddServerButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	AddServerButton->SetHint("Ajouter un serveur à la liste.");

	NickInfo = AddComponent(new TLabel(300,200,"Pseudo :", white_color, Font::GetInstance(Font::Normal)));
	Nick = AddComponent(new TEdit(Font::GetInstance(Font::Small), 300,225,200, NICKLEN, NICK_CHARS));
	Nick->SetHint("Votre pseudo permettra de vous distinguer des autres joueurs");

	Color = AddComponent(new TColorEdit(Font::GetInstance(Font::Small), "Couleur par défaut", 300, 250, 200));
	Color->SetHint("Couleur par default dans chaques parties");

	NationInfo = AddComponent(new TLabel(300, 275,"Nation par défaut :", white_color, Font::GetInstance(Font::Normal)));
	Nation = AddComponent(new TComboBox(Font::GetInstance(Font::Small), 300, 300, 200));
	for(uint i = 0; i < ECPlayer::N_MAX; ++i)
	{
		uint j = Nation->AddItem(false, std::string(nations_str[i].name), TypToStr(i));
		Nation->SetItemHint(j, nations_str[i].infos);
	}

	ResolutionInfo = AddComponent(new TLabel(300, 320, "Résolution :", white_color, Font::GetInstance(Font::Normal)));
	Resolution = AddComponent(new TComboBox(Font::GetInstance(Font::Small), 300, 345, 200));
	for(uint i = 0; i < ASIZE(resolutions); ++i)
		Resolution->AddItem(false, TypToStr(resolutions[i].w) + "x" + TypToStr(resolutions[i].h), "");

	FullScreen = AddComponent(new TCheckBox(Font::GetInstance(Font::Normal), 300, 365, "Plein écran", white_color));
	Music = AddComponent(new TCheckBox(Font::GetInstance(Font::Normal), 300, 385, "Musique", white_color));
	Music->SetHint("Si active, les fichiers audio contenus dans le repertoire suivant seront lus pendant le jeu:\n\n"
	                PKGDATADIR_SOUND INGAME_MUSIC);
	Effect = AddComponent(new TCheckBox(Font::GetInstance(Font::Normal), 300, 405, "Effets", white_color));

	Hints = AddComponent(new TMemo(Font::GetInstance(Font::Small), 550, 200, 250, 150));
	SetHint(Hints);

	SetBackground(Resources::Titlescreen());

	SetRelativePositions();
}
