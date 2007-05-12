/*
	Main

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
#include <SDL.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "state.h"
#include "parameters.h"
#include "re1_ps1_demo.h"
#include "re1_ps1_game.h"
#include "re1_pc_game.h"
#include "re2_ps1_demo.h"
#include "re2_pc_demo.h"
#include "re3_ps1_game.h"
#include "re3_pc_demo.h"

/*--- Defines ---*/

#define KEY_STAGE_DOWN		SDLK_a
#define KEY_STAGE_UP		SDLK_q
#define KEY_STAGE_RESET		SDLK_w
#define KEY_ROOM_DOWN		SDLK_z
#define KEY_ROOM_UP		SDLK_s
#define KEY_ROOM_RESET		SDLK_x
#define KEY_CAMERA_DOWN		SDLK_e
#define KEY_CAMERA_UP		SDLK_d
#define KEY_CAMERA_RESET	SDLK_c
#define KEY_GAMMA_DOWN		SDLK_r
#define KEY_GAMMA_UP		SDLK_f
#define KEY_GAMMA_RESET		SDLK_v

/*--- Functions ---*/

int main(int argc, char **argv)
{
	int quit=0;
	int reload_bg = 1;
	int redraw_bg = 1;
	int switch_fs = 0;
	int videoflags;
	SDL_Surface *screen;

	if (!CheckParm(argc,argv)) {
		DisplayUsage();
		exit(1);
	}

	state_init();
	printf("Game version: ");
	switch(game_state.version) {
		case GAME_RE1_PS1_DEMO:
			printf("Resident Evil, PS1, Demo\n");
			re1ps1demo_init(&game_state);
			break;
		case GAME_RE1_PS1_GAME:
			printf("Resident Evil, PS1, Game\n");
			re1ps1game_init(&game_state);
			break;
		case GAME_RE2_PS1_DEMO:
			printf("Resident Evil 2, PS1, Demo\n");
			re2ps1demo_init(&game_state);
			break;
		case GAME_RE3_PS1_GAME:
			printf("Resident Evil 3, PS1, Game\n");
			re3ps1game_init(&game_state);
			break;
		case GAME_RE1_PC_GAME:
			printf("Resident Evil, PC, Game\n");
			re1pcgame_init(&game_state);
			break;
		case GAME_RE2_PC_DEMO:
			printf("Resident Evil 2, PC, Demo\n");
			re2pcdemo_init(&game_state);
			break;
		case GAME_RE3_PC_DEMO:
			printf("Resident Evil 3, PC, Demo\n");
			re3pcdemo_init(&game_state);
			break;
		default:
			printf("No known version\n");
			exit(1);
	}

	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		fprintf(stderr, "Can not initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	screen = SDL_SetVideoMode(320, 240, 16, 0);
	if (!screen) {
		fprintf(stderr, "Unable to create screen: %s\n", SDL_GetError());
		return 0;
	}
	videoflags = screen->flags;
	SDL_WM_SetCaption(PACKAGE_STRING, PACKAGE_NAME); 
	SDL_SetGamma(gamma, gamma, gamma);

	while (!quit) {
		SDL_Event event;

		/* Evenements */
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					quit=1;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE:
							quit=1;
							break;
						case SDLK_RETURN:
							if (event.key.keysym.mod & KMOD_ALT) {
								switch_fs=1;
							}
							break;
						case KEY_STAGE_DOWN:
							game_state.stage -= 1;
							if (game_state.stage < 1) {
								game_state.stage = 7;
							}
							reload_bg = 1;
							break;						
						case KEY_STAGE_UP:
							game_state.stage += 1;
							if (game_state.stage > 7) {
								game_state.stage = 1;
							}
							reload_bg = 1;
							break;						
						case KEY_STAGE_RESET:
							game_state.stage = 1;
							reload_bg = 1;
							break;						
						case KEY_ROOM_DOWN:
							game_state.room -= 1;
							if (game_state.room < 0) {
								game_state.room = 0x1c;
							}
							reload_bg = 1;
							break;						
						case KEY_ROOM_UP:
							game_state.room += 1;
							if (game_state.room > 0x1c) {
								game_state.room = 0;
							}
							reload_bg = 1;
							break;						
						case KEY_ROOM_RESET:
							game_state.room = 0;
							reload_bg = 1;
							break;						
						case KEY_CAMERA_DOWN:
							game_state.camera -= 1;
							if ((game_state.camera<0) && (game_state.num_cameras>0)) {
								game_state.camera = game_state.num_cameras-1;
							}
							reload_bg = 1;
							break;						
						case KEY_CAMERA_UP:
							game_state.camera += 1;
							if (game_state.camera>=game_state.num_cameras) {
								game_state.camera = 0;
							}
							reload_bg = 1;
							break;						
						case KEY_CAMERA_RESET:
							game_state.camera = 0;
							reload_bg = 1;
							break;						
						case KEY_GAMMA_DOWN:
							gamma -= 0.1;
							if (gamma<0.1) {
								gamma = 0.1;
							}
							SDL_SetGamma(gamma, gamma, gamma);
							break;
						case KEY_GAMMA_UP:
							gamma += 0.1;
							if (gamma>2.0) {
								gamma = 2.0;
							}
							SDL_SetGamma(gamma, gamma, gamma);
							break;
						case KEY_GAMMA_RESET:
							gamma = 1.0;
							SDL_SetGamma(gamma, gamma, gamma);
							break;						
					}
					break;
			}
		}

		/* Etat */
		if (reload_bg) {
			reload_bg = 0;

			/* depack background image */
			state_loadbackground();

			/* redraw */
			if (game_state.background_surf) {
				if (SDL_MUSTLOCK(screen)) {
					SDL_LockSurface(screen);
				}
				SDL_BlitSurface(game_state.background_surf, NULL, screen, NULL);
				if (SDL_MUSTLOCK(screen)) {
					SDL_UnlockSurface(screen);
				}
				if (screen->flags & SDL_DOUBLEBUF) {
					SDL_Flip(screen);
				} else {
					SDL_UpdateRect(screen, 0,0,0,0);
				}
			}
		}

		if (switch_fs) {
			switch_fs=0;
			videoflags ^= SDL_FULLSCREEN;
			screen = SDL_SetVideoMode(320, 240, 16, videoflags);
			videoflags = screen->flags;
			reload_bg=1;
		}

		SDL_Delay(1);
	}
	
	state_unloadbackground();
	state_shutdown();

	return 0;
}
