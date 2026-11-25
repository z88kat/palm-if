
#include <PalmOS.h>
#include <VfsMgr.h>
#include <PceNativeCall.h>

#include "KronosRsc.h"

#include "defs.h"
#include "kronos.h"
#include "sections.h"
#include "display.h"

line_t *screen_head = NULL;
line_t *screen_last_shown = NULL;
line_t *screen_tail = NULL;
line_t *screen_last_draw = NULL;

UInt16 g_screen_width = 160;
UInt16 g_screen_wrappoint = 155;
UInt16 g_screen_length = 0;        // number of nodes in linked list

UInt16 g_row_height = 0;           // effective pixels/per row in 160scale
UInt16 g_scroll_up = 0;            // how scrolled we are

WinHandle g_winh = 0;
BitmapType *g_winb = NULL;
unsigned char *g_winbits = NULL;
UInt16 g_winbits_density = 0;

char status_left [ 250 ] = "\0\0";
char status_right [ 250 ] = "\0\0";

UInt16 g_y = 10; // y coord for text drawing

UInt8 g_font_height = 10;

UInt8 g_nextchar_style = 0;
UInt8 g_eat_newline = 0;

UInt32 g_current_theme = ct_black_on_white;

/* Code section declarations (see sections.h) */
static UInt16 EffectiveFntCharWidth (char x) SEC_KRONOS;
static UInt32 screen_native_call (DmResType resType, DmResID resID, void *userDataP) SEC_KRONOS;

void screen_format ( UInt16 maxwidth, UInt16 wrappoint ) {

  g_screen_width = maxwidth;
  g_screen_wrappoint = wrappoint;

  g_winh = WinGetDisplayWindow(); //WinGetActiveWindow();
  g_winb = WinGetBitmap ( g_winh );

  if ( g_features & F_OS5HIGHRES ) {
    g_winbits = BmpGetBits ( g_winb );
    g_winbits_density = BmpGetDensity ( g_winb );
  }

  return;
}

static UInt16 EffectiveFntCharWidth ( char x ) {
  UInt8 width;

  if ( g_features & F_MEDFONT ) {
    FntSetFont ( largeFont );
  }

  width = FntCharWidth ( x );

  if ( g_features & F_MEDFONT ) {
    FntSetFont ( stdFont );
  }

  if ( g_features & F_TINYFONT ) {
    width >>= 1;
  }

  return ( width );
}

void screen_push_style ( UInt8 style ) {
  g_nextchar_style = style;
  return;
}

void screen_push_string ( char *s ) {
  while ( *s != '\0' ) {
    screen_push ( *s );
    s++;
  }
}

/* remember that the first invocation, there is no list at all yet, so make
 * one!
 */
