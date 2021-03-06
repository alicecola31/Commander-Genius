/*
 * CSDLVideo.h
 *
 *  Created on: 05.02.2011
 *      Author: gerstrong
 *
 *  This class
 */

#ifndef CSDLVIDEO_H_
#define CSDLVIDEO_H_

#include "sdl/video/CVideoEngine.h"

class CSDLVideo : public CVideoEngine
{
public:
	CSDLVideo(const CVidConfig& VidConfig);

	bool resizeDisplayScreen(const CRect<Uint16>& newDim);
	bool createSurfaces();
	void collectSurfaces();
	void clearSurfaces();

    void setLightIntensity(const float intensity);

	void scaleNoFilter( 	SDL_Surface *srcSfc,
									const SDL_Rect *scrrect,
									SDL_Surface *dstSfc,
									const SDL_Rect *dstrect );

    SDL_Surface *getOverlaySurface()
    { return mpOverlaySurface.get(); }

    bool initOverlaySurface( const bool useAlpha,
                             const Uint16 width,
                             const Uint16 height );

	void updateScreen();

private:

    std::unique_ptr<SDL_Surface, SDL_Surface_Deleter> mpOverlaySurface; // For some situations like darkrooms we need to use that surface!

};

#endif /* CSDLVIDEO_H_ */
