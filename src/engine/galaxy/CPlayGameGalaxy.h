/*
 * CPlayGame.h
 *
 *  Created on: 31.01.2010
 *      Author: gerstrong
 */

#ifndef CPLAYGAMEGALAXY_H_
#define CPLAYGAMEGALAXY_H_

#include "../playgame/CPlayGame.h"
#include "../../common/CMap.h"

namespace galaxy
{

class CPlayGameGalaxy : public CPlayGame
{
public:
	CPlayGameGalaxy(CExeFile &ExeFile, char level,
			 char numplayers, char difficulty,
			 stOption *p_option);

	bool loadGameState();
	void process();
	bool init();
	void cleanup();

	virtual ~CPlayGameGalaxy();

private:
	CMap m_Map;
};

}

#endif /* CPLAYGAMEGALAXY_H_ */