void screen_push ( char c ) {
  char *insert;

  // swallowing output?
  if ( g_runflags & R_SWALLOW ) {
    return; // yum!
  }

  // first invocation?
  if ( screen_head == NULL ) {
    screen_head = MemPtrNew ( sizeof(line_t) ); // make
    if ( ! screen_head ) {
      DEBUGS("Screen: Out of memory");
    }
    MemSet ( screen_head, sizeof(line_t), '\0' ); // clear
    screen_tail = screen_head;
    g_screen_length ++;
  } // first time?

  // conserve RAM?
  //MemHeapCompact ( MemPtrHeapID ( screen_head ) );

#if 0
  if ( c > 0 ) {
    char buffer [ 20 ];
    StrPrintF ( buffer, "%u '%c' %s", c, c, screen_tail -> text );
    DEBUGS(buffer);
  }
#endif

#if 0
  if ( screen_head != screen_tail ) {
    ms_flush();
  }
#endif

  // discard?
  if ( c == '\r' ) {
    return;
  }

  if ( c == '\b' ) {
    return;
  }

  // add to screen system
  if ( ( StrLen ( screen_tail -> text ) > WRAPPOINT ) ||
       ( screen_tail -> pixwidth > g_screen_wrappoint ) )
  {
    /* by this point we have a buffer full of spaces or text, and want to add
     * a character to the end of it.. but its too far, so we need to wrap
     * which means making a new line, pointing the tail at it, and adding the
     * char. We move an incomplete word to the new line. If the word is the
     * whole line, we just wrap without copying it to avoid a loop.
     */

    line_t *last = screen_tail;               // record keeping
    line_t *n = MemPtrNew ( sizeof(line_t) ); // make
    if ( ! n ) {
      //DEBUGS("Screen: Out of memory");
      DEBUGU8(g_screen_length);
    }
    MemSet ( n, sizeof(line_t), '\0' );       // clear
    screen_tail -> next = n;                  // hook up the list
    screen_tail = n;                          // new tail pointer
    g_screen_length ++;

    if ( isspace ( c ) || c == '\n' ) {
      // its a space, so just ignore it and wrap, which we've already done
    } else {
      // its not a space; so wrap (or not) and make a new item and append it

      /* anything to copy from old tail? If so, copy it
       * Cases:
       * 1) Normal -- char end (blah blah blah)
       *    o) Look for last space; copy word form space to end
       * 2) Normal -- space end (blah blah blah )
       *    o) Just insert new char into new line
       * 2) No spaces (blaaaaaaaaaaaaaaaaaaaaaaaaah)
       *    o) Just insert new char into new line
       * 3) All spaces (              )
       *    o) Just insert new char into new line
       * So the only interesting case is the first one.
       */

      /* so was there a space somewhere in the line, and yet the last
       * character is not a space.. thats the normal case. Otherwise
       * its a freak case.
       */
      if ( ( StrChr ( last -> text, ' ' ) ) &&
	   ( ! isspace ( last -> text [ StrLen ( last -> text ) - 1 ] ) ) )
      {
	// word wrap first, then append
	char *iter = last -> text + StrLen ( last -> text ) - 1;

	// find last space
	while ( *iter != ' ' ) {
	  iter--;
	}

	// copy word after space
	StrCopy ( screen_tail -> text, iter + 1 );

	// erase word
	*iter = '\0';

      } else {
	// just append as normal..
      }

      // append new char
      insert = StrChr ( screen_tail -> text, '\0' );
      *insert = c;

      // append attributes
#ifdef ATTR
      screen_tail -> attr [ insert - screen_tail -> text ] = g_nextchar_style;
#endif

      // determine pixel width of new line (could be one char, could be
      // a bunch!)
      {
	char *iter = screen_tail -> text;
	screen_tail -> pixwidth  = 0;
	while ( *iter != '\0' ) {
	  screen_tail -> pixwidth += EffectiveFntCharWidth ( *iter );
	  iter++;
	}
      }

    } // space or char?

  } else {

    if ( c == '\n' ) {
      // make a new line and move to it
      line_t *n = MemPtrNew ( sizeof(line_t) ); // make
      if ( ! n ) {
	DEBUGS("Screen: Out of memory");
      }
      MemSet ( n, sizeof(line_t), '\0' );       // clear
      screen_tail -> next = n;                  // hook up the list
      screen_tail = n;                          // new tail pointer
      g_screen_length ++;

    } else {

      // just add the char; normal case
      insert = StrChr ( screen_tail -> text, '\0' );
      *insert = c;

      // append attributes
#ifdef ATTR
      screen_tail -> attr [ insert - screen_tail -> text ] = g_nextchar_style;
#endif

      screen_tail -> pixwidth += EffectiveFntCharWidth ( c );
    }

  }

  screen_push_style ( K_NO_STYLE ); // clear

  return;
}

/* if we have max_keep or more in buffer, trim to max_keep - under amount
 */
