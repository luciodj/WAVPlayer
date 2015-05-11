/*******************************************************************************
  Generated Main File 

  File Name:
    main.c

  Summary:
    This is the main file generated using MPLAB® Code Configurator

  *******************************************************************************/

#include <xc.h>
#include <string.h>
#include "MCC_Generated_Files/mcc.h"
#include "wave.h"

/****************************************************************************
 *
 *                        Main application
 *
****************************************************************************/
void main(void)
{
    unsigned long lc;   // file length
    MFILE* fp;          // file pointer
    unsigned int entry; // current piece

    // initialize the device
    SYSTEM_Initialize();

    // init File System
    LED0_LAT = 0;
    LED1_LAT = 0;

    while  (1)  // main loop
    {
        if ( mount() != 0)
        {
            LED0_LAT = 1;       // turn on LED0 if mount successful

            entry = 0;

            // look for a  W22 file
            while ( (fp = ffindM( "W22", &entry)) != NULL)
            {
                LED0_LAT = 0;       // turn off lED0 to save power
                LED1_LAT = 1;       // turn on LED1 if mount successful
                // wait for button press
                while( SW1_GetValue());
                // wait for button release
                while( !SW1_GetValue());
                LED1_LAT = 0;       // turn off LED1 to save power

                lc = InitWAV( fp);
                Play( fp, lc);
            } // while 
        }
        else
            LED0_LAT = 0;       // mounting error
    }
} // main
