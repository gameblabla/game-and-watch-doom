#include "lcd.h"
#include "main.h"
#include "fastking.h"
#include <eris/v810.h>
#include <eris/king.h>
#include <eris/tetsu.h>
#include <eris/romfont.h>
#include <eris/cd.h>
#include <eris/low/pad.h>
#include <eris/low/scsi.h>


#include "pcfx.h"

uint8_t framebuffer[256*240];
unsigned int ticcount = 0;

extern int nframe;

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

unsigned short pornpal[256] = {
	0x0088, 0x1888, 0x1088, 0x4b88, 0xff88, 0x1b88, 0x1388, 0x0b88, 
	0x0788, 0x3178, 0x2578, 0x1978, 0x1178, 0x3f79, 0x3779, 0x2f79, 
	0xcc7a, 0xc17a, 0xba7a, 0xb07b, 0xa97b, 0xa17b, 0x977b, 0x8f7b, 
	0x877b, 0x807b, 0x787b, 0x747b, 0x6c7b, 0x667b, 0x5e7b, 0x5a7b, 
	0x527b, 0x4e7b, 0x487b, 0x427b, 0x3c7b, 0x387b, 0x337b, 0x2f7b, 
	0x2a7b, 0x267b, 0x217b, 0x207b, 0x1d7a, 0x177a, 0x157a, 0x147a, 
	0xef79, 0xe979, 0xe379, 0xdd79, 0xda7a, 0xd46a, 0xce6a, 0xca6a, 
	0xc46b, 0xbc6b, 0xb46b, 0xac6b, 0xa46b, 0x9c6b, 0x946b, 0x906b, 
	0x896a, 0x816a, 0x7b6a, 0x766a, 0x6f6a, 0x686a, 0x616a, 0x5c6a, 
	0x5669, 0x4e79, 0x4779, 0x4179, 0x3979, 0x3179, 0x2a78, 0x2378, 
	0xef88, 0xe788, 0xdf88, 0xdb88, 0xd388, 0xcb88, 0xc788, 0xbf88, 
	0xb788, 0xb388, 0xab88, 0xa788, 0x9f88, 0x9788, 0x9388, 0x8b88, 
	0x8388, 0x7f88, 0x7788, 0x6e88, 0x6b88, 0x6388, 0x5b88, 0x5788, 
	0x4f88, 0x4788, 0x4388, 0x3b88, 0x3788, 0x2f88, 0x2788, 0x2388, 
	0xc554, 0xb955, 0xac55, 0x9f55, 0x9465, 0x8766, 0x7a66, 0x7066, 
	0x6566, 0x5866, 0x4c77, 0x3f77, 0x3377, 0x2677, 0x1b78, 0x1188, 
	0xab79, 0xa379, 0x9b79, 0x9379, 0x8b79, 0x8579, 0x7f79, 0x7779, 
	0x6f79, 0x6779, 0x6379, 0x5b79, 0x5679, 0x4e79, 0x4779, 0x4379, 
	0x8779, 0x7a79, 0x6e79, 0x6279, 0x5579, 0x4979, 0x3e79, 0x3579, 
	0x7a78, 0x6e78, 0x6678, 0x5d78, 0x5278, 0x4978, 0x4178, 0x3978, 
	0xef49, 0xd049, 0xb549, 0x9a4a, 0x805a, 0x655a, 0x505a, 0x3b6a, 
	0xff88, 0xe589, 0xcf7a, 0xb87b, 0xa27c, 0x8e6d, 0x786e, 0x616f, 
	0x4c60, 0x475f, 0x436f, 0x406f, 0x3c6e, 0x396e, 0x356e, 0x316d, 
	0x2e6d, 0x297c, 0x257c, 0x227c, 0x1e7b, 0x1b7b, 0x177a, 0x147a, 
	0xe998, 0xcda8, 0xb4b8, 0x9bb7, 0x82c7, 0x66d7, 0x4de7, 0x34f7, 
	0x1e07, 0x19f7, 0x17e7, 0x14e7, 0x11d7, 0x0ec7, 0x0cb7, 0x09b8, 
	0xff88, 0xef79, 0xdf79, 0xd26a, 0xc35b, 0xb65b, 0xa64c, 0x994d, 
	0x8e4c, 0x894c, 0x804c, 0x794c, 0x704c, 0x684c, 0x605c, 0x5b5c, 
	0xff88, 0xfa78, 0xf668, 0xf259, 0xee39, 0xea29, 0xe519, 0xe109, 
	0x565c, 0x4f5c, 0x475b, 0x3c6b, 0x3e79, 0x3279, 0x2779, 0x1f79, 
	0x09b8, 0x08a8, 0x06a8, 0x0598, 0x0398, 0x0298, 0x0188, 0x0088, 
	0xb14b, 0xdc3a, 0xb1bb, 0x69df, 0x55cd, 0x41bc, 0x2dab, 0x7c7a
};

