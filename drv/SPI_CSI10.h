/*-------------------------------------------------------------------------*
 * File:  SPI.h
 *-------------------------------------------------------------------------*
 * Description:
 *     SPI interrupt based driver for the RX62N's CSI10 peripheral.
 *-------------------------------------------------------------------------*/
#ifndef SPI_CSI10_H_
#define SPI_CSI10_H_

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/*-------------------------------------------------------------------------*
 * Constants:
 *-------------------------------------------------------------------------*/
#define SPI_CSI10_NUM_CHANNELS          5

#define SPI_CSI10_SDCARD_CHANNEL        0
#define SPI_CSI10_WIFI_CHANNEL          1
#define SPI_CSI10_LCD_CHANNEL           2
#define SPI_CSI10_PMOD1_CHANNEL         3
#define SPI_CSI10_PMOD2_CHANNEL         4

/*-------------------------------------------------------------------------*
 * Prototypes:
 *-------------------------------------------------------------------------*/
void SPI_CSI10_Init(uint32_t bitsPerSecond);
void SPI_CSI10_ChannelSetup(
        uint8_t channel,
        bool csActiveHigh,
        bool csActivePerByte);
bool SPI_CSI10_Transfer(
        uint8_t channel,
        uint32_t numBytes,
        const uint8_t *send_buffer,
        uint8_t *receive_buffer,
        void(*callback)(void));
bool SPI_CSI10_IsBusy(void);
void SPI_CSI10_DisableInterrupts(void);
void SPI_CSI10_EnableInterrupts(void);

#endif // SPI_CSI10_H_
/*-------------------------------------------------------------------------*
 * End of File:  SPI.h
 *-------------------------------------------------------------------------*/
