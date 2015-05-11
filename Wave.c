/*
** Wave.C
** 
**  Wave File Player 
**  Modified for operation at <22050 Hz (8-bit mono)
*/

#include <xc.h>
#include <stdlib.h>
#include <string.h>
#include "AudioDAC.h"
#include "SDMMC.h"
#include "fileio.h"
#include "MCC_Generated_Files/mcc.h"


// WAVE file constants
#define  RIFF_DWORD  0x46464952UL
#define  WAVE_DWORD  0x45564157UL
#define  DATA_DWORD  0x61746164UL
#define  FMT_DWORD   0x20746d66UL     
#define  WAV_DWORD   0x00564157UL


typedef struct {
    long ckID;
    long ckSize;
    long ckType;
} chunk;

typedef struct {
    // format chunk 
    unsigned short  subtype;    // compression code  
    unsigned short  channels;   // # of channels
                                //  (1= mono,2= stereo)
    unsigned long   srate;      // sample rate in Hz
    unsigned long   bps;        // bytes per second
    unsigned short  align;      // block alignement
    unsigned short  bitpsample; // bit per sample
//    unsigned short  extra;      // extra format bytes
} WAVE_fmt;


unsigned long InitWAV( MFILE * fp)
{
    chunk       ck;
    WAVE_fmt    wav;
    unsigned long lc;

    // audio codec parameters
    unsigned char skip, size, stereo;
    unsigned long rate;
    
    // 2. verify it is a RIFF-WAVE formatted file
    freadM( (void*)&ck, sizeof(chunk), fp);
       
    // check that file type is correct
    if (( ck.ckID != RIFF_DWORD) || ( ck.ckType != WAVE_DWORD))
        goto Exit;
        
    // 3. look for the chunk containing the wave format data
    freadM( (void*)&ck, 8, fp);
    if ( ck.ckID != FMT_DWORD)
        goto Exit;

    // 4. get the WAVE_FMT struct
    freadM( (void*)&wav, sizeof(WAVE_fmt), fp);
    stereo = wav.channels;
     
    // 5. skip extra format bytes
    fseekM( fp, ck.ckSize - sizeof(WAVE_fmt));
         
    // 6. search for the "data" chunk
    while( 1)
    {   // read next chunk 
        if ( freadM( (void*)&ck, 8, fp) != 8)
            goto Exit;

        if ( ck.ckID != DATA_DWORD)
            fseekM( fp, ck.ckSize );
        else
            break;
    } 

    // 7. find the data size 
    lc = ck.ckSize;

    // 8. compute the period and adjust the bit rate
    rate = wav.srate;          // r = samples per second
    skip = 1;                  // skip factor to reduce noise
    while ( rate < 44100)
    {
        rate <<= 1;            // divide sample rate by two
        skip <<= 1;            // multiply skip by two
    }

    // 9. check if the sample rate compatible 
    if ( rate > 44100)
    {
        fcloseM( fp);
        return FALSE;
    }
    
    // 10. init the Audio state machine
    CurBuf = 0;
    stereo = wav.channels;
    size  = 1;                  // default, bytes per channel
    if ( wav.bitpsample == 16)
        size = 2;
    
    // 11. init the audio state machine
    InitAudio( skip, size, stereo);

    // return the file length
    return lc;

    Exit:

    // 17. close the file
    fcloseM( fp);

    return 0;
}


void Play( MFILE *fp, unsigned long lc)
{
    unsigned r;
    unsigned char      last;

    // 1. start loading both buffers
    if ( lc < B_SIZE*2)         // allow for files > BSIZE*2
        return;

    // 1.b skip to the beginning of the next complete sector
    fp->pos = fp->top;              // effectively consider the rest as read

    // now the next read will cause an entire sector to be read in
    D->cb = 1;                      // move onto the second buffer
    r = freadM( B[0], B_SIZE, fp);  // fill buffer 0<-1 and 2 with first sector
    fp->pos = fp->top;              // effectively consider the rest as read

    D->cb = 0;                      // make buffer 0,1 active
    CurBuf = 0;                     // play from 0

    AEmptyFlag = FALSE;         // both buffers are full
    lc-= B_SIZE*2 ;             // keep track of what is left

    // 2. start the audio
    StartAudio();

    // 3. keep feeding the buffers in the playing loop
    while (lc >= 2* B_SIZE)
    {
        if ( SW1_GetValue() == 0) // on pressing the button-> abort
        {
            lc = 0;               // signal playback completed
            break;
        }
        
        if ( AEmptyFlag)
        {
            if ( CurBuf)             // if buffer 2 is in use
            {
//            LED3_LAT = 1;
                r = freadM( NULL, 2*B_SIZE, fp);  // read the next sector
                lc-= 2*B_SIZE;
//            LED3_LAT = 0;
            }
            else                    // buffer 0 is in use
            {
                memcpy( B[2], B[1], B_SIZE);
            }
            AEmptyFlag = FALSE;
            
        }
    } // while wav data available

    // 14. flush the buffers with the data tail
    if( lc>0) 
    {
        if ( AEmptyFlag)
        {
            if ( CurBuf)             // if buffer 2 is in use
            {
//            LED3_LAT = 1;
                r = freadM( NULL, lc, fp);  // read the next sector
                lc=0;
                // fill ther rest of the last buffer
                last = B[0][r-1];       // get hte last value
                while( r < B_SIZE)
                    B[0][r++] = last;   // filling
                
//            LED3_LAT = 0;
            }
            else                    // buffer 0 is in use
            {
                memcpy( B[2], B[1], lc);
            }
            AEmptyFlag = FALSE;

        while (!AEmptyFlag);
        }
    }

    // 16. stop playback
    HaltAudio();

    // close file
    fcloseM( fp);

    // wait for button release if pressed to abort playback
    while( !SW1_GetValue());

} // Play

