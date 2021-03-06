/*
 * CGUINumberControl.cpp
 *
 *  Created on: 06.03.2012
 *      Author: gerstrong
 */

#include "CGUINumberControl.h"
#include "graphics/CGfxEngine.h"
#include "sdl/input/CInput.h"
#include "sdl/input/InputEvents.h"
#include "sdl/CVideoDriver.h"
#include "common/CBehaviorEngine.h"
#include "core/mode/CGameMode.h"
#include "sdl/CTimer.h"
#include "StringUtils.h"

int CGUINumberControl::mTwirliconID = 10;

const int SLIDER_WIDTH = 16;



CGUINumberControl::CGUINumberControl(	const std::string& text,
					const int startValue,
					const int endValue,
					const int deltaValue,
					const int value,
					const bool slider ) :
mText(text),
mStartValue(startValue),
mEndValue(endValue),
mDeltaValue(deltaValue),
mValue(value),
mIncSel(false),
mDecSel(false),
mSlider(slider),
drawButton(&CGUINumberControl::drawNoStyle)
{
	mFontID = 1;
    
	if(g_pBehaviorEngine->getEngine() == ENGINE_VORTICON)
	{
	    drawButton = &CGUINumberControl::drawVorticonStyle;
	}
	else if(g_pBehaviorEngine->getEngine() == ENGINE_GALAXY)
	{
	    drawButton = &CGUINumberControl::drawGalaxyStyle;		
	}
	
	setupButtonSurface();
}


void CGUINumberControl::increment()
{
	setSelection(mValue+mDeltaValue);
}

void CGUINumberControl::decrement()
{
	setSelection(mValue-mDeltaValue);
}


bool CGUINumberControl::sendEvent(const InputCommands command)
{
	if(command == IC_STATUS || command == IC_JUMP || command == IC_RIGHT)
	{
		increment();
		return true;
	}
	else if(command == IC_LEFT)
	{
		decrement();
		return true;
	}
	return false;
}



const int CGUINumberControl::getSelection()
{
	return mValue;
}

void CGUINumberControl::setSelection( const int value )
{

	if( mStartValue>value )
		mValue = mStartValue;
	else if( mEndValue<value )
		mValue = mEndValue;
	else
		mValue = value;

	setupButtonSurface();
}


std::string CGUINumberControl::sliderStr()
{
	int ch;
	ch = (mDecSel) ? 8 : 1;
	std::string slider;
	slider = static_cast<char>(ch);

	const int sVal = (SLIDER_WIDTH-3)*(mValue - mStartValue) / (mEndValue - mStartValue);

	for( int l=0 ; l<sVal ; l++)
		slider += '\04';

	slider += '\05';

	for( int l=0 ; l<(SLIDER_WIDTH-3)-sVal ; l++)
		slider += '\06';

	ch = (mIncSel) ? 9 : 7;
	slider += static_cast<char>(ch);

	return slider;
}


void CGUINumberControl::setupButtonSurface()
{
    if(g_pBehaviorEngine->getEngine() != ENGINE_GALAXY)
	return;
    
	CFont &Font = g_pGfxEngine->getFont(mFontID);
	SDL_PixelFormat *format = g_pVideoDriver->getBlitSurface()->format;

	const std::string showText = "  " + mText + ": " + itoa(mValue);
	const std::string showTextL = "  " + mText + ":<" + itoa(mValue);
	const std::string showTextR = "  " + mText + ": " + itoa(mValue) + ">";
	mpTextDarkSfc.reset(Font.fetchColoredTextSfc( showText, SDL_MapRGB( format, 38, 134, 38)));
	mpTextLightSfc.reset(Font.fetchColoredTextSfc( showText, SDL_MapRGB( format, 84, 234, 84)));
	mpTextLightSfcR.reset(Font.fetchColoredTextSfc( showTextR, SDL_MapRGB( format, 84, 234, 84)));
	mpTextLightSfcL.reset(Font.fetchColoredTextSfc( showTextL, SDL_MapRGB( format, 84, 234, 84)));

	mpTextDisabledSfc.reset(Font.fetchColoredTextSfc( showText, SDL_MapRGB( format, 123, 150, 123)));
}




