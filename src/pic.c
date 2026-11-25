
#include <PalmOS.h>
#include <VfsMgr.h>
#include "KronosRsc.h"

#include "defs.h"
#include "kronos.h"
#include "sections.h"
#include "display.h"
#include "forms.h"


void PicFormInit ( FormPtr frmP ) {
  return;
}

void PicFormExit ( FormPtr frmP ) {
  return;
}

Boolean PicFormHandleEvent ( EventPtr eventP ) {
  Boolean handled = false;
  FormPtr f = FrmGetActiveForm();

  switch ( eventP -> eType ) {

  case frmOpenEvent:
    PicFormInit( f );
    FrmDrawForm ( f );
    FrmUpdateForm ( PicForm, 0 );
    handled = true;
    break;

  case frmUpdateEvent:
    handled = true; // don't erase me!
    // full size if it fits (320 x 208 high-resolution pixels)
    screen_draw_pic_fit ( 0, 16, 160, 104, true );
    break;

  case frmCloseEvent:
    PicFormExit ( f );
    handled = false;
    break;

  case keyDownEvent:
    
    if ( ( eventP -> data.keyDown.chr == chrCarriageReturn ) ||
	 ( eventP -> data.keyDown.chr == chrLineFeed ) )
    {
      // send a button press; keeps the code simple
      EventType e;
      e.eType = ctlSelectEvent;
      e.data.ctlSelect.controlID = PicOKButton;
      EvtAddEventToQueue ( &e );
    }

    handled = false;
    break;

  case ctlSelectEvent:

    switch ( eventP -> data.ctlSelect.controlID ) {

    case PicOKButton:
      FrmReturnToForm ( MainForm );
      handled = true;
      break;

    } /* switch on button tap */

  default:
    break;
		
  } /* switch on event */
	
  return ( handled );
}
