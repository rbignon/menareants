/* src/JoinGame.cpp - Functions to list game and join/create a game.
 *
 * Copyright (C) 2005-2011 Romain Bignon  <romain@menareants.org>
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

#include <fstream>
#include <math.h>
#include "Config.h"
#include "Debug.h"
#include "InGame.h"
#include "JoinGame.h"
#include "Main.h"
#include "Map.h"
#include "Outils.h"
#include "Resources.h"
#include "Sockets.h"
#include "Sound.h"
#include "Timer.h"
#include "gui/BouttonText.h"
#include "gui/Edit.h"
#include "gui/ListBox.h"
#include "gui/Memo.h"
#include "gui/MessageBox.h"
#include "tools/Font.h"
#include "tools/Video.h"

extern TLoadingForm   *LoadingForm;
extern TInGameForm    *InGameForm;
extern TOptionsForm   *OptionsForm;
extern TPingingForm   *PingingForm;
void LoadingGame(EC_Client* cl);
std::vector<std::string> TGameInfosForm::RecvMap;

TGameInfosForm *GameInfosForm = NULL; /**< Pointer to form whose show game infos */
int JOINED = 0;                       /**< 1 = joined, -1 = error */
bool RECOVERING = false;

/** We received an information showed in the player's screen.
 *
 * Syntax: INFO id [arg1 [arg2 ...]]
 *
 * Formatage informations :
 * - All format tags begin with '%' character
 * - There is a number between 0 and 9 in second position to identifiat number of argument in list
 * - After, there is a list of formaters :
 *    * 's' show as text argument
 *    * 'E' in form "Player!ID", this is an entity. There is an other formater :
 *          -> 'o' is this owner of entity
 *          -> 't' is the string type of entity (like "boat", it calls ECEntity::Qual() function)
 * - For example, a string like "%0Eo's %0Et shoots %1Eo's %1Et of %2s" will call for an INFO command
 *   formated like "INFO 01 Player!AA Player2!AB 200" and will show "Player's missilauncher shoots Player2's army of 200"
 */
int INFOCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	static struct
	{
		int flag;
		const char* msg;
	} messages[] =
	{
		/* 00 */ { 0,      0 },
		/* 01 */ { I_INFO, gettext_noop("%0Eo's %0Et shoot %1Eo's %1Et of %2s") }, // attaquant, attaqué, dommage
		/* 02 */ { I_SHIT, gettext_noop("%0Eo's %0Et invests %1s's McPuke installed in %2Eo's barracks!! He's going to eat everything! "
		                   "That will take %3s days for him!") }, // jouano, nom_exowner_du_mcdo, caserne_investie, nb_de_tours
		/* 03 */ { I_INFO, gettext_noop("DEBUGINFO: %0s") },
		/* 04 */ { I_SHIT, gettext_noop("%0Eo's %0Et is farting!!! Every infantry next to him will be damaged because of stink!!") },
		/* 05 */ { I_INFO, gettext_noop("Your %0Et brings you $%1s today.") } // gulag, how many
	};
	if(InGameForm)
	{
		uint id = StrToTyp<uint>(parv[1]);
		if(!id || id > ASIZE(messages)) return Debug(W_DESYNCH|W_SEND, "INFO: Reception d'un message dont l'id n'est pas dans la liste");

		const char* msg = _(messages[id].msg);
		std::string fmsg;

		while(*msg)
		{
			if(*msg == '%')
			{
				msg++;
				ParvList::size_type i;
				if(*msg < '0' || *msg > '9' || (i = *msg - '0' + 2) >= parv.size())
				{
					fmsg += '%', fmsg += *msg;
					Debug(W_WARNING|W_SEND, "INFO: Le numero %c est incorrect", *msg);
				}
				else
					switch(*++msg)
					{
						case 's':
						{
							msg++;
							fmsg += parv[i];
							break;
						}
						case 'E':
						{
							std::string et_name = parv[i];
							std::string nick = stringtok(et_name, "!");
							ECPlayer* pl = 0;
							if(nick != "*")
							{
								pl = me->Player()->Channel()->GetPlayer(nick.c_str());
								if(!pl)
								{
									fmsg += "<unknown entity>";
									Debug(W_WARNING|W_SEND, "INFO: Le player de l'unité %s est introuvable", parv[i].c_str());
									break;
								}
							}

							ECEntity* entity = 0;
							if(pl)
								entity = dynamic_cast<ECEntity*>(pl->Entities()->Find(et_name.c_str()));
							else
								entity = dynamic_cast<ECEntity*>(me->Player()->Channel()->Map()->Neutres()->Find(et_name.c_str()));

							if(!entity)
							{
								fmsg += "<unknown entity>";
								Debug(W_WARNING|W_SEND, "INFO: L'unité %s est introuvable", parv[i].c_str());
							}
							else switch(*++msg)
							{
								case 'o':
								{
									++msg;
									fmsg += pl ? nick : _("neutral");
									break;
								}
								case 't':
								{
									++msg;
									fmsg += entity->Qual();
									break;
								}
								default: fmsg += *msg;
							}
							break;
						}
						default: fmsg += '%';
					}
			}
			else
				fmsg += *msg++;
		}


		me->LockScreen();
		InGameForm->AddInfo(I_INFO, fmsg, 0);
		me->UnlockScreen();
	}

	return 0;
}

/** Someone wants to kick me
 *
 * Syntax: nick KICK victime [reason]
 */
int KICKCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(parv[1] != me->GetNick())
		return 0; // On s'en fou

	TForm::Message = StringF(_("You have been kicked from game by %s.\n\n%s"), parv[0].c_str(),
		                         parv.size() > 2 ? ((_("Reason is: ") + parv[2]).c_str()) : _("No reason specified"));

	return 0;
}

/** We receive a map !
 *
 * Syntax: SMAP ligne
 */
int SMAPCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	EChannel* chan = me->Player()->Channel();
	if(TGameInfosForm::RecvMap.empty() && chan->IsInGame() == false)
	{
		ECMap *map;
		me->LockScreen();
		if((map = dynamic_cast<ECMap*>(chan->Map())))
		{
			if(GameInfosForm)
				GameInfosForm->Preview->SetImage(NULL);
			MyFree(map);
		}
		chan->SetMap(NULL);
		me->UnlockScreen();
	}
	TGameInfosForm::RecvMap.push_back(parv[1]);
	return 0;
}

/** This is end of map.
 *
 * Syntax: EOSMAP [filename]
 */
int EOSMAPCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	EChannel *chan = me->Player()->Channel();
	if(TGameInfosForm::RecvMap.empty())
		Debug(W_DESYNCH|W_SEND, "EOSMAP: Reception d'une map vide !?");
	else if(chan->IsInGame() && chan->IsPinging() == false)
	{
		if(parv.size() < 2)
			return Debug(W_DESYNCH|W_SEND, "EOSMAP: Dans le cas d'une sauvegarde, pas de nom de fichier");

		std::string filename = MenAreAntsApp::GetInstance()->GetPath() + parv[1] + ".sav";
		std::ofstream fp(filename.c_str());

		FORit(std::string, TGameInfosForm::RecvMap, it)
			fp << *it << std::endl;
	}
	else
	{
		me->LockScreen();
		ECMap *map = 0;
		try
		{
			map = new ECMap(TGameInfosForm::RecvMap);
			map->Init();
			if(GameInfosForm)
				map->CreatePreview(GameInfosForm->PretButton->X() - GameInfosForm->Preview->X() - 30,
				                   GameInfosForm->PretButton->X() - GameInfosForm->Preview->X() - 30,
				                   P_BORDER);
		}
		catch(TECExcept &e)
		{
			delete map;
			Debug(W_ERR, "Unable to load a map :");
			vDebug(W_ERR|W_SEND, e.Message(), e.Vars());
			TGameInfosForm::RecvMap.clear();
			me->UnlockScreen();
			return 0;
		}
		chan->SetMap(map);
		map->SetChannel(chan);
		if(GameInfosForm)
		{
			GameInfosForm->MapTitle->SetCaption(chan->Map()->Name() + " (" + TypToStr(chan->Map()->MinPlayers()) +
												"-" + TypToStr(chan->Map()->MaxPlayers()) + ")");
			GameInfosForm->RecvMap.clear();
			GameInfosForm->Preview->SetImage(chan->Map()->Preview(), false);
			GameInfosForm->Preview->SetY(GameInfosForm->Window()->GetHeight() - 50 - GameInfosForm->Preview->Height());
			GameInfosForm->MapTitle->SetXY(GameInfosForm->Preview->X() + GameInfosForm->Preview->Width()/2 -
			                               GameInfosForm->MapTitle->Width()/2,
			                               GameInfosForm->Preview->Y() - GameInfosForm->MapTitle->Height() - 5);
			GameInfosForm->MapSetted();
			GameInfosForm->SetMustRedraw();
		}
		me->UnlockScreen();
	}
	return 0;
}

