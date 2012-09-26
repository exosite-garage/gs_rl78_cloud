/*-------------------------------------------------------------------------*
 * File:  Temperature_ADT7420.c
 *-------------------------------------------------------------------------*
 * Description:
 *     Temperature sensor driver using the ADT7420 over I2C.
 *-------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "Temperature_ADT7420.h"
#include <drv/I2C.h>
#include <system/mstimer.h>

/*-------------------------------------------------------------------------*
 * Constants:
 *-------------------------------------------------------------------------*/
/* ADT7420 IIC Registers */
#define ADT7420_ADDR                0x90
#define ADT7420_DEVICE_ID           0xC8    // bits 7:3 of mfg identification number
#define ADT7420_TEMP_MSB_REG        0x00
#define ADT7420_TEMP_LSB_REG        0x01
#define ADT7420_STATUS_REG          0x02
#define ADT7420_CONFIG_REG          0x03
#define ADT7420_T_HIGH_MSB_REG      0x04
#define ADT7420_T_HIGH_LSB_REG      0x05
#define ADT7420_T_LOW_MSB_REG       0x06
#define ADT7420_T_LOW_LSB_REG       0x07
#define ADT7420_T_CRIT_MSB_REG      0x08
#define ADT7420_T_CRIT_LSB_REG      0x09
#define ADT7420_HIST_REG            0x0A
#define ADT7420_ID_REG              0x0B
#define ADT7420_RESET_REG           0x2F

/*---------------------------------------------------------------------------*
 * Routine:  Temperature_ADT7420_Init
 *---------------------------------------------------------------------------*
 * Description:
 *      Initialize the temperature ADT7420 driver.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void Temperature_ADT7420_Init(void)
{
    /* Declare error flag */
    uint8_t cmd[2] = { ADT7420_CONFIG_REG, 0x00 };
    I2C_Request r;
    uint32_t timeout = MSTimerGet();

    r.iAddr = ADT7420_ADDR>>1;
    r.iSpeed = 100; /* kHz */
    r.iWriteData = cmd;
    r.iWriteLength = 2;
    r.iReadData = 0;
    r.iReadLength = 0;

    I2C_Start();
    I2C_Write(&r, 0);
    while ((I2C_IsBusy()) && (MSTimerDelta(timeout) < 10))
        {}
}

/*---------------------------------------------------------------------------*
 * Routine:  Temperature_ADT7420_Get
 *---------------------------------------------------------------------------*
 * Description:
 *      Read the value of the ADT7420 and return the temperature in Celcius.
 * Inputs:
 *      void
 * Outputs:
 *      uint16_t -- temperature with 4 bits of fraction and 12 bits of
 *          integer.
 *---------------------------------------------------------------------------*/
int16_t Temperature_ADT7420_Get(void)
{
    uint8_t target_reg;
    uint8_t target_data[2] = {0x00, 0x00};
    uint16_t temp = 0;
    uint32_t timeout = MSTimerGet();
    I2C_Request r;

    r.iAddr = ADT7420_ADDR>>1;
    r.iSpeed = 100;
    r.iWriteData = &target_reg;
    r.iWriteLength = 1;
    r.iReadData = target_data;
    r.iReadLength = 2;
    I2C_Write(&r, 0);
    while ((I2C_IsBusy()) && (MSTimerDelta(timeout) < 10))
        {}
    I2C_Read(&r, 0);
    while ((I2C_IsBusy()) && (MSTimerDelta(timeout) < 10))
        {}

    /* Convert the device measurement into a decimal number and insert
     into a temporary string to be displayed */
    temp = (target_data[0] << 8) + target_data[1];
    temp = temp >> 3;

    return temp;
}

/*-------------------------------------------------------------------------*
 * End of File:   Temperature_ADT7420.c
 *-------------------------------------------------------------------------*/
