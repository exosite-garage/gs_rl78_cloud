/*-------------------------------------------------------------------------*
 * File:  ADC.c
 *-------------------------------------------------------------------------*
 * Description:
 *     RL78 ADC polling driver.
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <system/platform.h>
#include "ADC.h"

/*---------------------------------------------------------------------------*
 * Routine:  Interrupt_ADC
 *---------------------------------------------------------------------------*
 * Description:
 *      This function is INTAD interrupt service routine.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
#pragma vector = INTAD_vect
__interrupt void Interrupt_ADC(void)
{
	/* Start user code. Do not edit comment generated here */
	/* End user code. Do not edit comment generated here */
}

/*---------------------------------------------------------------------------*
 * Routine:  ADC_Start
 *---------------------------------------------------------------------------*
 * Description:
 *      Start the RL78's A/D peripheral.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void ADC_Start(void)
{
    /*
        Initialize the AD converter
    */
    ADCEN = 1U;  /* supply AD clock */
    ADM0 = _AD_ADM0_INITIALVALUE;  /* disable AD conversion and clear ADM0 register */
    ADMK = 1U;  /* disable INTAD interrupt */
    ADIF = 0U;  /* clear INTAD interrupt flag */
    /* Set INTAD low priority */
    ADPR1 = 1U;
    ADPR0 = 1U;
    /* Set ANI0 - ANI7 pin as analog input */
    PM2 |= 0xFFU;
    ADM0 = _AD_CONVERSION_CLOCK_32 | _AD_TIME_MODE_NORMAL_1 | _AD_OPERMODE_SELECT;
    ADM1 = _AD_TRIGGER_SOFTWARE | _AD_CONVMODE_CONSELECT;
    ADM2 = _AD_POSITIVE_VDD | _AD_NEGATIVE_VSS | _AD_AREA_MODE_1 | _AD_RESOLUTION_10BIT;
    ADUL = _AD_ADUL_VALUE;
    ADLL = _AD_ADLL_VALUE;
    ADCE = 1U;  /* enable AD comparator */
    
    /*
        Start the AD converter
    */
    ADIF = 0U;  /* clear INTAD interrupt flag */
    ADMK = 0U;  /* enable INTAD interrupt */
    ADCS = 1U;  /* enable AD conversion */
}

/*---------------------------------------------------------------------------*
 * Routine:  ADC_EnableChannel
 *---------------------------------------------------------------------------*
 * Description:
 *      Enable one of the A/D channels.
 * Inputs:
 *      uint8_t channel -- Channel 0 to 7
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void ADC_SelectChannel(uint8_t channel)
{
    /* Set the analog input channel specification register */
    ADS = channel;
}

/*---------------------------------------------------------------------------*
 * Routine:  ADC_GetReading
 *---------------------------------------------------------------------------*
 * Description:
 *      Take an A/D reading using the  on a specific channel and
 *      return.
 * Inputs:
 *      uint16_t * const buffer -- pointer to the read buffer
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
uint32_t ADC_GetReading()
{
    return (ADCR >> 6U);
}

/*-------------------------------------------------------------------------*
 * End of File:  ADC.c
 *-------------------------------------------------------------------------*/
