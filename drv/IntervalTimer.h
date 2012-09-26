/*-------------------------------------------------------------------------*
 * File:  IntervalTimer.h
 *-------------------------------------------------------------------------*
 * Description:
 *     RL78 Timer driver using CMT3 on channel 3.
 *-------------------------------------------------------------------------*/
#ifndef IntervalTimer_H_
#define IntervalTimer_H_

#include <ior5f101le.h>

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <stdint.h>

/*-------------------------------------------------------------------------*
 * Macro definitions (Register bit)
 *-------------------------------------------------------------------------*/
/* 
    Interval timer control register (ITMC)
*/
/* Interval timer operation enable/disable specification (RINTE) */
#define _IT_OPERATION_DISABLE    (0x0000U)
#define _IT_OPERATION_ENABLE     (0x8000U)
/* Interval timer compare value (ITMCMP11 - 0) */
#define _ITMCMP_VALUE            (0x0020U)

/*-------------------------------------------------------------------------*
 * Macros:
 *-------------------------------------------------------------------------*/
#define IntervalTimer_DisableIRQ()     ITMK = 1U
#define IntervalTimer_EnableIRQ()      ITMK = 0U

/*-------------------------------------------------------------------------*
 * Prototypes:
 *-------------------------------------------------------------------------*/
void IntervalTimer_Start(void (*isr)(void));
void IntervalTimer_Stop(void);

#endif /* IntervalTimer_H_ */
/*-------------------------------------------------------------------------*
 * End of File:  IntervalTimer.h
 *-------------------------------------------------------------------------*/
