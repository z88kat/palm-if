
#include <PalmOS.h>
#include <VfsMgr.h>

#include "KronosRsc.h"

#include "defs.h"
#include "kronos.h"
#include "sections.h"
#include "adapter.h"
#include "display.h"
#include "mapper.h"

//#define WIDTH 78

/* globals
 */
type8 buffer[80], xpos = 0, bufpos = 0,
  log_on = 0, ms_gfx_enabled, filename[256];
FILE *log1 = 0, *log2 = 0;

static UInt8 g_loop;
static char keybuf [ 512 ] = "\0\0";
UInt32 g_current_pic = 0;
UInt8 g_current_pic_mode = 0;
char g_gamename [ 20 ];
char g_gfxname [ 20 ];
char g_savedir [ 20 ]; // gamename minus extension plus .sav - "thepawn.sav"
char g_last_loaded_file [ 40 ] = "default.sav";
UInt8 g_waiting_for_key = 0;
UInt32 g_features = 0;
UInt16 g_runflags = 0;

/* bitmaps and handles
 */
MemHandle h_crlf, h_bat1, h_bat2, h_bat3, h_bat4, h_thinking;
BitmapPtr b_crlf, b_bat1, b_bat2, b_bat3, b_bat4, b_thinking;

void cache_bitmaps ( void ) {
  h_crlf = DmGetResource ( 'Tbmp', ReturnCharBitmapFamily );
  b_crlf = MemHandleLock ( h_crlf );

  h_bat1 = DmGetResource ( 'Tbmp', Bat1BitmapFamily );
  b_bat1 = MemHandleLock ( h_bat1 );

  h_bat2 = DmGetResource ( 'Tbmp', Bat2BitmapFamily );
  b_bat2 = MemHandleLock ( h_bat2 );

  h_bat3 = DmGetResource ( 'Tbmp', Bat3BitmapFamily );
  b_bat3 = MemHandleLock ( h_bat3 );

  h_bat4 = DmGetResource ( 'Tbmp', Bat4BitmapFamily );
  b_bat4 = MemHandleLock ( h_bat4 );

  h_thinking = DmGetResource ( 'Tbmp', ThinkingBitmapFamily );
  b_thinking = MemHandleLock ( h_thinking );

  return;
}

void dump_bitmaps ( void ) {
  MemHandleUnlock ( h_crlf );
  b_crlf = NULL;
  DmReleaseResource ( h_crlf );

  MemHandleUnlock ( h_bat1 );
  b_bat1 = NULL;
  DmReleaseResource ( h_bat1 );

  MemHandleUnlock ( h_bat2 );
  b_bat2 = NULL;
  DmReleaseResource ( h_bat2 );

  MemHandleUnlock ( h_bat3 );
  b_bat3 = NULL;
  DmReleaseResource ( h_bat3 );

  MemHandleUnlock ( h_bat4 );
  b_bat4 = NULL;
  DmReleaseResource ( h_bat4 );

  MemHandleUnlock ( h_thinking );
  b_thinking = NULL;
  DmReleaseResource ( h_thinking );

  return;
}

/* primitives
 */

UInt16 palm_vfs_count ( void ) {
  UInt16 volref = 0;
  UInt32 voliter = vfsIteratorStart;
  UInt16 count = 0;

  if ( VFSVolumeEnumerate ( &volref,  &voliter ) != errNone ) {
    return ( 0 ); // error
  }

  count = 1;

  while ( VFSVolumeEnumerate ( &volref,  &voliter ) == errNone ) {
    //if ( voliter != vfsIteratorStop ) {
    count++;
    //}
  }

  return ( count );
}

UInt16 palm_vfs_iter ( void ) {
  UInt16 volref = 0;
  UInt32 voliter = vfsIteratorStart;
  FileRef file;
  UInt16 firstvolref;

  // can we enumerate at all?
  if ( VFSVolumeEnumerate ( &volref,  &voliter ) != errNone ) {
    return ( 0 );
  }

  // store first succeeding volume
  firstvolref = volref;

  // can we open our directory? If so, we found our directory :)
  if ( VFSFileOpen ( volref, VFSPATH, vfsModeRead, &file ) == errNone ) {
    VFSFileClose ( file );
    return ( volref );
  }

  // look through other volumes, trying to find our directory
  while ( VFSVolumeEnumerate ( &volref,  &voliter ) == errNone ) {

    if ( VFSFileOpen ( volref, VFSPATH, vfsModeRead, &file ) == errNone ) {
      VFSFileClose ( file );
      return ( volref );
    }

  }

  //return ( voliter );
  return ( firstvolref );
}

