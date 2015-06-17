/*
** fileio.c
** 
** FAT16 FILE I/O support
*/
#define DBGNOT
#include <stdlib.h>     // malloc...
#include <ctype.h>      // toupper...
#include <string.h>     // memcpy...
#include "fileio.h"     // file I/O routines

//-------------------------------------------------------------
// Master Boot Record key fields offsets
#define FO_MBR          0L   // master boot record sector LBA
#define FO_FIRST_P    0x1BE  // offset of first partition table
#define FO_FIRST_TYPE 0x1C2  // offset of first partition type
#define FO_FIRST_SECT 0x1C6  // first sector of first partition
#define FO_FIRST_SIZE 0x1CA  // number of sectors in partition
#define FO_SIGN       0x1FE  // MBR signature location (55,AA)

#define FAT_EOF       0xffff // last cluster in a file
#define FAT_MCLST     0xfff8 // max cluster value in a fat 

// Partition Boot Record key fields offsets
#define BR_SXC      0xd      // (byte) sector per cluster 
#define BR_RES      0xe      // (word) res sectors boot record
#define BR_FAT_SIZE 0x16     // (word) FAT size sectors 
#define BR_FAT_CPY  0x10     // (byte) number of FAT copies
#define BR_MAX_ROOT 0x11     // (odd word) max entries root dir

// directory entry management
#define DIR_ESIZE   32      // size of a directory entry(bytes)
#define DIR_NAME    0       // offset file name
#define DIR_EXT     8       // offset file extension
#define DIR_ATTRIB  11      // offset attribute( 00ARSHDV)
#define DIR_CTIME   14      // creation time
#define DIR_CDATE   16      // creation date
#define DIR_ADATE   18      // last access date
#define DIR_TIME    22      // offset last use time  (word)
#define DIR_DATE    24      // offset last use date  (word)
#define DIR_CLST    26      // offset first cluster FAT (word)
#define DIR_SIZE    28      // offset of file size (dword)
#define DIR_DEL     0xE5    // marker deleted entry
#define DIR_EMPTY   0       // marker last entry in directory

// global definitions
char FError;                // error mail box   
MEDIA *D;                   // mounting info for storage device

#define F_MAX 3             // max number of files open

MFILE F[1];
MEDIA disk;

unsigned char B[F_MAX][256];

