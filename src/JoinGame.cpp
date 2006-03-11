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

#include "InGame.h"
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

extern TLoadingForm   *LoadingForm;
void LoadingGame(EC_Client* cl);

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
			if((map = dynamic_cast<ECMap*>(chan->Map())))
			{
				GameInfosForm->Preview->SetImage(NULL);
				MyFree(map);
			}
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
				vDebug(W_ERR|W_SEND, e.Message, e.Vars);
				return 0;
			}
			GameInfosForm->MapTitle->SetCaption(chan->Map()->Name() + " (" + TypToStr(chan->Map()->MinPlayers()) +
			                                    "-" + TypToStr(chan->Map()->MaxPlayers()) + ")");
			GameInfosForm->RecvMap.clear();
			GameInfosForm->Preview->SetImage(chan->Map()->Preview(), false);
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
		vDebug(W_DESYNCH|W_SEND, "Reception d'un LSP hors de la fen�tre de liste des chans", VPName(ListGameForm));

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
		++it)
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
		return Debug(W_DESYNCH|W_SEND, "Reception d'un SET sans �tre dans un chan");

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
						{ /* Red�fini la limite des SpinEdit de chaques joueurs */
							for(std::vector<TComponent*>::iterator it=GameInfosForm->Players->GetList().begin();
							it!=GameInfosForm->Players->GetList().end();
							++it)
							{
								TPlayerLine *pline = dynamic_cast<TPlayerLine*>(*it);
								if(!pline) /* Ce n'est pas un TPlayerLine */
									continue;
								pline->position->SetMax(chan->GetLimite());
							}
						}
					}
					else Debug(W_DESYNCH|W_SEND, "SET +l: sans limite");
				}
				else
					vDebug(W_DESYNCH|W_SEND, "SET -l: interdit !", VSName(parv[0].c_str()) VSName(parv[1].c_str()));
				break;
			case 'W': if(add) chan->SetState(EChannel::WAITING); break;
			case 'S':
				if(add)
				{
					chan->SetState(EChannel::SENDING);
					LoadingGame(me);
				}
				break;
			case 'P':
				if(add)
				{
					/* On incr�mente la date si ce n'est pas justement le premier jour */
					if(!chan->State() == EChannel::SENDING)
						chan->Map()->NextDay();
					chan->SetState(EChannel::PLAYING);
				}
				break;
			case 'A': if(add) chan->SetState(EChannel::ANIMING); break;
			case 'm':
				if(add)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH|W_SEND, "SET +m: sans numero"); break; }
					if(GameInfosForm)
					{
						GameInfosForm->MapList->Select(StrToTyp<uint>(parv[j++]));
						GameInfosForm->MyPosition->SetEnabled();
						GameInfosForm->MyColor->SetEnabled();
					}
					else
						Debug(W_DESYNCH|W_SEND, "SET +m: hors de GameInfosForm !");
				}
				else
					Debug(W_DESYNCH|W_SEND, "SET -m: theoriquement impossible");
				break;
			case '!':
				if(!players.size()) { Debug(W_DESYNCH|W_SEND, "SET %c!: sans sender", add ? '+' : '-'); break; }
				for(PlayerList::iterator it=players.begin(); it != players.end(); ++it)
				{
					(*it)->SetReady(add);
					if((*it)->IsMe() && GameInfosForm)
						GameInfosForm->PretButton->SetEnabled(!add);
				}
				break;
			case '$':
				if(j>=parv.size()) Debug(W_DESYNCH|W_SEND, "SET %c$: sans argent", add ? '+' : '-');
				else me->Player()->SetMoney(StrToTyp<int>(parv[j++]));
				break;
			case 'o':
			{
				if(j>=parv.size()) { Debug(W_DESYNCH|W_SEND, "SET %co: sans nick", add ? '+' : '-'); break; }
				ECPlayer *pl = chan->GetPlayer(parv[j++].c_str());
				if(!pl)
				{
					Debug(W_DESYNCH|W_SEND, "SET %co: %s non trouv�", add ? '+' : '-', parv[(j-1)].c_str());
					break;
				}
				pl->SetOp(add);
				break;
			}
			case 'c':
				if(!players.size()) { Debug(W_DESYNCH|W_SEND, "SET %cc: sans sender humain", add ? '+' : '-'); break; }
				if(add)
				{
					if(j<parv.size())
						for(PlayerList::iterator it=players.begin(); it != players.end(); ++it)
						{
							if(!(*it)->IsMe() && (*it)->Position() > 0 && GameInfosForm->MyPosition)
								GameInfosForm->MyColor->DelBadValue((*it)->Color());
							if(!(*it)->SetColor(StrToTyp<uint>(parv[j++])))
								Debug(W_DESYNCH|W_SEND, "SET +c: couleur trop grande !?");
							else
							{
								if(!(*it)->IsMe() && (*it)->Color() > 0 && GameInfosForm->MyColor)
									GameInfosForm->MyColor->AddBadValue((*it)->Color());
								if((*it)->IsMe() && GameInfosForm)
									GameInfosForm->PretButton->SetEnabled((*it)->Position() && (*it)->Color());
							}
						}
					else Debug(W_DESYNCH|W_SEND, "SET +c: sans couleur");
				}
				else
					for(PlayerList::iterator it=players.begin(); it != players.end(); ++it)
					{
						(*it)->SetColor(0);
						if((*it)->IsMe() && GameInfosForm)
							GameInfosForm->PretButton->SetEnabled(false);
					}
				break;
			case 'p':
				if(!players.size()) { Debug(W_DESYNCH|W_SEND, "SET %cp: sans sender humain", add ? '+' : '-'); break; }
				if(add)
				{
					if(j<parv.size())
						for(PlayerList::iterator it=players.begin(); it != players.end(); ++it)
						{
							if(!(*it)->IsMe() && (*it)->Position() > 0 && GameInfosForm->MyPosition)
								GameInfosForm->MyPosition->DelBadValue((*it)->Position());
							if(!(*it)->SetPosition(StrToTyp<uint>(parv[j++])))
								Debug(W_DESYNCH|W_SEND, "SET +p: position hors limite");
							else
							{
								if(!(*it)->IsMe() && (*it)->Position() > 0 && GameInfosForm->MyPosition)
									GameInfosForm->MyPosition->AddBadValue((*it)->Position());
								if((*it)->IsMe() && GameInfosForm)
									GameInfosForm->PretButton->SetEnabled((*it)->Position() && (*it)->Color());
							}
						}
					else Debug(W_DESYNCH|W_SEND, "SET +p: sans position");
				}
				else
					for(PlayerList::iterator it=players.begin(); it != players.end(); ++it)
					{
						if(!(*it)->IsMe() && (*it)->Position() > 0 && GameInfosForm->MyPosition)
							GameInfosForm->MyPosition->DelBadValue((*it)->Position());
						(*it)->SetPosition(0);
						if((*it)->IsMe() && GameInfosForm)
							GameInfosForm->PretButton->SetEnabled(false);
					}
				break;
			default:
				Debug(W_DESYNCH|W_SEND, "SET %c%c: Reception d'un mode non support�", add ? '+' : '-', parv[1][i]);
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
	if(!me->Player()) return Debug(W_DESYNCH|W_SEND, "Reception d'un PLS sans �tre dans un chan");

	std::vector<int> pos_badval;
	std::vector<int> col_badval;
	for(ParvList::iterator parvi=(parv.begin()+1); parvi!=parv.end(); ++parvi)
	{
		const char *nick = (*parvi).c_str();
		bool owner = false, ready = false, op = false;

		if(*nick == '*')
			owner = true, *nick++;
		if(*nick == '@')
			op = true, *nick++;
		if(*nick == '!')
			ready = true, *nick++;

		std::string nb;
		int pos = 0, col = 0;
		while(*nick != ',')
			nb += *nick++;
		nick++;
		pos = StrToTyp<uint>(nb);

		nb.clear();
		while(*nick != ',')
			nb += *nick++;
		nick++;
		col = StrToTyp<uint>(nb);

		ECPlayer *pl = 0;
		if(!strcasecmp(nick, me->GetNick().c_str()))
		{ /* On a d�j� cr�� notre player */
			if(owner) me->Player()->SetOwner();
			if(op) me->Player()->SetOp();
			pl = me->Player();
		}
		else
			pl = new ECPlayer(nick, me->Player()->Channel(), owner, op, false);
		pl->SetColor(col);
		pl->SetPosition(pos);
		pl->SetReady(ready);
		if(GameInfosForm)
		{
			TPlayerLine *pline = new TPlayerLine(pl);
			GameInfosForm->Players->AddLine(pline);
			if(pl == me->Player())
			{
				GameInfosForm->MyPosition = pline->position;
				GameInfosForm->MyPosition->SetEnabled(false);
				GameInfosForm->MyColor = pline->couleur;
				GameInfosForm->MyColor->SetEnabled(false);
			}
			else
			{ /* GameInfosForm->MyPosition et GameInfosForm->MyColor ne sont probablement pas encore d�fini ! */
				if(pos > 0) 
					pos_badval.push_back(pos);
				if(col > 0)
					col_badval.push_back(col);
			}
		}
	}
	if(GameInfosForm)
	{
		if(GameInfosForm->MyPosition)
		{
			for(std::vector<int>::iterator it = pos_badval.begin(); it != pos_badval.end(); ++it)
				GameInfosForm->MyPosition->AddBadValue(*it);
			if(me->Player()->Channel()->Map())
				GameInfosForm->MyPosition->SetEnabled();
		}
		if(GameInfosForm->MyColor)
		{
			for(std::vector<int>::iterator it = col_badval.begin(); it != col_badval.end(); ++it)
				GameInfosForm->MyColor->AddBadValue(*it);
			if(me->Player()->Channel()->Map())
				GameInfosForm->MyColor->SetEnabled();
		}
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
	if(!players.empty())
	/* Le joueur est reconnu (!?) */
		return vDebug(W_DESYNCH|W_SEND, "Reception d'un JOIN d'un joueur connu !",
		                                VSName(players[0]->GetNick()));
	else
	{ /* Le joueur est inconnu */
		if(parv[0] == me->GetNick())
		{ /* C'est moi qui join */
			EChannel *c = new EChannel(parv[1]);
			me->SetPlayer(new ECPlayer(parv[0].c_str(), c, false, false, true));
		}
		else
		{ /* C'est un user qui rejoin le chan */
			if(!me->Player()) return Debug(W_DESYNCH|W_SEND, "Reception d'un join sans �tre sur un chan");

			ECPlayer *pl = new ECPlayer(parv[0].c_str(), me->Player()->Channel(), false, false, false);
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
	if(players.empty())
		return vDebug(W_DESYNCH|W_SEND, "Reception d'un LEAVE d'un joueur inconnu !",
		                                VSName(parv[0].c_str()));

	/** \note Dans le protocole il ne devrait y avoir qu'un seul d�part
	 *       � la fois mais bon par respect de ce qui est *possible*
	 *       (on ne sait jamais, apr�s tout, on pourrait imaginer dans
	 *        un avenir des "services" qui pourraient "hacker" (plusieurs
	 *        d�parts ?) ou alors pour une eventuelle IA du serveur)
	 */
	for(PlayerList::iterator playersi=players.begin(); playersi != players.end();)
	{
		if((*playersi)->IsMe())
		{
			EChannel *c = me->Player()->Channel();
			me->ClrPlayer(); /* <=> me->pl = 0 */
			if(GameInfosForm)
				GameInfosForm->Players->Hide();
			delete c;
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
				  ++it)
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
		throw ECExcept(VPName(client), "Non connect�");
		
	std::string name;
	bool create = false;

	if(!cname)
	{
		create = true;
		name = Menu::EnterString("Nom", "", false);
		if(name.empty()) return true;

		cname = name.c_str();
	}

	/* D�claration membres fixes */
	GameInfosForm = new TGameInfosForm;
	GameInfosForm->Chat->AddItem("*** Vous avez bien rejoint le jeu " +
	                                    std::string(cname), green_color);

	JOINED = false;
	client->sendrpl(create ? client->rpl(EC_Client::CREATE) : client->rpl(EC_Client::JOIN), FormatStr(cname));

	WAIT_EVENT_T(JOINED, i, 0.3);
	if(!JOINED)
	{
		MyFree(GameInfosForm);
		if(client->Player())
			client->sendrpl(client->rpl(EC_Client::LEAVE));
		return false;
	}

	SDL_Event event;
	bool eob = false;
	EChannel *chan = NULL;

	try
	{
		if(!client->Player() || !client->Player()->Channel())
			throw ECExcept(VPName(client->Player()), "Dans aucun chan");
		chan = client->Player()->Channel();

		GameInfosForm->Title->SetCaption("Jeu : " + std::string(chan->GetName()));

		GameInfosForm->MapList->SetEnabled(client->Player()->IsOp());
	
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
						if(client->Player()->IsPriv())
						{
							GameInfosForm->MapList->SetEnabled();
							if(GameInfosForm->MapList->Clic( event.button.x, event.button.y))
								client->sendrpl(client->rpl(EC_Client::SET),
								                ("+m " + TypToStr(GameInfosForm->MapList->GetSelectedItem())).c_str());

							if(client->Player()->IsOwner())
								for(std::vector<TComponent*>::iterator it=GameInfosForm->Players->GetList().begin();
								it!=GameInfosForm->Players->GetList().end();
								++it)
								{
									TPlayerLine* pll = dynamic_cast<TPlayerLine*>(*it);
									if(pll && pll->OwnZone(event.button.x, event.button.y) && !pll->Player()->IsMe() &&
									   !pll->Player()->IsOwner())
									{
										if(pll->Player()->IsOp())
											client->sendrpl(client->rpl(EC_Client::SET),
														("-o " + std::string(pll->Player()->GetNick())).c_str());
										else
											client->sendrpl(client->rpl(EC_Client::SET),
														("+o " + std::string(pll->Player()->GetNick())).c_str());
									}
								}
						}
						else
							GameInfosForm->MapList->SetEnabled(false);
						if(GameInfosForm->MyPosition &&
						GameInfosForm->MyPosition->Clic(event.button.x, event.button.y))
								client->sendrpl(client->rpl(EC_Client::SET),
									("+p " + TypToStr(GameInfosForm->MyPosition->Value())).c_str());
						if(GameInfosForm->MyColor &&
						GameInfosForm->MyColor->Clic(event.button.x, event.button.y))
								client->sendrpl(client->rpl(EC_Client::SET),
									("+c " + TypToStr(GameInfosForm->MyColor->Value())).c_str());
/*						break;
					case SDL_MOUSEBUTTONDOWN:*/
						GameInfosForm->SendMessage->Clic(event.button.x, event.button.y);
						if(GameInfosForm->RetourButton->Test(event.button.x, event.button.y))
							eob = true;
						if(GameInfosForm->PretButton->Test(event.button.x, event.button.y))
							client->sendrpl(client->rpl(EC_Client::SET), "+!");
						break;
					default:
						break;
				}
			}
			GameInfosForm->Update();
		} while(!eob && client->IsConnected() && client->Player() &&
		        client->Player()->Channel()->State() != EChannel::SENDING);
	
	}
	catch(TECExcept &e)
	{
		MyFree(GameInfosForm);
		if(client && client->Player())
			client->sendrpl(client->rpl(EC_Client::LEAVE));
		throw;
	}
	MyFree(GameInfosForm);

	if(client->Player()->Channel()->State() == EChannel::SENDING)
	{ /* LA PARTIE SE LANCE */
		LoadGame(chan);
	}

	if(client->Player())
		client->sendrpl(client->rpl(EC_Client::LEAVE));
	
	WAIT_EVENT(!JOINED, i);
	
	return true;
}

