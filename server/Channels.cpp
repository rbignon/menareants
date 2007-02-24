/* server/Channels.cpp - Channels functions
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

#include "Channels.h"
#include "Server.h"
#include "Commands.h"
#include "Outils.h"
#include "Debug.h"
#include "Main.h"
#include "Map.h"
#include "Colors.h"
#include "Units.h"
#include "InGame.h"
#include "IA.h"
#include <cstdarg>
#include <list>
#include <map>

ChannelVector ChanList;

/********************************************************************************************
 *                              Commandes                                                   *
 ********************************************************************************************/

/** The owner can save the game.
 *
 * Syntax: SAVE filename
 */
int SAVECommand::Exec(TClient* cl, std::vector<std::string> parv)
{
	if(!cl || !cl->Player() || !cl->Player()->IsOwner())
		return Debug(W_DESYNCH, "SAVE: par un non owner ou en dehors d'un salon");

	if(!cl->Player()->Channel()->Map() || cl->Player()->Channel()->State() != EChannel::PLAYING)
		return Debug(W_DESYNCH, "SAVE: pas de map ou pas +P(laying)");

	std::vector<std::string> map_file;
	cl->Player()->Channel()->Map()->Save(map_file);

	for(std::vector<std::string>::iterator it = map_file.begin(); it != map_file.end(); ++it)
		cl->sendrpl(MSG_SENDMAP, *it);

	cl->sendrpl(MSG_ENDOFSMAP, parv[1]);

	return 0;
}

/** The owner can eject someone from channel.
 *
 * Syntax: KICK victime [reason]
 */
int KICKCommand::Exec(TClient* cl, std::vector<std::string> parv)
{
	if(!cl || !cl->Player())
		return Debug(W_DESYNCH, "KICK en dehors d'un salon");

	if(!cl->Player()->IsOwner())
		return Debug(W_DESYNCH, "KICK: le sender n'est pas owner");

	if(!cl->Player()->Channel()->Joinable())
		return Debug(W_DESYNCH, "KICK: on ne peut pas kicker pendant une partie.");

	ECPlayer *pl = cl->Player()->Channel()->GetPlayer(parv[1].c_str());

	if(!pl)
		return Debug(W_DESYNCH, "KICK: joueur non trouvé");

	if(pl == cl->Player())
		return Debug(W_DESYNCH, "KICK: le joueur cherche à se kicker");

	if(pl->Client())
	{
		pl->Client()->sendrpl(cl->Player(), MSG_KICK, ECArgs(pl->Nick(), parv.size() > 2 ? parv[2] : ""));

		pl->Client()->ClrPlayer();
	}

	TClient* victim = pl->Client();
	pl->Channel()->RemovePlayer(pl, USE_DELETE);

	if(victim)
		victim->sendrpl(victim->Nick(), MSG_LEAVE);

	return 0;
}

/** A player wants to send a message to his allies.
 *
 * Syntax: AMSG message
 */
int AMSGCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(!cl->Player())
		return vDebug(W_DESYNCH, "AMSG en dehors d'un salon", VSName(cl->GetNick()) VPName(cl->Player()));

	ECArgs args(parv[1]);

	BPlayerVector pls = cl->Player()->Allies();
	for(BPlayerVector::const_iterator it=pls.begin(); it != pls.end(); ++it)
	{
		if(!(dynamic_cast<ECPlayer*> (*it))->Client()) continue;

		(dynamic_cast<ECPlayer*> (*it))->Client()->sendrpl(cl->Nick(), MSG_AMSG, args);
	}
	return 0;
}

/** A player wants to send a message to channel.
 *
 * Syntax: MSG message
 */
int MSGCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(!cl->Player())
		return vDebug(W_DESYNCH, "MSG en dehors d'un salon", VSName(cl->GetNick()) VPName(cl->Player()));

	cl->Player()->Channel()->sendto_players(cl->Player(), ToVec(cl->Player()), MSG_MSG, parv[1]);
	return 0;
}

/** A player wants to set an attribute in channel.
 *
 * Syntax: SET modes [params ...]
 */
int SETCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(!cl->Player() || !cl->Player()->Channel())
		return Debug(W_DESYNCH, "SET en dehors d'un chan");

	ECPlayer *sender = cl->Player();

	bool add = true;
	bool ready = false;
	const char NEEDREADY_ALL = 1;
	const char NEEDREADY_ME = 2;
	char need_ready = 0;
	uint j = 2;
	ECMap* map = 0;
	uint mapi = 0;

	std::string modes;
	ECArgs params;
	bool last_add = true;

	{
		BPlayerVector plv = sender->Channel()->Players();
		for(BPlayerVector::iterator pli = plv.begin(); pli != plv.end(); ++pli)
		{
			ECPlayer* pl = dynamic_cast<ECPlayer*>(*pli);
			if(pl->Client())
				pl->Client()->Lock();
		}
	}

	for(uint i=0; i<parv[1].size(); i++)
	{
		char changed = 0;
		const char YES_NOPARAMS = 1;
		const char YES_WITHPARAM = 2;
		switch(parv[1][i])
		{
			case '+': add = true; break;
			case '-': add = false; break;
			case 'd':
			{
				if(!add)
				{
					Debug(W_DESYNCH, "SET -d: interdit.");
					break;
				}
				if(sender->Channel()->State() != EChannel::PLAYING)
				{
					Debug(W_DESYNCH, "SET +d: channel non +P");
					break;
				}
				std::string s = parv[j++];
				if(s.find(':') == std::string::npos)
				{
					Debug(W_DESYNCH, "SET +d '%s': chaine invalide", s.c_str());
					break;
				}
				std::string nick = stringtok(s, ":");
				ECPlayer* pl = sender->Channel()->GetPlayer(nick.c_str());
				if(!pl)
				{
					Debug(W_DESYNCH, "SET +d '%s:%s': player introuvable", nick.c_str(), s.c_str());
					break;
				}
				int value = StrToTyp<int>(s);
				if(value <= 0)
					break;
				if(sender->Money() < value)
					value = sender->Money();
				sender->DownMoney(value);
				pl->UpMoney(value);
				changed = YES_WITHPARAM;
				break;
			}
			case 'e':
			{
				if(!add)
				{
					Debug(W_DESYNCH, "SET -e: interdit.");
					break;
				}
				if(sender->Channel()->State() != EChannel::PLAYING)
				{
					Debug(W_DESYNCH, "SET +e: channel non +P");
					break;
				}
				std::string s = parv[j++];
				if(s.find(':') == std::string::npos)
				{
					Debug(W_DESYNCH, "SET +e '%s': chaine invalide", s.c_str());
					break;
				}
				std::string nick = stringtok(s, ":");
				ECPlayer* pl = sender->Channel()->GetPlayer(nick.c_str());
				if(!pl)
				{
					Debug(W_DESYNCH, "SET +e '%s:%s': player introuvable", nick.c_str(), s.c_str());
					break;
				}
				ECBCountry* country = sender->MapPlayer()->FindCountry(s.c_str());
				if(!country)
				{
					Debug(W_DESYNCH, "SET +e '%s:%s': country introuvable pour %s", nick.c_str(), s.c_str(), sender->GetNick());
					break;
				}
				country->ChangeOwner(pl->MapPlayer());
				changed = YES_WITHPARAM;
				break;
			}
			case 'v':
			{
				if(!add)
				{
					Debug(W_DESYNCH, "SET -v: interdit.");
					break;
				}
				if(!sender->Channel()->IsPinging())
				{
					Debug(W_DESYNCH, "SET +v: impossible hors du +Q (PINGING)");
					break;
				}
				if(j>=parv.size()) { Debug(W_DESYNCH, "SET +v: pas de nick"); break; }
				ECPlayer *pl = sender->Channel()->GetPlayer(parv[j++].c_str());
				if(!pl) { Debug(W_DESYNCH, "SET +v: %s non trouvé", parv[(j-1)].c_str()); break; }

				if(!pl->Disconnected() || pl->Lost())
				{
					Debug(W_DESYNCH, "SET +v: %s n'est pas deconnecté ou a déjà perdu.", pl->GetNick());
					break;
				}
				if(!pl->AddVote(sender))
				{
					Debug(W_DESYNCH, "SET +v: %s a déjà voté pour %s", sender->GetNick(), pl->GetNick());
					break;
				}
				/* Comme on a pas reçu un nombre en argument, on ne le fait pas envoyer plus bas mais tout de suite.
				 * Comme ça ça apparait avant l'expulsion éventuelle */
				sender->Channel()->send_modes(pl, "+v", TypToStr(pl->Votes()));
				uint h = sender->Channel()->NbHumains();
				uint r =   (h == 1) ? 1                // Il n'y a qu'un humain donc son seul vote compte
				                    : !(h%2) ? h/2     // Nombre pair d'humains donc la moitier doit voter pour
				                          : h/2+1;     // Nombre impair d'humains donc la moitier + 1 doit voter pour
				if(pl->Votes() >= r)
				{ /* EXPULSION */
					sender->Channel()->RemovePlayer(pl, false);
					sender->Channel()->CheckPinging();
				}
				break;
			}
			case 't':
			{
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %ct: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(!sender->IsPriv())
					return Debug(W_DESYNCH, "SET %c%c: d'un non privilégié", add ? '+' : '-', parv[1][i]);

				if(add)
				{
					if(j<parv.size())
					{
						sender->Channel()->SetTurnTime(StrToTyp<uint>(parv[j++]));
						changed = YES_WITHPARAM;
					}
					else
						Debug(W_DESYNCH, "SET +t: sans argument");
				}
				else
					Debug(W_DESYNCH, "SET -t: interdit.");
				break;
			}
			case 'b':
			{
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %cb: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(!sender->IsPriv())
					return Debug(W_DESYNCH, "SET %c%c: d'un non privilégié", add ? '+' : '-', parv[1][i]);

				if(add)
				{
					if(j<parv.size())
					{
						sender->Channel()->BeginMoney() = StrToTyp<uint>(parv[j++]);
						changed = YES_WITHPARAM;
					}
					else
						Debug(W_DESYNCH, "SET +b: sans argument");
				}
				else
					Debug(W_DESYNCH, "SET -b: interdit.");
				break;
			}
			case 'm':
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %cm: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(!sender->IsPriv())
					return Debug(W_DESYNCH, "SET %c%c: d'un non privilégié", add ? '+' : '-', parv[1][i]);

				if(add)
				{
					if(j<parv.size())
					{
						mapi = StrToTyp<uint>(parv[j++]);
						if(mapi >= (sender->Channel()->IsMission() ? MissionList : MapList).size())
							Debug(W_DESYNCH, "SET +m: de la map %d hors de la liste (%d)", mapi,
							                 (sender->Channel()->IsMission() ? MissionList : MapList).size());
						/* Pour éviter la mise de plusieurs maps successivement, on ne traite que la dernière
						 * donnée. Donc une fois qu'on a traité la validité du numero de la map, on le met dans
						 * une variable qui sera vérifié à la fin de la boucle.
						 * On ne rajoutera qu'à ce moment là le "+m" et le paramètre, donc pas de valeur attribuée
						 * à changed ici.
						 */
						else
							map = (sender->Channel()->IsMission() ? MissionList : MapList)[mapi];
					}
				}
				else
					Debug(W_DESYNCH, "SET -m: interdit.");
				break;
			case '!':
				/* Autorise seulement à se déclarer comme OK, ne peut en aucun cas retirer ce qu'il
				 * a dit par la suite.
				 */
				if(!add)
					{ Debug(W_DESYNCH, "SET %c%c: interdit.", add ? '+' : '-', parv[1][i]); break; }

				if(sender->Ready())
				{
					Debug(W_WARNING, "SET +%c: sender->Ready()=TRUE", parv[1][i]);
					break;
				}
				if(sender->Channel()->State() == EChannel::PINGING)
				{
					Debug(W_WARNING, "SET +!: interdit en mode +Q (pinging)");
					break;
				}
				if(!sender->Channel()->Map())
				{
					vDebug(W_DESYNCH, "SET +!: alors qu'il n'y a pas de map", VPName(sender->Channel()->Map()));
					break;
				}
				if(sender->Channel()->Joinable() && (!sender->Channel()->Owner() || !sender->Channel()->Owner()->Ready())
				   && !sender->IsOwner())
				{
					BPlayerVector::iterator it;
					BPlayerVector plv = sender->Channel()->Players();
					uint nok = 0;
					for(it = plv.begin(); it != plv.end(); ++it) if((*it)->Ready()) ++nok;
					if(nok >= (sender->Channel()->Map()->MaxPlayers() - 1))
						break; // C'est pas forcément une erreur
				}

				if(!sender->Channel()->Joinable() || !sender->IsIA())
					ready = true;
				sender->SetReady(add);
				if(need_ready == NEEDREADY_ME) need_ready = 0;
				changed = YES_NOPARAMS;
				break;
			case 'r':
			{
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %cr: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(!sender->IsPriv())
				{
					Debug(W_DESYNCH, "SET %cr: d'un non privilégié", add ? '+' : '-');
					break;
				}
				sender->Channel()->SetFastGame(add);
				changed = YES_NOPARAMS;
				break;
			}
			case 'o':
			{
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %co: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(!sender->IsOwner())
					{ Debug(W_DESYNCH, "SET %c%c: d'un non owner", add ? '+' : '-', parv[1][i]); break; }
				if(j>=parv.size()) { Debug(W_DESYNCH, "SET %co: pas de nick", add ? '+' : '-'); break; }
				ECPlayer *pl = sender->Channel()->GetPlayer(parv[j++].c_str());
				if(!pl) { Debug(W_DESYNCH, "SET %co: %s non trouvé", add ? '+' : '-', parv[(j-1)].c_str()); break; }
				if(pl->IsOwner())
				{
					Debug(W_DESYNCH, "SET %co: %s est owner et ne peut pas être op.", parv[(j-1)].c_str());
					break;
				}
				pl->SetOp(add);
				changed = YES_WITHPARAM;
				break;
			}
			case 'a':
			{
				if(sender->Channel()->IsMission() && !sender->IsIA())
				{
					Debug(W_DESYNCH, "SET %ca: impossible dans une mission de la part d'un humain", add ? '+' : '-');
					break;
				}
				if(j>=parv.size()) { Debug(W_DESYNCH, "SET %ca: pas de nick", add ? '+' : '-'); break; }
				ECPlayer *pl = sender->Channel()->GetPlayer(parv[j++].c_str());
				if(!pl) { Debug(W_DESYNCH, "SET %ca: %s non trouvé", add ? '+' : '-', parv[(j-1)].c_str()); break; }
				changed = YES_WITHPARAM;
				if(add)
				{
					if(sender->IsAllie(pl))
						changed = 0;
					else
						sender->AddAllie(pl);
				}
				else if(!sender->RemoveAllie(pl))
					changed = 0; // En cas de lag, si on reçoit deux fois de suite un -a, ça peut etre long..
				break;
			}
			case 'n':
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %cn: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(add)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH, "SET +n: sans nation"); break; }
					uint nation = StrToTyp<uint>(parv[j++]);
					if(nation > 0 && nation != sender->Nation())
					{
						if(nation >= ECPlayer::N_MAX)
						{
							Debug(W_DESYNCH, "SET +n %d >= %d(maxcouleur)", nation, ECPlayer::N_MAX);
							break;
						}
						BPlayerVector::iterator it;
						BPlayerVector plv = sender->Channel()->Players();
						for(it = plv.begin(); it != plv.end() && (*it)->Nation() != nation; ++it);
						if(it != plv.end())
							break; /* Nation déjà utilisée */
					}
					sender->SetNation(nation);
					if(!need_ready) need_ready = NEEDREADY_ME;
					changed = YES_WITHPARAM;
				}
				else
				{
					sender->SetNation(ECPlayer::N_NONE);
					if(!need_ready) need_ready = NEEDREADY_ME;
					changed = YES_NOPARAMS;
				}
				break;
			case 'c':
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %cc: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(add)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH, "SET +c: sans couleur"); break; }
					uint color = StrToTyp<uint>(parv[j++]);
					if(color > 0 && color != sender->Color())
					{
						if(color >= COLOR_MAX)
						{
							Debug(W_DESYNCH, "SET +c %d >= %d(maxcouleur)", color, COLOR_MAX);
							break;
						}
						BPlayerVector::iterator it;
						BPlayerVector plv = sender->Channel()->Players();
						for(it = plv.begin(); it != plv.end() && (*it)->Color() != color; ++it);
						if(it != plv.end())
							break; /* Couleur déjà utilisée */
					}
					sender->SetColor(color);
					if(!need_ready) need_ready = NEEDREADY_ME;
					changed = YES_WITHPARAM;
				}
				else
				{
					sender->SetColor(COLOR_NONE);
					if(!need_ready) need_ready = NEEDREADY_ME;
					changed = YES_NOPARAMS;
				}
				break;
			case 'p':
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %cp: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(!sender->Channel()->Map())
				{
					Debug(W_DESYNCH, "SET %cp: alors qu'il n'y a pas de map", add ? '+' : '-');
					break;
				}
				if(sender->Channel()->IsMission())
				{
					Debug(W_DESYNCH, "SET %cp: impossible lors d'une mission", add?'+':'-');
					break;
				}
				if(add)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH, "SET +p: sans couleur"); break; }
					uint place = StrToTyp<uint>(parv[j++]);
					if(place > 0 && sender->Position() != place)
					{
						if(place > sender->Channel()->Limite())
						{
							Debug(W_DESYNCH, "SET +p %d > %d(limite)", place,
							                 sender->Channel()->Limite());
							break;
						}
						BPlayerVector::iterator it;
						BPlayerVector plv = sender->Channel()->Players();
						for(it = plv.begin(); it != plv.end() && (*it)->Position() != place; ++it);
						if(it != plv.end())
							break; /* Position déjà prise */
					}
					sender->SetPosition(place);
					if(!need_ready) need_ready = NEEDREADY_ME;
					changed = YES_WITHPARAM;
				}
				else
				{
					sender->SetPosition(0);
					if(!need_ready) need_ready = NEEDREADY_ME;
					changed = YES_NOPARAMS;
				}
				break;
			default:
				Debug(W_DESYNCH, "SET %c%c: Reception d'un mode non supporté", add ? '+' : '-', parv[1][i]);
				break;
		}
		if(changed)
		{
			if(modes.empty() || last_add != add)
				modes += add ? '+' : '-';
			last_add = add;
			modes += parv[1][i];
			if(changed == YES_WITHPARAM)
				params += parv[(j-1)];
		}
	}

	if(map && (!sender->Channel()->Map() || mapi != sender->Channel()->Map()->Num()))
	{
		sender->Channel()->SetMap(map);
		need_ready = NEEDREADY_ALL;
		if(modes.empty() || !last_add)
		{
			modes += '+';
			last_add = true;
		}
		modes += 'm';
		params += TypToStr(mapi);
		if(map->MaxPlayers() > app.GetConf()->DefLimite())
			map->MaxPlayers() = app.GetConf()->DefLimite();
		if(sender->Channel()->Limite() != map->MaxPlayers())
		{
			modes += 'l';
			params += TypToStr(map->MaxPlayers());
		}
	}

	if(!modes.empty())
	{
		ECArgs args(modes);
		args += params;
		sender->Channel()->sendto_players(0, ToVec(cl->Player()), MSG_SET, args);
	}

	if(map && sender->Channel()->Limite() != map->MaxPlayers())
		sender->Channel()->SetLimite(map->MaxPlayers());

	if(need_ready)
	{
		switch(need_ready)
		{
			case NEEDREADY_ALL: sender->Channel()->NeedReady(); break;
			case NEEDREADY_ME:  sender->NeedReady(); break;
		}
		ready = false;
	}

	/* Si tout le monde est READY, on passe d'un etat de la partie à un autre. */
	if(ready)
		sender->Channel()->CheckReadys();

	{
		BPlayerVector plv = sender->Channel()->Players();
		for(BPlayerVector::iterator pli = plv.begin(); pli != plv.end(); ++pli)
		{
			ECPlayer* pl = dynamic_cast<ECPlayer*>(*pli);
			if(pl->Client())
				pl->Client()->UnLock();
		}
	}

	return 0;
}

