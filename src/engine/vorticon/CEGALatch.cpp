/*
 * CEGALatch.cpp
 *
 *  Created on: 11.07.2009
 *      Author: gerstrong
 */

#include "CEGALatch.h"
#include "fileio/ResourceMgmt.h"
#include "fileio/lz.h"
#include "graphics/CGfxEngine.h"
#include "sdl/CVideoDriver.h"
#include "fileio/TypeDefinitions.h"
#include "common/CBehaviorEngine.h"
#include "CLogFile.h"
#include "engine/CPlanes.h"
#include "FindFile.h"
#include <SDL.h>
#include <stdio.h>
#include <string.h>

CEGALatch::CEGALatch( int planesize,
					 long bitmaptablelocation,
					 short fonttiles,
					 long fontlocation,
					 long screentiles,
					 long screenlocation,
					 short num16tiles,
					 long tiles16location,
					 short bitmaps,
					 long bitmaplocation) :
RawData(NULL)
{
	m_latchplanesize = planesize;
	m_bitmaptablelocation = bitmaptablelocation;
	m_fonttiles = fonttiles;
	m_fontlocation = fontlocation;
	m_screentiles = screentiles;
	m_screenlocation = screenlocation;
	m_num16tiles = num16tiles;
	m_tiles16location = tiles16location;
	m_bitmaps = bitmaps;
	m_bitmaplocation = bitmaplocation;
}

bool CEGALatch::loadHead( char *data, short m_episode )
{
	SDL_Rect bmpRect;
	bmpRect.x = bmpRect.y = 0;

	data += m_bitmaptablelocation;

	g_pGfxEngine->createEmptyBitmaps(m_bitmaps);
	for(int i=0 ; i<m_bitmaps ; i++)
	{
		std::string name;
		//char name[9];
		CBitmap &Bitmap = g_pGfxEngine->getBitmap(i);
		memcpy(&bmpRect.w,data+16*i,2);
		memcpy(&bmpRect.h,data+16*i+2,2);
		name = static_cast<const char*>(data+16*i+8);
		bmpRect.w *= 8; // The width is always divided by eight while read, so we need to extend it back to res coords

		name = name.substr(0,8); // Cut the rest of data down, if junk detected in the exe file
		TrimSpaces(name);
		if( name != "" ) Bitmap.setName( name );
		else
		{
			if (m_episode == 1)
			{
				const std::string default_names[] = { "TITLE", "IDLOGO", "F1HELP", "HIGHSCOR",
					"NAME", "SCORE", "PARTS", "GAMEOVER", "AN", "PRESENT", "APOGEE", "KEENSHIP", "WINDON",
					"WINDOFF", "ONEMOMEN", "OFAN", "PRODUCT", "IDSOFT" }; // in case the names are empty
				Bitmap.setName( default_names[i] );
			}
			else if (m_episode == 2)
			{
				const std::string default_names[] = { "TITLE", "IDLOGO", "F1HELP", "HIGHSCOR",
					"NAME", "SCORE", "SAVED", "GAMEOVER", "AN", "PRESENT", "APOGEE", "KEENSHIP", "WINDON",
					"WINDOFF", "ONEMOMEN", "OFAN", "PRODUCT", "IDSOFT" }; // in case the names are empty
				Bitmap.setName( default_names[i] );
			}
			else if (m_episode == 3)
			{
				const std::string default_names[] = { "TITLE", "IDLOGO", "F1HELP", "HIGHSCOR",
					"NAME", "SCORE", "GAMEOVER", "AN", "PRESENT", "APOGEE", "ONEMOMEN", "OFAN", "PRODUCT", "IDSOFT" }; // in case the names are empty
				Bitmap.setName( default_names[i] );
			}
		}
		Bitmap.createSurface(g_pVideoDriver->getScrollSurface()->flags,
							bmpRect,
							g_pGfxEngine->Palette.m_Palette);
	}
	return true;
}




void CEGALatch::loadTilemap(CTilemap &Tilemap, CPlanes &Planes, const int episode, const std::string &path)
{
	Tilemap.CreateSurface( g_pGfxEngine->Palette.m_Palette, SDL_SWSURFACE, m_num16tiles, 4, 13 );
	SDL_Surface *sfc = Tilemap.getSDLSurface();
	SDL_FillRect(sfc,NULL, 0);
	
	if(SDL_MUSTLOCK(sfc))	SDL_LockSurface(sfc);
	Uint8 *u_pixel = (Uint8*) sfc->pixels;

	for(int p=0;p<4;p++)
		Planes.readPlaneofTiles(p, u_pixel, 13, 16, m_num16tiles);

	if(SDL_MUSTLOCK(sfc))	SDL_UnlockSurface(sfc);

	// Load Hi-Colour, VGA, SVGA Tiles into the tilemap	
	if(Tilemap.loadHiresTile("gfx/ck" + itoa(episode) + "tiles", path))
	{
	  g_pLogFile->textOut(GREEN, "Additional VGA Bitmap for the Tileset has been loaded successfully!");
	}

	// Adapt the tilemap to the display, so they are faster blit
	Tilemap.optimizeSurface();  
}



