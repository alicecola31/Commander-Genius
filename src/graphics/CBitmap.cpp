/*
 * CBitmap.cpp
 *
 *  Created on: 03.09.2009
 *      Author: gerstrong
 */

#include "CBitmap.h"
#include "CPalette.h"
#include "FindFile.h"
#include "sdl/CVideoDriver.h"

CBitmap::CBitmap()
{}


CBitmap::CBitmap(const std::shared_ptr<SDL_Surface> &bmpSfc)
{
	mpBitmapSurface = bmpSfc;
}

CBitmap::CBitmap(const CBitmap &bitmap) :
mName(bitmap.getName())
{
	SDL_Surface *sfc = bitmap.getSDLSurface();
#if SDL_VERSION_ATLEAST(2, 0, 0)
    
#else
    if( sfc != nullptr )
		mpBitmapSurface.reset(SDL_DisplayFormat( sfc ), &SDL_FreeSurface );
#endif
}

///
// Creation Routines
///
bool CBitmap::createSurface(Uint32 flags, SDL_Rect rect, SDL_Color *Palette)
{
	mpBitmapSurface.reset( SDL_CreateRGBSurface(flags, rect.w, rect.h, 8, 0, 0, 0, 0), &SDL_FreeSurface );
#if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_SetSurfaceColorMod(mpBitmapSurface.get(), Palette->r, Palette->g, Palette->b);
    SDL_SetColorKey(mpBitmapSurface.get(), SDL_TRUE, COLORKEY);
#else
    SDL_SetColors(mpBitmapSurface.get(), Palette, 0, 255);
	SDL_SetColorKey(mpBitmapSurface.get(), SDL_SRCCOLORKEY, COLORKEY);
#endif
	
	if(mpBitmapSurface)
	  return true;
	else
	  return false;
}

bool CBitmap::optimizeSurface()
{
	if(mpBitmapSurface)
	{
		SDL_Surface *temp_surface;
#if SDL_VERSION_ATLEAST(2, 0, 0)
        
#else
        temp_surface = SDL_DisplayFormat(mpBitmapSurface.get());
		mpBitmapSurface.reset( temp_surface, &SDL_FreeSurface );
#endif
		return true;
	}
	else
		return false;
}

bool CBitmap::loadHQBitmap( const std::string& filename )
{
	if(!IsFileAvailable(filename))
		return false;

	if( mpBitmapSurface )
	{
		SDL_Surface *tempSurface = SDL_LoadBMP(GetFullFileName(filename).c_str());
		if(tempSurface)
		{
			SDL_Surface *displaysurface = SDL_ConvertSurface(tempSurface, mpBitmapSurface->format, mpBitmapSurface->flags);
			SDL_BlitSurface(displaysurface, NULL, mpBitmapSurface.get(), NULL);
			SDL_FreeSurface(displaysurface);
			SDL_FreeSurface(tempSurface);
			return true;
		}
	}
	return false;
}


/**
 * \brief The function that blits the sprite to dst
 * \param x		 		X-Coordinate, indicating the position on dst
 * \param y				Y-Coordinate, indicating the position on dst
 */
void CBitmap::draw(Uint16 x, Uint16 y)
{
    _draw( g_pVideoDriver->getBlitSurface(), x, y);
}

///
// Drawing Routines
///
void CBitmap::_draw(SDL_Surface *dst, Uint16 x, Uint16 y) const
{
	SDL_Rect dst_rect;
	dst_rect.x = x;	dst_rect.y = y;
	
	SDL_BlitSurface(mpBitmapSurface.get(), NULL, dst, &dst_rect);
}