// pass in zero to get a return of number of save files; pass in >0 to
// get details about that file
UInt16 palm_save_count ( UInt16 entry, char *r_filename ) {
  UInt16 volref = palm_vfs_iter();
  FileRef dir;
  FileInfoType file;
  UInt32 diter = vfsIteratorStart;
  char filename [ 256 ] = "";
  UInt16 counter = 0;
  char path [ 300 ];
  Err error;

  file.nameP = filename; // point to local buffer
  file.nameBufLen = 256; // sizeof ( fileName );

  StrPrintF ( path, "%s%s", VFSPATH, g_savedir );

  error = VFSFileOpen ( volref, path, vfsModeRead, &dir );

  if ( error != errNone ) {
    return ( 0 );
  }

  while ( diter != vfsIteratorStop ) {
    VFSDirEntryEnumerate ( dir, &diter, &file );
    if ( ( StrStr ( filename, ".sav" ) ) ||
	 ( StrStr ( filename, ".SAV" ) ) )
    {
      counter++;

      if ( entry == counter ) {
	StrCopy ( r_filename, filename );
	return ( 0 );
      }

    }
  } // while

  return ( counter );
}

dirent_t *dirlist = NULL;

void dirlist_build ( void ) {
  UInt16 count = palm_game_count ( 0, NULL );
  UInt16 iter;
  char filename [ 32 ];
  dirent_t *p;

  for ( iter = 0; iter < count; iter++ ) {

    palm_game_count ( iter + 1, filename );
    p = MemPtrNew ( sizeof(dirent_t) );
    StrCopy ( p -> filename, filename );
    p -> next = NULL;

    if ( dirlist ) {
      dirent_t *foo = dirlist;
      dirent_t *last = NULL;

      while ( ( foo ) &&
	      ( StrCaselessCompare ( p -> filename, foo -> filename ) > 0 ) )
      {
	last = foo;
	foo = foo -> next;
      }

      if ( last ) {
	last -> next = p;
	p -> next = foo;
      } else {
	p -> next = dirlist;
	dirlist = p;
      }

    } else {
      // first
      dirlist = p;
    }

  }

  return;
}

char *dirlist_lookup ( UInt16 entry ) {
  dirent_t *iter = dirlist;

  while ( entry ) {
    iter = iter -> next;
    entry--;
  }

  return ( iter -> filename );
}

// pass in zero to get a return of number of save files; pass in >0 to
// get details about that file
UInt16 palm_game_count ( UInt16 entry, char *r_filename ) {
  UInt16 volref = palm_vfs_iter();
  FileRef dir;
  FileInfoType file;
  UInt32 diter = vfsIteratorStart;
  char filename [ 256 ] = "";
  UInt16 counter = 0;
  Err error;

  file.nameP = filename; // point to local buffer
  file.nameBufLen = 256; // sizeof ( fileName );

  error = VFSFileOpen ( volref, VFSPATH, vfsModeRead, &dir );

  if ( error != errNone ) {
    return ( 0 );
  }

  while ( diter != vfsIteratorStop ) {
    VFSDirEntryEnumerate ( dir, &diter, &file );
#if 0
    if ( ( filename [ 0 ] ) &&
	 ( StrStr ( filename, ".sav" ) == NULL ) &&
	 ( StrStr ( filename, ".SAV" ) == NULL ) &&
	 ( StrStr ( filename, ".gfx" ) == NULL ) &&
	 ( StrStr ( filename, ".GFX" ) == NULL ) &&
	 ( StrStr ( filename, ".dir" ) == NULL ) &&
	 ( StrStr ( filename, ".DIR" ) == NULL )
       )
#else
    if ( filename [ 0 ] == '\0' ) {
      continue;
    }
    if ( ( StrStr ( filename, ".mag" ) ) ||
	 ( StrStr ( filename, ".MAG" ) ) ||
	 ( StrStr ( filename, ".dat" ) ) ||
	 ( StrStr ( filename, ".DAT" ) ) ||
	 ( StrStr ( filename, ".sa"   ) ) ||
	 ( StrStr ( filename, ".SA"   ) ) ||
	 ( StrStr ( filename, ".z"   ) ) ||
	 ( StrStr ( filename, ".Z"   ) ) ||
	 ( is_level9 ( filename ) )
       )
#endif
    {
      counter++;

      if ( entry == counter ) {
	StrCopy ( r_filename, filename );
	return ( 0 );
      }

    }
  } // while

  return ( counter );
}

