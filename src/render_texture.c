/*
	Textures for 3D objects

	Copyright (C) 2008	Patrice Mandin

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

#include <SDL.h>

#include "background_tim.h"
#include "render_texture.h"
#include "video.h"
#include "dither.h"
#include "parameters.h"

/*--- Functions ---*/

/* Load texture from a TIM image file as pointer */
render_texture_t *render_texture_load_from_tim(void *tim_ptr)
{
	tim_header_t *tim_header;
	Uint16 *pal_header;
	int num_colors, num_palettes, i,j, paletted, img_offset;
	int w,h, wpot, tim_type;
	render_texture_t *tex;
	tim_size_t *tim_size;

	/* Read dimensions */
	tim_header = (tim_header_t *) tim_ptr;
	if (SDL_SwapLE32(tim_header->magic) != MAGIC_TIM) {
		fprintf(stderr, "Not a TIM file\n");
		return NULL;
	}

	num_palettes = SDL_SwapLE16(tim_header->nb_palettes);
	if (num_palettes>MAX_TEX_PALETTE) {
		fprintf(stderr, "Does not support %d palettes per texture\n", num_palettes);
		return NULL;
	}

	num_colors = SDL_SwapLE16(tim_header->palette_colors);

	paletted = 0;
	img_offset = SDL_SwapLE32(tim_header->offset) + 20;

	tim_size = (tim_size_t *) (&((Uint8 *) tim_ptr)[img_offset-4]);
	w = SDL_SwapLE16(tim_size->width);
	h = SDL_SwapLE16(tim_size->height);

	tim_type = SDL_SwapLE16(tim_header->type);
	switch(tim_type) {
		case TIM_TYPE_4:
			paletted = 1;
			w <<= 2;
			break;
		case TIM_TYPE_8:
			paletted = 1;
			w <<= 1;
			break;
		case TIM_TYPE_16:
			break;
	}
	if ((w==0) || (h==0)) {
		fprintf(stderr, "Can not read image dimension\n");
		return NULL;
	}

	/* Align on POT size */
	wpot = 2;
	while (wpot<w) {
		wpot <<= 1;
	}	

	/* Allocate memory */
	tex = calloc(1, sizeof(render_texture_t) + wpot*h*(paletted ? 1 : 2));
	if (!tex) {
		fprintf(stderr, "Can not allocate memory for texture\n");
		return NULL;
	}

	tex->paletted = paletted;
	tex->pitch = wpot*(paletted ? 1 : 2);
	tex->w = w;
	tex->h = h;

	/* Copy palettes to video format */
	if (paletted) {
		SDL_PixelFormat *fmt = video.screen->format;

		pal_header = & ((Uint16 *) tim_ptr)[sizeof(tim_header_t)/2];
		for (i=0; i<num_palettes; i++) {
			for (j=0; j<num_colors; j++) {
				int r,g,b;

				Uint16 color = *pal_header++;
				color = SDL_SwapLE16(color);

				r = color & 31;
				r = (r<<3)|(r>>2);
				g = (color>>5) & 31;
				g = (g<<3)|(g>>2);
				b = (color>>10) & 31;
				b = (b<<3)|(b>>2);

				if ((fmt->BytesPerPixel==1) && params.dithering) {
					tex->palettes[j][i] = dither_nearest_index(r,g,b);
				} else {
					tex->palettes[j][i] = SDL_MapRGB(fmt, r,g,b);
				}
			}
		}
	}

	/* Copy data */
	switch(tim_type) {
		case TIM_TYPE_4:
			{
				Uint8 *src_pixels = &((Uint8 *) tim_ptr)[img_offset];
				Uint8 *tex_pixels = &((Uint8 *)tex)[sizeof(render_texture_t)];
				for (i=0; i<h; i++) {
					Uint8 *tex_line = tex_pixels;
					for (j=0; j<w>>1; j++) {
						Uint8 color = *src_pixels++;
						*tex_line++ = color & 15;
						*tex_line++ = (color>>4) & 15;
					}
					tex_pixels += tex->pitch;
				}
			}
			break;
		case TIM_TYPE_8:
			{
				Uint8 *src_pixels = &((Uint8 *) tim_ptr)[img_offset];
				Uint8 *tex_pixels = &((Uint8 *)tex)[sizeof(render_texture_t)];
				for (i=0; i<h; i++) {
					memcpy(tex_pixels, src_pixels, w);
					src_pixels += w;
					tex_pixels += tex->pitch;
				}
			}
			break;
		case TIM_TYPE_16:
			{
				Uint16 *src_pixels = (Uint16 *) (&((Uint8 *) tim_ptr)[img_offset]);
				Uint16 *tex_pixels = (Uint16 *) (&((Uint8 *)tex)[sizeof(render_texture_t)]);
				for (i=0; i<h; i++) {
					for (j=0; j<w; j++) {
						/* TODO: convert to video format */
						/* FIXME: need to allocate more for video bpp=24 or 32 */
					}
				}
			}
			break;
	}

	return tex;
}

void render_texture_shutdown(render_texture_t *texture)
{
	if (texture) {
		free(texture);
	}
}