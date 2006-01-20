/* src/JoinGame.cpp - Functions to list game and join/create a game.
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

#include "JoinGame.h"
#include "Main.h"
#include "Resources.h"
#include "Sockets.h"
#include "gui/ListBox.h"
#include "gui/BouttonText.h"
#include "gui/Menu.h"
#include "gui/Memo.h"
#include "gui/MessageBox.h"
#include "gui/Edit.h"
#include "tools/Font.h"
#include "Outils.h"
#include "Debug.h"

TListGameForm  *ListGameForm = NULL;
TGameInfosForm *GameInfosForm = NULL;
bool EOL = false, JOINED = false;

/* <nick> MSG <message> */
int MSGCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(GameInfosForm->Chat)
		GameInfosForm->Chat->AddItem("<" + parv[0] + "> " + parv[1],
	                     strstr(parv[1].c_str(), me->GetNick().c_str()) ? red_color : black_color);
	return 0;
}

/* LSP <nom> <nbjoueur> [nbmax] */
int LSPCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(!ListGameForm) return 0;

	if(parv[3] == "0")
		ListGameForm->GList->AddItem(false, parv[1] + "   " + parv[2], parv[1], black_color, true);
	else
		ListGameForm->GList->AddItem(false, parv[1] + "   " + parv[2] + "/" + parv[3], parv[1],
		                 (parv.size() <= 3 || parv[2] != parv[3]) ? black_color : red_color,
		                 (parv.size() <= 3 || parv[2] != parv[3]) ? true : false);
	return 0;
}

/* EOL */
int EOLCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	EOL = true;
	return 0;
}

/* :%s SET <modes> [params]
 *
 * La reception des modes se fait de manière IRC: chaques "attributs" sont "stockés" dans
 * une lettre et il peut y avoir un attribut mis à la suite.
 */
int SETCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(!me->Player() || !me->Player()->Channel())
		return Debug(W_DESYNCH|W_SEND, "Reception d'un SET sans être dans un chan");

	ECPlayer *sender = 0;
	if(players.size()) sender = players[0];

	bool add = true;
	uint j = 2;
	for(uint i=0; i<parv[1].size(); i++)
	{
		switch(parv[1][i])
		{
			case '+': add = true; break;
			case '-': add = false; break;
			case 'l':
				if(add)
				{
					if(j<parv.size()) me->Player()->Channel()->SetLimite(StrToTyp<uint>(parv[j++]));
					else Debug(W_DESYNCH|W_SEND, "+l sans limite");
				}
				else
					me->Player()->Channel()->SetLimite(0);
				break;
			case 'W': me->Player()->Channel()->SetState(EChannel::WAITING); break;
			case 'S': me->Player()->Channel()->SetState(EChannel::SENDING); break;
			case 'P': me->Player()->Channel()->SetState(EChannel::PLAYING); break;
			case 'A': me->Player()->Channel()->SetState(EChannel::ANIMING); break;
			case '!':
				if(!players.size()) { Debug(W_DESYNCH|W_SEND, "+/-! sans sender"); break; }
				for(PlayerIterator it=players.begin(); it != players.end(); it++)
					(*it)->SetReady(add);
				break;
			case 'o':
			{
				if(j>=parv.size()) { Debug(W_DESYNCH|W_SEND, "+o sans nick"); break; }
				ECPlayer *pl = me->Player()->Channel()->GetPlayer(parv[j++].c_str());
				if(!pl) { Debug(W_DESYNCH|W_SEND, "%s non trouvé", parv[(j-1)].c_str()); break; }
				pl->SetOwner(add);
				break;
			}
			case 'c':
				if(!sender) { Debug(W_DESYNCH|W_SEND, "+c sans sender humain"); break; }
				if(add)
				{
					if(j<parv.size()) sender->SetColor(StrToTyp<uint>(parv[j++]));
					else Debug(W_DESYNCH|W_SEND, "+c sans couleur");
				}
				else
					sender->SetColor(0);
				break;
			case 'p':
				if(!sender) { Debug(W_DESYNCH|W_SEND, "+p sans sender humain"); break; }
				if(add)
				{
					if(j<parv.size()) sender->SetPlace(StrToTyp<uint>(parv[j++]));
					else Debug(W_DESYNCH|W_SEND, "+p sans position");
				}
				else
					sender->SetPlace(0);
				break;
			default:
				Debug(W_DESYNCH|W_SEND, "Reception d'un mode non supporté (%c)", parv[1][i]);
				break;
		}
	}
	return 0;
}

/* PLS [[@]<pos>,<col>,<nick>] [[[@]<pos>,<col>,<nick>] ...]
 *
 * Affiche le nom des joueurs lors du join d'une partie
 */
int PLSCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(!me->Player()) return Debug(W_DESYNCH|W_SEND, "Reception d'un PLS sans être dans un chan");

	JOINED = true;
	for(ParvList::iterator parvi=(parv.begin()+1); parvi!=parv.end(); parvi++)
	{
		const char *nick = (*parvi).c_str();
		bool owner = false, ready = false;

		if(*nick == '@')
			owner = true, *nick++;
		if(*nick == '!')
			ready = true, *nick++;

		std::string nb;
		int pos = 0, col = 0;
		while(*nick != ',')
			nb += *nick++;
		nick++;
		pos = StrToTyp<uint>(nb);

		while(*nick != ',')
			nb += *nick++;
		nick++;
		col = StrToTyp<uint>(nb);

		ECPlayer *pl = 0;
		if(!strcasecmp(nick, me->GetNick().c_str()))
		{ /* On a déjà créé notre player */
			if(owner) me->Player()->SetOwner();
			pl = me->Player();
		}
		else
			pl = new ECPlayer(nick, me->Player()->Channel(), owner, false);
		pl->SetColor(col);
		pl->SetPlace(pos);
		pl->SetReady(ready);
		if(GameInfosForm)
		{
			TPlayerLine *pline = new TPlayerLine(pl);
			GameInfosForm->Players->AddLine(pline);
			if(pl == me->Player())
				GameInfosForm->MyPosition = pline->position;
		}
	}
	if(GameInfosForm)
		GameInfosForm->RecalcMemo();
	return 0;
}

/* <nick> JOI <chan> */
int JOICommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(players.size())
	/* Le joueur est reconnu (!?) */
		return vDebug(W_DESYNCH|W_SEND, "Reception d'un JOIN d'un joueur connu !",
		                                VSName(players[0]->GetNick()));
	else
	{ /* Le joueur est inconnu */
		if(parv[0] == me->GetNick())
		{ /* C'est moi qui join */
			EChannel *c = new EChannel(parv[1]);
			me->SetPlayer(new ECPlayer(parv[0].c_str(), c, false, true));
		}
		else
		{ /* C'est un user qui rejoin le chan */
			if(!me->Player()) return Debug(W_DESYNCH|W_SEND, "Reception d'un join sans être sur un chan");

			ECPlayer *pl = new ECPlayer(parv[0].c_str(), me->Player()->Channel(), false, false);
			if(GameInfosForm)
			{
				GameInfosForm->Chat->AddItem("*** " + parv[0] + " rejoint la partie", green_color);
				GameInfosForm->Players->AddLine(new TPlayerLine(pl));
				GameInfosForm->RecalcMemo();
			}
		}
	}
	return 0;
}

/* <nick> LEA */
int LEACommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(!players.size())
		return vDebug(W_DESYNCH|W_SEND, "Reception d'un LEAVE d'un joueur inconnu !",
		                                VSName(parv[0].c_str()));

	/* Note: Dans le protocole il ne devrait y avoir qu'un seul départ
	 *       à la fois mais bon par respect de ce qui est *possible*
	 *       (on ne sait jamais, après tout, on pourrait imaginer dans
	 *        un avenir des "services" qui pourraient "hacker" (plusieurs
	 *        départs ?) ou alors pour une eventuelle IA du serveur)
	 */
	for(PlayerIterator playersi=players.begin(); playersi != players.end(); playersi++)
	{
		if((*playersi)->IsMe())
		{
			delete me->Player()->Channel();
			me->ClrPlayer(); /* <=> me->pl = 0 */
			return 0;
		}
		else
		{
			if(GameInfosForm)
			{
				GameInfosForm->Chat->AddItem("*** " + std::string((*playersi)->GetNick()) +
				                             " quitte la partie", green_color);
				for(std::vector<TComponent*>::iterator it=GameInfosForm->Players->GetList().begin();
				  it!=GameInfosForm->Players->GetList().end();
				  it++)
				{
					TPlayerLine *pline = ((TPlayerLine*) (*it));
					if(!pline)
					{ /* Il ne devrait PAS être possible que la liste contienne un element nul */
						vDebug(W_ERR|W_SEND, "La liste contient un element vide", VPName(*it));
						continue;
					}
					/* TODO: En revanche, il devrait (et il y aurra) des elements qui ne seront
					 *       *pas* des TPlayerLine. Il faut donc arriver à trouver le type de *it.
					 */
					printf("p - %s\n", pline->Player()->GetNick());
					if(pline->Player() == (*playersi))
					{
						GameInfosForm->Players->RemoveLine(*it);
						GameInfosForm->RecalcMemo();
						break;
					}
				}
			}
			me->Player()->Channel()->RemovePlayer((*playersi), true);
		}
	}
	return 0;
}