//-------------------------------------------------------------
// mount    initializes a MEDIA structure for file IO access
//
// will mount only the first partition on the disk/card
MEDIA * mount( void)
{ 
    LBA  psize;      // number of sectors in partition
    LBA  firsts;     // first sector inside the first partition
    int i;
    unsigned char *buffer;
    
#ifdef DBG
    Fcount = Bcount = 0;
#endif

    // 0. init the I/Os
    InitSD();

    // 1. check if the card is in the slot 
    if (!DetectSD()) 
    {
        FError = FE_NOT_PRESENT;
        return NULL;    
    }

    // 2. initialize the card    
    if ( InitMedia())
    { 
        FError = FE_CANNOT_INIT;
        return NULL;
    }

    // 3. allocate space for a MEDIA structure 
    D = &disk;

    // 4. allocate space for a temp sector buffer
    D->cb = 0;           // take the first two half buffers [0][1]
    buffer = &B[ 0];

    // 5.  get the Master Boot Record 
    if ( !ReadSECTOR( FO_MBR, buffer))
    {
        FError = FE_CANNOT_READ_MBR;
//        free( D); free( buffer);
        return NULL;
    }
        
    // 6. check if the MBR sector is valid 
    //      verify the signature word
    if (( buffer[ FO_SIGN] != 0x55) ||
        ( buffer[ FO_SIGN +1] != 0xAA))
    {
        FError = FE_INVALID_MBR;
//        free( D); free( buffer);
        return NULL;
    }

//    Valid Master Boot Record Loaded
    
    // 7. read the number of sectors in partition
    psize = ReadL( buffer, FO_FIRST_SIZE);
    
    // 8. check if the partition type is acceptable
    i = buffer[ FO_FIRST_TYPE];
    switch ( i)
    {
        case 0x04:
        case 0x06:
        case 0x0E:
            // valid FAT16 options
            break;
        default:
            FError = FE_PARTITION_TYPE;
//            free( D); free( buffer);
            return NULL;
    } // switch
        
    // 9. get the first partition first sector -> Boot Record
    //  get the 32 bit offset to the first partition 
    firsts = ReadL( buffer, FO_FIRST_SECT); 
    // assuming FO_MBR == 0 
    
    // 10. get the sector loaded (boot record)
    if ( !ReadSECTOR( firsts, buffer))
    {
//        free( D); free( buffer);
        return NULL;
    }

    // 11. check if the boot record is valid 
    //      verify the signature word
    if (( buffer[ FO_SIGN] != 0x55) ||
        ( buffer[ FO_SIGN +1] != 0xAA))
    {
        FError = FE_INVALID_BR;
//        free( D); free( buffer);
        return NULL;
    }

// Valid Partition Boot Record Loaded       
    // get the full partition/drive layout

    // 12. determine the size of a cluster
    D->sxc = buffer[ BR_SXC];
    // this will also act as flag that the disk is mounted
    
    // 13. determine fat, root and data LBAs
    // FAT = first sector in partition 
    // (boot record) + reserved records
    D->fat = firsts + ReadW( buffer, BR_RES);
    D->fatsize = ReadW( buffer, BR_FAT_SIZE);
    D->fatcopy = buffer[ BR_FAT_CPY];
    
    // 14. ROOT = FAT + (sectors per FAT *  copies of FAT)
    D->root = D->fat + ( D->fatsize * D->fatcopy);
    
    // 15. MAX ROOT is the maximum number of entries 
    //      in the root directory
    D->maxroot = ReadOddW( buffer, BR_MAX_ROOT) ;
    
    // 16. DATA = ROOT + (MAXIMUM ROOT *32 / 512) 
    D->data = D->root + ( D->maxroot >> 4); 
    // assuming maxroot % 16 == 0!!!

    // 17. max clusters in this partition 
    //       = (tot sectors - sys sectors )/sxc
    D->maxcls = (psize - (D->data - firsts)) / D->sxc;
    
    // 18. free up the temporary buffer
//    free( buffer);
    return D;

} // mount


//-------------------------------------------------------------
// umount    initializes a MEDIA structure for file IO access
//
void umount( void)
{ 
//    free( D);
    D = NULL;
} // umount


//-------------------------------------------------------------
// 
unsigned ReadDIR( MFILE *fp, unsigned e)
// loads current entry sector in file buffer
// returns      FAIL/TRUE
{        
    LBA l;

    // load the root sector containing the DIR entry "e"
    l = fp->mda->root + (e >> 4);
    fp->fpage = -1;             // invalidate FAT cache
    
    return ( ReadSECTOR( l, fp->buffer));

} // ReadDIR


#ifdef _FWRITE_ENABLED
//-------------------------------------------------------------
// 
unsigned WriteDIR( MFILE *fp, unsigned entry)
// write current entry sector 
// returns      FAIL/TRUE
{        
    LBA l = fp->mda->root + (entry >> 4);
    // write the root sector 
    return ( WriteSECTOR( l, fp->buffer)) ;
} // WriteDIR
#endif