/* private event loop; why you ask? Its so that we can have it modal, and
 * handle it differently than the out-of-game event loop
 */
void palm_event_loop ( void ) {
  UInt16 error;
  EventType event;
  static UInt8 autosavep = 0;

  g_loop = 1;

  // if g_loop (implies stop the loop), or if keyboard input is in there,
  // stop the loop so we can handle input
  while ( g_loop && keybuf [ 0 ] == '\0' ) {

    EvtGetEvent ( &event, evtWaitForever );

    // is there an intention to quit?
    if ( event.eType == appStopEvent ) {

      // user wants to bail; we can't bail until we autosave, so have
      // we autosaved yet? If so, we just exeunt normally. If we have not
      // yet autosaved.. do that and skip the event.

      if ( autosavep ) {
	// we've been here already...

	// have we completed the save yet? if not, keep going until
	// we have. If we have completed a save, the keyboard buffer
	// will be empty, so we can just bail out

	if ( keybuf [ 0 ] == '\0' ) {
	  // bail!
	  autosavep = 0; // reset for next game
	} else {
	  // keep going!
	  EvtAddEventToQueue ( &event );
	  event.eType = nilEvent;
	}

      } else {
	autosavep = 1; // mark as having been here

	/* auto save */
	force_auto_save();

	// keep going!
	EvtAddEventToQueue ( &event );
	event.eType = nilEvent;

      }

    } // stop event?

    if (! SysHandleEvent(&event))
      if (! MenuHandleEvent(0, &event, &error))
	if (! AppHandleEvent(&event))
	  FrmDispatchEvent(&event);

    if ( event.eType == appStopEvent ) {

      /* now we exeunt with alarums
       */
      if ( is_magnetic ( g_gamename ) ) {
	ms_stop();
      } else if ( is_zcode ( g_gamename ) ) {
	EventType e;

	// trigger quit
	z_quit();
	g_loop = 0;

	// for some reason needs another event to bail out, so
	// just fire one off :/
	e.eType = ctlSelectEvent;
	e.data.ctlSelect.controlID = MainCRLFGraphicButton;
	EvtAddEventToQueue ( &e );

      } else if ( is_level9 ( g_gamename ) ) {
	l9_stop();
	g_loop = 0;

      } else if ( is_scottfree ( g_gamename ) ) {
	extern UInt8 scott_finished;
	EventType e;

	g_loop = 0;
	scott_finished = 1;

	// for some reason needs another event to bail out, so
	// just fire one off :/
	e.eType = keyDownEvent;

	e.data.keyDown.chr = 'e';
	EvtAddEventToQueue ( &e );

	e.data.keyDown.chr = '\n';
	EvtAddEventToQueue ( &e );

      }
      break;
    }

  } // while

  return;
}

UInt32 query_heap_free ( void ) {
  UInt16 heapid = 0;
  UInt32 max, bytes;

  MemHeapFreeBytes ( heapid, &bytes, &max );

  return ( max );
}

void *MemPtrNewL ( UInt32 size ) {

#define MALLOCTHRESH 65500
  //#define MALLOCTHRESH 32000

#if 0
  {
    char buffer [ 50 ];
    StrPrintF ( buffer, "%lu / %lu", size, query_heap_free() );
    DEBUGS ( buffer );
  }
#endif

  if ( query_heap_free() < 5000 ) {
    DEBUGS( "Runtime memory very low! Kronos may not be able to operate "
	    "this game on this unit!" );
  } else if ( query_heap_free() < 15000 ) {
    DEBUGS( "Runtime memory getting low! Try disabling Hacks and DAs." );
  }

  if ( size > MALLOCTHRESH ) { // the max MemPtrNew will give *g*
    SysAppInfoPtr appInfoP;
    UInt16        ownerID;

    ownerID =
      ((SysAppInfoPtr)SysGetAppInfo(&appInfoP, &appInfoP))->memOwnerID;

    return MemChunkNew ( 0, size, ownerID |
			 memNewChunkFlagNonMovable |
			 memNewChunkFlagAllowLarge );

  }

  return MemPtrNew(size);
}

