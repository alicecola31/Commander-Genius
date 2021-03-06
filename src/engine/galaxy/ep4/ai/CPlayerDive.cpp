/*
 * CPlayerDive.cpp
 *
 *  Created on: 19.06.2011
 *      Author: gerstrong
 */

#include "CPlayerDive.h"
#include "sdl/input/CInput.h"
#include "CBubbles.h"


namespace galaxy {

const int A_KEENSWIM_MOVE = 0;


CPlayerDive::CPlayerDive(CMap *pmap,
		const Uint16 foeID,
		Uint32 x,
		Uint32 y,
		direction_t facedir,
		CInventory &l_Inventory,
		stCheat &Cheatmode) :
CPlayerBase(pmap, foeID, x, y,
		facedir,
		l_Inventory,
		Cheatmode),
m_swimupspeed(0),
m_breathtimer(0)
{
	mActionMap[A_KEENSWIM_MOVE] = (void (CPlayerBase::*)()) &CPlayerDive::processDiving;

	setupGalaxyObjectOnMap(0x19EC, A_KEENSWIM_MOVE);
}

const int DIE_FALL_MAX_INERTIA = 150;

void CPlayerDive::kill(const bool force)
{
	// Here were prepare Keen to die, setting the action to die
	if(!m_Cheatmode.god || force)
	{
		if(mp_processState == &CPlayerBase::processDying && yinertia < 0)
			return;

		g_pSound->playSound( SOUND_KEEN_DIE, PLAY_NORESTART );
		setupGalaxyObjectOnMap(0x0D2E, (rand()%2));
		m_dying = true;
		yinertia = -DIE_FALL_MAX_INERTIA;
		solid = false;
		honorPriority = false;
		mp_processState = &CPlayerBase::processDying;
	}
}


const int MAXMOVESPEED = 20;
const int MOVESPEED = 30;
const int WATERFALLSPEED = 10;
const int BREATH_TIME = 60;

void CPlayerDive::processDiving()
{
	// In case no-clipping was triggered, make it solid, or remove it...
	if(m_Cheatmode.noclipping)
	{
		solid = !solid;
		m_Cheatmode.noclipping = false;
	}


	// If Player presses Jump button, make Keen swim faster
	if(g_pInput->getPressedCommand(IC_JUMP))
	{
		// Animate more Keen when pressing Jump button
		if(getActionNumber(A_KEENSWIM_MOVE))
			setActionForce(A_KEENSWIM_MOVE+1);
		else
			setActionForce(A_KEENSWIM_MOVE);

		if(m_swimupspeed<MAXMOVESPEED)
			m_swimupspeed = MAXMOVESPEED;
	}

	// Swimming - Left and Right
	if(g_pInput->getHoldedCommand(IC_LEFT))
	{
		if(!g_pInput->getHoldedCommand(IC_UP) && !g_pInput->getHoldedCommand(IC_DOWN))
			yDirection = 0;

		moveLeft(MOVESPEED+m_swimupspeed);
		xDirection = LEFT;
	}
	else if(g_pInput->getHoldedCommand(IC_RIGHT))
	{
		if(!g_pInput->getHoldedCommand(IC_UP) && !g_pInput->getHoldedCommand(IC_DOWN))
			yDirection = 0;

		moveRight(MOVESPEED+m_swimupspeed);
		xDirection = RIGHT;
	}

	// Up and down swimming
	if( g_pInput->getHoldedCommand(IC_UP))
	{
		moveUp(MOVESPEED+m_swimupspeed);
		yDirection = UP;
	}
	
	else if(g_pInput->getHoldedCommand(IC_DOWN))
	{
		moveDown(MOVESPEED+m_swimupspeed);
		yDirection = DOWN;
	}
	else
	{
		moveDown(WATERFALLSPEED+m_swimupspeed);
		yDirection = DOWN;
	}


	// Slow down the swim speed, by time
	if(m_swimupspeed>0)
	{
		m_swimupspeed--;
		if(m_swimupspeed<0)
			m_swimupspeed = 0;
	}

	if( m_breathtimer >= BREATH_TIME )
	{
		playSound(SOUND_BUBBLE);
		int dir_offset = (xDirection==RIGHT) ? +(1<<CSF) : -(1<<CSF) ;
		CBubbles *Bubble = new CBubbles(mp_Map, 0, getXMidPos()+dir_offset, getYMidPos(), false);
		g_pBehaviorEngine->m_EventList.add( new EventSpawnObject( Bubble ) );
		m_breathtimer = 0;
	}
	else
		m_breathtimer++;

	m_camera.process();
	m_camera.processEvents();
}

void CPlayerDive::process()
{
	(this->*mp_processState)();

	processLevelMiscFlagsCheck();

	if(!processActionRoutine())
			exists = false;
}

}
