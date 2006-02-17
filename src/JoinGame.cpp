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
#include "Timer.h"
#include "Map.h"

TListGameForm  *ListGameForm = NULL;  /**< Pointer to form whose list games */
TGameInfosForm *GameInfosForm = NULL; /**< Pointer to form whose show game infos */
bool EOL = false;                     /**< EOL is setted to \a true by thread when it received all list of games */
bool JOINED = false;                  /**< JOINED is setted to \a true by thread when it has joined a channel */

/** We receive a map !
 *
 * Syntax: SMAP ligne
 */
int SMAPCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(GameInfosForm)
	{
		EChannel* chan = me->Player()->Channel();
		if(GameInfosForm->RecvMap.empty())
		{
			ECMap *map;
			if((map = chan->Map()))
				MyFree(map);
			chan->SetMap(NULL);
		}
		GameInfosForm->RecvMap.push_back(parv[1]);
	}
	return 0;
}

/** This is end of map.
 *
 * Syntax: EOSMAP
 */
int EOSMAPCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(GameInfosForm)
	{
		if(GameInfosForm->RecvMap.empty())
			Debug(W_DESYNCH|W_SEND, "EOSMAP: Reception d'une map vide !?");
		else
		{
			EChannel *chan = me->Player()->Channel();
			try
			{
				chan->SetMap(new ECMap(GameInfosForm->RecvMap));
			}
			catch(TECExcept &e)
			{
				Debug(W_ERR, "Unable to load a map :");
				vDebug(W_ERR, e.Message, e.Vars);
			}
			GameInfosForm->MapTitle->SetCaption(chan->Map()->Name() + " (" + TypToStr(chan->Map()->MinPlayers()) +
			                                    "-" + TypToStr(chan->Map()->MaxPlayers()) + ")");
			GameInfosForm->RecvMap.clear();
		}
	}
	return 0;
}

/** This is map list.
 *
 * Syntax: LSM nom min max
 */
int LSMCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(GameInfosForm)
	{
		if(!GameInfosForm->RecvMapList)
		{
			GameInfosForm->RecvMapList = true;
			GameInfosForm->MapList->ClearItems();
		}
		GameInfosForm->MapList->AddItem(false, parv[1] + " (" + parv[2] + "-" + parv[3] + ")", parv[1], black_color, true);
	}
	return 0;
}

/** This is end of map list.
 *
 * Syntax: EOMAP
 */
int EOMAPCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(GameInfosForm)
		GameInfosForm->RecvMapList = false;
	return 0;
}

/** We can't rejoin channel.
 * @note this function isn't very usefull...
 *
 * Syntax: ER1
 */
int ER1Command::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	JOINED = false;
	return 0;
}

/** We received a message in channel.
 *
 * Syntax: nick MSG message
 */
int MSGCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(GameInfosForm->Chat)
		GameInfosForm->Chat->AddItem("<" + parv[0] + "> " + parv[1],
	                     strstr(parv[1].c_str(), me->GetNick().c_str()) ? red_color : black_color);
	return 0;
}

/** List games.
 *
 * Syntax: LSP nom nbjoueur [nbmax]
 */
int LSPCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(!ListGameForm)
		vDebug(W_DESYNCH|W_SEND, "Reception d'un LSP hors de la liste des chans", VPName(ListGameForm));

	if(parv[3] == "0")
		ListGameForm->GList->AddItem(false, parv[1] + "   " + parv[2], parv[1], black_color, true);
	else
		ListGameForm->GList->AddItem(false, parv[1] + "   " + parv[2] + "/" + parv[3], parv[1],
		                 (parv.size() <= 3 || parv[2] != parv[3]) ? black_color : red_color,
		                 (parv.size() <= 3 || parv[2] != parv[3]) ? true : false);
	return 0;
}

/** End of channel list
 *
 * Syntax: EOL
 */
int EOLCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	EOL = true;
	return 0;
}

#if 0 /** \todo peut toujours servir! */
static TPlayerLine* GetPlayerLineFromPlayer(ECPlayer* pl)
{
	if(GameInfosForm)
		for(std::vector<TComponent*>::iterator it=GameInfosForm->Players->GetList().begin();
		it!=GameInfosForm->Players->GetList().end();
		it++)
		{
			TPlayerLine *pline = dynamic_cast<TPlayerLine*>(*it);
			if(!pline) /* Ce n'est pas un TPlayerLine */
				continue;
			if(pl == pline->Player())
				return pline;
		}
	return NULL;
}
#endif

/** An user have setted some modes in the channel.
 *
 * Syntax: nicks SET modes [params]
 *
 * Reception of modes looks like IRC: each "attributes" are stocked in a letter and you can put several attributs in
 * the same command.
 */
int SETCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(!me->Player() || !me->Player()->Channel())
		return Debug(W_DESYNCH|W_SEND, "Reception d'un SET sans être dans un chan");

	ECPlayer *sender = 0;
	EChannel *chan = me->Player()->Channel();
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
					if(j<parv.size())
					{
						chan->SetLimite(StrToTyp<uint>(parv[j++]));

						if(GameInfosForm)
						{ /* Redéfini la limite des SpinEdit de chaques joueurs */
							for(std::vector<TComponent*>::iterator it=GameInfosForm->Players->GetList().begin();
							it!=GameInfosForm->Players->GetList().end();
							it++)
							{
								TPlayerLine *pline = dynamic_cast<TPlayerLine*>(*it);
								if(!pline) /* Ce n'est pas un TPlayerLine */
									continue;
								pline->position->SetMax(chan->GetLimite());
							}
						}
					}
					else Debug(W_DESYNCH|W_SEND, "+l sans limite");
				}
				else
					vDebug(W_DESYNCH|W_SEND, "-l interdit !", VSName(parv[0].c_str()) VSName(parv[1].c_str()));
				break;
			case 'W': if(add) chan->SetState(EChannel::WAITING); break;
			case 'S': if(add) chan->SetState(EChannel::SENDING); break;
			case 'P': if(add) chan->SetState(EChannel::PLAYING); break;
			case 'A': if(add) chan->SetState(EChannel::ANIMING); break;
			case 'm':
				if(add)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH|W_SEND, "+m sans numero"); break; }
					if(GameInfosForm)
						GameInfosForm->MapList->Select(StrToTyp<uint>(parv[j++]));
					else
						Debug(W_DESYNCH|W_SEND, "SET +m hors de GameInfosForm !");
				}
				else
					Debug(W_DESYNCH|W_SEND, "SET -m theoriquement impossible");
				break;
			case '!':
				if(!players.size()) { Debug(W_DESYNCH|W_SEND, "+/-! sans sender"); break; }
				for(PlayerList::iterator it=players.begin(); it != players.end(); it++)
					(*it)->SetReady(add);
				break;
			case 'o':
			{
				if(j>=parv.size()) { Debug(W_DESYNCH|W_SEND, "+o sans nick"); break; }
				ECPlayer *pl = chan->GetPlayer(parv[j++].c_str());
				if(!pl) { Debug(W_DESYNCH|W_SEND, "%s non trouvé", parv[(j-1)].c_str()); break; }
				pl->SetOwner(add);
				break;
			}
			case 'c':
				if(!players.size()) { Debug(W_DESYNCH|W_SEND, "+c sans sender humain"); break; }
				if(add)
				{
					if(j<parv.size())
						for(PlayerList::iterator it=players.begin(); it != players.end(); it++)
							(*it)->SetColor(StrToTyp<uint>(parv[j++]));
					else Debug(W_DESYNCH|W_SEND, "+c sans couleur");
				}
				else
					for(PlayerList::iterator it=players.begin(); it != players.end(); it++)
						(*it)->SetColor(0);
				break;
			case 'p':
				if(!players.size()) { Debug(W_DESYNCH|W_SEND, "+p sans sender humain"); break; }
				if(add)
				{
					if(j<parv.size())
						for(PlayerList::iterator it=players.begin(); it != players.end(); it++)
						{
							if(!(*it)->IsMe() && (*it)->Position() > 0 && GameInfosForm->MyPosition)
								GameInfosForm->MyPosition->DelBadValue((*it)->Position());
							(*it)->SetPosition(StrToTyp<uint>(parv[j++]));
							if(!(*it)->IsMe() && (*it)->Position() > 0 && GameInfosForm->MyPosition)
								GameInfosForm->MyPosition->AddBadValue((*it)->Position());
						}
					else Debug(W_DESYNCH|W_SEND, "+p sans position");
				}
				else
					for(PlayerList::iterator it=players.begin(); it != players.end(); it++)
					{
						if(!(*it)->IsMe() && (*it)->Position() > 0 && GameInfosForm->MyPosition)
							GameInfosForm->MyPosition->DelBadValue((*it)->Position());
						(*it)->SetPosition(0);
					}
				break;
			default:
				Debug(W_DESYNCH|W_SEND, "Reception d'un mode non supporté (%c)", parv[1][i]);
				break;
		}
	}
	return 0;
}

/** Show players when you join a game.
 *
 * Syntax : PLS [[@]pos,col,nick] [[[@]pos,col,nick] ...]
 */
int PLSCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(!me->Player()) return Debug(W_DESYNCH|W_SEND, "Reception d'un PLS sans être dans un chan");

	std::vector<int> pos_badval;
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
		pl->SetPosition(pos);
		pl->SetReady(ready);
		if(GameInfosForm)
		{
			TPlayerLine *pline = new TPlayerLine(pl);
			GameInfosForm->Players->AddLine(pline);
			if(pl == me->Player())
				GameInfosForm->MyPosition = pline->position;
			else if(pos > 0) /* GameInfosForm->MyPosition n'est probablement pas encore défini ! */
				pos_badval.push_back(pos);
		}
	}
	if(GameInfosForm)
	{
		 if(GameInfosForm->MyPosition)
		 	for(std::vector<int>::iterator it = pos_badval.begin(); it != pos_badval.end(); it++)
				GameInfosForm->MyPosition->AddBadValue(*it);
		GameInfosForm->RecalcMemo();
	}
	JOINED = true;
	return 0;
}

/** Someone (or me) has joined the channel.
 *
 * Syntax: nick JOI chan
 */
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

/** Someone (or me) has leave the channel.
 *
 * Syntax: nick LEA 
 */
int LEACommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(!players.size())
		return vDebug(W_DESYNCH|W_SEND, "Reception d'un LEAVE d'un joueur inconnu !",
		                                VSName(parv[0].c_str()));

	/** \note Dans le protocole il ne devrait y avoir qu'un seul départ
	 *       à la fois mais bon par respect de ce qui est *possible*
	 *       (on ne sait jamais, après tout, on pourrait imaginer dans
	 *        un avenir des "services" qui pourraient "hacker" (plusieurs
	 *        départs ?) ou alors pour une eventuelle IA du serveur)
	 */
	for(PlayerList::iterator playersi=players.begin(); playersi != players.end();)
	{
		if((*playersi)->IsMe())
		{
			delete me->Player()->Channel();
			me->ClrPlayer(); /* <=> me->pl = 0 */
			JOINED = false;
			return 0;
		}
		else
		{
			if(GameInfosForm)
			{
				GameInfosForm->Players->Hide();
				GameInfosForm->Chat->AddItem("*** " + std::string((*playersi)->GetNick()) +
				                             " quitte la partie", green_color);
				for(std::vector<TComponent*>::iterator it=GameInfosForm->Players->GetList().begin();
				  it!=GameInfosForm->Players->GetList().end();
				  it++)
				{
					TPlayerLine *pline = dynamic_cast<TPlayerLine*>(*it);
					if(!pline) /* Ce n'est pas un TPlayerLine */
						continue;

					if(pline->Player() == (*playersi))
					{
						if(!GameInfosForm->Players->RemoveLine(*it))
							throw ECExcept(VPName(*it) VPName(*playersi), "Player-line introuvable");
						GameInfosForm->RecalcMemo();
						break;
					}
				}
				GameInfosForm->Players->Show();
			}
			if(me->Player()->Channel()->RemovePlayer((*playersi), true))
				playersi = players.erase(playersi);
			else
			{
				vDebug(W_DESYNCH|W_SEND, "LEA - Impossible de supprimer un player", VPName(*playersi));
				playersi++;
			}
		}
	}
	return 0;
}

bool EuroConqApp::GameInfos(const char *cname)
{
	if(!client)
		throw ECExcept(VPName(client), "Non connecté");
		
	std::string name;

	if(!cname)
	{
		name = Menu::EnterString("Nom", "", false);
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
		MyFree(GameInfosForm);
		client->sendrpl(client->rpl(EC_Client::LEAVE));
		return false;
	}

	SDL_Event event;
	bool eob = false;

	try
	{
		if(!client->Player() || !client->Player()->Channel())
			throw ECExcept(VPName(client->Player()), "Dans aucun chan");
		EChannel *chan = client->Player()->Channel();

		GameInfosForm->Title->SetCaption("Jeu : " + std::string(chan->GetName()));

		GameInfosForm->MapList->SetEnabled(client->Player()->IsOwner());
	
		do
		{
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
					case SDL_MOUSEBUTTONDOWN:
						GameInfosForm->Chat->Clic( event.button.x, event.button.y);
						if(client->Player()->IsOwner())
						{
							GameInfosForm->MapList->SetEnabled();
							if(GameInfosForm->MapList->Clic( event.button.x, event.button.y))
								client->sendrpl(client->rpl(EC_Client::SET),
								                ("+m " + TypToStr(GameInfosForm->MapList->GetSelectedItem())).c_str());
							
						}
						else
							GameInfosForm->MapList->SetEnabled(false);
						if(GameInfosForm->MyPosition &&
						GameInfosForm->MyPosition->Clic(event.button.x, event.button.y))
								client->sendrpl(client->rpl(EC_Client::SET),
									("+p " + TypToStr(GameInfosForm->MyPosition->Value())).c_str());
/*						break;
					case SDL_MOUSEBUTTONDOWN:*/
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
			GameInfosForm->Update();
	
			big_font.WriteCenter(400,50, "Jeu : " + std::string(chan->GetName()), black_color);
		} while(!eob && client->IsConnected() && client->Player());
	
	}
	catch(TECExcept &e)
	{
		MyFree(GameInfosForm);
		client->sendrpl(client->rpl(EC_Client::LEAVE));
		throw;
	}
	MyFree(GameInfosForm);

	client->sendrpl(client->rpl(EC_Client::LEAVE));
	
	WAIT_EVENT(!JOINED, i);
	
	return true;
}

