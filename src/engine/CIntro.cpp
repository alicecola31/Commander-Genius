/*
 * CIntro.cpp
 *
 *  Created on: 19.06.2009
 *      Author: gerstrong
 */

#include "CIntro.h"
#include "sdl/input/CInput.h"
#include "sdl/CVideoDriver.h"
#include "sdl/CTimer.h"

CIntro::CIntro()
{
	for(size_t i = 0; i < sizeof(mp_bm)/sizeof(mp_bm[0]); ++i) mp_bm[i] = NULL;
	m_timer = 0;
	m_introtime = 10;  // Total time (in seconds) to elapse until Main menu opens
	m_scrolly = 200;
	m_finished = false;
	mp_bmp_surface = g_pVideoDriver->mpVideoEngine->getBlitSurface();
	SDL_FillRect(mp_bmp_surface, NULL, 0);
}

void CIntro::init()
{
	// Load the Title Bitmap
	mp_bm[0] = g_pGfxEngine->getBitmap("AN");
	mp_bm[1] = g_pGfxEngine->getBitmap("APOGEE");
	mp_bm[2] = g_pGfxEngine->getBitmap("PRESENT");
	mp_bm[3] = g_pGfxEngine->getBitmap("OFAN");
	mp_bm[4] = g_pGfxEngine->getBitmap("IDSOFT");
	mp_bm[5] = g_pGfxEngine->getBitmap("PRODUCT");
	
	for(int j=0 ; j<6 ; j++)
	{
		if(mp_bm[j] == NULL)
			continue;
		if (j==0)
			m_mid[j] = (320/2)-((mp_bm[j]->getWidth()+4)/2);
		else
			m_mid[j] = (320/2)-(mp_bm[j]->getWidth()/2);
	}
	
	g_pTimer->ResetSecondsTimer();
}

void CIntro::ponder()
{
	if( m_timer<1 ) m_timer++;
	else
	{
		m_timer=0;
		if( m_scrolly>42 )	m_scrolly--;
	}

	
	if( g_pInput->getPressedAnyCommand() )
	{
	    m_finished = true;
	}
	
	// Check if time for Intro is out
	if(g_pTimer->HasSecElapsed())
	{
		if(m_introtime <= 0) m_finished = true;
		else m_introtime--;
	}
}

void CIntro::render()
{
    if(mp_bm[0] != NULL)
    {
        mp_bm[1]->draw( m_mid[1], m_scrolly+11);

        if(m_scrolly<=42) // Show this, when scrolling is finished
        {
            mp_bm[0]->draw( m_mid[0], m_scrolly);
            mp_bm[2]->draw( m_mid[2], m_scrolly+40);
            mp_bm[3]->draw( m_mid[3], m_scrolly+52);
            mp_bm[4]->draw( m_mid[4], m_scrolly+66);
            mp_bm[5]->draw( m_mid[5], m_scrolly+108);
        }
    }
}