bool CEGALatch::loadData( std::string &path, short episode, int version, unsigned char *data, bool compresseddata )
{
	std::string filename;
	byte *RawData;
    Uint16 width, height;
    SDL_Surface *sfc;


	filename = getResourceFilename("egalatch.ck" + itoa(episode), path);

	FILE* latchfile = OpenGameFile(filename,"rb");

	if(!latchfile)
		return false;

	RawData = new byte[m_latchplanesize * 4];
    // get the data out of the file into the memory, decompressing it if necessary.
    if (compresseddata)
    {
		if (lz_decompress(latchfile, (unsigned char*) RawData))
		{
			return 1;
		}
    }
    else
    {
    	for(int i=0 ; i<(m_latchplanesize*4) ; i++)
    		RawData[i] = fgetc(latchfile);
    }

    fclose(latchfile);

	// these are the offsets of the different video planes as
	// relative to each other--that is if a pixel in plane1
	// is at N, the byte for that same pixel in plane3 will be
	// at (N + plane3).
	unsigned long plane1, plane2, plane3, plane4;

	plane1 = 0;
	plane2 = (m_latchplanesize * 1);
	plane3 = (m_latchplanesize * 2);
	plane4 = (m_latchplanesize * 3);

	// ** read the 8x8 tiles **
	// set up the getbit() function of CPlanes class
	CPlanes Planes(RawData);
	Planes.setOffsets(plane1 + m_fontlocation, plane2 + m_fontlocation,
					  plane3 + m_fontlocation, plane4 + m_fontlocation, 0);
	// Load these graphics into the CFont Class of CGfxEngine
	// The original vorticon engine only uses one fontmap, but we use another for
	// extra icons. For example sliders are in that map

	g_pGfxEngine->freeFonts();
	g_pGfxEngine->createEmptyFontmaps(3);

	g_pGfxEngine->getFont(0).loadinternalFont();

	CFont &Font = g_pGfxEngine->getFont(1);
	Font.CreateSurface( g_pGfxEngine->Palette.m_Palette, SDL_SWSURFACE );
	sfc = Font.getSDLSurface();

	g_pGfxEngine->getFont(2).loadAlternateFont();


	if(SDL_MUSTLOCK(sfc)) SDL_LockSurface(sfc);

	Uint8 *pixel = (Uint8*) sfc->pixels;
	SDL_FillRect(sfc, NULL, 0);

	for(int p=0;p<4;p++)
		Planes.readPlaneofTiles(p, pixel, 16, 8, m_fonttiles);

	if(SDL_MUSTLOCK(sfc)) SDL_UnlockSurface(sfc);

	// prepare to ** read the 16x16 tiles **
	Planes.setOffsets(plane1 + m_tiles16location,
					 plane2 + m_tiles16location,
					 plane3 + m_tiles16location,
					 plane4 + m_tiles16location,
					 0);

	g_pGfxEngine->freeTilemap();
	g_pGfxEngine->createEmptyTilemaps(2);
	
	loadTilemap(g_pGfxEngine->getTileMap(0), Planes, episode, path);
	
	// prepare to ** read the 16x16 tiles **, this is for the second plane.
	Planes.setOffsets(plane1 + m_tiles16location,
					 plane2 + m_tiles16location,
					 plane3 + m_tiles16location,
					 plane4 + m_tiles16location,
					 0);	
	
	loadTilemap(g_pGfxEngine->getTileMap(1), Planes, episode, path);

	// make masked tiles according to it's surfaces
	applyMasks();

	////////////////////
	/// Load Bitmaps ///
	////////////////////

	Planes.setOffsets(plane1 + m_bitmaplocation,
					plane2 + m_bitmaplocation,
					plane3 + m_bitmaplocation,
					plane4 + m_bitmaplocation,
					0);

	// decode bitmaps into the BitmapData structure. The bitmaps are
	// loaded into one continuous stream of image data, with the bitmaps[]
	// array giving pointers to where each bitmap starts within the stream.

	for(int p=0 ; p<4 ; p++)
	{
		for(int b=0 ; b<m_bitmaps ; b++)
		{
		    CBitmap &bitmap = g_pGfxEngine->getBitmap(b);
			// this points to the location that we're currently
			// decoding bitmap data to

			sfc= bitmap.getSDLSurface();
			if(SDL_MUSTLOCK(sfc)) SDL_LockSurface(sfc);
			Uint8* pixel = (Uint8*) sfc->pixels;
			if(p==0)
				SDL_FillRect(sfc, NULL, 0);
			width = bitmap.getWidth(); height = bitmap.getHeight();
			// Now read the raw data

			Planes.readPlane(p, pixel, width, height);

			if(SDL_MUSTLOCK(sfc)) SDL_UnlockSurface(sfc);
		}
	}

	// optimize the bitmaps and load hq bitmaps if there are some.
	for(int b=0 ; b<m_bitmaps ; b++)
	{
		CBitmap &bitmap = g_pGfxEngine->getBitmap(b);
		bitmap.optimizeSurface();
	}

	std::set<std::string> filelist;
	FileListAdder fileListAdder;
	std::string gfxpath = JoinPaths(path, "gfx");
	GetFileList(filelist, fileListAdder, gfxpath, false, FM_REG);
	FilterFilelist(filelist, "bitmap");

	std::set<std::string>::iterator it = filelist.begin();

	for( ; it != filelist.end() ; it++ )
	{
		std::string filename=*it;
		int num = getRessourceID(filename, "bitmap");
		CBitmap &bitmap = g_pGfxEngine->getBitmap(num);
		filename = getResourceFilename("gfx/" + filename, path, false);
		bitmap.loadHQBitmap(filename);
	}

	if(RawData){ delete[] RawData; RawData = NULL;}

	return true;
}

