/*
 * CGamePlayMode.h
 *
 *  Created on: 26.03.2011
 *      Author: gerstrong
 */

#ifndef CGAMEPLAYMODE_H_
#define CGAMEPLAYMODE_H_

#include "CGameMode.h"
#include "engine/playgame/CPlayGame.h"
#include "fileio/CSaveGameController.h"
#include <memory>
#include <string>

class CGamePlayMode : public CGameMode
{
public:
	CGamePlayMode( const int Episode, const int Numplayers,
			const std::string& DataDirectory,
			const int startLevel = -1);

	CGamePlayMode( GMSwitchToPlayGameMode &gpmode );

	

	void init();

	void loadGame();

    void ponder();
    void render();

private:
	int m_startLevel;
	int m_Episode;
	Uint8 m_Numplayers;
	std::string m_DataDirectory;
	CSaveGameController m_SavedGame;
	std::unique_ptr<CPlayGame> mp_PlayGame;
	
	std::shared_ptr<SDL_Surface> mpFPSSurface;
};

#endif /* CGAMEPLAYMODE_H_ */
