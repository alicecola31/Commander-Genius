/*
 * CBounder.cpp
 *
 *  Created on: 02.07.2011
 *      Author: gerstrong
 */

#include "CBounder.h"

namespace galaxy
{

enum BOUNDER_ACTION
{
A_BOUNDER_BOUNCE = 0,
A_BOUNDER_MOVE = 2,
A_BOUNDER_ONFLOOR = 4,
A_BOUNDER_STUNNED = 5
};

const int MAX_BOUNCE_BOOST = -115;
const int HOR_SPEED = 40;

CBounder::CBounder(CMap *pmap, const Uint16 foeID, Uint32 x, Uint32 y) :
CStunnable(pmap, foeID, x, y),
bounceboost(0),
mpInteractPlayer(NULL)
{
	mActionMap[A_BOUNDER_BOUNCE] = (GASOFctr) &CBounder::processBounce;
	mActionMap[A_BOUNDER_MOVE] = (GASOFctr) &CBounder::processBounce;
	mActionMap[A_BOUNDER_MOVE+1] = (GASOFctr) &CBounder::processBounce;
	mActionMap[A_BOUNDER_ONFLOOR] = (GASOFctr) &CBounder::processOnFloor;
	mActionMap[A_BOUNDER_STUNNED] = (GASOFctr) &CStunnable::processGettingStunned;

	setupGalaxyObjectOnMap(0x2F12, A_BOUNDER_BOUNCE);
	xDirection = 0;	
}


void CBounder::getTouchedBy(CSpriteObject &theObject)
{    
    if( !getActionStatus(A_BOUNDER_STUNNED) )
    {
	CStunnable::getTouchedBy(theObject);
    }

	if( CPlayerLevel *player = dynamic_cast<CPlayerLevel*>(&theObject) )
	{
		mpInteractPlayer = player;

		const int m_py2 = player->getYDownPos();
		const int m_y2 = getYUpPos()+(4<<STC);
		if( m_py2 <= m_y2 && !player->pSupportedbyobject && !player->m_jumpdownfromobject )
			player->pSupportedbyobject = this;

	}
	
	if(dead || theObject.dead)
		return;	

	// Was it a bullet? Then make it stunned.
	if( theObject.exists && dynamic_cast<CBullet*>(&theObject) )
	{
		setAction( A_BOUNDER_STUNNED );
		dead = true;
		theObject.dead = true;
	}
	
}

void CBounder::processBounce()
{
    	std::vector<CTileProperties> &TileProperty = g_pBehaviorEngine->getTileProperties();
	
	int xMid = getXMidPos();
	int y2 = getYDownPos();

	// When bounder hits the floor start the inertia again.
	if( blockedd || TileProperty[mp_Map->at(xMid>>CSF, (y2+1)>>CSF)].bup>1 )
	{
		setAction( A_BOUNDER_ONFLOOR );
		return;
	}

	// Will block the player when bounder touches him, but he is not riding him
	if( mpInteractPlayer )
	{
		if(!mpInteractPlayer->pSupportedbyobject)
		{
			const int mMidPX = mpInteractPlayer->getXMidPos();
			const int mMidX = getXMidPos();

			if( mMidPX > mMidX + (4<<STC) )
				mpInteractPlayer->blockedl = 1;
			else if( mMidPX < mMidX - (4<<STC) )
				mpInteractPlayer->blockedr = 1;
		}
	}

	if(xDirection == LEFT)
	{
		moveLeft(HOR_SPEED, false);
	}
	else if(xDirection == RIGHT)
	{
		moveRight(HOR_SPEED, false);
	}
}

void CBounder::processOnFloor()
{
	yinertia = MAX_BOUNCE_BOOST;
	playSound( SOUND_BOUNCE_LOW );

	// Decide whether go left, right or just bounce up.
	switch( rand() % 3 )
	{
	case 1:
		xDirection = LEFT;
		break;
	case 2:
		xDirection = RIGHT;
		break;
	default:
		xDirection = 0;
		break;
	}

	// If player is standing on bounder, he can control him a bit also
	if(mpInteractPlayer)
	{
		if(mpInteractPlayer->pSupportedbyobject)
		{
			const int mMidPX = mpInteractPlayer->getXMidPos();
			const int mMidX = getXMidPos();

			if( mMidPX > mMidX + (4<<STC) )
				xDirection = RIGHT;
			else if( mMidPX < mMidX - (4<<STC) )
				xDirection = LEFT;
			else
				xDirection = 0;
		}

	}

	switch(xDirection)
	{
	case LEFT:
		setAction( A_BOUNDER_MOVE+1 );
		break;
	case RIGHT:
		setAction( A_BOUNDER_MOVE );
		break;
	default:
		setAction( A_BOUNDER_BOUNCE );
		break;
	}
}

void CBounder::movePlatLeft(const int amnt)
{
	// First move the object on platform if any
	if(mpInteractPlayer)
	{
		if(!mpInteractPlayer->m_jumpdownfromobject && mpInteractPlayer->pSupportedbyobject)
			mpInteractPlayer->moveLeft(amnt);
	}
}

void CBounder::movePlatRight(const int amnt)
{
	// First move the object on platform if any
	if(mpInteractPlayer)
	{
		if(!mpInteractPlayer->m_jumpdownfromobject && mpInteractPlayer->pSupportedbyobject)
			mpInteractPlayer->moveRight(amnt);
	}
}

void CBounder::movePlayerUp(const int amnt)
{
	// First move the object on platform if any
	if(mpInteractPlayer)
	{
		if(!mpInteractPlayer->m_jumpdownfromobject && mpInteractPlayer->pSupportedbyobject)
		{
			mpInteractPlayer->moveUp(amnt);
		}
	}
}

void CBounder::movePlayerDown(const int amnt)
{
	// First move the object on platform if any
	if( mpInteractPlayer )
	{
		if(!mpInteractPlayer->m_jumpdownfromobject && mpInteractPlayer->pSupportedbyobject)
		{
			mpInteractPlayer->moveDown(amnt);
		}
	}
}





void CBounder::moveLeft(const int amnt, const bool force)
{
	CSpriteObject::moveLeft(amnt, force);
	movePlatLeft(amnt);
}

void CBounder::moveRight(const int amnt, const bool force)
{
	CSpriteObject::moveRight(amnt, force);
	movePlatRight(amnt);
}

void CBounder::moveUp(const int amnt)
{
	CSpriteObject::moveUp(amnt);
	movePlayerUp(amnt);
}

void CBounder::moveDown(const int amnt)
{
	CSpriteObject::moveDown(amnt);
	movePlayerDown(amnt);
}



void CBounder::process()
{
	// Bounce
	performCollisions();
	performGravityMid();

	(this->*mp_processState)();

	if( blockedl )
	{
		xDirection = RIGHT;
		setAction( A_BOUNDER_MOVE );
	}
	else if(blockedr)
	{
		xDirection = LEFT;
		setAction( A_BOUNDER_MOVE+1 );
	}

	// check if someone is still standing on the platform
	if( mpInteractPlayer )
	{
		if( !hitdetect(*mpInteractPlayer) )
		{
			mpInteractPlayer->pSupportedbyobject = nullptr;
			mpInteractPlayer->m_jumpdownfromobject = false;
			mpInteractPlayer = NULL;
		}
		else if(mpInteractPlayer->pSupportedbyobject)
		{

			const int m_py2 = mpInteractPlayer->getYDownPos();
			const int m_y2 = getYUpPos()+(2<<STC);
			const int moveY = m_y2 - m_py2;

			if( moveY < 0 )
				movePlayerUp(-moveY);
			else if( moveY > 0 )
				movePlayerDown(moveY);

		}
	}

	if(!processActionRoutine())
			exists = false;
}


};