/** This is map list.
 *
 * Syntax: LSM nom min max infos
 */
int LSMCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(GameInfosForm)
	{
		me->LockScreen();
		if(!GameInfosForm->RecvMapList)
		{
			GameInfosForm->RecvMapList = true;
			GameInfosForm->MapList->ClearItems();
		}
		TListBoxItem* i = GameInfosForm->MapList->AddItem(false, parv[1] + " (" + parv[2] + "-" + parv[3] + ")", parv[1], white_color,
		                                         true);
		if(parv.size() > 4)
			i->SetHint(parv[4].c_str());
		me->UnlockScreen();
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

/** We received a private message in channel.
 *
 * Syntax: nick AMSG message
 */
int AMSGCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(InGameForm) // AddInfo lock lui même l'écran
		InGameForm->AddInfo(I_CHAT, "[private] <" + parv[0] + "> " + parv[1],
	                     strstr(parv[1].c_str(), me->GetNick().c_str()) ? 0 : *(players.begin()));
	else if(GameInfosForm)
	{
		me->LockScreen();
		GameInfosForm->Chat->AddItem("[private] <" + parv[0] + "> " + parv[1],
	                     strstr(parv[1].c_str(), me->GetNick().c_str()) ? red_color : white_color);
		me->UnlockScreen();
	}

	return 0;
}

/** We received a message in channel.
 *
 * Syntax: nick MSG message
 */
int MSGCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	me->LockScreen();
	if(InGameForm)
		InGameForm->AddInfo(I_CHAT, "<" + parv[0] + "> " + parv[1],
	                     strstr(parv[1].c_str(), me->GetNick().c_str()) ? 0 : *(players.begin()));
	else if(GameInfosForm)
		GameInfosForm->Chat->AddItem("<" + parv[0] + "> " + parv[1],
	                     strstr(parv[1].c_str(), me->GetNick().c_str()) ? red_color : white_color);
	me->UnlockScreen();

	return 0;
}

