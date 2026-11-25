
#define NULL (void*)0

#include <PceNativeCall.h>
#include "endianutils.h"
#include "cjtype.h"
#include "cjarmutil.h"

unsigned long NativeFunction ( const void *emulStateP,
			       void *userData68KP,
			       Call68KFuncType *call68KFuncP );

unsigned long NativeFunction ( const void *emulStateP,
			       void *userData68KP,
			       Call68KFuncType *call68KFuncP )
{
  armutil_t globs;

  /* local vars for this function
   */
  UInt16 x, y;
  UInt16 tx, ty, ex, ey;

  /* function needs these paramatres:
   * sx, sy -- coordinate to draw image at
   * xscaled, yscaled -- dimensions after scaling
   * p -- pointer into picture source data buffer
   * cshift -- index of first palette entry in real palette
   * scale -- amount scaled by
   * w, h -- width, height of source image
   */
  UInt32 *args = userData68KP; // conveniance and pre-swapped by OS
  UInt32 sx = ByteSwap32 ( *(args+0) );
  UInt32 sy = ByteSwap32 ( *(args+1) );
  UInt16 xscaled = ByteSwap32 ( *(args+2) );
  UInt16 yscaled = ByteSwap32 ( *(args+3) );
  UInt8 *p = (void*) ByteSwap32 ( *(args+4) );
  UInt8 cshift = ByteSwap32 ( *(args+5) );
  UInt8 scale = ByteSwap32 ( *(args+6) );
  UInt16 w = ByteSwap32 ( *(args+7) );
  /* args+8 holds the height, which is not needed here */

  /* build globals state so that armutils can do their thing
   */
  globs.emulStateP = emulStateP;
  globs.call68KFuncP = call68KFuncP;
  global = &globs;

  /* the following is copied right out of display.c verbatim; the above
   * sets everything needed up.
   */

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

  return ( 0 );
}
