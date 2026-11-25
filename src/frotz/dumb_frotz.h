/* dumb-frotz.h
 * $Id: dumb-frotz.h,v 1.1.1.1 2002/03/26 22:38:34 feedle Exp $
 * Frotz os functions for a standard C library and a dumb terminal.
 * Now you can finally play Zork Zero on your Teletype.
 *
 * Copyright 1997, 1998 Alembic Petrofsky <alembic@petrofsky.berkeley.ca.us>.
 * Any use permitted provided this notice stays intact.
 */
#include "frotz.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <errno.h>
//#include <string.h>
//#include <ctype.h>
//#include <time.h>

/* from ../common/setup.h */
extern f_setup_t f_setup;

/* From input.c.  */
bool is_terminator (zchar);

/* dumb-input.c */
bool dumb_handle_setting(const char *setting, bool show_cursor, bool startup) SEC_FROTZIO;
void dumb_init_input(void) SEC_FROTZIO;

/* dumb-output.c */
void dumb_init_output(void) SEC_FROTZIO;
bool dumb_output_handle_setting(const char *setting, bool show_cursor,
				bool startup) SEC_FROTZIO;
void dumb_show_screen(bool show_cursor) SEC_FROTZIO;
void dumb_show_prompt(bool show_cursor, char line_type) SEC_FROTZIO;
void dumb_dump_screen(void) SEC_FROTZIO;
void dumb_display_user_input(char *) SEC_FROTZIO;
void dumb_discard_old_input(int num_chars) SEC_FROTZIO;
void dumb_elide_more_prompt(void) SEC_FROTZIO;
void dumb_set_picture_cell(int row, int col, char c) SEC_FROTZIO;

/* dumb-pic.c */
void dumb_init_pictures(char *graphics_filename) SEC_FROTZIO;