void EuroConqApp::ListGames()
{
	if(!client)
		throw ECExcept(VPName(client), "Non connecté");

	SDL_Event event;

	ListGameForm = new TListGameForm;

	bool eob = false, refresh = true;
	Timer timer; /* Utilisation d'un timer pour compter une véritable minute */
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
			timer.reset();
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

		if(timer.time_elapsed(true) > 60) refresh = true; /* VÉRITABLE minute */
	} while(!eob && client->IsConnected());

	MyFree(ListGameForm);
	return;
}

/********************************************************************************************
 *                               TListGameForm                                              *
 ********************************************************************************************/

TGameInfosForm::TGameInfosForm()
	: TForm()
{
	Title = AddComponent(new TLabel(400,50,"Jeu", black_color, &big_font));

	Chat = AddComponent(new TMemo(60,325,315,200,30));

	SendMessage = AddComponent(new TEdit(60,530,315, MAXBUFFER-20));
	PretButton = AddComponent(new TButtonText(600,400, 100,49, "Pret"));
	RetourButton = AddComponent(new TButtonText(600,450,100,49, "Retour"));

	Players = AddComponent(new TList(60, 80));
	Players->AddLine(new TPlayerLineHeader);

	MapList = AddComponent(new TListBox(600, 100, 150, 200));
	MapTitle = AddComponent(new TLabel(550, 300, "", black_color, &big_font));

	SetBackground(Resources::Menuscreen());

	MyPosition = 0;
	RecvMapList = false;
}

TGameInfosForm::~TGameInfosForm()
{
	delete MapTitle;
	delete MapList;
	delete Players;
	delete RetourButton;
	delete PretButton;
	delete SendMessage;
	delete Chat;
	delete Title;
}

void TGameInfosForm::RecalcMemo()
{
	Chat->SetXY(60, Players->GetY() + Players->GetHeight());
	Chat->SetHeight(505-Players->GetHeight()-Players->GetX()); /* On définit une jolie taille */
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

	SetBackground(Resources::Menuscreen());
}

TListGameForm::~TListGameForm()
{
	delete GList;
	delete RetourButton;
	delete CreerButton;
	delete RefreshButton;
	delete JoinButton;
	delete Title;
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

void TPlayerLine::SetXY (uint px, uint py)
{
	TComponent::SetXY(px, py);
	if(position)
		position->SetXY(px+210, py);
}

void TPlayerLine::Init()
{
	assert(pl);
	if(position) delete position;
	                    /*  label   x    y  w  min      max                  step  defvalue */
	position = new TSpinEdit("",  x+210, y, 50, 0, pl->Channel()->GetLimite(), 1,    0);
	position->Init();
}

void TPlayerLine::Draw(uint souris_x, uint souris_y)
{
	assert(pl);

	if(pl->Position() != (unsigned int)position->Value()) position->SetValue(pl->Position());

	big_font.WriteLeft(x, y, "OK", pl->Ready() ? red_color : gray_color);
	if(pl->IsOwner())
		big_font.WriteLeft(x+50, y, "*", red_color);
	big_font.WriteLeft(x+65, y, pl->GetNick(), black_color);
	position->Draw(souris_x, souris_y);
}

/********************************************************************************************
 *                               TPlayerLineHeader                                          *
 ********************************************************************************************/

TPlayerLineHeader::TPlayerLineHeader()
{
	h = 30;
	label = 0;
}

void TPlayerLineHeader::Init()
{
	if(label)
		delete label;

	std::string s = "Pret  Pseudo           Position";
	label = new TLabel(x, y, s, black_color, &big_font);
}

void TPlayerLineHeader::SetXY (uint px, uint py)
{
	TComponent::SetXY(px, py);
	if(label)
		label->SetXY(px, py);
}

TPlayerLineHeader::~TPlayerLineHeader()
{
	if(label)
		delete label;
}

void TPlayerLineHeader::Draw(uint souris_x, uint souris_y)
{
	label->Draw(souris_x, souris_y);
}
