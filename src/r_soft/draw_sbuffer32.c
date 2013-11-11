/*
	2D drawing functions
	SBuffer renderer, 32 bits mode

	Copyright (C) 2008-2013	Patrice Mandin

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL.h>
#include <assert.h>

#include "../video.h"
#include "../parameters.h"

#include "../r_common/render.h"
#include "../r_common/r_misc.h"

#include "dither.h"
#include "draw.h"
#include "draw_sbuffer.h"

/*--- Defines ---*/

/*#define FORCE_OPAQUE 1*/

/*--- Functions ---*/

void draw_render_fill32(SDL_Surface *surf, Uint8 *dst_line, sbuffer_segment_t *segment, int x1,int x2)
{
	Uint32 color;
	float r,g,b;
	Uint32 *dst_col = (Uint32 *) dst_line;
	int i;

	r = segment->start.r;
	g = segment->start.g;
	b = segment->start.b;
	if (draw.correctPerspective>0) {
		r /= segment->start.w;
		g /= segment->start.w;
		b /= segment->start.w;
	}

	color = SDL_MapRGB(surf->format, r,g,b);

	for (i=x1; i<=x2; i++) {
		*dst_col++ = color;
	}
}

void draw_render_gouraud32_pc0(SDL_Surface *surf, Uint8 *dst_line, sbuffer_segment_t *segment, int x1,int x2)
{
	float r1,g1,b1, r2,g2,b2, r,g,b, dr,dg,db;
	int dxtotal, i;
	Uint32 *dst_col = (Uint32 *) dst_line;

	dxtotal = segment->end.x - segment->start.x + 1;

	r1 = segment->start.r;
	g1 = segment->start.g;
	b1 = segment->start.b;
	r2 = segment->end.r;
	g2 = segment->end.g;
	b2 = segment->end.b;

	dr = (r2-r1)/dxtotal;
	dg = (g2-g1)/dxtotal;
	db = (b2-b1)/dxtotal;

	r = r1 + dr * (x1-segment->start.x);
	g = g1 + dg * (x1-segment->start.x);
	b = b1 + db * (x1-segment->start.x);

	for (i=x1; i<=x2; i++) {
		*dst_col++ = SDL_MapRGB(surf->format, r,g,b);
		r += dr;
		g += dg;
		b += db;
	}
}

void draw_render_gouraud32_pc1(SDL_Surface *surf, Uint8 *dst_line, sbuffer_segment_t *segment, int x1,int x2)
{
	float r1,g1,b1, r2,g2,b2, r,g,b, dr,dg,db, invw;
	int dxtotal, i;
	Uint32 *dst_col = (Uint32 *) dst_line;

	dxtotal = segment->end.x - segment->start.x + 1;

	invw = 1.0f / segment->start.w;
	r1 = segment->start.r * invw;
	g1 = segment->start.g * invw;
	b1 = segment->start.b * invw;
	invw = 1.0f / segment->end.w;
	r2 = segment->end.r * invw;
	g2 = segment->end.g * invw;
	b2 = segment->end.b * invw;

	dr = (r2-r1)/dxtotal;
	dg = (g2-g1)/dxtotal;
	db = (b2-b1)/dxtotal;

	r = r1 + dr * (x1-segment->start.x);
	g = g1 + dg * (x1-segment->start.x);
	b = b1 + db * (x1-segment->start.x);

	for (i=x1; i<=x2; i++) {
		*dst_col++ = SDL_MapRGB(surf->format, r,g,b);
		r += dr;
		g += dg;
		b += db;
	}
}

void draw_render_gouraud32_pc3(SDL_Surface *surf, Uint8 *dst_line, sbuffer_segment_t *segment, int x1,int x2)
{
	float r1,g1,b1, r2,g2,b2, r,g,b, dr,dg,db;
	float w1, w2, w, dw, invw;
	int dxtotal, i;
	Uint32 *dst_col = (Uint32 *) dst_line;

	dxtotal = segment->end.x - segment->start.x + 1;

	r1 = segment->start.r;
	g1 = segment->start.g;
	b1 = segment->start.b;
	w1 = segment->start.w;
	r2 = segment->end.r;
	g2 = segment->end.g;
	b2 = segment->end.b;
	w2 = segment->end.w;

	dr = (r2-r1)/dxtotal;
	dg = (g2-g1)/dxtotal;
	db = (b2-b1)/dxtotal;
	dw = (w2-w1)/dxtotal;

	r = r1 + dr * (x1-segment->start.x);
	g = g1 + dg * (x1-segment->start.x);
	b = b1 + db * (x1-segment->start.x);
	w = w1 + dw * (x1-segment->start.x);

	for (i=x1; i<=x2; i++) {
		int rr,gg,bb;

		invw = 1.0f / w;
		rr = r * invw;
		gg = g * invw;
		bb = b * invw;
		*dst_col++ = SDL_MapRGB(surf->format, rr,gg,bb);
		r += dr;
		g += dg;
		b += db;
		w += dw;
	}
}