//-------------------------------------------------------------
// Find a File entry in current directory
//
unsigned FindDIR( MFILE *fp)
// fp       file structure
// return   found/not_found/fail 
{
    unsigned eCount;            // current entry counter
    unsigned e;                 // current entry offset 
    int i, a;             
    MEDIA *mda = fp->mda;
    
    // 1. start from the first entry
    eCount = 0;

    // load the first sector of root
    if ( !ReadDIR( fp, eCount))
        return FAIL;
            
    // 2. loop until you reach the end or find the file
    while ( 1)
    {
    // 2.0 determine the offset in current buffer
        e = (eCount&0xf) * DIR_ESIZE;

    // 2.1 read the first char of the file name 
        a = fp->buffer[ e + DIR_NAME]; 

    // 2.2 terminate if it is empty (end of the list)        
        if ( a == DIR_EMPTY)
        {
            return NOT_FOUND;
        } // empty entry

    // 2.3 skip erased entries if looking for a match
        if ( a != DIR_DEL)
        {
        // 2.3.1 if not VOLume or DIR compare the names
            a = fp->buffer[ e + DIR_ATTRIB];
            if ( !(a & (ATT_DIR | ATT_HIDE)) )
            {   
                // compare file name and extension
                for (i=DIR_NAME; i<DIR_ATTRIB; i++)
                {
                   if ( fp->buffer[ e + i] !=  fp->name[i])
                        break;  // difference found
                }

                if ( i == DIR_ATTRIB)
                {
                  // entry found, fill the file structure 
                  fp->entry = eCount;         // store index
                  fp->time = ReadW( fp->buffer, e+DIR_TIME);
                  fp->date = ReadW( fp->buffer, e+DIR_DATE);
                  fp->size = ReadL( fp->buffer, e+DIR_SIZE);
                  fp->cluster = ReadL( fp->buffer, e+DIR_CLST);
                  return FOUND;      
                }
            } // not dir nor vol
        } // not deleted
        
    // 2.4 get the next entry
        eCount++;
        if ( (eCount & 0xf) == 0)
        {   // load a new sector from the Dir
            if ( !ReadDIR( fp, eCount))
                return FAIL; 
        }

    //  2.5. exit the loop if reached the end or error
        if ( eCount >= mda->maxroot)
            return NOT_FOUND;       // last entry reached
    
    }// while

    return NOT_FOUND;
    
} // FindDIR


//-------------------------------------------------------------
// Read the content of cluster link in FAT
//
// return : 0000 cluster is empty
//          0FF8 last cluster in a chain
//          0xxx next cluster in a chain
//          FFFF FAIL

unsigned ReadFAT( MFILE *fp, unsigned ccls)
// mda      disk structure
// ccls     current cluster 
// return   next cluster value, 
//          0xffff if failed or last 
{
    unsigned p, c;
    LBA l;
        
    // get page of current cluster in fat
    p = ccls >>8;               // 256 clusters per sector  

    // check if already cached
    if (fp->fpage != p)
    {   
        // load the fat sector containing the cluster
        l = fp->mda->fat + p;   
        if ( !ReadSECTOR( l, fp->buffer))
            return FAT_EOF;      // failed
        
        // note the sector contains a valid FAT page cache 
        fp->fpage = ccls>>8;
    }
        
    // get the next cluster value
    // cluster = 0xabcd
    // packed as:     0   |   1    |   2   |  3    |
    // word p       0   1 |  2   3 | 4   5 | 6   7 |..
    //              cd  ab|  cd  ab| cd  ab| cd  ab| 
    c = ReadOddW( fp->buffer, ((ccls & 0xFF)<<1));
    
    return c;
     
} // ReadFAT


//-------------------------------------------------------------
// Get Next Cluster
//
unsigned NextFAT( MFILE * fp, unsigned n)
// fp   file structure
// n    number of links in FAT cluster chain to jump through
//      n==1, next cluster in the chain
{
    unsigned    c;
    MEDIA * mda = fp->mda;

    // loop n times
    do {
        // get the next cluster link from FAT
        c = ReadFAT( fp, fp->ccls);

        // compare against max value of a cluster in FATxx
        // return if eof
        if ( c >= FAT_MCLST)    // check against eof
        {
            FError = FE_FAT_EOF;
            return FAIL;    // seeking beyond EOF
        }

        // check if cluster value is valid
        if ( c >= mda->maxcls)
        {
            FError = FE_INVALID_CLUSTER;
            return FAIL;
        }

    } while (--n > 0);// loop end

    // update the MFILE structure
    fp->ccls = c;

    return TRUE;
} // NextFAT