#if 0 /** \todo peut toujours servir! */
static TPlayerLine* GetPlayerLineFromPlayer(ECPlayer* pl)
{
	if(GameInfosForm)
	{
		std::vector<TComponent*> list = GameInfosForm->Players->GetList();
		for(std::vector<TComponent*>::iterator it=list.begin(); it!=list.end(); ++it)
		{
			TPlayerLine *pline = dynamic_cast<TPlayerLine*>(*it);
			if(!pline) /* Ce n'est pas un TPlayerLine */
				continue;
			if(pl == pline->Player())
				return pline;
		}
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
	for(std::string::const_iterator c = parv[1].begin(); c != parv[1].end(); ++c)
	{
		switch(*c)
		{
			case '+': add = true; break;
			case '-': add = false; break;
			case 'l':
				if(add)
				{
					if(j<parv.size())
						chan->SetLimite(StrToTyp<uint>(parv[j++]));
					else Debug(W_DESYNCH|W_SEND, "SET +l: sans limite");
				}
				else
					vDebug(W_DESYNCH|W_SEND, "SET -l: interdit !", VSName(parv[0].c_str()) VSName(parv[1].c_str()));
				break;
			case 'W': if(add) chan->SetState(EChannel::WAITING); break;
			case 'Q': if(add) chan->SetState(EChannel::PINGING); break;
			case 'S':
				if(add)
				{
					me->Player()->SetReady(false);
					LoadingGame(me);
					chan->SetState(EChannel::SENDING);
				}
				break;
			case 'P':
				if(add)
				{
					/* Si ce n'était pas un animation c'est pas un *nouveau* tour */
					if(chan->State() == EChannel::ANIMING)
					{
						chan->Map()->NextDay();
						++chan->Map()->NbDays();
						me->LockScreen();
						chan->Map()->CreatePreview(120,120, P_ENTITIES);
						std::vector<ECBEntity*> ents = chan->Map()->Entities()->List();
						for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
						{
							(*enti)->Played();
							(*enti)->CancelEvents();
						}
						if(InGameForm)
						{
							InGameForm->BarreLat->Icons->SetList(EntityList.Buildings(me->Player()), TBarreLat::SelectUnit);
							Resources::SoundBegin()->Play();
							InGameForm->GetElapsedTime()->reset();
							InGameForm->AddInfo(I_INFO, _("*** NEW TURN: ") + chan->Map()->Date()->String());
						}
						me->UnlockScreen();
					}
					else
						chan->Map()->InitDate()->SetDate(chan->Map()->Date());
					if(InGameForm && InGameForm->BarreLat)
					{
						chan->Map()->ShowWaitMessage.clear();
				 		InGameForm->BarreLat->Date->SetCaption(chan->Map()->Date()->String());
				 		InGameForm->BarreLat->Show();
						// Move the lateral bar
						int t0 = InGameForm->GetTimer()->get_time();
						const int time_to_show = 600; // in ms
						while(InGameForm->GetTimer()->get_time() < t0 + time_to_show)
						{
							int dt = InGameForm->GetTimer()->get_time() - t0;
							int x = (int)SCREEN_WIDTH - int(float(InGameForm->BarreLat->Width()) * sin((float)dt / (float)time_to_show * M_PI_2));

							InGameForm->Map->ToRedraw(InGameForm->BarreLat);
				 			InGameForm->BarreLat->SetXY(x, InGameForm->BarreLat->Y());
							SDL_Delay(10);
						}
			 			InGameForm->BarreLat->SetXY((int)SCREEN_WIDTH - InGameForm->BarreLat->Width(),
			 			                            InGameForm->BarreLat->Y());
			 			InGameForm->Map->ToRedraw(InGameForm->BarreLat);

				 		InGameForm->Map->SetEnabled(true);
				 		InGameForm->ShowBarreLat(true);
				 	}
				 	chan->SetState(EChannel::PLAYING);
				}
				break;
			case 'A':
				 if(add)
				 {
				 	chan->SetState(EChannel::ANIMING);
				 	if(InGameForm && InGameForm->BarreLat)
				 	{
				 		Resources::SoundEnd()->Play();
				 		InGameForm->AddInfo(I_INFO, _("*** END OF TURN."));
				 		InGameForm->Map->SetEnabled(false);
				 		InGameForm->ShowBarreLat(false);

						// Move the lateral bar
						int t0 = InGameForm->GetTimer()->get_time();
						const int time_to_hide = 600; // in ms
						while(InGameForm->GetTimer()->get_time() < t0 + time_to_hide)
						{
							int dt = InGameForm->GetTimer()->get_time() - t0;
							int x = (int)SCREEN_WIDTH - int(float(InGameForm->BarreLat->Width()) * sin((float)(time_to_hide - dt) / (float)time_to_hide * M_PI_2));

							InGameForm->Map->ToRedraw(InGameForm->BarreLat);
				 			InGameForm->BarreLat->SetXY(x, InGameForm->BarreLat->Y());
							SDL_Delay(10);
						}
			 			InGameForm->BarreLat->SetXY(SCREEN_WIDTH, InGameForm->BarreLat->Y());
			 			InGameForm->Map->ToRedraw(InGameForm->BarreLat);
				 		InGameForm->BarreLat->Hide();
				 		if(InGameForm->BarreLat->ProgressBar)
				 			InGameForm->BarreLat->ProgressBar->SetValue(0);
				 	}
				 }
				 break;
			case 'E': if(add) chan->SetState(EChannel::SCORING); break;
			case 'd':
				if(add && InGameForm)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH|W_SEND, "SET +d: sans infos"); break; }
					std::string s = parv[j++];
					std::string nick = stringtok(s, ":");
					ECPlayer* pl = chan->GetPlayer(nick.c_str());
					if(!pl)
						Debug(W_DESYNCH|W_SEND, "SET +d '%s:%s': player introuvable", nick.c_str(), s.c_str());
					else if(pl->IsMe())
						InGameForm->AddInfo(I_INFO, StringF(_("%s gives you $%s"), sender->GetNick(), s.c_str()));
					else
						InGameForm->AddInfo(I_INFO, StringF(_("%s gives $%s to %s"), sender->GetNick(), s.c_str(), pl->GetNick()));
				}
				break;
			case 'e':
				if(add && InGameForm)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH|W_SEND, "SET +d: sans infos"); break; }
					std::string s = parv[j++];
					std::string nick = stringtok(s, ":");
					ECPlayer* pl = chan->GetPlayer(nick.c_str());
					if(!pl)
						Debug(W_DESYNCH|W_SEND, "SET +d '%s:%s': player introuvable", nick.c_str(), s.c_str());
					else if(pl->IsMe())
						InGameForm->AddInfo(I_INFO, StringF(_("%s gives you a country"), sender->GetNick()));
					else
						InGameForm->AddInfo(I_INFO, StringF(_("%s gives a country to %s"), sender->GetNick(), pl->GetNick()));
				}
				break;
			case 'v':
				if(add)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH|W_SEND, "SET +v: sans numero"); break; }
					sender->Votes() = StrToTyp<uint>(parv[j]);
					if(PingingForm)
					{
						me->LockScreen();
						std::vector<TComponent*> list = PingingForm->Players->GetList();
						for(std::vector<TComponent*>::iterator it=list.begin(); it!=list.end(); ++it)
						{
							TPingingPlayerLine* pll = dynamic_cast<TPingingPlayerLine*>(*it);
							if(pll && pll->Player() == sender)
							{
								pll->NbVotes->SetCaption(parv[j]);
								break;
							}
						}
						me->UnlockScreen();
					}
				}
				else
					Debug(W_DESYNCH|W_SEND, "SET -v: interdit");

				j++;
				break;
			case 'm':
				if(add)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH|W_SEND, "SET +m: sans numero"); break; }
					if(GameInfosForm)
					{
						GameInfosForm->MapList->Select(StrToTyp<uint>(parv[j])); // pas incr j ici, lire avant le break;
						if(sender != me->Player())
							GameInfosForm->MapList->ScrollTo(StrToTyp<uint>(parv[j]));
						GameInfosForm->PretButton->SetEnabled(true);
						me->LockScreen();
						GameInfosForm->SetMustRedraw();
						me->UnlockScreen();
					}
				}
				else
					Debug(W_DESYNCH|W_SEND, "SET -m: theoriquement impossible");

				// On le fait ici car il a pu y avoir des erreurs ou surtout, il est possible qu'on ait recover une partie
				// (ce qui fait que GameInfosForm == 0)
				j++;
				break;
			case '@':
			{
				if(!players.size() || j >= parv.size() || !chan->Map())
				{
					Debug(W_DESYNCH|W_SEND, "SET %c@: incorrect", add ? '+' : '-');
					break;
				}
				me->LockScreen();
				BCountriesVector cv = chan->Map()->Countries();
				const char* ident = parv[j++].c_str();
				for(BCountriesVector::iterator ci = cv.begin(); ci != cv.end(); ++ci)
					if(!strcmp((*ci)->ID(), ident))
					{
						std::string country_name = (*ci)->Name();
						if(country_name.empty())
							country_name = ident;

						bool update = false;
						ECBPlayer* last_owner = (*ci)->Owner() ? (*ci)->Owner()->Player() : 0;
						if(InGameForm && last_owner == me->Player())
						{
							update = true;
							if(add)
								InGameForm->AddInfo(I_SHIT, StringF(_("%s has taken your country (%s)"), players[0]->GetNick(), country_name.c_str()));
							else
								InGameForm->AddInfo(I_SHIT, StringF(_("Your country %s is now neutral!"), country_name.c_str()));
						}

						(*ci)->ChangeOwner(add ? players[0]->MapPlayer() : 0);
						if(InGameForm && !update &&
						   (players[0]->IsMe() || players[0]->IsAllie(me->Player()) || (last_owner && last_owner->IsAllie(me->Player()))))
						{
							if(add)
								InGameForm->AddInfo(I_INFO, StringF(_("%s is now owned by %s"), country_name.c_str(), players[0]->GetNick()));
							else
								InGameForm->AddInfo(I_INFO, StringF(_("%s is now neutral"), country_name.c_str()));
						}
						if(InGameForm && chan->State() == EChannel::PLAYING)
							// Il n'est pas nécessaire de mettre à jour la preview en ANIMING, car c'est fait automatiquement à la fin du tour
							chan->Map()->CreatePreview(120,120, P_ENTITIES);
						break;
					}
				me->UnlockScreen();
				break;
			}
			case '!':
				if(!players.size()) { Debug(W_DESYNCH|W_SEND, "SET %c!: sans sender", add ? '+' : '-'); break; }
				me->LockScreen();
				for(PlayerList::iterator it=players.begin(); it != players.end(); ++it)
				{
					(*it)->SetReady(add);
					if((*it)->IsMe())
					{
						if(GameInfosForm)
							GameInfosForm->PretButton->SetEnabled(!add);
						if(!add && (chan->State() == EChannel::SENDING || chan->State() == EChannel::ANIMING))
							me->sendrpl(MSG_SET, "+!");
						if(InGameForm)
						{
							if(!(*it)->Lost())
								InGameForm->BarreLat->PretButton->SetEnabled(!add);
							if(add && chan->State() == EChannel::PLAYING)
								InGameForm->Map->SetCreateEntity(0);
						}

						/* En mode ANIMING, la confirmation de chaque evenement est manifesté par le +!
						 * et marque la fin d'un evenement, donc on a plus besoin de s'en rappeler.
						 * On peut considérer qu'on passe par là à chaques fins d'evenements
						 */
						if(add && chan->State() == EChannel::ANIMING)
						{
							me->UnlockScreen();
							if(chan->CurrentEvent() & ARM_ATTAQ)
								SDL_Delay(500);
							me->LockScreen();
							chan->SetCurrentEvent(0);
						}
					}
					if(add && InGameForm && chan->State() == EChannel::PLAYING)
						InGameForm->AddInfo(I_INFO, StringF(_("%s is ready."), (*it)->GetNick()));
				}
				me->UnlockScreen();
				break;
			case '$':
				if(j>=parv.size()) Debug(W_DESYNCH|W_SEND, "SET %c$: sans argent", add ? '+' : '-');
				else if(players.empty()) Debug(W_DESYNCH|W_SEND, "SET %c$: sans sender", add ? '+' : '-');
				else if(!add) Debug(W_DESYNCH|W_SEND, "SET -$: interdit");
				else
				{
					int money = StrToTyp<int>(parv[j++]);
					if(InGameForm && players[0]->IsMe())
					{
						if(money > players[0]->Money())
						{
							if(chan->State() == EChannel::ANIMING)
							{
								InGameForm->BarreLat->TurnMoney->SetCaption(StringF(_("$%d/t"), (money - players[0]->Money())));
								InGameForm->BarreLat->TurnMoney->SetX(
								                                InGameForm->BarreLat->X() + InGameForm->BarreLat->Width() -
								                                InGameForm->BarreLat->TurnMoney->Width() - 15);
							}
							InGameForm->AddInfo(I_INFO, StringF(_("*** You earn $%d"), (money - players[0]->Money())));
						}
						SDL_Delay(50);
						InGameForm->BarreLat->Money->SetCaption(StringF(_("$%d"), money));
						InGameForm->BarreLat->Money->SetColor(red_color);

				 		SDL_Delay(200);
				 		InGameForm->BarreLat->Money->SetColor(white_color);

					}
					players[0]->SetMoney(money);
				}
				break;
			case 'o':
			{
				if(j>=parv.size()) { Debug(W_DESYNCH|W_SEND, "SET %co: sans nick", add ? '+' : '-'); break; }
				ECPlayer *pl = chan->GetPlayer(parv[j++].c_str());
				if(!pl)
				{
					Debug(W_DESYNCH|W_SEND, "SET %co: %s non trouvé", add ? '+' : '-', parv[(j-1)].c_str());
					break;
				}
				pl->SetOp(add);
				if(GameInfosForm && pl->IsMe())
					GameInfosForm->ChangeStatus(add);
				break;
			}
			case 'b':
				if(GameInfosForm && j<parv.size())
					GameInfosForm->BeginMoney->SetValue(StrToTyp<int>(parv[j]));
				j++; // Il faut ++ dans tous les cas
				break;
			case 't':
			{
				if(j>=parv.size()) { Debug(W_DESYNCH, "SET +t: sans temps"); break; }
				uint time = StrToTyp<uint>(parv[j++]);
				chan->SetTurnTime(time);
				if(GameInfosForm)
					GameInfosForm->TurnTime->SetValue(time);
				break;
			}
			case 'r':
			{
				if(GameInfosForm)
					GameInfosForm->SpeedGame->Check(add);
				break;
			}
			case 's':
			{
				if(GameInfosForm)
					GameInfosForm->Scoring->Check(add);
				chan->SetScoring(add);
				if(chan->State() == EChannel::PLAYING)
				{
					WAIT_EVENT_T(InGameForm, i, 5);
					if(InGameForm)
						InGameForm->AddInfo(I_WARNING, _("*** You have selected \"Scoring\" Option, but game's scores will not be counted because:\n"
						                                 "    - You are playing only with AIs players.\n"
						                                 "    - Or you are playing on a non-official server.\n"));
				}
				break;
			}
			case '_':
			{
				if(!add)
					{ Debug(W_DESYNCH|W_SEND, "SET -_: theoriquement impossible, on ne peut pas \"déperdre\""); break; }
				if(InGameForm)
				{
					if(sender->IsMe())
					{
						InGameForm->AddInfo(I_SHIT, _("*** VOUS HAVE LOST!!!"));
						// On enlève le brouillard pour que l'user puisse voir toute la partie
						chan->Map()->SetBrouillard(false);
						InGameForm->Map->SetBrouillard(false);
					}
					else
						InGameForm->AddInfo(I_SHIT, StringF(_("*** %s has lost."), sender->GetNick()));
					me->LockScreen();
					chan->Map()->CreatePreview(120,120, P_ENTITIES);
					me->UnlockScreen();
				}
				if(sender)
					sender->SetLost();

				if(PingingForm)
				{
					me->LockScreen();
					PingingForm->UpdateList();
					me->UnlockScreen();
				}
				break;
			}
			case 'w':
			{
				if(sender)
				{
					sender->SetDisconnected(add);
					if(InGameForm)
						InGameForm->AddInfo(I_SHIT, StringF(_("*** %s has leave game"), sender->GetNick()));
				}

				if(PingingForm)
				{
					me->LockScreen();
					PingingForm->UpdateList();
					me->UnlockScreen();
				}
				break;
			}
			case 'a':
			{
				if(j>=parv.size()) { Debug(W_DESYNCH|W_SEND, "SET %ca: sans nick", add ? '+' : '-'); break; }
				ECPlayer *pl = chan->GetPlayer(parv[j++].c_str());
				if(!pl)
				{
					Debug(W_DESYNCH|W_SEND, "SET %ca: %s non trouvé", add ? '+' : '-', parv[(j-1)].c_str());
					break;
				}
				if(add)
				{
					sender->AddAllie(pl);
					if(InGameForm)
					{
						if(sender->IsMe())
							InGameForm->AddInfo(I_INFO, StringF(_("*** You become allied to %s"), pl->GetNick()));
						else if(pl->IsMe())
						{
							if(pl->IsAllie(sender))
								InGameForm->AddInfo(I_INFO, StringF(_("*** %s has accepted your alliance"), sender->GetNick()));
							else
								InGameForm->AddInfo(I_INFO, StringF(_("*** %s become allied to you."), sender->GetNick()));
						}
						else
							InGameForm->AddInfo(I_INFO, StringF(_("*** %s become allied to %s"), sender->GetNick(), pl->GetNick()));
					}
					if(OptionsForm)
					{
						std::vector<TComponent*> list = OptionsForm->Players->GetList();
						for(std::vector<TComponent*>::iterator it=list.begin(); it!=list.end(); ++it)
						{
							TOptionsPlayerLine* pll = dynamic_cast<TOptionsPlayerLine*>(*it);
							if(pll)
							{
								if(pll->Player() == pl && sender->IsMe())
									pll->allie->SetCaption(">");
								else if(pll->Player() == sender && pl->IsMe())
									pll->recipr->SetCaption("<");
							}
						}
					}
				}
				else
				{
					if(!sender->RemoveAllie(pl))
						Debug(W_DESYNCH|W_SEND, "SET -a %s: Il se trouve qu'il n'était pas mon allié !!", pl->GetNick());
					else if(InGameForm)
					{
						if(sender->IsMe())
							InGameForm->AddInfo(I_INFO, StringF(_("*** You have broken your alliance with %s"), pl->GetNick()));
						else if(pl->IsMe())
								InGameForm->AddInfo(I_INFO, StringF(_("*** %s has broken his alliance with you"), sender->GetNick()));
						else
							InGameForm->AddInfo(I_INFO, StringF(_("*** %s has broken his alliance with %s"),sender->GetNick(), pl->GetNick()));
					}
					if(OptionsForm)
					{
						std::vector<TComponent*> list = OptionsForm->Players->GetList();
						for(std::vector<TComponent*>::iterator it=list.begin(); it!=list.end(); ++it)
						{
							TOptionsPlayerLine* pll = dynamic_cast<TOptionsPlayerLine*>(*it);
							if(pll)
							{
								if(pll->Player() == pl && sender->IsMe())
									pll->allie->SetCaption(" ");
								else if(pll->Player() == sender && pl->IsMe())
									pll->recipr->SetCaption(" ");
							}
						}
					}
				}
				if(pl->IsMe())
				{ /* On affiche ou cache les territoires où était les unités de mes alliés */
					std::vector<ECBEntity*> ents = sender->Entities()->List();
					for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
						dynamic_cast<ECEntity*>(*enti)->SetShowedCases(add, true);
					me->LockScreen();
					if(chan && chan->Map())
						chan->Map()->CreatePreview(120,120, P_ENTITIES);
					me->UnlockScreen();
				}
				break;
			}
			case 'n':
				if(!players.size()) { Debug(W_DESYNCH|W_SEND, "SET %cn: sans sender humain", add ? '+' : '-'); break; }
				if(add)
				{
					if(j<parv.size())
					{
						for(PlayerList::iterator it=players.begin(); it != players.end(); ++it)
							if(!(*it)->SetNation(StrToTyp<uint>(parv[j])))
								Debug(W_DESYNCH|W_SEND, "SET +n: couleur trop grande !?");
						j++;
					}
					else Debug(W_DESYNCH|W_SEND, "SET +n: sans couleur");
				}
				else
					for(PlayerList::iterator it=players.begin(); it != players.end(); ++it)
					{
						if(!(*it)->IsMe() && (*it)->Nation() > 0 && GameInfosForm->MyNation)
							GameInfosForm->MyNation->Item((*it)->Nation())->SetEnabled();
						(*it)->SetNation(0);
					}
				break;
			case 'c':
				if(!players.size()) { Debug(W_DESYNCH|W_SEND, "SET %cc: sans sender humain", add ? '+' : '-'); break; }
				if(add)
				{
					if(j<parv.size())
					{
						for(PlayerList::iterator it=players.begin(); it != players.end(); ++it)
						{
							if(!(*it)->IsMe() && (*it)->Color() > 0 && GameInfosForm->MyColor)
								GameInfosForm->MyColor->DelBadValue((*it)->Color());
							if(!(*it)->SetColor(StrToTyp<uint>(parv[j])))
								Debug(W_DESYNCH|W_SEND, "SET +c: couleur trop grande !?");
							else if(!(*it)->IsMe() && (*it)->Color() > 0 && GameInfosForm->MyColor)
								GameInfosForm->MyColor->AddBadValue((*it)->Color());
						}
						j++;
					}
					else Debug(W_DESYNCH|W_SEND, "SET +c: sans couleur");
				}
				else
					for(PlayerList::iterator it=players.begin(); it != players.end(); ++it)
					{
						if(!(*it)->IsMe() && (*it)->Color() > 0 && GameInfosForm->MyColor)
							GameInfosForm->MyColor->DelBadValue((*it)->Color());
						(*it)->SetColor(0);
					}
				break;
			case 'p':
				if(!players.size()) { Debug(W_DESYNCH|W_SEND, "SET %cp: sans sender humain", add ? '+' : '-'); break; }
				if(add)
				{
					if(j<parv.size())
					{
						for(PlayerList::iterator it=players.begin(); it != players.end(); ++it)
							if(!(*it)->SetPosition(StrToTyp<uint>(parv[j])))
								Debug(W_DESYNCH|W_SEND, "SET +p: position hors limite");
						j++;
					}
					else Debug(W_DESYNCH|W_SEND, "SET +p: sans position");
				}
				else
					for(PlayerList::iterator it=players.begin(); it != players.end(); ++it)
						(*it)->SetPosition(0);

				me->LockScreen();
				GameInfosForm->RefreshPositions();
				me->UnlockScreen();
				break;
			default:
				Debug(W_DESYNCH|W_SEND, "SET %c%c: Reception d'un mode non supporté", add ? '+' : '-', *c);
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

	std::vector<int> col_badval;
	std::vector<int> nat_badval;
	for(ParvList::iterator parvi=(parv.begin()+1); parvi!=parv.end(); ++parvi)
	{
		const char *nick = (*parvi).c_str();
		bool owner = false, ready = false, op = false;

		if(*nick == '*')
			owner = true, nick++;
		if(*nick == '@')
			op = true, nick++;
		if(*nick == '!')
			ready = true, nick++;

		std::string nb;
		int pos = 0, col = 0, nat = 0;
		while(*nick != ',')
			nb += *nick++;
		nick++;
		pos = StrToTyp<uint>(nb);

		nb.clear();
		while(*nick != ',')
			nb += *nick++;
		nick++;
		col = StrToTyp<uint>(nb);

		nb.clear();
		while(*nick != ',')
			nb += *nick++;
		nick++;
		nat = StrToTyp<uint>(nb);

		ECPlayer *pl = 0;
		if(!strcasecmp(nick, me->GetNick().c_str()))
		{ /* On a déjà créé notre player */
			if(owner) me->Player()->SetOwner();
			if(op) me->Player()->SetOp();
			pl = me->Player();
		}
		else
			pl = new ECPlayer(nick, me->Player()->Channel(), owner, op, false, (*nick == IA_CHAR));
		pl->SetColor(col);
		pl->SetPosition(pos);
		pl->SetReady(ready);
		pl->SetNation(nat);
		if(GameInfosForm)
		{
			TPlayerLine *pline = new TPlayerLine(pl);
			GameInfosForm->Players->AddLine(pline);
			if(pl == me->Player())
			{
				GameInfosForm->MyColor = pline->couleur;
				GameInfosForm->MyNation = pline->nation;
			}
			else
			{ /* GameInfosForm->MyNation et GameInfosForm->MyColor ne sont probablement pas encore défini ! */
				if(col > 0)
					col_badval.push_back(col);
				if(nat > 0)
					nat_badval.push_back(nat);
				pline->couleur->SetEnabled(false);
				pline->nation->SetEnabled(false);
				if(me->Player()->IsOwner())
					pline->Nick->SetHint(StringF(_("Clic on his nickname to kick %s"), pl->GetNick()));
			}
		}
	}
	if(GameInfosForm)
	{
		if(GameInfosForm->MyColor)
			for(std::vector<int>::iterator it = col_badval.begin(); it != col_badval.end(); ++it)
				GameInfosForm->MyColor->AddBadValue(*it);
		if(GameInfosForm->MyNation)
			for(std::vector<int>::iterator it = nat_badval.begin(); it != nat_badval.end(); ++it)
				GameInfosForm->MyNation->Item(*it)->SetEnabled(false);
		GameInfosForm->RecalcMemo();
	}
	JOINED = 1;
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
			EChannel *c = new EChannel(parv[1], (parv[1] == "."));
			me->SetPlayer(new ECPlayer(parv[0], c, false, false, true, false));

			/* En cas de gros lag, il se peut que le joueur ait supprimé GameInfosForm avant même avoir reçu le JOIN.
			 * Dans ce cas on prévient quand même le serveur qu'on part.
			 */
			if(!GameInfosForm && !RECOVERING)
				me->sendrpl(MSG_LEAVE);
		}
		else
		{ /* C'est un user qui rejoin le chan */
			if(!me->Player()) return Debug(W_DESYNCH|W_SEND, "Reception d'un join sans être sur un chan");

			me->LockScreen();
			ECPlayer *pl = new ECPlayer(parv[0].c_str(), me->Player()->Channel(), false, false, false,
			                            (parv[0][0] == IA_CHAR));
			if(GameInfosForm)
			{
				GameInfosForm->Chat->AddItem(StringF(_("*** %s has joined game"), parv[0].c_str()), fgreen_color);
				TPlayerLine *pline;
				GameInfosForm->Players->AddLine((pline = new TPlayerLine(pl)));
				GameInfosForm->RecalcMemo();
				pline->couleur->SetEnabled(false);
				pline->nation->SetEnabled(false);
				GameInfosForm->RefreshPositions();
				if(!me->Player()->Channel()->IsMission() && !pl->IsIA())
					GameInfosForm->CheckScoringEnable();

				if(me->Player()->IsOwner())
					pline->Nick->SetHint(StringF(_("Clic on his nickname to kick %s"), pl->GetNick()));
			}
			me->UnlockScreen();
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

	/** \note Dans le protocole il ne devrait y avoir qu'un seul départ
	 *       à la fois mais bon par respect de ce qui est *possible*
	 *       (on ne sait jamais, après tout, on pourrait imaginer dans
	 *        un avenir des "services" qui pourraient "hacker" (plusieurs
	 *        départs ?) ou alors pour une eventuelle IA du serveur)
	 */
	me->LockScreen();
	// PAS DE RETURN ICI
	for(PlayerList::iterator playersi=players.begin(); playersi != players.end();)
	{
		if((*playersi)->IsMe())
		{
			EChannel *c = me->Player()->Channel();
			JOINED = 0;
			me->ClrPlayer(); /* <=> me->pl = 0 */
			if(GameInfosForm)
				GameInfosForm->Players->Hide();

			if(c->WantLeave()) MyFree(GameInfosForm);

			delete c;
			me->UnlockScreen();
			return 0;
		}
		else
		{
			if(GameInfosForm)
			{
				if((*playersi)->Color() && GameInfosForm->MyColor)
					GameInfosForm->MyColor->DelBadValue((*playersi)->Color());
				if((*playersi)->Nation() && GameInfosForm->MyNation)
					GameInfosForm->MyNation->Item((*playersi)->Nation())->SetEnabled();

				GameInfosForm->Chat->AddItem(StringF(_("*** %s has leave game"), (*playersi)->GetNick()), fgreen_color);
				std::vector<TComponent*> plrs = GameInfosForm->Players->GetList();
				for(std::vector<TComponent*>::iterator it=plrs.begin(); it!=plrs.end(); ++it)
				{
					TPlayerLine *pline = dynamic_cast<TPlayerLine*>(*it);
					if(!pline) /* Ce n'est pas un TPlayerLine */
						continue;

					if(pline->Player() == (*playersi))
					{
						if(!GameInfosForm->Players->RemoveLine(*it, USE_DELETE))
							throw ECExcept(VPName(*it) VPName(*playersi), "Player-line introuvable");
						GameInfosForm->RecalcMemo();
						break;
					}
				}
				GameInfosForm->RefreshPositions();
				GameInfosForm->SetMustRedraw();
			}
			if(LoadingForm)
			{
				LoadingForm->Players->Hide();
				std::vector<TComponent*> plrs = LoadingForm->Players->GetList();
				for(std::vector<TComponent*>::iterator it=plrs.begin(); it!=plrs.end(); ++it)
				{
					TPlayerLine *pline = dynamic_cast<TPlayerLine*>(*it);
					if(!pline) /* Ce n'est pas un TPlayerLine */
						continue;

					if(pline->Player() == (*playersi))
					{
						if(!LoadingForm->Players->RemoveLine(*it, USE_DELETE))
							throw ECExcept(VPName(*it) VPName(*playersi), "Player-line introuvable");
						break;
					}
				}
				LoadingForm->Players->Show();
			}
			if(InGameForm)
			{
				InGameForm->AddInfo(I_INFO, StringF(_("*** %s has leave game"), (*playersi)->GetNick()));
				me->Player()->Channel()->Map()->CreatePreview(120,120, P_ENTITIES);
			}
			if(me->Player()->Channel()->RemovePlayer((*playersi), USE_DELETE))
				playersi = players.erase(playersi);
			else
			{
				vDebug(W_DESYNCH|W_SEND, "LEA - Impossible de supprimer un player", VPName(*playersi));
				playersi++;
			}
			if (GameInfosForm)
				GameInfosForm->CheckScoringEnable();
		}
	}
	me->UnlockScreen();
	return 0;
}

