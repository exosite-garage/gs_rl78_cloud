/*-------------------------------------------------------------------------*
 * File:  IntervalTimer.c
 *-------------------------------------------------------------------------*
 * Description:
 *     RL78 Interval Timer driver
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <system/platform.h>
#include "IntervalTimer.h"

/*-------------------------------------------------------------------------*
 * Globals:
 *-------------------------------------------------------------------------*/
/* Interrupt service routine function pointer to call per timer interrupt. */
static void (*G_IntervalTimer_ISR)(void);

/*---------------------------------------------------------------------------*
 * Interrupt Routine:  Interrupt_IntervalTimer
 *---------------------------------------------------------------------------*
 * Description:
 *      Each time the timer fires off an interrupt, this routine catches it
 *      and passes it to the given interrupt service routine.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
#pragma vector = INTIT_vect
__interrupt static void Interrupt_IntervalTimer(void)
{
    /* Call interrupt service routine */
    G_IntervalTimer_ISR();
}

/*---------------------------------------------------------------------------*
 * Routine:  IntervalTimer_Start
 *---------------------------------------------------------------------------*
 * Description:
 *      Start the timer counting.
 * Inputs:
 *      void (*isr)(void) -- interrupt service routine to call per interrupt.
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void IntervalTimer_Start(void (*isr)(void))
{
    /*
        Initialize the IT module.
    */
  
    /* Record the callback for an interrupt match */
    G_IntervalTimer_ISR = isr;

    /* supply IT clock */
    RTCEN = 1U;
    /* disable IT operation */
    ITMC = _IT_OPERATION_DISABLE;
    /* disable INTIT interrupt */
    ITMK = 1U;
    /* clear INTIT interrupt flag */
    ITIF = 0U;
    /* Set INTIT low priority */
    ITPR1 = 1U;
    ITPR0 = 1U;
    ITMC = _ITMCMP_VALUE;
    
    
    /*
        Start the IT module.
    */
    
    /* enable IT operation */
    ITMC |= _IT_OPERATION_ENABLE;
    /* clear INTIT interrupt flag */
    ITIF = 0U;
    /* enable INTIT interrupt */
    ITMK = 0U;
}

/*---------------------------------------------------------------------------*
 * Routine:  IntervalTimer_Stop
 *---------------------------------------------------------------------------*
 * Description:
 *      Stop the timer counting.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void IntervalTimer_Stop(void)
{
    /*
        Stop the IT module.
    */
  
    /* disable INTIT interrupt */
    ITMK = 1U;
    /* clear INTIT interrupt flag */
    ITIF = 0U;
    /* disable IT operation */
    ITMC &= (uint16_t)~_IT_OPERATION_ENABLE;
}

/*-------------------------------------------------------------------------*
 * End of File:  IntervalTimer.h
 *-------------------------------------------------------------------------*/