// I_GetTime
// returns time in 1/35 second tics
//
int  HAL_GetTick (void)
{
	return ticcount;
}

static char* myitoa(int value) {
    static char buffer[12];  // Enough for an integer (-2147483648 has 11 characters + 1 for '\0')
    char* ptr = buffer + sizeof(buffer) - 1;
    int is_negative = 0;

    // Null-terminate the buffer
    *ptr = '\0';

    // Handle negative numbers
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    // Process each digit
    do {
        *--ptr = (value % 10) + '0';
        value /= 10;
    } while (value);

    // Add the negative sign if necessary
    if (is_negative) {
        *--ptr = '-';
    }

    return ptr;
}

void DEBUGTXT(char* toprint)
{
	print_at(0, 1, 12, "                  ");	
	print_at(0, 1, 12, toprint);	
}

void lcd_init()
{
	/*int i;
	u16 microprog[16];
	eris_king_init();
	eris_tetsu_init();
	
	eris_tetsu_set_priorities(0, 0, 1, 3, 0, 0, 0);
	eris_tetsu_set_king_palette(0, 0, 0, 0);
	eris_tetsu_set_rainbow_palette(0);

	eris_king_set_bg_prio(KING_BGPRIO_0, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, 0);
	eris_king_set_bg_mode(KING_BGMODE_256_PAL, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE);
	//eris_king_set_bg_mode(KING_BGMODE_64K, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE, KING_BGPRIO_HIDE);
	eris_king_set_kram_pages(0, 0, 0, 0);

	for(i = 0; i < 16; i++) {
		microprog[i] = KING_CODE_NOP;
	}

	microprog[0] = KING_CODE_BG0_CG_0;
	microprog[1] = KING_CODE_BG0_CG_1;
	microprog[2] = KING_CODE_BG0_CG_2;
	microprog[3] = KING_CODE_BG0_CG_3;	


	eris_king_disable_microprogram();
	eris_king_write_microprogram(microprog, 0, 16);
	eris_king_enable_microprogram();

	eris_tetsu_set_rainbow_palette(0);
	for(i = 0; i < ARRAY_SIZE(pornpal); i++) {
		eris_tetsu_set_palette(i, pornpal[i]);
	}

	eris_tetsu_set_video_mode(TETSU_LINES_262, 0, TETSU_DOTCLOCK_5MHz, TETSU_COLORS_16, TETSU_COLORS_16, 0, 0, 1, 0, 0, 0, 0);
	eris_king_set_bat_cg_addr(KING_BG0, 0, 0);
	eris_king_set_bat_cg_addr(KING_BG0SUB, 0, 0);
	eris_king_set_scroll(KING_BG0, 0, 0);
	eris_king_set_bg_size(KING_BG0, KING_BGSIZE_256, KING_BGSIZE_256, KING_BGSIZE_256, KING_BGSIZE_256);*/
	
	eris_king_init();
	eris_tetsu_init();
	eris_pad_init(0);
	
	eris_low_cdda_set_volume(63,63);
	
	Set_Video(KING_BGMODE_256_PAL);
	initTimer(0, 1423);
	Upload_Palette(pornpal);

	
	print_at(0, 1, 12, "EXAMPLE");
}


void lcd_exit()
{
	/*SDL_FreeSurface(screen);
	SDL_Quit();*/
}

void lcd_flip()
{
	int i;
	
	ticcount++;
	
	print_at(0, 2, 12, myitoa(ticcount));
	
	eris_king_set_kram_write(0, 1);
	king_kram_write_buffer(framebuffer, 256*240);
	vsync(0);
	++nframe;
	
	// Clear BG0's RAM
	// I_VideoBuffer
	/*for(i = 0x0; i < 256 * 240; i++) {
		eris_king_kram_write(framebuffer[i]);
	}
	eris_king_set_kram_write(0, 1);*/
	//SDL_Flip(screen);
}