bool MenAreAntsApp::RecoverGame(std::string chan)
{
	EC_Client* client = &Server;

	if(!client)
		return false;

	RECOVERING = true;

	client->sendrpl(MSG_JOIN, chan);

	{ WAIT_EVENT_T(JOINED != 0, i, 2); }

	if(JOINED <= 0 || !client->Player())
	{
		RECOVERING = false;
		return false;
	}

	{ WAIT_EVENT_T(client->Player()->Channel()->State() >= EChannel::SENDING, i, 5); }

	RECOVERING = false;

	if(client->Player()->Channel()->State() >= EChannel::SENDING)
		LoadGame(client->Player()->Channel());

	if(client->Player())
	{
		client->Player()->Channel()->SetWantLeave();
		client->sendrpl(MSG_LEAVE);
	}
	return true;
}

bool MenAreAntsApp::GameInfos(const char *cname, TForm* form, int flags)
{
	if(!Server.IsConnected())
		throw ECExcept(VBName(Server.IsConnected()), "Non connecté");

	std::string name; /* note: comme cpath va pointer sur la valeur de name tout au long du truc, il est préferable
	                   *       de le laisser en global de la fonction et pas uniquement dans le bloc if(!cname)
	                   */
	bool create = false;

	EC_Client* client = &Server;

#ifdef SETTED_NAME
	std::string setted_name = client->GetNick() + "'s game";
#endif
	if(!cname)
	{
		create = true;
		if(flags & G_MISSION)
			cname = ".";
		else
#ifdef SETTED_NAME
			cname = setted_name.c_str();
#else
		{
			TMessageBox mb(_("Enter game's name to create"), HAVE_EDIT|BT_OK|BT_CANCEL, form);
			mb.Edit()->SetAvailChars(CHAN_CHARS);
			mb.Edit()->SetMaxLen(GAMELEN);
			if(mb.Show() == BT_OK)
				name = mb.EditText();
			if(name.empty()) return true;

			cname = name.c_str();
		}
#endif
	}

	/* Déclaration membres fixes */
	GameInfosForm = new TGameInfosForm(Video::GetInstance()->Window(), client, (flags & G_MISSION));
	GameInfosForm->Chat->AddItem(StringF(_("*** You have rejoin %s"), cname), fgreen_color);

	JOINED = 0;
	if(create)
		client->sendrpl(MSG_JOIN, ECArgs(cname, "$"));
	else
		client->sendrpl(MSG_JOIN, cname);

	/* On attend 2 secondes de lag, mais si jamais on a reçu une erreur on y partira avant */
	WAIT_EVENT_T(JOINED != 0, i, 2);
	if(JOINED <= 0)
	{
		if(client->Player())
		{
			client->Player()->Channel()->SetWantLeave();
			client->sendrpl(MSG_LEAVE);
		}
		else
			MyFree(GameInfosForm);
		return false;
	}

	EChannel *chan = NULL;
	GameInfosForm->SetMutex(mutex);

	try
	{
		if(!client->Player() || !client->Player()->Channel())
			throw ECExcept(VPName(client->Player()), "Dans aucun chan");
		chan = client->Player()->Channel();

		if(Config::GetInstance()->color)
			client->sendrpl(MSG_SET, ECArgs("+c", TypToStr(Config::GetInstance()->color)));
		if(Config::GetInstance()->nation)
			client->sendrpl(MSG_SET, ECArgs("+n", TypToStr(Config::GetInstance()->nation)));

		if(flags & G_MISSION)
			GameInfosForm->Scoring->Hide();
		if(flags & G_ESCARMOUCHE)
			client->sendrpl(MSG_SET, "-s");

		GameInfosForm->Title->SetCaption((flags & G_MISSION) ? _("Alone game") : chan->GetName());
		if(client->Player()->IsOwner())
		{
			GameInfosForm->PretButton->SetEnabled();
			GameInfosForm->PretButton->SetImage(new ECSprite(Resources::MPlayButton(), Video::GetInstance()->Window()));
			GameInfosForm->PretButton->SetHint(_("Launch game"));
		}

		GameInfosForm->ChangeStatus(client->Player()->IsPriv());

		GameInfosForm->Run();

	}
	catch(TECExcept &e)
	{
		if(client && client->Player())
		{
			client->Player()->Channel()->SetWantLeave();
			client->sendrpl(MSG_LEAVE);
		}
		else
			MyFree(GameInfosForm);
		throw;
	}

	if(!client || !client->Player())
		MyFree(GameInfosForm);
	else
	{
		if(client->Player()->Channel()->State() >= EChannel::SENDING)
			LoadGame(chan); /* LA PARTIE SE LANCE */

		if(client->Player())
		{
			client->Player()->Channel()->SetWantLeave();
			client->sendrpl(MSG_LEAVE);
		}
	}

	return true;
}

