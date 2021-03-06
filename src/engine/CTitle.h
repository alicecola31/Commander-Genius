/*
 * CTitle.h
 *
 *  Created on: 03.10.2009
 *      Author: gerstrong
 *
 * This class manages the titlescreen of the game which
 * is very simple and maybe the class is a bit
 * overloaded for such task. Nevertheless we do it
 * that way because if more games are added,
 * it may get more complicated as the title may
 * differ from other passive modes.
 */


#ifndef CTITLE_H_
#define CTITLE_H_

#include <SDL.h>
#include <vector>
#include <memory>
#include "graphics/CGfxEngine.h"
#include "common/CSpriteObject.h"
#include "common/CMap.h"

class CTitle
{
public:

	CTitle(CMap &map);

	bool init(int Episode);
    void ponder();
    void render();

	bool isFinished()
	{ return mFinished; }

private:
	std::vector< std::unique_ptr<CSpriteObject> > mObjects;
	bool mFinished;
	unsigned int mTime;
	CMap &mMap;
};
#endif /* CTITLE_H_ */
