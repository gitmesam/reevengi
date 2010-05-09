/*
	RE2
	PC
	Demo

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

#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#include "filesystem.h"
#include "state.h"
#include "re2_pc_demo.h"
#include "depack_adt.h"
#include "parameters.h"
#include "video.h"
#include "render.h"
#include "log.h"
#include "model_emd2.h"
#include "room_rdt2.h"

/*--- Defines ---*/

#define MAX_MODELS	0x17

/*--- Types ---*/

/*--- Constant ---*/

static const char *re2pcdemo_bg = "common/stage%d/rc%d%02x%1x.adt";
static const char *re2pcdemo_bgmask = "common/stage%d/rs%d%02x%1x.adt";
static const char *re2pcdemo_room = "pl0/rd%c/room%d%02x0.rdt";
static const char *re2pcdemo_model = "pl0/emd0/em0%02x.%s";
static const char *re2pcdemo_font = "common/dat%c/select_w.tim";

static const int map_models[MAX_MODELS]={
	0x10,	0x11,	0x12,	0x13,	0x15,	0x16,	0x17,	0x18,
	0x1e,	0x1f,	0x20,	0x21,	0x22,	0x2d,	0x48,	0x4a,
	0x50,	0x51,	0x54,	0x55,	0x58,	0x59,	0x5a
};

/*--- Variables ---*/

static int game_lang = 'u';

/*--- Functions prototypes ---*/

static void re2pcdemo_shutdown(void);

static void re2pcdemo_loadbackground(void);
static void re2pcdemo_loadbackground_mask(void);
static int re2pcdemo_load_adt_bg(const char *filename);
static int re2pcdemo_load_adt_bgmask(const char *filename);

static void re2pcdemo_loadroom(void);
static int re2pcdemo_loadroom_rdt(const char *filename);

static render_skel_t *re2pcdemo_load_model(int num_model);

static void load_font(void);
static void get_char(int ascii, int *x, int *y, int *w, int *h);

static void get_model_name(char name[32]);

/*--- Functions ---*/

void re2pcdemo_init(state_t *game_state)
{
	game_state->priv_load_background = re2pcdemo_loadbackground;
	game_state->priv_load_bgmask = re2pcdemo_loadbackground_mask;
	game_state->priv_load_room = re2pcdemo_loadroom;
	game_state->priv_shutdown = re2pcdemo_shutdown;

	if (game_state->version == GAME_RE2_PC_DEMO_P) {
		game_lang = 'p';
	}

	game_state->priv_load_model = re2pcdemo_load_model;

	game_state->load_font = load_font;
	game_state->get_char = get_char;

	game_state->get_model_name = get_model_name;
}

static void re2pcdemo_shutdown(void)
{
}

static void re2pcdemo_loadbackground(void)
{
	char *filepath;

	filepath = malloc(strlen(re2pcdemo_bg)+8);
	if (!filepath) {
		fprintf(stderr, "Can not allocate mem for filepath\n");
		return;
	}
	sprintf(filepath, re2pcdemo_bg, game_state.num_stage, game_state.num_stage,
		game_state.num_room, game_state.num_camera);

	logMsg(1, "adt: Start loading %s ...\n", filepath);

	logMsg(1, "adt: %s loading %s ...\n",
		re2pcdemo_load_adt_bg(filepath) ? "Done" : "Failed",
		filepath);

	free(filepath);
}

static int re2pcdemo_load_adt_bg(const char *filename)
{
	SDL_RWops *src;
	int retval = 0;
	
	src = FS_makeRWops(filename);
	if (src) {
		Uint8 *dstBuffer;
		int dstBufLen;

		adt_depack(src, &dstBuffer, &dstBufLen);

		if (dstBuffer && dstBufLen) {
			if (dstBufLen == 320*256*2) {
				SDL_Surface *image = adt_surface((Uint16 *) dstBuffer, 1);
				if (image) {
					game_state.background = render.createTexture(RENDER_TEXTURE_CACHEABLE);
					if (game_state.background) {
						game_state.background->load_from_surf(game_state.background, image);
						retval = 1;
					}
					SDL_FreeSurface(image);
				}
			}
			free(dstBuffer);
		}
		SDL_RWclose(src);
	}

	return retval;
}

static void re2pcdemo_loadbackground_mask(void)
{
	char *filepath;

	filepath = malloc(strlen(re2pcdemo_bgmask)+8);
	if (!filepath) {
		fprintf(stderr, "Can not allocate mem for filepath\n");
		return;
	}
	sprintf(filepath, re2pcdemo_bgmask, game_state.num_stage, game_state.num_stage,
		game_state.num_room, game_state.num_camera);

	logMsg(1, "adt: Start loading %s ...\n", filepath);

	logMsg(1, "adt: %s loading %s ...\n",
		re2pcdemo_load_adt_bgmask(filepath) ? "Done" : "Failed",
		filepath);

	free(filepath);
}

