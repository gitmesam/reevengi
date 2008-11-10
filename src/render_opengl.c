/*
	Render engine
	OpenGL

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef ENABLE_OPENGL

#include <SDL.h>
#include <SDL_opengl.h>

#include "dyngl.h"

#include "render.h"

/*--- Functions prototypes ---*/

static void set_projection(float angle, float aspect, float z_near, float z_far);
static void set_modelview(float x_from, float y_from, float z_from,
	float x_to, float y_to, float z_to,
	float x_up, float y_up, float z_up);
static void scale(float x, float y, float z);
static void translate(float x, float y, float z);
static void push_matrix(void);
static void pop_matrix(void);

static void line(SDL_Surface *surf,
	float x1, float y1, float z1,
	float x2, float y2, float z2,
	Uint32 color);
static void scaled_image(SDL_Surface *surf, SDL_Surface *source,
	int x, int y, int w, int h);

/*--- Functions ---*/

void render_opengl_init(render_t *render)
{
	render->set_projection = set_projection;
	render->set_modelview = set_modelview;
	render->scale = scale;
	render->translate = translate;
	render->push_matrix = push_matrix;
	render->pop_matrix = pop_matrix;

	render->line = line;
	render->scaled_image = scaled_image;
}

static void set_projection(float angle, float aspect, float z_near, float z_far)
{
	gl.MatrixMode(GL_PROJECTION);
	gl.LoadIdentity();
	gluPerspective(angle, aspect, z_near, z_far);
}

static void set_modelview(float x_from, float y_from, float z_from,
	float x_to, float y_to, float z_to,
	float x_up, float y_up, float z_up)
{
	gl.MatrixMode(GL_MODELVIEW);
	gl.LoadIdentity();
	gluLookAt(x_from, y_from, z_from,
		x_to, y_to, z_to,
		x_up, y_up, z_up);
}

static void scale(float x, float y, float z)
{
	gl.Scalef(x,y,z);
}

static void translate(float x, float y, float z)
{
	gl.Translatef(x,y,z);
}

static void push_matrix(void)
{
	gl.PushMatrix();
}

static void pop_matrix(void)
{
	gl.PopMatrix();
}

static void line(SDL_Surface *surf,
	float x1, float y1, float z1,
	float x2, float y2, float z2,
	Uint32 color)
{
	gl.Color4ub((color>>16) & 0xff, (color>>8) & 0xff,
		color & 0xff, (color>>24) & 0xff);

	gl.Begin(GL_LINES);
	gl.Vertex3f(x1,y1,z1);
	gl.Vertex3f(x2,y2,z2);
	gl.End();
}

static void scaled_image(SDL_Surface *surf, SDL_Surface *source,
	int x, int y, int w, int h)
{
}

#else

void render_opengl_init(render_t *render)
{
}

#endif /* ENABLE_OPENGL */