#ifdef _WRITE_ENABLED
//-------------------------------------------------------------
// Find a New entry in root directory or an empty entry
//
unsigned NewDIR( MFILE *fp)
// fp       file structure
// return   found/fail,  fp->entry filled
{
    unsigned eCount;            // current entry counter
    unsigned e;                 // current entry offset
    int a;
    MEDIA *mda = fp->mda;

    // 1. start from the first entry
    eCount = 0;
    // load the first sector of root
    if ( !ReadDIR( fp, eCount))
        return FAIL;

    // 2. loop until you reach the end or find the file
    while ( 1)
    {
    // 2.0 determine the offset in current buffer
        e = (eCount&0xf) * DIR_ESIZE;

    // 2.1 read the first char of the file name
        a = fp->buffer[ e + DIR_NAME];

    // 2.2 terminate if it is empty (end of the list)or deleted
        if (( a == DIR_EMPTY) ||( a == DIR_DEL))
        {
            fp->entry = eCount;
            return FOUND;
        } // empty or deleted entry found


    // 2.3 get the next entry
        eCount++;
        if ( (eCount & 0xf) == 0)
        { // load a new sector from the root
            if ( !ReadDIR( fp, eCount))
                return FAIL;
        }

    // 2.4 exit the loop if reached the end or error
        if ( eCount > mda->maxroot)
            return NOT_FOUND;       // last entry reached
    }// while

    return FAIL;
} // NewDIR

//-------------------------------------------------------------
// Write a cluster link in all FAT copies
//
unsigned WriteFAT( MFILE *fp, unsigned cls, unsigned v)
// mda      disk structure
// cls      current cluster 
// v        next value
// return   TRUE if successful, or FAIL
{
    int i;
    unsigned p;
    LBA l;
    
    // load the FAT page containing the requested cluster
    ReadFAT( fp, cls);
    
    // cluster = 0xabcd
    // packed as:     0   |   1    |   2   |  3    |
    // word p       0   1 |  2   3 | 4   5 | 6   7 |..
    //              cd  ab|  cd  ab| cd  ab| cd  ab| 
       
    // locate the cluster within the FAT page
    p = (cls & 0xff)*2;    // 2 bytes per cluster 

    // get the next cluster value
    fp->buffer[ p] = v;       // lsb 
    fp->buffer[ p+1] = (v>>8);// msb
    
    // update all FAT copies
    l = fp->mda->fat + (cls>>8);
    for ( i=0; i<fp->mda->fatcopy; i++, l += fp->mda->fatsize)
       if ( !WriteSECTOR( l, fp->buffer))
           return FAIL; 
    
    return TRUE;
     
} // WriteFAT


//-------------------------------------------------------------
// Allocate New Cluster
//
unsigned NewFAT( MFILE * fp)
// fp       file structure
// fp->ccls     ==0 if first cluster to be allocated
//              !=0 if additional cluster 
// return   TRUE/FAIL
//  fp->ccls new cluster number 
{
    unsigned i, c = fp->ccls;
    
    // sequentially scan through the FAT 
    //    looking for an empty cluster
    do {
        c++;    // check next cluster in FAT
        // check if reached last cluster in FAT, 
        // re-start from top
        if ( c >= fp->mda->maxcls)
            c = 0;  
            
        // check if full circle done, media full
        if ( c == fp->ccls)
        {
            FError = FE_MEDIA_FULL;
            return FAIL;
        }
            
        // look at its value 
        i = ReadFAT( fp, c);
            
    } while ( i!=0);    // scanning for an empty cluster
    
    // mark the new cluster as taken, and last in chain
    WriteFAT( fp, c, FAT_EOF);
    
    // if not first cluster, link current cluster to new one
    if ( fp->ccls >0)
        WriteFAT( fp, fp->ccls, c);
    
    // update the MFILE structure
    fp->ccls = c;
    
    // invalidate the FAT cache 
    //   (since it will soon be overwritten with data)
    fp->fpage = -1;
        
    return TRUE;
} // NewFAT


//-------------------------------------------------------------
//
unsigned WriteDATA( MFILE *fp)
{
    LBA l;

    // calculate lba of cluster/sector
    l = fp->mda->data+(LBA)(fp->ccls-2) * fp->mda->sxc+fp->sec;

    return ( WriteSECTOR( l, fp->buffer));

} // WriteDATA

#endif