static int re2pcdemo_load_adt_bgmask(const char *filename)
{
	SDL_RWops *src;
	int retval = 0;
	
	src = FS_makeRWops(filename);
	if (src) {
		Uint8 *dstBuffer;
		int dstBufLen;

		adt_depack(src, &dstBuffer, &dstBufLen);

		if (dstBuffer && dstBufLen) {
			game_state.bg_mask = render.createTexture(RENDER_TEXTURE_MUST_POT);
			if (game_state.bg_mask) {
				game_state.bg_mask->load_from_tim(game_state.bg_mask, dstBuffer);

				game_state.bg_mask->mark_trans(game_state.bg_mask, 0,
					0,0,
					game_state.bg_mask->w, game_state.bg_mask->h);

				retval = 1;
			}
			free(dstBuffer);
		}
		SDL_RWclose(src);
	}

	return retval;
}

static void re2pcdemo_loadroom(void)
{
	char *filepath;

	filepath = malloc(strlen(re2pcdemo_room)+8);
	if (!filepath) {
		fprintf(stderr, "Can not allocate mem for filepath\n");
		return;
	}
	sprintf(filepath, re2pcdemo_room, game_lang, game_state.num_stage, game_state.num_room);

	logMsg(1, "adt: Start loading %s ...\n", filepath);

	logMsg(1, "adt: %s loading %s ...\n",
		re2pcdemo_loadroom_rdt(filepath) ? "Done" : "Failed",
		filepath);

	free(filepath);
}

static int re2pcdemo_loadroom_rdt(const char *filename)
{
	PHYSFS_sint64 length;
	void *file;

	file = FS_Load(filename, &length);
	if (!file) {
		return 0;
	}
	
	game_state.room = room_create(file, length);
	if (!game_state.room) {
		free(file);
		return 0;
	}

	room_rdt2_init(game_state.room);

	return 1;
}

render_skel_t *re2pcdemo_load_model(int num_model)
{
	char *filepath;
	render_skel_t *model = NULL;
	void *emd, *tim;
	PHYSFS_sint64 emd_length, tim_length;

	if (num_model>=MAX_MODELS) {
		num_model = MAX_MODELS-1;
	}
	num_model = map_models[num_model];

	filepath = malloc(strlen(re2pcdemo_model)+8);
	if (!filepath) {
		fprintf(stderr, "Can not allocate mem for filepath\n");
		return NULL;
	}
	sprintf(filepath, re2pcdemo_model, num_model, "emd");

	logMsg(1, "emd: Start loading model %s ...\n", filepath);

	emd = FS_Load(filepath, &emd_length);
	if (emd) {
		sprintf(filepath, re2pcdemo_model, num_model, "tim");
		tim = FS_Load(filepath, &tim_length);
		if (tim) {
			model = model_emd2_load(emd, tim, emd_length, tim_length);
			free(tim);
		}
		free(emd);
	}	

	logMsg(1, "emd: %s loading model %s ...\n",
		model ? "Done" : "Failed",
		filepath);

	free(filepath);
	return model;
}

static void load_font(void)
{
	Uint8 *font_file;
	PHYSFS_sint64 length;
	int retval = 0;
	char *filepath;
	const char *filename = re2pcdemo_font;

	filepath = malloc(strlen(filename)+8);
	if (!filepath) {
		fprintf(stderr, "Can not allocate mem for filepath\n");
		return;
	}
	sprintf(filepath, filename, game_lang);

	logMsg(1, "Loading font from %s...\n", filepath);

	font_file = FS_Load(filepath, &length);
	if (font_file) {
		game_state.font = render.createTexture(0);
		if (game_state.font) {
			game_state.font->load_from_tim(game_state.font, font_file);
			retval = 1;
		}

		free(font_file);
	}

	logMsg(1, "Loading font from %s... %s\n", filepath, retval ? "Done" : "Failed");

	free(filepath);
}

static void get_char(int ascii, int *x, int *y, int *w, int *h)
{
	*x = *y = 0;
	*w = 8;
	*h = 10;

	if ((ascii<=32) || (ascii>=96+27)) {
		return;
	}

	ascii -= 32;
	*x = (ascii & 31)<<3;
	*y = (ascii>>5)*10;
}

static void get_model_name(char name[32])
{
	int num_model = game_state.num_model;

	if (num_model>MAX_MODELS-1) {
		num_model = MAX_MODELS-1;
	}

	sprintf(name, "em0%02x.emd", map_models[num_model]);
}