void draw_render_textured32_pc0(SDL_Surface *surf, Uint8 *dst_line, sbuffer_segment_t *segment, int x1,int x2)
{
	float u1,v1, u2,v2, duf,dvf;
	int dxtotal, dx, i;
	render_texture_t *tex = segment->texture;
	Uint32 u,v,du,dv;
	Uint32 ubits, umask, vbits, vmask;
	Uint32 *dst_col = (Uint32 *) dst_line;

	dxtotal = segment->end.x - segment->start.x + 1;
	dx = x1-segment->start.x;

	u1 = segment->start.u * 65536.0f;
	v1 = segment->start.v * 65536.0f;
	u2 = segment->end.u * 65536.0f;
	v2 = segment->end.v * 65536.0f;

	duf = (u2-u1)/dxtotal;
	dvf = (v2-v1)/dxtotal;

	u = u1 + duf * dx;
	v = v1 + dvf * dx;
	du = duf;
	dv = dvf;

	ubits = logbase2(tex->pitchw);
	umask = (1<<ubits)-1;
	vbits = logbase2(tex->pitchh);
	vmask = (1<<vbits)-1;
	vmask <<= ubits;

	if (tex->paletted) {
		Uint32 *palette = tex->palettes[segment->tex_num_pal];
		Uint8 *alpha_pal = tex->alpha_palettes[segment->tex_num_pal];
		Uint8 *tex_pixels = (Uint8 *) tex->pixels;

		for (i=x1; i<=x2; i++) {
			Uint8 c;
			Uint32 pu,pv;

			pu = u>>16;		/* 0000XXXX */
			pu &= umask;		/* 0000---X */
			pv = v>>(16-ubits);	/* 000YYYYy */
			pv &= vmask;		/* 000YYYY- */

#ifdef FORCE_OPAQUE
			*dst_col++ = palette[tex_pixels[pv|pu]];
#else
			c = tex_pixels[pv|pu];
			if (alpha_pal[c]) {
				*dst_col = palette[c];
			}
			dst_col++;
#endif

			u += du;
			v += dv;
		}
	} else {
		Uint32 *tex_pixels = (Uint32 *) tex->pixels;

		for (i=x1; i<=x2; i++) {
			Uint32 pu,pv;

			pu = u>>16;		/* 0000XXXX */
			pu &= umask;		/* 0000---X */
			pv = v>>(16-ubits);	/* 000YYYYy */
			pv &= vmask;		/* 000YYYY- */

			*dst_col++ = tex_pixels[pv|pu];

			u += du;
			v += dv;
		}
	}
}