void EuroConqApp::ListGames()
{
	if(!client)
		throw ECExcept(VPName(client), "Non connect�");

	SDL_Event event;

	ListGameForm = new TListGameForm;

	bool eob = false, refresh = true;
	Timer timer; /* Utilisation d'un timer pour compter une v�ritable minute */
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
							TMessageBox mb(250,300,
							                  std::string("Impossible de joindre le salon " +
							                  ListGameForm->GList->ReadValue(
							                           ListGameForm->GList->GetSelectedItem())
							                  + ".\nVeuillez reessayer").c_str(), BT_OK, ListGameForm);
							mb.Show();
						}
						refresh = true;
						timer.reset();
					}
					else if(ListGameForm->RefreshButton->Test(event.button.x, event.button.y))
						refresh = true;
					else if(ListGameForm->CreerButton->Test(event.button.x, event.button.y))
					{
						if(!GameInfos(NULL))
						{
							TMessageBox mb(250,300,
												std::string("Impossible de cr�er le salon.\n"
												"Son nom est peut �tre d�j� utilis�.").c_str(),
												BT_OK, ListGameForm);
							mb.Show();
						}
						refresh = true;
						timer.reset();
					}
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

		if(timer.time_elapsed(true) > 60) refresh = true; /* V�RITABLE minute */
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
	Title = AddComponent(new TLabel(400,50,"Jeu", black_color, &app.Font()->big));

	Chat = AddComponent(new TMemo(60,325,315,200,30));

	SendMessage = AddComponent(new TEdit(60,530,315, MAXBUFFER-20));
	PretButton = AddComponent(new TButtonText(600,400, 100,49, "Pret"));
	PretButton->SetEnabled(false);

	RetourButton = AddComponent(new TButtonText(600,450,100,49, "Retour"));

	Players = AddComponent(new TList(60, 80));
	Players->AddLine(new TPlayerLineHeader);

	MapList = AddComponent(new TListBox(400, 320, 150, 225));

	MapTitle = AddComponent(new TLabel(550, 80, "", black_color, &app.Font()->big));
	Preview = AddComponent(new TImage(550, 110));

	SetBackground(Resources::Menuscreen());

	MyPosition = 0;
	RecvMapList = false;
}

