
#ifndef h_screen_h
#define h_screen_h

#include "sections.h"

/* Screen is to be composed of a linked list of char arrays; each array will
 * be maximum width; we know a font is at least say 4 pixels per char average,
 * so thats 160/4 or 40 chars wide; at worst 320/2 or 160 chars wide.. or with
 * UX50 and a 1 pixel font (ha!) we'd get say 480 chars wide but more likely
 * a max of 250 chars will be fine :)
 *
 * As chars are inserted, we add them to the entry at the end of the list; if
 * we exceed the line, we wordwrap ourselves.
 *
 * The last line has a pointer to it for speed, as does the last shown line.
 *
 * When we "flush" (update the physical screen), we need to show everything
 * from the last shown line to the last line; if its too many lines, we'll need
 * to [MORE] until they're all shown.
 *
 * Newlines (\n) are included in the text of each line, since the current line
 * may not have one.
 *
 */

//#define MAXWIDTH 250
#define MAXWIDTH 125
#define WRAPPOINT 245

//#define ATTR

#define K_NO_STYLE   0
#define K_BOLD_STYLE 1

typedef struct _line_t {
  char text [ MAXWIDTH ];               // accrued text
#ifdef ATTR
  UInt8 attr [ MAXWIDTH ];              // should use 4bit per attr instead?
#endif
  UInt16 pixwidth;                      // pixel width of accrued text
  struct _line_t *next;                 // next row
} line_t;

extern line_t *screen_head;             // first in list (history..)
extern line_t *screen_last_shown;       // last shown line
extern line_t *screen_tail;             // last in list (appending to now)
extern line_t *screen_last_draw;        // top of last drawn screen
extern UInt16 g_screen_length;          // number of nodes in linked list

extern UInt16 g_row_height;             // effective pixels/per row in 160scale
extern UInt16 g_scroll_up;              // how scrolled we are

extern char status_left [ 250 ];        // status line
extern char status_right [ 250 ];

extern UInt32 g_current_theme;          // current theme number

void screen_format ( UInt16 maxwidth, UInt16 wrappoint ) SEC_KRONOS; // set screen params
void screen_push ( char c ) SEC_KRONOS;            // handle wordwrap, line creation, etc.
void screen_push_string ( char *s ) SEC_KRONOS;    // shortcut
void screen_trim_history ( UInt16 max_keep, UInt16 under ) SEC_KRONOS; // kill history
void screen_push_style ( UInt8 style ) SEC_KRONOS; // next char only

void screen_draw ( line_t *begin, line_t *end ) SEC_KRONOS;
void screen_draw_row ( line_t *l, UInt16 x, UInt16 y ) SEC_KRONOS;

UInt16 screen_query_row ( UInt16 ypixel ) SEC_KRONOS;   // how many rows up from bottom?
line_t *screen_query_at ( UInt16 lines_up ) SEC_KRONOS; // find line x lines from bottom
UInt8 screen_query_word_at ( UInt16 lines_up, UInt16 x, char *r_word ) SEC_KRONOS;

void screen_draw_pic ( UInt32 x, UInt32 y, UInt32 pic, UInt8 scale ) SEC_KRONOS;
void screen_draw_pic_fit ( Int16 x, Int16 y, Int16 maxw, Int16 maxh,
			   Boolean centre ) SEC_KRONOS;

void screen_reset_scroll ( void ) SEC_KRONOS;

void screen_prompt_more ( void ) SEC_KRONOS;

typedef enum {
  ct_last = 1,  /* use last set theme; default is DOS */
  ct_dos,
  ct_atari_st,
  ct_amiga,
  ct_black_on_white,
  ct_vic20
} colour_theme_t;

void screen_theme_setup ( colour_theme_t t ) SEC_KRONOS;
void screen_theme_undo ( void ) SEC_KRONOS;

#endif