void screen_trim_history ( UInt16 max_keep, UInt16 under ) {
  UInt16 killcount;
  line_t *iter, *next;

  if ( g_screen_length <= max_keep ) {
    return; // not yet exceeded
  }

  // need to trim some lines..
  killcount = g_screen_length - max_keep + under;

#if 0
  {
    char buffer [ 200 ];
    StrPrintF ( buffer, "Got %d, kill %d", g_screen_length, killcount );
    DEBUGS(buffer);
  }
#endif

  // killemall!
  iter = screen_head;
  next = iter -> next;

  while ( killcount ) {

    MemPtrFree ( iter );

    killcount--;
    iter = next;
    next = next -> next;
  }

  screen_head = iter;
  g_screen_length = max_keep - under;

  return;
}

UInt16 screen_query_row ( UInt16 ypixel ) {

  //DEBUGU8(ypixel);
  //DEBUGU8(g_y);

  if ( g_features & F_TINYFONT ) {
    UInt8 lineadder = 5;

    if ( g_features & F_MEDFONT ) {
      lineadder = 6;
    }

    /* has the whole screen yet been filled? If not, we have special
     * considerations..
     */
    if ( g_y < 120 ) {
      return (      (    ( g_y - ypixel )    / lineadder )     + 1 );
    } else {
      return (      (    ( g_y + 10 - ypixel )    / lineadder )     + 0 );
    }

  }

  return ( ( ( 130 - ypixel ) / g_font_height ) + 1 );
}

line_t *screen_query_at ( UInt16 lines_up ) {
  UInt16 skipper;
  line_t *iter;

  if ( g_screen_length <= lines_up ) {
    return ( NULL );
  }

  skipper = g_screen_length - lines_up - 1;

  iter = screen_head;

  while ( skipper -- ) {
    iter = iter -> next;
  }

  return ( iter );
}

UInt8 screen_query_word_at ( UInt16 lines_up, UInt16 x, char *r_word ) {
  line_t *line = screen_query_at ( lines_up );
  UInt16 pixiter;
  char *iter;

  if ( ! line ) {
    return ( 0 );
  }

  //DEBUGS(line->text);

  iter = line -> text;
  pixiter = 0;

  if ( g_features & F_TINYFONT ) {
    if ( g_features & F_MEDFONT ) {
      FntSetFont ( largeFont );
    }
    while ( *iter && pixiter < (2*x) ) {
      pixiter += FntCharWidth ( *iter );
      iter++;
    }
    if ( g_features & F_MEDFONT ) {
      FntSetFont ( stdFont );
    }
  } else {
    while ( *iter && pixiter < x ) {
      pixiter += FntCharWidth ( *iter );
      iter++;
    }
  }

  if ( ( *iter ) &&
       ( ! isspace ( *iter ) ) )
  {
    // found a word under pen!
    char *begin = iter;
    UInt8 count;

    // trace back for non-space
    while ( ( iter >= line -> text ) &&
	    ( ! isspace ( *iter ) ) )
    {
      begin = iter;
      iter--;
    }

    if ( ( iter >= line -> text ) &&
	 ( isspace ( *iter ) ) )
    {
      iter++;
    }

    //DEBUGS(begin);

    // find space after begin of word
    iter = StrChr ( begin, ' ' );

    // no space? find end of line then!
    if ( ! iter ) {
      iter = StrChr ( begin, '\0' );
    }

    // copy the word!
    //StrNCopy ( r_word, begin, iter - begin );
    count = iter - begin;
    while ( count-- ) {
      if ( TxtCharIsAlNum ( *begin ) ) {
	*r_word++ = *begin;
      }
      begin++;
    }

    return ( 1 );
  }

  return ( 0 );
}

