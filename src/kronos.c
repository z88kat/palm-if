
#include <PalmOS.h>
#include <VfsMgr.h>

#include "KronosRsc.h"

#include "defs.h"
#include "kronos.h"
#include "adapter.h"
#include "display.h"
#include "mapper.h"

/* You know, had I considered up front that I would be building multiple
 * interpreters into one applicatiojn, I'd have done it very differently.
 * At minimum, a struct containing function pointers to operations would
 * be nice, so I could say "game -> auto_save" instead of having a
 * bunch of if is_magnetic and if is_zcode all over the place. What
 * a sad state of affairs :)
 *
 * I'll fix later.
 *
 */

/* word lists
 * HINT: Someday load this from a...
 * o textfile? edittable from desktop then?
 * o palm database? Memopad entry? DOC file? -> Palm edittable...
 *
 * FORMAT:
 * '$' becomes the tapped on word
 * '|' becomes Return character if its at the end of the line
 */
char *relative_word_list[] = {
  "$ ",
  "$|",
  "Ask $ about ",
  "Drop $|",
  "Enter $|",
  "Examine $|",
  "Get $|",
  "Look at $|",
  "Open $|",
  "Read $|",
  NULL
};

char *standalone_word_list[] = {
  "Drop ",
  "Examine ",
  "Exits|",
  "Get ",
  "Go ",
  "Look ",
  "Open ",
  "Read ",
  "Say ",
  "Search ",
  NULL
};

char *navigation_word_list[] = {
  "Up|",
  "Down|",
  "NW|",
  "NE|",
  "SW|",
  "SE|",
  "Go ",
  "Enter ",
  NULL
};

void WordsDrawList ( Int16 itemNum, RectangleType *bounds, Char **itemsText ) {
  UInt8 newlinep = 0;
  char buffer [ 200 ] = "";

  //WinDrawChars ( itemsText [ itemNum ], StrLen ( itemsText [ itemNum ] ),
  //  bounds -> topLeft.x, bounds -> topLeft.y );

  StrCopy ( buffer, itemsText [ itemNum ] );

  if ( StrChr ( buffer, '|' ) ) {
    *StrChr ( buffer, '|' ) = '\0';
    newlinep = 1;
  } 

  WinDrawChars ( buffer, StrLen ( buffer ),
		 bounds -> topLeft.x, bounds -> topLeft.y );

  /* do we need to draw a newline image? */
  if ( newlinep ) {
    newlinep = FntCharsWidth ( buffer, StrLen ( buffer ) );
    WinDrawBitmap ( b_crlf,
		   bounds -> topLeft.x + newlinep + 3,
		   bounds -> topLeft.y );
  }

  return;
}

void words_translate ( char *tapword, char *from, char *r_to ) {
  char source [ 200 ];
  char *iter;

  MemSet ( source, sizeof(source), '\0' );

  // make sure from is edittable..
  StrNCopy ( source, from, 200 );

  /* translate $ */
  if ( ( iter = StrChr ( source, '$' ) ) ) {
    *iter = '\0'; // truncate left side
    StrCopy ( r_to, source );
    StrCat ( r_to, tapword );
    StrCat ( r_to, iter + 1 );
  } else {
    StrCopy ( r_to, source );
  }

  /* translate | */
#if 0
  if ( ( iter = StrChr ( source, '|' ) ) ) {
    *iter = '\n';
  }
#endif

  return;
}

