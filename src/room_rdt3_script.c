/*
	Room description
	RE3 RDT script

	Copyright (C) 2009-2010	Patrice Mandin

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

#include "room.h"
#include "room_rdt2.h"
#include "log.h"
#include "room_rdt3_script_common.h"
#include "room_rdt3_script_dump.h"

/*--- Defines ---*/

#define INST_END_SCRIPT	0xff

/*--- Types ---*/

/*
typedef struct {
	Uint8 type;
	Uint8 flag;
} script_condition_27_t;

typedef struct {
	Uint8 type;
	Uint8 flag;
	Uint8 object;
	Uint8 value;
} script_condition_ck_t;

typedef struct {
	Uint8 type;
	Uint8 flag;
	Uint8 object;
	Uint8 value;
	Uint16 value2;
} script_condition_cmp_t;

typedef struct {
	Uint8 type;
	Uint8 flag;
	Uint8 object;
	Uint8 value;
	Uint8 unknown[6];
} script_condition_cc_t;

typedef union {
	Uint8 type;
	script_condition_ck_t	ck;
	script_condition_cmp_t	cmp;
	script_condition_cc_t	cc;
} script_condition_t;

typedef struct {
	Uint8 opcode;
	Uint8 variable;
	Uint16 value;
} script_set_t;
*/

typedef struct {
	Uint8 opcode;
	Uint8 length;
} script_inst_len_t;

/*--- Variables ---*/

static const script_inst_len_t inst_length[]={
	/* 0x00-0x0f */
	{INST_NOP,	1},
	{INST_RETURN,	2},
	{INST_SLEEP_1,	1},
	{0x03,		2},
	{INST_EVT_EXEC,	sizeof(script_exec_t)},
	{INST_EVT_KILL,	2},
	{INST_IF,	sizeof(script_if_t)},
	{INST_ELSE,	sizeof(script_else_t)},
	{INST_END_IF,	2},
	{0x09,		1},
	{INST_SLEEP_N,	sizeof(script_sleepn_t)},
	{INST_SLEEP_W,	1},
	{0x0c,		1},
	{INST_BEGIN_FOR,	sizeof(script_for_t)},
	{0x0e,		5},
	{INST_END_FOR,	2},

	/* 0x10-0x1f */
	{INST_BEGIN_WHILE,	sizeof(script_begin_while_t)},
	{INST_END_WHILE,	2},
	{INST_DO,	sizeof(script_do_t)},
	{INST_WHILE,	sizeof(script_while_t)},
	{INST_BEGIN_SWITCH,	sizeof(script_begin_switch_t)},
	{INST_CASE,	sizeof(script_case_t)},
	{INST_DEFAULT,	2},
	{INST_END_SWITCH,	2},
	{0x18,		6},
	{INST_FUNC,	2},
	{0x1a,		4},
	{INST_BREAK,	2},	
	{0x1c,		1},
	{INST_EVAL_CC,	4},
	{INST_VALUE_SET,	4},
	{INST_SET1,	4},

	/* 0x20-0x2f */
	{INST_CALC_OP,	6},
	{0x21,		4},
	{INST_EVT_CUT,	4},
	{0x23,		1},
	{0x24,		2},
	{0x25,		4},
	{0x26,		6},
	{0x27,		1},
	{0x28,		1},
	{0x29,		8},
	{0x2a,		6},
	{0x2c,		2},
	{INST_LINE_BEGIN,	sizeof(script_line_begin_t)},
	{INST_LINE_MAIN,	sizeof(script_line_main_t)},
	{INST_LINE_END,		2},

	/* 0x30-0x3f */
	{0x30,		6},
	{0x31,		6},
	{INST_LIGHT_COLOR_SET,		6},
	{INST_AHEAD_ROOM_SET,	sizeof(script_ahead_room_set_t)},
	{0x34,		10},
	{INST_EVAL_BGM_TBL_CK,		6},
	{0x36,		3},
	{0x37,		2},
	{0x38,		2},
	{0x39,		16},
	{0x3a,		16},
	{INST_CHASER_ITEM_SET,		3},
	{0x3c,		1},
	{0x3d,		2},
	{0x3e,		2},
	{INST_FLOOR_SET,	sizeof(script_floor_set_t)},

	/* 0x40-0x4f */
	{INST_VAR_SET,		4},
	{INST_CALC_STORE,	3},
	{INST_CALC_LOAD,	3},
	{0x43,		6},
	{0x44,		6},
	{0x45,		4},
	{INST_FADE_SET,	sizeof(script_fade_set_t)},
	{INST_WORK_SET,	sizeof(script_work_set_t)},
	{0x48,		4},
	{0x49,		1},
	{0x4a,		1},
	{0x4b,		1},
	{INST_EVAL_CK,	4},
	{INST_FLAG_SET,	sizeof(script_set_flag_t)},
	{INST_EVAL_CMP,	6},
	{0x4f,		1},

	/* 0x50-0x5f */
	{INST_CUT_CHG,	2},
	{0x51,		1},
	{INST_CUT_AUTO,		2},
	{INST_CUT_REPLACE,	sizeof(script_cut_replace_t)},
	{0x54,		4},
	{INST_POS_SET,		8},
	{INST_DIR_SET,		8},
	{INST_SET_VIB0,		6},
	{0x58,		6},
	{INST_SET_VIB_FADE,	8},
	{INST_RBJ_SET,		2},
	{INST_MESSAGE_ON,	6},
	{0x5c,		2},
	{0x5d,		1},
	{0x5e,		3},
	{0x5f,		2},

	/* 0x60-0x6f */
	{0x60,			22},
	{INST_DOOR_SET,	sizeof(script_door_set_t)},
	{0x62,			40},
	{INST_AOT_SET,		20},
	{INST_AOT_SET_4P,	28},
	{INST_AOT_RESET,	10},
	{0x66,			2},
	{INST_ITEM_AOT_SET,	22},
	{0x68,			30},
	{INST_KAGE_SET,	14},
	{INST_SUPER_SET,	16},
	{0x6b,			2},
	{0x6c,			4},
	{0x6d,			4},
	{INST_SCA_ID_SET,	4},
	{0x6f,			2},

	/* 0x70-0x7f */
	{INST_ESPR_ON,	16},
	{0x71, 		18},
	{0x72, 		22},
	{INST_ESPR3D_ON2,	24},
	{INST_ESPR_KILL,	5},
	{INST_ESPR_KILL2,	2},
	{0x76, 		3},
	{INST_SE_ON,	12},
	{INST_BGM_CTL,	6},
	{INST_XA_ON,	4},
	{0x7a, 		2},
	{INST_BGM_TBL_SET,	6},
	{0x7c,		1},
	{INST_EM_SET,	24},
	{0x7e,		2},
	{INST_OM_SET,	40},

	/* 0x80-0x8f */
	{INST_PLC_MOTION,	4},
	{INST_PLC_DEST,		8},
	{INST_PLC_NECK,		10},
	{INST_PLC_RET,		1},
	{INST_PLC_FLG,		4},
	{0x85,		2},
	{0x86,		1},
	{INST_PLC_STOP,	1},
	{INST_PLC_ROT,	4},
	{INST_PLC_CNT,	2},
	{0x8a,		1},
	{0x8b,		1},
	{0x8c,		1},
	{0x8e,		4},
	{0x8f,		2}
};

