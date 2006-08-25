/* server/IA.h - Header of IA.cpp
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

#ifndef ECD_IA_H
#define ECD_IA_H

#include "Server.h"
#include "Outils.h"
#include <string>
#include <vector>
#include <map>

class ECPlayer;
class EChannel;
class ECBEntity;
class ECBCase;

/** Artificial Intelligence class based on TClient */
class TIA : public TClient
{
/* Constructeur/Destructeur */
public:
	TIA()
	    : TClient(-1, "127.0.0.bot"), lock(false)
	{}

/* Methodes */
public:

	virtual void RemoveEntity(ECBEntity* e);

	/** Send an unformated message. */
	int sendbuf(char* buf, int len) { return ia_recv(buf); }

	int ia_recv(std::string msg);
	void recv_msgs();
	int recv_one_msg(std::string msg);

	int ia_send(std::string msg) { return parsemsg(msg); }

	bool Join(EChannel* chan);

	void FirstMovements();
	void MakeAllies();
	void CheckIfIReady();
	void WantMoveTo(ECBEntity* enti, ECBCase* dest, uint nb_cases = 0);

/* Attributs */
public:

	virtual bool Locked() const { return lock; }
	virtual void Lock() { lock = true; }
	virtual void UnLock() { lock = false; recv_msgs(); }

/* Commandes */
private:
	static int SETCommand (std::vector<ECPlayer*> players, TIA *me, std::vector<std::string> parv);
	static int LEACommand (std::vector<ECPlayer*> players, TIA *me, std::vector<std::string> parv);

/* Classes privées */
private:

	class UseTransportBoat;
	/** Virtual class, might be upgraded by any other classes */
	class Strategy
	{
	public:

		Strategy(TIA* i) : ia(i) { assert(ia); }
		virtual ~Strategy()
		{
			std::vector<ECBEntity*> ents = entities.List();
			for(std::vector<ECBEntity*>::iterator it = ents.begin(); it != ents.end(); ++it)
				IA()->recruted[*it] = false;
		}

		/** Return false if we want to remove this strategy */
		virtual bool Exec() = 0;

		ECList<ECBEntity*>* Entities() { return &entities; }
		TIA* IA() const { return ia; }

		void AddEntity(ECBEntity* e)
		{
			entities.Add(e);
			IA()->recruted[e] = true;
		}
		void RemoveEntity(ECBEntity* e)
		{
			entities.Remove(e);
			IA()->recruted[e] = false;
		}

	private:
		TIA* ia;
		ECList<ECBEntity*> entities;
	};

/* Methodes privées */
private:

	void UseStrategy(Strategy* s, ECBEntity* e);
	void AddStrategy(Strategy* s) { strategies.push_back(s); }
	bool RemoveStrategy(Strategy*, bool use_delete = false);

/* Variables privées */
private:
	bool lock;
	std::vector<std::string> msgs;

	std::map<int, uint> units;

	std::vector<Strategy*> strategies;
	std::map<ECBEntity*, bool> recruted;
};

#endif /* ECD_IA_H */
