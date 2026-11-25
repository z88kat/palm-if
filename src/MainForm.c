/******************************************************************************
 *
 * Copyright (c) 1999 Palm Computing, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: MainForm.c
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <VfsMgr.h>
#include "KronosRsc.h"
#include "defs.h"
#include "kronos.h"
#include "sections.h"
#include "display.h"
#include "mapper.h"
#include "forms.h"

#include "frotz.h"

/***********************************************************************
 *
 * FUNCTION:    MainFormInit
 *
 * DESCRIPTION: This routine initializes the MainForm form.
 *
 * PARAMETERS:  frm - pointer to the MainForm form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
/* Code section declarations (see sections.h) */
static void MainFormInit (FormPtr frmP) SEC_KRONOS;
static void MainFormExit (FormPtr frmP) SEC_KRONOS;
static Boolean MainFormDoCommand (UInt16 command) SEC_KRONOS;
static void MainDrawList (Int16 itemNum, RectangleType *bounds, Char **itemsText) SEC_KRONOS;

static void MainFormInit ( FormPtr frmP ) {
  UInt32 theme;

  FrmSetFocus ( frmP, FrmGetObjectIndex ( frmP, MainEntryField ) );

  if ( g_features & F_OS5HIGHRES ) {
    /* we have the choices of normal, medium and tiny font */
    FrmShowObject ( frmP, FrmGetObjectIndex ( frmP, MainTinyCheckbox ) );
    FrmShowObject ( frmP, FrmGetObjectIndex ( frmP, MainMediumCheckbox ) );
    FrmShowObject ( frmP, FrmGetObjectIndex ( frmP, MainNormalCheckbox ) );
  } else {
    /* low res device; no offerings.. normal font always */
    FrmHideObject ( frmP, FrmGetObjectIndex ( frmP, MainTinyCheckbox ) );
    FrmHideObject ( frmP, FrmGetObjectIndex ( frmP, MainNormalCheckbox ) );
    FrmHideObject ( frmP, FrmGetObjectIndex ( frmP, MainMediumCheckbox ) );
  }

  if ( is_zcode ( g_gamename ) ) {
    FrmShowObject ( frmP, FrmGetObjectIndex ( frmP, MainFullscreenCheckbox ) );
  } else {
    FrmHideObject ( frmP, FrmGetObjectIndex ( frmP, MainFullscreenCheckbox ) );
  }

  /* colour choices? */
  if ( g_features & F_COLOUR ) {

    if ( FtrGet ( CREATORID, FTR_THEME, &theme ) == 0 ) {
      screen_theme_setup ( theme );
    } else {
      // default theme
      screen_theme_setup ( ct_black_on_white );
    }

  } // colour

  return;
}

static void MainFormExit ( FormPtr frmP ) {

  // colour choices remove
  if ( g_features & F_COLOUR ) {
    FtrSet ( CREATORID, FTR_THEME, g_current_theme );
    screen_theme_undo();
  }

  return;
}

