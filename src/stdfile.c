#include <PalmOS.h>
#include <VfsMgr.h>

#include "KronosRsc.h"

#include "defs.h"
#include "kronos.h"
#include "sections.h"
#include "display.h"

FILE *fopen ( signed char *filename, char *mode ) {
  UInt32 voliter = palm_vfs_iter();
  char path [ 256 ];
  Err error;
  FILE *myfile = MemPtrNew ( sizeof(FILE) );

#ifdef DEBUGFILE
  DEBUGS( __FUNCTION__);
#endif

  StrCopy ( myfile -> filename, filename );
  myfile -> tail = 0;
  myfile -> ref = 0;
  myfile -> size = 0;

  if ( StrChr ( mode, 'r' ) && StrChr ( mode, 'w' ) ) {
    myfile -> bufferp = 0; // no buffering on read+write files
    DEBUGS("Internal eror: Game fopen() is confusing");
  } else if ( StrChr ( mode, 'w' ) ) {
    myfile -> bufferp = 2; // buffering single direction stream
  } else {
    myfile -> bufferp = 1; // buffering single direction stream
  }

  /* frotz uses fopen to write save files and such; magnetic and frotz
   * both use fopen for general purpose work, though, so we can't assume
   * this is to the save directory!
   */
  if ( StrStr ( filename, ".sav" ) ) {
    // looks like a save so lets stick in the save-dir

    StrPrintF ( path, "%s%s", VFSPATH, g_savedir );
    error = VFSDirCreate ( voliter, path ); // error is okay

    StrPrintF ( path, "%s%s/%s", VFSPATH, g_savedir, filename );

  } else if ( StrStr ( filename, ".txt" ) ) {
    // probably a notes.txt or script.txt or something so stuff it in game dir

    StrPrintF ( path, "%s%s", VFSPATH, g_savedir );
    error = VFSDirCreate ( voliter, path ); // error is okay

    StrPrintF ( path, "%s%s/%s", VFSPATH, g_savedir, filename );

  } else {
    // looks like general purpose work, so don't screw with path
    StrPrintF ( path, "%s%s", VFSPATH, filename );
  }

  //DEBUGS(path);

  error = VFSFileOpen ( voliter, path, vfsModeReadWrite, & (myfile->ref) );

  if ( error != errNone ) {

    if ( StrChr ( mode, 'w' ) ) {
      /* couldn't open it, but try and create it.. */
      error = VFSFileCreate ( voliter, path );

      if ( error != errNone ) {
	return ( NULL );
      }

      error = VFSFileOpen ( voliter, path, vfsModeReadWrite, & (myfile->ref) );

      if ( error != errNone ) {
	MemPtrFree ( myfile );
	return ( NULL );
      }

    } else {
      MemPtrFree ( myfile );
      return ( NULL );
    }

  }

  VFSFileSize ( myfile -> ref, & (myfile -> size) );

#ifdef DEBUGFILE
  DEBUGS( "Exit");
#endif

  return ( myfile );
}

Int8 fclose ( FILE *file ) {

#ifdef DEBUGFILE
  DEBUGS(__FUNCTION__);
#endif

  if ( file -> bufferp == 2 && file -> tail ) {
    // write buffer, so we need to empty the buffer into the stream
    UInt32 result;
    VFSFileWrite ( file -> ref, file -> tail, file -> buffer, &result );
  }

  VFSFileClose ( file -> ref );

  MemPtrFree ( file );

#ifdef DEBUGFILE
  DEBUGS( "Exit");
#endif

  return ( 0 );
}

Int8 fseek ( FILE *file, UInt32 rel, UInt16 source ) {
  Err error;

#ifdef DEBUGFILE
  DEBUGS(__FUNCTION__);
#endif

  file -> tail = 0; // should only fseeking on a read stream so we can dump it
  error = VFSFileSeek ( file -> ref, source, rel );

#ifdef DEBUGFILE
  DEBUGS( "Exit");
#endif

  return ( 0 ); // success
}

