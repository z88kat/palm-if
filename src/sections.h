#ifndef h_sections_h
#define h_sections_h

/*
 * Code section layout
 * -------------------
 * Kronos is ~80K of 68K code, but a Palm OS code resource is limited to 64K
 * and a 68000 PC-relative call can only reach +/-32K. The application is
 * therefore split into several code resources (prc-tools "multiple code
 * sections", see kronos.def):
 *
 *   .text     (code #1) the startup code (Starter.c), libgcc and the
 *                       Frotz (Z-machine) core
 *   kronos    (code #2) the Kronos user interface and C library shims
 *   magnetic  (code #3) the Magnetic Scrolls interpreter (emu.c)
 *   scott     (code #4) ScottFree and its curses shims
 *   frotzio   (code #5) Frotz screen, save/restore, error and hot-key code
 *                       and the "dumb" front end
 *   level9    (code #6) the Level 9 interpreter and its front end (l9palm.c)
 *   level9b   (code #7) the Level 9 bitmap picture decoders (bitmap.c)
 *
 * prc-tools rule: every function placed in a named section must be declared
 * with its section attribute wherever it is called (or its address taken).
 * A call made without the attribute in scope is compiled as a call into
 * .text and will crash at run time. Calls *into* .text need no annotation.
 *
 * prc-tools also cannot relocate pointers in static data that point into a
 * code section other than .text (they are silently dropped), so a table of
 * function pointers must either point only into .text or be filled in at
 * run time (see init_opcodes in frotz/process.c).
 *
 * tools/check-sections.py verifies the linked program for such mistakes and
 * runs as part of every build.
 */

#define SEC_KRONOS   __attribute__ ((section ("kronos")))
#define SEC_MAGNETIC __attribute__ ((section ("magnetic")))
#define SEC_SCOTT    __attribute__ ((section ("scott")))
#define SEC_FROTZIO  __attribute__ ((section ("frotzio")))
#define SEC_LEVEL9   __attribute__ ((section ("level9")))
#define SEC_LEVEL9B  __attribute__ ((section ("level9b")))

#endif