bool EuroConqApp::GameInfos(const char *cname)
{
	if(!client)
		throw ECExcept(VPName(client), "Non connecté");

	if(!cname)
	{
		std::string name = Menu::EnterString("Nom", "", false);
		if(name.empty()) return true;

		cname = name.c_str();
	}

	/* Déclaration membres fixes */
	GameInfosForm = new TGameInfosForm;
	GameInfosForm->Chat->AddItem("*** Vous avez bien rejoint le jeu " +
	                                    std::string(cname), green_color);

	JOINED = false;
	client->sendrpl(client->rpl(EC_Client::JOIN), cname);
	WAIT_EVENT(JOINED, i);
	if(!JOINED)
	{
		delete GameInfosForm;
		GameInfosForm = 0;
		return false;
	}

	SDL_Event event;
	bool eob = false;

try
{
	if(!client->Player() || !client->Player()->Channel())
		throw ECExcept(VPName(client->Player()), "Dans aucun chan");
	EChannel *chan = client->Player()->Channel();

	do
	{
		int x=0, y=0;
		while( SDL_PollEvent( &event) )
		{
			switch(event.type)
			{
				case SDL_KEYUP:
					GameInfosForm->SendMessage->PressKey(event.key.keysym);
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							eob = true;
							break;
						case SDLK_RETURN:
							if(GameInfosForm->SendMessage->Focused())
							{
								client->sendrpl(client->rpl(EC_Client::MSG),
								             FormatStr(GameInfosForm->SendMessage->GetString().c_str()));
								GameInfosForm->Chat->AddItem("<" + client->GetNick() + "> " +
								             GameInfosForm->SendMessage->GetString(), black_color);
								GameInfosForm->SendMessage->ClearString();
							}
							break;
						default: break;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					GameInfosForm->Chat->Clic( event.button.x, event.button.y);
					if(GameInfosForm->MyPosition &&
					   GameInfosForm->MyPosition->Clic(event.button.x, event.button.y))
							client->sendrpl(client->rpl(EC_Client::SET),
							       ("+p " + TypToStr(GameInfosForm->MyPosition->Value())).c_str());
					break;
				case SDL_MOUSEBUTTONDOWN:
					GameInfosForm->SendMessage->Clic(event.button.x, event.button.y);
					if(GameInfosForm->RetourButton->Test(event.button.x, event.button.y))
						eob = true;
					if(GameInfosForm->PretButton->Test(event.button.x, event.button.y))
					{
						client->sendrpl(client->rpl(EC_Client::SET), "+!");
						GameInfosForm->PretButton->SetEnabled(false);
					}
					break;
				default:
					break;
			}
		}
		SDL_GetMouseState( &x, &y);

		GameInfosForm->Update(x, y, false);

		big_font.WriteCenter(400,50, "Jeu : " + std::string(chan->GetName()), black_color);
/*		vert += 30;
		for(unsigned int i=0; i<chan->Players().size();i++, vert += 30)
		{
			ECPlayer *pl = ((ECPlayer*)chan->Players()[i]);

			big_font.WriteLeft(50, vert, "OK", pl->Ready() ? red_color : gray_color);
			if(pl->IsOwner()) big_font.WriteLeft(90, vert, "*", red_color);
			big_font.WriteLeft(105, vert, pl->GetNick(), black_color);
			pos.SetXY(200, vert);
			pos.Draw(x, y);
		}
		GameInfosForm->Chat->SetXY(75, vert);
		GameInfosForm->Chat->SetHeight(525-vert); * On définit une jolie taille */

		SDL_Flip(sdlwindow);
	} while(!eob && client->IsConnected() && client->Player());

}
catch(TECExcept &e)
{
	delete GameInfosForm;
	GameInfosForm = NULL;
	throw;
}
	delete GameInfosForm;
	GameInfosForm = NULL;

	client->sendrpl(client->rpl(EC_Client::LEAVE));
	return true;
}