void screen_draw ( line_t *begin, line_t *end ) {
  UInt16 xmin = 0;
  RectangleType rectwaste;
  RectangleType rect;
  UInt8 lines_so_far = 0;
  UInt8 rowscale = 1;

  // are we showing only end results here?
  if ( ( g_features & F_OS5HIGHRES ) &&
       ( g_runflags & R_HOLDDRAW ) )
  {
    WinScreenLock ( winLockCopy /* winLockDontCare */ );
  }

  // default
  g_font_height = 10; // back to default before figuring it out again

  // pick font
  if ( g_features & F_TINYFONT ) {
    rowscale = 2;

    if ( g_features & F_MEDFONT ) {
      FntSetFont ( largeFont );
      g_font_height = FntLineHeight() - 1;
    }

  } else {
    FntSetFont ( stdFont );
    g_font_height = FntLineHeight() - 1;
  }

  g_row_height = g_font_height / rowscale;

  // start from the bottom as of Kronos 1.2.2
  if ( g_y == 10 ) {
    while ( g_y <= 110 ) {
      g_y += g_row_height;
    }
  }

  // scroll area
  rect.topLeft.x = 0;
  rect.topLeft.y = 10;
  rect.extent.x = 160;
  rect.extent.y = 121; // was 120

  if ( begin ) {
    /* log beginning for later, in case we need to redraw */
    screen_last_draw = begin;
  } else {
    /* we weren't told a beginning, so just redraw last screen! */
    begin = screen_last_draw;
  }

  if ( ! begin ) {
    return; // woops!
  }

  /* show all the new lines
   */
  while ( begin != end ) {

    if ( g_y >= 110 ) {
      static UInt8 foo = 3; /* getting some weird spacing in there.. this
			     * is a kludge fix that needs to be investigated
			     */
      if ( foo >= 3 ) {
	//WinDrawRectangleFrame ( 1, &rect );
	WinScrollRectangle ( &rect, winUp,
			     g_font_height / rowscale, &rectwaste );
	WinEraseRectangle ( &rectwaste, 0 );
      } else {
	foo++;
      }
    }

    // render it
    screen_draw_row ( begin, xmin, g_y );

    begin = begin -> next;

    if ( g_y < 120 ) {
      g_y += ( g_font_height / rowscale );
    }

    lines_so_far ++;

    if ( lines_so_far > (11*rowscale) && begin -> next ) {

      // skip more prompt if we're forcing fullscreen redraw
      if ( ! ( g_runflags & R_FULLREDRAW ) ) {
	screen_prompt_more();
      }

      lines_so_far = 0;
    }

  } // while

  /* reset
   */
  FntSetFont ( stdFont );

  /* status line
   */
  {
    /* clear old text */
    char clear[] = "                                                         "
                   "                                                         ";
    WinDrawInvertedChars ( clear, StrLen ( clear ), xmin, 0 );

    /* draw new text */
    WinDrawInvertedChars ( status_left, StrLen ( status_left ), xmin, 0 );
    WinDrawInvertedChars
      ( status_right, StrLen ( status_right ),
	160 - FntCharsWidth ( status_right, StrLen ( status_right ) ), 0 );
  }

  // are we showing only end results here?
  if ( ( g_features & F_OS5HIGHRES ) &&
       ( g_runflags & R_HOLDDRAW ) )
  {
    WinScreenUnlock();
  }

  return;
}

void screen_draw_row ( line_t *l, UInt16 x, UInt16 y ) {

  /* draw using OS5 tiny font trick? */
  if ( g_features & F_TINYFONT ) {
    BmpSetDensity ( g_winb, kDensityLow );
    WinSetDrawMode ( winOverlay ); // cheesily avoid clipping descender
    WinPaintChars ( l -> text, StrLen ( l -> text ), x, y );
    BmpSetDensity ( g_winb, g_winbits_density );
  } else {
    WinDrawChars ( l -> text, StrLen ( l -> text ), x, y );
  }

  return;
}

