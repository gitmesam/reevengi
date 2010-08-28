/*
	Room description
	RE3 RDT script instructions

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

#ifndef ROOM_RDT3_SCRIPT_COMMON_H
#define ROOM_RDT3_SCRIPT_COMMON_H 1

/*--- Defines ---*/

/* 0x00-0x0f */
#define INST_NOP	0x00
#define INST_RETURN	0x01
#define INST_SLEEP_1	0x02
#define INST_EVT_EXEC	0x04
#define INST_EVT_KILL	0x05
#define INST_IF		0x06
#define INST_ELSE	0x07
#define INST_END_IF	0x08
#define INST_SLEEP_N	0x0a
#define INST_SLEEP_W	0x0b
#define INST_FOR	0x0d
#define INST_FOR_END	0x0f

/* 0x10-0x1f */
#define INST_WHILE	0x10
#define INST_WHILE_END	0x11
#define INST_DO		0x12
#define INST_DO_END	0x13
#define INST_SWITCH	0x14
#define INST_CASE	0x15
#define INST_SWITCH_END	0x17
#define INST_GOTO	0x18
#define INST_FUNC	0x19
#define INST_BREAK	0x1b
#define INST_EVAL_CC	0x1d
#define INST_VALUE_SET	0x1e
#define INST_SET1	0x1f

/* 0x20-0x2f */
#define INST_CALC_OP	0x20
#define INST_EVT_CUT	0x22
#define INST_LINE_BEGIN	0x2d
#define INST_LINE_MAIN	0x2e
#define INST_LINE_END	0x2f

/* 0x30-0x3f */
#define INST_LIGHT_COLOR_SET	0x32
#define INST_AHEAD_ROOM_SET	0x33
#define INST_EVAL_BGM_TBL_CK	0x35
#define INST_CHASER_ITEM_SET	0x3b
#define INST_FLOOR_SET	0x3f

/* 0x40-0x4f */
#define INST_VAR_SET	0x40
#define INST_CALC_STORE	0x41
#define INST_CALC_LOAD	0x42
#define INST_FADE_SET	0x46
#define INST_WORK_SET	0x47
#define INST_EVAL_CK	0x4c
#define INST_FLAG_SET	0x4d
#define INST_EVAL_CMP	0x4e

/* 0x50-0x5f */
#define INST_CUT_CHG	0x50
#define INST_CUT_AUTO	0x52
#define INST_CUT_REPLACE	0x53
#define INST_POS_SET	0x55
#define INST_DIR_SET	0x56
#define INST_SET_VIB0	0x57
#define INST_SET_VIB_FADE	0x59
#define INST_RBJ_SET	0x5a
#define INST_MESSAGE_ON	0x5b

/* 0x60-0x6f */
#define INST_DOOR_SET	0x61
#define INST_AOT_SET	0x63
#define INST_AOT_SET_4P	0x64
#define INST_AOT_RESET	0x65
#define INST_ITEM_AOT_SET	0x67
#define INST_KAGE_SET	0x69
#define INST_SUPER_SET	0x6a
#define INST_SCA_ID_SET	0x6e

/* 0x70-0x7f */
#define INST_ESPR_ON	0x70
#define INST_ESPR3D_ON2	0x73
#define INST_ESPR_KILL	0x74
#define INST_ESPR_KILL2	0x75
#define INST_SE_ON	0x77
#define INST_BGM_CTL	0x78
#define INST_XA_ON	0x79
#define INST_BGM_TBL_SET	0x7b
#define INST_EM_SET	0x7d
#define INST_OM_SET	0x7f

/* 0x80-0x89 */
#define INST_PLC_MOTION	0x80
#define INST_PLC_DEST	0x81
#define INST_PLC_NECK	0x82
#define INST_PLC_RET	0x83
#define INST_PLC_FLG	0x84
#define INST_PLC_STOP	0x87
#define INST_PLC_ROT	0x88
#define INST_PLC_CNT	0x89

/*--- Types ---*/

/* 0x00-0x0f */

typedef struct {
	Uint8 opcode;
	Uint8 mask;
	Uint8 func[2];
} script_exec_t;

typedef struct {
	Uint8 opcode;
	Uint8 unknown0;
	Uint16 block_length;
} script_if_t;	/* always followed by script_condition_t */

