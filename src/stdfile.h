
#ifndef h_stdfile_h
#define h_stdfile_h

#include "sections.h"

/* define if you want a alert to show each function as entered
 */
//#define DEBUGFILE 1

/* simulate stdio FILE using VFS; why not?
 */
typedef struct {
  char filename [ 34 ]; // for fun
  UInt32 size; // get file size (for read types)
  FileRef ref;
  UInt8 bufferp; // 1 for buffered input, 2 for output, 0 no buffering
  char buffer [ 255 ];
  UInt8 tail;
} FILE;
//#define FILE FileRef

#define stderr ((FILE*) 37)
#define stdin ((FILE*) 36)
#define stdout ((FILE*) 35)

#define SEEK_SET vfsOriginBeginning
#define SEEK_CUR vfsOriginCurrent
#define SEEK_END vfsOriginEnd

FILE *fopen ( signed char *filename, char *mode ) SEC_KRONOS;
Int8 fclose ( FILE *file ) SEC_KRONOS;
Int8 fseek ( FILE *file, UInt32 rel, UInt16 source ) SEC_KRONOS;
UInt32 fread ( void *buf, UInt32 size, UInt32 num, FILE *file ) SEC_KRONOS;
UInt32 fwrite ( void *buf, UInt32 size, UInt32 num, FILE *file ) SEC_KRONOS;
UInt8 save_delete ( char *filename ) SEC_KRONOS;
int fgetc ( FILE *file ) SEC_KRONOS;
int fputs ( const char *str, FILE *stream ) SEC_KRONOS;
int fputc ( UInt8 c, FILE *file ) SEC_KRONOS;
UInt32 ftell ( FILE *file ) SEC_KRONOS;
int fprintf ( FILE *file, char *fmt, ... ) SEC_KRONOS;
#define ferror(f) 0
#define feof(x) 0
long int fscanf_get_d ( FILE *f ) SEC_KRONOS;
char fscanf_get_c ( FILE *f ) SEC_KRONOS;

/* special extensions
 */
#define STDTEMPFILE "KronosTemp"

MemHandle pdb_temp_open ( char *filename ) SEC_KRONOS; // copy to RAM and open
void pdb_temp_close ( char *filename ) SEC_KRONOS;     // back to VFS and delete temp

UInt8 file_exists ( char *filename ) SEC_KRONOS;
UInt8 file_delete ( char *filename );

#endif
