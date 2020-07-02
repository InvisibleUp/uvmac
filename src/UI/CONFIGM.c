/*
 * CONFIG Mode
 * 
 * A replacement for Control Mode, with prettier grapgics and actual settings
 * 
 */

#include <SDL2.h> // everything else is deprecated now
#include <stdlib.h>
#include <stdint.h>
#include "CONFIGM.h"
#include "UI/SDL2/OSGLUSD2.h"

/* -- Public Functions -- */

void ConfigMode_Tick()
{
	// Get the screen context and just draw something there for now
	
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderDrawRect(renderer, {.x = 16, .y = 16, .w = 128, .h = 128});
}
