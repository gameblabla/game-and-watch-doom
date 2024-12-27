// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#ifndef PCFX
#include <SDL/SDL.h>
#endif

#include "config.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_event.h"
#include "d_main.h"
#include "i_video.h"
#include "z_zone.h"

#include "tables.h"
#include "doomkeys.h"

#include <stdint.h>
#include <stdbool.h>
#include "lcd.h"
#include "gfx.h"
#include "buttons.h"
// #include "images.h"
// #include "touch.h"
// #include "button.h"

// The screen buffer; this is modified to draw things to the screen

byte *I_VideoBuffer;

// If true, game is running as a screensaver

boolean screensaver_mode = false;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible;

// Mouse acceleration
//
// This emulates some of the behavior of DOS mouse drivers by increasing
// the speed when the mouse is moved fast.
//
// The mouse input values are input directly to the game, but when
// the values exceed the value of mouse_threshold, they are multiplied
// by mouse_acceleration to increase the speed.

float mouse_acceleration = 2.0;
int mouse_threshold = 10;

int usemouse = 0;

// If true, keyboard mapping is ignored, like in Vanilla Doom.
// The sensible thing to do is to disable this if you have a non-US
// keyboard.

int vanilla_keyboard_mapping = true;


typedef struct
{
	byte r;
	byte g;
	byte b;
} col_t;

col_t palette[256];

// Palette converted to RGB565
#ifdef RGB565_BUF
static uint16_t rgb565_palette[256];
#endif

// Last touch state

// static touch_state_t last_touch_state;

// Last button state

static bool last_button_state;
static short previous_buttons;

// run state

static bool run;

void I_InitGraphics (void)
{
	// gfx_image_t keys_img;
	// gfx_coord_t coords;

	// gfx_init_img (&keys_img, 40, 320, GFX_PIXEL_FORMAT_RGB565, RGB565_BLACK);
	// keys_img.pixel_data = (uint8_t*)img_keys;
	// gfx_init_img_coord (&coords, &keys_img);

	// gfx_draw_img (&keys_img, &coords);
	// lcd_refresh ();

	// gfx_draw_img (&keys_img, &coords);
	// lcd_refresh ();

	// I_VideoBuffer = (byte*)Z_Mall32oc (SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);
	I_VideoBuffer = SCREENBUFFER;
	screenvisible = true;
}

void I_ShutdownGraphics (void)
{
	// Z_Free (I_VideoBuffer);
}

void I_StartFrame (void)
{

}







static short pressed_buttons;


enum ButtonState {
	BTN_None,
	BTN_Up,
	BTN_Down
};
enum ButtonState check_button(event_t *e, short old_state, short new_state, short doom_button, short our_button) {
	if((our_button & old_state) && !(our_button & new_state)) {
		event_t event;
		event.type = ev_keyup;
		event.data1 = doom_button;
		event.data2 = -1;
		event.data3 = -1;
		D_PostEvent (&event);
		return true;
	}
	if(!(our_button & old_state) && (our_button & new_state)) {
		event_t event;
		event.type = ev_keydown;
		event.data1 = doom_button;
		event.data2 = -1;
		event.data3 = -1;
		D_PostEvent (&event);
		e->data1 = doom_button;
		return false;
	}
}