/** A client wants to join a channel.
 *
 * Syntax: JOI [nom|.] [$]
 */
int JOICommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	/* Ne peut être que sur un seul salon à la fois */
	if(cl->Player() || parv[1].empty())
		return vDebug(W_WARNING, "JOI: Essaye de joindre plusieurs salons", VSName(cl->GetNick())
		                          VSName(cl->Player()->Channel()->GetName()) VName(parv[1]));

	bool create = (parv.size() == 3 && parv[2] == "$"), mission = false;
	EChannel* chan = NULL;
	ECPlayer* pl;

	if(parv[1][0] == '.' && parv[1].size() == 1)
		mission = true;
	else if(parv[1].size() > GAMELEN)
		parv[1] = parv[1].substr(0,GAMELEN);

	for(ChannelVector::iterator it=ChanList.begin(); it != ChanList.end(); ++it)
		if((*it)->State() != EChannel::SCORING && !strcasecmp((*it)->GetName(), parv[1].c_str()))
		{
			chan = *it;
			break;
		}

	if(!chan && create)
	{ /* Création du salon */

		/* Il y a trop de parties */
		if(ChanList.size() >= app.GetConf()->MaxGames())
			return cl->sendrpl(ERR_CANT_CREATE);

		if(!mission)
			for(std::string::iterator c = parv[1].begin(); c != parv[1].end(); ++c)
				if(!strchr(CHAN_CHARS, *c))
				{
					vDebug(W_WARNING, "JOI: Le nom donné est incorrect", parv[1]);
					return cl->sendrpl(ERR_CANT_CREATE);
				}

		chan = new EChannel(parv[1], mission);
		pl = new ECPlayer(cl, chan, true, false);
		ChanList.push_back(chan);
		chan->SetOwner(pl);
	}
	else
	{ /* Rejoins un salon existant */
		if(mission || create || !chan)
			return cl->sendrpl(ERR_CANT_JOIN);

		if(chan->State() == EChannel::PINGING)
		{
			BPlayerVector pls = chan->Players();
			BPlayerVector::iterator ppl;
			for(ppl = pls.begin(); ppl != pls.end() && (!(*ppl)->CanRejoin() || (*ppl)->Nick() != cl->Nick()); ++ppl);

			/* Il n'est pas parmis ceux qu'on attend */
			if(ppl == pls.end())
				return cl->sendrpl(ERR_CANT_JOIN);

			pl = dynamic_cast<ECPlayer*>(*ppl);
			pl->SetDisconnected(false);
			pl->SetClient(cl);
			app.MSet("-r", pl->Nick());
		}
		else
		{
			if(!chan->Joinable() || chan->IsMission())
			{
				vDebug(W_WARNING, "JOI: Le client essaye de joindre un salon en jeu ou une mission", VSName(chan->GetName())
								VSName(cl->GetNick()) VIName(chan->State()) VName(chan->Name()));
				return cl->sendrpl(ERR_CANT_JOIN);
			}
			if(chan->Limite() && chan->NbPlayers() >= chan->Limite())
				return cl->sendrpl(ERR_CANT_JOIN);
			pl = new ECPlayer(cl, chan, false, false);
		}
	}
	cl->SetPlayer(pl);

	/** @page JOIN_MSGS Join's messages
	 *
	 * Messages sent on a join are :
	 * <pre>
	 * :me JOI chan
	 * LSM map1 1 5
	 * LSM map2 1 5
	 * EOM
	 * :server SET +lWm
	 * SMAP ligne1
	 * SMAP ligne2
	 * EOSMAP
	 * PLIST 0,0,me \@1,5prout
	 * </pre>
	 * @date 17/02/2006
	 */

	if(chan->State() == EChannel::PINGING)
	{
		chan->sendto_players(pl, ToVec(pl), MSG_SET, "-w");
		cl->sendrpl(cl->Player(), MSG_JOIN, chan->GetName());
	}
	else
		chan->sendto_players(0, ToVec(cl->Player()), MSG_JOIN, chan->GetName());

	for(MapVector::iterator it = (mission?MissionList:MapList).begin(); it != (mission?MissionList:MapList).end(); ++it)
		cl->sendrpl(MSG_LISTMAPS, ECArgs((*it)->Name(),
		                                 TypToStr((*it)->MinPlayers()),
		                                 TypToStr((*it)->MaxPlayers()),
		                                 (*it)->MapInfos().empty() ? "" : (*it)->MapInfos().front()));

	cl->sendrpl(MSG_ENDOFMAPS);

	cl->sendrpl(app.GetConf()->ServerName(), MSG_SET, ECArgs(chan->ModesStr()).DontSplit());

	if(chan->Map())
	{ /* Si la map existe on l'envoie */
		std::vector<std::string> map_file = chan->Map()->MapFile();
		for(std::vector<std::string>::iterator it = map_file.begin(); it != map_file.end(); ++it)
		{
			const char* c = (*it).c_str();
			// Pour ne pas avoir à modifier toutes les maps, UNIT reste sans '_'
			if(*c == '_' || !strncmp(c, "UNIT", 4)) continue;
			cl->sendrpl(MSG_SENDMAP, *it);
		}

		cl->sendrpl(MSG_ENDOFSMAP);
	}

	cl->sendrpl(MSG_PLIST, ECArgs(chan->PlayerList()).DontSplit());

	if(chan->IsMission())
	{
		pl->SetPosition(1);
		chan->send_modes(ToVec(pl), "+p", "1");
	}

	if(chan->IsPinging())
	{
		/* On avertit le joueur de tous les autres +w (disconnected), ainsi que de ceux
		 * qui sont alliés avec vous. */
		BPlayerVector players = chan->Players();
		for(BPlayerVector::iterator it = players.begin(); it != players.end(); ++it)
		{
			if((*it)->CanRejoin())
				cl->sendrpl(*it, MSG_SET, "+w");
			if((*it)->IsAllie(pl))
				cl->sendrpl(*it, MSG_SET, ECArgs("+a", pl->Nick()));
		}
		/* Et ceux avec qui vous etes alliés */
		players = pl->Allies();
		for(BPlayerVector::const_iterator it = players.begin(); it != players.end(); ++it)
			cl->sendrpl(pl, MSG_SET, ECArgs("+a", (*it)->Nick()));

		/* On se met en mode +S pour que le joueur se synch */
		cl->sendrpl(app.GetConf()->ServerName(), MSG_SET, "-Q+S");

		/* On envoie les entitées */
		chan->SendEntities(pl);

		/* On envoie les propriétés */
		std::vector<ECBCountry*> cntys = chan->Map()->Countries();
		FORit(ECBCountry*, cntys, cnty)
			if((*cnty)->Owner() && (*cnty)->Owner()->Player())
				cl->sendrpl((*cnty)->Owner()->Player(), MSG_SET, ECArgs("+@", (*cnty)->ID()));

		/* Et l'argent */
		cl->sendrpl(cl->Player(), MSG_SET, ECArgs("+$", TypToStr(pl->Money())));

		/* On repasse en mode +Q dans le cas où y a encore pinging.
		 * Le problème provient si CheckPinging() retourne true. En effet, ça enverrait :
		 *  R - :menareants.coderz.info SET -S+Q
		 *  R - :menareants.coderz.info SET -Q+P
		 * Le problème bien sur est que CheckPinging() ne sait pas si le client était +S ou +Q...
		 * Il est à noter tout de même que les -<maj> ne servent à RIEN, dans le sens où c'est un enum, et
		 * que le client s'en branle. Simplement c'est par conformité au protocole et par cohérence.
		 */

		if(pl->Ready())
			cl->sendrpl(pl, MSG_SET, "+!");
		cl->sendrpl(app.GetConf()->ServerName(), MSG_SET, "-S+Q");
		chan->CheckPinging();
	}

	Debug(W_CONNS, "-> %s@%s rejoint la partie %s", cl->GetNick(), cl->GetIp(), chan->GetName());

	return 0;
}