UInt32 fread ( void *dest, UInt32 size, UInt32 num, FILE *file ) {
  UInt8 *buf = dest;
  UInt32 result;
  Err error;
  UInt32 ext = size * num;

#ifdef DEBUGFILE
  DEBUGS(__FUNCTION__);
  DEBUGS(file->filename);
  DEBUGU32(num*size);
#endif

  // do we have any data in the buffer already? if so, lets first use
  // that and then pick up any extra needed
  if ( ext < file -> tail ) {
    // buffer already has all the data
    //DEBUGS("less");

    // pick up buffered bytes
    MemMove ( buf, file -> buffer, ext );

    // shuffle buffer up into place
    MemMove ( file -> buffer, file -> buffer + ext, file -> tail - ext );

    // update tail to new size
    file -> tail -= ext;

  } else {
    // buffer doesn't have enough data
    //DEBUGS("more");

    // move buffer into buf
    MemMove ( buf, file -> buffer, file -> tail ); // might be zero

    // go to end of buff
    buf += file -> tail;

    // num remaining
    ext -= file -> tail;

    // empty buffer
    file -> tail = 0; // now empty

    // pick up remaining
    error = VFSFileRead ( file -> ref, ext, buf, &result );

  }

#ifdef DEBUGFILE
  DEBUGS( "Exit");
#endif

  //return ( result );
  return ( size * num );
}

UInt32 fwrite ( void *src, UInt32 size, UInt32 num, FILE *file ) {
  UInt8 *buf = src;
  UInt32 result;

#ifdef DEBUGFILE
  DEBUGS(__FUNCTION__);
  DEBUGU32(size*num);
#endif

  // for fwrite, lets assume they're calling it since they want to write
  // a block of stuff, not just a single character; so first lets flush
  // any buffer so far, and then write out their buffer
  // (later could shove their buf into our file buffer..)
#if 0
  Err error;

  if ( file -> tail ) {
    VFSFileWrite ( file -> ref, file -> tail, file -> buffer, &result );
    file -> tail = 0;
  }

  if ( size*num ) {
    error = VFSFileWrite ( file -> ref, size*num, buf, &result );
  }
#else
  result = size*num;
  while ( result ) {
    fputc ( *buf++, file );
    result--;
  }
#endif

#ifdef DEBUGFILE
  DEBUGS( "Exit");
#endif

  //return ( result );
  return ( size * num );
}

int fgetc ( FILE *file ) {
  int c = EOF; /* returned when the file is exhausted */

#ifdef DEBUGFILE
  DEBUGS(__FUNCTION__);
#endif

  // buffer empty? If so, populate it..
  if ( file -> tail == 0 ) {
    //fread ( &c, 1, 1, file );
    UInt32 size = 255;

    if ( file -> size - ftell ( file ) < 255 ) {
      size = file -> size - ftell ( file );
    }

    fread ( file -> buffer, size, 1, file );

    file -> tail += size;
  }

  // already in the buffer?
  if ( file -> tail ) {

    // disrupts loading display
    //draw_progress_bar ( 100 * file -> tail / 255 );

    c = (UInt8) file -> buffer [ 0 ];
    MemMove ( file -> buffer, file -> buffer + 1, file -> tail - 1 );
    file -> tail -= 1;

  }

#ifdef DEBUGFILE
  DEBUGS( "Exit");
#endif

  return ( c ); // -1 on error?
}

int fputc ( UInt8 c, FILE *file ) {

#ifdef DEBUGFILE
  DEBUGS(__FUNCTION__);
#endif

  if ( file == stdout ) {
    screen_push ( c );
  } else {

    draw_progress_bar ( 100 * file -> tail / 255 );

    // buffer large enough to hold another char?
    if ( file -> tail == 255 ) {
      UInt32 result;
      VFSFileWrite ( file -> ref, file -> tail, file -> buffer, &result );
      file -> tail = 0;
      //fwrite ( &c, 1, 1, file );
    }

    // insert into buffer
    file -> buffer [ file -> tail++ ] = c;

  }

#ifdef DEBUGFILE
  DEBUGS( "Exit");
#endif

  return ( 0 );
}

UInt32 ftell ( FILE *file ) {
  UInt32 result;
  Err error;

#ifdef DEBUGFILE
  DEBUGS(__FUNCTION__);
#endif

  error = VFSFileTell ( file -> ref, &result );

  if ( file -> bufferp == 1 ) {
    result -= file -> tail;
  } else if ( file -> bufferp == 2 ) {
    result += file -> tail;
  }

#ifdef DEBUGFILE
  DEBUGS( "Exit");
#endif

  return ( result );
}

/*
 * The following are unbuffered, though they may use buffered functions above
 * to do their work; that is to say, the following are simpler to read :)
 *
 */

