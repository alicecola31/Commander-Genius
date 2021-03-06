/*
 * COptions.cpp
 *
 *  Created on: 28.11.2009
 *      Author: gerstrong
 */

#include "COptions.h"
#include "sdl/input/CInput.h"
#include "sdl/CVideoDriver.h"
#include "common/CSettings.h"


COptions::COptions() :
CBaseMenu( CRect<float>(0.1f, 0.14f, 0.8f, NUM_OPTIONS*0.07f) ),
mpOption(g_pBehaviorEngine->m_option)
{

	for( int i = 0 ; i < NUM_OPTIONS ; i++ )
	{
		mpOptionList.push_back( new CGUISwitch(mpOption[i].menuname) );
		mpMenuDialog->addControl( mpOptionList.back() );
	}

	setMenuLabel("OPTIONSMENULABEL");
}

void COptions::init()
{
	std::list<CGUISwitch*>::iterator it = mpOptionList.begin();

	for( int i=0 ; it != mpOptionList.end() ; it++, i++ )
		(*it)->enable( mpOption[i].value );

}


void COptions::ponder()
{
    CBaseMenu::ponder();

    auto it = mpOptionList.begin();

	for( int i=0 ; it != mpOptionList.end() ; it++, i++ )
		mpOption[i].value = (*it)->isEnabled();
}

void COptions::release()
{
	g_pSettings->saveGameOptions();
}