void EuroConqApp::ListGames()
{
	if(!client)
		throw ECExcept(VPName(client), "Non connecté");

	SDL_Event event;

	ListGameForm = new TListGameForm;

	bool eob = false, refresh = true;
	int i;
	do
	{
		if(refresh)
		{
			ListGameForm->GList->ClearItems();
			EOL = false;
			client->sendrpl(client->rpl(EC_Client::LISTGAME));
			WAIT_EVENT(EOL, j);
			if(!EOL)
			{
				delete ListGameForm;
				ListGameForm = 0;
				return; /* On a attendu pour rien */
			}
			refresh = false;
			ListGameForm->JoinButton->SetEnabled(false);
			i=0;
		}
		while( SDL_PollEvent( &event) )
		{
			switch(event.type)
			{
				case SDL_KEYUP:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							eob = true;
							break;
						default: break;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					ListGameForm->GList->Clic( event.button.x, event.button.y);
					if(ListGameForm->JoinButton->Enabled() &&
					   ListGameForm->JoinButton->Test(event.button.x, event.button.y))
					{
						if(!GameInfos(ListGameForm->GList->ReadValue(
						             ListGameForm->GList->GetSelectedItem()).c_str()))
						{
							TMessageBox mb(150,300,
							                  std::string("Impossible de joindre le salon " +
							                  ListGameForm->GList->ReadValue(
							                           ListGameForm->GList->GetSelectedItem())
							                  + ".\nVeuillez reessayer").c_str(), BT_OK, ListGameForm);
							mb.Show();
						}
					}
					else if(ListGameForm->RefreshButton->Test(event.button.x, event.button.y))
						refresh = true;
					else if(ListGameForm->CreerButton->Test(event.button.x, event.button.y))
						GameInfos(NULL);
					else if(ListGameForm->RetourButton->Test(event.button.x, event.button.y))
						eob = true;
					break;
				default:
					break;
			}
		}
		if(ListGameForm->GList->GetSelectedItem() >= 0 &&
		   ListGameForm->GList->Enabled(ListGameForm->GList->GetSelectedItem()))
			ListGameForm->JoinButton->SetEnabled(true);
		ListGameForm->Update();

		//big_font.WriteCenter(400,180, "Liste des parties", black_color);

		if(++i >= 100) refresh = true; /* ~= une minute ? */
	} while(!eob && client->IsConnected());

	delete ListGameForm;
	ListGameForm = NULL;
	return;
}

/********************************************************************************************
 *                               TListGameForm                                              *
 ********************************************************************************************/

TGameInfosForm::TGameInfosForm()
	: TForm()
{
	Chat = AddComponent(new TMemo(75,325,300,200,30));

	SendMessage = AddComponent(new TEdit(75,530,300, MAXBUFFER-20));
	PretButton = AddComponent(new TButtonText(600,400, 100,49, "Pret"));
	RetourButton = AddComponent(new TButtonText(600,450,100,49, "Retour"));

	Players = AddComponent(new TList(50, 30));

	SetBackground(Resources::Titlescreen());

	MyPosition = 0;
}

TGameInfosForm::~TGameInfosForm()
{
	delete Chat;
	delete SendMessage;
	delete RetourButton;
	delete PretButton;
	delete Players;
}

void TGameInfosForm::RecalcMemo()
{
	Chat->SetXY(75, Players->GetHeight()+20);
	Chat->SetHeight(525-Players->GetHeight()-20); /* On définit une jolie taille */
}

/********************************************************************************************
 *                               TListGameForm                                              *
 ********************************************************************************************/

TListGameForm::TListGameForm()
	: TForm()
{
	Title = AddComponent(new TLabel(300,150,"Liste des parties", black_color, &big_font));

	JoinButton = AddComponent(new TButtonText(500,200,100,49, "Joindre"));
	RefreshButton = AddComponent(new TButtonText(500,250,100,49, "Actualiser"));
	CreerButton = AddComponent(new TButtonText(500,300,100,49, "Creer"));
	RetourButton = AddComponent(new TButtonText(500,350,100,49, "Retour"));

	GList = AddComponent(new TListBox(300,200,200,300));

	SetBackground(Resources::Titlescreen());
}

TListGameForm::~TListGameForm()
{
	delete JoinButton;
	delete RefreshButton;
	delete CreerButton;
	delete RetourButton;
	delete GList;
}

/********************************************************************************************
 *                               TPlayerLine                                                *
 ********************************************************************************************/

TPlayerLine::TPlayerLine(ECPlayer *_pl)
{
	pl = _pl;
	h = 30;
	position = 0;
}

TPlayerLine::~TPlayerLine()
{
	if(position) delete position;
}

void TPlayerLine::Init()
{
	if(position) delete position;
	position = new TSpinEdit("", x+155, y, 50, 0, pl->Channel()->NbPlayers(), 1, 0);
	position->Init();
}

void TPlayerLine::Draw(uint souris_x, uint souris_y)
{
	assert(pl);

	if(pl->Place() != (unsigned int)position->Value()) position->SetValue(pl->Place());

	big_font.WriteLeft(x, y, "OK", pl->Ready() ? red_color : gray_color);
	if(pl->IsOwner())
		big_font.WriteLeft(x+40, y, "*", red_color);
	big_font.WriteLeft(x+55, y, pl->GetNick(), black_color);
	position->Draw(souris_x, souris_y);
}