int fprintf ( FILE *file, char *fmt, ... ) {
  // swallow...its generally used for error output only
  return ( 0 );
}

/* Reads a decimal number like fscanf ( f, "%ld" ): skips white space
 * (including the CRs of DOS-format game files), reads an optional sign
 * and the digits, and consumes the one character that ends the number.
 * Returns 0 at end of file.
 */
long int fscanf_get_d ( FILE *f ) {
  char buf [ 12 ]; /* sign, 10 digits, NUL */
  UInt8 len = 0;
  int c;

  do {
    c = fgetc ( f );
  } while ( c == ' ' || c == '\t' || c == '\r' || c == '\n' );

  if ( c == '-' ) {
    buf [ len++ ] = c;
    c = fgetc ( f );
  }

  while ( c >= '0' && c <= '9' ) {
    if ( len < sizeof ( buf ) - 1 ) {
      buf [ len++ ] = c;
    }
    c = fgetc ( f );
  }

  buf [ len ] = '\0';

  return ( StrAToI ( buf ) );
}

char fscanf_get_c ( FILE *f ) {
  char foo;
  //fread ( &foo, sizeof(char), 1, f );
  foo = fgetc ( f );
  return ( foo );
}

int fputs ( const char *str, FILE *stream ) {

  while ( *str != '\0' ) {
    fputc ( *str, stream );
    str++;
  }

  return ( 0 );
}

static DmOpenRef opendb;

MemHandle pdb_temp_open ( char *filename ) {
  LocalID dbid;
  FILE *f;
  UInt32 size;
  MemHandle h;
  UInt16 at;
  char *p;

  // find and delte existing temp file
  dbid = DmFindDatabase ( 0, STDTEMPFILE );

  if ( dbid ) {
    DmDeleteDatabase ( 0, dbid );
  }

  // open notes file if exists
  f = fopen ( filename, "r" );

  if ( f ) {
    VFSFileSize ( f -> ref, &size );
  } else {
    size = 200;
  }

  // create new temp database
  DmCreateDatabase ( 0, STDTEMPFILE, CREATORID, 'TEMP', false );

  dbid = DmFindDatabase ( 0, STDTEMPFILE );

  opendb = DmOpenDatabase ( 0, dbid, dmModeReadWrite );

  // create rec0 to hold our record
  at = dmMaxRecordIndex;
  h = DmNewRecord ( opendb, &at, size );

  // lock
  p = MemHandleLock ( h );

  // copy content in
  if ( f ) {
    UInt16 iter = 0;

    while ( iter < size ) {
      char x = fgetc ( f );
      DmWrite ( p, iter, &x, 1 );
      iter++;
    }

  } else {
    char x = '\0';
    DmWrite ( p, 0, &x, 1 );
  }

  // close file
  if ( f ) {
    fclose ( f );
  }

  // unlock
  MemHandleUnlock ( h );

  return ( h );
}

void pdb_temp_close ( char *filename ) {
  FILE *f;
  MemHandle h;
  UInt16 size, iter;
  char *p;
  LocalID dbid;

  // open file to write back to
  f = fopen ( filename, "w" );

  // find the record with the content
  h = DmQueryRecord ( opendb, 0 );

  size = MemHandleSize ( h );

  // lock
  p = MemHandleLock ( h );

  // copy content
  for ( iter = 0; iter < size; iter++ ) {
    fputc ( *p++, f );
  }

  // unlock
  MemHandleUnlock ( h );

  // release
  DmReleaseRecord ( opendb, 0, false );

  // close
  fclose ( f );

  // close database
  DmCloseDatabase ( opendb );

  // delete database
  dbid = DmFindDatabase ( 0, STDTEMPFILE );
  DmDeleteDatabase ( 0, dbid );

  return;
}

UInt8 file_exists ( char *filename ) {
  FILE *f = fopen ( filename, "r" );

  if ( f ) {
    fclose ( f );
    return ( 1 );
  }

  return ( 0 );
}

UInt8 save_delete ( char *filename ) {
  char path [ 256 ];
  Err error;

  StrPrintF ( path, "%s%s/%s", VFSPATH, g_savedir, filename );

  error = VFSFileDelete ( palm_vfs_iter(), filename );

  if ( error == errNone ) {
    return ( 1 );
  }

  return ( 0 );
}
