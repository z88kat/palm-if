/* process.c - Interpreter loop and program control
 *	Copyright (c) 1995-1997 Stefan Jokisch
 *
 * This file is part of Frotz.
 *
 * Frotz is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Frotz is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "frotz.h"

#ifdef DJGPP
#include "djfrotz.h"
#endif


zword zargs[8];
int zargc;

static int finished = 0;

static void __extended__ (void);
static void __illegal__ (void);

/*
 * Opcode dispatch tables.
 *
 * Kronos: these are filled in at run time by init_opcodes rather than with
 * static initialisers. The opcode functions live in several code sections
 * and prc-tools cannot relocate static pointers into code sections other
 * than .text (see ../sections.h).
 */

void (*op0_opcodes[0x10]) (void);
void (*op1_opcodes[0x10]) (void);
void (*var_opcodes[0x40]) (void);
void (*ext_opcodes[0x1d]) (void);

/*
 * init_opcodes
 *
 * Fill in the opcode dispatch tables. Must run before init_memory, which
 * patches some entries according to the story file version.
 *
 */

void init_opcodes (void)
{
    void (**t) (void);

    t = op0_opcodes;
    t[0x00] = z_rtrue;
    t[0x01] = z_rfalse;
    t[0x02] = z_print;
    t[0x03] = z_print_ret;
    t[0x04] = z_nop;
    t[0x05] = z_save;
    t[0x06] = z_restore;
    t[0x07] = z_restart;
    t[0x08] = z_ret_popped;
    t[0x09] = z_catch;
    t[0x0a] = z_quit;
    t[0x0b] = z_new_line;
    t[0x0c] = z_show_status;
    t[0x0d] = z_verify;
    t[0x0e] = __extended__;
    t[0x0f] = z_piracy;

    t = op1_opcodes;
    t[0x00] = z_jz;
    t[0x01] = z_get_sibling;
    t[0x02] = z_get_child;
    t[0x03] = z_get_parent;
    t[0x04] = z_get_prop_len;
    t[0x05] = z_inc;
    t[0x06] = z_dec;
    t[0x07] = z_print_addr;
    t[0x08] = z_call_s;
    t[0x09] = z_remove_obj;
    t[0x0a] = z_print_obj;
    t[0x0b] = z_ret;
    t[0x0c] = z_jump;
    t[0x0d] = z_print_paddr;
    t[0x0e] = z_load;
    t[0x0f] = z_call_n;

    t = var_opcodes;
    t[0x00] = __illegal__;
    t[0x01] = z_je;
    t[0x02] = z_jl;
    t[0x03] = z_jg;
    t[0x04] = z_dec_chk;
    t[0x05] = z_inc_chk;
    t[0x06] = z_jin;
    t[0x07] = z_test;
    t[0x08] = z_or;
    t[0x09] = z_and;
    t[0x0a] = z_test_attr;
    t[0x0b] = z_set_attr;
    t[0x0c] = z_clear_attr;
    t[0x0d] = z_store;
    t[0x0e] = z_insert_obj;
    t[0x0f] = z_loadw;
    t[0x10] = z_loadb;
    t[0x11] = z_get_prop;
    t[0x12] = z_get_prop_addr;
    t[0x13] = z_get_next_prop;
    t[0x14] = z_add;
    t[0x15] = z_sub;
    t[0x16] = z_mul;
    t[0x17] = z_div;
    t[0x18] = z_mod;
    t[0x19] = z_call_s;
    t[0x1a] = z_call_n;
    t[0x1b] = z_set_colour;
    t[0x1c] = z_throw;
    t[0x1d] = __illegal__;
    t[0x1e] = __illegal__;
    t[0x1f] = __illegal__;
    t[0x20] = z_call_s;
    t[0x21] = z_storew;
    t[0x22] = z_storeb;
    t[0x23] = z_put_prop;
    t[0x24] = z_read;
    t[0x25] = z_print_char;
    t[0x26] = z_print_num;
    t[0x27] = z_random;
    t[0x28] = z_push;
    t[0x29] = z_pull;
    t[0x2a] = z_split_window;
    t[0x2b] = z_set_window;
    t[0x2c] = z_call_s;
    t[0x2d] = z_erase_window;
    t[0x2e] = z_erase_line;
    t[0x2f] = z_set_cursor;
    t[0x30] = z_get_cursor;
    t[0x31] = z_set_text_style;
    t[0x32] = z_buffer_mode;
    t[0x33] = z_output_stream;
    t[0x34] = z_input_stream;
    t[0x35] = z_sound_effect;
    t[0x36] = z_read_char;
    t[0x37] = z_scan_table;
    t[0x38] = z_not;
    t[0x39] = z_call_n;
    t[0x3a] = z_call_n;
    t[0x3b] = z_tokenise;
    t[0x3c] = z_encode_text;
    t[0x3d] = z_copy_table;
    t[0x3e] = z_print_table;
    t[0x3f] = z_check_arg_count;

    t = ext_opcodes;
    t[0x00] = z_save;
    t[0x01] = z_restore;
    t[0x02] = z_log_shift;
    t[0x03] = z_art_shift;
    t[0x04] = z_set_font;
    t[0x05] = z_draw_picture;
    t[0x06] = z_picture_data;
    t[0x07] = z_erase_picture;
    t[0x08] = z_set_margins;
    t[0x09] = z_save_undo;
    t[0x0a] = z_restore_undo;
    t[0x0b] = z_print_unicode;
    t[0x0c] = z_check_unicode;
    t[0x0d] = __illegal__;
    t[0x0e] = __illegal__;
    t[0x0f] = __illegal__;
    t[0x10] = z_move_window;
    t[0x11] = z_window_size;
    t[0x12] = z_window_style;
    t[0x13] = z_get_wind_prop;
    t[0x14] = z_scroll_window;
    t[0x15] = z_pop_stack;
    t[0x16] = z_read_mouse;
    t[0x17] = z_mouse_window;
    t[0x18] = z_push_stack;
    t[0x19] = z_put_wind_prop;
    t[0x1a] = z_print_form;
    t[0x1b] = z_make_menu;
    t[0x1c] = z_picture_table;

} /* init_opcodes */