void MemPtrFreeL ( void *ptr ) {
  MemPtrFree(ptr);
}

FieldPtr GetFocusObjectPtr ( void ) {
  FormPtr f;
  UInt16 focus;
  FormObjectKind objtype;

  f = FrmGetActiveForm();
  focus = FrmGetFocus ( f );

  if ( focus == noFocus ) {
    return ( NULL );
  }

  objtype = FrmGetObjectType ( f, focus );

  if ( objtype == frmFieldObj ) {
    return ( FrmGetObjectPtr ( f, focus ) );
  } else if ( objtype == frmTableObj ) {
    return ( TblGetCurrentField ( FrmGetObjectPtr ( f, focus ) ) );
  }

  return ( NULL );
}

int update_field ( FormPtr f, int obid, char *newtext ) {
  MemHandle newhandle, oldhandle;
  FieldPtr field;

  newhandle = MemHandleNew ( StrLen ( newtext ) + 1 );

  if ( ! newhandle ) {
    return ( 0 );
  }

  StrCopy ( MemHandleLock ( newhandle ), newtext );
  MemHandleUnlock ( newhandle );

  field = FrmGetObjectPtr ( f, FrmGetObjectIndex ( f, obid ) );

  oldhandle = FldGetTextHandle ( field );

  FldSetTextHandle ( field, newhandle );

  if ( oldhandle ) {
    MemHandleFree ( oldhandle );
  }

  return ( 1 );
}

void *realloc ( void *ptr, UInt32 size ) {
#if 0
  Err error;
  error = MemPtrResize ( ptr, size );
  if ( error != errNone ) {
    return ( NULL );
  }
  return ( ptr );
#else
  UInt32 oldsize = MemPtrSize ( ptr );
  UInt8 *newp = MemPtrNewL ( size );
  if ( ! newp ) {
    //DEBUGU32(query_heap_free());
    //DEBUGU32(size);
    return ( NULL ); // out of memory!
  }
  MemMove ( newp, ptr, oldsize < size ? oldsize : size );
  MemPtrFree ( ptr );
  return ( newp );
#endif
}

int printf ( const char *fmt, ... ) {
  return ( 0 );
}

UInt8 palm_file_exists ( char *name ) {
  UInt32 voliter = palm_vfs_iter();
  char path [ 256 ];
  FileRef file;

  StrPrintF ( path, "%s%s", VFSPATH, name );

  if ( VFSFileOpen ( voliter, path, vfsModeReadWrite, &file ) == errNone ) {
    VFSFileClose ( file );
    return ( 1 ); // was able to open it
  }

  return ( 0 );
}

/* Builds "<card>/Palm/Programs/Kronos/<game>.dir/<name>.sav" for a
 * Magnetic Scrolls save file; copies the bare file name to filename.
 */
static void ms_save_path ( type8s *name, char *path, char *filename,
			   UInt16 filename_size ) SEC_KRONOS;
static void ms_save_path ( type8s *name, char *path, char *filename,
			   UInt16 filename_size )
{
  if ( name == NULL ) {
    name = "default"; // a bug in Wonderland gives us a NULL name
  }

  StrNCopy ( filename, name, filename_size - 5 );
  filename [ filename_size - 5 ] = '\0';
  if ( StrStr ( filename, ".sav" ) == NULL ) {
    StrCat ( filename, ".sav" );
  }

  StrPrintF ( path, "%s%s/%s", VFSPATH, g_savedir, filename );
}

/* Both return 0 on success. On failure the game reports the error. */
type8 ms_load_file ( type8s *name, type8 *ptr, type16 size ) {
  UInt32 voliter = palm_vfs_iter();
  char path [ 256 ];
  char filename [ 40 ];
  FileRef file;
  UInt32 result = 0;
  Err error;

  ms_save_path ( name, path, filename, sizeof ( filename ) );

  if ( VFSFileOpen ( voliter, path, vfsModeRead, &file ) != errNone ) {
    return ( 1 );
  }

  error = VFSFileRead ( file, size, ptr, &result );

  VFSFileClose ( file );

  if ( error != errNone || result != size ) {
    return ( 1 );
  }

  StrCopy ( g_last_loaded_file, filename );

  return ( 0 );
}