/** A player wants to leave a channel.
 *
 * Syntax: LEA
 */
int LEACommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	/* N'est pas dans un salon */
	if(!cl->Player())
		return vDebug(W_DESYNCH, "LEA en dehors d'un salon", VSName(cl->GetNick()) VPName(cl->Player()));

	EChannel *chan = cl->Player()->Channel();

	Debug(W_CONNS, "<- %s@%s quitte la partie %s", cl->GetNick(), cl->GetIp(), chan->GetName());

	if(chan->Joinable() && cl->Player()->IsOwner() || chan->NbHumains() == 1)
	{ /* L'owner s'en vas, le chan se clos,
	   * ou alors il n'y a plus que des IA sur le chan
	   */
		chan->ByeEveryBody();
		delete chan;
	}
	else
	{
		if(cl->Player()->IsOwner())
			chan->SetOwner(0);
		/** \note Comme le joueur a été supprimé d'abord, on envoit à tous les joueurs et on envoie à coté
		  *       au quitter car il n'est plus dans la liste.
		  *       La raison pour laquelle on remove AVANT de LEAVE c'est dans le cas où le salon est en jeu,
		  *       il faut que les suppressions d'unités et neutralités arrivent AVANT la suppression du
		  *       joueur chez les clients
		  */
		chan->RemovePlayer(cl->Player(), USE_DELETE);
		// Note: le sendto_players est dans RemovePlayer, mais on envoie quand meme le LEA à cl qui n'a rien reçu
		cl->sendrpl(cl->Nick(), MSG_LEAVE);
		if(!chan->NbPlayers())
			delete chan;

		cl->ClrPlayer();
	}

	return 0;
}

/** A player wants to list all channels.
 *
 * Syntax: LSP
 */
int LSPCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	for(ChannelVector::iterator it=ChanList.begin(); it != ChanList.end(); ++it)
	{
		if((*it)->IsMission() || (*it)->State() == EChannel::SCORING) continue;
		cl->sendrpl(MSG_GLIST, ECArgs((*it)->Name(),
		                              (*it)->Joinable() ? "+" : "-",
		                              TypToStr((*it)->NbPlayers()),
		                              TypToStr((*it)->Limite()),
		                              (*it)->Map() ? (*it)->Map()->Name() : ""));
	}

	return cl->sendrpl(MSG_ENDOFGLIST);
}

/********************************************************************************************
 *                               EPlayer                                                    *
 ********************************************************************************************/

ECPlayer::ECPlayer(TClient *_client, EChannel *_chan, bool _owner, bool _op)
	: ECBPlayer(_client->Nick(), _chan, _owner, _op), client(_client)
{

}

void ECPlayer::NeedReady()
{
	if(!Ready()) return;

	SetReady(false);
	Channel()->send_modes(this, "-!");

	return;
}

void ECPlayer::SetMoney(int m)
{
	ECBPlayer::SetMoney(m);
	if(Client())
		client->sendrpl(Nick(), MSG_SET, ECArgs("+$", TypToStr(m)));
}

bool ECPlayer::IsIA() const
{
	return (client && client->IsIA());
}

std::vector<TClient*> ECPlayer::ClientAllies() const
{
	std::nrvector<TClient*> players;

	for(BPlayerVector::const_iterator it = allies.begin(); it != allies.end(); ++it)
	{
		ECPlayer* pl = dynamic_cast<ECPlayer*>(*it);
		if(pl->Client())
			players.push_back(pl->Client());
	}
	return players;
}

bool ECPlayer::AddVote(ECBPlayer* pl)
{
	for(BPlayerVector::iterator it = votes.begin(); it != votes.end(); ++it)
		if(*it == pl)
			return false;

	votes.push_back(pl);
	return true;
}

bool ECPlayer::RemoveBreakPoint(ECBCase* c)
{
	for (std::vector<BreakPoint>::iterator it = breakpoints.begin(); it != breakpoints.end(); )
	{
		if (it->c == c)
		{
			it = breakpoints.erase(it);
			return true;
		}
		else
			++it;
	}
	return false;
}

