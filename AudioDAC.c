/*
** AudioDAC.c
**
** 07/22/06 AudioPWM.c v1.1 PIC24
** 12/01/07 AudioPWM.c v1.2 All buffers unsigned char, no_auto_psv
** 14/04/15 v2.0 Modified to use the PIC16F170x 8-bit DAC
*/
#include "AudioDAC.h"

// global definitions
char           CurBuf;       // index of buffer in use
volatile char  AEmptyFlag;   // flag a buffer needs to be filled

// internal variables
//char Stereo;                 // flag stereo play back
//char Fix;                    // sign fix for 16-bit samples
char Skip;                   // skip factor, reduce sample/rate
char Size;                   // sample size (8 or 16-bit)

// local definitions
unsigned char *BPtr;         // pointer inside active buffer
unsigned char BCount;        // count bytes used
//unsigned char Bytes;         // number of bytes per sample

void InitAudio(  char skip, char size, char stereo)
{
    // 1. init pointers 
    CurBuf = 0;             // start with buffer0 active first
    BPtr = &B[ CurBuf][size-1];
    BCount = B_SIZE;        // number of samples to be played
    AEmptyFlag = 0;
    Skip = skip;
//    Fix = (size==2)? 0x80 : 0;  // sign correction for 16-bit
//    Stereo = stereo;
    Size = size;
//    Bytes = size*stereo;

} // InitAudio


void StartAudio( void)
{
    // 1. init the timebase
    TMR2_StartTimer();

    // 2. activate the PWM modules
    // configured in the System init
    INTERRUPT_PeripheralInterruptEnable();
    INTERRUPT_GlobalInterruptEnable();
} // start audio


void HaltAudio( void)
{
    TMR2_StopTimer();
} // HaltAudio


void TMR2_CallBack( void)
{
    static int sk = 1;
//    unsigned char temp;

    // 1. skip to increase the bitrate (avoid PWM noise)
    if ( --sk == 0)
     {
        // reload the skip 
        sk = Skip;

        // 2. load the new samples for the next cycle
        DAC1CON1 =   *BPtr>>1 ;     // ^ Fix);
//        if ( Stereo==2)
//            temp = 30 + (*(BPtr + Size) ^ Fix);
//        else    // mono
//            OC2RS = OC1RS;
//        BPtr += Bytes;
        BPtr++;

        // 3. check if buffer emptied
//        BCount -= Bytes;
        BCount--;
        if ( BCount == 0)
        {
            // 3.1 swap buffers
            CurBuf = 2 - CurBuf;
            BPtr = &B[ CurBuf][Size-1];
            
            // 3.2 buffer refilled
            BCount = B_SIZE;
    
            // 3.3 flag a new buffer needs to be filled
            AEmptyFlag = 1;
        }
    } 
    
} // T2 Interrupt

