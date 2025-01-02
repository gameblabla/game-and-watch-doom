//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
// DESCRIPTION:
//	Endianess handling, swapping 16bit and 32bit.
//


#ifndef __I_SWAP__
#define __I_SWAP__

// Endianess handling.
// WAD files are stored little endian.

// Just use SDL's endianness swapping functions.

// These are deliberately cast to signed values; this is the behaviour
// of the macros in the original source and some code relies on it.

#ifdef SYS_BIG_ENDIAN

static inline uint16_t MySwapLE16(uint16_t value) {
    return ((value >> 8) | (value << 8));
}

static inline uint32_t MySwapLE32(uint32_t value) {
    return ((value >> 24) |
                            ((value & 0x00FF0000) >> 8) |
                            ((value & 0x0000FF00) << 8) |
                            (value << 24));
}

#define SHORT(x)  ((signed short) SDL_SwapLE16(x))
#define LONG(x)   ((signed int) SDL_SwapLE32(x))


#else

#warning "This is little endian"

#define SHORT(x)  ((signed short) (x))
#define LONG(x)   ((signed int) (x))

#define SYS_LITTLE_ENDIAN

#endif


#endif