void draw_render_textured32_pc1(SDL_Surface *surf, Uint8 *dst_line, sbuffer_segment_t *segment, int x1,int x2)
{
	float u1,v1, u2,v2, invw, duf,dvf;
	int dxtotal, dx, i;
	render_texture_t *tex = segment->texture;
	Uint32 u,v,du,dv;
	Uint32 ubits, umask, vbits, vmask;
	Uint32 *dst_col = (Uint32 *) dst_line;

	dxtotal = segment->end.x - segment->start.x + 1;
	dx = x1-segment->start.x;

	invw = 65536.0f / segment->start.w;
	u1 = segment->start.u * invw;
	v1 = segment->start.v * invw;
	invw = 65536.0f / segment->end.w;
	u2 = segment->end.u * invw;
	v2 = segment->end.v * invw;

	duf = (u2-u1)/dxtotal;
	dvf = (v2-v1)/dxtotal;

	u = u1 + duf * dx;
	v = v1 + dvf * dx;
	du = duf;
	dv = dvf;

	ubits = logbase2(tex->pitchw);
	umask = (1<<ubits)-1;
	vbits = logbase2(tex->pitchh);
	vmask = (1<<vbits)-1;
	vmask <<= ubits;

	if (tex->paletted) {
		Uint32 *palette = tex->palettes[segment->tex_num_pal];
		Uint8 *alpha_pal = tex->alpha_palettes[segment->tex_num_pal];
		Uint8 *tex_pixels = (Uint8 *) tex->pixels;

		for (i=x1; i<=x2; i++) {
			Uint8 c;
			Uint32 pu,pv;

			pu = u>>16;		/* 0000XXXX */
			pu &= umask;		/* 0000---X */
			pv = v>>(16-ubits);	/* 000YYYYy */
			pv &= vmask;		/* 000YYYY- */

#ifdef FORCE_OPAQUE
			*dst_col++ = palette[tex_pixels[pv|pu]];
#else
			c = tex_pixels[pv|pu];
			if (alpha_pal[c]) {
				*dst_col = palette[c];
			}
			dst_col++;
#endif

			u += du;
			v += dv;
		}
	} else {
		Uint32 *tex_pixels = (Uint32 *) tex->pixels;

		for (i=x1; i<=x2; i++) {
			Uint32 pu,pv;

			pu = u>>16;		/* 0000XXXX */
			pu &= umask;		/* 0000---X */
			pv = v>>(16-ubits);	/* 000YYYYy */
			pv &= vmask;		/* 000YYYY- */

			*dst_col++ = tex_pixels[pv|pu];

			u += du;
			v += dv;
		}
	}
}

void draw_render_textured32_pc2(SDL_Surface *surf, Uint8 *dst_line, sbuffer_segment_t *segment, int x1,int x2)
{
	float u1,v1, u2,v2, du,dv, u,v;
	float w1, w2, w, dw, invw;
	float du16,dv16,dw16;
	int dxtotal, i;
	Uint32 ubits, umask, vbits, vmask;
	render_texture_t *tex = segment->texture;
	Uint32 *dst_col = (Uint32 *) dst_line;

	dxtotal = segment->end.x - segment->start.x + 1;

	u1 = segment->start.u;
	v1 = segment->start.v;
	w1 = segment->start.w;

	u2 = segment->end.u;
	v2 = segment->end.v;
	w2 = segment->end.w;

	du = (u2-u1)/dxtotal;
	dv = (v2-v1)/dxtotal;
	dw = (w2-w1)/dxtotal;

	u1 += du * (x1-segment->start.x);
	v1 += dv * (x1-segment->start.x);
	w1 += dw * (x1-segment->start.x);

	du16 = du * 16.0f;
	dv16 = dv * 16.0f;
	dw16 = dw * 16.0f;

	ubits = logbase2(tex->pitchw);
	umask = (1<<ubits)-1;
	vbits = logbase2(tex->pitchh);
	vmask = (1<<vbits)-1;
	vmask <<= ubits;

	if (tex->paletted) {
		Uint32 *palette = tex->palettes[segment->tex_num_pal];
		Uint8 *alpha_pal = tex->alpha_palettes[segment->tex_num_pal];
		Uint8 *tex_pixels = (Uint8 *) tex->pixels;

		for (i=x1; i<=x2; i+=16) {
			int j;
			Uint32 dui, dvi, uu, vv, uu2,vv2;

			u2 = u1 + du16;
			v2 = v1 + dv16;
			w2 = w1 + dw16;

			invw = 65536.0f / w1;
			uu = u1 * invw;
			vv = v1 * invw;
			invw = 65536.0f / w2;
			uu2 = u2 * invw;
			vv2 = v2 * invw;

			dui = (uu2-uu)/16.0f;
			dvi = (vv2-vv)/16.0f;

			for (j=0; j<MIN(x2-i+1,16); j++) {
				Uint8 c;
				Uint32 pu,pv;

				pu = uu>>16;		/* 0000XXXX */
				pu &= umask;		/* 0000---X */
				pv = vv>>(16-ubits);	/* 000YYYYy */
				pv &= vmask;		/* 000YYYY- */

#ifdef FORCE_OPAQUE
				*dst_col++ = palette[tex_pixels[pv|pu]];
#else
				c = tex_pixels[pv|pu];
				if (alpha_pal[c]) {
					*dst_col = palette[c];
				}
				dst_col++;
#endif

				uu += dui;
				vv += dvi;
			}

			u1 = u2;
			v1 = v2;
			w1 = w2;
		}
	} else {
		Uint32 *tex_pixels = (Uint32 *) tex->pixels;

		for (i=x1; i<=x2; i+=16) {
			int j;
			Uint32 dui, dvi, uu, vv, uu2,vv2;

			u2 = u1 + du16;
			v2 = v1 + dv16;
			w2 = w1 + dw16;

			invw = 65536.0f / w1;
			uu = u1 * invw;
			vv = v1 * invw;
			invw = 65536.0f / w2;
			uu2 = u2 * invw;
			vv2 = v2 * invw;

			dui = (uu2-uu)/16.0f;
			dvi = (vv2-vv)/16.0f;

			for (j=0; j<MIN(x2-i+1,16); j++) {
				Uint32 pu,pv;

				pu = uu>>16;		/* 0000XXXX */
				pu &= umask;		/* 0000---X */
				pv = vv>>(16-ubits);	/* 000YYYYy */
				pv &= vmask;		/* 000YYYY- */

				*dst_col++ = tex_pixels[pv|pu];

				uu += dui;
				vv += dvi;
			}

			u1 = u2;
			v1 = v2;
			w1 = w2;
		}
	}
}