TGameInfosForm::~TGameInfosForm()
{
	delete Preview;
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
	Chat->SetHeight(505-Players->GetHeight()-Players->GetX()); /* On d�finit une jolie taille */
}

/********************************************************************************************
 *                               TListGameForm                                              *
 ********************************************************************************************/

TListGameForm::TListGameForm()
	: TForm()
{
	Title = AddComponent(new TLabel(300,150,"Liste des parties", black_color, &app.Font()->big));

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
	couleur = 0;
}

TPlayerLine::~TPlayerLine()
{
	if(position) delete position;
	if(couleur) delete couleur;
}

void TPlayerLine::SetXY (uint px, uint py)
{
	TComponent::SetXY(px, py);
	if(position)
		position->SetXY(px+210, py);
	if(couleur)
		couleur->SetXY(px+310, py);
}

bool TPlayerLine::OwnZone(uint _x, uint _y)
{
	if(_x > x+45 && _x < x+70 && _y > y && _y < y+h)
		return true;
	else
		return false;
}

void TPlayerLine::Init()
{
	assert(pl);
	if(position) delete position;
	                    /*  label   x    y  w  min      max                  step  defvalue */
	position = new TSpinEdit("",  x+210, y, 50, 0, pl->Channel()->GetLimite(), 1,    0);
	position->Init();
	couleur = new TColorEdit("",  x+340, y, 50);
	couleur->Init();
}

void TPlayerLine::Draw(uint souris_x, uint souris_y)
{
	assert(pl);

	if(pl->Position() != (unsigned int)position->Value()) position->SetValue(pl->Position());
	if(pl->Color() != (unsigned int)couleur->Value()) couleur->SetValue(pl->Color());

	app.Font()->big.WriteLeft(x, y, "OK", pl->Ready() ? red_color : gray_color);
	if(pl->IsOwner())
		app.Font()->big.WriteLeft(x+55, y, "*", red_color);
	else if(pl->IsOp())
		app.Font()->big.WriteLeft(x+45, y, "@", red_color);
	app.Font()->big.WriteLeft(x+70, y, pl->GetNick(), black_color);
	position->Draw(souris_x, souris_y);
	couleur->Draw(souris_x, souris_y);
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

	std::string s = "Pret   Pseudo        Position     Couleur";
	label = new TLabel(x, y, s, black_color, &app.Font()->big);
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
