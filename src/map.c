
#include <PalmOS.h>
#include <VfsMgr.h>
#include "KronosRsc.h"

#include "defs.h"
#include "kronos.h"
#include "sections.h"
#include "display.h"
#include "mapper.h"
#include "map.h"

map_room_t *map_center = NULL;

/* Code section declarations (see sections.h) */
static void FindDrawList (Int16 itemNum, RectangleType *bounds, Char **itemsText) SEC_KRONOS;

static
void FindDrawList ( Int16 itemNum, RectangleType *bounds, Char **itemsText ) {
  char *text = find_room_by_count ( itemNum ) -> room;

  // draw filename
  WinDrawChars ( text, StrLen ( text ),
		 bounds -> topLeft.x, bounds -> topLeft.y );

  return;
}

void MapFormInit ( FormPtr frmP ) {
  map_center = find_room_by_name ( status_left );
  FrmUpdateForm ( MapForm, 999 );
  return;
}

void MapFormExit ( FormPtr frmP ) {
  return;
}

Boolean MapFormHandleEvent ( EventPtr eventP ) {
  Boolean handled = false;
  FormPtr frmP = FrmGetActiveForm();

  switch ( eventP -> eType ) {

  case frmOpenEvent:
    MapFormInit( frmP );
    FrmDrawForm ( frmP );

    if ( map_center == NULL ) {
      EventType e;

      DEBUGS ( "Current room isn't mapped yet! Try moving around a bit "
	       "first." );

      e.eType = ctlSelectEvent;
      e.data.ctlSelect.controlID = MapOKButton;
      EvtAddEventToQueue ( &e );
    }

    handled = true;
    break;

  case frmCloseEvent:
    MapFormExit ( frmP );
    handled = false;
    break;

  case frmUpdateEvent:
    handled = true;
    {
      map_room_t *r = map_center;

      if ( ! r ) {
	break;
      }

      // we've got a room match! Update fields to show room names

      update_field ( frmP, MapCenterField, r-> room );

      if ( r -> hall [ md_north ] ) {
	update_field ( frmP, MapNorthField,
		       find_room_by_id ( r -> hall [ md_north ] ) -> room );
	FrmShowObject ( frmP, FrmGetObjectIndex ( frmP, MapNorthButton ) );
      } else {
	FrmHideObject ( frmP, FrmGetObjectIndex ( frmP, MapNorthButton ) );
	update_field ( frmP, MapNorthField, "" );
      }

      if ( r -> hall [ md_south ] ) {
	update_field ( frmP, MapSouthField,
		       find_room_by_id ( r -> hall [ md_south ] ) -> room );
	FrmShowObject ( frmP, FrmGetObjectIndex ( frmP, MapSouthButton ) );
      } else {
	update_field ( frmP, MapSouthField, "" );
	FrmHideObject ( frmP, FrmGetObjectIndex ( frmP, MapSouthButton ) );
      }

      if ( r -> hall [ md_east ] ) {
	update_field ( frmP, MapEastField,
		       find_room_by_id ( r -> hall [ md_east ] ) -> room );
	FrmShowObject ( frmP, FrmGetObjectIndex ( frmP, MapEastButton ) );
      } else {
	update_field ( frmP, MapEastField, "" );
	FrmHideObject ( frmP, FrmGetObjectIndex ( frmP, MapEastButton ) );
      }

      if ( r -> hall [ md_west ] ) {
	update_field ( frmP, MapWestField,
		       find_room_by_id ( r -> hall [ md_west ] ) -> room );
	FrmShowObject ( frmP, FrmGetObjectIndex ( frmP, MapWestButton ) );
      } else {
	update_field ( frmP, MapWestField, "" );
	FrmHideObject ( frmP, FrmGetObjectIndex ( frmP, MapWestButton ) );
      }

      if ( r -> hall [ md_northeast ] ) {
	update_field
	  ( frmP, MapNortheastField,
	    find_room_by_id ( r -> hall [ md_northeast ] ) -> room );
	FrmShowObject ( frmP, FrmGetObjectIndex ( frmP, MapNortheastButton ) );
      } else {
	FrmHideObject ( frmP, FrmGetObjectIndex ( frmP, MapNortheastButton ) );
	update_field ( frmP, MapNortheastField, "" );
      }

      if ( r -> hall [ md_northwest ] ) {
	update_field
	  ( frmP, MapNorthwestField,
	    find_room_by_id ( r -> hall [ md_northwest ] ) -> room );
	FrmShowObject ( frmP, FrmGetObjectIndex ( frmP, MapNorthwestButton ) );
      } else {
	FrmHideObject ( frmP, FrmGetObjectIndex ( frmP, MapNorthwestButton ) );
	update_field ( frmP, MapNorthwestField, "" );
      }

      if ( r -> hall [ md_southwest ] ) {
	update_field
	  ( frmP, MapSouthwestField,
	    find_room_by_id ( r -> hall [ md_southwest ] ) -> room );
	FrmShowObject ( frmP, FrmGetObjectIndex ( frmP, MapSouthwestButton ) );
      } else {
	FrmHideObject ( frmP, FrmGetObjectIndex ( frmP, MapSouthwestButton ) );
	update_field ( frmP, MapSouthwestField, "" );
      }

      if ( r -> hall [ md_southeast ] ) {
	update_field
	  ( frmP, MapSoutheastField,
	    find_room_by_id ( r -> hall [ md_southeast ] ) -> room );
	FrmShowObject ( frmP, FrmGetObjectIndex ( frmP, MapSoutheastButton ) );
      } else {
	FrmHideObject ( frmP, FrmGetObjectIndex ( frmP, MapSoutheastButton ) );
	update_field ( frmP, MapSoutheastField, "" );
      }

      if ( r -> hall [ md_up ] ) {
	FrmShowObject ( frmP, FrmGetObjectIndex ( frmP, MapUpButton ) );
      } else {
	FrmHideObject ( frmP, FrmGetObjectIndex ( frmP, MapUpButton ) );
      }

      if ( r -> hall [ md_down ] ) {
	FrmShowObject ( frmP, FrmGetObjectIndex ( frmP, MapDownButton ) );
      } else {
	FrmHideObject ( frmP, FrmGetObjectIndex ( frmP, MapDownButton ) );
      }

      FrmDrawForm ( frmP );
    }
    break;

  case keyDownEvent:

    if ( ( eventP -> data.keyDown.chr == chrCarriageReturn ) ||
	 ( eventP -> data.keyDown.chr == chrLineFeed ) )
    {
      // send a button press; keeps the code simple
      EventType e;
      e.eType = ctlSelectEvent;
      e.data.ctlSelect.controlID = MapOKButton;
      EvtAddEventToQueue ( &e );
    }

    handled = true;
    break;

  case ctlSelectEvent:

    switch ( eventP -> data.ctlSelect.controlID ) {

    case MapFindPopTrigger:
      handled = true;
      {
	ListType *list = (ListType*) GetObjectPtr ( MapFindList );
	Int16 s;

	LstSetSelection ( list, -1 );
	LstSetDrawFunction ( list, FindDrawList );
	LstSetListChoices ( list, NULL, mapper_count() );

	s = LstPopupList ( list );

	if ( s >= 0 ) {
	  //char buffer [ 100 ];
	  //StrPrintF ( buffer, "Rm: '%s'", find_room_by_count ( s ) -> room );
	  //DEBUGS(buffer);
	  map_center = find_room_by_count ( s );
	  FrmUpdateForm ( MapForm, 999 );
	}

      }
      break;

    case MapUpButton:
      if ( map_center -> hall [ md_up ] ) {
	map_center = find_room_by_id ( map_center -> hall [ md_up ] );
	FrmUpdateForm ( MapForm, 999 );
      }
      handled = true;
      break;

    case MapDownButton:
      if ( map_center -> hall [ md_down ] ) {
	map_center = find_room_by_id ( map_center -> hall [ md_down ] );
	FrmUpdateForm ( MapForm, 999 );
      }
      handled = true;
      break;

    case MapWestButton:
      if ( map_center -> hall [ md_west ] ) {
	map_center = find_room_by_id ( map_center -> hall [ md_west ] );
	FrmUpdateForm ( MapForm, 999 );
      }
      handled = true;
      break;

    case MapEastButton:
      if ( map_center -> hall [ md_east ] ) {
	map_center = find_room_by_id ( map_center -> hall [ md_east ] );
	FrmUpdateForm ( MapForm, 999 );
      }
      handled = true;
      break;

    case MapNorthButton:
      if ( map_center -> hall [ md_north ] ) {
	map_center = find_room_by_id ( map_center -> hall [ md_north ] );
	FrmUpdateForm ( MapForm, 999 );
      }
      handled = true;
      break;

    case MapSouthButton:
      if ( map_center -> hall [ md_south ] ) {
	map_center = find_room_by_id ( map_center -> hall [ md_south ] );
	FrmUpdateForm ( MapForm, 999 );
      }
      handled = true;
      break;

    case MapNorthwestButton:
      if ( map_center -> hall [ md_northwest ] ) {
	map_center = find_room_by_id ( map_center -> hall [ md_northwest ] );
	FrmUpdateForm ( MapForm, 999 );
      }
      handled = true;
      break;

    case MapNortheastButton:
      if ( map_center -> hall [ md_northeast ] ) {
	map_center = find_room_by_id ( map_center -> hall [ md_northeast ] );
	FrmUpdateForm ( MapForm, 999 );
      }
      handled = true;
      break;

    case MapSoutheastButton:
      if ( map_center -> hall [ md_southeast ] ) {
	map_center = find_room_by_id ( map_center -> hall [ md_southeast ] );
	FrmUpdateForm ( MapForm, 999 );
      }
      handled = true;
      break;

    case MapSouthwestButton:
      if ( map_center -> hall [ md_southwest ] ) {
	map_center = find_room_by_id ( map_center -> hall [ md_southwest ] );
	FrmUpdateForm ( MapForm, 999 );
      }
      handled = true;
      break;

    case MapOKButton:
      FrmReturnToForm ( MainForm );
      handled = true;
      break;

    case MapRecenterButton:
      map_center = find_room_by_name ( status_left );
      FrmUpdateForm ( MapForm, 999 );
      handled = true;
      break;

    } /* switch on button tap */

  default:
    break;
		
  } /* switch on event */
	
  return ( handled );
}
