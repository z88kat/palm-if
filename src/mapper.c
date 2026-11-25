
#include <PalmOS.h>
#include <VfsMgr.h>

#include "KronosRsc.h"

#include "defs.h"
#include "kronos.h"
#include "display.h"
#include "mapper.h"

/* Holy crap, the French get 5 weeks of vacation per year, minimum!
 * North America, listen up!
 */

char *mapdir_text [ md_max ] = {
  "North", "South", "East", "West", "Up", "Down",
  "Northeast", "Southeast", "Southwest", "Northwest"
};

char last_command [ 10 ] = "";
char last_location [ 50 ] = "";
map_room_t *map = NULL;

void mapper_command ( char *text ) {

  if ( text [ 0 ] && text [ 0 ] != '\n' ) {

    // copy first few letters of command
    StrNCopy ( last_command, text, 9 );

    // convert to all lower
    StrToLower ( last_command, last_command );

    // ditch trailing newline
    if ( StrChr ( last_command, '\n' ) ) {
      * StrChr ( last_command, '\n' ) = '\0';
    }

    // turn long words into short ones
    if ( StrCompare ( last_command, "east" ) == 0 ) {
      StrPrintF ( last_command, "e" );
    } else if ( StrCompare ( last_command, "west" ) == 0 ) {
      StrPrintF ( last_command, "w" );
    } else if ( StrCompare ( last_command, "north" ) == 0 ) {
      StrPrintF ( last_command, "n" );
    } else if ( StrCompare ( last_command, "south" ) == 0 ) {
      StrPrintF ( last_command, "s" );
    } else if ( StrCompare ( last_command, "up" ) == 0 ) {
      StrPrintF ( last_command, "u" );
    } else if ( StrCompare ( last_command, "down" ) == 0 ) {
      StrPrintF ( last_command, "d" );
    } else if ( StrCompare ( last_command, "northeast" ) == 0 ) {
      StrPrintF ( last_command, "ne" );
    } else if ( StrCompare ( last_command, "northwest" ) == 0 ) {
      StrPrintF ( last_command, "nw" );
    } else if ( StrCompare ( last_command, "southeast" ) == 0 ) {
      StrPrintF ( last_command, "se" );
    } else if ( StrCompare ( last_command, "southwest" ) == 0 ) {
      StrPrintF ( last_command, "sw" );
    }

    //DEBUGS(last_command);
  }

  return;
}

void mapper_location ( char *text ) {
  UInt8 mapdir = md_max;
  map_room_t *room_from, *room_to;

  /* early bail?
   */
  if ( ! ( g_features & F_MAPPER ) ) {
    return;
  }

  /* if it goes to same named location, is there a way we can tell if
   * its the very same room, or a room with the same name?
   *
   * TO BE DECIDED!
   *
   */

  /* map last-command to a cardinal direction; if no match, we bail
   */
  if ( StrLen ( last_command ) == 1 ) {

    // turn command into cardinal direction
    switch ( last_command [ 0 ] ) {
    case 'n': mapdir = md_north; break;
    case 's': mapdir = md_south; break;
    case 'e': mapdir = md_east; break;
    case 'w': mapdir = md_west; break;
    case 'u': mapdir = md_up; break;
    case 'd': mapdir = md_down; break;
    }

  } else {

    if ( StrCompare ( last_command, "ne" ) == 0 ) {
      mapdir = md_northeast;
    } else if ( StrCompare ( last_command, "nw" ) == 0 ) {
      mapdir = md_northwest;
    } else if ( StrCompare ( last_command, "se" ) == 0 ) {
      mapdir = md_southeast;
    } else if ( StrCompare ( last_command, "sw" ) == 0 ) {
      mapdir = md_southwest;
    }

  }

  if ( mapdir == md_max ) {
    // no movement command match
    return;
  }

#if 0 /* report */
  char buffer [ 100 ];
  StrPrintF ( buffer, "Moved %s from %s to %s",
	      last_command, last_location, text );
  DEBUGS(buffer);
#endif

  // bail if last_location isn't known yet, like when we just started a game
  if ( last_location [ 0 ] == '\0' ) {
    StrNCopy ( last_location, text, 49 );
    return; // we'll catch it as the player moves around
  }

  // find or create source room
  room_from = find_or_create_room ( last_location );

  // find or create target room
  room_to = find_or_create_room ( text );

  // link to self?
  if ( room_from == room_to ) {
    // bail
    return;
  }

  // link source to target
  if ( room_from -> hall [ mapdir ] ) {

    // already a connection in this direction.. lets see if its to the
    // same place or a different place (ugh!)
    if ( room_to == find_room_by_id ( room_from -> hall [ mapdir ] ) ) {
      // same place, so we needn't re-connect it
    } else {
      // a different place.. crap!
      room_from -> hallflag [ mapdir ] |= HALLFLAG_COMPLEX;
    }

  } else {
    // a new connection!
    room_from -> hall [ mapdir ] = room_to -> roomid;
  }

  StrNCopy ( last_location, text, 49 );

  return;
}