type8 ms_save_file ( type8s *name, type8 *ptr, type16 size ) {
  UInt32 voliter = palm_vfs_iter();
  char path [ 256 ];
  char filename [ 40 ];
  FileRef file;
  UInt32 result = 0;
  Err error;

  StrPrintF ( path, "%s%s", VFSPATH, g_savedir );
  VFSDirCreate ( voliter, path ); // fails harmlessly if it exists

  ms_save_path ( name, path, filename, sizeof ( filename ) );

  VFSFileCreate ( voliter, path ); // fails harmlessly if it exists

  if ( VFSFileOpen ( voliter, path, vfsModeReadWrite, &file ) != errNone ) {
    return ( 1 );
  }

  error = VFSFileWrite ( file, size, ptr, &result );

  VFSFileClose ( file );

  if ( error != errNone || result != size ) {
    return ( 1 );
  }

  return ( 0 );
}

void ms_statuschar ( type8 c ) {
  static char buf_left [ 250 ];
  static char buf_right [ 250 ];
  static char *cursor = buf_left;

  if ( c == '\t' ) {
    if ( ( cursor >= buf_left ) &&
	 ( cursor <= ( buf_left + sizeof(buf_left) ) ) )
    {
      cursor = buf_right;
    } else {
      cursor = buf_left;
    }
  } else if ( c == '\n' || c == '\r' ) {
    cursor = buf_left;
    MemSet ( status_left, sizeof(status_left), '\0' ); // clear
    MemSet ( status_right, sizeof(status_right), '\0' ); // clear
    StrCopy ( status_left, buf_left );
    StrCopy ( status_right, buf_right );
    MemSet ( buf_left, sizeof(buf_left), '\0' ); // clear
    MemSet ( buf_right, sizeof(buf_right), '\0' ); // clear

    // register with mapper
    mapper_location ( status_left );

  } else if ( ( cursor >= buf_left &&
		cursor < buf_left + sizeof ( buf_left ) - 1 ) ||
	      ( cursor >= buf_right &&
		cursor < buf_right + sizeof ( buf_right ) - 1 ) )
  {
    *cursor++ = c; /* anything beyond the buffer is dropped */
  }

  return;
}

void ms_statuschar_string ( char *s ) {
  while ( *s != '\0' ) {
    ms_statuschar ( *s );
    s++;
  }
  return;
}

void ms_flush ( void ) {

  /* show all the lines from screen-last-shown to screen-tail, scrolling as
   * necessary and pausing for [MORE] as necessary.
   */

  if ( screen_last_shown == screen_tail ) {
    return; // nothing to show
  }

  if ( screen_last_shown == NULL ) {
    screen_last_shown = screen_head;
  }

  // screen_push ( '\n' );

  /* render */
  screen_draw ( screen_last_shown, screen_tail );
  screen_last_shown = screen_tail;

  /* trim old history so we don't run out of RAM */
  screen_trim_history ( 200, 50 );


  return;
}

void ms_putchar ( type8 c ) {
  screen_push ( c );
  return;
}

void keyboard_clear ( void ) {
  keybuf [ 0 ] = '\0';
}

void keyboard_push ( char *text ) {

  // reset scrolling if we've moved the display..
  screen_reset_scroll();

  // push char
  StrCat ( keybuf, text );
  if ( StrChr ( text, '\n' ) ) {
    g_loop = 0; // stop so input can be registered
  } else {
    UInt8 iter;
    for ( iter = 0; iter < StrLen ( text ); iter++ ) {
      ms_putchar ( text [ iter ] );
    }
  }

  // register with the mapper
  mapper_command ( text );

  return;
}

