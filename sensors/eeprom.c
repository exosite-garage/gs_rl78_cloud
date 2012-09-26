/*-------------------------------------------------------------------------*
 * File:  EEPROM.c
 *-------------------------------------------------------------------------*
 * Description:
 *     Temperature sensor driver using the ADT7420 over I2C.
 *-------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "eeprom.h"
#include <drv/I2C.h>
#include <system/mstimer.h>

/*-------------------------------------------------------------------------*
 * Constants:
 *-------------------------------------------------------------------------*/
/* ADT7420 IIC Registers */
#define EEPROM_DEVICE_ADDRESS       0xA0

/*---------------------------------------------------------------------------*
 * Routine:  EEPROM_Write
 *---------------------------------------------------------------------------*
 * Description:
 *      Write EEPROM 
 * Inputs:
 *      uint16_t    add,
        char *  pBuff
 * Outputs:
 *      
 *---------------------------------------------------------------------------*/
void EEPROM_Write(uint16_t addr, char *pdata)
{
    /* Declare error flag */
    uint8_t send[256];
    I2C_Request r;
    uint16_t len = 2;

    send[0] = addr & 0xFF00;
    send[1] = addr & 0x00FF;

    while (*pdata != '\0')
    {
      send[len] = (uint8_t)*pdata;
      len++;
      pdata++;
    }

    uint32_t timeout = MSTimerGet();

    r.iAddr = EEPROM_DEVICE_ADDRESS>>1;
    r.iSpeed = 100; /* kHz */
    r.iWriteData = send;
    r.iWriteLength = len;
    r.iReadData = 0;
    r.iReadLength = 0;

    I2C_Start();
    I2C_Write(&r, 0);
    while ((I2C_IsBusy()) && (MSTimerDelta(timeout) < 10))
        {}
}

/*---------------------------------------------------------------------------*
 * Routine:  EEPROM_Seq_Read
 *---------------------------------------------------------------------------*
 * Description:
 *      Read the value of the address and return the data .
 * Inputs:
 *      void
 * Outputs:
 *      uint16_t -- temperature with 4 bits of fraction and 12 bits of
 *          integer.
 *---------------------------------------------------------------------------*/
int16_t EEPROM_Seq_Read(uint16_t addr,uint8_t *pdata, uint16_t r_lenth)
{
    uint8_t target_address[2];
    uint32_t timeout = MSTimerGet();
    I2C_Request r;
    int16_t result = 0;

    target_address[0] = addr & 0xFF00;
    target_address[1] = addr & 0x00FF;     

    r.iAddr = EEPROM_DEVICE_ADDRESS >> 1;
    r.iSpeed = 100;
    r.iWriteData = target_address;
    r.iWriteLength = 2;
    r.iReadData = pdata;
    r.iReadLength = r_lenth;
    I2C_Write(&r, 0);
    while ((I2C_IsBusy()) && (MSTimerDelta(timeout) < 10))
        {}
    I2C_Read(&r, 0);
    while ((I2C_IsBusy()) && (MSTimerDelta(timeout) < 10))
        {}

    result = 1;

    return result;
}

/*-------------------------------------------------------------------------*
 * End of File:   Temperature_ADT7420.c
 *-------------------------------------------------------------------------*/
