/*
	Rescale and display background

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

#ifndef RENDER_BACKGROUND_H
#define RENDER_BACKGROUND_H 1

void render_background_init(video_t *this, video_surface_t *source);
void render_background_shutdown(video_t *this);

void render_background(video_t *this, video_surface_t *source);

#endif /* RENDER_BACKGROUND_H */
