#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>
#ifndef PCFX
#include <SDL/SDL.h>
extern SDL_Surface* screen;
#endif



#ifdef RGB565_BUF
extern uint8_t framebuffer[256*240];
#endif

#ifdef PCFX
extern uint8_t framebuffer[256*240];
#define SCREENBUFFER framebuffer
#define GFX_MAX_WIDTH 256
#else
#define GFX_MAX_WIDTH 256
#define SCREENBUFFER screen->pixels
#endif

void lcd_init();
void lcd_exit();
void lcd_flip();

#endif
