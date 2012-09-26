/*-------------------------------------------------------------------------*
 * File:  YRDKRL78G13.h
 *-------------------------------------------------------------------------*
 * Description:
 *     Defines for the Renesas RDK RX62N development board.
 *-------------------------------------------------------------------------*/
#ifndef YRDKRL78G13_H
#define YRDKRL78G13_H

#define RL78_MAIN_SYSTEM_CLOCK          12000000    /* MHz */
#define RL78_SUBSYSTEM_CLOCK            32768       /* kHz */
#define RL78_INTERNAL_LOWSPEED_CLOCK    15000       /* kHz */

#define _F8_ "%d"
#define _F16_ "%d"
#define _F32_ "%ld"

#define DI      __disable_interrupt
#define EI      __enable_interrupt
#define HALT    __halt
#define NOP     __no_operation
#define STOP    __stop

#endif
/*-------------------------------------------------------------------------*
 * End of File:  YRDKRL78G13.h
 *-------------------------------------------------------------------------*/
