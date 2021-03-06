/*
 * CGUISwitch.cpp
 *
 *  Created on: 04.03.2012
 *      Author: gerstrong
 */

#include "CGUISwitch.h"
#include "StringUtils.h"
#include "graphics/CGfxEngine.h"
#include "sdl/CVideoDriver.h"
#include "common/CBehaviorEngine.h"

CGUISwitch::CGUISwitch( const std::string& text ) :
CGUIComboSelection( text, filledStrList( 2, "off", "on" ) )
{

}

void CGUISwitch::drawVorticonStyle(SDL_Rect& lRect)
{
	if(!mEnabled)
		return;

	SDL_Surface *blitsfc = g_pVideoDriver->getBlitSurface();

	// Now lets draw the text of the list control
	CFont &Font = g_pGfxEngine->getFont(2);

	Font.drawFont( blitsfc, mText, lRect.x+24, lRect.y, false );
	Font.drawFont( blitsfc, ":", lRect.x+24+mText.size()*8, lRect.y, false );

	std::string text;
	if( (*mOLCurrent) == "off" )
		text = "\24\25\26\27";
	else
		text = "\34\35\36\37";


	Font.drawFont( blitsfc, text, lRect.x+24+(mText.size()+2)*8, lRect.y, false );

	drawTwirl(lRect);

}

void CGUISwitch::processRender(const CRect<float> &RectDispCoordFloat)
{

	// Transform to the display coordinates
	CRect<float> displayRect = mRect;
	displayRect.transform(RectDispCoordFloat);
	SDL_Rect lRect = displayRect.SDLRect();

	if(g_pBehaviorEngine->getEngine() == ENGINE_VORTICON)
	{
		drawVorticonStyle(lRect);
	}
	else
	{
		(this->*drawButton)(lRect);
	}

}