/*
 * init_process
 *
 * Initialize process variables.
 *
 */

void init_process (void)
{
    finished = 0;
} /* init_process */


/*
 * load_operand
 *
 * Load an operand, either a variable or a constant.
 *
 */

static void load_operand (zbyte type)
{
    zword value;

    if (type & 2) { 			/* variable */

	zbyte variable;

	CODE_BYTE (variable)

	if (variable == 0)
	    value = *sp++;
	else if (variable < 16)
	    value = *(fp - variable);
	else {
	    zword addr = h_globals + 2 * (variable - 16);
	    LOW_WORD (addr, value)
	}

    } else if (type & 1) { 		/* small constant */

	zbyte bvalue;

	CODE_BYTE (bvalue)
	value = bvalue;

    } else CODE_WORD (value) 		/* large constant */

    zargs[zargc++] = value;

}/* load_operand */

/*
 * load_all_operands
 *
 * Given the operand specifier byte, load all (up to four) operands
 * for a VAR or EXT opcode.
 *
 */

static void load_all_operands (zbyte specifier)
{
    int i;

    for (i = 6; i >= 0; i -= 2) {

	zbyte type = (specifier >> i) & 0x03;

	if (type == 3)
	    break;

	load_operand (type);

    }

}/* load_all_operands */

/*
 * interpret
 *
 * Z-code interpreter main loop
 *
 */

void interpret (void)
{

    do {

	zbyte opcode;

	CODE_BYTE (opcode)

	zargc = 0;

	if (opcode < 0x80) {			/* 2OP opcodes */

	    load_operand ((zbyte) (opcode & 0x40) ? 2 : 1);
	    load_operand ((zbyte) (opcode & 0x20) ? 2 : 1);

	    var_opcodes[opcode & 0x1f] ();

	} else if (opcode < 0xb0) {		/* 1OP opcodes */

	    load_operand ((zbyte) (opcode >> 4));

	    op1_opcodes[opcode & 0x0f] ();

	} else if (opcode < 0xc0) {		/* 0OP opcodes */

	    op0_opcodes[opcode - 0xb0] ();

	} else {				/* VAR opcodes */

	    zbyte specifier1;
	    zbyte specifier2;

	    if (opcode == 0xec || opcode == 0xfa) {	/* opcodes 0xec */
		CODE_BYTE (specifier1)                  /* and 0xfa are */
		CODE_BYTE (specifier2)                  /* call opcodes */
		load_all_operands (specifier1);		/* with up to 8 */
		load_all_operands (specifier2);         /* arguments    */
	    } else {
		CODE_BYTE (specifier1)
		load_all_operands (specifier1);
	    }

	    var_opcodes[opcode - 0xc0] ();

	}

#if defined(DJGPP) && defined(SOUND_SUPPORT)
    if (end_of_sound_flag)
	end_of_sound ();
#endif

    } while (finished == 0);

    finished--;

}/* interpret */

/*
 * call
 *
 * Call a subroutine. Save PC and FP then load new PC and initialise
 * new stack frame. Note that the caller may legally provide less or
 * more arguments than the function actually has. The call type "ct"
 * can be 0 (z_call_s), 1 (z_call_n) or 2 (direct call).
 *
 */

