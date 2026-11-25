
#ifndef h_mapper_h
#define h_mapper_h

#include "sections.h"

enum {
  md_north = 0,
  md_south,
  md_east,
  md_west,
  md_up,
  md_down,
  md_northeast,
  md_southeast,
  md_southwest,
  md_northwest,
  md_max
};

extern char *mapdir_text [ md_max ];

#define HALLFLAG_UNKNOWN 0 /* nothing known about this direction yet */
#define HALLFLAG_COMPLEX 1 /* multiple rooms same direction :/ */
#define HALLFLAG_CLOSED  2 /* proven theres nothing in this direction */

#define ROOMNAMELEN      50

typedef struct _map_room_t {
  UInt32 roomid;
  char room [ ROOMNAMELEN ];
  UInt32 hall [ 10 ];
  UInt8 hallflag [ 10 ];
  struct _map_room_t *next;
} map_room_t;

void mapper_command ( char *text ) SEC_KRONOS;  // feed a command in
void mapper_location ( char *text ) SEC_KRONOS; // feed a location in
UInt8 mapper_dump ( char *text, char *r_buf ) SEC_KRONOS; // detail whats known of room

UInt16 mapper_count ( void ) SEC_KRONOS;        // how many known rooms in automap?

map_room_t *find_or_create_room ( char *text ) SEC_KRONOS;
map_room_t *find_room_by_id ( UInt32 id ) SEC_KRONOS;
map_room_t *find_room_by_name ( char *text ) SEC_KRONOS;
map_room_t *find_room_by_count ( UInt16 count ) SEC_KRONOS;

UInt8 mapper_save ( char *filename ) SEC_KRONOS;
UInt8 mapper_load ( char *filename ) SEC_KRONOS;

void mapper_clear ( void ) SEC_KRONOS;

#endif