/***********************************************************************
 *
 * FUNCTION:    MainFormDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean MainFormDoCommand ( UInt16 command ) {
  Boolean handled = false;
  FormPtr frmP = FrmGetActiveForm();
  FieldPtr field;

  switch ( command ) {

  case ColourAtariST:
    handled = true;
    screen_theme_undo();
    screen_theme_setup ( ct_atari_st );
    draw_controls();
    break;

  case ColourBlackOnWhite:
    handled = true;
    screen_theme_undo();
    screen_theme_setup ( ct_black_on_white );
    draw_controls();
    break;

  case ColourCommodoreAmiga:
    handled = true;
    screen_theme_undo();
    screen_theme_setup ( ct_amiga );
    draw_controls();
    break;

  case ColourCommodoreVic20:
    handled = true;
    screen_theme_undo();
    screen_theme_setup ( ct_vic20 );
    draw_controls();
    break;

  case ColourMSDOS:
    handled = true;
    screen_theme_undo();
    screen_theme_setup ( ct_dos );
    draw_controls();
    break;

  case GameLoad:
    {
      EventType e;
      e.eType = ctlSelectEvent;
      e.data.ctlSelect.controlID = MainLoadGraphicButton;
      EvtAddEventToQueue ( &e );
    }
    handled = true;
    break;

  case GameSave:
    {
      EventType e;
      e.eType = ctlSelectEvent;
      e.data.ctlSelect.controlID = MainSaveGraphicButton;
      EvtAddEventToQueue ( &e );
    }
    handled = true;
    break;

  case GameAutomapExits:
    {
      char foo [ 500 ] = "";
      if ( mapper_dump ( status_left, foo ) ) {
	DEBUGS ( foo );
      } else {
	DEBUGS ( "No exits automapped yet." );
      }
    }
    handled = true;
    break;

  case GameAutomapView:
    FrmPopupForm ( MapForm );
    handled = true;
    break;

  case GameAddGameNotes:
    FrmPopupForm ( NoteForm );
    handled = true;
    break;

  case GameQuit:
    keyboard_push ( "quit\n" );
    screen_push_string ( "quit\n" );
    handled = true;
    break;

  case GameZoomPicture:
    {
      EventType e;
      e.eType = ctlSelectEvent;
      e.data.ctlSelect.controlID = MainPicButton;
      EvtAddEventToQueue ( &e );
    }
    handled = true;
    break;

  case MainOptionsAboutKronos:
    MenuEraseStatus(0);		// Clear the menu status from the display.
    frmP = FrmInitForm (AboutForm);

    update_field ( frmP, AboutVersionField, VERSION );

    /* update Feature debug list */
    {
      char buffer [ 100 ] = "";

      if ( g_features & F_OS5HIGHRES ) {
	StrCat ( buffer, "5" );
      }

      if ( g_features & F_COLOUR ) {
	StrCat ( buffer, "C" );
      }

      if ( g_features & F_MAPPER ) {
	StrCat ( buffer, "M" );
      }

      update_field ( frmP, AboutDetailsField, buffer );
    }

    FrmDoDialog (frmP);		// Display the About Box.
    FrmDeleteForm (frmP);
    handled = true;
    break;

  case MainOptionsQuitKronos:
    ms_stop();
    {
      EventType e;
      e.eType = appStopEvent;
      EvtAddEventToQueue ( &e );
    }
    handled = true;
    break;

  case EditFreeHeapRAM:
    {
      char buffer [ 200 ];
      StrPrintF ( buffer, "Free: %lu", query_heap_free() );
      FrmCustomAlert ( MessageAlert, buffer, NULL, NULL );

      StrPrintF ( buffer, "Compact Heap?" );

      if ( screen_tail &&
	   FrmCustomAlert ( YesNoAlert, buffer, NULL, NULL ) == YesNoYes )
      {
	MemHeapCompact ( MemPtrHeapID ( screen_tail ) );

	StrPrintF ( buffer, "Free: %lu", query_heap_free() );

	FrmCustomAlert ( MessageAlert, buffer, NULL, NULL );
      }

    }
    handled = true;
    break;

  case EditUndo:
  case EditCut:
  case EditCopy:
  case EditPaste:
  case EditSelectAll:

    field = GetFocusObjectPtr();

    if ( ! field ) {
      break;
    }

    if ( command == EditUndo )           { FldUndo ( field ); }
    else if ( command == EditCut )       { FldCut ( field ); }
    else if ( command == EditCopy )      { FldCopy ( field ); }
    else if ( command == EditPaste )     { FldPaste ( field ); }
    else if ( command == EditSelectAll ) {
      FldSetSelection ( field, 0, FldGetTextLength ( field ) );
    }

    handled = true;  /* default handler works fine */
    break;

  case EditKeyboard:
    SysKeyboardDialog ( kbdDefault );
    handled = true;
    break;

  case EditGrafitti:
    SysGraffitiReferenceDialog ( referenceDefault );
    handled = true;
    break;

  }

  return handled;
}

static
void MainDrawList ( Int16 itemNum, RectangleType *bounds, Char **itemsText ) {
  char filename [ 200 ];
  palm_save_count ( itemNum + 1, filename );
  WinDrawChars ( filename, StrLen ( filename ),
		 bounds -> topLeft.x, bounds -> topLeft.y );
}

