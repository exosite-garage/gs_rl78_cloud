/*-------------------------------------------------------------------------*
 * File:  I2C.h
 *-------------------------------------------------------------------------*
 * Description:
 *      Simple implementation of the Renesas RL78 I2C Interface.
 *-------------------------------------------------------------------------*/
#ifndef I2C_H_
#define I2C_H_

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/*-------------------------------------------------------------------------*
 * Macro definitions (Register bit)
 *-------------------------------------------------------------------------*/
/*
    IICA Control Register (IICCTLn0)
*/
/* IIC operation enable (IICEn) */
#define _IICA_OPERATION_DISABLE       (0x00U)    /* stop operation */
#define _IICA_OPERATION_ENABLE        (0x80U)    /* enable operation */
/* Exit from communications (LRELn) */
#define _IICA_COMMUNICATION_NORMAL    (0x00U)    /* normal operation */
#define _IICA_COMMUNICATION_EXIT      (0x40U)    /* exit from current communication */
/* Wait cancellation (WRELn) */
#define _IICA_WAIT_NOTCANCEL          (0x00U)    /* do not cancel wait */
#define _IICA_WAIT_CANCEL             (0x20U)    /* cancel wait */
/* Generation of interrupt when stop condition (SPIEn) */
#define _IICA_STOPINT_DISABLE         (0x00U)    /* disable */
#define _IICA_STOPINT_ENABLE          (0x10U)    /* enable */
/* Wait and interrupt generation (WTIMn) */
#define _IICA_WAITINT_CLK8FALLING     (0x00U)    /* generated at the eighth clock's falling edge */
#define _IICA_WAITINT_CLK9FALLING     (0x08U)    /* generated at the ninth clock's falling edge */
/* Acknowledgement control (ACKEn) */
#define _IICA_ACK_DISABLE             (0x00U)    /* enable acknowledgement */
#define _IICA_ACK_ENABLE              (0x04U)    /* disable acknowledgement */
/* Start condition trigger (STTn) */
#define _IICA_START_NOTGENERATE       (0x00U)    /* do not generate start condition */
#define _IICA_START_GENERATE          (0x02U)    /* generate start condition */
/* Stop condition trigger (SPTn) */
#define _IICA_STOP_NOTGENERATE        (0x00U)    /* do not generate stop condition */
#define _IICA_STOP_GENERATE           (0x01U)    /* generate stop condition */

/*
    IICA Status Register (IICSn)
*/
/* Master device status (MSTSn) */
#define _IICA_STATUS_NOTMASTER        (0x00U)    /* slave device status or communication standby status */
#define _IICA_STATUS_MASTER           (0x80U)    /* master device communication status */
/* Detection of arbitration loss (ALDn) */
#define _IICA_ARBITRATION_NO          (0x00U)    /* arbitration win or no arbitration */
#define _IICA_ARBITRATION_LOSS        (0x40U)    /* arbitration loss */
/* Detection of extension code reception (EXCn) */
#define _IICA_EXTCODE_NOT             (0x00U)    /* extension code not received */
#define _IICA_EXTCODE_RECEIVED        (0x20U)    /* extension code received */
/* Detection of matching addresses (COIn) */
#define _IICA_ADDRESS_NOTMATCH        (0x00U)    /* addresses do not match */
#define _IICA_ADDRESS_MATCH           (0x10U)    /* addresses match */
/* Detection of transmit/receive status (TRCn) */
#define _IICA_STATUS_RECEIVE          (0x00U)    /* receive status */ 
#define _IICA_STATUS_TRANSMIT         (0x08U)    /* transmit status */
/* Detection of acknowledge signal (ACKDn) */
#define _IICA_ACK_NOTDETECTED         (0x00U)    /* ACK signal was not detected */
#define _IICA_ACK_DETECTED            (0x04U)    /* ACK signal was detected */
/* Detection of start condition (STDn) */
#define _IICA_START_NOTDETECTED       (0x00U)    /* start condition not detected */
#define _IICA_START_DETECTED          (0x02U)    /* start condition detected */
/* Detection of stop condition (SPDn) */
#define _IICA_STOP_NOTDETECTED        (0x00U)    /* stop condition not detected */
#define _IICA_STOP_DETECTED           (0x01U)    /* stop condition detected */