void call (zword routine, int argc, zword *args, int ct)
{
    long pc;
    zword value;
    zbyte count;
    int i;

    if (sp - stack < 4)
	runtime_error (ERR_STK_OVF);

    GET_PC (pc)

    *--sp = (zword) (pc >> 9);
    *--sp = (zword) (pc & 0x1ff);
    *--sp = (zword) (fp - stack - 1);
    *--sp = (zword) (argc | (ct << (f_setup.save_quetzal ? 12 : 8)));

    fp = sp;
    frame_count++;

    /* Calculate byte address of routine */

    if (h_version <= V3)
	pc = (long) routine << 1;
    else if (h_version <= V5)
	pc = (long) routine << 2;
    else if (h_version <= V7)
	pc = ((long) routine << 2) + ((long) h_functions_offset << 3);
    else /* h_version == V8 */
	pc = (long) routine << 3;

    if (pc >= story_size)
	runtime_error (ERR_ILL_CALL_ADDR);

    SET_PC (pc)

    /* Initialise local variables */

    CODE_BYTE (count)

    if (count > 15)
	runtime_error (ERR_CALL_NON_RTN);
    if (sp - stack < count)
	runtime_error (ERR_STK_OVF);

    if (f_setup.save_quetzal)
	fp[0] |= (zword) count << 8;	/* Save local var count for Quetzal. */

    value = 0;

    for (i = 0; i < count; i++) {

	if (h_version <= V4)		/* V1 to V4 games provide default */
	    CODE_WORD (value)		/* values for all local variables */

	*--sp = (zword) ((argc-- > 0) ? args[i] : value);

    }

    /* Start main loop for direct calls */

    if (ct == 2)
	interpret ();

}/* call */

/*
 * ret
 *
 * Return from the current subroutine and restore the previous stack
 * frame. The result may be stored (0), thrown away (1) or pushed on
 * the stack (2). In the latter case a direct call has been finished
 * and we must exit the interpreter loop.
 *
 */

void ret (zword value)
{
    long pc;
    int ct;

    if (sp > fp)
	runtime_error (ERR_STK_UNDF);

    sp = fp;

    ct = *sp++ >> (f_setup.save_quetzal ? 12 : 8);
    frame_count--;
    fp = stack + 1 + *sp++;
    pc = *sp++;
    pc = ((long) *sp++ << 9) | pc;

    SET_PC (pc)

    /* Handle resulting value */

    if (ct == 0)
	store (value);
    if (ct == 2)
	*--sp = value;

    /* Stop main loop for direct calls */

    if (ct == 2)
	finished++;

}/* ret */

/*
 * branch
 *
 * Take a jump after an instruction based on the flag, either true or
 * false. The branch can be short or long; it is encoded in one or two
 * bytes respectively. When bit 7 of the first byte is set, the jump
 * takes place if the flag is true; otherwise it is taken if the flag
 * is false. When bit 6 of the first byte is set, the branch is short;
 * otherwise it is long. The offset occupies the bottom 6 bits of the
 * first byte plus all the bits in the second byte for long branches.
 * Uniquely, an offset of 0 means return false, and an offset of 1 is
 * return true.
 *
 */

void branch (bool flag)
{
    long pc;
    zword offset;
    zbyte specifier;
    zbyte off1;
    zbyte off2;

    CODE_BYTE (specifier)

    off1 = specifier & 0x3f;

    if (!flag)
	specifier ^= 0x80;

    if (!(specifier & 0x40)) {		/* it's a long branch */

	if (off1 & 0x20)		/* propagate sign bit */
	    off1 |= 0xc0;

	CODE_BYTE (off2)

	offset = (off1 << 8) | off2;

    } else offset = off1;		/* it's a short branch */

    if (specifier & 0x80) {

	if (offset > 1) {		/* normal branch */

	    GET_PC (pc)
	    pc += (short) offset - 2;
	    SET_PC (pc)

	} else ret (offset);		/* special case, return 0 or 1 */
    }

}/* branch */

/*
 * store
 *
 * Store an operand, either as a variable or pushed on the stack.
 *
 */

void store (zword value)
{
    zbyte variable;

    CODE_BYTE (variable)

    if (variable == 0)
	*--sp = value;
    else if (variable < 16)
	*(fp - variable) = value;
    else {
	zword addr = h_globals + 2 * (variable - 16);
	SET_WORD (addr, value)
    }

}/* store */

/*
 * direct_call
 *
 * Call the interpreter loop directly. This is necessary when
 *
 * - a sound effect has been finished
 * - a read instruction has timed out
 * - a newline countdown has hit zero
 *
 * The interpreter returns the result value on the stack.
 *
 */