/********************************************************************************************
 *                               EChannel                                                   *
 ********************************************************************************************/

EChannel::EChannel(std::string _name, bool mission)
	: ECBChannel(_name, mission), owner(0), fast_game(true), begin_money(20000), first_playing(0), playing(0)
{
	ECBChannel::SetLimite(app.GetConf()->DefLimite()); /* Limite par default */
	app.NBchan++;
	app.NBwchan++;
	app.NBtotchan++;
	app.MSet("+wg", ECArgs(TypToStr(app.NBwchan), TypToStr(app.NBchan)));
}

EChannel::~EChannel()
{
	app.NBchan--;
	if(State() == EChannel::WAITING) app.NBwchan--;
	else app.NBachan--;

	app.MSet("+wg", ECArgs(TypToStr(app.NBwchan), TypToStr(app.NBchan)));

	for(BPlayerVector::iterator pl = players.begin(); pl != players.end(); ++pl)
		if((*pl)->CanRejoin())
			app.MSet((*pl)->Nick(), "-r");

	for (ChannelVector::iterator it = ChanList.begin(); it != ChanList.end(); )
	{
		if (*it == this)
		{
			it = ChanList.erase(it);
			return;
		}
		else
			++it;
	}
}

void EChannel::CheckReadys()
{
	uint c = 0;
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
		if((*it)->Ready() || (*it)->Lost() || (*it)->Disconnected())
			c++; // On s'attend pas à ce qu'un joueur qui ait perdu soit pret :)
	if(c == NbPlayers() || State() == EChannel::WAITING)
	{
		switch(State())
		{
			case EChannel::WAITING:
			{
				if(!Map() || !Owner()->Ready())
					break;

				if(c < Map()->MinPlayers())
				{
					if(!IsMission()) break;

					BMapPlayersVector mpv = Map()->MapPlayers();
					for(BMapPlayersVector::iterator it=mpv.begin(); it != mpv.end(); ++it)
					{
						if(Owner()->Position() == (*it)->Num()) continue;

						TIA* IA = dynamic_cast<TIA*>(app.addclient(-1, ""));
						IA->SetNick(IA_CHAR + (*it)->Nick());
						SetAuth(IA);

						if(!IA->Join(this))
						{
							Debug(W_WARNING, "SET +!(starting game): Impossible de créer une IA");
							continue;
						}

						//IA->ia_send("SET +p " + TypToStr((*it)->Num()));
					}
				}

				/* Si le salon est une pré-partie et qu'il y a eu un +!, on vérifie que
				* si tout le monde est READY pour, dans ce cas là, lancer la partie
				* et jarter ceux qui sont en trop.
				*/
				std::list<uint> colors;
				for(uint i = 1; i < COLOR_MAX; ++i) colors.push_back(i);
				std::list<uint> nations;
				for(uint i = 1; i < ECPlayer::N_MAX; ++i) nations.push_back(i);
				std::list<uint> positions;
				for(uint i = 1; i <= Map()->MaxPlayers(); i++) positions.push_back(i);

				BMapPlayersVector mpv = Map()->MapPlayers();
				for(BPlayerVector::iterator it=players.begin(); it != players.end();)
					if(!(*it)->Ready())
					{
						TClient* cl = (dynamic_cast<ECPlayer*>(*it))->Client();
						sendto_players(0, *it, MSG_LEAVE);

						delete *it;
						it = players.erase(it);
						if(cl)
							cl->ClrPlayer();
						if(!NbPlayers())
						{
							delete this;
							throw ECExcept("", "Pourquoi passe-t-on par là ?");
						}
					}
					else
					{
						if((*it)->Position()) positions.remove((*it)->Position());
						if((*it)->Color()) colors.remove((*it)->Color());
						if((*it)->Nation()) nations.remove((*it)->Nation());
						++it;
					}
				for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
				{
					if(!(*it)->Position())
					{
						uint p = rand() % positions.size();
						std::list<uint>::iterator pos = positions.begin();
						for(uint i = 0; i != p && pos != positions.end(); ++pos, ++i);
						if(pos != positions.end())
							(*it)->SetPosition(*pos);
						else
							throw ECExcept(0, "Impossible d'attribuer une position aleatoirement !?");
						sendto_players(0, *it, MSG_SET, ECArgs("+p", TypToStr(*pos)));
						positions.erase(pos);
					}
					if(!(*it)->Color())
					{
						uint p = rand() % colors.size();
						std::list<uint>::iterator col = colors.begin();
						for(uint i = 0; i != p && col != colors.end(); ++col, ++i);
						if(col != colors.end())
							(*it)->SetColor(*col);
						else
							throw ECExcept(0, "Impossible d'attribuer une couleur aleatoirement !?");
						sendto_players(0, *it, MSG_SET, ECArgs("+c", TypToStr(*col)));
						colors.erase(col);
					}
					if(!(*it)->Nation())
					{
						uint p = rand() % nations.size();
						std::list<uint>::iterator nat = nations.begin();
						for(uint i = 0; i != p && nat != nations.end(); ++nat, ++i);
						if(nat != nations.end())
							(*it)->SetNation(*nat);
						else
							throw ECExcept(0, "Impossible d'attribuer une nation aleatoirement !?");
						sendto_players(0, *it, MSG_SET, ECArgs("+n", TypToStr(*nat)));
						nations.erase(nat);
					}
					BMapPlayersVector::iterator mpi;
					for(mpi = mpv.begin(); mpi != mpv.end() && (*it)->Position() != (*mpi)->Num(); ++mpi);
					if(mpi == mpv.end())
						throw ECExcept(VIName((*it)->Position()), "Position introuvable !?");
					(*mpi)->SetPlayer(*it);
					(*it)->SetMapPlayer(*mpi);
				}
				Map()->ClearMapPlayers();

				Debug(W_CONNS, "-- Lancement de la partie %s sur la map %s", GetName(), Map()->Name().c_str());
				SetState(EChannel::SENDING);
				sendto_players(0, app.ServerName(), MSG_SET, "-W+S");
				app.NBwchan--;
				app.NBachan++;
				app.MSet("+w", ECArgs(TypToStr(app.NBwchan)));

				for(BPlayerVector::iterator pl=players.begin();; ++pl)
				{
					std::vector<std::string> units = (pl==players.end() ? Map()->NeutralUnits() :
					                                                      (*pl)->MapPlayer()->Units());
					for(std::vector<std::string>::iterator it = units.begin(); it != units.end(); ++it)
					{
						std::string line = *it;
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

						const char *e_name = FindEntityName(pl == players.end() ? 0 : dynamic_cast<ECPlayer*>(*pl));
						ECEntity* entity = CreateAnEntity(StrToTyp<uint>(type), e_name, pl == players.end() ? 0 : *pl,
						                                  (*Map())(x,y));
						entity->SetNb(StrToTyp<uint>(number));
						Map()->AddAnEntity(entity);
						if(entity->IsHidden())
						{
							if(entity->Owner() && entity->Owner()->Client())
								SendArm(entity->Owner()->Client(), entity, ARM_CREATE, entity->Case()->X(),
								                                                       entity->Case()->Y());
						}
						else
							SendArm(NULL, entity, ARM_CREATE|ARM_HIDE, entity->Case()->X(), entity->Case()->Y());
					}
					if(pl == players.end())
						break;
					dynamic_cast<ECPlayer*>(*pl)->SetMoney(BeginMoney());
				}

				first_playing = rand()%players.size();

				NeedReady();

				break;
			}
			case EChannel::SENDING:
			{
				/* Le client est pret (a tout affiché, mémorisé, ...), la partie se lance donc et on
					* commence en PLAYING.
					*/
				SetState(EChannel::PLAYING);
				sendto_players(0, app.ServerName(), MSG_SET, "-S+P");
				NeedReady();
				break;
			}
			case EChannel::PLAYING:
			{
				/* Tous les clients ont fini de jouer. On va maintenant passer aux animations */
				SetState(EChannel::ANIMING);
				sendto_players(0, app.ServerName(), MSG_SET, "-P+A");

				/* Initialisation des animations */
				InitAnims();

				/* InitAnims() envoie déjà les CREATE */
				//NextAnim();

				NeedReady();
				break;
			}
			case EChannel::ANIMING:
			{
				/* Suite des animations, ou alors si il n'y en a plus on repasse en playing */
				if(dynamic_cast<ECMap*>(Map())->Events().empty())
				{ /* Plus d'animations */

					std::vector<ECBEntity*> entv = Map()->Entities()->List();
					std::map<ECBPlayer*, int> money;
					for(std::vector<ECBEntity*>::iterator enti = entv.begin(); enti != entv.end();)
					{
						if(dynamic_cast<ECEntity*>(*enti)->IsZombie())
						{
							ECList<ECBEntity*>::iterator it = enti;
							++it;
							Map()->RemoveAnEntity(*enti, USE_DELETE);
							enti = it;
						}
						else
						{
							(*enti)->Played(); /* On marque bien qu'il a été joué */
							for(BPlayerVector::iterator it = players.begin(); it != players.end(); ++it)
								money[*it] += (*enti)->TurnMoney(*it);
							++enti;
						}
					}
					/* On attribut à tout le monde son argent */
					for(BPlayerVector::iterator it = players.begin(); it != players.end(); ++it)
					{
						ECPlayer* pl = dynamic_cast<ECPlayer*>(*it);
						// Inutile en theorie, mais c'est vraiment une précaution.
						if(pl->Events()->Empty() == false)
						{
							FDebug(W_WARNING, "Il reste des evenements dans un player !");
							dynamic_cast<ECPlayer*>(*it)->Events()->Clear();
						}

						if(pl->Lost()) continue;
						int nb_units = 0;
						entv = pl->Entities()->List();
						for(std::vector<ECBEntity*>::iterator enti = entv.begin(); enti != entv.end(); ++enti)
						{
							/* Si le jeu est en fastgame, seules les batiments qui ne sont pas cachés et qui ne sont
							 * pas dans l'eau comptent pour rester en vie. Une fois qu'on les a perdu on a perdu.
							 */
							if(!(*enti)->IsHidden() && !(*enti)->IsTerrain() && ((*enti)->IsBuilding() && !(*enti)->IsNaval() || !FastGame()))
								nb_units++;
						}
						if(!nb_units)
						{
							sendto_players(0, pl, MSG_SET, "+_");
							pl->SetLost();
							for(std::vector<ECBEntity*>::iterator enti = entv.begin(); enti != entv.end();)
							{
								ECList<ECBEntity*>::iterator it = enti;
								++it;
								SendArm(NULL, dynamic_cast<ECEntity*>(*enti), ARM_REMOVE);
								Map()->RemoveAnEntity(*enti, USE_DELETE);
								enti = it;
							}
						}
						else
						{
							pl->CalculBestRevenu(money[*it]);
							pl->UpMoney(money[*it]);
						}
					}

					if(!CheckEndOfGame())
					{
						SetState(EChannel::PLAYING);
						if(!CheckPinging())
						{
							SetState(EChannel::PLAYING);
							sendto_players(0, app.ServerName(), MSG_SET, "-A+P");
						}
						Map()->NextDay();
					}
				}
				else
					NextAnim();
				NeedReady();
				break;
			}
			case EChannel::PINGING:
			case EChannel::SCORING:
			{
				/* on s'en branle il ne se passe rien, en SCORING tout ce qu'on attend d'un joueur c'est un LEA,
				 * et en PINGING on attend simplement de voter
				 */
				break;
			}
		}
	}
}

