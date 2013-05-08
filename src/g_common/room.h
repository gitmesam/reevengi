/*
	Room data

	Copyright (C) 2007-2013	Patrice Mandin

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef ROOM_H
#define ROOM_H 1

#include <SDL.h>

/*--- Defines ---*/

/*--- External types ---*/

typedef struct render_texture_s render_texture_t;
typedef struct render_mask_s render_mask_t;

/*--- Types ---*/

typedef struct room_s room_t;

struct room_s {
	void (*dtor)(room_t *this);

	void (*load)(room_t *this, int stage, int room, int camera);
	void (*load_background)(room_t *this, int stage, int room, int camera);
	void (*load_bgmask)(room_t *this, int stage, int room, int camera);

	/* RDT file */
	void *file;
	Uint32 file_length;

	/* Background image */
	render_texture_t *background;

	/* and its masks */
	render_texture_t *bg_mask;
	render_mask_t *rdr_mask;

	int num_cameras;
};

/*--- Variables ---*/

/*--- Functions ---*/

room_t *room_ctor(void);

#endif /* ROOM_H */
