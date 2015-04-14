/*
** SD/MMC card interface 
**
** Low Pin Count Eval + PIC16F1829
** 
*/

#include "HardwareProfile.h"
#include "SDMMC.h"

  // I/O definitions 
    #define SDWP    SD_WP    // Write Protect input
    #define SDCD    SD_CD    // Card Detect input
    #define SDCS    SD_CS    // Card Select output
  // Card Select TRIS control
    #define TRISCS() SD_CS_TRIS = 0;



// SD card commands
#define SD_RESET            0 // a.k.a. GO_IDLE (CMD0)
#define INIT                1 // a.k.a. SEND_OP_COND (CMD1)
#define SEND_CSD            9
#define SEND_CID           10
#define SET_BLEN           16
#define READ_SINGLE        17
#define WRITE_SINGLE       24
#define APP_CMD            55
#define SEND_APP_OP        41

// SD card responses
#define DATA_START       0xFE
#define DATA_ACCEPT      0x05


void InitSD( void)
{
    SDCS = 1;
    TRISCS();           // make Card Select an output pin
    
    // init the spi module for a slow (safe) clock speed first
    OpenSPISlow();

} // InitSD


#define ReadSPI()   WriteSPI( 0xFF)
#define ClockSPI()  WriteSPI( 0xFF)
#define DisableSD() SDCS = 1; ClockSPI()
#define EnableSD()  SDCS = 0


int DetectSD( void)
{
    return ( !SDCD);  
} // Detect SD

int DetectWP( void)
{
    return ( !SD_WE);
} // Detect WP


int SendSDCmd( unsigned char c, LBA a)
{
    int i, r;

    // enable SD card
    EnableSD();

    // send a comand packet (6 bytes)
    WriteSPI( c | 0x40);    // send command + frame bit
    WriteSPI( a>>24);       // MSB of the address
    WriteSPI( a>>16);
    WriteSPI( a>>8);
    WriteSPI( a);           // LSB
    
    WriteSPI( 0x95);        // send CMD0 CRC

    // now wait for a response (allow for up to 8 bytes delay)
    i = 9;
    do {
        r = ReadSPI();      // check if ready
        if ( r != 0xFF) break;
    } while ( --i > 0);

    return ( r);         

/* return response
    FF - timeout 
    00 - command accepted
    01 - command received, card in idle state after RESET

other codes:
    bit 0 = Idle state
    bit 1 = Erase Reset
    bit 2 = Illegal command
    bit 3 = Communication CRC error
    bit 4 = Erase sequence error
    bit 5 = Address error
    bit 6 = Parameter error
    bit 7 = Always 0
*/
    // NOTE CSCD is still low
} // SendSDCmd


int ReadSECTOR( LBA a, unsigned char *p)
// a        LBA of sector requested
// p        pointer to sector buffer
// returns  TRUE if successful
{
    int r, i;
    unsigned char c =0;

//    LED1_LAT = 1;
    r = SendSDCmd( READ_SINGLE, ( a << 9));
    if ( r == 0)    // check if command was accepted
    {  
        // wait for a response
        i = 10000;
        do{
            r = ReadSPI();
            if ( r == DATA_START) break;
        }while( --i>0);

//        LED1_LAT = 0;
//        LED2_LAT = 1;
      // if it did not timeout, read a 512 byte sector of data
        if ( i)
        {
//            for( i=0; i<512; i++)
            do
            {
                SSP1BUF = 0xFF;               
                asm("NOP");
                asm("NOP");
                asm("NOP");
                asm("NOP");
                *p++ = SSP1BUF;
            } while ( --c != 0);

            do
            {
                SSP1BUF = 0xFF;
                asm("NOP");
                asm("NOP");
                asm("NOP");
                asm("NOP");
                *p++ = SSP1BUF;
            } while ( --c != 0);
//        LED2_LAT = 0;

            // ignore CRC
            ClockSPI();
            ClockSPI();
        } // data arrived
    } // command accepted

    // remember to disable the card
    DisableSD();
    //READ_LED = 0;

    return ( r == DATA_START);  // return TRUE if successful
} // ReadSECTOR





int InitMedia( void)
{
    int i, r;

    // 1. with the card not selected     
    DisableSD();

    // 2. send 80 clock cycles start up
    for ( i=0; i<10; i++)
        ClockSPI();

    // 3. now select the card
    EnableSD();

    // 4. send a reset command to enter SPI mode
    r = SendSDCmd( SD_RESET, 0); DisableSD();
    if ( r != 1)
        return 0x84;

    // 5. send repeatedly INIT  
    i = 10000;  // up to .3 sec before timeout
    do {
        r = SendSDCmd( INIT, 0); DisableSD();
        if ( !r) break; 
    } while( --i > 0);
    if ( i==0)   
        return 0x85;       // timed out 
    
    // 6. increase speed
    OpenSPIFast();
    return 0;           

} // InitMedia
