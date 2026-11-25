
#include <PalmOS.h>
#include <VfsMgr.h>
#include "KronosRsc.h"

#include "defs.h"
#include "kronos.h"
#include "display.h"
#include "note.h"

void NoteFormInit ( FormPtr frmP ) {
  MemHandle h;
  FieldType *f = (FieldType*) GetObjectPtr ( NoteNoteField );

  // set focus on field
  FrmSetFocus ( frmP, FrmGetObjectIndex ( frmP, NoteNoteField ) );

  // copy existing note file to memory database for editting
  h = pdb_temp_open ( NOTEFILE );

  // associate handle
  FldSetTextHandle ( f, h );

  return;
}

void NoteFormExit ( FormPtr frmP ) {
  FieldType *f = (FieldType*) GetObjectPtr ( NoteNoteField );

  // ditch tailing buffer crap
  FldCompactText ( f );

  // dissociate record so we can free it ourselves
  FldSetTextHandle ( f, NULL );

  // return record to VFS and destroy RAM version
  pdb_temp_close ( NOTEFILE );

  return;
}

Boolean NoteFormHandleEvent ( EventPtr eventP ) {
  Boolean handled = false;
  FormPtr frmP = FrmGetActiveForm();

  switch ( eventP -> eType ) {

  case frmOpenEvent:
    NoteFormInit( frmP );
    FrmDrawForm ( frmP );

    {
      FieldType *f = (FieldType*) GetObjectPtr ( NoteNoteField );
      UInt16 pos, height, fheight;

      // scroll to the end of the text
      FldGetScrollValues ( f, &pos, &height, &fheight );

      if ( height > fheight ) {
	FldScrollField ( f, height - fheight, winDown );
      }

      // set scrollbar
      update_scrollbar ( frmP, NoteNoteField, NoteScrollyScrollBar );

    }

    handled = true;
    break;

  case frmCloseEvent:
    NoteFormExit ( frmP );
    handled = false;
    break;

  case keyDownEvent:

    if ( eventP -> data.keyDown.chr == pageUpChr ) {
      scroll_page ( frmP, winUp, NoteNoteField );
      update_scrollbar ( frmP, NoteNoteField, NoteScrollyScrollBar );
      handled = true;
    } else if ( eventP -> data.keyDown.chr == pageDownChr ) {
      scroll_page ( frmP, winDown, NoteNoteField );
      update_scrollbar ( frmP, NoteNoteField, NoteScrollyScrollBar );
      handled = true;
    }
    break;

  case fldChangedEvent:
    update_scrollbar ( frmP, NoteNoteField, NoteScrollyScrollBar );
    handled = true;
    break;

  case sclRepeatEvent:
    scroll_note ( frmP,
		  eventP -> data.sclRepeat.newValue -
		    eventP -> data.sclRepeat.value,
		  false, NoteNoteField );
    update_scrollbar ( frmP, NoteNoteField, NoteScrollyScrollBar );
    break;

  case ctlSelectEvent:

    switch ( eventP -> data.ctlSelect.controlID ) {

    case NoteOKButton:
      NoteFormExit ( frmP );
      FrmReturnToForm ( MainForm );
      handled = true;
      break;

    } /* switch on button tap */

  default:
    break;
		
  } /* switch on event */
	
  return ( handled );
}

void update_scrollbar ( FormPtr f, UInt16 fld, UInt16 scrollbar ) {
  ScrollBarPtr scroll;
  FieldPtr field;
  UInt16 currentPosition;
  UInt16 textHeight;
  UInt16 fieldHeight;
  UInt16 maxValue;

  field = FrmGetObjectPtr ( f, FrmGetObjectIndex ( f, fld ) );
  FldGetScrollValues ( field, &currentPosition, &textHeight, &fieldHeight );

  if ( textHeight > fieldHeight ) {
    maxValue = textHeight - fieldHeight;
  } else if ( currentPosition ) {
    maxValue = currentPosition;
  } else {
    maxValue = 0;
  }

  scroll = FrmGetObjectPtr ( f, FrmGetObjectIndex ( f, scrollbar ) );

  SclSetScrollBar ( scroll, currentPosition, 0, maxValue, fieldHeight - 1 );

  return;
}

void scroll_note ( FormPtr f, int num, Boolean redraw, UInt16 fld ) {
  FieldPtr field;

  field = FrmGetObjectPtr ( f, FrmGetObjectIndex ( f, fld ) );

  if ( num < 0 ) {
    FldScrollField ( field, -num, winUp );
  } else {
    FldScrollField ( field, num, winDown );
  }

  return;
}

void scroll_page ( FormPtr f, WinDirectionType direction, UInt16 fld ) {
  FieldPtr field;

  field = FrmGetObjectPtr ( f, FrmGetObjectIndex ( f, fld ) );

  if ( FldScrollable ( field, direction ) ) {
    int num = FldGetVisibleLines ( field ) - 1;

    if ( direction == winUp ) {
      num = -num;
    }

    scroll_note ( f, num, true, fld );

  }

  return;
}
