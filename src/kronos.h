
#ifndef h_kronos_h
#define h_kronos_h

#include "sections.h"

/* DEDICATION
 * This code is for Swampie, may he rest in pieace :( When it thunders outside,
 * maybe its old Stevie on the big Harley in the sky.
 */

/*
 * KRONOS
 * A IF-engine encapsulation and interface for Palm OS. Or something.
 * Jeff Mitchell, Codejedi, 2003.
 *
 * Kronos is a "adapter" for Palm OS; it includes a user interface and
 * all the Palm OS centric stuff needed to handle an IF game. It also includes
 * wrappers for Standard C libs and other things not normally available to
 * Palm OS, to make porting IF engines seamless and painless.
 *
 * The goals of the project:
 * 1) To port many IF engines to Palm OS under one consistent hood. A good
 *    user experience.
 * 2) To make the porting process easy; we do not wish to break away from the
 *    source projects.. Kronos will use Magnetic 2.2 or later; it should be
 *    trivial to upgrade to a new Magnetic should one ever come along. As such
 *    we must minimize code changes to the foreign codebases.
 * 3) Reusability; since Palm OS lacks a Standard C library (in name; most
 *    functions are available under different names with different
 *    requirements), then it is hoped that Kronos's file I/O and perhaps
 *    other routines will be of use for things like a Nethack project or
 *    other cool simple POSIX apps.
 * 4) Most importantly.. I want to beat The Pawn, since it drove me mad
 *    a long time ago. I've been using Magnetic on and off (see my other
 *    ports of it) for a long time.. but only my PDA is at my side. Well,
 *    my PDA and my wife but she makes a bad interpreter :)
 */

#define VERSION "1.5"
#define CREATORID 'Magn'

/* LICENSING
 * Kronos itself (formerly Magnetic/Palm) is GPL.
 * Do not remove the source history or the GPL license.. that would be a
 * violation of the GPL. But do use enjoy Kronos and if it helps you
 * in your own projects be shedding light on the horrors of Palm OS
 * high res or filesystem code, then I am honoured to be at your service.
 *
 * WARRATY
 * No warranty is implied. Enjoy at your own risk.
 *
 * CREDITS
 * The application includes credits; Kronos would not be useful were it not
 * for the many authors of Frotz, ScottFree, Magnetic, and indeed for
 * Infocom, Magnetic Scrolls, Adventure International, Graham Nelson and
 * many many others.
 * 
 * My wife is my soul.
 */

/* USAGE:
 *
 * See the README.txt included in the source directory.
 *
 * 1) Foreign modules generally need only include a few header files to get
 * going; you'll need to comment or #if out existing head includes since
 * Palm OS lacks things like stdio.h; but #include kronos.h and adapter.h
 * and they shoudl be buildable, with luck.
 *
 * 2) You likely need to twiddle the display code; ie: They often use
 * printf() or fputc() or the like; fine. But for Kronos to actually render
 * anything, use ms_flush() to update the display.
 *
 */

/* COMPILE NOTES
 * Kronos was originally built with CodeWarrior 9 and Constructor. It is now
 * built with prc-tools (m68k-palmos-gcc 2.95) and PilRC; see README.md and
 * the Makefile. The code is split over several code resources, which
 * imposes some rules on declarations: see sections.h.
 *
 * The ARMlet (src/arm/picview.c) is built with arm-palmos-gcc and linked
 * with src/arm/armletldscript.
 */

/*
 * conveniance defines
 */

#define VFSROOT        "/Palm/Programs/"
#define VFSPATH        "/Palm/Programs/Kronos/"

#define NOTEFILE       "notes.txt" /* notes for game, in save dir */
#define MAPFILE        "map.txt"   /* for map storage */
#define AUTOSAVEFILE   "autosave.sav"

#define THUMB_X        115
#define THUMB_Y        131

/*
 * conveniance includes
 */

#include "stdfile.h"
#include "adapter.h"
#include "curses.h"

/* emu kickers
 */
int emu_main ( int argc, char **argv ) SEC_KRONOS;
void keyboard_push ( char *text ) SEC_KRONOS;
UInt8 is_magnetic ( char *filename ) SEC_KRONOS;
UInt8 is_scottfree ( char *filename ) SEC_KRONOS;
UInt8 is_zcode ( char *filename ) SEC_KRONOS;
UInt8 is_level9 ( char *filename ) SEC_KRONOS;

/* Level 9 front end (src/level9/l9palm.c) */
int l9_main ( void ) SEC_LEVEL9;
void l9_stop ( void ) SEC_LEVEL9;
void l9_cleanup ( void ) SEC_LEVEL9;
extern UInt8 g_l9_autoload;
Boolean l9_has_picture ( void ) SEC_LEVEL9;
void l9_draw_thumbnail ( void ) SEC_LEVEL9;
UInt8 *l9_picture ( UInt16 *w, UInt16 *h, RGBColorType *pal,
		    UInt16 *ncolours ) SEC_LEVEL9;
void scott_main ( int argc, char *argv[] ) SEC_SCOTT;
void z_quit ( void );

/* emu assist
 */
UInt8 force_auto_save ( void ) SEC_KRONOS;
UInt8 force_auto_load ( void ) SEC_KRONOS;
void kronos_reset ( void ) SEC_KRONOS;

/* word lists
 */
extern char *relative_word_list[];
extern char *standalone_word_list[];
extern char *navigation_word_list[];

/* assumes List box is in the right location and sized; this function will
 * push the correct words into the display field or keyboard buffer as
 * appropriate
 */
void words_translate ( char *tapword, char *from, char *r_to ) SEC_KRONOS;
void words_popword ( char *words[], ListType *list, char *tapword ) SEC_KRONOS;
void WordsDrawList ( Int16 itemNum, RectangleType *bounds, Char **itemsText ) SEC_KRONOS;

#endif
