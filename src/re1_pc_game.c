/*
	RE1
	PC
	Game

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
#include "depack_pak.h"
#include "re1_pc_game.h"
#include "parameters.h"

/*--- Defines ---*/

/*--- Types ---*/

/*--- Variables ---*/

static const char *re1pcgame_bg = "horr/usa/stage%d/rc%d%02x%d.pak";

/*--- Functions prototypes ---*/

static int re1pcgame_load_pak_bg(const char *filename);

/*--- Functions ---*/

void re1pcgame_init(state_t *game_state)
{
	game_state->load_background = re1pcgame_loadbackground;
	game_state->shutdown = re1pcgame_shutdown;
}

void re1pcgame_shutdown(void)
{
}

void re1pcgame_loadbackground(void)
{
	char *filepath;

	filepath = malloc(strlen(re1pcgame_bg)+8);
	if (!filepath) {
		fprintf(stderr, "Can not allocate mem for filepath\n");
		return;
	}
	sprintf(filepath, re1pcgame_bg, game_state.stage, game_state.stage, game_state.room, game_state.camera);

	if (re1pcgame_load_pak_bg(filepath)) {
		printf("pak: Loaded %s\n", filepath);
	} else {
		fprintf(stderr, "pak: Can not load %s\n", filepath);
	}

	free(filepath);
}

int re1pcgame_load_pak_bg(const char *filename)
{
	SDL_RWops *src;
	int retval = 0;
	
	src = FS_makeRWops(filename);
	if (src) {
		Uint8 *dstBuffer;
		int dstBufLen;

		pak_depack(src, &dstBuffer, &dstBufLen);

		if (dstBuffer && dstBufLen) {
			SDL_RWops *tim_src;
			game_state.num_cameras = 8;
			
			tim_src = SDL_RWFromMem(dstBuffer, dstBufLen);
			if (tim_src) {
				game_state.background_surf = background_tim_load(tim_src);
				if (game_state.background_surf) {
					retval = 1;
				}

				SDL_FreeRW(tim_src);
			}

			free(dstBuffer);
		}

		SDL_RWclose(src);
	}

	return retval;
}