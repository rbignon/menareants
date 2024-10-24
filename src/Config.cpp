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
	hostname = DEF_METASERVER;
	port = 5460;
#ifdef WIN32
	nick = getenv("USERNAME");
#else
	nick = getenv("USER");
#endif
	screen_width = 1024;
	screen_height = 768;
	fullscreen = true;
	color = 0;
	nation = 0;
	server_list.clear();
	server_list.push_back(DEF_METASERVER ":5460");
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
		else if(key == "FULLSCREEN") fullscreen = (ligne == "1" || ligne == "true");
		else if(key == "MUSIC") music = (ligne == "1" || ligne == "true");
		else if(key == "EFFECT") effect = (ligne == "1" || ligne == "true");
		else if(key == "PASSWORD") passwd = ligne;
		else if(key == "TTF" && version <= 2) { /* On conserve la compatibilité */ }
		else
		{
			std::cerr << "Incorrect file: " << std::endl;
			std::cerr << ": " << key << " " << ligne << std::endl;
			MenAreAntsApp::GetInstance()->FirstRun();
			return set_defaults();
		}
	}
	if(hostname.empty() || port < 1 || port > 65535 || color >= COLOR_MAX ||
	   nation >= ECPlayer::N_MAX || nick.empty())
	{
		std::cerr << "Unable to read configuration." << std::endl;
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
		std::cerr << "Unable to create configuration file." << std::endl;
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
	fp << "FULLSCREEN " << fullscreen << std::endl;
	fp << "MUSIC " << music << std::endl;
	fp << "EFFECT " << effect << std::endl;
	if(passwd.empty() == false)
		fp << "PASSWORD " << passwd << std::endl;

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

	TButton*        OkButton;
	TButton*        CancelButton;
	TListBox*       ServerList;
	TEdit*          Nick;
	TColorEdit*     Color;
	TComboBox*      Nation;
	TLabel*         Title;
	TLabel*         Info;
	TLabel*         NickInfo;
	TLabel*         NationInfo;
	TEdit*          NewServer;
	TButton*        AddServerButton;
	TButton*        DelServerButton;
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
		TMessageBox(_("Please enter a server name (in form \"host:port\")."), BT_OK, form).Show();
		return;
	}
	std::string host = stringtok(port, ":");
	if(port.empty())
		port = "5460";
	else if(!is_num(port.c_str()))
	{
		TMessageBox(_("Port is an integer value."), BT_OK, form).Show();
		return;
	}
	host = host + ":" + port;
	conf->server_list.push_back(host);
	form->ServerList->AddItem(true, host, host, white_color, true);
	form->NewServer->ClearString();
	return;
}