map_room_t *find_or_create_room ( char *text ) {
  map_room_t *room = map;

  while ( room ) {

    if ( StrCompare ( room -> room, text ) == 0 ) {
      return ( room );
    }

    room = room -> next;
  }

  room = MemPtrNew ( sizeof(map_room_t) );
  MemSet ( room, sizeof(map_room_t), '\0' );

  room -> roomid = TimGetSeconds() + SysRandom ( 0 );
  while ( find_room_by_id ( room -> roomid ) ) {
    room -> roomid = TimGetSeconds() + SysRandom ( 0 );
  }

  StrCopy ( room -> room, text );
  room -> next = map;
  map = room;

  return ( room );
}

map_room_t *find_room_by_id ( UInt32 id ) {
  map_room_t *r = map;

  while ( r ) {
    if ( r -> roomid == id ) {
      return ( r );
    }
    r = r -> next;
  }

  return ( r );
}

map_room_t *find_room_by_name ( char *text ) {
  map_room_t *r = map;

  while ( r ) {
    if ( StrCompare ( r -> room, text ) == 0 ) {
      return ( r );
    }
    r = r -> next;
  }

  return ( r );
}

map_room_t *find_room_by_count ( UInt16 count ) {
  map_room_t *r = map;

  while ( r && count ) {
    r = r -> next;
    count--;
  }

  return ( r );
}

UInt8 mapper_dump ( char *text, char *r_buf ) {
  UInt8 mapdir;
  map_room_t *r = find_room_by_name ( text );
  map_room_t *t;

  if ( ! r ) {
    return ( 0 );
  }

  for ( mapdir = 0; mapdir < md_max; mapdir++ ) {

    if ( r -> hall [ mapdir ] ) {
      t = find_room_by_id ( r -> hall [ mapdir ] );

      //StrCat ( r_buf, r -> room );
      StrCat ( r_buf, "- " );
      StrCat ( r_buf, mapdir_text [ mapdir ] );
      StrCat ( r_buf, " to '" );
      StrCat ( r_buf, t -> room );
      StrCat ( r_buf, "'\n" );

    } // if

  } // for

  return ( 1 );
}

UInt8 mapper_save ( char *filename ) {
  FILE *f;
  map_room_t *r = map;
  char buffer [ 200 ];
  UInt8 iter;
  UInt16 roomcount = 0;

  /* room count
   */
  roomcount = mapper_count();

  /* save map
   */

  f = fopen ( filename, "w" );

  if ( ! f ) {
    return ( 0 );
  }

  StrPrintF ( buffer, "%u\n", roomcount );
  fputs ( buffer, f );

  /* for each room
   */

  r = map;

  while ( r ) {

    /* write room out */
    StrPrintF ( buffer, "%lu\n", r -> roomid );
    fputs ( buffer, f );

    StrPrintF ( buffer, "%s\n", r -> room );
    fputs ( buffer, f );

    for ( iter = 0; iter < 10; iter++ ) {
      StrPrintF ( buffer, "%lu\n", r -> hall [ iter ] );
      fputs ( buffer, f );
    }

    for ( iter = 0; iter < 10; iter++ ) {
      StrPrintF ( buffer, "%u\n", r -> hallflag [ iter ] );
      fputs ( buffer, f );
    }

    StrPrintF ( buffer, "\n" );
    fputs ( buffer, f );

    r = r -> next;
  }

  fclose ( f );

  return ( 1 );
}

UInt8 mapper_load ( char *filename ) {
  FILE *f;
  UInt16 roomcount;
  UInt8 iter;
  map_room_t *r;
  char *cursor;
  char c;

  map = NULL; // reset

  f = fopen ( filename, "r" );

  if ( ! f ) {
    return ( 1 ); // success; no map!
  }

  roomcount = fscanf_get_d ( f );

  /* for each room..
   */

  while ( roomcount ) {

    // create a room
    r = find_or_create_room ( "__TEMP__" );

    /* get room id */
    r -> roomid = fscanf_get_d ( f );

    /* get room name */
    cursor = r -> room;
    MemSet ( r -> room, ROOMNAMELEN, '\0' );

    while ( ( c = fgetc ( f ) ) != '\n' ) {
      *cursor++ = c;
    }

    /* error? */
    //if ( r -> room [ 0 ] == '\0' ) {
    //  DEBUGS( "blank room name!" );
    //}

    /* get connections */
    for ( iter = 0; iter < 10; iter++ ) {
      r -> hall [ iter ] = fscanf_get_d ( f );
    }

    /* get flags */
    for ( iter = 0; iter < 10; iter++ ) {
      r -> hallflag [ iter ] = fscanf_get_d ( f );
    }

    /* hook it up */
    // done already

    // next room
    roomcount--;
  }

  fclose ( f );

  return ( 1 );
}

void mapper_clear ( void ) {
  map_room_t *next;
  map_room_t *iter = map;

  while ( iter ) {
    next = iter -> next;
    MemPtrFree ( iter );
    iter = next;
  }

  map = NULL;

  // clear state; this way if we load another game we don't corrupt it
  // with info from last game :)
  last_location [ 0 ] = '\0';

  return;
}

UInt16 mapper_count ( void ) {
  map_room_t *r = map;
  UInt16 roomcount = 0;

  /* room count
   */
  while ( r ) {
    roomcount++;
    r = r -> next;
  }

  return ( roomcount );
}
