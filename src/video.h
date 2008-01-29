/*
	Video backend

	Copyright (C) 2007	Patrice Mandin

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

#ifndef VIDEO_H
#define VIDEO_H 1

typedef struct video_s video_t;

struct video_s {
	int width, height, bpp, flags;
	SDL_Surface *screen;
	int num_screenshot;

	void (*setVideoMode)(video_t *this, int width, int height, int bpp);
	void (*swapBuffers)(video_t *this);
	void (*screenShot)(video_t *this);

	void (*initScreen)(video_t *this);
	void (*refreshBackground)(video_t *this);
	void (*drawBackground)(video_t *this, SDL_Surface *surf);
};

void video_soft_init(video_t *this);

int video_opengl_loadlib(void);
void video_opengl_init(video_t *this);

#endif /* VIDEO_H */
