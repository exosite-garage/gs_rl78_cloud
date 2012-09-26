/*-------------------------------------------------------------------------*
 * File:  UART2.h
 *-------------------------------------------------------------------------*
 * Description:
 *     FIFO driven UART2 driver for RL78.
 *-------------------------------------------------------------------------*/
#ifndef _UART2_H
#define _UART2_H

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/*-------------------------------------------------------------------------*
 * Prototypes:
 *-------------------------------------------------------------------------*/
void UART2_Start(uint32_t baud);
void UART2_Stop(void);
bool UART2_ReceiveByte(uint8_t *aByte);
bool UART2_SendByte(uint8_t aByte);
uint32_t UART2_SendData(const uint8_t *aData, uint32_t aLen);
void UART2_SendDataBlock(const uint8_t *aData, uint32_t aLen);
bool UART2_IsTransmitEmpty(void);

#endif // _UART2_H
/*-------------------------------------------------------------------------*
 * End of File:  UART2.h
 *-------------------------------------------------------------------------*/