/***********************************************************************
 *
 * FUNCTION:    MainFormHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the 
 *              "MainForm" of this application.
 *
 * PARAMETERS:  eventP  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
Boolean MainFormHandleEvent ( EventPtr eventP ) {
  Boolean handled = false;
  FormPtr frmP = FrmGetActiveForm();

  switch (eventP->eType) {

  case menuEvent:
    return MainFormDoCommand(eventP->data.menu.itemID);

  case frmOpenEvent:
    MainFormInit( frmP);
    FrmDrawForm ( frmP);
    draw_battery();
    handled = true;
    break;

  case frmCloseEvent:
    MainFormExit ( frmP );
    handled = false;
    break;

  case frmUpdateEvent:
    // To do any custom drawing here, first call FrmDrawForm(), then do your
    // drawing, and then set handled to true.
    break;

#if 0
  case sclRepeatEvent:
    // redraw screen showing current scrolled text
    {
      UInt16 beginat = eventP -> data.sclRepeat.newValue;
      line_t *begin = screen_head, *end;

      while ( beginat ) {
	begin = begin -> next;
	beginat--;
      }

      end = begin;
      beginat= query_rows_per_screen();

      while ( beginat ) {
	end = end -> next;
	beginat--;
      }

      screen_draw ( begin, screen_tail );

    }
    break;
#endif

  case penDownEvent:
    // did pen down occur over text?

    if ( ms_is_running() || g_waiting_for_key ) {
      RectangleType rect;
      char word [ 50 ];

      /* pull down menu?
       */
      rect.topLeft.x = 0;
      rect.topLeft.y = 0;
      rect.extent.x = 80;
      rect.extent.y = 10;

      if ( RctPtInRectangle ( eventP -> screenX,
			      eventP -> screenY, &rect) == true )
      {
	EventType e;
	e.eType = keyDownEvent;
	e.data.keyDown.chr = vchrMenu;
	EvtAddEventToQueue ( &e );
	//SndPlaySystemSound ( sndError );
	handled = true;
	break;
      }

      /* maybe a tap in the text region?
       */
      rect.topLeft.x = 0;
      rect.topLeft.y = 10;
      rect.extent.x = 160;
      rect.extent.y = 120;

      if ( RctPtInRectangle ( eventP -> screenX,
			      eventP -> screenY, &rect) == false )
      {
	handled = false;
	break;
      }

      MemSet ( word, sizeof(word), '\0' ); // clear

      screen_query_word_at ( screen_query_row ( eventP -> screenY ) +
			       g_scroll_up,
			     eventP -> screenX, word );

      if ( word [ 0 ] ) {
#if 0
	StrCat ( word, " " );
	FldInsert ( (FieldType*) GetObjectPtr ( MainEntryField ),
		    word, StrLen ( word ) );
	FldDrawField ( (FieldType*) GetObjectPtr ( MainEntryField ) );
#else
	ListType *list = (ListType*) GetObjectPtr ( MainPopList );
	words_popword ( relative_word_list, list, word );
#endif
      }

      handled = true;
    }
    break;

  case ctlRepeatEvent:

    switch ( eventP -> data.ctlRepeat.controlID ) {

    case MainScrollUpRepeating:
      display_scroll_up();
      break;

    case MainScrollDownRepeating:
      display_scroll_down();
      break;

    } // switch

    break;

  case ctlSelectEvent:

    switch ( eventP -> data.ctlSelect.controlID ) {

    case MainStartButton:
    {
      char *argv[1] = { "Foo" };
      RectangleType rect;

      /* clear scren so Start button is now invisible (otherwise it
       * shows scroll remnants)
       */
      rect.topLeft.x = 0;
      rect.topLeft.y = 12;
      rect.extent.x = 160;
      rect.extent.y = 90;
      WinEraseRectangle ( &rect, 0 );

      /* do we have the appropriate files handy?
       */
      if ( ! palm_file_exists ( g_gamename ) ) {
	char buffer [ 500 ];
	StrPrintF ( buffer, "Couldn't find %s in %s so can't run that game.",
		    g_gamename, VFSPATH );
	FrmCustomAlert ( MessageAlert, buffer, NULL, NULL );
	goto skipit;
      }

      if ( ( is_magnetic ( g_gamename ) ) &&
	   ( ! palm_file_exists ( g_gfxname ) ) )
      {
	char buffer [ 500 ];
	StrPrintF ( buffer, "WARNING: Couldn't find %s in %s we're running "
		    "without graphics!", g_gfxname, VFSPATH );
	FrmCustomAlert ( MessageAlert, buffer, NULL, NULL );
      }

      /* tiny fonts?
       */
      if ( g_features & F_OS5HIGHRES ) {
	/* check for small, med, or large */
	ControlType *tc = (ControlType*) GetObjectPtr ( MainTinyCheckbox );
	ControlType *mc = (ControlType*) GetObjectPtr ( MainMediumCheckbox );

	g_features &= ~F_TINYFONT;
	g_features &= ~F_MEDFONT;

	if ( CtlGetValue ( tc ) ) {
	  g_features |= F_TINYFONT;
	} else if ( CtlGetValue ( mc ) ) {
	  g_features |= F_TINYFONT;
	  g_features |= F_MEDFONT;
	}

      }

      /* init screen
       */
      if ( g_features & F_TINYFONT ) {
	screen_format ( 160, 135 );
      } else {
	screen_format ( 160, 155 );
      }

      /* any tricks to make zmachine games happier?
       */
      g_runflags &= ~R_HOLDDRAW;   // turn off end result drawing
      g_runflags &= ~R_FULLREDRAW; // turn off fullscreen redraw

      if ( is_zcode ( g_gamename ) ) {
	ControlType *tc =
	  (ControlType*) GetObjectPtr ( MainFullscreenCheckbox );

	if ( CtlGetValue ( tc ) ) {
	  g_runflags |= R_FULLREDRAW;
	  g_runflags |= R_HOLDDRAW;
	}

      }

      /* reset Kronos
       */
      kronos_reset();

      /* try loading automap
       */
      if ( g_features & F_MAPPER ) {
	mapper_load ( MAPFILE );
      }

      /* any autosave to pick up?
       */
      if ( file_exists ( AUTOSAVEFILE ) ) {
	char buffer [ 150 ] =
	  "An autosave has been found; would you like to load it now? "
	  "(You can load it manually later if you like.)";

	if ( FrmCustomAlert ( YesNoAlert, buffer, NULL, NULL ) == YesNoYes ) {

	  // a note for the user
	  screen_push_string ( "Loading autosave ...\n" );
	  ms_flush();

	  // turn off the display
	  g_runflags |= R_SWALLOW;

	  // enter the load commands
	  force_auto_load();

	}

	// We could do a if ( force_auto_load() ) here and
	// then file_delete() the auto-save-file; but to what end?
	// Might as well leave it in place in case of unit crash.
	// The autosave will write over it anyway, so theres no harm.

      }

      /* fire up the emu and set up the new event loop. If an interpreter
       * gives up it calls exit(), which comes back here with a non-zero
       * ErrSetJump result (see adapter.c); release what the game held.
       */
      g_exit_armed = 1;
      if ( ErrSetJump ( g_exit_jump ) != 0 ) {

	if ( is_zcode ( g_gamename ) ) {
	  reset_memory ();
	  os_reset_screen ();
	} else if ( is_magnetic ( g_gamename ) ) {
	  ms_freemem ();
	} else if ( is_level9 ( g_gamename ) ) {
	  l9_cleanup ();
	}

      } else if ( is_zcode ( g_gamename ) ) {
	/* zmachine */

	story_name = g_gamename;

	os_init_setup();
	init_opcodes ();
	/**/init_buffer ();
	/**/init_err ();
	init_memory ();
	/**/init_process ();
	/**/init_sound ();
	os_init_screen ();
	init_undo ();

	z_restart ();

	interpret ();

	reset_memory ();
	os_reset_screen ();

      } else if ( is_magnetic ( g_gamename ) ) {
	/* magnetic scrolls */

	/* any pre-keyboard-buffer? */
	if ( StrStr ( g_gamename, "wonder" ) ||
	     StrStr ( g_gamename, "WONDER" ) )
	{
	  // looks like Mag-Scroll's Wonderland..
	  // lets turn on graphics by default!
	  keyboard_push ( "graphics on\n" );
	  //DEBUGS("wonder");
	}

	// run them!
	emu_main ( 1, argv );

      } else if ( is_scottfree ( g_gamename ) ) {
	scott_main ( 1, argv );
      } else if ( is_level9 ( g_gamename ) ) {
	l9_main ();
      }
      g_exit_armed = 0;

      /* save automap
       */
      if ( g_features & F_MAPPER ) {
	mapper_save ( MAPFILE );
      }

      // wipe automap
      mapper_clear();

    skipit:

      /* we've exitted the private game event loop, so must be time to
       * leave.. bail out of the engine!
       */
#if 0
      {
	EventType e;
	e.eType = appStopEvent;
	EvtAddEventToQueue ( &e );
      }
#endif
      FrmGotoForm ( PickerForm );

    }
    handled = true;
    break;

    case MainWordsGraphicButton:
      handled = true;
      {
	ListType *list = (ListType*) GetObjectPtr ( MainPopList );
	words_popword ( standalone_word_list, list, "" );
      }
      break;

    case MainNavigateGraphicButton:
      handled = true;
      {
	ListType *list = (ListType*) GetObjectPtr ( MainPopList );
	words_popword ( navigation_word_list, list, "" );
      }
      break;

    case MainLoadGraphicButton:
      handled = true;
      {
	Int16 choices = palm_save_count ( 0, NULL );

	if ( choices > 0 ) {
	  ListType *list = (ListType*) GetObjectPtr ( MainPopList );
	  LstSetSelection ( list, -1 );
	  LstSetListChoices ( list, NULL, choices );
	  LstSetDrawFunction ( list, MainDrawList );
	  LstSetPosition ( list, 80, 103 ); // bottom right
	  LstSetHeight ( list, 5 );
	  choices = LstPopupList ( list );
	  if ( choices > -1 ) {
	    char filename [ 40 ];
	    palm_save_count ( choices + 1, filename );

	    if ( is_magnetic ( g_gamename ) ) {
	      keyboard_push ( "load\n" );
	    } else if ( is_scottfree ( g_gamename ) ) {
	      keyboard_push ( "load\n" );
	    } else if ( is_zcode ( g_gamename ) ) {
	      keyboard_push ( "restore\n" );
	    } else if ( is_level9 ( g_gamename ) ) {
	      // the interpreter's own restore: the games' asks "Really
	      // restore?" and, in some, a question from the manual
	      keyboard_push ( "#restore\n" );
	    }

	    keyboard_push ( filename );
	    keyboard_push ( "\n" );
	  }
	} else {
	  FrmCustomAlert ( MessageAlert,
			   "No save files found to load.", NULL, NULL );
	}

      }
      break;

    case MainSaveGraphicButton:
      keyboard_push ( "save\n" );
      update_field ( frmP, MainEntryField, g_last_loaded_file );
      FldDrawField ( (FieldType*) GetObjectPtr ( MainEntryField ) );
      handled = true;
      break;

    case MainLGraphicButton:
      handled = true;
      keyboard_push ( "Look\n" ); // L works with M-S and ZC, but not with SF
      screen_push_string ( "Look" );
      break;

    case MainNGraphicButton:
    case MainSGraphicButton:
    case MainEGraphicButton:
    case MainWGraphicButton:
    case MainIGraphicButton:
      handled = true;
      {
	char dir = 0;
	char cmd [ 5 ];
	switch ( eventP -> data.ctlSelect.controlID ) {
	case MainNGraphicButton: dir = 'N'; break;
	case MainSGraphicButton: dir = 'S'; break;
	case MainEGraphicButton: dir = 'E'; break;
	case MainWGraphicButton: dir = 'W'; break;
	case MainIGraphicButton: dir = 'I'; break;
	}
	StrPrintF ( cmd, "%c\n", dir );
	keyboard_push ( cmd );
	screen_push ( cmd [ 0 ] );
     }
     break;

    case MainPicButton:
      handled = true;
      if ( is_magnetic ( g_gamename ) ||
	   ( is_level9 ( g_gamename ) && l9_has_picture() ) )
      {
	FrmPopupForm ( PicForm );
      } else {
	FrmCustomAlert ( MessageAlert,
			 "This game has no artwork.", NULL, NULL );
      }
      break;

    case MainClearGraphicButton:
      update_field ( frmP, MainEntryField, "" );
      FldDrawField ( (FieldType*) GetObjectPtr ( MainEntryField ) );
      handled = true;
      break;

    case MainCRLFGraphicButton:
    {
      char *text = NULL;

      // fetch text
      text = FldGetTextPtr ( (FieldType*) GetObjectPtr ( MainEntryField ) );

      if ( text ) {

	// push into emu buffer
	keyboard_push ( text );
	keyboard_push ( "\n" );

	// remove text from entry field
	update_field ( frmP, MainEntryField, "" );
	FldDrawField ( (FieldType*) GetObjectPtr ( MainEntryField ) );

      } else {
	// no text! just hit return
	keyboard_push ( "\n" );
      }

      // make sure battery stays halfway up to date
      draw_battery();

    }
    handled = true;
    break;

    } // switch

    break;

  case keyDownEvent:
    
    if ( ( eventP -> data.keyDown.chr == chrCarriageReturn ) ||
	 ( eventP -> data.keyDown.chr == chrLineFeed ) )
    {
      // send a button press; keeps the code simple
      EventType e;
      e.eType = ctlSelectEvent;
      e.data.ctlSelect.controlID = MainCRLFGraphicButton;
      EvtAddEventToQueue ( &e );

      handled = true;

    } else if ( eventP -> data.keyDown.chr == pageUpChr ) {

      if ( g_features & F_OS5HIGHRES ) {
	EventType e;
	e.eType = ctlRepeatEvent;
	e.data.ctlRepeat.controlID = MainScrollUpRepeating;
	EvtAddEventToQueue ( &e );
	EvtAddEventToQueue ( &e );
	EvtAddEventToQueue ( &e );
	EvtAddEventToQueue ( &e );
	EvtAddEventToQueue ( &e );
	EvtAddEventToQueue ( &e );
      } else {
	// pre-OS5 could crash with Repeater events.. OS bug :/
	display_scroll_up();
	display_scroll_up();
	display_scroll_up();
	display_scroll_up();
	display_scroll_up();
	display_scroll_up();
	display_scroll_up();
      }

      handled = false;

    } else if ( eventP -> data.keyDown.chr == pageDownChr ) {

      if ( g_features & F_OS5HIGHRES ) {
	EventType e;
	e.eType = ctlRepeatEvent;
	e.data.ctlRepeat.controlID = MainScrollDownRepeating;
	EvtAddEventToQueue ( &e );
	EvtAddEventToQueue ( &e );
	EvtAddEventToQueue ( &e );
	EvtAddEventToQueue ( &e );
	EvtAddEventToQueue ( &e );
	EvtAddEventToQueue ( &e );
      } else {
	// pre-OS5 could crash with Repeater events.. OS bug :/
	display_scroll_down();
	display_scroll_down();
	display_scroll_down();
	display_scroll_down();
	display_scroll_down();
	display_scroll_down();
      }

      handled = false;
    }

    break;

  default:
    break;
		
  }
	
  return handled;
}