void CGUINumberControl::processLogic()
{
	// Here we check if the mouse-cursor/Touch entry clicked on our Button
	if( MouseMoveEvent *mouseevent = g_pInput->m_EventList.occurredEvent<MouseMoveEvent>() )
	{
		CVec MousePos = mouseevent->Pos;

		if( mRect.HasPoint(MousePos) )
		{
			if(mouseevent->Type == MOUSEEVENT_MOVED)
			{
				mDecSel = false;
				mIncSel = false;

				if( MousePos.x < mRect.x+(mRect.w)/2.0f )
					mDecSel = true;
				else if( MousePos.x > mRect.x+(mRect.w)/2.0f )
					mIncSel = true;

				mHovered = true;
				g_pInput->m_EventList.pop_Event();
			}
			else if(mouseevent->Type == MOUSEEVENT_BUTTONDOWN)
			{
				mButtonDown = true;
				g_pInput->m_EventList.pop_Event();
			}
			else if(mouseevent->Type == MOUSEEVENT_BUTTONUP)
			{
				mButtonUp = true;
				mHovered = true;
				mButtonDown = false;


				if( MousePos.x < mRect.x+(mRect.w)/2.0f )
				{
					// Cycle through the values
					if( mValue > mStartValue )
						decrement();
				}
				else if( MousePos.x > mRect.x+(mRect.w)/2.0f )
				{
					// Cycle through the values
					if( mValue < mEndValue )
						increment();
				}

				setupButtonSurface();
				g_pInput->m_EventList.pop_Event();
			}
		}
		else
		{
			mIncSel = false;
			mDecSel = false;
			mHovered = false;
			mButtonDown = false;
			mButtonUp = false;
		}
	}
}





void CGUINumberControl::drawGalaxyStyle(SDL_Rect& lRect)
{
	SDL_Surface *blitsfc = g_pVideoDriver->getBlitSurface();

	if(!mEnabled)
	{
		SDL_BlitSurface(mpTextDisabledSfc.get(), NULL, blitsfc, &lRect);
	}
	else
	{
		if(mHovered)
		{
		    if(mDecSel)
			SDL_BlitSurface(mpTextLightSfcL.get(), NULL, blitsfc, &lRect);
		    else if(mIncSel)
			SDL_BlitSurface(mpTextLightSfcR.get(), NULL, blitsfc, &lRect);
		    else
			SDL_BlitSurface(mpTextLightSfc.get(), NULL, blitsfc, &lRect);
		}
		else // Button is not hovered
		{
		    SDL_BlitSurface(mpTextDarkSfc.get(), NULL, blitsfc, &lRect);
		}
	}

	drawBlinker(lRect);
}




void CGUINumberControl::drawVorticonStyle(SDL_Rect& lRect)
{

	SDL_Surface *blitsfc = g_pVideoDriver->getBlitSurface();

	// Now lets draw the text of the list control
	CFont &Font = g_pGfxEngine->getFont(mFontID);

	Font.drawFont( blitsfc, mText, lRect.x+24, lRect.y, false );
	Font.drawFont( blitsfc, ":", lRect.x+24+mText.size()*8, lRect.y, false );

	if(mSlider)
	{
		g_pGfxEngine->getFont(2).drawFont( blitsfc, sliderStr(), lRect.x+16+(mText.size()+2)*8, lRect.y, false );
	}
	else
	{
		std::string text = (mDecSel) ? "\025" : " ";
		text += itoa(mValue);
		if(mIncSel)
			text += static_cast<char>(17);
		else
			text += " ";

		Font.drawFont( blitsfc, text, lRect.x+24+(mText.size()+2)*8, lRect.y, false );
	}

	drawTwirl(lRect);

}


void CGUINumberControl::drawNoStyle(SDL_Rect& lRect)
{

	SDL_Surface *blitsfc = g_pVideoDriver->getBlitSurface();

	if( mButtonUp )
	{
		drawRect( blitsfc, &lRect, 1, 0x00BBBBBB, 0x00CFCFCF );
	}
	else if( mButtonDown )
	{
		drawRect( blitsfc, &lRect, 1, 0x00BBBBBB, 0x00DFDFDF );
	}
	else if( mHovered )
	{
		drawRect( blitsfc, &lRect, 1, 0x00BBBBBB, 0x00EFEFEF );
	}
	else
	{
		drawRect( blitsfc, &lRect, 1, 0x00BBBBBB, 0x00FFFFFF );
	}

	// Now lets draw the text of the list control
	CFont &Font = g_pGfxEngine->getFont(mFontID);

	Font.drawFontCentered( blitsfc, mText, lRect.x, lRect.w, lRect.y, lRect.h,false );

}


void CGUINumberControl::processRender(const CRect<float> &RectDispCoordFloat)
{

	// Transform to the display coordinates
	CRect<float> displayRect = mRect;
	displayRect.transform(RectDispCoordFloat);
	SDL_Rect lRect = displayRect.SDLRect();

	(this->*drawButton)(lRect);

}
