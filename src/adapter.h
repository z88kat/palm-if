
#ifndef h_adapter_h
#define h_adapter_h

#include "sections.h"

/* Since Magnetic assumes things like the presence of the Standard C library,
 * we need to wrap up Palm OS functions to look like them, or take them out
 * altogether. Rather than wreck all the Magnetic code, we'll opt for the
 * wrapper method.
 *
 * GOAL: Ease and speed of port; its a little hackish here, but I needed to
 * get it done fast or it would never get done (very limited time). And wanted
 * to play the games.. I'd been meaning to do this port since 1997 :)
 *
 */

/* externs
 */
extern UInt32 g_current_pic;
extern UInt8 g_current_pic_mode;
extern char g_gamename [ 20 ];
extern char g_gfxname [ 20 ];
extern char g_savedir [ 20 ];
extern char g_last_loaded_file [ 40 ];
extern UInt8 g_waiting_for_key;
extern MemHandle h_crlf, h_bat1, h_bat2, h_bat3, h_bat4, h_thinking;
extern BitmapPtr b_crlf, b_bat1, b_bat2, b_bat3, b_bat4, b_thinking;

/* application/system features
 */
extern UInt32 g_features;

#define F_OS5HIGHRES 1
#define F_COLOUR     2
#define F_TINYFONT   8  /* only set if OS5highres is also true! */
#define F_MEDFONT   16  /* add to TINYFONT for medium font */
#define F_MAPPER    32  /* add support for mapper; eats RAM! */

extern UInt16 g_runflags;

#define R_SWALLOW    1 /* eat output keybuf empty */
#define R_FULLREDRAW 2 /* zcode always draw full virtual screen */
#define R_HOLDDRAW   4 /* On OS5 will only show end results; not intermed. */

/* system persistent "feature memory"
 */
#define FTR_THEME    1 /* feature 1 is theme; a UInt8 */

/* types
 */
typedef UInt32 size_t;
typedef UInt32 time_t;
#define TRUE 1
#define FALSE 0 
#define FILENAME_MAX 123

/* signals
 */
#define SIGINT 42
#define SIGQUIT 43
#define SIGTSTP 44
#define SIG_IGN 0

/* memory/misc functions
 */
#define malloc(x) MemPtrNewL(x)
void *realloc ( void *ptr, UInt32 size ) SEC_KRONOS;
#define free(x) MemPtrFreeL(x)
#define memcpy(t,f,s) MemMove(t,f,s)
#define time(x) TimGetSeconds()
#define strcasecmp StrCaselessCompare
#define strncasecmp StrNCaselessCompare
#define strcmp StrCompare
#define strchr(s,c) StrChr ( s, c )
#define isspace TxtCharIsSpace
#define strncpy StrNCopy
#define strncmp StrNCompare
#define toupper(x) (x>='a' && x<='z' ? (x - ('a'-'A')) : (x))
#define tolower(x) (x>='A' && x<='Z' ? (x + ('a'-'A')) : (x))
#define strcpy(t,s) StrCopy ( t, s )
#define strerror(e) "Error"
#define sprintf StrPrintF
#define memset(s,c,n) MemSet(s,n,c)
#define islower(c) TxtCharIsLower(c)
#define isupper(c) TxtCharIsUpper(c)
#define strlen(c) StrLen(c)
#define atoi(a) StrAToI(a)
#define atof(a) 1
#define strcat(s,a) StrCat(s,a)
#define memmove(d,s,z) MemMove(d,s,z)

/* directory listing
 */
typedef struct _dirent_t {
  char filename [ 32 ];
  struct _dirent_t *next;
} dirent_t;

extern dirent_t *dirlist;

void dirlist_build ( void ) SEC_KRONOS;
char *dirlist_lookup ( UInt16 entry ) SEC_KRONOS;

/*
 * Palm OS utility functions
 */

/* memory alloc
 */
#define memNewChunkFlagNonMovable    0x0200
#define memNewChunkFlagAllowLarge    0x1000  // this is not in the sdk *g*
SysAppInfoPtr 
SysGetAppInfo(SysAppInfoPtr *uiAppPP, SysAppInfoPtr *actionCodeAppPP)
  SYS_TRAP(sysTrapSysGetAppInfo); 

void *MemPtrNewL ( UInt32 size ) SEC_KRONOS;
void MemPtrFreeL ( void *ptr ) SEC_KRONOS;
UInt32 query_heap_free ( void ) SEC_KRONOS;

/* VFS filesystem utility
 */
UInt16 palm_vfs_count ( void ) SEC_KRONOS;
UInt16 palm_vfs_iter ( void ) SEC_KRONOS;
UInt16 palm_save_count ( UInt16 entry, char *r_filename ) SEC_KRONOS;
UInt16 palm_game_count ( UInt16 entry, char *r_filename ) SEC_KRONOS;
UInt8 palm_file_exists ( char *name ) SEC_KRONOS;

/* exit() for the interpreters: abandons the running game and returns to the
 * point in MainForm.c that started it (see adapter.c).
 */
extern ErrJumpBuf g_exit_jump;
extern UInt8 g_exit_armed;
void exit ( int status ) SEC_KRONOS;

/* event loop for custom modal
 */
void palm_event_loop ( void ) SEC_KRONOS;
Boolean AppHandleEvent ( EventPtr eventP );

/* object referential
 */
void *GetObjectPtr ( UInt16 objectID );
FieldPtr GetFocusObjectPtr ( void ) SEC_KRONOS;

/* gadgets
 */
int update_field ( FormPtr f, int obid, char *newtext ) SEC_KRONOS;
void cache_bitmaps ( void ) SEC_KRONOS;
void dump_bitmaps ( void ) SEC_KRONOS;
void draw_battery ( void ) SEC_KRONOS;
void draw_progress_bar ( UInt8 perc ) SEC_KRONOS;
void draw_thinking ( void ) SEC_KRONOS;
void erase_thinking ( void ) SEC_KRONOS;
void draw_controls ( void ) SEC_KRONOS;
void erase_controls ( void ) SEC_KRONOS;
void display_scroll_up ( void ) SEC_KRONOS;
void display_scroll_down ( void ) SEC_KRONOS;
void ms_statuschar_string ( char *s ) SEC_KRONOS;

/* keyboard interface
 */
void keyboard_push ( char *text ) SEC_KRONOS;
void keyboard_clear ( void ) SEC_KRONOS;

/* debugging and special alerts
 */

#define DEBUGC(x) FrmCustomAlert ( MessageAlert, x, NULL, NULL );

#define DEBUGU8(x) {                                   \
  char buffer [ 200 ];                                 \
  StrPrintF ( buffer, "%u", x );                       \
  FrmCustomAlert ( MessageAlert, buffer, NULL, NULL ); \
}

#define DEBUGU32(x) {                                  \
  char buffer [ 200 ];                                 \
  StrPrintF ( buffer, "%lu", x );                      \
  FrmCustomAlert ( MessageAlert, buffer, NULL, NULL ); \
}

#define DEBUGS(s) {                                    \
  FrmCustomAlert ( MessageAlert, s, NULL, NULL );      \
}

#endif
