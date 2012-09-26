/*-------------------------------------------------------------------------*
 * File:  App_Common.c
 *-------------------------------------------------------------------------*
 * Description:
 *     Common routines used by all the Apps.
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <HostApp.h>
#include <system/platform.h>
#include <CmdLib/AtCmdLib.h>
#include <system/mstimer.h>
#include <drv/GainSpan_SPI.h>
#include "Apps.h"

/*-------------------------------------------------------------------------*
 * Globals:
 *-------------------------------------------------------------------------*/
uint8_t G_received[APP_MAX_RECEIVED_DATA + 1];
uint32_t G_receivedCount = 0;

/*---------------------------------------------------------------------------*
 * Routine:  App_Write
 *---------------------------------------------------------------------------*
 * Description:
 *      ATCmdLib callback to write a string of characters to the module.
 * Inputs:
 *      const uint8_t *txData -- string of bytes
 *      uint32_t dataLength -- Number of bytes to transfer
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void App_Write(const uint8_t *txData, uint32_t dataLength)
{
#ifdef HOST_APP_INTERFACE_SPI
    while (dataLength--) {
        /* Keep trying to send this data until it goes */
        while (!GainSpan_SPI_SendByte(*txData)) {
            /* Process any incoming data as well */
            GainSpan_SPI_Update();
        }

        txData++;
    }
#else
    while (dataLength--) {
        /* Keep trying to send this data until it goes */
        while (!UART2_SendByte(*txData)) {
        }
        txData++;
    }
#endif
}

/*---------------------------------------------------------------------------*
 * Routine:  App_Read
 *---------------------------------------------------------------------------*
 * Description:
 *      ATCmdLib callback to read a string of characters from the module.
 *      This routine can block if needed until the data arrives.
 * Inputs:
 *      uint8_t *rxData -- Pointer to a place to store a string of bytes
 *      uint32_t dataLength -- Number of bytes to transfer
 *      uint8_t blockFlag -- true/non-zero to wait for bytes, else false/zero.
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
bool App_Read(uint8_t *rxData, uint32_t dataLength, uint8_t blockFlag)
{
#ifdef HOST_APP_INTERFACE_SPI
    bool got_data = false;

    /* Keep getting data if we have a number of bytes to fetch */
    while (dataLength) {
        /* Try to get a byte */
        if (GainSpan_SPI_ReceiveByte(rxData)) {
            /* Got a byte, move up to the next position */
            rxData++;
            dataLength--;
            got_data = true;
        } else {
            /* Did not get a byte, are we block?  If not, stop here */
            if (!blockFlag)
                break;
        }
    }

    return got_data;
#else
    bool got_data = false;

    /* Keep getting data if we have a number of bytes to fetch */
//    while (dataLength) {
        /* Try to get a byte */
//        if (UART2_ReceiveByte(rxData)) {
            /* Got a byte, move up to the next position */
//            rxData++;
//            dataLength--;
//            got_data = true;
//        } else {
            /* Did not get a byte, are we block?  If not, stop here */
//            if (!blockFlag)
//            break;
//        }
//    }
//
    return got_data;
#endif
}

/*---------------------------------------------------------------------------*
 * Routine:  App_PrepareIncomingData
 *---------------------------------------------------------------------------*
 * Description:
 *      ATCmdLib callback to reset the incoming data state.
 * Inputs:
 *      uint8_t cid -- Current connection
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void App_PrepareIncomingData(uint8_t cid)
{
    G_receivedCount = 0;
    G_received[0] = '\0';
}

/*---------------------------------------------------------------------------*
 * Routine:  App_ProcessIncomingData
 *---------------------------------------------------------------------------*
 * Description:
 *      ATCmdLib callback that is called when a byte has come in for a
 *      specific connection.
 * Inputs:
 *      uint8_t cid -- Connection receiving data
 *      uint8_t rxData -- Byte received
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void App_ProcessIncomingData(uint8_t cid, uint8_t rxData)
{
    if (G_receivedCount < APP_MAX_RECEIVED_DATA) {
        G_received[G_receivedCount++] = rxData;
        G_received[G_receivedCount] = '\0';
    }
}

/*-------------------------------------------------------------------------*
 * End of File:  App_Common.c
 *-------------------------------------------------------------------------*/