//-------------------------------------------------------------
//
unsigned ReadDATA( MFILE *fp)
{
    LBA l;

    // calculate lba of cluster/sector
    l = fp->mda->data+(LBA)(fp->ccls-2) * fp->mda->sxc+fp->sec;
    fp->fpage = -1;         // invalidate FAT cache
    
    return( ReadSECTOR( l, fp->buffer));

} // ReadDATA

 
//-------------------------------------------------------------
// Open File on the default storage device D
//
MFILE *fopenM( const char *filename, const char *mode)
{
    char c;
    int i, r, e;
    unsigned char *b;       
    MFILE *fp;              

    // 1.  check if storage device is mounted 
    if ( D == NULL)       // unmounted
    {
        FError = FE_MEDIA_NOT_MNTD;
        return NULL;
    }
    
    // 2. allocate a buffer for the file
    b = &B[ D->cb];
    
    // 3. allocate a MFILE structure on the heap
    fp = &F[0];
    
    // 4. set pointers to the MEDIA structure and buffer
    fp->mda = D;
    fp->buffer = b;

    // 5. format the filename into name 
    for( i=0; i<8; i++)
    {
        // read a char and convert to upper case
        c = toupper( *filename++);      
        // extension or short name no-extension
        if (( c == '.') || ( c == '\0'))
            break;
        else 
            fp->name[i] = c;
    } // for
    // if short fill the rest up to 8 with spaces
    while ( i<8) fp->name[i++] = ' ';

    // 6. if there is an extension
    if ( c != '\0') 
    {    
        for( i=8; i<11; i++)
        {
            // read a char and convert to upper case
            c = toupper( *filename++);      
            if ( c == '.')
                c = toupper( *filename++);
            if ( c == '\0')                 // short extension
                break;
            else 
                fp->name[i] = c;
        } // for
        // if short fill the rest up to 3 with spaces
        while ( i<11) fp->name[i++] = ' ';
    } // if

    // 7. copy the file mode character  (r, w) 
    if ((*mode == 'r')||(*mode == 'w'))
        fp->mode = *mode;
    else
    { 
        FError = FE_INVALID_MODE;
        goto ExitOpen;
    }

    // 8. Search for the file in current directory
    if ( ( r = FindDIR( fp)) == FAIL)
    { 
        FError = FE_FIND_ERROR;
        goto ExitOpen;
    }
    
    // 9. init all counters to the beginning of the file
    fp->seek = 0;               // first byte in file
    fp->sec = 0;                // first sector in the cluster
    fp->pos = 0;                // first byte in sector/cluster
    
    // 10. depending on the mode (read or write) 
    if ( fp->mode == 'r') 
    {   // 10.1 'r' open for reading
        if  ( r == NOT_FOUND)
        {
            FError = FE_FILE_NOT_FOUND;
            goto ExitOpen;
        }
        else
        {  // found             
        // 10.2 set current cluster pointer on first cluster
            fp->ccls = fp->cluster;     

        // 10.3 read a sector of data from the file
            if ( !ReadDATA( fp))
            {
                goto ExitOpen;
            }
        
        // 10.4 determine how much data is really inside buffer
            if ( fp->size-fp->seek < 512)
                fp->top = fp->size - fp->seek;
            else
                fp->top = 512;
        } // found
    } // 'r' 

#ifdef _WRITE_ENABLED
    else // 11.  open for 'write'
    { 
        if  ( r == NOT_FOUND)
        {
            // 11.1 allocate a first cluster to it
            fp->ccls = 0;            // indicate brand new file
            if ( NewFAT( fp) != TRUE)
            { // must be media full 
                FError = FE_MEDIA_FULL;
                goto ExitOpen;
            }
            fp->cluster = fp->ccls; 

            // 11.2 create a new entry
            // search again, for an empty entry this time
            if ( (r = NewDIR( fp)) == FAIL)
            {   // report any error
                FError = FE_IDE_ERROR;
                goto ExitOpen;
            }
            // 11.3 new entry not found 
            if ( r == NOT_FOUND)
            { 
                FError = FE_DIR_FULL;
                goto ExitOpen;
            }
            else // 11.4 new entry identified fp->entry filled 
            {
                // 11.4.1
                fp->size = 0;

                // 11.4.2    determine offset in DIR sector
                e = (fp->entry & 0xf) * DIR_ESIZE;    
        
                // 11.4.3 init all fields to 0
                for (i=0; i<32; i++)
                        fp->buffer[ e + i]  = 0;
        
                // 11.4.4 set date and time
                fp->date = 0x378A; // Dec 10th, 2007
                fp->buffer[ e + DIR_CDATE]  = fp->date;
                fp->buffer[ e + DIR_CDATE+1]= fp->date>>8;
                fp->buffer[ e + DIR_DATE]  = fp->date;
                fp->buffer[ e + DIR_DATE+1]= fp->date>>8;

                fp->time = 0x6000; // 12:00:00 PM
                fp->buffer[ e + DIR_CTIME]  = fp->time;
                fp->buffer[ e + DIR_CTIME+1]= fp->time>>8;
                fp->buffer[ e + DIR_TIME]  = fp->time+1;
                fp->buffer[ e + DIR_TIME+1]= fp->time>>8;
        
                // 11.4.5 set first cluster
                fp->buffer[ e + DIR_CLST]  = fp->cluster;
                fp->buffer[ e + DIR_CLST+1]= (fp->cluster>>8);
                
                // 11.4.6 set name
                for ( i = 0; i<DIR_ATTRIB; i++)
                    fp->buffer[ e + i] = fp->name[i];
                
                // 11.4.7 set attrib
                fp->buffer[ e + DIR_ATTRIB] = ATT_ARC;
                
                // 11.4.8  update the directory sector;
                if ( !WriteDIR( fp, fp->entry))
                {
                    FError = FE_IDE_ERROR;
                    goto ExitOpen;
                }
            } // new entry
        } // not found

        else // file exist already, report error
        {
            FError = FE_FILE_OVERWRITE;
            goto ExitOpen;
        }
            
    } // w request
#endif
    
    // 12. Exit with success    
    return fp;

    // 13. Exit with error
ExitOpen:
//    free( fp->buffer);
//    free( fp);
    return NULL;
        
} // fopenM