bool EChannel::CheckEndOfGame()
{
	if(State() != EChannel::PLAYING && State() != EChannel::ANIMING)
		return false;

	bool end_of_game = true;
	for(std::vector<ECBPlayer*>::iterator it = players.begin(); it != players.end() && end_of_game; ++it)
	{
		if((*it)->Lost()) continue;
		for(std::vector<ECBPlayer*>::iterator it2 = players.begin(); it2 != players.end() && end_of_game; ++it2)
		{
			if((*it2)->Lost()) continue;
			if(*it != *it2 && !(*it)->IsAllie(*it2))
				return false;
		}
	}

	SetState(EChannel::SCORING);
	sendto_players(0, app.ServerName(), MSG_SET, "-A+E");
	for(std::vector<ECBPlayer*>::iterator it = players.begin(); it != players.end(); ++it)
	{
		ECPlayer* pl = dynamic_cast<ECPlayer*>(*it);
		ECArgs args;
		args += TypToStr(pl->Stats()->killed);
		args += TypToStr(pl->Stats()->shooted);
		args += TypToStr(pl->Stats()->created);
		args += TypToStr(pl->Stats()->score);
		args += TypToStr(pl->Stats()->best_revenu);
		app.MSet(pl, "+kdcsrg", args);
		sendto_players(0, pl, MSG_SCORE, args);
	}
	return true;
}

void EChannel::SendArm(TClient* cl, std::vector<ECEntity*> et, uint flag, uint x, uint y, ECData data,
                       std::vector<ECEvent*> events)
{
	std::vector<TClient*> clv;
	if(cl)
		clv.push_back(cl);

	SendArm(clv, et, flag, x, y, data, events);
}

void EChannel::SendArm(std::vector<TClient*> cl, ECEntity* et, uint flag, uint x, uint y, ECData data,
                       std::vector<ECEvent*> events)
{
	std::vector<ECEntity*> plv;
	if(et)
		plv.push_back(et);

	SendArm(cl, plv, flag, x, y, data, events);
}

void EChannel::SendArm(TClient* cl, ECEntity* et, uint flag, uint x, uint y, ECData data,
                       std::vector<ECEvent*> events)
{
	std::vector<ECEntity*> plv;
	if(et)
		plv.push_back(et);

	std::vector<TClient*> clv;
	if(cl)
		clv.push_back(cl);

	SendArm(clv, plv, flag, x, y, data, events);
}