int direct_call (zword addr)
{
    zword saved_zargs[8];
    int saved_zargc;
    int i;

    /* Calls to address 0 return false */

    if (addr == 0)
	return 0;

    /* Save operands and operand count */

    for (i = 0; i < 8; i++)
	saved_zargs[i] = zargs[i];

    saved_zargc = zargc;

    /* Call routine directly */

    call (addr, 0, 0, 2);

    /* Restore operands and operand count */

    for (i = 0; i < 8; i++)
	zargs[i] = saved_zargs[i];

    zargc = saved_zargc;

    /* Resulting value lies on top of the stack */

    return (short) *sp++;

}/* direct_call */

/*
 * __extended__
 *
 * Load and execute an extended opcode.
 *
 */

static void __extended__ (void)
{
    zbyte opcode;
    zbyte specifier;

    CODE_BYTE (opcode)
    CODE_BYTE (specifier)

    load_all_operands (specifier);

    if (opcode < 0x1d)			/* extended opcodes from 0x1d on */
	ext_opcodes[opcode] ();		/* are reserved for future spec' */

}/* __extended__ */

/*
 * __illegal__
 *
 * Exit game because an unknown opcode has been hit.
 *
 */

static void __illegal__ (void)
{

    runtime_error (ERR_ILL_OPCODE);

}/* __illegal__ */

/*
 * z_catch, store the current stack frame for later use with z_throw.
 *
 *	no zargs used
 *
 */

void z_catch (void)
{

    store (f_setup.save_quetzal ? frame_count : (zword) (fp - stack));

}/* z_catch */

/*
 * z_throw, go back to the given stack frame and return the given value.
 *
 *	zargs[0] = value to return
 *	zargs[1] = stack frame
 *
 */

void z_throw (void)
{

    if (f_setup.save_quetzal) {
	if (zargs[1] > frame_count)
	    runtime_error (ERR_BAD_FRAME);

	/* Unwind the stack a frame at a time. */
	for (; frame_count > zargs[1]; --frame_count)
	    fp = stack + 1 + fp[1];
    } else {
	if (zargs[1] > STACK_SIZE)
	    runtime_error (ERR_BAD_FRAME);

	fp = stack + zargs[1];
    }

    ret (zargs[0]);

}/* z_throw */

/*
 * z_call_n, call a subroutine and discard its result.
 *
 * 	zargs[0] = packed address of subroutine
 *	zargs[1] = first argument (optional)
 *	...
 *	zargs[7] = seventh argument (optional)
 *
 */

void z_call_n (void)
{

    if (zargs[0] != 0)
	call (zargs[0], zargc - 1, zargs + 1, 1);

}/* z_call_n */

/*
 * z_call_s, call a subroutine and store its result.
 *
 * 	zargs[0] = packed address of subroutine
 *	zargs[1] = first argument (optional)
 *	...
 *	zargs[7] = seventh argument (optional)
 *
 */

void z_call_s (void)
{

    if (zargs[0] != 0)
	call (zargs[0], zargc - 1, zargs + 1, 0);
    else
	store (0);

}/* z_call_s */

/*
 * z_check_arg_count, branch if subroutine was called with >= n arg's.
 *
 * 	zargs[0] = number of arguments
 *
 */

void z_check_arg_count (void)
{

    if (fp == stack + STACK_SIZE)
	branch (zargs[0] == 0);
    else
	branch (zargs[0] <= (*fp & 0xff));

}/* z_check_arg_count */

/*
 * z_jump, jump unconditionally to the given address.
 *
 *	zargs[0] = PC relative address
 *
 */

void z_jump (void)
{
    long pc;

    GET_PC (pc)

    pc += (short) zargs[0] - 2;

    if (pc >= story_size)
	runtime_error (ERR_ILL_JUMP_ADDR);

    SET_PC (pc)

}/* z_jump */

/*
 * z_nop, no operation.
 *
 *	no zargs used
 *
 */

void z_nop (void)
{

    /* Do nothing */

}/* z_nop */

/*
 * z_quit, stop game and exit interpreter.
 *
 *	no zargs used
 *
 */

void z_quit (void)
{

    finished = 9999;

}/* z_quit */

/*
 * z_ret, return from a subroutine with the given value.
 *
 *	zargs[0] = value to return
 *
 */

void z_ret (void)
{

    ret (zargs[0]);

}/* z_ret */

/*
 * z_ret_popped, return from a subroutine with a value popped off the stack.
 *
 *	no zargs used
 *
 */

void z_ret_popped (void)
{

    ret (*sp++);

}/* z_ret_popped */

/*
 * z_rfalse, return from a subroutine with false (0).
 *
 * 	no zargs used
 *
 */

void z_rfalse (void)
{

    ret (0);

}/* z_rfalse */

/*
 * z_rtrue, return from a subroutine with true (1).
 *
 * 	no zargs used
 *
 */

void z_rtrue (void)
{

    ret (1);

}/* z_rtrue */
