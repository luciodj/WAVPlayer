/*
** SDMMC.h
**
** SDMMC card interface  
**
*/
#include "HardwareProfile.h"

#ifndef _SDMMC
#define _SDMMC

#define FALSE   0
#define TRUE    !FALSE
#define FAIL    0


typedef unsigned long LBA; // logic block address, 32 bit wide


void InitSD( void);     // initializes the I/O pins for the SD/MMC interface 
int InitMedia( void);   // initializes the SD/MMC memory device

int DetectSD( void);    // detects the card presence 
int DetectWP( void);    // detects the position of the write protect switch

int ReadSECTOR ( LBA, unsigned char *);  // reads a block of data
int WriteSECTOR( LBA, unsigned char *);  // writes a block of data

#endif

