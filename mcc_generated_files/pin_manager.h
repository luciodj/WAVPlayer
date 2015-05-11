/**
  @Generated Pin Manager Header File

  @Company:
    Microchip Technology Inc.

  @File Name:
    pin_manager.h

  @Summary:
    This is the Pin Manager file generated using MPLAB® Code Configurator

  @Description:
    This header file provides implementations for pin APIs for all pins selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB® Code Configurator - v2.25
        Device            :  PIC16F1709
        Version           :  1.01
    The generated drivers are tested against the following:
        Compiler          :  XC8 v1.34
        MPLAB             :  MPLAB X v2.35 or v3.00
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

#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set DACOUT1 aliases
#define DACOUT1_TRIS               TRISA0
#define DACOUT1_LAT                LATA0
#define DACOUT1_PORT               RA0
#define DACOUT1_WPU                WPUA0
#define DACOUT1_ANS                ANSA0
#define DACOUT1_SetHigh()    do { LATA0 = 1; } while(0)
#define DACOUT1_SetLow()   do { LATA0 = 0; } while(0)
#define DACOUT1_Toggle()   do { LATA0 = ~LATA0; } while(0)
#define DACOUT1_GetValue()         RA0
#define DACOUT1_SetDigitalInput()    do { TRISA0 = 1; } while(0)
#define DACOUT1_SetDigitalOutput()   do { TRISA0 = 0; } while(0)

#define DACOUT1_SetPullup()    do { WPUA0 = 1; } while(0)
#define DACOUT1_ResetPullup()   do { WPUA0 = 0; } while(0)
#define DACOUT1_SetAnalogMode()   do { ANSA0 = 1; } while(0)
#define DACOUT1_SetDigitalMode()   do { ANSA0 = 0; } while(0)
// get/set SW1 aliases
#define SW1_TRIS               TRISA2
#define SW1_LAT                LATA2
#define SW1_PORT               RA2
#define SW1_WPU                WPUA2
#define SW1_ANS                ANSA2
#define SW1_SetHigh()    do { LATA2 = 1; } while(0)
#define SW1_SetLow()   do { LATA2 = 0; } while(0)
#define SW1_Toggle()   do { LATA2 = ~LATA2; } while(0)
#define SW1_GetValue()         RA2
#define SW1_SetDigitalInput()    do { TRISA2 = 1; } while(0)
#define SW1_SetDigitalOutput()   do { TRISA2 = 0; } while(0)

#define SW1_SetPullup()    do { WPUA2 = 1; } while(0)
#define SW1_ResetPullup()   do { WPUA2 = 0; } while(0)
#define SW1_SetAnalogMode()   do { ANSA2 = 1; } while(0)
#define SW1_SetDigitalMode()   do { ANSA2 = 0; } while(0)
// get/set SDI aliases
#define SDI_TRIS               TRISB4
#define SDI_LAT                LATB4
#define SDI_PORT               RB4
#define SDI_WPU                WPUB4
#define SDI_ANS                ANSB4
#define SDI_SetHigh()    do { LATB4 = 1; } while(0)
#define SDI_SetLow()   do { LATB4 = 0; } while(0)
#define SDI_Toggle()   do { LATB4 = ~LATB4; } while(0)
#define SDI_GetValue()         RB4
#define SDI_SetDigitalInput()    do { TRISB4 = 1; } while(0)
#define SDI_SetDigitalOutput()   do { TRISB4 = 0; } while(0)

#define SDI_SetPullup()    do { WPUB4 = 1; } while(0)
#define SDI_ResetPullup()   do { WPUB4 = 0; } while(0)
#define SDI_SetAnalogMode()   do { ANSB4 = 1; } while(0)
#define SDI_SetDigitalMode()   do { ANSB4 = 0; } while(0)
// get/set SCK aliases
#define SCK_TRIS               TRISB6
#define SCK_LAT                LATB6
#define SCK_PORT               RB6
#define SCK_WPU                WPUB6
#define SCK_SetHigh()    do { LATB6 = 1; } while(0)
#define SCK_SetLow()   do { LATB6 = 0; } while(0)
#define SCK_Toggle()   do { LATB6 = ~LATB6; } while(0)
#define SCK_GetValue()         RB6
#define SCK_SetDigitalInput()    do { TRISB6 = 1; } while(0)
#define SCK_SetDigitalOutput()   do { TRISB6 = 0; } while(0)

#define SCK_SetPullup()    do { WPUB6 = 1; } while(0)
#define SCK_ResetPullup()   do { WPUB6 = 0; } while(0)
// get/set LED0 aliases
#define LED0_TRIS               TRISC0
#define LED0_LAT                LATC0
#define LED0_PORT               RC0
#define LED0_WPU                WPUC0
#define LED0_ANS                ANSC0
#define LED0_SetHigh()    do { LATC0 = 1; } while(0)
#define LED0_SetLow()   do { LATC0 = 0; } while(0)
#define LED0_Toggle()   do { LATC0 = ~LATC0; } while(0)
#define LED0_GetValue()         RC0
#define LED0_SetDigitalInput()    do { TRISC0 = 1; } while(0)
#define LED0_SetDigitalOutput()   do { TRISC0 = 0; } while(0)

#define LED0_SetPullup()    do { WPUC0 = 1; } while(0)
#define LED0_ResetPullup()   do { WPUC0 = 0; } while(0)
#define LED0_SetAnalogMode()   do { ANSC0 = 1; } while(0)
#define LED0_SetDigitalMode()   do { ANSC0 = 0; } while(0)
// get/set LED1 aliases
#define LED1_TRIS               TRISC1
#define LED1_LAT                LATC1
#define LED1_PORT               RC1
#define LED1_WPU                WPUC1
#define LED1_ANS                ANSC1
#define LED1_SetHigh()    do { LATC1 = 1; } while(0)
#define LED1_SetLow()   do { LATC1 = 0; } while(0)
#define LED1_Toggle()   do { LATC1 = ~LATC1; } while(0)
#define LED1_GetValue()         RC1
#define LED1_SetDigitalInput()    do { TRISC1 = 1; } while(0)
#define LED1_SetDigitalOutput()   do { TRISC1 = 0; } while(0)

#define LED1_SetPullup()    do { WPUC1 = 1; } while(0)
#define LED1_ResetPullup()   do { WPUC1 = 0; } while(0)
#define LED1_SetAnalogMode()   do { ANSC1 = 1; } while(0)
#define LED1_SetDigitalMode()   do { ANSC1 = 0; } while(0)
// get/set LED2 aliases
#define LED2_TRIS               TRISC2
#define LED2_LAT                LATC2
#define LED2_PORT               RC2
#define LED2_WPU                WPUC2
#define LED2_ANS                ANSC2
#define LED2_SetHigh()    do { LATC2 = 1; } while(0)
#define LED2_SetLow()   do { LATC2 = 0; } while(0)
#define LED2_Toggle()   do { LATC2 = ~LATC2; } while(0)
#define LED2_GetValue()         RC2
#define LED2_SetDigitalInput()    do { TRISC2 = 1; } while(0)
#define LED2_SetDigitalOutput()   do { TRISC2 = 0; } while(0)

#define LED2_SetPullup()    do { WPUC2 = 1; } while(0)
#define LED2_ResetPullup()   do { WPUC2 = 0; } while(0)
#define LED2_SetAnalogMode()   do { ANSC2 = 1; } while(0)
#define LED2_SetDigitalMode()   do { ANSC2 = 0; } while(0)
// get/set LED3 aliases
#define LED3_TRIS               TRISC3
#define LED3_LAT                LATC3
#define LED3_PORT               RC3
#define LED3_WPU                WPUC3
#define LED3_ANS                ANSC3
#define LED3_SetHigh()    do { LATC3 = 1; } while(0)
#define LED3_SetLow()   do { LATC3 = 0; } while(0)
#define LED3_Toggle()   do { LATC3 = ~LATC3; } while(0)
#define LED3_GetValue()         RC3
#define LED3_SetDigitalInput()    do { TRISC3 = 1; } while(0)
#define LED3_SetDigitalOutput()   do { TRISC3 = 0; } while(0)

#define LED3_SetPullup()    do { WPUC3 = 1; } while(0)
#define LED3_ResetPullup()   do { WPUC3 = 0; } while(0)
#define LED3_SetAnalogMode()   do { ANSC3 = 1; } while(0)
#define LED3_SetDigitalMode()   do { ANSC3 = 0; } while(0)
// get/set CS aliases
#define CS_TRIS               TRISC4
#define CS_LAT                LATC4
#define CS_PORT               RC4
#define CS_WPU                WPUC4
#define CS_SetHigh()    do { LATC4 = 1; } while(0)
#define CS_SetLow()   do { LATC4 = 0; } while(0)
#define CS_Toggle()   do { LATC4 = ~LATC4; } while(0)
#define CS_GetValue()         RC4
#define CS_SetDigitalInput()    do { TRISC4 = 1; } while(0)
#define CS_SetDigitalOutput()   do { TRISC4 = 0; } while(0)

#define CS_SetPullup()    do { WPUC4 = 1; } while(0)
#define CS_ResetPullup()   do { WPUC4 = 0; } while(0)
// get/set SDO aliases
#define SDO_TRIS               TRISC7
#define SDO_LAT                LATC7
#define SDO_PORT               RC7
#define SDO_WPU                WPUC7
#define SDO_ANS                ANSC7
#define SDO_SetHigh()    do { LATC7 = 1; } while(0)
#define SDO_SetLow()   do { LATC7 = 0; } while(0)
#define SDO_Toggle()   do { LATC7 = ~LATC7; } while(0)
#define SDO_GetValue()         RC7
#define SDO_SetDigitalInput()    do { TRISC7 = 1; } while(0)
#define SDO_SetDigitalOutput()   do { TRISC7 = 0; } while(0)

#define SDO_SetPullup()    do { WPUC7 = 1; } while(0)
#define SDO_ResetPullup()   do { WPUC7 = 0; } while(0)
#define SDO_SetAnalogMode()   do { ANSC7 = 1; } while(0)
#define SDO_SetDigitalMode()   do { ANSC7 = 0; } while(0)

/**
 * @Param
    none
 * @Returns
    none
 * @Description
    GPIO and peripheral I/O initialization
 * @Example
    PIN_MANAGER_Initialize();
 */
void PIN_MANAGER_Initialize (void);

/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handling routine
 * @Example
    PIN_MANAGER_IOC();
 */
void PIN_MANAGER_IOC(void);

#endif // PIN_MANAGER_H
/**
 End of File
*/