/* :<nick>!<arm> ARM [+<nb>] [*<pos>] [%<type>] [><pos>] [<[pos]] [-] [.] [{] [}] [(] [)] [~<type>,<data>] */
void EChannel::SendArm(std::vector<TClient*> cl, std::vector<ECEntity*> et, uint flag, uint x, uint y, ECData data,
                       std::vector<ECEvent*> events)
{
	if(!(flag &
	   (ARM_ATTAQ|ARM_MOVE|ARM_RETURN|ARM_TYPE|ARM_NUMBER|ARM_LOCK|ARM_REMOVE|ARM_DEPLOY|
	    ARM_CONTENER|ARM_UNCONTENER|ARM_DATA)))
		return;

	ECArgs to_send;
	std::string senders;

	if(flag & ARM_MOVE)
	{
		if(!events.empty())
		{
			for(std::vector<ECEvent*>::const_iterator it = events.begin(); it != events.end(); ++it)
			{
				if(!(*it)) continue; // Possible
				if((*it)->Flags() != ARM_MOVE && (*it)->Flags() != flag)
					SendArm(cl, (*it)->Entity(), ((*it)->Flags()|ARM_NOPRINCIPAL) & ~ARM_MOVE);
				if(!(*it)->Move()->FirstCase())
				{
					FDebug(W_WARNING, "Il n'y a pas de FirstCase !?");
					continue;
				}
				ECEntity* entity = (*it)->Entity();
				std::string s = "=" + entity->LongName() + "," + TypToStr((*it)->Move()->FirstCase()->X()) +
				                                           "," + TypToStr((*it)->Move()->FirstCase()->Y());
				if(!(*it)->Move()->Empty())
					s += "," + (*it)->Move()->MovesString(flag & ARM_ATTAQ ? (*Map())(x,y) : (*it)->Case());
				to_send += s;
			}
		}
		else
			for(std::vector<ECEntity*>::const_iterator it = et.begin(); it != et.end(); ++it)
				if(!(flag & ARM_TYPE) && !(*it)->Move()->Empty() && (*it)->Move()->FirstCase())
					to_send += "=" + (*it)->LongName() + "," + TypToStr((*it)->Move()->FirstCase()->X()) +
					                                     "," + TypToStr((*it)->Move()->FirstCase()->Y()) +
					                                     "," + (*it)->Move()->MovesString(0);
				else
					to_send += "=" + (*it)->LongName() + "," + TypToStr(x) +
					                                     "," + TypToStr(y);
	}
	if(flag & ARM_ATTAQ)
		to_send += "*" + TypToStr(x) + "," + TypToStr(y);
	else if(flag & ARM_RETURN)
		to_send += "<" + TypToStr(x) + "," + TypToStr(y);

	if(flag & ARM_TYPE)
	{
		if(et.empty()) FDebug(W_WARNING, "SendArm(ARM_TYPE): Il n'y a pas d'entité");
		else
			to_send += "%" + TypToStr(et.front()->Type());
	}
	if(flag & ARM_NUMBER)
	{
		if(flag & ARM_HIDE)
			to_send += "+";
		else if(et.empty()) FDebug(W_WARNING, "SendArm(ARM_NUMER): Il n'y a pas d'entité");
		else
			to_send += "+" + TypToStr(et.front()->Nb());
	}
	if(flag & ARM_LOCK)
		to_send += ".";
	if(flag & ARM_REMOVE)
		to_send += "-";
	if(flag & ARM_NOPRINCIPAL)
		to_send += "&";
	if((flag & ARM_DEPLOY) || ((flag & ARM_CREATE) && et.front()->Deployed()))
		to_send += (et.front()->Deployed()) ? "{" : "}";
	if(flag & ARM_CONTENER && !et.empty())
	{
		if(!et.front()->Parent()) FDebug(W_WARNING, "SendArm(ARM_CONTENER): L'entité n'est pas membre d'un contener");
		else
			to_send += ")" + et.front()->Parent()->LongName();
	}
	if(flag & ARM_UNCONTENER)
		to_send += "(";
	if(flag & ARM_DATA)
		to_send += "~" + TypToStr(data.type) + "," + data.data;
	if(flag & ARM_UPGRADE)
		to_send += "U";
	if(flag & ARM_INVEST)
		to_send += "@";

	/* Si c'est le joueur neutre qui envoie, c'est '*' le nom du player */
	if(!cl.empty())
	{
		for(std::vector<TClient*>::iterator it = cl.begin(); it != cl.end(); ++it)
			(*it)->sendrpl(et, MSG_ARM, to_send);
	}
	else
	{
		if(flag & ARM_RECURSE)
		{
			/* On envoie à chaques joueurs qui a envoyé un element sa version non cachée */
			for(BPlayerVector::iterator pl=players.begin(); pl != players.end(); ++pl)
			{
				if(dynamic_cast<ECPlayer*>(*pl)->Client() == 0) continue;
				std::vector<ECEntity*>::iterator it;
				for(it = et.begin();
				    it != et.end() && (*it)->Owner() != *pl && (!(*it)->Owner() || !(*it)->Owner()->IsAllie(*pl));
				    ++it);
				if(it != et.end())
					dynamic_cast<ECPlayer*>(*pl)->Client()->sendrpl(et, MSG_ARM, to_send);
			}
		}
		else if(flag & ARM_HIDE)
		{
			/* On envoie la version cachée uniquement aux joueurs qui n'ont pas d'entity incluses dans cette action. */
			for(BPlayerVector::iterator pl=players.begin(); pl != players.end(); ++pl)
			{
				if(dynamic_cast<ECPlayer*>(*pl)->Client() == 0) continue;
				std::vector<ECEntity*>::iterator it;
				for(it = et.begin();
				    it != et.end() && (*it)->Owner() != *pl && (!(*it)->Owner() || !(*it)->Owner()->IsAllie(*pl));
				    ++it);
				if(it == et.end())
					dynamic_cast<ECPlayer*>(*pl)->Client()->sendrpl(et, MSG_ARM, to_send);
			}
			if(!(flag & ARM_NOCONCERNED))
			{
				flag &= ~ARM_HIDE;
				SendArm(cl, et, flag|ARM_RECURSE, x, y, data, events);
			}
		}
		else
			sendto_players(NULL, et, MSG_ARM, to_send);
	}
}

void EChannel::ByeEveryBody(ECBPlayer* exception)
{
	for(BPlayerVector::iterator pi = players.begin(); pi != players.end(); ++pi)
	{
		if(!*pi || *pi == exception) continue;
		ECPlayer *p = dynamic_cast<ECPlayer*>(*pi);
		if(!p->Client()) continue;
		if(p->Client()->IsIA())
			app.delclient(p->Client());
		else
		{
			p->Client()->sendrpl(p, MSG_LEAVE);
			p->Client()->ClrPlayer();
		}
	}
}

const char* EChannel::FindEntityName(ECPlayer* pl)
{
	static Entity_ID num;

	assert(sizeof num == 3);

	num[0] = 'A';
	num[1] = 'A';
	num[2] = '\0';

	bool unchecked = false;
	BEntityVector ev = pl ? pl->Entities()->List() : Map()->Neutres()->List();

	do
	{
		BEntityVector::iterator it;
		for(it = ev.begin(); it != ev.end() && strcmp((*it)->ID(), num); it++);

		if(it == ev.end())
			unchecked = true;
		else
		{
			ev.erase(it);
			if(num[1] < 'Z') num[1]++;
			else if(num[0] >= 'Z') break;
			else
			{
				num[0]++;
				num[1] = 'A';
			}
		}
	} while(!unchecked);

	if(!unchecked)
		throw ECExcept(VBName(unchecked), "Il y a trop d'unités !!");

	return num;
}

void EChannel::SetMap(ECBMap *m)
{
	/** \note Send infos like this :
	 * <pre>
	 * :p1,p2 SET -p!
	 * SMAP line1
	 * SMAP line2
	 * [...]
	 * SMAP liney
	 * EOSMAP
	 * :p SET +ml 0 5
	 * </pre>
	 */

	if(Map())
	{
		delete Map();
		ECBChannel::SetMap(0);
	}

	if(!m) return;

	ECMap* _new_map = 0;
	try
	{
		_new_map = new ECMap(m->MapFile(), dynamic_cast<ECMap*>(m)->Num());
		_new_map->Init();
	}
	catch(TECExcept &e)
	{
		delete _new_map;
		vDebug(W_ERR, e.Message(), e.Vars());
		return;
	}

	ECBChannel::SetMap(_new_map);
	_new_map->SetChannel(this);

	/* Envoie d'une map */
	std::vector<std::string> map_file = Map()->MapFile();
	for(std::vector<std::string>::iterator it = map_file.begin(); it != map_file.end(); ++it)
	{
		const char* c = (*it).c_str();
		// Pour ne pas avoir à modifier toutes les maps, UNIT reste sans '_'
		if(*c == '_' || !strncmp(c, "UNIT", 4)) continue;
		sendto_players(NULL, 0, MSG_SENDMAP, *it);
	}
	sendto_players(NULL, 0, MSG_ENDOFSMAP);
	return;
}

void EChannel::SetLimite(unsigned int l)
{
	ECBChannel::SetLimite(l);

	PlayerVector plv;
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
		if((*it)->Position() > Limite())
		{
			(*it)->SetPosition(0);
			plv.push_back(dynamic_cast<ECPlayer*> (*it));
		}
	if(!plv.empty())
		send_modes(plv, "-p");
	return;
}

BPlayerVector::size_type EChannel::NbHumains() const
{
	BPlayerVector::size_type s = 0;
	for(BPlayerVector::const_iterator it=players.begin(); it != players.end(); ++it)
		if(!(*it)->IsIA() && dynamic_cast<ECPlayer*>(*it)->Client() != 0)
			++s;
	return s;
}

void EChannel::NeedReady()
{
	PlayerVector plv;
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
	{
		if((*it)->Ready())
		{
			(*it)->SetReady(false);
			plv.push_back(dynamic_cast<ECPlayer*> (*it));
		}
	}
	send_modes(plv, "-!");

	return;
}

