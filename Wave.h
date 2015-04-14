/*----------------------------------------------------------------------
 ** Wave.h
 **
 **  Wave File Player
 **  Uses 2 x 8 bit PWM channels at 44kHz
 */
#include "fileio.h"

unsigned long InitWAV( MFILE *);
void Play( MFILE *, unsigned long);