void I_GetEvent (void)
{
	event_t event;
	bool button_state;

	// button_state = button_read ();

	if (last_button_state != button_state)
	{
		last_button_state = button_state;

		event.type = last_button_state ? ev_keydown : ev_keyup;
		event.data1 = KEY_FIRE;
		event.data2 = -1;
		event.data3 = -1;

		D_PostEvent (&event);
	}

	/* gameblabla */
	short buttons = buttons_get();
	if(buttons != previous_buttons) {
		// if(buttons < previous_buttons) {
		// 	// key released
		// 	event.type = ev_keyup;
		// } else {
		// 	// key pressed
		// 	event.type = ev_keydown;
		// }

		check_button(&event, previous_buttons, buttons, KEY_LEFTARROW, B_Left);
		
		check_button(&event, previous_buttons, buttons, KEY_RIGHTARROW, B_Right);
		check_button(&event, previous_buttons, buttons, KEY_UPARROW, B_Up);
		check_button(&event, previous_buttons, buttons, KEY_DOWNARROW, B_Down);
		check_button(&event, previous_buttons, buttons, KEY_ENTER, B_B);
		check_button(&event, previous_buttons, buttons, KEY_FIRE, B_A);
		
		check_button(&event, previous_buttons, buttons, KEY_USE, B_C);
		
		
		check_button(&event, previous_buttons, buttons, KEY_RALT, B_D);
		check_button(&event, previous_buttons, buttons, KEY_ESCAPE, B_START);

		// if(buttons) {
		// 	event.type = ev_keydown;
		// } else {
		// 	event.type = ev_keyup;
		// }

		// if(buttons & B_Left) {
		// 	event.data1 = KEY_ENTER;
		// }
		
		// event.data2 = -1;
		// event.data3 = -1;
		previous_buttons = buttons;
		// D_PostEvent (&event);
	}


	


	// touch_main ();

	// if ((touch_state.x != last_touch_state.x) || (touch_state.y != last_touch_state.y) || (touch_state.status != last_touch_state.status))
	// {
	// 	last_touch_state = touch_state;

	// 	event.type = (touch_state.status == TOUCH_PRESSED) ? ev_keydown : ev_keyup;
	// 	event.data1 = -1;
	// 	event.data2 = -1;
	// 	event.data3 = -1;

	// 	if ((touch_state.x > 49)
	// 	 && (touch_state.x < 72)
	// 	 && (touch_state.y > 104)
	// 	 && (touch_state.y < 143))
	// 	{
	// 		// select weapon
	// 		if (touch_state.x < 60)
	// 		{
	// 			// lower row (5-7)
	// 			if (touch_state.y < 119)
	// 			{
	// 				event.data1 = '5';
	// 			}
	// 			else if (touch_state.y < 131)
	// 			{
	// 				event.data1 = '6';
	// 			}
	// 			else
	// 			{
	// 				event.data1 = '1';
	// 			}
	// 		}
	// 		else
	// 		{
	// 			// upper row (2-4)
	// 			if (touch_state.y < 119)
	// 			{
	// 				event.data1 = '2';
	// 			}
	// 			else if (touch_state.y < 131)
	// 			{
	// 				event.data1 = '3';
	// 			}
	// 			else
	// 			{
	// 				event.data1 = '4';
	// 			}
	// 		}
	// 	}
	// 	else if (touch_state.x < 40)
	// 	{
	// 		// button bar at bottom screen
	// 		if (touch_state.y < 40)
	// 		{
	// 			// enter
	// 			event.data1 = KEY_ENTER;
	// 		}
	// 		else if (touch_state.y < 80)
	// 		{
	// 			// escape
	// 			event.data1 = KEY_ESCAPE;
	// 		}
	// 		else if (touch_state.y < 120)
	// 		{
	// 			// use
	// 			event.data1 = KEY_USE;
	// 		}
	// 		else if (touch_state.y < 160)
	// 		{
	// 			// map
	// 			event.data1 = KEY_TAB;
	// 		}
	// 		else if (touch_state.y < 200)
	// 		{
	// 			// pause
	// 			event.data1 = KEY_PAUSE;
	// 		}
	// 		else if (touch_state.y < 240)
	// 		{
	// 			// toggle run
	// 			if (touch_state.status == TOUCH_PRESSED)
	// 			{
	// 				run = !run;

	// 				event.data1 = KEY_RSHIFT;

	// 				if (run)
	// 				{
	// 					event.type = ev_keydown;
	// 				}
	// 				else
	// 				{
	// 					event.type = ev_keyup;
	// 				}
	// 			}
	// 			else
	// 			{
	// 				return;
	// 			}
	// 		}
	// 		else if (touch_state.y < 280)
	// 		{
	// 			// save
	// 			event.data1 = KEY_F2;
	// 		}
	// 		else if (touch_state.y < 320)
	// 		{
	// 			// load
	// 			event.data1 = KEY_F3;
	// 		}
	// 	}
	// 	else
	// 	{
	// 		// movement/menu navigation
	// 		if (touch_state.x < 100)
	// 		{
	// 			if (touch_state.y < 100)
	// 			{
	// 				event.data1 = KEY_STRAFE_L;
	// 			}
	// 			else if (touch_state.y < 220)
	// 			{
	// 				event.data1 = KEY_DOWNARROW;
	// 			}
	// 			else
	// 			{
	// 				event.data1 = KEY_STRAFE_R;
	// 			}
	// 		}
	// 		else if (touch_state.x < 180)
	// 		{
	// 			if (touch_state.y < 160)
	// 			{
	// 				event.data1 = KEY_LEFTARROW;
	// 			}
	// 			else
	// 			{
	// 				event.data1 = KEY_RIGHTARROW;
	// 			}
	// 		}
	// 		else
	// 		{
	// 			event.data1 = KEY_UPARROW;
	// 		}
	// 	}

	// 	D_PostEvent (&event);
	// }
}