ECPlayer *EChannel::GetPlayer(const char* nick)
{
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
		if(!strcasecmp((*it)->GetNick(), nick))
			return ((ECPlayer*) (*it));
	return NULL;
}

ECPlayer *EChannel::GetPlayer(TClient *cl)
{
	for(BPlayerVector::const_iterator it=players.begin(); it != players.end(); ++it)
		if((dynamic_cast<ECPlayer*> (*it))->Client() == cl)
			return ((ECPlayer*) (*it));

	return NULL;
}

void EChannel::send_modes(ECPlayer *sender, std::string modes, ECArgs args)
{
	PlayerVector plv;
	plv.push_back(sender);

	send_modes(plv, modes, args);
	return;
}

void EChannel::send_modes(PlayerVector senders, std::string modes, ECArgs a)
{
	if(senders.empty()) return;

	ECArgs args;
	args.Push(modes);
	args += a;

	sendto_players(NULL, senders, MSG_SET, args);
	return;
}

int EChannel::sendto_players(ECPlayer* one, ECBPlayer* pl, ECMessage cmd, ECArgs args)
{
	return sendto_players(one, pl ? pl->Nick() : "", cmd, args);
}

int EChannel::sendto_players(ECPlayer* one, std::vector<ECEntity*> from, ECMessage cmd, ECArgs args)
{
	std::string buf;

	for(std::vector<ECEntity*>::const_iterator it = from.begin(); it != from.end(); ++it)
	{
		if(it != from.begin()) buf += ",";
		buf += (*it)->LongName();
	}
	return sendto_players(one, buf, cmd, args);
}

int EChannel::sendto_players(ECPlayer* one, PlayerVector from, ECMessage cmd, ECArgs args)
{
	std::string buf;

	for(PlayerVector::const_iterator it = from.begin(); it != from.end(); ++it)
	{
		if(it != from.begin()) buf += ",";
		buf += (*it)->Nick();
	}
	return sendto_players(one, buf, cmd, args);
}

int EChannel::sendto_players(ECPlayer* one, std::string from, ECMessage cmd, ECArgs args)
{
	for(BPlayerVector::const_iterator it=players.begin(); it != players.end(); ++it)
	{
		if(!(dynamic_cast<ECPlayer*> (*it))->Client() || *it == one) continue;

		(dynamic_cast<ECPlayer*> (*it))->Client()->sendrpl(from, cmd, args);
	}
	return 0;
}

bool EChannel::RemovePlayer(ECBPlayer* ppl, bool use_delete)
{
	bool b = !IsInGame() ? ECBChannel::RemovePlayer(ppl, false) : true;

	if(!b) return false;

	ECPlayer* pl = dynamic_cast<ECPlayer*>(ppl);

	if(!Joinable())
	{
		std::vector<ECBEntity*> ents = pl->Entities()->List();
		for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
		{
			ECEntity* entity = dynamic_cast<ECEntity*>(*enti);
			entity->CancelEvents();
			SendArm(0, entity, ARM_REMOVE);
			entity->SetOwner(0);
			/* Ne pas passer par ECMap::RemoveAnEntity() pour éviter le temps perdu à le supprimer dans ECPlayer */
			entity->Case()->Entities()->Remove(entity);
			Map()->Entities()->Remove(entity);
			MyFree(entity);
		}
		if(pl->MapPlayer())
		{
			BCountriesVector conts = pl->MapPlayer()->Countries();
			for(BCountriesVector::iterator conti = conts.begin(); conti != conts.end(); ++conti)
			{
				(*conti)->SetOwner(0);
				send_modes(pl, "-@", (*conti)->ID());
			}
		}
	}
	pl->Entities()->Clear();

	if(!IsInGame())
	{
		if(pl->Client())
			sendto_players(0, pl, MSG_LEAVE);

		if(!IsInGame() && use_delete)
			delete pl;
	}
	else
	{
		send_modes(pl, "+_");
		if(pl->Client())
			pl->Client()->SetPlayer(0);
		pl->ClearClient();
		pl->SetLost();
	}

	if(State() != EChannel::PLAYING || !CheckEndOfGame())
		CheckReadys();

	return true;
}

/** \attention Lors de rajouts de modes, modifier API paragraphe 4. Modes */
std::string EChannel::ModesStr() const
{
	std::string     modes = "+",  params = "";
	                modes += "l", params = " " + TypToStr(Limite());
	if(Map())       modes += "m", params += " " + TypToStr(Map()->Num());
	                modes += "b", params += " " + TypToStr(begin_money);
	if(FastGame())  modes += "r";
	                modes += "t", params += " " + TypToStr(TurnTime());

	switch(State())
	{
		case WAITING: modes += "W"; break;
		case SENDING: modes += "S"; break;
		case PLAYING: modes += "P"; break;
		case ANIMING: modes += "A"; break;
		case PINGING: modes += "Q"; break;
		case SCORING: modes += "E"; break;
	}
	/* Pas d'espace nécessaire ici, rajouté à chaques fois qu'on ajoute un param */
	return (modes + params);
}

void EChannel::send_info (ECPlayer* pl, info_messages id, ECArgs args)
{
	ECArgs real_args;
	real_args += TypToStr(id);
	real_args += args;

	if(pl)
	{
		assert(pl->Client());
		pl->Client()->sendrpl(MSG_INFO, real_args);
	}
	else
		sendto_players(0, "", MSG_INFO, real_args);
}

void EChannel::SendEntities(ECPlayer* pl)
{
	if(!Map() || !pl->Client()) return;

	std::vector<ECBEntity*> ents = Map()->Entities()->List();

	/* Pour aller plus vite, sinon SendArm(TClient*, ...) va le faire à chaque fois */
	std::vector<TClient*> clients;
	clients.push_back(pl->Client());

	for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
	{
		if((*enti)->IsHidden() && (*enti)->Owner() != pl) continue;
		ECEntity* entity = dynamic_cast<ECEntity*>(*enti);

		if(entity->Parent())
		{
			SendArm(clients, entity, entity->Owner() == pl ? ARM_CREATE : (ARM_CREATE|ARM_HIDE),
			                         entity->Parent()->Case()->X(), entity->Parent()->Case()->Y());
			SendArm(clients, dynamic_cast<ECEntity*>(entity->Parent()), entity->Parent()->Owner() == pl ? ARM_CREATE : (ARM_CREATE|ARM_HIDE),
			                                                            entity->Parent()->Case()->X(), entity->Parent()->Case()->Y());
			SendArm(clients, entity, ARM_CONTENER);
		}
		else
			SendArm(clients, entity, entity->Owner() == pl ? ARM_CREATE : (ARM_CREATE|ARM_HIDE), entity->Case()->X(), entity->Case()->Y());

		entity->Resynch(pl);

		if(entity->Owner() == pl || entity->Owner() && entity->Owner()->IsAllie(pl))
		{
			std::vector<ECEvent*> events = entity->Events()->List();
			FOR(ECEvent*, events, event)
			{
				switch(event->Flags())
				{
					case ARM_ATTAQ:
						SendArm(clients, entity, ARM_ATTAQ, event->Case()->X(), event->Case()->Y());
						break;
					case ARM_MOVE:
					case ARM_CONTAIN:
					case ARM_UNCONTAIN:
						SendArm(clients, entity, ARM_MOVE, event->Case()->X(), event->Case()->Y(), 0, ToVec(event));
						break;
					case ARM_DEPLOY:
						SendArm(clients, event->Entity(), ARM_DEPLOY);
					default:
						break;
				}
			}
		}
	}
}

bool EChannel::CheckPinging()
{
	for(BPlayerVector::iterator pl = players.begin(); pl != players.end(); ++pl)
		if((*pl)->CanRejoin())
		{
			if(State() == PLAYING)
			{
				SetState(EChannel::PINGING);
				sendto_players(0, app.ServerName(), MSG_SET, "-P+Q");
			}
			return (State() == PINGING);
		}

	if(State() == PINGING)
	{
		SetState(EChannel::PLAYING);
		sendto_players(0, app.ServerName(), MSG_SET, "-Q+P");
		if(!CheckEndOfGame())
			CheckReadys();
	}
	return false;
}

