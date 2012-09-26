/*-------------------------------------------------------------------------*
 * File:  mstimer.c
 *-------------------------------------------------------------------------*
 * Description:
 *     Using a timer, provide a one millisecond accurate timer.
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <stdbool.h>
#include <system/platform.h>
#include <drv/IntervalTimer.h>
#include "mstimer.h"

/*-------------------------------------------------------------------------*
 * Globals:
 *-------------------------------------------------------------------------*/
/* 32-bit counter of current number of milliseconds since timer started */
static volatile uint32_t G_msTimer;

/*---------------------------------------------------------------------------*
 * Routine:  _MSTimerISR
 *---------------------------------------------------------------------------*
 * Description:
 *      Internal interrupt service routine to increment counter per
 *      timer interrupt.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
static void _MSTimerISR(void)
{
    G_msTimer++;
}

/*---------------------------------------------------------------------------*
 * Routine:  MSTimerInit
 *---------------------------------------------------------------------------*
 * Description:
 *      Initialize and start the one millisecond timer counter.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void MSTimerInit(void)
{
    // Reset counter
    IntervalTimer_DisableIRQ();
    G_msTimer = 0;
    IntervalTimer_Start(_MSTimerISR);
    IntervalTimer_EnableIRQ();
}

/*---------------------------------------------------------------------------*
 * Routine:  MSTimerGet
 *---------------------------------------------------------------------------*
 * Description:
 *      Get the number of millisecond counters since started.  This value
 *      rolls over ever 4,294,967,296 ms or 49.7102696 days.
 * Inputs:
 *      void
 * Outputs:
 *      uint32_t -- Millisecond counter since timer started or last rollover.
 *---------------------------------------------------------------------------*/
uint32_t MSTimerGet(void)
{
    uint32_t t1;

    // Disable CMT3 timer interrupt
    IntervalTimer_DisableIRQ();
    // Grab the millisecond timer value
    t1 = G_msTimer;
    // Enable interrupts
    IntervalTimer_EnableIRQ();

    return t1;
}

/*---------------------------------------------------------------------------*
 * Routine:  MSTimerDelta
 *---------------------------------------------------------------------------*
 * Description:
 *      Calculate the current number of milliseconds expired since a given
 *      start timer value.
 * Inputs:
 *      uint32_t start -- Timer value at start of delta.
 * Outputs:
 *      uint32_t -- Millisecond counter since given timer value.
 *---------------------------------------------------------------------------*/
uint32_t MSTimerDelta(uint32_t start)
{
    return MSTimerGet() - start;
}

/*---------------------------------------------------------------------------*
 * Routine:  MSTimerDelay
 *---------------------------------------------------------------------------*
 * Description:
 *      Routine to idle and delay a given number of milliseconds doing
 *      nothing.
 * Inputs:
 *      uint32_t ms -- Number of milliseconds to delay
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void MSTimerDelay(uint32_t ms)
{
    uint32_t start = MSTimerGet();

    while (MSTimerDelta(start) < ms) {
    }
}

/*-------------------------------------------------------------------------*
 * End of File:  mstimer.c
 *-------------------------------------------------------------------------*/

