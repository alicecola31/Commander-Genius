/*
 * CEffects.h
 *
 *  Created on: 14.12.2009
 *      Author: gerstrong
 *
 *  This special class is used for the graphical effects that are seen in the game.
 *  Some are normal effects seen in the DOS-Version of Commander Keen, other are
 *  new and others were partly included somewhere else in the past versions of CG.
 *  There were workarounds and now are well integrated.
 */

#ifndef CEFFECTS_H_
#define CEFFECTS_H_

#include <SDL.h>
#include "core/LogicDrawSplitter.h"

class CEffects : public GameState
{
public:
	CEffects() :
	mFinished(false) {}

	bool finished() { return mFinished; }
    void setFinished(const bool value) { mFinished = true; }

	virtual ~CEffects() {}

protected:
	bool mFinished;
};

#endif /* CEFFECTS_H_ */
