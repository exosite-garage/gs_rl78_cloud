/*-------------------------------------------------------------------------*
 * File:  hwsetup.h
 *-------------------------------------------------------------------------*
 * Description:
 *     Setup all the devices and ports for this hardware.
 *-------------------------------------------------------------------------*/
#ifndef HWSETUP_H
#define HWSETUP_H

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/***********************************************************************************************************************
Macro definitions (Register bit)
***********************************************************************************************************************/
/*
    Clock operation mode control register (CMC)
*/
/* High-speed system clock pin operation mode (EXCLK, OSCSEL) */
#define _CGC_HISYS_PIN           (0xC0U)
#define _CGC_HISYS_PORT          (0x00U) /* X1, X2 as I/O port */
#define _CGC_HISYS_OSC           (0x40U) /* X1, X2 as crystal/ceramic resonator connection */
#define _CGC_HISYS_PORT1         (0x80U) /* X1, X2 as I/O port */
#define _CGC_HISYS_EXT           (0xC0U) /* X1 as I/O port, X2 as external clock input */
/* Subsystem clock pin operation mode (EXCLKS, OSCSELS) */
#define _CGC_SUB_PIN             (0x30U)
#define _CGC_SUB_PORT            (0x00U) /* XT1, XT2 as I/O port */
#define _CGC_SUB_OSC             (0x10U) /* XT1, XT2 as crystal connection */
#define _CGC_SUB_PORT1           (0x20U) /* XT1, XT2 as I/O port */
#define _CGC_SUB_EXT             (0x30U) /* XT1 as I/O port, XT2 as external clock input */
/* XT1 oscillator oscillation mode selection (AMPHS1, AMPHS0) */
#define _CGC_SUBMODE_DEFAULT     (0x00U)
#define _CGC_SUBMODE_LOW         (0x00U) /* low power consumption oscillation */
#define _CGC_SUBMODE_NORMAL      (0x02U) /* normal oscillation */
#define _CGC_SUBMODE_ULOW        (0x04U) /* ultra-low power consumption oscillation */
/* Control of X1 high-speed system clock oscillation frequency (AMPH) */
#define _CGC_SYSOSC_DEFAULT      (0x00U)
#define _CGC_SYSOSC_UNDER10M     (0x00U) /* fX <= 10MHz */
#define _CGC_SYSOSC_OVER10M      (0x01U) /* fX > 10MHz */

/*
    Clock operation status control register (CSC)
*/
/* Control of high-speed system clock operation (MSTOP) */
#define _CGC_HISYS_OPER          (0x00U) /* X1 oscillator/external clock operating */
#define _CGC_HISYS_STOP          (0x80U) /* X1 oscillator/external clock stopped */
/* Subsystem clock operation (XTSTOP) */
#define _CGC_SUB_OPER            (0x00U) /* XT1 oscillator operating */
#define _CGC_SUB_STOP            (0x40U) /* XT1 oscillator stopped */
/* High-speed OCO operation (HIOSTOP) */
#define _CGC_HIO_OPER            (0x00U) /* high-speed OCO operating */
#define _CGC_HIO_STOP            (0x01U) /* high-speed OCO stopped */

/*
    Oscillation stabilization time counter status register (OSTC)
*/
/* Oscillation stabilization time status (MOST18 - MOST8) */
#define _CGC_OSCSTAB_STA0        (0x00U) /* < 2^8/fX */
#define _CGC_OSCSTAB_STA8        (0x80U) /* 2^8/fX */
#define _CGC_OSCSTAB_STA9        (0xC0U) /* 2^9/fX */
#define _CGC_OSCSTAB_STA10       (0xE0U) /* 2^10/fX */
#define _CGC_OSCSTAB_STA11       (0xF0U) /* 2^11/fX */
#define _CGC_OSCSTAB_STA13       (0xF8U) /* 2^13/fX */
#define _CGC_OSCSTAB_STA15       (0xFCU) /* 2^15/fX */
#define _CGC_OSCSTAB_STA17       (0xFEU) /* 2^17/fX */
#define _CGC_OSCSTAB_STA18       (0xFFU) /* 2^18/fX */

/*
    Oscillation stabilization time select register (OSTS)
*/
/* Oscillation stabilization time selection (OSTS2 - OSTS0) */
#define _CGC_OSCSTAB_SEL8        (0x00U) /* 2^8/fX */
#define _CGC_OSCSTAB_SEL9        (0x01U) /* 2^9/fX */
#define _CGC_OSCSTAB_SEL10       (0x02U) /* 2^10/fX */
#define _CGC_OSCSTAB_SEL11       (0x03U) /* 2^11/fX */
#define _CGC_OSCSTAB_SEL13       (0x04U) /* 2^13/fX */
#define _CGC_OSCSTAB_SEL15       (0x05U) /* 2^15/fX */
#define _CGC_OSCSTAB_SEL17       (0x06U) /* 2^17/fX */
#define _CGC_OSCSTAB_SEL18       (0x07U) /* 2^18/fX */