/********************************************************************************************
 *                               TGameInfosForm                                             *
 ********************************************************************************************/

void TGameInfosForm::OnKeyUp(SDL_keysym key)
{
	switch (key.sym)
	{
		case SDLK_ESCAPE:
			want_quit = true;
			break;
		case SDLK_RETURN:
			if(SendMessage->Focused() && !SendMessage->Empty())
			{
				client->sendrpl(MSG_MSG, SendMessage->GetString());
				Chat->AddItem("<" + client->GetNick() + "> " + SendMessage->GetString(), white_color);
				SendMessage->ClearString();
			}
			break;
		default: break;
	}
}

void TGameInfosForm::AfterDraw()
{
	if(!client->IsConnected() || !client->Player() || client->Player()->Channel()->State() != EChannel::WAITING)
	{
		want_quit = true;
		return;
	}
	if(client->Player()->IsPriv() && !MapList->Enabled() && listmapclick.time_elapsed(true) > 2)
		MapList->SetEnabled();
}

void TGameInfosForm::MapListChange(TListBox* MapList)
{
	if(!Server.Player()->IsPriv() || MapList->Selected() < 0)
		return;

	Server.sendrpl(MSG_SET, ECArgs("+m", TypToStr(MapList->Selected())));
	GameInfosForm->listmapclick.reset();
	MapList->SetEnabled(false);
}

