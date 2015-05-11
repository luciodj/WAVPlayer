/*
 * File:   HardwareProfile.h
 *
 * Hardware platform: PIC16F1 LV Evaluation Board + Audio
 */

#ifndef _HARDWARE_PROFILE_H
#define _HARDWARE_PROFILE_H

#define F1_LV_EVAL    1601

#include <xc.h>

#include "MCC_Generated_Files/mcc.h"


/*********************************************************************
 * PIC16 clock
 ********************************************************************/
#define GetSystemClock()            (_XTAL_FREQ)
#define	GetPeripheralClock()        (GetSystemClock()/4)
#define GetInstructionClock()       (GetSystemClock()/4)


/*********************************************************************
 * IOs for the micro SD card interface
 *********************************************************************/

#define USE_SD_INTERFACE_WITH_SPI

    // Registers for the SPI module 
    #define MDD_USE_SPI_1

    // MDD SPI Configuration

    // Description: SD-SPI Chip Select Output bit
    #define SD_CS               LATCbits.LATC4
    // Description: SD-SPI Chip Select TRIS bit
    #define SD_CS_TRIS          TRISCbits.TRISC4

    // Description: SD-SPI Card Detect Input bit
    #define SD_CD               0
    // Description: SD-SPI Card Detect TRIS bit
    #define SD_CD_TRIS          W

    // Description: SD-SPI Write Protect Check Input bit
    #define SD_WE               0
    // Description: SD-SPI Write Protect Check TRIS bit
    #define SD_WE_TRIS          W               // assigned to other input pin!


    
#define OpenSPISlow()     SPI_InitializeSlow()
#define OpenSPIFast()     SPI_Initialize()
#define WriteSPI( b)      SPI_Exchange8bit( b)

#endif // HARDWARE_PROFILE_H
