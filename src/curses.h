#ifndef h_curses_h
#define h_curses_h

#include "sections.h"

/* curses / POSIX shims for ScottFree; see curses.c */

#define WINDOW int

void perror ( const char *s ) SEC_SCOTT;
void endwin ( void ) SEC_SCOTT;
int signal ( int sig, void *handler ) SEC_SCOTT;
int initscr ( void ) SEC_SCOTT;
WINDOW *newwin ( int lines, int cols, int y, int x ) SEC_SCOTT;
void scrollok ( WINDOW *w, int on ) SEC_SCOTT;
void leaveok ( WINDOW *w, int on ) SEC_SCOTT;
void idlok ( WINDOW *w, int on ) SEC_SCOTT;
void cbreak ( void ) SEC_SCOTT;
void noecho ( void ) SEC_SCOTT;
void wmove ( WINDOW *w, int y, int x ) SEC_SCOTT;
void wclrtoeol ( WINDOW *w ) SEC_SCOTT;
void scroll ( WINDOW *w ) SEC_SCOTT;
int getuid ( void ) SEC_SCOTT;
void srand ( int seed ) SEC_SCOTT;
int rand ( void ) SEC_SCOTT;
void werase ( WINDOW *w ) SEC_SCOTT;
void wrefresh ( WINDOW *w ) SEC_SCOTT;
void sleep ( int secs ) SEC_SCOTT;
int wgetch ( WINDOW *w ) SEC_SCOTT;
#define getyx(w,y,x) ((void) 0)
void mvwaddch ( WINDOW *w, int y, int x, char c ) SEC_SCOTT;
void waddch ( WINDOW *w, char c ) SEC_SCOTT;
void wprintw ( WINDOW *w, const char *text, ... ) SEC_SCOTT;
int sscanf ( const char *buf, const char *format, char *verb, char *noun ) SEC_SCOTT;

int printf ( const char *fmt, ... ) SEC_KRONOS;
#define puts(s) printf ( "%s\n", (s) )
#define ungetc(c,f) ((void) 0)
#define putchar(c) screen_push(c)
#define getchar() ms_getchar(0)

#endif