void TGameInfosForm::SpinEditChange(TSpinEdit* SpinEdit)
{
	if(SpinEdit == GameInfosForm->BeginMoney)
		Server.sendrpl(MSG_SET, ECArgs("+b", TypToStr(SpinEdit->Value())));
	else if(SpinEdit == GameInfosForm->TurnTime)
		Server.sendrpl(MSG_SET, ECArgs("+t", TypToStr(SpinEdit->Value())));
}

void TGameInfosForm::OnClic(const Point2i& mouse, int button, bool&)
{
	client->LockScreen();
	if(client->Player()->IsOwner())
	{
		std::vector<TComponent*> list = Players->GetList();
		for(std::vector<TComponent*>::iterator it=list.begin(); it!=list.end(); ++it)
		{
			TPlayerLine* pll = dynamic_cast<TPlayerLine*>(*it);
			if(pll && !pll->Player()->IsMe() && !pll->Player()->IsOwner())
			{
				if(pll->OwnZone(mouse, button))
				{
					if(pll->Player()->IsOp())
						client->sendrpl(MSG_SET, ECArgs("-o", pll->Player()->Nick()));
					else
						client->sendrpl(MSG_SET, ECArgs("+o", pll->Player()->Nick()));
				}
				else if(pll->Nick->Test(mouse, button))
				{
					TMessageBox mb(StringF(_("You want to kick %s from game.\nPlease enter a reason:"), pll->Nick->Caption().c_str()),
							HAVE_EDIT|BT_OK|BT_CANCEL, this);
					std::string name;
					if(mb.Show() == BT_OK)
					{
						name = mb.EditText();
						client->sendrpl(MSG_KICK, ECArgs(pll->Nick->Caption(), name));
					}
				}
			}
		}
	}
	bool tmp = MyNation->Opened();
	if(MyColor && MyColor->Clic(mouse, button))
		client->sendrpl(MSG_SET, ECArgs("+c", TypToStr(MyColor->Value())));
	else if(MyNation && MyNation->Clic(mouse, button) && tmp && !MyNation->Opened() && MyNation->Selected() != -1)
		client->sendrpl(MSG_SET, ECArgs("+n", TypToStr(GameInfosForm->MyNation->Selected())));
	else if(RetourButton->Test(mouse, button))
	{
		if(TMessageBox(_("Do you really want to leave game?"), BT_YES|BT_NO, this).Show() == BT_YES)
			want_quit = true;
	}
	else if(PretButton->Test(mouse, button))
	{
		bool ok = true;
		if(client->Player()->IsOwner())
		{
			EChannel* chan = client->Player()->Channel();
			if(!chan->Map())
			{
				TMessageBox(_("Please select a map!"), BT_OK, this).Show();
				ok = false;
			}
			else if(chan->NbPlayers() < chan->Map()->MinPlayers() && !chan->IsMission())
			{
				TMessageBox(StringF(_("There are not sufficient players. This map requires between %d and %d players."),
				                      chan->Map()->MinPlayers(), chan->Map()->MaxPlayers()), BT_OK, this).Show();
				ok = false;
			}
			else
			{
				BPlayerVector::iterator it;
				BPlayerVector plv = chan->Players();
				uint ok = 0;
				for(it = plv.begin(); it != plv.end(); ++it) if((*it)->Ready()) ok++;
				if((ok+1) < chan->Map()->MinPlayers() && !chan->IsMission()) /* +1 for me */
				{
					TMessageBox(StringF(_("It is necessary at least that %d are ready"), chan->Map()->MinPlayers()),
					            BT_OK, this).Show();
					ok = false;
				}
				else if((ok+1) != chan->NbPlayers() &&
					TMessageBox(StringF(_("All players aren't ready. Are you sure to want to start game?\nNo ready players will be kicked")),
						BT_YES|BT_NO, this).Show() != BT_YES)
					ok = false;
			}
		}
		if(ok)
			client->sendrpl(MSG_SET, "+!");
	}
	else if(SpeedGame->Test(mouse, button))
		client->sendrpl(MSG_SET, SpeedGame->Checked() ? "+r" : "-r");
	else if(Scoring->Test(mouse, button))
		client->sendrpl(MSG_SET, Scoring->Checked() ? "+s" : "-s");
	else if(CreateIAButton->Test(mouse, button))
	{
		TMessageBox mb(_("AI's name to create:"), HAVE_EDIT|BT_OK|BT_CANCEL, this);
		mb.Edit()->SetAvailChars(NICK_CHARS);
		mb.Edit()->SetMaxLen(NICKLEN-1); // -1 parce que le serveur rajoutera IA_CHAR ('&') devant le pseudonyme
		std::string name;
		if(mb.Show() == BT_OK)
			name = mb.EditText();
		if(!name.empty())
			client->sendrpl(MSG_IA_JOIN, name);
	}
	else
	{
		uint i = 1;
		FORit(TButtonText*, Positions, bt)
		{
			if((*bt)->Test(mouse, button))
			{
				if((*bt)->Tag)
					client->sendrpl(MSG_SET, ECArgs("+p", "0"));
				else
					client->sendrpl(MSG_SET, ECArgs("+p", TypToStr(i)));
			}
			i++;
		}
	}
	client->UnlockScreen();
}

