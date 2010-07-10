/*
 * CVorticonElite.h
 *
 *  Created on: 05.07.2010
 *      Author: gerstrong
 */

#ifndef CVORTICONELITE_H_
#define CVORTICONELITE_H_

#include "CVorticon.h"
#include "../../../common/CPlayer.h"
#include <vector>

// the "Vorticon Elite" enemies in ep2
enum vortelite_actions{
	VORTELITE_WALK, VORTELITE_JUMP,
	VORTELITE_ABOUTTOFIRE, VORTELITE_FIRED,
	VORTELITE_DYING, VORTELITE_DEAD,
};

class CVorticonElite : public CVorticon
{
public:
	CVorticonElite( CMap *p_map, std::vector<CPlayer> &mp_vec_Player,
			std::vector<CObject*> &mp_vec_Obj,
			Uint32 x, Uint32 y );
	void process();
	void initiatejump();
private:
	vortelite_actions state;

	unsigned char timer,timer2;
	unsigned int animtimer;
	unsigned char frame;
	signed int inertiay;
	unsigned char movedir;
	unsigned int timesincefire;

	int dist_traveled;
	std::vector<CObject*>& m_Object;
};

#endif /* CVORTICONELITE_H_ */