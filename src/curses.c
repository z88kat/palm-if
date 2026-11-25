/*
 * Minimal curses / POSIX shims so that ScottFree's curses front end builds
 * on Palm OS. Output goes to the Kronos display (display.c), input comes
 * from the Kronos keyboard buffer (adapter.c). Everything else is a no-op.
 */

#include <PalmOS.h>
#include <VfsMgr.h>

#include "defs.h"
#include "kronos.h"
#include "sections.h"
#include "display.h"

void perror ( const char *s ) { (void) s; }
/* Code section declarations (see sections.h) */
static const char *scan_word (const char *s, char *out, int max) SEC_SCOTT;

void endwin ( void ) {}
int signal ( int sig, void *handler ) { (void) sig; (void) handler; return 0; }
int initscr ( void ) { return 0; }
WINDOW *newwin ( int lines, int cols, int y, int x ) {
  (void) lines; (void) cols; (void) y; (void) x;
  return NULL;
}
void scrollok ( WINDOW *w, int on ) { (void) w; (void) on; }
void leaveok ( WINDOW *w, int on ) { (void) w; (void) on; }
void idlok ( WINDOW *w, int on ) { (void) w; (void) on; }
void cbreak ( void ) {}
void noecho ( void ) {}
void wmove ( WINDOW *w, int y, int x ) { (void) w; (void) y; (void) x; }
void wclrtoeol ( WINDOW *w ) { (void) w; screen_push ( '\n' ); }
void scroll ( WINDOW *w ) { (void) w; }
int getuid ( void ) { return 1; }
void srand ( int seed ) { SysRandom ( seed ); }
int rand ( void ) { return SysRandom ( 0 ); }
void werase ( WINDOW *w ) { (void) w; }
void wrefresh ( WINDOW *w ) { (void) w; ms_flush(); }
void sleep ( int secs ) { (void) secs; }
int wgetch ( WINDOW *w ) { (void) w; return ms_getchar ( 0 ); }
void mvwaddch ( WINDOW *w, int y, int x, char c ) {
  (void) w; (void) y; (void) x; (void) c;
}
void waddch ( WINDOW *w, char c ) { (void) w; (void) c; }

/* Output only; ScottFree passes pre-formatted text, never format arguments. */
void wprintw ( WINDOW *w, const char *text, ... ) {
  (void) w;
  screen_push_string ( (char *) text );
}

/* Copy one whitespace-delimited word of at most max-1 characters. */
static const char *scan_word ( const char *s, char *out, int max ) {
  int n = 0;

  while ( *s == ' ' || *s == '\t' ) {
    s++;
  }

  while ( *s && *s != ' ' && *s != '\t' ) {
    if ( n < max - 1 ) {
      out [ n++ ] = *s;
    }
    s++;
  }

  out [ n ] = '\0';
  return s;
}

/* The only sscanf() in ScottFree is sscanf(buf, "%9s %9s", verb, noun). */
int sscanf ( const char *buf, const char *format, char *verb, char *noun ) {
  (void) format;

  buf = scan_word ( buf, verb, 10 );
  if ( verb [ 0 ] == '\0' ) {
    return 0;
  }

  scan_word ( buf, noun, 10 );
  return noun [ 0 ] ? 2 : 1;
}