void TGameInfosForm::CheckScoringEnable()
{
	EChannel* chan = client->Player()->Channel();

	if(chan->IsMission())
		return;

	bool only_ia = !(chan->NbHumains() == chan->NbPlayers() && chan->NbPlayers() > 1);

	Scoring->SetEnabled(client->Player()->IsPriv() && !only_ia);
	Scoring->Check(!only_ia && chan->Scoring());
}

void TGameInfosForm::ChangeStatus(bool add)
{
	MapList->SetEnabled(add);
	SpeedGame->SetEnabled(add);
	Scoring->SetEnabled(add);
	BeginMoney->SetEnabled(add);
	TurnTime->SetEnabled(add);

	if(mission) return;

	CreateIAButton->SetVisible(add);
	CheckScoringEnable();
}

void TGameInfosForm::RefreshPositions()
{
	if(Positions.empty()) return;

	FORit(TButtonText*, Positions, bt)
	{
		(*bt)->SetCaption("");
		(*bt)->SetEnabled(!client->Player()->Channel()->IsMission());
		(*bt)->Tag = 0;
	}

	int i = 1;
	std::vector<TComponent*> list = Players->GetList();
	for(std::vector<TComponent*>::iterator it=list.begin(); it!=list.end(); ++it)
	{
		TPlayerLine* pll = dynamic_cast<TPlayerLine*>(*it);
		if(!pll) continue;

		ECPlayer* pl = pll->Player();

		pll->Pos->SetCaption(TypToStr(i));

		if(pl->Position())
		{
			TButtonText* bt = Positions[pl->Position()-1];
			bt->SetCaption(TypToStr(i));
			bt->Tag = i;
			if(!pl->IsMe())
				bt->SetEnabled(false);
		}

		i++;
	}
}

void TGameInfosForm::MapSetted()
{
	assert(client->Player()->Channel());

	ECMap* map = client->Player()->Channel()->Map();

	for(std::vector<TButtonText*>::iterator bt = Positions.begin(); bt != Positions.end(); ++bt)
		RemoveComponent(*bt);
	Positions.clear();

	if(map)
	{
		std::vector<ECMapPlayer*> map_players = map->MapPlayers();
		for(std::vector<ECMapPlayer*>::iterator it = map_players.begin(); it != map_players.end(); ++it)
		{
			uint begin_x = map->Width(), begin_y = map->Height(), max_x = 0, max_y = 0;
			std::vector<ECountry*> coun = (*it)->Countries();
			for(std::vector<ECountry*>::iterator ci = coun.begin(); ci != coun.end(); ++ci)
			{
				std::vector<ECBCase*> cas = (*ci)->Cases();
				for(std::vector<ECBCase*>::iterator casi = cas.begin(); casi != cas.end(); ++casi)
				{
					if((*casi)->Flags() & (C_TERRE))
					{
						if(begin_x > (*casi)->X()) begin_x = (*casi)->X();
						if(begin_y > (*casi)->Y()) begin_y = (*casi)->Y();
						if(max_x < (*casi)->X()) max_x = (*casi)->X();
						if(max_y < (*casi)->Y()) max_y = (*casi)->Y();
					}
				}
			}
			TButtonText* bt = AddComponent(new TButtonText(Preview->X() + (begin_x+max_x)/2 * map->PixelSize(),
			                                               Preview->Y() + (begin_y+max_y)/2 * map->PixelSize(),
			                                               150,50, "", Font::GetInstance(Font::Normal), black_color));
			bt->SetImage(new ECSprite(Resources::RadioButton(), Window()));
			bt->SetAlwaysRedraw();
			Positions.push_back(bt);
		}
	}
	RefreshPositions();
}