//-------------------------------------------------------------
// Read File
//
// returns      number of bytes actually transferred
//
unsigned freadM( char * dest, unsigned size, MFILE *fp)
// fp       pointer to MFILE structure
// dest     pointer to destination buffer
// count    number of bytes to transfer
// returns  number of bytes actually transferred
{
    MEDIA * mda = fp->mda;      
    unsigned count=size;        // counts bytes to be transfer
    unsigned len;          
    

    // 1. check if fp points to a valid open file structure
    if (( fp->mode != 'r')) 
    {   // invalid file or not open in read mode
        FError = FE_INVALID_FILE; 
        return 0;
    }

    // 2. loop to transfer the data
    while ( count>0)
    {               

        // 2.1 check if EOF reached 
        if ( fp->seek >= fp->size)
        {
            FError = FE_EOF;  // reached the end
            break;
        }
        
        // 2.2 load a new sector if necessary
        if (fp->pos == fp->top) 
        {   
            fp->pos = 0;
            fp->sec++;
            
            // 2.2.1 get a new cluster if necessary
            if ( fp->sec == mda->sxc)
            {
                fp->sec = 0;
//                fp->ccls++;       // HACK!
                if ( !NextFAT( fp, 1))
                    break;
            }
            // 2.2.2 load a sector of data
            if ( !ReadDATA( fp))
                break;

            // 2.2.3 determine how much data is inside buffer
            if ( fp->size-fp->seek < 512)
                fp->top = fp->size - fp->seek;
            else
                fp->top = 512;
         } //  load new sector

        // 2.3 copy as many bytes as possible in a single chunk
        // take as much as fits in the current sector
        if ( fp->pos+count < fp->top)
            // fits all in current sector
            len = count;          
        else 
            // take a first chunk, there is more
            len = fp->top - fp->pos;    

        // check if dummy read
        if ( dest)
            memcpy( dest, fp->buffer + fp->pos, len);

        // 2.4 update all counters and pointers
        count-= len;            // compute what is left
        dest += len;            // advance destination pointer
        fp->pos += len;         // advance pointer in sector
        fp->seek += len;        // advance the seek pointer
        
    } // while count
    
    // 3. return number of bytes actually transferred 
    return size-count;

} // freadM