type8 ms_getchar ( type8 trans ) {
  char c = '\n';
  static UInt8 futzed_p = 0;

  g_waiting_for_key = 1;

  /* we swallowing input? If so, bail if keybuf is empty.
   */
  if ( ( g_runflags & R_SWALLOW ) &&
       ( StrChr ( keybuf, '\0' ) - keybuf < 5 ) )
  {
    g_runflags &= ~R_SWALLOW;
  }

  /* since the game is expecting input, it must have displayed a prompt; if
   * the prompt is not ">" like we expect, show the prompt over the button
   * bar so the user knows what hes doing..
   */
  if ( ( screen_tail ) &&
       ( StrChr ( screen_tail -> text, '>' ) == NULL )
     )
  {
    RectangleType r;

    // erase prompt row
    r.topLeft.x = 0;
    r.topLeft.y = 132;
    r.extent.x = 160; //110;
    r.extent.y = 10;
    WinEraseRectangle ( &r, 0 );

    // erase thumbnail
    if ( is_magnetic ( g_gamename ) || is_level9 ( g_gamename ) ) {
      r.topLeft.x = THUMB_X;
      r.topLeft.y = THUMB_Y;
      r.extent.x = 60;
      r.extent.y = 60;
      WinEraseRectangle ( &r, 0 );
    }

    // draw prompt
    WinDrawChars ( screen_tail -> text,
		   StrLen ( screen_tail -> text ), 0, 132 );

    // note that we've futzed up the display
    futzed_p = 1;

  } else if ( futzed_p ) {

    draw_controls();

    if ( is_magnetic ( g_gamename ) ) {
      screen_draw_pic ( THUMB_X, THUMB_Y, g_current_pic, 5 );
    } else if ( is_level9 ( g_gamename ) ) {
      l9_draw_thumbnail();
    }

    futzed_p = 0;
  }

  /* run event loop
   */
  if ( keybuf [ 0 ] == '\0' ) {
    palm_event_loop();
  }

  /* something in the in buffer? feed it back to system!
   */
  if ( keybuf [ 0 ] ) {
    c = keybuf [ 0 ];
    MemMove ( keybuf, keybuf + 1, StrLen ( keybuf + 1 ) + 1 );
  }

  // frotz to make it look better, we push in a return (Magnetic Scrolls
  // and Level 9 games print their own)
  if ( c == '\n' && ! is_magnetic ( g_gamename ) &&
       ! is_level9 ( g_gamename ) )
  {
    screen_push ( '\n' );
  }

  // done
  g_waiting_for_key = 0;

#if 0
  // old thinking code
  erase_controls();
  WinDrawBitmap ( b_thinking, 0, 132 );
#endif
  //draw_thinking();

  return ( c );
}

void ms_showpic ( type32 c, type8 mode ) {

  // we swallowing input right now?
  if ( ! ( g_runflags & R_SWALLOW ) ) {

    // showing a different pic from last time?
    if ( c != g_current_pic ) {
      /* erase the old thumbnail first: a shorter picture would leave its
       * bottom rows behind, recoloured by the new picture's palette */
      RectangleType r;
      r.topLeft.x = THUMB_X;
      r.topLeft.y = THUMB_Y;
      r.extent.x = 60;
      r.extent.y = 60;
      WinEraseRectangle ( &r, 0 );

      g_current_pic = c;
      g_current_pic_mode = mode;
      screen_draw_pic ( THUMB_X, THUMB_Y, g_current_pic, 5 );
    }

  }

  return;
}

/* The interpreters call exit() when they cannot carry on. On Palm OS that
 * means abandoning the game: MainForm.c arms g_exit_jump (ErrSetJump) before
 * starting a game, and we unwind back there, after which the game's memory is
 * released and the picker is shown again.
 */
ErrJumpBuf g_exit_jump;
UInt8 g_exit_armed = 0;

void exit ( int status ) {

  if ( status != 0 ) {
    FrmCustomAlert ( MessageAlert,
		     "The game has stopped because of an error.", NULL, NULL );
  }

  if ( g_exit_armed ) {
    g_exit_armed = 0;
    ErrLongJump ( g_exit_jump, 1 );
  }

  ErrFatalDisplay ( "exit() called with no game running" );
  for ( ;; ) ;
}

void ms_fatal ( char *txt ) {
  return;
}

type8 ms_showhints ( struct ms_hint * hints ) {
  return 0; // skip it
}

UInt8 is_magnetic ( char *filename ) {

  if ( StrStr ( filename, ".mag" ) ||
       StrStr ( filename, ".MAG" ) )
  {
    return ( 1 );
  }

  return ( 0 );
}

