#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_pixels.h>
#include "CNFGRAPI.h"
#include "SYSDEPNS.h"
#include "UTIL/ENDIANAC.h"
#include "UI/MYOSGLUE.h"
#include "UI/COMOSGLU.h"
#include "STRCONST.h"
#include "OSGLUSD2.h"

/* --- video out --- */

#if MayFullScreen
int hOffset;
int vOffset;
#endif
bool UseFullScreen = (WantInitFullScreen != 0);
bool UseMagnify = (WantInitMagnify != 0);

bool gBackgroundFlag = false;
bool gTrueBackgroundFlag = false;
bool CurSpeedStopped = false;
SDL_Window *main_wind = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
SDL_PixelFormat *format = NULL;

uint8_t * ScalingBuff = nullpr;
uint8_t * CLUT_final;

// Set the display palette from the Macintosh's memory or w/e
static int SetPalette(SDL_Palette *palette, const SDL_Color *macColors, int ncolors)
{
	return SDL_SetPaletteColors(palette, macColors, 0, ncolors);
}

// Get pixel format for a given screen depth
// Note: this is complete and total guesswork right now. Lol.
uint32_t GetPixFormatFromDepth(int depth)
{
	switch(depth)
	{
	case 1:	
		return SDL_PIXELFORMAT_INDEX1LSB;
	case 4:
		return SDL_PIXELFORMAT_INDEX4LSB;
	case 8:
		return SDL_PIXELFORMAT_INDEX8;
	case 16:
		return SDL_PIXELFORMAT_RGB332;
	case 24:
		return SDL_PIXELFORMAT_RGB24;
	case 32:
		return SDL_PIXELFORMAT_RGBA8888;
	default:
		return SDL_PIXELFORMAT_UNKNOWN;
	}
}

// We aren't being smart about *what* to update, since if we use SDL2 properly
// the update operating is stunningly cheap.
GLOBALOSGLUPROC Screen_OutputFrame(uint8_t * src_ptr)
{
	if (EmVideoDisable) { return; }
	
	uint32_t src_format = GetPixFormatFromDepth(vMacScreenDepth+1);
	void *pixels;
	int pitch;
	
	// Setup source surface
	SDL_Surface *src = SDL_CreateRGBSurfaceWithFormatFrom(
		src_ptr, 
		vMacScreenWidth,
		vMacScreenHeight,
		vMacScreenDepth+1,
		vMacScreenByteWidth,
		src_format
	);
	
	// Setup dst surface
	SDL_LockTexture(texture, NULL, &pixels, &pitch);
	SDL_Surface *dst = SDL_CreateRGBSurfaceWithFormatFrom(
		pixels, 
		vMacScreenWidth,
		vMacScreenHeight,
		32, vMacScreenWidth * 4,
		SDL_PIXELFORMAT_RGBX8888
	);
	
	// Blit src to dst
	SDL_BlitSurface(src, NULL, dst, NULL);
	
	// Free surfaces
	SDL_FreeSurface(src);
	SDL_FreeSurface(dst);
	
	// Render the texture
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_UnlockTexture(texture);
	SDL_RenderPresent(renderer);
}

GLOBALOSGLUPROC DoneWithDrawingForTick(void)
{
	return;
}