/*
    IICA Flag Register (IICFn)
*/
/* STT clear flag (STCFn) */
#define _IICA_STARTFLAG_GENERATE      (0x00U)    /* generate start condition */
#define _IICA_STARTFLAG_UNSUCCESSFUL  (0x80U)    /* start condition generation unsuccessful */
/* IIC bus status flag (IICBSYn) */
#define _IICA_BUS_RELEASE             (0x00U)    /* bus release status */
#define _IICA_BUS_COMMUNICATION       (0x40U)    /* bus communication status */
/* Initial start enable trigger (STCENn) */
#define _IICA_START_WITHSTOP          (0x00U)    /* generate start condition without detecting a stop condition */
#define _IICA_START_WITHOUTSTOP       (0x02U)    /* generate start condition upon detection of a stop condition */
/* Communication reservation function disable bit (IICRSVn) */
#define _IICA_RESERVATION_ENABLE      (0x00U)    /* enable communication reservation */
#define _IICA_RESERVATION_DISABLE     (0x01U)    /* disable communication reservation */

/*
    IICA Control Register 1 (IICCTLn1)
*/
/* Control of address match wakeup (WUPn) */
#define _IICA_WAKEUP_STOP             (0x00U)    /* stop address match wakeup function in STOP mode */
#define _IICA_WAKEUP_ENABLE           (0x80U)    /* enable address match wakeup function in STOP mode */
/* Detection of SCL0 pin level (CLDn) */
#define _IICA_SCL_LOW                 (0x00U)    /* detect clock line at low level */
#define _IICA_SCL_HIGH                (0x20U)    /* detect clock line at high level */
/* Detection of SDA0 pin level (DADn) */
#define _IICA_SDA_LOW                 (0x00U)    /* detect data line at low level */
#define _IICA_SDA_HIGH                (0x10U)    /* detect data line at high level */
/* Operation mode switching (SMCn) */
#define _IICA_MODE_STANDARD           (0x00U)    /* operates in standard mode */
#define _IICA_MODE_HIGHSPEED          (0x08U)    /* operates in high-speed mode */
/* Digital filter operation control (DFCn) */
#define _IICA_FILTER_OFF              (0x00U)    /* digital filter off */ 
#define _IICA_FILTER_ON               (0x04U)    /* digital filter on */
/* Operation of clock dividing frequency permission (PRSn) */
#define _IICA_fCLK                    (0x00U)    /* clock of dividing frequency operation (fCLK) */ 
#define _IICA_fCLK_HALF               (0x01U)    /* 2 clock of dividing frequency operation (fCLK/2) */

/*-------------------------------------------------------------------------*
 * Macro definitions
 *-------------------------------------------------------------------------*/
#define _IICA0_MASTERADDRESS          (0x10U)

/*-------------------------------------------------------------------------*
 * Types:
 *-------------------------------------------------------------------------*/
typedef enum {
    I2C_OK = 0,

    // currently active
    I2C_BUSY = 1,

    // some error occurred (bus arbitration?)
    I2C_ERROR = 2,

    // Received NAK
    I2C_NAK = 3,

    // Hardware is not ready to perform function
    I2C_NOT_READY = 4,
    
    // I2C operation stopped
    I2C_STOP = 5,
} T_uEZI2CStatus;

typedef struct {
    uint8_t iAddr; // 7-bit address of I2C device
    uint16_t iSpeed; // in kHz
    const uint8_t *iWriteData; // 0 or NULL value means no write action
    uint8_t iWriteLength;
    uint8_t *iReadData; // 0 or NULL value means no read action
    uint8_t iReadLength;
    volatile T_uEZI2CStatus iStatus;
} I2C_Request;

/*-------------------------------------------------------------------------*
 * Prototypes:
 *-------------------------------------------------------------------------*/
void I2C_Start(void);
void I2C_Stop(void);
void I2C_Read(I2C_Request *iRequest, void(*callback)(void));
void I2C_Write(I2C_Request *iRequest, void(*callback)(void));
bool I2C_IsBusy(void);

#endif // I2C_H_
/*-------------------------------------------------------------------------*
 * End of File:  I2C.h
 *-------------------------------------------------------------------------*/