UInt8 is_scottfree ( char *filename ) {

  if ( StrStr ( filename, ".sa" ) ||
       StrStr ( filename, ".SA" ) )
  {
    return ( 1 );
  }

  return ( 0 );
}

/* Level 9 games: GAMEDAT1.DAT etc. renamed to <game><part>.l9, or
 * Spectrum snapshots */
UInt8 is_level9 ( char *filename ) {

  if ( StrStr ( filename, ".l9" ) ||
       StrStr ( filename, ".L9" ) ||
       StrStr ( filename, ".sna" ) ||
       StrStr ( filename, ".SNA" ) )
  {
    return ( 1 );
  }

  return ( 0 );
}

UInt8 is_zcode ( char *filename ) {

  /* ScottFree games are often named "adv01.sa.dat"; they are not Z-code
   * even though they end in .dat. Callers test the types in different
   * orders, so the types must not overlap.
   */
  if ( is_scottfree ( filename ) ) {
    return ( 0 );
  }

  if ( StrStr ( filename, ".dat" ) ||
       StrStr ( filename, ".DAT" ) ||
       StrStr ( filename, ".z" ) ||
       StrStr ( filename, ".Z" )
     )
  {
    return ( 1 );
  }

  return ( 0 );
}

int emu_main ( int argc, char **argv ) {
  type8 running, /* i, */ *gamename = 0, *gfxname = 0, *hintname = 0;
  type32 dlimit, slimit;

  if (sizeof(type8) != 1 || sizeof(type16) != 2 || sizeof(type32) != 4)	{
    fprintf(stderr,
	    "You have incorrect typesizes, please edit the typedefs and\n"
	    "recompile or proceed on your own risk...\n");
    exit(1);
  }

  dlimit = slimit = 0xffffffff;

#if 0
  for (i = 1; i < argc; i++) {

    if (argv[i][0] == '-') {

      switch (tolower(argv[i][1])) {
      case 'd':
	if (strlen(argv[i]) > 2)
	  dlimit = atoi(&argv[i][2]);
	else
	  dlimit = 0;
	break;

      case 's':
	if (strlen(argv[i])>2)
	  slimit = atoi(&argv[i][2]);
	else
	  slimit = 655360;
	break;

      case 't':
	if (!(log2 = fopen(&argv[i][2],"w")))
	  printf("Failed to open \"%s\" for writing.\n",&argv[i][2]);
	break; 

      case 'r':
	if (log1 = fopen(&argv[i][2],"r"))
	  log_on = 1;
	else
	  printf("Failed to open \"%s\" for reading.\n",&argv[i][2]);
	break;

      case 'w':
	if (log1 = fopen(&argv[i][2],"w"))
	  log_on = 2;
	else
	  printf("Failed to open \"%s\" for writing.\n",&argv[i][2]);
	break;

      default:
	printf("Unknown option -%c, ignoring.\n",argv[i][1]);
	break;
      } // switch

    } // if
    else if (!gamename)
      gamename = argv[i];
    else if (!gfxname)
      gfxname = argv[i];
    else if (!hintname)
      hintname = argv[i];

  } // for
#endif

  //gamename = "pawn.mag";
  //gfxname = "pawn.gfx";

  //gamename = "myth.mag";
  //gfxname = "myth.gfx";

  gamename = g_gamename;
  gfxname = g_gfxname;

  if ( ! gamename ) {
    printf("Magnetic 2.2 - a Magnetic Scrolls interpreter\n\n");
    printf("Usage: %s [options] game [gfxfile] [hintfile]\n\n"
	   "Where the options are:\n"
	   " -dn    activate register dump (after n instructions)\n"
	   " -rname read script file\n"
	   " -sn    safety mode, exits automatically (after n instructions)\n"
	   " -tname write transcript file\n"
	   " -wname write script file\n\n"
	   "The interpreter commands are:\n"
	   " #undo   undo - don't use it near are_you_sure prompts\n"
	   " #logoff turn off script writing\n\n",argv[0]);
    exit(1);
  }

  if ( ! ( ms_gfx_enabled = ms_init ( gamename, gfxname, hintname ) ) ) {
    printf("Couldn't start up game \"%s\".\n",gamename);
    exit(1);
  }

  ms_gfx_enabled--;
  running = 1;

  while ( ( ms_count() < slimit ) && running ) {
    if (ms_count() >= dlimit)
      ms_status();
    running = ms_rungame();
  }

  if ( ms_count() == slimit ) {
    printf("\n\nSafety limit (%ld) reached.\n",slimit);
    ms_status();
  }

  ms_freemem();

  if (log_on)
    fclose(log1);
  if (log2)
    fclose(log2);

  printf("\nExiting.\n");

  return 0;
}

