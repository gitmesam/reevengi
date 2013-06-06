/*
	Room data

	Copyright (C) 2007-2013	Patrice Mandin

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

#ifndef ROOM_H
#define ROOM_H 1

/*--- Defines ---*/

#define ROOM_SCRIPT_INIT 0	/* Initialization script, run once */
#define ROOM_SCRIPT_RUN	1	/* Running script, while the player is in the room */

enum {
	ROOM_MAP_OFF=0,
	ROOM_MAP_2D,
	ROOM_MAP_2D_TO_3D,
	ROOM_MAP_3D,
	ROOM_MAP_3D_TO_2D
};

/*--- External types ---*/

typedef struct render_texture_s render_texture_t;
typedef struct render_mask_s render_mask_t;

typedef struct room_camswitch_s room_camswitch_t;
typedef struct room_door_s room_door_t;
typedef struct room_collision_s room_collision_t;

/*--- Types ---*/

typedef struct room_camera_s room_camera_t;

struct room_camera_s {
	Sint32 from_x;	/* Camera is there */
	Sint32 from_y;
	Sint32 from_z;
	Sint32 to_x;	/* looking at this point */
	Sint32 to_y;
	Sint32 to_z;
};

typedef struct room_s room_t;

struct room_s {
	void (*dtor)(room_t *this);

	/* Return full path to room filename, ptr must be freed */
	char *(*getFilename)(room_t *this, int stage, int room, int camera);
	void (*loadFile)(room_t *this, int stage, int room, int camera);

	void (*load)(room_t *this, int stage, int room, int camera);
	void (*init)(room_t *this);

	void (*load_background)(room_t *this, int stage, int room, int camera);
	void (*load_bgmask)(room_t *this, int stage, int room, int camera);
	void (*changeCamera)(room_t *this, int stage, int room, int camera);

	/* RDT file */
	void *file;
	Uint32 file_length;

	/* Background image for current camera */
	render_texture_t *background;

	/*--- Camera positions ---*/
	int num_cameras;

	int (*getNumCameras)(room_t *this);
	void (*getCamera)(room_t *this, int num_camera, room_camera_t *room_camera);

	/*--- Camera switches ---*/
	int (*getNumCamSwitches)(room_t *this);
	void (*getCamSwitch)(room_t *this, int num_camswitch, room_camswitch_t *room_camswitch);
	int (*checkCamSwitch)(room_t *this, int num_camera, float x, float y);
	void (*drawCamSwitches)(room_t *this);

	/*--- Boundaries ---*/
	int (*getNumBoundaries)(room_t *this);
	void (*getBoundary)(room_t *this, int num_boundary, room_camswitch_t *room_boundary);
	int (*checkBoundary)(room_t *this, int num_camera, float x, float y);
	void (*drawBoundaries)(room_t *this);

	/*--- Background masking ---*/
	render_texture_t *bg_mask;
	render_mask_t *rdr_mask;

	void (*initMasks)(room_t *this, int num_camera);
	void (*drawMasks)(room_t *this, int num_camera);

	/*--- Texts ---*/
	void (*getText)(room_t *this, int lang, int num_text, char *buffer, int bufferLen);

	/*--- Script execution ---*/
	Uint8 *cur_inst;
	int cur_inst_offset;
	int script_length;

	Uint8 *(*scriptInit)(room_t *this, int num_script);	/* Init a script, return ptr of first inst */
	int (*scriptGetInstLen)(room_t *this, Uint8 *curInstPtr);	/* Get current instruction length */
	void (*scriptExecInst)(room_t *this);	/* Execute an instruction */

	void (*scriptDump)(room_t *this, int num_script);	/* Dump a script */
	void (*scriptExec)(room_t *this, int num_script);	/* Execute script */

	/*--- Doors ---*/
	int num_doors;
	room_door_t *doors;

	void (*addDoor)(room_t *this, room_door_t *door);

	/* Return door entered, of NULL if none entered */
	room_door_t *(*enterDoor)(room_t *this, Sint16 x, Sint16 y);

	/*--- Collision objects ---*/
	int (*getNumCollisions)(room_t *this);
	void (*getCollision)(room_t *this, int num_collision, room_collision_t *room_collision);

	/*--- Map ---*/
	int map_mode;

	void (*toggleMapModePrev)(room_t *this);
	void (*toggleMapModeNext)(room_t *this);
	void (*drawMap)(room_t *this);
};

/*--- Variables ---*/

/*--- Functions ---*/

room_t *room_ctor(void);

#endif /* ROOM_H */