//-------------------------------------------------------------
// Close a File
//
unsigned fcloseM( MFILE *fp)
{
    unsigned r;

    r = FAIL;

#ifdef _WRITE_ENABLED
    // 1. check if it was open for write
    if ( fp->mode == 'w')   
    {
        unsigned e;
        
        // 1.1 if the current buffer contains data, flush it
        if ( fp->pos >0)
        {   
            if ( !WriteDATA( fp))
                goto ExitClose; 
        } 
        
        // 1.2      finally update the dir entry, 
        // 1.2.1    retrieve the dir sector
        if ( !ReadDIR( fp, fp->entry))
            goto ExitClose;
    
        // 1.2.2    determine position in DIR sector
        e = (fp->entry & 0xf) * DIR_ESIZE;    

        // 1.2.3 update file size
        fp->buffer[ e + DIR_SIZE]  = fp->size;
        fp->buffer[ e + DIR_SIZE+1]= fp->size>>8;
        fp->buffer[ e + DIR_SIZE+2]= fp->size>>16;
        fp->buffer[ e + DIR_SIZE+3]= fp->size>>24;

        // 1.2.4    update the directory sector;
        if ( !WriteDIR( fp, fp->entry))
            goto ExitClose;
    } // write
#endif
    
    // 2. exit with success
    r = TRUE;

//ExitClose:

    // 3. free up the buffer and the MFILE struct
//    free( fp->buffer);
//    free( fp);
    return( r);
    
} // fcloseM




//----------------------------------------------------------------------
// fseekM
//
// Advances the file pointer by count positions from CUR_POS
// simple implementation of a seek function
// returns 0 if successfull

unsigned fseekM( MFILE * fp, unsigned count)
{
    unsigned d, r;

    while ( count)
    {
        d = ( count >= 16) ? 16 : count;
        r = freadM( NULL, d, fp);
        count -= r;
        if ( r != d)
            break;      // reached end of file or error
    } // while

    return count;
} // fseekM



//-------------------------------------------------------------
// Scans the current disk and compiles a list of files 
//   with a given extension
//
unsigned listTYPE( char *list, int max, const char *ext )
// list     array of file names max * 8
// max      number of entries
// ext      file extension we are searching for
// return   number of files found
{
    unsigned eCount;        // current entry counter
    unsigned eOffs;         // current entry offset in buffer
    unsigned x, a, r;             
    MFILE *fp;
    unsigned char *b;
    
    x = 0;
    r = 0;

    // 1.  check if storage device is mounted 
    if ( D == NULL)         // unmounted
    {
        FError = FE_MEDIA_NOT_MNTD;
        return 0;
    }
    
    // 2. allocate a buffer for the file
    b = &B[D->cb];
    
    // 3. allocate a MFILE structure on the heap
    fp = &F[0];
    
    // 4. set pointers to the MEDIA structure and buffer
    fp->mda = D;
    fp->buffer = b;

    
    // 5. start from first entry, load first sector from root
    eCount = 0;
    eOffs = 0;
    if ( !ReadDIR( fp, eCount))
    {    //report error
        FError = FE_FIND_ERROR;
        goto ListExit;
    }
            
    // 6. loop until you reach the end of the root directory
    while ( TRUE)
    {    
    // 6.1 read the first char of the file name 
        a =  fp->buffer[ eOffs + DIR_NAME]; 

    // 6.2 terminate if it is empty (end of the list)        
        if ( a == DIR_EMPTY)
            break;
    // 6.3 
        if ( a != DIR_DEL)
        {
          // if not hidden, print the file name and size
          a = fp->buffer[ eOffs + DIR_ATTRIB];
          if ( !( a & (ATT_HIDE|ATT_DIR))) 
          {   //  check the file extension
            if ( !memcmp( &fp->buffer[ eOffs+DIR_EXT], ext, 3))
            {   
               // copy file name in playlist
               memcpy( &list[x*8], 
                       &fp->buffer[ eOffs+DIR_NAME], 8);
                    
               // check if maximum number of entries reached
               if ( ++x >= max)
                   break;
            }                        
          } // file
        } // not deleted or hidden
        
    // 6.4 get the next entry
    //     exit the loop if reached the end or error
        eCount++;
        if ( eCount > fp->mda->maxroot)
            break;              // last entry reached

        eOffs += 32;
        if ( eOffs >= 512)
        { 
            eOffs = 0;
            if ( !ReadDIR( fp, eCount))
            {
                FError = FE_FIND_ERROR;
                goto ListExit;
            }
        }
    }// while 
    
    // 7. return the number of files found
    r = x;

ListExit:
    // 8. free buffers and return
//    free( fp->buffer);
//    free( fp);
    return r;

} // listTYPE