/*
    System clock control register (CKC)
*/
/* Status of CPU/peripheral hardware clock fCLK (CLS) */
#define _CGC_CPUCLK_MAIN         (0x00U) /* main system clock (fMAIN) */
#define _CGC_CPUCLK_SUB          (0x80U) /* subsystem clock (fSUB) */
/* Selection of CPU/peripheral hardware clock fCLK (CSS) */
#define _CGC_CPUCLK_SELMAIN      (0x00U) /* main system clock (fMAIN) */
#define _CGC_CPUCLK_SELSUB       (0x40U) /* subsystem clock (fSUB) */
/* Status of Main system clock fMAIN (MCS) */
#define _CGC_MAINCLK_HIO         (0x00U) /* high-speed OCO clock (fIH) */
#define _CGC_MAINCLK_HISYS       (0x20U) /* high-speed system clock (fMX) */
/* Selection of Main system clock fMAIN (MCM0) */
#define _CGC_MAINCLK_SELHIO      (0x00U) /* high-speed OCO clock (fIH) */
#define _CGC_MAINCLK_SELHISYS    (0x10U) /* high-speed system clock (fMX) */

/*
    Operation speed mode control register (OSMC)
*/
/* Setting in subsystem clock HALT mode (RTCLPC) */
#define _CGC_SUBINHALT_ON        (0x00U) /* enables supply of subsystem clock to peripheral functions */
#define _CGC_SUBINHALT_OFF       (0x80U) /* stops supply to peripheral functions other than RTC and interval timer */
/* RTC macro operation clock (WUTMMCK0) */
#define _CGC_RTC_CLK_FSUB        (0x00U) /* use fSUB clock */
#define _CGC_RTC_CLK_FIL         (0x10U) /* use fIL clcok */

/*
    CRC control register (CRC0CTL)
*/
/* CRC calculation operation (CRC0EN) */
#define _CGC_CRC_OPERATION_OFF   (0x00U) /* stops operation */
#define _CGC_CRC_OPERATION_ON    (0x80U) /* CRC calculation is performed after enter HALT mode */
/* CRC calculation range (FEA5 - FEA0) */
#define _CGC_CRC_AREA0           (0x00U) /* 0 - 3FFBH (16K - 4byte) */
#define _CGC_CRC_AREA1           (0x01U) /* 0 - 7FFBH (32K - 4byte) */
#define _CGC_CRC_AREA2           (0x02U) /* 0 - BFFBH (48K - 4byte) */
#define _CGC_CRC_AREA3           (0x03U) /* 0 - FFFBH (64K - 4byte) */

/*
    Illegal memory access detection control register (IAWCTL)
*/
/* Illegal memory access detection control (IAWEN) */
#define _CGC_ILLEGAL_ACCESS_OFF  (0x00U) /* disables illegal memory access detection */
#define _CGC_ILLEGAL_ACCESS_ON   (0x80U) /* enables illegal memory access detection */
/* RAM guard area (GRAM1, GRAM0) */
#define _CGC_RAM_GUARD_OFF       (0x00U) /* invalid, it is possible to write RAM */
#define _CGC_RAM_GUARD_ARAE0     (0x10U) /* 128 bytes from RAM bottom address */
#define _CGC_RAM_GUARD_ARAE1     (0x20U) /* 256 bytes from RAM bottom address */
#define _CGC_RAM_GUARD_ARAE2     (0x30U) /* 512 bytes from RAM bottom address */
/* PORT register guard (GPORT) */
#define _CGC_PORT_GUARD_OFF      (0x00U) /* invalid, it is possible to write PORT register */
#define _CGC_PORT_GUARD_ON       (0x04U) /* valid, it is impossible to write PORT register, but possible for read */
/* Interrupt register guard (GINT) */   
#define _CGC_INT_GUARD_OFF       (0x00U) /* invalid, it is possible to write interrupt register */
#define _CGC_INT_GUARD_ON        (0x02U) /* valid, impossible to write interrupt register, but possible for read */
/* CSC register guard (GCSC) */         
#define _CGC_CSC_GUARD_OFF       (0x00U) /* invalid, it is possible to write CSC register */
#define _CGC_CSC_GUARD_ON        (0x01U) /* valid, it is impossible to write CSC register, but possible for read */

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define CGC_SUBWAITTIME             (160U)

/*-------------------------------------------------------------------------*
 * Prototypes:
 *-------------------------------------------------------------------------*/
void ConfigureOperatingFrequency(void);
void ConfigureOutputPorts(void);
void ConfigureInterrupts(void);
void EnablePeripheralModules(void);
void HardwareSetup(void);

#endif
/*-------------------------------------------------------------------------*
 * End of File:  hwsetup.h
 *-------------------------------------------------------------------------*/