void draw_render_textured32_pc3(SDL_Surface *surf, Uint8 *dst_line, sbuffer_segment_t *segment, int x1,int x2)
{
	float u1,v1, u2,v2, du,dv, u,v;
	float w1, w2, w, dw;
	int dxtotal, i;
	Uint32 ubits, umask, vbits, vmask;
	render_texture_t *tex = segment->texture;
	Uint32 *dst_col = (Uint32 *) dst_line;

	dxtotal = segment->end.x - segment->start.x + 1;

	u1 = segment->start.u;
	v1 = segment->start.v;
	w1 = segment->start.w;
	u2 = segment->end.u;
	v2 = segment->end.v;
	w2 = segment->end.w;

	du = (u2-u1)/dxtotal;
	dv = (v2-v1)/dxtotal;
	dw = (w2-w1)/dxtotal;

	u = u1 + du * (x1-segment->start.x);
	v = v1 + dv * (x1-segment->start.x);
	w = w1 + dw * (x1-segment->start.x);

	ubits = logbase2(tex->pitchw);
	umask = (1<<ubits)-1;
	vbits = logbase2(tex->pitchh);
	vmask = (1<<vbits)-1;
	vmask <<= ubits;

	if (tex->paletted) {
		Uint32 *palette = tex->palettes[segment->tex_num_pal];
		Uint8 *alpha_pal = tex->alpha_palettes[segment->tex_num_pal];
		Uint8 *tex_pixels = (Uint8 *) tex->pixels;

		for (i=x1; i<=x2; i++) {
			Uint8 c;
			Uint32 pu,pv;
			float invw;

			invw = 65536.0f / w;
			pu = u * invw;	/* XXXXxxxx */
			pv = v * invw;	/* YYYYyyyy */

			pu >>= 16;		/* 0000XXXX */
			pu &= umask;		/* 0000---X */
			pv >>= 16-ubits;	/* 000YYYYy */
			pv &= vmask;		/* 000YYYY- */

#ifdef FORCE_OPAQUE
			*dst_col++ = palette[tex_pixels[pv|pu]];
#else
			c = tex_pixels[pv|pu];
			if (alpha_pal[c]) {
				*dst_col = palette[c];
			}
			dst_col++;
#endif

			u += du;
			v += dv;
			w += dw;
		}
	} else {
		Uint32 *tex_pixels = (Uint32 *) tex->pixels;

		for (i=x1; i<=x2; i++) {
			Uint32 pu,pv;
			float invw;

			invw = 65536.0f / w;
			pu = u * invw;	/* XXXXxxxx */
			pv = v * invw;	/* YYYYyyyy */

			pu >>= 16;		/* 0000XXXX */
			pu &= umask;		/* 0000---X */
			pv >>= 16-ubits;	/* 000YYYYy */
			pv &= vmask;		/* 000YYYY- */

			*dst_col++ = tex_pixels[pv|pu];

			u += du;
			v += dv;
			w += dw;
		}
	}
}
