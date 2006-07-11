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
#include <string>
#include <vector>
#include <map>

class ECPlayer;

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

	/** Send an unformated message. */
	int sendbuf(char* buf, int len) { return ia_recv(buf); }
	
	int ia_recv(std::string msg);
	void recv_msgs();
	int recv_one_msg(std::string msg);
	
	int ia_send(std::string msg) { return parsemsg(msg); }

	void FirstMovements();

/* Attributs */
public:

	virtual bool Locked() const { return lock; }
	virtual void Lock() { lock = true; }
	virtual void UnLock() { lock = false; recv_msgs(); }

/* Commandes */
private:
	static int SETCommand (std::vector<ECPlayer*> players, TIA *me, std::vector<std::string> parv);
	static int LEACommand (std::vector<ECPlayer*> players, TIA *me, std::vector<std::string> parv);
	static int ARMCommand (std::vector<ECPlayer*> players, TIA *me, std::vector<std::string> parv);
	bool lock;
	std::vector<std::string> msgs;

	std::map<int, uint> units;
};

#endif /* ECD_IA_H */