static UInt32
screen_native_call ( DmResType resType, DmResID resID, void *userDataP )
{
  UInt32    processorType;
  MemHandle armH;
  MemPtr    armP;
  UInt32    result;

  // get the processor type
  FtrGet ( sysFileCSystem, sysFtrNumProcessorID, &processorType );
	
  // running on ARM? (if not, old machine, or perhaps Simulator on x86)
#if 0
  if ( ( processorType >= sysFtrNumProcessorARM720T ) &&
       ( processorType <= sysFtrNumProcessorARM710A ) )
#else
  if ( sysFtrNumProcessorIsARM ( processorType ) )
#endif
  {
    // running on ARM; call the actual ARM resource
    armH = DmGetResource ( resType, resID );

    if ( ! armH ) {
      return ( 0 );
    }

    armP = MemHandleLock ( armH );

    if ( ! armP ) {
      return ( 0 );
    }
	
    result = PceNativeCall ( armP, userDataP );
	
    MemHandleUnlock ( armH );
    DmReleaseResource ( armH );

  } else if ( processorType == sysFtrNumProcessorx86 ) {
    // foo
    result = 0xFFFF;
  } else {
    // foo
    result = 0xFFFF;
  }
	
  return ( result );
}

static UInt8 *screen_pic_get ( UInt32 pic, UInt16 *w, UInt16 *h,
			       RGBColorType *pal, UInt16 *ncolours ) SEC_KRONOS;
static void screen_pic_render ( UInt32 sx, UInt32 sy, UInt8 *p,
				UInt16 w, UInt16 h, RGBColorType *pal,
				UInt16 ncolours, UInt8 scale ) SEC_KRONOS;

/* The picture to show and its palette (up to 32 colours, 8 bits a
 * channel): Magnetic Scrolls pictures come from the game by number, Level 9
 * ones from l9palm.c. Returns NULL if there is none. */
static UInt8 *screen_pic_get ( UInt32 pic, UInt16 *w, UInt16 *h,
			       RGBColorType *pal, UInt16 *ncolours ) {
  UInt8 *p;
  UInt16 mspal [ 16 ];
  UInt8 anim;
  UInt16 iter;

  if ( is_level9 ( g_gamename ) ) {
    return ( l9_picture ( w, h, pal, ncolours ) );
  }

  p = ms_extract ( pic, w, h, mspal, &anim );

  /* Magnetic Scrolls colours have 3 bits for each pigment (a range of
   * 0-7), so multiply each by 32 to get a range of 256 */
  for ( iter = 0; iter < 16; iter++ ) {
    pal [ iter ].r = ( ( mspal [ iter ] >> 8 ) & 0x0007 ) << 5;
    pal [ iter ].g = ( ( mspal [ iter ] >> 4 ) & 0x0007 ) << 5;
    pal [ iter ].b = ( ( mspal [ iter ] >> 0 ) & 0x0007 ) << 5;
  }
  *ncolours = 16;

  return ( p );
}

void screen_draw_pic ( UInt32 sx, UInt32 sy, UInt32 pic, UInt8 scale ) {
  UInt8 *p;
  UInt16 w, h, ncolours;
  RGBColorType pal [ 32 ];

  p = screen_pic_get ( pic, &w, &h, pal, &ncolours );
  if ( p ) {
    screen_pic_render ( sx, sy, p, w, h, pal, ncolours, scale );
  }
}

/* Draws the current picture as large as fits in maxw x maxh at (x, y),
 * centred across that width if centre is set */
void screen_draw_pic_fit ( Int16 x, Int16 y, Int16 maxw, Int16 maxh,
			   Boolean centre ) {
  UInt8 *p;
  UInt16 w, h, ncolours;
  RGBColorType pal [ 32 ];
  UInt8 scale = 2; // the smallest the drawing code supports

  p = screen_pic_get ( g_current_pic, &w, &h, pal, &ncolours );
  if ( ! p ) {
    return;
  }

  while ( w / scale > maxw || h / scale > maxh ) {
    scale++;
  }
  if ( centre ) {
    x += ( maxw - w / scale ) / 2;
  }

  screen_pic_render ( x, y, p, w, h, pal, ncolours, scale );
}

