/*
	RE1 RID
	Camera positions

	Copyright (C) 2009-2013	Patrice Mandin

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

#ifndef RDT2_PRI_H
#define RDT2_PRI_H 1

/*--- Defines ---*/

/*--- External types ---*/

struct room_s;

/*--- Types ---*/

typedef struct {
	Uint16 num_offset;
	Uint16 num_masks;
} rdt2_pri_header_t;

typedef struct {
	Uint16 count;
	Uint16 unknown;
	Uint16 dst_x, dst_y;
} rdt2_pri_offset_t;

typedef struct {
	Uint8 src_x, src_y;
	Uint8 dst_x, dst_y;
	Uint16 depth, size;
} rdt2_pri_square_t;

typedef struct {
	Uint8 src_x, src_y;
	Uint8 dst_x, dst_y;
	Uint16 depth, zero;
	Uint16 width, height;
} rdt2_pri_rect_t;

/*--- Functions ---*/

void rdt2_pri_initMasks(struct room_s *this, int num_camera);
void rdt2_pri_drawMasks(struct room_s *this, int num_camera);

#endif /* RDT2_PRI_H */
