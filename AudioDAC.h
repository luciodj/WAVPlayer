/*
** AudioPWM.h
**
*/
#include "HardwareProfile.h"
#define B_SIZE   256    // audio buffer size

extern unsigned char B[3][B_SIZE]; // data buffer
extern char CurBuf;              // index of buffer in use
extern volatile char AEmptyFlag; // flag a buffer needs to be filled

void InitAudio( char skip, char size, char stereo);
void StartAudio( void);
void HaltAudio( void);
