/******************************************************************************
 *
 * Copyright (c) 1999 Palm Computing, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: Starter.c
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <VfsMgr.h>
#include "KronosRsc.h"

#include "kronos.h"
#include "defs.h"
#include "display.h"
#include "forms.h"
#include "map.h"
#include "note.h"


/***********************************************************************
 *
 *   Entry Points
 *
 ***********************************************************************/


/***********************************************************************
 *
 *   Internal Structures
 *
 ***********************************************************************/


/***********************************************************************
 *
 *   Global variables
 *
 ***********************************************************************/
//static Boolean HideSecretRecords;


/***********************************************************************
 *
 *   Internal Constants
 *
 ***********************************************************************/
#define appVersionNum				0x01

// Define the minimum OS version we support (2.0 for now).
#define ourMinVersion	sysMakeROMVersion(3,6,0,sysROMStageRelease,0)
#define kPalmOS10Version	sysMakeROMVersion(1,0,0,sysROMStageRelease,0)


/***********************************************************************
 *
 *   Internal Functions
 *
 ***********************************************************************/


/***********************************************************************
 *
 * FUNCTION:    RomVersionCompatible
 *
 * DESCRIPTION: This routine checks that a ROM version is meet your
 *              minimum requirement.
 *
 * PARAMETERS:  requiredVersion - minimum rom version required
 *                                (see sysFtrNumROMVersion in SystemMgr.h 
 *                                for format)
 *              launchFlags     - flags that indicate if the application 
 *                                UI is initialized.
 *
 * RETURNED:    error code or zero if rom is compatible
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags) {
  UInt32 romVersion;

  // See if we're on in minimum required version of the ROM or later.
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
  if (romVersion < requiredVersion) {
    if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
	(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
    {
      FrmAlert (RomIncompatibleAlert);
		
      // Palm OS 1.0 will continuously relaunch this app unless we switch to 
      // another safe one.
      if (romVersion <= kPalmOS10Version) {
	AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
      }
    }

    return sysErrRomIncompatible;
  }

  return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    GetObjectPtr
 *
 * DESCRIPTION: This routine returns a pointer to an object in the current
 *              form.
 *
 * PARAMETERS:  formId - id of the form to display
 *
 * RETURNED:    void *
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
void *GetObjectPtr ( UInt16 objectID ) {
  FormPtr frmP;
  frmP = FrmGetActiveForm();
  return (FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID)));
}



/***********************************************************************
 *
 * FUNCTION:    AppHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
Boolean AppHandleEvent ( EventPtr eventP ) {
  UInt16 formId;
  FormPtr frmP;

  if ( eventP -> eType == frmLoadEvent ) {
    // Load the form resource.
    formId = eventP->data.frmLoad.formID;
    frmP = FrmInitForm(formId);
    FrmSetActiveForm(frmP);

    // Set the event handler for the form.  The handler of the currently
    // active form is called by FrmHandleEvent each time is receives an
    // event.
    switch ( formId ) {

    case MainForm:
      FrmSetEventHandler ( frmP, MainFormHandleEvent );
      break;

    case PicForm:
      FrmSetEventHandler ( frmP, PicFormHandleEvent );
      break;

    case PickerForm:
      FrmSetEventHandler ( frmP, PickerFormHandleEvent );
      break;

    case MapForm:
      FrmSetEventHandler ( frmP, MapFormHandleEvent );
      break;

    case NoteForm:
      FrmSetEventHandler ( frmP, NoteFormHandleEvent );
      break;

    default:
      // ErrFatalDisplay("Invalid Form Load Event");
      break;

    }
    return true;
  }
	
  return false;
}


/***********************************************************************
 *
 * FUNCTION:    AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.  
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void AppEventLoop(void)
{
	UInt16 error;
	EventType event;

	do {
		EvtGetEvent(&event, evtWaitForever);

		if (! SysHandleEvent(&event))
			if (! MenuHandleEvent(0, &event, &error))
				if (! AppHandleEvent(&event))
					FrmDispatchEvent(&event);

	} while (event.eType != appStopEvent);
}


/***********************************************************************
 *
 * FUNCTION:     AppStart
 *
 * DESCRIPTION:  Get the current application's preferences.
 *
 * PARAMETERS:   nothing
 *
 * RETURNED:     Err value 0 if nothing went wrong
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err AppStart ( void ) {
  Err error;
  FileRef dirRef = 0;
  UInt32 voliter;

  UInt32 romVersion;
  UInt32 os350 = sysMakeROMVersion(3,5,0,sysROMStageRelease,0);
  UInt32 os300 = sysMakeROMVersion(3,0,0,sysROMStageRelease,0);
  UInt32 os400 = sysMakeROMVersion(3,9,0,sysROMStageRelease,0); /* 3.9 :/ */

  // See if we're on in minimum required version of the ROM or later.
  FtrGet ( sysFtrCreator, sysFtrNumROMVersion, &romVersion );

  if ( romVersion >= os300 /* os400 */ ) {
    UInt32 vfsMgrVersion;
    Err err;

    /* VFS Manager depends on extension manager, so we can skip checking for
     * ExMgr and go right for VFS.
     */

    err = FtrGet ( sysFileCVFSMgr, vfsFtrIDVersion, &vfsMgrVersion );

    if ( err ) {
      /* VFS Manager not installed */
      FrmCustomAlert ( MessageAlert,
		       "Kronos requires expansion media your device "
		       "does not support. Sorry!", NULL, NULL );
      return ( -1 );

    } else {

      /* "check version number of VFS Manager, if necessary". How does one
       * know what version numbers to expect, or which features are in
       * which versions :/
       */
      // if(vfsMgrVersion == expectedVFSMgrVersionNum)
      //palm_features |= pf_cards;

    } /* if err */

  } /* vfs */

  /* how many VFS we got? */
  //DEBUGU8 ( palm_vfs_count() );

  /* create our directory if we can't open it.. */
  voliter = palm_vfs_iter();

  error = VFSFileOpen ( voliter, VFSPATH, vfsModeRead, &dirRef );

  if ( error != errNone ) {
    error = VFSDirCreate ( voliter, VFSROOT );
    error = VFSDirCreate ( voliter, VFSPATH );

    error = VFSFileOpen ( voliter, VFSPATH, vfsModeRead, &dirRef );

    if ( error != errNone ) {
      FrmCustomAlert ( MessageAlert,
		       "Kronos requires expansion media for its "
		       "data files; put media in first slot.", NULL, NULL );
      return ( -1 );
    }

  }

  VFSFileClose ( dirRef );

  /* OS5 highres?
   */
  if ( romVersion >= os400 ) {
    UInt32 winvers = 0;

    /* since Tungsten W has T|T highres, and is OS4, we must check for
     * the existance of the OS5 highres API even though it is OS4
     */
    if ( ( FtrGet ( sysFtrCreator, sysFtrNumWinVersion, &winvers ) == 0 ) &&
	 ( winvers >= 4 ) )
    {
      /* high density api is available! Check if density is present */
      UInt16 density = 0;

      /* high density feature set available; is high resolution available? */
      WinGetSupportedDensity ( &density );

      if ( density == kDensityDouble ) {
	g_features |= F_OS5HIGHRES;
      } else {

	while ( density != 0 ) {

	  WinGetSupportedDensity ( &density );

	  if ( density == kDensityDouble ) {
	    g_features |= F_OS5HIGHRES;
	    break;
	  }

	} /* while */

      } /* density */

    } // no error?

  } // os400

  /* colour at all?
   */
  if ( romVersion >= os350 ) {
    /* check for colour support */
    Err err;
    Boolean bool = false;

    err = WinScreenMode ( winScreenModeGetSupportsColor,
			  NULL, NULL, NULL, &bool );

    if ( ( ! err ) &&
	 ( bool == true ) )
    {
      g_features |= F_COLOUR;
    }

  }

  /* mapper support?
   */
  //if ( romVersion >= os450 ) {
  if ( query_heap_free() > 300000 ) {
    g_features |= F_MAPPER;
    //DEBUGS("Mapper on");
  }

  /* if not colour, start up grayscale mode
   */
  if ( ! ( g_features & F_COLOUR ) ) {
    UInt32 depth = 4;
    Boolean colp = false;
    WinScreenMode ( winScreenModeSet, NULL, NULL, &depth, &colp );
  }

  // load up bitmaps
  cache_bitmaps();

  // load directory
  dirlist_build();

  /* show heap size */
  //DEBUGU32 ( MemHeapSize(0) );

  return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    AppStop
 *
 * DESCRIPTION: Save the current state of the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void AppStop ( void ) {
  /* if not colour, shut down grayscale mode
   */
  if ( ! ( g_features & F_COLOUR ) ) {
    WinScreenMode ( winScreenModeSetToDefaults, NULL, NULL, NULL, NULL );
  }

  // unload bitmaps
  dump_bitmaps();

  // Close all the open forms.
  FrmCloseAllForms ();

  return;
}


/***********************************************************************
 *
 * FUNCTION:    StarterPalmMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code. 
 *              cmdPB - pointer to a structure that is associated with the launch code. 
 *              launchFlags -  word value providing extra information about the launch.
 *
 * RETURNED:    Result of launch
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static UInt32 StarterPalmMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err error;

	error = RomVersionCompatible (ourMinVersion, launchFlags);
	if (error) return (error);

	switch ( cmd ) {

	case sysAppLaunchCmdNormalLaunch:
	  error = AppStart();
	  if (error) 
	    return error;
				
	  //FrmGotoForm ( MainForm );
	  FrmGotoForm ( PickerForm );

	  AppEventLoop();
	  AppStop();
	  break;

	default:
	  break;

	}
	
	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code. 
 *              cmdPB - pointer to a structure that is associated with the launch code. 
 *              launchFlags -  word value providing extra information about the launch.
 * RETURNED:    Result of launch
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
UInt32 PilotMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	return StarterPalmMain(cmd, cmdPBP, launchFlags);
}