void draw_battery ( void ) {
  Boolean plug;
  UInt8 perc;

  SysBatteryInfo ( false, NULL, NULL, NULL, NULL, &plug, &perc );

  if ( plug == true ) {
    WinDrawBitmap ( b_bat4, 80, 133 );
  } else if ( perc < 25 ) {
    WinDrawBitmap ( b_bat1, 80, 133 );
  } else if ( perc < 50 ) {
    WinDrawBitmap ( b_bat2, 80, 133 );
  } else if ( perc < 75 ) {
    WinDrawBitmap ( b_bat3, 80, 133 );
  } else {
    WinDrawBitmap ( b_bat4, 80, 133 );
  }

  return;
}

void draw_progress_bar ( UInt8 perc ) {
  static UInt32 lasttime = 0;

  if ( lasttime != TimGetSeconds() ) {
    RectangleType r;

    r.topLeft.x = 0;
    r.topLeft.y = 133;
    r.extent.y = 3;

    r.extent.x = 160 * perc / 100;
    WinDrawRectangle ( &r, 0 );

    r.topLeft.x = r.extent.x;
    r.extent.x = 160 - r.extent.x;
    WinEraseRectangle ( &r, 0 );

    lasttime = TimGetSeconds();
  }

  return;
}

void erase_thinking ( void ) {
  RectangleType r;

  r.topLeft.x = 90;
  r.topLeft.y = 142;
  r.extent.x = 22;
  r.extent.y = 11;

  WinEraseRectangle ( &r, 0 );

  return;
}

void draw_thinking ( void ) {
  WinDrawBitmap ( b_thinking, 90, 142 );
}

void erase_controls ( void ) {
  RectangleType r;

  r.topLeft.x = 0;
  r.topLeft.y = 132;
  r.extent.x = 160; //110;
  r.extent.y = 30; //10;

  WinEraseRectangle ( &r, 0 );

  return;
}

void draw_controls ( void ) {
  RectangleType r;

  r.topLeft.x = 0;
  r.topLeft.y = 132;
  r.extent.x = 160; //110;
  r.extent.y = 30; //10;

  WinEraseRectangle ( &r, 0 );

  CtlDrawControl ( (ControlType*) GetObjectPtr ( MainLGraphicButton ) );
  CtlDrawControl ( (ControlType*) GetObjectPtr ( MainNGraphicButton ) );
  CtlDrawControl ( (ControlType*) GetObjectPtr ( MainSGraphicButton ) );
  CtlDrawControl ( (ControlType*) GetObjectPtr ( MainEGraphicButton ) );
  CtlDrawControl ( (ControlType*) GetObjectPtr ( MainWGraphicButton ) );
  CtlDrawControl ( (ControlType*) GetObjectPtr ( MainIGraphicButton ) );
  CtlDrawControl ( (ControlType*) GetObjectPtr ( MainNavigateGraphicButton ) );
  CtlDrawControl ( (ControlType*) GetObjectPtr ( MainWordsGraphicButton ) );
  //CtlDrawControl ( (ControlType*) GetObjectPtr ( MainNil3GraphicButton ) );
  CtlDrawControl ( (ControlType*) GetObjectPtr ( MainLoadGraphicButton ) );
  CtlDrawControl ( (ControlType*) GetObjectPtr ( MainSaveGraphicButton ) );

  FrmCopyLabel ( FrmGetActiveForm(), MainPromptLabel, ">" );
  CtlDrawControl ( (ControlType*) GetObjectPtr ( MainCRLFGraphicButton ) );
  CtlDrawControl ( (ControlType*) GetObjectPtr ( MainClearGraphicButton ) );

  CtlDrawControl ( (ControlType*) GetObjectPtr ( MainScrollUpRepeating ) );
  CtlDrawControl ( (ControlType*) GetObjectPtr ( MainScrollDownRepeating ) );

  FldDrawField ( (FieldType*) GetObjectPtr ( MainEntryField ) );

  draw_battery();

  return;
}
