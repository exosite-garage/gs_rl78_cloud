/**************************************************
 *
 * This is a template file.
 *
 * Copyright 2011 IAR Systems AB.
 *
* $Revision: 1.6 $
 *
 **************************************************/
#include <system/platform.h>
#include <HostApp.h>
#include <init/hwsetup.h>
#include <drv\Glyph\lcd.h>
#include <system\mstimer.h>
#include <sensors\Potentiometer.h>
#include <sensors\Temperature_ADT7420.h>
#include <sensors/eeprom.h>
#include <system\console.h>
#include <drv\UART0.h>
#include <drv\SPI_CSI10.h>
#include <Apps/Apps.h>


/*-------------------------------------------------------------------------*
 * Macros:
 *-------------------------------------------------------------------------*/
/* Set option bytes */
#pragma location = "OPTBYTE"
__root const uint8_t opbyte0 = 0xEFU;
#pragma location = "OPTBYTE"
__root const uint8_t opbyte1 = 0xFFU;
#pragma location = "OPTBYTE"
__root const uint8_t opbyte2 = 0xE9U;
#pragma location = "OPTBYTE"
__root const uint8_t opbyte3 = 0x04U;

/* Set security ID */
#pragma location = "SECUID"
__root const uint8_t secuid[10] = 
    {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

/*---------------------------------------------------------------------------*
 * Routine:  main
 *---------------------------------------------------------------------------*
 * Description:
 *      Setup the hardware, setup the peripherals, show the startup banner,
 *      wait for the module to power up, run a selected test or app.
 * Inputs:
 *      void
 * Outputs:
 *      int -- always 0.
 *---------------------------------------------------------------------------*/
int main(void)
{
    /* Configure the hardware */
    HardwareSetup();       
    /* Initialize millisecond timer */
    MSTimerInit();

    /* Initialize SPI */
    SPI_CSI10_Init(SPI_BITS_PER_SECOND);

    /* Setup LCD SPI channel for Chip Select P10, active low, active per byte  */
    SPI_CSI10_ChannelSetup(SPI_CSI10_LCD_CHANNEL, false, true);

    /* Send LCD configuration */
    InitialiseLCD();

    UART0_Start(UART0_BAUD_RATE);

    /* Setup WIFI SPI channel for Chip Select P7x, active low, active per byte  */
    SPI_CSI10_ChannelSetup(SPI_WIFI_CHANNEL, false, true);

    Temperature_ADT7420_Init();
    Potentiometer_Init();

    DisplayLCD(LCD_LINE1, "GS Demo 1.01");
    DisplayLCD(LCD_LINE2, "            ");

    /* Give the unit a little time to start up */
    /* (300 ms for GS1011 and 1000 ms for GS1500) */
    MSTimerDelay(1000);

    App_Exosite();

    return 0;
}