void words_popword ( char *words[], ListType *list, char *tapword ) { 
  UInt16 num = 0;
  Int16 choice;
  FormPtr frmP = FrmGetActiveForm();
  char *xlate [ 20 ];

  /* count choices */
  while ( words [ num ] != NULL ) {
    xlate [ num ] = MemPtrNew ( StrLen ( words [ num ] ) + 50 );
    MemSet ( xlate [ num ], StrLen ( words [ num ] ) + 49, '\0' );
    words_translate ( tapword, words [ num ], xlate [ num ] );
    num++;
  }

  /* pop up list */
  LstSetPosition ( list, 60, 53 + (10*num) ); // bottom right
  LstSetHeight ( list, num < 11 ? num : 10 );
  LstSetSelection ( list, -1 );
  LstSetDrawFunction ( list, WordsDrawList );
  LstSetListChoices ( list, xlate, num );

  /* pop it up */
  choice = LstPopupList ( list );

  if ( choice < 0 ) {
    goto donezo;
  }

  /* what to do.. */
  if ( StrChr ( words [ choice ], '|' ) ) {
    char *text = NULL;
    /* theres a return, so just shove it into keyboard and clear field */

    * StrChr ( xlate [ choice ], '|' ) = '\0'; // cut out the newline

    // fetch existing text
    text = FldGetTextPtr ( (FieldType*) GetObjectPtr ( MainEntryField ) );

    if ( text ) {
      // push into emu buffer
      keyboard_push ( text );
    }

    // space at the end? if not, add a space!
    if ( ( text && text [ 0 ] ) &&
	 ( * ( StrChr ( text, '\0' ) - 1 ) != ' ' )
       )
    {
      // insert a space
      keyboard_push ( " " );
    }

    /* push into keyboard buffer */
    keyboard_push ( xlate [ choice ] );
    keyboard_push ( "\n" );
    
    /* clear the field */
    update_field ( frmP, MainEntryField, "" ); // clear!

  } else {
    /* theres no return, so put it into field */
    char *text = NULL;

    // fetch existing text
    text = FldGetTextPtr ( (FieldType*) GetObjectPtr ( MainEntryField ) );

    // space at the end? if not, add a space!
    if ( ( text ) &&
	 ( text [ 0 ] ) &&
	 ( * ( StrChr ( text, '\0' ) - 1 ) != ' ' )
       )
    {
      // insert a space
      FldInsert ( (FieldType*) GetObjectPtr ( MainEntryField ), " ", 1 );
    }

    //update_field ( frmP, MainEntryField, buffer ); // yoink!
    FldInsert ( (FieldType*) GetObjectPtr ( MainEntryField ),
		xlate [ choice ], StrLen ( xlate [ choice ] ) );

  }

  FldDrawField ( (FieldType*) GetObjectPtr ( MainEntryField ) );

 donezo:
  num--;
  while ( num ) {
    MemPtrFree ( xlate [ num ] );
    num--;
  }

  return;
}

UInt8 force_auto_save ( void ) {

  /* autosave not yet supported; could easily be hacked into scott and
   * frotz, but I don't think you can just save at arbitrary points
   * in Magnetic..
   */

  erase_controls();

  {
    char buffer [ 50 ] = "                   A U T O S A V E";
    WinDrawChars ( buffer, StrLen ( buffer ), 10, THUMB_Y + 15 );
  }

  if ( is_zcode ( g_gamename ) ) {

    keyboard_push ( "save\n" );
    keyboard_push ( AUTOSAVEFILE "\n" );

    if ( file_exists ( AUTOSAVEFILE ) ) {
      keyboard_push ( "y\n" ); // overwrite
    }

  } else if ( is_magnetic ( g_gamename ) ) {

    keyboard_push ( "save\n" );
    keyboard_push ( AUTOSAVEFILE "\n" );
    keyboard_push ( "y\n" ); // are you sure?

  } else if ( is_scottfree ( g_gamename ) ) {

    keyboard_push ( "save\n" );
    keyboard_push ( AUTOSAVEFILE "\n" );

  } else if ( is_level9 ( g_gamename ) ) {

    // the interpreter's own save, which asks no questions
    keyboard_push ( "#save\n" );
    keyboard_push ( AUTOSAVEFILE "\n" );

  }

  return ( 1 );
}

UInt8 force_auto_load ( void ) {

  if ( is_zcode ( g_gamename ) ) {
    keyboard_push ( "restore\n" );
    keyboard_push ( AUTOSAVEFILE "\n" );
    keyboard_push ( "look\n" );

  } else if ( is_magnetic ( g_gamename ) ) {

    keyboard_push ( "load\n" );
    keyboard_push ( AUTOSAVEFILE "\n" );
    keyboard_push ( "y\n" ); // are you sure?
    keyboard_push ( "look\n" );

  } else if ( is_scottfree ( g_gamename ) ) {

    keyboard_push ( "load\n" );
    keyboard_push ( AUTOSAVEFILE "\n" );
    keyboard_push ( "look\n" );

  } else if ( is_level9 ( g_gamename ) ) {

    // l9_main restores it before the game starts (see l9palm.c)
    g_l9_autoload = 1;
    keyboard_push ( "look\n" );

  }

  return ( 1 );
}

void kronos_reset ( void ) {
  keyboard_clear();
  return;
}