/* Draws picture p (w x h, one palette index per pixel) at (sx, sy), taking
 * every scale-th pixel; on high-resolution screens 2 x 2 screen pixels are
 * drawn for each, so a scale of 2 shows the picture at full size */
static void screen_pic_render ( UInt32 sx, UInt32 sy, UInt8 *p,
				UInt16 w, UInt16 h, RGBColorType *pal,
				UInt16 ncolours, UInt8 scale ) {
  register UInt16 x, y;
  IndexedColorType c;
  UInt16 iter;
  UInt16 xscaled, yscaled;
  UInt16 cshift = 0;

  /* put the picture's colours in the system palette, from entry 100 */
  if ( g_features & F_COLOUR ) {

    cshift = 100;

    for ( iter = 0; iter < ncolours; iter++ ) {
      pal [ iter ].index = 100 + iter;
    }

    WinPalette ( winPaletteSet, cshift, ncolours, pal );
  }

  /*
   * render image data
   */

  if ( g_features & F_OS5HIGHRES ) {
    // OS5 high res screen direct draw

#if 1
    UInt16 tx, ty, ex, ey;
    UInt32 *args = MemPtrNew ( 10 * sizeof(UInt32) );

    xscaled = w / scale;
    yscaled = h / scale;

    WinSetCoordinateSystem ( kCoordinatesDouble );
    c = WinSetForeColor ( 0 );

    //WinScreenLock ( winLockDontCare );

    // set up armlet
    * ( args + 0 ) = (UInt32) sx;
    * ( args + 1 ) = (UInt32) sy;
    * ( args + 2 ) = (UInt32) xscaled;
    * ( args + 3 ) = (UInt32) yscaled;
    * ( args + 4 ) = (UInt32) p;
    * ( args + 5 ) = (UInt32) cshift;
    * ( args + 6 ) = (UInt32) scale;
    * ( args + 7 ) = (UInt32) w;
    * ( args + 8 ) = (UInt32) h;

    // try using the ARMlet
    if ( screen_native_call ( 'ARMC', 1001, args ) == 0xFFFF ) {

      // armlet failed for some reason, so resort to using 68k code

#define DRAWPIXELOS5(x,y,z) WinSetForeColor ( z ); WinDrawPixel ( x, y )

      for ( x = 0; x < xscaled; x++ ) {
	tx = (2*sx) + (2*x);
	ty = (2*sy);
	ex = x * scale;
	ey = 0;

	for ( y = 0; y < yscaled; y++ ) {

	  DRAWPIXELOS5 ( tx, ty, p [ ey + ex ] + cshift );
	  DRAWPIXELOS5 ( tx+1, ty, p [ ey + ex + 1 ] + cshift );
	  DRAWPIXELOS5 ( tx, ty+1,
			 p [ ( ( y * scale + 1 ) * w ) + ex ] + cshift );
	  DRAWPIXELOS5 ( tx+1, ty+1,
			 p [ ( ( y * scale + 1 ) * w ) + ex + 1 ] + cshift );
	  ty += 2;
	  ey += (scale * w);

	} // y

      } // x

    } // armlet?

    //WinScreenUnlock();
    MemPtrFree ( args );

    // restore
    WinSetForeColor ( c );
    WinSetCoordinateSystem ( kCoordinatesStandard );
#else
    /* fast draw into bitmap */
    BitmapType *b;
    Err error;
    UInt8 *bits;
    UInt16 span;
    UInt8 *row;

    xscaled = w / scale;
    yscaled = h / scale;

    b = BmpCreate ( w, h, 8, NULL, &error );
    bits = BmpGetBits ( b );
    BmpGetDimensions ( b, NULL, NULL, &span ); // bytes per row
    BmpSetDensity ( b, kDensityDouble );

    for ( y = 0; y < yscaled; y++ ) {
      row = bits + ( span * y );
      for ( x = 0; x < xscaled; x++ ) {
	*row = p [ ( y * scale * w ) + ( x * scale ) ] + cshift;
	row++;
      }
    }

    WinDrawBitmap ( b, sx, sy );

    BmpDelete ( b );

#endif

  } else {
    // low res OS draw

    xscaled = w / scale;
    yscaled = h / scale;

    for ( x = 0; x < xscaled; x++ ) {
      for ( y = 0; y < yscaled; y++ ) {
	c = WinSetForeColor ( p [ ( y * scale * w ) + ( x * scale ) ] + cshift );
	WinDrawPixel ( sx + x, sy + y );
	WinSetForeColor ( c );
      }
    }

  }

  return;
}

