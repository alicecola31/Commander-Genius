/*
 * CPlayGame.h
 *
 *  Created on: 31.01.2010
 *      Author: gerstrong
 */

#ifndef CPLAYGAMEGALAXY_H_
#define CPLAYGAMEGALAXY_H_

#include "../playgame/CPlayGame.h"

namespace galaxy
{

class CPlayGameGalaxy : public CPlayGame
{
public:
	CPlayGameGalaxy(CExeFile &ExeFile, char level,
			 char numplayers, char difficulty);

	bool loadGameState();
	void process();
	bool init();
	void cleanup();

	virtual ~CPlayGameGalaxy();
};

}

#endif /* CPLAYGAMEGALAXY_H_ */