//-------------------------------------------------------------
// Open Next File with given extension after last entry
//
MFILE *ffindM( const char *ext, unsigned *entry)
{
    unsigned int eOffs;
    char a;
    MFILE *fp;

    // 1.  check if storage device is mounted
    if ( D == NULL)       // unmounted
    {
        FError = FE_MEDIA_NOT_MNTD;
        return NULL;
    }

    // 3. allocate a MFILE structure on the heap
    fp = &F[0];

    // 4. set pointers to the MEDIA structure and buffer
    fp->mda = D;
    fp->buffer = &B[ D->cb];

    // 5. start from given entry, load its sector from root
    if ( !ReadDIR( fp, *entry))
    {    //report error
        FError = FE_FIND_ERROR;
        return NULL;
    }

    // 6. loop until you reach a match or the end of the root directory
    while ( *entry < fp->mda->maxroot)
    {
        eOffs = (*entry & 0xF) << 5;      // 32 bytes per entry
        // 6.1 read the first char of the file name
        a =  fp->buffer[ eOffs + DIR_NAME];

        // 6.2 terminate if it is empty (end of the list)
        if ( a == DIR_EMPTY)
            break;

        // 6.3 if not deleted entry to be skipped
        if ( a != DIR_DEL)
        {
          // if not hidden or directory name
          a = fp->buffer[ eOffs + DIR_ATTRIB];
          if ( !( a & (ATT_HIDE|ATT_DIR)))
          {   //  check the file extension

            // compare extension
            if ( !memcmp( &fp->buffer[ eOffs+DIR_EXT], ext, 3))
            {
                // found it
                // 7. copy the file mode character  (r, w)
                fp->mode = 'r';

                // 8. entry found, fill the file structure
                fp->entry = *entry;         // store index
                fp->time    = ReadW( fp->buffer, eOffs+DIR_TIME);
                fp->date    = ReadW( fp->buffer, eOffs+DIR_DATE);
                fp->size    = ReadL( fp->buffer, eOffs+DIR_SIZE);
                fp->cluster = ReadL( fp->buffer, eOffs+DIR_CLST);

                // 9. init all counters to the beginning of the file
                fp->seek = 0;               // first byte in file
                fp->sec = 0;                // first sector in the cluster
                fp->pos = 0;                // first byte in sector/cluster

                // 10. set current cluster pointer on first cluster
                fp->ccls = fp->cluster;

                // 11. read a sector of data from the file
                if ( !ReadDATA( fp))
                {
                    FError = FE_FIND_ERROR;
                    return NULL;
                }

                // 12. determine how much data is really inside the buffer
                if ( fp->size < 512)
                    fp->top = fp->size;
                else
                    fp->top = 512;

                // success
                (*entry)++;
                return fp;
            }
          } // it's a file
        } // it's not deleted or hidden

        // 13. get the next entry
        (*entry)++;

        if (( *entry & 0xF) == 0)
        {
            if ( !ReadDIR( fp, *entry))
            {
                FError = FE_FIND_ERROR;
                return NULL;
            }
        }
    }// while

    // 14. finished without finding it
    return NULL;

} // ffindM
