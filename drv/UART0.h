/*-------------------------------------------------------------------------*
 * File:  UART0.h
 *-------------------------------------------------------------------------*
 * Description:
 *     FIFO driven UART0 driver for RL78.
 *-------------------------------------------------------------------------*/
#ifndef _UART0_H
#define _UART0_H

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/*-------------------------------------------------------------------------*
 * Prototypes:
 *-------------------------------------------------------------------------*/
void UART0_Start(uint32_t baud);
void UART0_Stop(void);
bool UART0_ReceiveByte(uint8_t *aByte);
bool UART0_SendByte(uint8_t aByte);
uint32_t UART0_SendData(const uint8_t *aData, uint32_t aLen);
void UART0_SendDataBlock(const uint8_t *aData, uint32_t aLen);
bool UART0_IsTransmitEmpty(void);

#endif // _UART0_H
/*-------------------------------------------------------------------------*
 * End of File:  UART0.h
 *-------------------------------------------------------------------------*/