void screen_reset_scroll ( void ) {

  if ( g_scroll_up ) {
    g_scroll_up = 0;
    screen_draw ( screen_last_draw, screen_tail );
  }

  return;
}

void screen_prompt_more ( void ) {
  FormPtr frmP;

  if ( ! ( g_runflags & R_SWALLOW ) ) {
    frmP = FrmInitForm ( MoreForm );
    FrmDoDialog ( frmP );		// Display the About Box.
    FrmDeleteForm ( frmP );
  }

  return;
}

UInt8 screen_theme_backup [ UILastColorTableEntry ];

void screen_theme_setup ( colour_theme_t t ) {

  if ( g_features & F_COLOUR ) {
    RGBColorType back, front;
    UInt8 iter;

    /* first, back up existing system theme
     */
    for ( iter = 0; iter < UILastColorTableEntry; iter++ ) {
      screen_theme_backup [ iter ] = UIColorGetTableEntryIndex ( iter );
    }

    /* set up our custom theme
     */

    if ( t == ct_last ) {
      t = g_current_theme;
    } else {
      g_current_theme = t;
    }

    if ( t == ct_dos ) {
      back.r = 30; back.g = 30; back.b = 30;
      front.r = 200; front.g = 200; front.b = 200;
    } else if ( t == ct_atari_st ) {
      back.r = 230; back.g = 230; back.b = 230;
      front.r = 20; front.g = 20; front.b = 20;
    } else if ( t == ct_amiga ) {
      back.r = 30; back.g = 30; back.b = 230;
      front.r = 230; front.g = 230; front.b = 230;
    } else if ( t == ct_vic20 ) {
      back.r = 230; back.g = 230; back.b = 230;
      front.r = 30; front.g = 30; front.b = 230;
    } else if ( t == ct_black_on_white ) {
      back.r = 255; back.g = 255; back.b = 255;
      front.r = 0; front.g = 0; front.b = 0;
    }

    UIColorSetTableEntry ( UIFormFill, &back );

    UIColorSetTableEntry ( UIFieldBackground, &back );
    UIColorSetTableEntry ( UIFieldText, &front );
    UIColorSetTableEntry ( UIFieldCaret, &front );

    UIColorSetTableEntry ( UIObjectForeground, &front );
    UIColorSetTableEntry ( UIObjectFill, &back );
    UIColorSetTableEntry ( UIDialogFill, &back );

    WinPalette ( winPaletteSet, 255, 1, &front );

    WinEraseLine ( 0, 131, 160, 131 );

  } // colour

  return;
}

void screen_theme_undo ( void ) {

  if ( g_features & F_COLOUR ) {
    UInt8 iter;
    RGBColorType rgb;

    // restore palette
    WinPalette ( winPaletteSetToDefault, 0, 0, NULL );
    WinPalette ( winPaletteSetToDefault, 255, 0, NULL );

    // restore theme from backup
    for ( iter = 0; iter < UILastColorTableEntry; iter++ ) {
      WinIndexToRGB ( screen_theme_backup [ iter ], &rgb );
      //WinPalette ( winPaletteGet, screen_theme_backup [ iter ], 1, &rgb );
      UIColorSetTableEntry ( iter, &rgb );
    }

  } // colour

  return;
}