void display_scroll_up ( void ) {
  RectangleType rectwaste;
  RectangleType rect;
  line_t *top;
  UInt8 screenrows;

  if ( g_features & F_MEDFONT ) {
    screenrows = 20;
  } else if ( g_features & F_TINYFONT ) {
    screenrows = 23;
  } else {
    screenrows = 12;
  }

  // scroll
  g_scroll_up ++;

  // find new top line
  top = screen_query_at ( screenrows + g_scroll_up );

  if ( ! top ) {
    g_scroll_up --;
    return;
  }

  // scroll area
  rect.topLeft.x = 0;
  rect.topLeft.y = 11;
  rect.extent.x = 160;
  rect.extent.y = 120;

  WinScrollRectangle ( &rect, winDown, g_row_height, &rectwaste );
  WinEraseRectangle ( &rectwaste, 0 );

  // render it
  if ( g_features & F_MEDFONT ) {
    FntSetFont ( largeFont );
  }

  screen_draw_row ( top, 0, 11 );

  FntSetFont ( stdFont );

  return;
}

void display_scroll_down ( void ) {
  RectangleType rectwaste;
  RectangleType rect;
  line_t *top;
  UInt8 screenrows;

  // scroll
  if ( g_scroll_up == 0 ) {
    return;
  }

  g_scroll_up --;

  // find new top line
  top = screen_query_at ( g_scroll_up + 1 );

  // scroll area
  rect.topLeft.x = 0;
  rect.topLeft.y = 11;
  rect.extent.x = 160;
  rect.extent.y = 120;

  WinScrollRectangle ( &rect, winUp, g_row_height, &rectwaste );
  WinEraseRectangle ( &rectwaste, 0 );

  // render it
  if ( g_features & F_MEDFONT ) {
    FntSetFont ( largeFont );
  }

  if ( g_features & F_MEDFONT ) {
    screenrows = 20;
  } else if ( g_features & F_TINYFONT ) {
    screenrows = 23;
  } else {
    screenrows = 12;
  }

  screen_draw_row ( top, 0,
		    10 + ( ( screenrows - 1 ) * g_row_height) );

  FntSetFont ( stdFont );

  return;
}
