#include "lcd.h"
#include "main.h"

#include <SDL/SDL.h>

#ifdef RGB565_BUF
uint8_t framebuffer[320*240];
#endif


unsigned int ticcount = 0;
SDL_Surface* screen;
SDL_Surface* tmp;

// I_GetTime
// returns time in 1/35 second tics
//
int  HAL_GetTick (void)
{
	return SDL_GetTicks();
}

void DEBUGTXT(char* toprint)
{
	printf("%s\n", toprint);
}

void lcd_init()
{
	SDL_Init( SDL_INIT_VIDEO );
	SDL_ShowCursor(0);
	screen = SDL_SetVideoMode(256, 240, 8, SDL_HWSURFACE);
	tmp = SDL_LoadBMP("pal.bmp");
	SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, tmp->format->palette->colors, 0, 256);
	SDL_FreeSurface(tmp);
}


void lcd_exit()
{
	SDL_FreeSurface(screen);
	SDL_Quit();
}

void lcd_flip()
{
	SDL_Flip(screen);
}
