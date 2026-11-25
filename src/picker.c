
#include <PalmOS.h>
#include <VfsMgr.h>
#include "KronosRsc.h"

#include "defs.h"
#include "kronos.h"
#include "sections.h"
#include "display.h"
#include "forms.h"

/* Code section declarations (see sections.h) */
static void GamesDrawList (Int16 itemNum, RectangleType *bounds, Char **itemsText) SEC_KRONOS;

static
void GamesDrawList ( Int16 itemNum, RectangleType *bounds, Char **itemsText ) {
  //char filename [ 50 ];
  char *filename = NULL;
  IndexedColorType oldbgcolour = 0;

  // set background
  if ( ( g_features & F_COLOUR ) &&
       ( itemNum & 1 ) )
  {
    oldbgcolour = WinSetBackColor ( 18 );

    /* if the bg colour isn't 0, it means the item is selected, so don't
     * muck with its colour scheme!
     */
    if ( oldbgcolour != 0 ) {
      WinSetBackColor ( oldbgcolour );
    }

    WinEraseRectangle ( bounds, 0 );
  }

  // get filename
  //palm_game_count ( itemNum + 1, filename );
  filename = dirlist_lookup ( itemNum );

  // draw filename
  WinDrawChars ( filename, StrLen ( filename ),
		 bounds -> topLeft.x, bounds -> topLeft.y );

  // draw type
  if ( is_magnetic ( filename ) ) {
    WinDrawChars ( "MS", 2, bounds -> topLeft.x + 130, bounds -> topLeft.y );
  } else if ( is_zcode ( filename ) ) {
    WinDrawChars ( "ZC", 2, bounds -> topLeft.x + 130, bounds -> topLeft.y );
  } else if ( is_scottfree ( filename ) ) {
    WinDrawChars ( "SF", 2, bounds -> topLeft.x + 130, bounds -> topLeft.y );
  } else if ( is_level9 ( filename ) ) {
    WinDrawChars ( "L9", 2, bounds -> topLeft.x + 130, bounds -> topLeft.y );
  }

  // restore background
  if ( ( g_features & F_COLOUR ) &&
       ( itemNum & 1 ) )
  {
    WinSetBackColor ( oldbgcolour );
  }

  return;
}

void PickerFormInit ( FormPtr frmP ) {
  ListType *list = (ListType*) GetObjectPtr ( PickerGamesList );
  UInt16 num = palm_game_count ( 0, NULL );

  LstSetSelection ( list, -1 );
  LstSetDrawFunction ( list, GamesDrawList );
  LstSetListChoices ( list, NULL, num );

  return;
}

void PickerFormExit ( FormPtr frmP ) {
  return;
}

Boolean PickerFormHandleEvent ( EventPtr eventP ) {
  Boolean handled = false;
  FormPtr f = FrmGetActiveForm();

  switch ( eventP -> eType ) {

  case frmOpenEvent:
    PickerFormInit( f );
    FrmDrawForm ( f );
    //FrmUpdateForm ( PickerForm, 0 );
    handled = true;
    break;

  case frmCloseEvent:
    PickerFormExit ( f );
    handled = false;
    break;

  case keyDownEvent:

    if ( eventP -> data.keyDown.chr == pageUpChr ) {
      ListType *list = (ListType*) GetObjectPtr ( PickerGamesList );
      LstScrollList ( list, winUp, LstGetVisibleItems ( list ) );
    } else if ( eventP -> data.keyDown.chr == pageDownChr ) {
      ListType *list = (ListType*) GetObjectPtr ( PickerGamesList );
      LstScrollList ( list, winDown, LstGetVisibleItems ( list ) );
    }

    handled = true;
    break;

  case ctlSelectEvent:

    switch ( eventP -> data.ctlSelect.controlID ) {

    case PickerPlayButton:
      {
	ListType *list = (ListType*) GetObjectPtr ( PickerGamesList );
	//char filename [ 50 ];
	char *filename = NULL;

	if ( LstGetSelection ( list ) < 0 ) {
	  FrmCustomAlert ( MessageAlert,
			   "Please select a game and press 'Play' again",
			   NULL, NULL );
	  break;
	}

	filename = dirlist_lookup ( LstGetSelection ( list ) );
	//palm_game_count ( LstGetSelection ( list ) + 1, filename );

	StrCopy ( g_gamename, filename );

	StrCopy ( g_savedir, filename );
	StrPrintF ( StrChr ( g_savedir, '.' ), ".dir" );

	if ( is_magnetic ( filename ) ) {
	  StrCopy ( g_gfxname, filename );
	  StrPrintF ( StrChr ( g_gfxname, '.' ), ".gfx" );
	} else {
	  StrCopy ( g_gfxname, "" );
	}

	if ( ( is_zcode ( filename ) ) &&
	     ( ! ( g_features & F_OS5HIGHRES ) ) )
	{
	  FrmCustomAlert ( MessageAlert,
			   "Some ZCode games may perform badly "
			   "on older units.",
			   NULL, NULL );
	}

	FrmGotoForm ( MainForm );
      }

      handled = true;
      break;

    } /* switch on button tap */

  default:
    break;
		
  } /* switch on event */
	
  return ( handled );
}