typedef struct {
	Uint8 opcode;
	Uint8 unknown0;
	Uint16 block_length;
} script_else_t;

typedef struct {
	Uint8 opcode;
	Uint8 delay;
	Uint8 unknown;
} script_sleepn_t;

typedef struct {
	Uint8 opcode;
	Uint8 unknown;
	Uint16 block_length;
	Uint16 count;
} script_for_t;

/* 0x10-0x1f */

typedef struct {
	Uint8 opcode;
	Uint8 unknown0;
	Uint16 block_length;
} script_while_t;	/* always followed by script_condition_t */

typedef struct {
	Uint8 opcode;
	Uint8 unknown;
} script_do_end_t;	/* always followed by script_condition_t */

typedef struct {
	Uint8 opcode;
	Uint8 object;
	Uint16 block_length;
} script_switch_t;

typedef struct {
	Uint8 opcode;
	Uint8 dummy;
	Uint16 block_length;
	Uint16 value;
} script_case_t;

typedef struct {
	Uint8 opcode;
	Uint8 num_func;
} script_func_t;

/* 0x20-0x2f */

typedef struct {
	Uint8 opcode;
	Uint8 flag;
	Uint8 size;
	Uint8 dummy;
} script_line_begin_t;

typedef struct {
	Uint8 opcode;
	Uint8 flag;
	Uint16 p1, p2;
} script_line_main_t;

/* 0x30-0x3f */

typedef struct {
	Uint8 opcode;
	Uint8 dummy;
	Uint16 value;
} script_ahead_room_set_t;

typedef struct {
	Uint8 opcode;
	Uint8 num_floor;
	Uint8 value;
} script_floor_set_t;

/* 0x40-0x4f */

typedef struct {
	Uint8 opcode;
	Uint8 num_var;
	Sint16 value;
} script_var_set_t;

typedef struct {
	Uint8 opcode;
	Uint8 unknown0[10];
} script_fade_set_t;

typedef struct {
	Uint8 opcode;
	Uint8 unknown;
	Uint8 object;
} script_work_set_t;

typedef struct {
	Uint8 opcode;
	Uint8 flag;
	Uint8 object;
	Uint8 value;
} script_set_flag_t;

/* 0x50-0x5f */

typedef struct {
	Uint8 opcode;
	Uint8 before;
	Uint8 after;
} script_cut_replace_t;

/* 0x60-0x6f */

typedef struct {
	Uint8 opcode;
	Uint8 unknown0;
	Uint8 id;
	Uint8 unknown1[3];
	Sint16 x,y,w,h;
	Sint16 next_x,next_y,next_z;
	Sint16 next_dir;
	Uint8 next_stage,next_room,next_camera;
	Uint8 unknown2;
	Uint8 door_type;
	Uint8 door_lock;
	Uint8 unknown3;
	Uint8 door_locked;
	Uint8 door_key;
	Uint8 unknown4;
} script_door_set_t;

/* 0x70-0x7f */
/* 0x80-0x89 */

/* All instructions */

typedef union {
	Uint8 opcode;

	/* 0x00-0x0f */
	script_exec_t	exec;
	script_if_t	i_if;
	script_else_t	i_else;
	script_sleepn_t sleepn;
	script_for_t		i_for;

	/* 0x10-0x1f */
	script_while_t		i_while;
	script_do_end_t		i_end_do;
	script_switch_t		i_switch;
	script_case_t		i_case;
	script_func_t func;

	/* 0x20-0x2f */
	script_line_begin_t	line_begin;
	script_line_main_t	line_main;

	/* 0x30-0x3f */
	script_ahead_room_set_t	ahead_room_set;
	script_floor_set_t	floor_set;

	/* 0x40-0x4f */
	script_var_set_t	var_set;
	script_fade_set_t	fade_set;
	script_work_set_t	work_set;
	script_set_flag_t	set_flag;

	/* 0x50-0x5f */
	script_cut_replace_t	cut_replace;

	/* 0x60-0x6f */
	script_door_set_t	door_set;

	/* 0x70-0x7f */
	/* 0x80-0x89 */

} script_inst_t;

#endif /* ROOM_RDT3_SCRIPT_COMMON_H */
