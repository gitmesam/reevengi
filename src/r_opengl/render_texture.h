/*
	Textures for 3D objects
	OpenGL backend

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

#ifndef RENDER_TEXTURE_OPENGL_H
#define RENDER_TEXTURE_OPENGL_H 1

/*--- External types ---*/

struct render_texture_s;

/*--- Types ---*/

typedef struct render_texture_gl_s render_texture_gl_t;

struct render_texture_gl_s {
	struct render_texture_s render_texture;

	GLenum textureTarget;
	GLuint texture_id[MAX_TEX_PALETTE];
};

/*--- Functions prototypes ---*/

/* Create a texture */
struct render_texture_s *render_texture_gl_create(int flags);

#endif /* RENDER_TEXTURE_OPENGL_H */
