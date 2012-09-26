/*-------------------------------------------------------------------------*
 * File:  Apps.h
 *-------------------------------------------------------------------------*
 * Description:
 *     Routines in the common file and each of the different demo
 *     Apps to run.
 *-------------------------------------------------------------------------*/
#ifndef APPS_H_
#define APPS_H_

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/*-------------------------------------------------------------------------*
 * Constants:
 *-------------------------------------------------------------------------*/
#define APP_MAX_RECEIVED_DATA         320

/*-------------------------------------------------------------------------*
 * Globals:
 *-------------------------------------------------------------------------*/
extern uint8_t G_received[APP_MAX_RECEIVED_DATA + 1];
extern uint32_t G_receivedCount;

/*-------------------------------------------------------------------------*
 * Prototypes:
 *-------------------------------------------------------------------------*/
void App_Exosite(void);
void App_Write(const uint8_t *txData, uint32_t dataLength);
bool App_Read(uint8_t *rxData, uint32_t dataLength, uint8_t blockFlag);
void App_PrepareIncomingData(uint8_t cid);
void App_ProcessIncomingData(uint8_t cid, uint8_t rxData);
void App_CheckDataIN(void);
#endif // APPS_H_
/*-------------------------------------------------------------------------*
 * End of File:  Apps.h
 *-------------------------------------------------------------------------*/