// Convert the normal tiles to masked tiles
void CEGALatch::applyMasks()
{
	SDL_Surface *frontSfc = g_pGfxEngine->getTileMap(1).getSDLSurface();
	SDL_Surface *backSfc = g_pGfxEngine->getTileMap(0).getSDLSurface();

	if(SDL_MUSTLOCK(frontSfc)) SDL_LockSurface(frontSfc);
	if(SDL_MUSTLOCK(backSfc)) SDL_LockSurface(backSfc);

	const Uint8 bpp = frontSfc->format->BytesPerPixel;

	for( Uint16 t=0 ; t<m_num16tiles ; t++ )
	{
		if( g_pBehaviorEngine->getTileProperties().at(t).behaviour == -2 )  // This is for masked tiles.
		{
			SDL_Rect srcRect, dstRect;
			srcRect.w = srcRect.h = 16;
			dstRect.w = dstRect.h = 16;
			
			srcRect.x = 16*(t%13);
			srcRect.y = 16*(t/13);		  
		  
			SDL_FillRect( frontSfc, &srcRect, SDL_MapRGBA(frontSfc->format, 0, 0, 0, 0) );
		  
			for( Uint16 x=0 ; x<16 ; x++ ) for( Uint16 y=0 ; y<16 ; y++ )
			{	
				Uint32 u_colour = 0;
				Uint8 r,g,b;
				Uint8 *u_offset = (Uint8*)backSfc->pixels + bpp*((y+16*((t+1)/13))*13*16 + 16*((t+1)%13) + x);
				memcpy( &u_colour, u_offset, bpp );
				SDL_GetRGB( u_colour, backSfc->format, &r, &g, &b );

				/*SDL_Rect dstRect;
				srcRect.w = srcRect.h = 1;
				srcRect.x = 16*((t+1)%13) + x;
				srcRect.y = y+16*((t+1)/13);*/

				Uint8 alpha;
					
				if( r>=250 && g>=250 && b>=250 ) // In this case set it to zero
				{
				    alpha = r = g = b = 0;
				}
				else // Get the pixel of the previous tile. If the mask has colour, use alpha channel, black is opaque
				{
				    alpha = 255 - (r+g+b)/3;
				    u_offset = (Uint8*)backSfc->pixels + bpp*((y+16*(t/13))*13*16 + 16*(t%13) + x);
				    memcpy( &u_colour, u_offset, bpp);
				    SDL_GetRGB( u_colour, backSfc->format, &r, &g, &b);					    
				}
				
				SDL_Rect srcRect;
				srcRect.w = srcRect.h = 1;
				//srcRect.x = 16*((t+1)%13) + x;
				//srcRect.y = y+16*((t+1)/13);
				srcRect.x = 16*((t)%13) + x;
				srcRect.y = y+16*((t)/13);
				
				SDL_FillRect( frontSfc, &srcRect, SDL_MapRGBA(frontSfc->format, r, g, b, alpha) );
			}
		}
	}

	if(SDL_MUSTLOCK(backSfc)) 
	  SDL_UnlockSurface(backSfc);
	if(SDL_MUSTLOCK(frontSfc)) 
	  SDL_UnlockSurface(frontSfc);
}

CEGALatch::~CEGALatch() {
	if(RawData) delete [] RawData, RawData = NULL;
}