/*--- Functions prototypes ---*/

static Uint8 *scriptFirstInst(room_t *this, int num_script);
static int scriptGetInstLen(Uint8 *curInstPtr);
static void scriptExecInst(room_t *this);

/*--- Functions ---*/

void room_rdt3_scriptInit(room_t *this)
{
	this->scriptPrivFirstInst = scriptFirstInst;
	this->scriptPrivGetInstLen = scriptGetInstLen;
	this->scriptPrivExecInst = scriptExecInst;

	this->scriptDump = room_rdt3_scriptDump;
}

static Uint8 *scriptFirstInst(room_t *this, int num_script)
{
	rdt2_header_t *rdt_header;
	Uint32 offset, smaller_offset;
	Uint16 *functionArrayPtr;
	int i, room_script = RDT2_OFFSET_INIT_SCRIPT;

	if (!this) {
		return NULL;
	}
	if (num_script == ROOM_SCRIPT_RUN) {
		return NULL;
		/*room_script = RDT2_OFFSET_ROOM_SCRIPT;*/
	}

	rdt_header = (rdt2_header_t *) this->file;
	offset = SDL_SwapLE32(rdt_header->offsets[room_script]);

	this->script_length = this->cur_inst_offset = 0;
	this->cur_inst = NULL;

	if (offset>0) {
		/* Search smaller offset after script to calc length */
		smaller_offset = this->file_length;
		for (i=0; i<21; i++) {
			Uint32 next_offset = SDL_SwapLE32(rdt_header->offsets[i]);
			if ((next_offset>0) && (next_offset<smaller_offset) && (next_offset>offset)) {
				smaller_offset = next_offset;
			}
		}
		if (smaller_offset>offset) {
			this->script_length = smaller_offset - offset;
		}

		/* Start of script is an array of offsets to the various script functions
		 * The first offset also gives the first instruction to execute
		 */
		functionArrayPtr = (Uint16 *) (& ((Uint8 *) this->file)[offset]);

		this->cur_inst_offset = SDL_SwapLE16(functionArrayPtr[0]);
		this->cur_inst = (& ((Uint8 *) this->file)[offset + this->cur_inst_offset]);
	}

	logMsg(1, "rdt3: Script %d at offset 0x%08x, length 0x%04x\n", num_script, offset, this->script_length);

	return this->cur_inst;
}

static int scriptGetInstLen(Uint8 *curInstPtr)
{
	int i;

	assert(curInstPtr);

	for (i=0; i< sizeof(inst_length)/sizeof(script_inst_len_t); i++) {
		if (inst_length[i].opcode == curInstPtr[0]) {
			return inst_length[i].length;
		}
	}

	return 0;
}

static void scriptExecInst(room_t *this)
{
	script_inst_t *inst;

	if (!this) {
		return;
	}
	if (!this->cur_inst) {
		return;
	}

	inst = (script_inst_t *) this->cur_inst;

	switch(inst->opcode) {
		case INST_DOOR_SET:
			{
				script_door_set_t *doorSet = (script_door_set_t *) inst;
				room_door_t roomDoor;

				roomDoor.x = SDL_SwapLE16(doorSet->x);
				roomDoor.y = SDL_SwapLE16(doorSet->y);
				roomDoor.w = SDL_SwapLE16(doorSet->w);
				roomDoor.h = SDL_SwapLE16(doorSet->h);

				roomDoor.next_x = SDL_SwapLE16(doorSet->next_x);
				roomDoor.next_y = SDL_SwapLE16(doorSet->next_y);
				roomDoor.next_z = SDL_SwapLE16(doorSet->next_z);
				roomDoor.next_dir = SDL_SwapLE16(doorSet->next_dir);

				roomDoor.next_stage = doorSet->next_stage+1;
				roomDoor.next_room = doorSet->next_room;
				roomDoor.next_camera = doorSet->next_camera;

				this->addDoor(this, &roomDoor);
			}
			break;
	}
}