void I_StartTic (void)
{
	I_GetEvent();
}

void I_UpdateNoBlit (void)
{
}

void I_FinishUpdate (void)
{
	#ifndef PCFX
	extern SDL_Surface* screen;
	#endif
	int x, y;
	byte index;

	// lcd_vsync = false;

	#ifdef RGB565_BUF
	for (y = 0; y < SCREENHEIGHT; y++)
	{
		for (x = 0; x < SCREENWIDTH; x++)
		{
			index = framebuffer[y * SCREENWIDTH + x];
			I_VideoBuffer[y * GFX_MAX_WIDTH + x] = rgb565_palette[index];
			// framebuffer[x * GFX_MAX_WIDTH + (GFX_MAX_WIDTH - y - 1)] = 
		}
	}
	#endif
	lcd_flip();

	// lcd_refresh ();

	// lcd_vsync = true;
}

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT);
}

//
// I_SetPalette
//
void I_SetPalette (byte* doompalette)
{
	/*#ifdef RGB565_BUF
	int i;
	col_t* c;

	for (i = 0; i < 256; i++)
	{
		c = (col_t*)palette;

		rgb565_palette[i] = (gammatable[c->r],
									   gammatable[c->g],
									   gammatable[c->b]);

		doompalette += 3;
	}
	#else
    uint16_t i;

    for (i=0; i<256; ++i)
    {
        // Zero out the bottom two bits of each channel - the PC VGA
        // controller only supports 6 bits of accuracy.

        palette[i].r = gammatable[*doompalette++] & ~3;
        palette[i].g = gammatable[*doompalette++] & ~3;
        palette[i].b = gammatable[*doompalette++] & ~3;
    }
	#endif*/
}

// Given an RGB value, find the closest matching palette index.

int I_GetPaletteIndex (int r, int g, int b)
{
	#ifdef RGB565_BUF
    int best, best_diff, diff;
    int i;
    col_t color;

    best = 0;
    best_diff = INT_MAX;

    for (i = 0; i < 256; ++i)
    {
    	color.r = GFX_RGB565_R(rgb565_palette[i]);
    	color.g = GFX_RGB565_G(rgb565_palette[i]);
    	color.b = GFX_RGB565_B(rgb565_palette[i]);

        diff = (r - color.r) * (r - color.r)
             + (g - color.g) * (g - color.g)
             + (b - color.b) * (b - color.b);

        if (diff < best_diff)
        {
            best = i;
            best_diff = diff;
        }

        if (diff == 0)
        {
            break;
        }
    }

    return best;
    #else
    int best, best_diff, diff;
    int i;

    best = 0; best_diff = INT_MAX;

    for (i = 0; i < 256; ++i)
    {
        diff = (r - palette[i].r) * (r - palette[i].r)
             + (g - palette[i].g) * (g - palette[i].g)
             + (b - palette[i].b) * (b - palette[i].b);

        if (diff < best_diff)
        {
            best = i;
            best_diff = diff;
        }

        if (diff == 0)
        {
            break;
        }
    }

    return best;
    #endif
}

void I_BeginRead (void)
{
}

void I_EndRead (void)
{
}

void I_SetWindowTitle (char *title)
{
}

void I_GraphicsCheckCommandLine (void)
{
}

void I_SetGrabMouseCallback (grabmouse_callback_t func)
{
}

void I_EnableLoadingDisk (void)
{
}

void I_BindVideoVariables (void)
{
}

void I_DisplayFPSDots (boolean dots_on)
{
}

void I_CheckIsScreensaver (void)
{
}