TGameInfosForm::TGameInfosForm(ECImage* w, EC_Client* cl, bool _mission)
	: TForm(w), RecvMapList(false), mission(_mission), client(cl)
{
	Title = AddComponent(new TLabel(20,"Jeu", white_color, Font::GetInstance(Font::Huge)));

	Players = AddComponent(new TList(50, 110));
	Players->AddLine(new TPlayerLineHeader);

	int chat_width = 325 * Window()->GetWidth() / 800;
	Chat = AddComponent(new TMemo(Font::GetInstance(Font::Small), 50,325,chat_width,495,/*maxlines*/50));
	SendMessage = AddComponent(new TEdit(Font::GetInstance(Font::Small), 50,Window()->GetHeight()-45,chat_width,
	                                     MAXBUFFER-20));

	MapTitle = AddComponent(new TLabel(50 + chat_width + 40, 345, "", white_color, Window()->GetWidth() < 1000 ? Font::GetInstance(Font::Normal) : Font::GetInstance(Font::Big)));
	Preview = AddComponent(new TImage(50 + chat_width + 30, 380));

	int right_x = Window()->GetWidth() - 220;

	PretButton = AddComponent(new TButton(right_x,110, 150,50));
	PretButton->SetEnabled(false);
	PretButton->SetImage(new ECSprite(Resources::ReadyButton(), Video::GetInstance()->Window()));
	PretButton->SetHint(_("Are you ready ?"));
	PretButton->SetAlwaysRedraw();
	RetourButton = AddComponent(new TButton(right_x+PretButton->Width(),110,150,50));
	RetourButton->SetImage(new ECSprite(Resources::BackButton(), Video::GetInstance()->Window()));
	RetourButton->SetHint(_("Leave game"));
	RetourButton->SetAlwaysRedraw();
	CreateIAButton = AddComponent(new TButton(right_x + PretButton->Width()/2,110+4*PretButton->Height()/5,150,50));
	CreateIAButton->Hide();
	CreateIAButton->SetImage(new ECSprite(Resources::AIButton(), Video::GetInstance()->Window()));
	CreateIAButton->SetHint(_("Add an artificial player (an AI) in game"));
	CreateIAButton->SetAlwaysRedraw();

	MapList = AddComponent(new TListBox(Rectanglei(right_x, CreateIAButton->Y()+CreateIAButton->Height(), 200, 160)));
	MapList->SetOnChange(TGameInfosForm::MapListChange);
	SpeedGame = AddComponent(new TCheckBox(Font::GetInstance(Font::Normal), right_x, MapList->Y()+MapList->Height()+5, _("Quick game"), white_color));
	SpeedGame->SetHint(_("A player has lost when he hasn't got any building."));
	SpeedGame->Check();

	Scoring = AddComponent(new TCheckBox(Font::GetInstance(Font::Normal), right_x, SpeedGame->Y()+SpeedGame->Height()+5, _("Scoring"), white_color));
	Scoring->SetHint(_("Do you want to send to meta-server scoring of game ?"));
	Scoring->SetEnabled(false);

	                                                                     /*  label        x    y    w  min   max  step */
	/* defvalue */
	BeginMoney = AddComponent(new TSpinEdit(Font::GetInstance(Font::Normal), _("Money: "),  right_x, Scoring->Y()+Scoring->Height()+5, 150, 0, 100000, 5000,
	15000));
	BeginMoney->SetHint(_("Money earned by each players at begin of game"));
	BeginMoney->SetOnChange(TGameInfosForm::SpinEditChange);

	TurnTime = AddComponent(new TSpinEdit(Font::GetInstance(Font::Normal), _("Turn durat.: "),  right_x, BeginMoney->Y()+BeginMoney->Height()+5, 150, /*min*/mission ? 0 : 15, 360, 15,
	60));
	TurnTime->SetHint(_("Maximal time in seconds of each turn"));
	TurnTime->SetOnChange(TGameInfosForm::SpinEditChange);

	SetBackground(Resources::Titlescreen());

	MyColor = 0;
	MyNation = 0;

	if(mission)
	{
		Chat->Hide();
		SendMessage->Hide();
		Preview->SetX(chat_width/2);
	}
}

void TGameInfosForm::RecalcMemo()
{
	Chat->SetXY(50, Players->Y() + Players->Height());
	Chat->SetHeight(Window()->GetHeight() - 65 - Players->Height()-Players->Y()); /* On définit une jolie taille */
}

/********************************************************************************************
 *                               TPlayerLine                                                *
 ********************************************************************************************/

TPlayerLine::TPlayerLine(ECPlayer *_pl)
{
	pl = _pl;
	size.y = 30;
	size.x = 530;
	couleur = 0;
	nation = 0;
	Ready = 0;
	Nick = 0;
	Status = 0;
	Pos = 0;
}

TPlayerLine::~TPlayerLine()
{
	delete Pos;
	delete nation;
	delete couleur;
	delete Ready;
	delete Nick;
	delete Status;
}

void TPlayerLine::SetXY (int px, int py)
{
	TComponent::SetXY(px, py);

	if(couleur) couleur->SetXY(px+305, py);
	if(nation) nation->SetXY(px+395, py);

	if(Ready) Ready->SetXY(px, py);
	if(Pos) Pos->SetXY(px+75, py+7);
	if(Status) Status->SetXY(px+88, py);
	if(Nick) Nick->SetXY(px+105, py);
}

bool TPlayerLine::OwnZone(const Point2i& mouse, int button)
{
	return (button == SDL_BUTTON_LEFT && mouse.x > X()+88 && mouse.x < X()+104 && mouse.y > Y() && mouse.y < int(Y() + Height()));
}

bool TPlayerLine::Test (const Point2i& mouse, int button) const
{
  return (Visible() && Enabled() && ((X() <= mouse.x) && (mouse.x <= int(X()+Width())) && (Y() <= mouse.x) &&
         (mouse.y <= int((nation && nation->Opened() ? nation->Y() + nation->Height() : Y() + Height())))));
}

void TPlayerLine::Init()
{
	assert(pl);
	delete Pos;
	delete couleur;
	delete nation;
	delete Ready;
	delete Status;
	delete Nick;
	                    /*  label   x    y  w  min      max                  step  defvalue */
	couleur = new TColorEdit(Font::GetInstance(Font::Small), "",  X()+305, Y(), 50);
	MyComponent(couleur);
	couleur->SetHint(("Your color"));
	nation = new TComboBox(Font::GetInstance(Font::Small), X()+395, Y(), 120);
	MyComponent(nation);

	Pos = new TLabel(X()+75, Y()+7, " ", white_color, Font::GetInstance(Font::Normal));
	Ready = new TLabel(X(), Y(), "OK", gray_color, Font::GetInstance(Font::Big));
	Status = new TLabel(X()+88, Y(), pl->IsOwner() ? "*" : "", red_color, Font::GetInstance(Font::Big));
	Nick = new TLabel(X()+105, Y(), pl->GetNick(), white_color, Font::GetInstance(Font::Big));

	MyComponent(Pos);
	MyComponent(Ready);
	MyComponent(Status);
	MyComponent(Nick);

	for(uint i = 0; i < ECPlayer::N_MAX; ++i)
	{
		TListBoxItem* j = nation->AddItem(false, gettext(nations_str[i].name), "");
		j->SetHint(gettext(nations_str[i].infos));
	}

	size.x = nation->X() + nation->Width();
}

void TPlayerLine::Draw(const Point2i& mouse)
{
	assert(pl);

	if((int)pl->Color() != couleur->Value()) couleur->SetValue(pl->Color());
	if((int)pl->Nation() != nation->Selected()) nation->Select(pl->Nation());
	Status->SetCaption(pl->IsOp() ? "@" : pl->IsOwner() ? "*" : "");
	Ready->SetColor(pl->Ready() ? red_color : gray_color);

	Pos->Draw(mouse);
	Ready->Draw(mouse);
	Status->Draw(mouse);
	Nick->Draw(mouse);
	couleur->Draw(mouse);
	nation->Draw(mouse);

	if(couleur->Test(mouse))
		SetHint(couleur->Hint());
	else if(nation->Test(mouse))
		SetHint(nation->Hint());
	else if(Nick->Test(mouse))
		SetHint(Nick->Hint());
	else if(OwnZone(mouse, SDL_BUTTON_LEFT) && !pl->IsOp() && !pl->IsOwner())
		SetHint(Status->Hint());
	else
		SetHint("");
}

/********************************************************************************************
 *                               TPlayerLineHeader                                          *
 ********************************************************************************************/

TPlayerLineHeader::TPlayerLineHeader()
{
	size.y = 30;
	size.x = 530;
	label = 0;
}

void TPlayerLineHeader::Init()
{
	if(label)
		delete label;

	std::string s = _("Ready  Nickname      Color  Nation");
	label = new TLabel(X(), Y(), s, white_color, Font::GetInstance(Font::Big));
	MyComponent(label);
	size.x = label->Width();
}

void TPlayerLineHeader::SetXY (int px, int py)
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

void TPlayerLineHeader::Draw(const Point2i& mouse)
{
	label->Draw(mouse);
}
