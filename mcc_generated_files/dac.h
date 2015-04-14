/**
  DAC Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    dac.h

  @Summary
    This is the generated header file for the DAC driver using MPLAB® Code Configurator

  @Description
    This header file provides APIs for driver for DAC.
    Generation Information :
        Product Revision  :  MPLAB® Code Configurator - v2.10.3
        Device            :  PIC16F1709
        Driver Version    :  2.00
    The generated drivers are tested against the following:
        Compiler          :  XC8 v1.34
        MPLAB             :  MPLAB X 2.26
*/

/*
Copyright (c) 2013 - 2015 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*/

#ifndef _DAC_H
#define _DAC_H

/**
  Section: Included Files
*/

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif

/**
  Section: DAC APIs
*/

/**
  @Summary
    Initializes the DAC

  @Description
    This routine initializes the DAC.
    This routine must be called before any other DAC routine is called.
    This routine should only be called once during system initialization.

  @Preconditions
    None

  @Param
    None

  @Returns
    None

  @Comment
    Use the DACOUT1 (RA0) to output 8-bit audio samples

  @Example
    <code>
    DAC_Initialize();
    </code>
*/
void DAC_Initialize(void);

/**
  @Summary
    Set Input data into DAC.

  @Description
    This routine pass the digital input data into
    DAC voltage reference control register.

  @Preconditions
    The DAC_Initialize() routine should be called
    prior to use this routine.

  @Param
    inputData - 8bit digital data to DAC.

  @Returns
    None

  @Example
    <code>
    uint8_t count=0;

    DAC_Initialize();

    for(count=0; count<=30; count++)
    {
        DAC_SetOutput(count);
    }

    while(1)
    {
    }
    </code>
*/
void DAC_SetOutput(uint8_t inputData);

#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif // _DAC_H
/**
 End of File
*/
