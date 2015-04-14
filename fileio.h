/*
** fileio.h
**
** FAT16 file I/O support
*/
#include "sdmmc.h"

extern char FError; 	        // mailbox for error reporting

// FILEIO ERROR CODES
#define FE_IDE_ERROR        1   // IDE command execution error
#define FE_NOT_PRESENT      2   // CARD not present
#define FE_PARTITION_TYPE   3   // WRONG partition type
#define FE_INVALID_MBR      4   // MBR sector invalid signtr 
#define FE_INVALID_BR       5   // Boot Record invalid signtr 
#define FE_MEDIA_NOT_MNTD   6   // Media not mounted
#define FE_FILE_NOT_FOUND   7   // File not found,open for read
#define FE_INVALID_FILE     8   // File not open
#define FE_FAT_EOF          9   // attempt to read beyond EOF
#define FE_EOF             10   // Reached the end of file
#define FE_INVALID_CLUSTER 11   // Invalid cluster > maxcls 
#define FE_DIR_FULL        12   // All root dir entry are taken
#define FE_MEDIA_FULL      13   // All clusters taken
#define FE_FILE_OVERWRITE  14   // A file with same name exist 
#define FE_CANNOT_INIT     15   // Cannot init the CARD
#define FE_CANNOT_READ_MBR 16   // Cannot read the MBR 
#define FE_MALLOC_FAILED   17   // Could not allocate memory
#define FE_INVALID_MODE    18   // Mode was not r.w.
#define FE_FIND_ERROR      19   // Failure during FILE search

typedef struct { 
    unsigned char cb;           // current buffer counter [0]=0,1 [1]=1,2
    LBA     fat;                // lba of FAT
    LBA     root;               // lba of root directory
    LBA     data;               // lba of the data area 
    unsigned short maxroot;     // max entries in root dir
    unsigned short maxcls;      // max clusters in partition
    unsigned short fatsize;     // number of sectors
    unsigned char fatcopy;      // number of copies
    unsigned char sxc;          // number sectors per cluster 
    } MEDIA;

extern MEDIA * D;


typedef struct {
    unsigned char * buffer;     // sector buffer
    MEDIA * mda;                // media structure pointer
    unsigned short cluster;     // first cluster
    unsigned short ccls;        // current cluster in file
    unsigned short sec;         // sector in current cluster
    unsigned short pos;         // position in current sector
    unsigned short top;         // bytes in the buffer
    long     seek;              // position in the file
    long     size;              // file size
    unsigned short time;        // last update time
    unsigned short date;        // last update date
    char     name[11];          // file name
    char     mode;              // mode 'r', 'w'
    unsigned short fpage;       // FAT page currently loaded 
    unsigned short entry;       // entry position in cur dir
    } MFILE;
    
// file attributes    
#define ATT_RO      1           // attribute read only
#define ATT_HIDE    2           // attribute hidden
#define ATT_SYS     4           //  "       system file
#define ATT_VOL     8           //  "       volume label
#define ATT_DIR     0x10        //  "       sub-directory
#define ATT_ARC     0x20        //  "       (to) archive 
#define ATT_LFN     0x0f        // mask for Long File Name 

#define FOUND       2           // directory entry match
#define NOT_FOUND   1           // directory entry not found


// macros to extract words and longs from a byte array 
// watch out, a processor trap will be generated if the address 
//    is not word aligned 
#define ReadW( a, f) *(unsigned short*)(a+f)     
#define ReadL( a, f) *(unsigned long *)(a+f)

// this is a "safe" versions of ReadW 
//    to be used on odd address fields
#define ReadOddW( a, f) (*(a+f) + ( *(a+f+1) << 8))

// prototypes
unsigned NextFAT( MFILE * fp, unsigned n);
unsigned NewFAT(  MFILE * fp);

unsigned ReadDIR( MFILE *fp, unsigned entry);
unsigned FindDIR( MFILE *fp);
unsigned NewDIR ( MFILE *fp);

MEDIA * mount( void);
void    umount( void);

MFILE *  fopenM  ( const char *name, const char *mode);
unsigned freadM  ( char * dest, unsigned count, MFILE *);
unsigned fwriteM ( void * src, unsigned count, MFILE *);
unsigned fcloseM ( MFILE *);
unsigned fseekM  ( MFILE *, unsigned);

unsigned listTYPE( char *list, int max, const char *ext );
MFILE *ffindM( const char *ext, unsigned *entry);

