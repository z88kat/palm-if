
#ifndef h_cjarmutil_h
#define h_cjarmutil_h

/* these are stubs to call the Palm OS functions; no direct drawing is done
 * here, so these remain compatible through future OS version. Direct draw
 * routines are kept elsewhere.
 */

/* this sturcture contains global state; here it is used to store the OS
 * callback goods so as to avoid pushing and popping them on the stack for
 * every function call
 */
typedef struct {
  const void *emulStateP;
  Call68KFuncType *call68KFuncP;
} armutil_t;

register armutil_t *global asm ("r8");

static inline void WinDrawPixel ( UInt32 x1, UInt32 y1 ) {
  // #define sysTrapWinDrawPixel 0xA383
  UInt8 args [ 4 ];

  /* arguments are endian-swapped, and in reverse order */
  *( (UInt16*) &(args [ 2 ]) ) = ByteSwap16 ( y1 );
  *( (UInt16*) &(args [ 0 ]) ) = ByteSwap16 ( x1 );

  global ->
    call68KFuncP ( global -> emulStateP, PceNativeTrapNo(0xA383), &args, 4 );

  return;
}

static inline void WinSetForeColor ( UInt8 c ) {
  // #define sysTrapWinSetForeColor 0xA398
  UInt8 args [ 4 ];
  args [ 0 ] = c;

  global ->
    call68KFuncP ( global -> emulStateP, PceNativeTrapNo(0xA398),
		   &args, 1 /* | kPceNativeWantA0 */ );

  return;
}

#endif