void Config::WantDelServer(TObject* OkButton, void* configinst)
{
	TConfigForm* form = static_cast<TConfigForm*>(OkButton->Parent());
	Config* conf = static_cast<Config*>(configinst);

	if(form->ServerList->Selected() == -1)
		return;

	std::string text = form->ServerList->SelectedItem()->Value();
	form->ServerList->RemoveSelected();

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
		TMessageBox(_("Please enter a nickname."), BT_OK, form).Show();
		return;
	}

	if(form->Nick->Text() != conf->nick)
	{
		conf->cookie.clear();
		conf->passwd.clear();
	}

	conf->nick = form->Nick->GetString();

	if(form->ServerList->Selected() == -1)
	{
		TMessageBox(_("Please select a server."), BT_OK, form).Show();
		return;
	}

	std::string p = form->ServerList->SelectedItem()->Value();
	std::string h = stringtok(p, ":");
	int pp = h.empty() ? MSERV_DEFPORT : StrToTyp<int>(p);
	if(pp > 1 && pp < 65535)
	{
		conf->port = pp;
		conf->hostname = h;
	}

	conf->color = form->Color->Value();
	conf->nation = form->Nation->Selected();

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
	int i = listbox->Selected ();
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

		ConfigForm->ServerList->AddItem((h == hostname && p == TypToStr(port)), *it, *it, white_color, true);
		if(h == hostname && p == TypToStr(port)) found = true;
	}
	if(!found)
		ConfigForm->ServerList->AddItem(true, hostname + ":" + TypToStr(port), hostname, white_color, true);

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

	OkButton->SetXY(Window()->GetWidth() - OkButton->Width() - 20,
	                200);
	CancelButton->SetXY(Window()->GetWidth() - OkButton->Width() - 20,
	                    OkButton->Y() + OkButton->Height());

	int x = Window()->GetWidth()/2 - 50;
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
	Title = AddComponent(new TLabel(60,_("Configuration"), white_color, Font::GetInstance(Font::Huge)));

	Info = AddComponent(new TLabel(160,_("This is your first run, please configure your game"),
	                                        white_color, Font::GetInstance(Font::Normal)));
	Info->Hide();

	OkButton = AddComponent(new TButton(600,400, 150,50));
	OkButton->SetImage(new ECSprite(Resources::OkButton(), Video::GetInstance()->Window()));
	CancelButton = AddComponent(new TButton(600,450, 150,50));
	CancelButton->SetImage(new ECSprite(Resources::CancelButton(), Video::GetInstance()->Window()));

	ServerList = AddComponent(new TListBox(Rectanglei(50, 200, 220, 300)));
	ServerList->SetHint(_("Select a meta-server in this list."));

	NewServer = AddComponent(new TEdit(Font::GetInstance(Font::Small), 50,510,208));
	NewServer->SetHint(_("Server to add in form \"host[:port]\""));

	DelServerButton = AddComponent(new TButton(270,417,100,30));
	DelServerButton->SetImage(new ECSprite(Resources::RemoveButton(), Window()));
	DelServerButton->SetHint(_("Delete selected server in list."));
	AddServerButton = AddComponent(new TButton(270,480,100,30));
	AddServerButton->SetImage(new ECSprite(Resources::AddButton(), Window()));
	AddServerButton->SetHint(_("Add a server in this list."));

	NickInfo = AddComponent(new TLabel(300,200,_("Nickname:"), white_color, Font::GetInstance(Font::Normal)));
	Nick = AddComponent(new TEdit(Font::GetInstance(Font::Small), 300,225,200, NICKLEN, NICK_CHARS));
	Nick->SetHint(_("Your nickname is used to recognize each users."));

	Color = AddComponent(new TColorEdit(Font::GetInstance(Font::Small), _("Default color:"), 300, 250, 200));
	Color->SetHint(_("Default color in each games."));

	NationInfo = AddComponent(new TLabel(300, 275, _("Default nation:"), white_color, Font::GetInstance(Font::Normal)));
	Nation = AddComponent(new TComboBox(Font::GetInstance(Font::Small), 300, 300, 200));
	for(uint i = 0; i < ECPlayer::N_MAX; ++i)
		// Pour la visibilité, je précise dans ce commentaire qu'on ajoute un item qui retourne un TListBoxItem,
		// sur lequel on fait un SetHint.
		Nation->AddItem(false, gettext(nations_str[i].name), TypToStr(i))->SetHint(gettext(nations_str[i].infos));

	ResolutionInfo = AddComponent(new TLabel(300, 320, _("Resolution:"), white_color, Font::GetInstance(Font::Normal)));
	Resolution = AddComponent(new TComboBox(Font::GetInstance(Font::Small), 300, 345, 200));
	for(uint i = 0; i < ASIZE(resolutions); ++i)
		Resolution->AddItem(false, TypToStr(resolutions[i].w) + "x" + TypToStr(resolutions[i].h), "");

	FullScreen = AddComponent(new TCheckBox(Font::GetInstance(Font::Normal), 300, 365, _("Fullscreen"), white_color));
	Music = AddComponent(new TCheckBox(Font::GetInstance(Font::Normal), 300, 385, _("Music"), white_color));
	Music->SetHint(_("If this is active, music files are in this directory:\n\n") +
	                std::string(PKGDATADIR_SOUND INGAME_MUSIC));
	Effect = AddComponent(new TCheckBox(Font::GetInstance(Font::Normal), 300, 405, _("Effects"), white_color));

	SetBackground(Resources::Titlescreen());

	SetRelativePositions();
}
