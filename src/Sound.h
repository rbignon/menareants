/* src/Sound.h - Header of Sound.cpp
 *
 * Copyright (C) 2005 Romain Bignon  <Progs@headfucking.net>
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

#ifndef EC_SOUND_H
#define EC_SOUND_H

#include <string>
#include <SDL_mixer.h>
#include <map>
#include <vector>

/** \warning With gdb there is some problems... */
class Sound
{
/* Static */
private:
	static bool init;
	static bool playing_music;
	static int frequency;
	static int channels;
	static std::map<int, Sound*> chunks;
	static void EndChunk(int channel);
	static void EndMusic();
	static void NextMusic(std::vector<Sound*>::iterator);
	static std::vector<Sound*> musics;

public:
	static void Init();
	static void End();
	static void StopAll();
	static int StopEffects();
	static void StopMusic();
	static void PlayMusic();
	static void NextMusic();
	static void SetMusicList(std::string path);
	static void EraseMusicList();

/* Constructeur/Destructeur */
public:

	Sound(std::string path, bool music = false);
	~Sound();

/* Methodes */
public:

	void Play(bool repeat = false);
	int Stop();
	void Free();

/* Attributs */
public:

	bool IsMusic() const { return is_music; }
	bool Playing() const { return playing; }

/* Variables privées */
private:
	bool is_music;
	std::string path;
	bool playing;
	int channel;
	Mix_Music* music;
	Mix_Chunk* chunk;
};

#endif /* EC_SOUND_H */
