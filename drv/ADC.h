/*-------------------------------------------------------------------------*
 * File:  ADC.h
 *-------------------------------------------------------------------------*
 * Description:
 *     RL78 ADC polling driver.
 *-------------------------------------------------------------------------*/
#ifndef ADC_H_
#define ADC_H_

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <stdint.h>


/*-------------------------------------------------------------------------*
 * Macro definitions (Register bit)
 *-------------------------------------------------------------------------*/
/*
    Peripheral enable register 0 (PER0)
*/
/* Control of AD converter input clock (ADCEN) */
#define _AD_CLOCK_STOP               (0x00U) /* stop supply of input clock */
#define _AD_CLOCK_SUPPLY             (0x20U) /* supply input clock */

/*
    AD converter mode register 0 (ADM0)
*/
#define _AD_ADM0_INITIALVALUE        (0x00U)
/* AD conversion operation control (ADCS) */
#define _AD_CONVERSION_ENABLE        (0x80U) /* enable AD conversion operation control */
#define _AD_CONVERSION_DISABLE       (0x00U) /* disable AD conversion operation control */
/* Specification of AD conversion operation mode (ADMD) */
#define _AD_OPERMODE_SELECT          (0x00U) /* select operation mode */
#define _AD_OPERMODE_SCAN            (0x40U) /* scan operation mode */
/* AD conversion clock selection (FR2 - FR0) */
#define _AD_CONVERSION_CLOCK_64      (0x00U) /* fCLK/64 */
#define _AD_CONVERSION_CLOCK_32      (0x08U) /* fCLK/32 */
#define _AD_CONVERSION_CLOCK_16      (0x10U) /* fCLK/16 */
#define _AD_CONVERSION_CLOCK_8       (0x18U) /* fCLK/8 */
#define _AD_CONVERSION_CLOCK_6       (0x20U) /* fCLK/6 */
#define _AD_CONVERSION_CLOCK_5       (0x28U) /* fCLK/5 */
#define _AD_CONVERSION_CLOCK_4       (0x30U) /* fCLK/4 */
#define _AD_CONVERSION_CLOCK_2       (0x38U) /* fCLK/2 */
/* Specification AD conversion time mode (LV1, LV0) */
#define _AD_TIME_MODE_NORMAL_1       (0x00U) /* normal 1 mode */
#define _AD_TIME_MODE_NORMAL_2       (0x02U) /* normal 2 mode */
#define _AD_TIME_MODE_LOWVOLTAGE_1   (0x04U) /* low-voltage 1 mode */
#define _AD_TIME_MODE_LOWVOLTAGE_2   (0x06U) /* low-voltage 2 mode */
/* AD comparator operation control (ADCE) */
#define _AD_COMPARATOR_ENABLE        (0x01U) /* enable comparator operation control */
#define _AD_COMPARATOR_DISABLE       (0x00U) /* disable comparator operation control */

/*
    Analog input channel specification register (ADS)
*/
/* Specification of analog input channel (ADISS, ADS4 - ADS0) */
/* Select mode */
#define _AD_INPUT_CHANNEL_0          (0x00U) /* ANI0 */
#define _AD_INPUT_CHANNEL_1          (0x01U) /* ANI1 */
#define _AD_INPUT_CHANNEL_2          (0x02U) /* ANI2 */
#define _AD_INPUT_CHANNEL_3          (0x03U) /* ANI3 */
#define _AD_INPUT_CHANNEL_4          (0x04U) /* ANI4 */
#define _AD_INPUT_CHANNEL_5          (0x05U) /* ANI5 */
#define _AD_INPUT_CHANNEL_6          (0x06U) /* ANI6 */
#define _AD_INPUT_CHANNEL_7          (0x07U) /* ANI7 */
#define _AD_INPUT_CHANNEL_16         (0x10U) /* ANI16 */
#define _AD_INPUT_CHANNEL_17         (0x11U) /* ANI17 */
#define _AD_INPUT_CHANNEL_18         (0x12U) /* ANI18 */
#define _AD_INPUT_CHANNEL_19         (0x13U) /* ANI19 */
#define _AD_INPUT_TEMPERSENSOR_0     (0x80U) /* temperature sensor 0 output is used to be the input channel */
#define _AD_INPUT_INTERREFVOLT       (0x81U) /* internal reference voltage output is used to be the input channel */
/* Scan mode */
#define _AD_INPUT_CHANNEL_0_3        (0x00U) /* ANI0 - ANI3 */
#define _AD_INPUT_CHANNEL_1_4        (0x01U) /* ANI1 - ANI4 */
#define _AD_INPUT_CHANNEL_2_5        (0x02U) /* ANI2 - ANI5 */
#define _AD_INPUT_CHANNEL_3_6        (0x03U) /* ANI3 - ANI6 */
#define _AD_INPUT_CHANNEL_4_7        (0x04U) /* ANI4 - ANI7 */

/*
    AD converter mode register 1 (ADM1)
*/
/* AD trigger mode selection (ADTMD1, ADTMD0) */
#define _AD_TRIGGER_SOFTWARE         (0x00U) /* software trigger mode */
#define _AD_TRIGGER_HARDWARE_NOWAIT  (0x80U) /* hardware trigger mode (no wait) */
#define _AD_TRIGGER_HARDWARE_WAIT    (0xC0U) /* hardware trigger mode (wait) */
/* AD convertion mode selection (ADSCM) */
#define _AD_CONVMODE_CONSELECT       (0x00U) /* continuous convertion mode */
#define _AD_CONVMODE_ONESELECT       (0x20U) /* oneshot convertion mode */
/* Trigger signal selection (ADTRS1, ADTRS0) */
#define _AD_TRIGGER_INTTM01          (0x00U) /* INTTM01 */
#define _AD_TRIGGER_INTRTC           (0x02U) /* INTRTC */
#define _AD_TRIGGER_INTIT            (0x03U) /* INTIT */

/*
    AD converter mode register 2 (ADM2)
*/
/* AD VREF(+) selection (ADREFP1, ADREFP0) */
#define _AD_POSITIVE_VDD             (0x00U) /* use VDD as VREF(+) */
#define _AD_POSITIVE_AVREFP          (0x40U) /* use AVREFP as VREF(+) */
#define _AD_POSITIVE_INTERVOLT       (0x80U) /* use internal voltage as VREF(+) */
/* AD VREF(-) selection (ADREFM) */
#define _AD_NEGATIVE_VSS             (0x00U) /* use VSS as VREF(-) */
#define _AD_NEGATIVE_AVREFM          (0x20U) /* use AVREFM as VREF(-) */
/* AD conversion result upper/lower bound value selection (ADRCK) */
#define _AD_AREA_MODE_1              (0x00U) /* generates INTAD when ADLL ¡Ü ADCRH ¡Ü ADUL */
#define _AD_AREA_MODE_2_3            (0x08U) /* generates INTAD when ADUL < ADCRH or ADLL > ADCRH */
/* AD wakeup function selection (AWC) */
#define _AD_WAKEUP_OFF               (0x00U) /* stop wakeup function */
#define _AD_WAKEUP_ON                (0x04U) /* use wakeup function */
/* AD resolution selection (ADTYP) */
#define _AD_RESOLUTION_10BIT         (0x00U) /* 10 bits */
#define _AD_RESOLUTION_8BIT          (0x01U) /* 8 bits */

/*
    AD test function register (ADTES)
*/
/* AD test mode signal (ADTES2 - ADTES0) */
#define _AD_NORMAL_INPUT             (0x00U) /* normal mode */
#define _AD_TEST_AVREFM              (0x02U) /* use AVREFM as test signal */
#define _AD_TEST_AVREFP              (0x03U) /* use AVREFP as test signal */

/*
    AD port configuration register (ADPC)
*/
/* Analog input/digital input switching (ADPC3 - ADPC0) */
#define _AD_ADPC_8ANALOG             (0x00U) /* ANI0 - ANI7 */
#define _AD_ADPC_7ANALOG             (0x08U) /* ANI0 - ANI6 */
#define _AD_ADPC_6ANALOG             (0x07U) /* ANI0 - ANI5 */
#define _AD_ADPC_5ANALOG             (0x06U) /* ANI0 - ANI4 */
#define _AD_ADPC_4ANALOG             (0x05U) /* ANI0 - ANI3 */
#define _AD_ADPC_3ANALOG             (0x04U) /* ANI0 - ANI2 */
#define _AD_ADPC_2ANALOG             (0x03U) /* ANI0 - ANI1 */
#define _AD_ADPC_1ANALOG             (0x02U) /* ANI0 */
#define _AD_ADPC_0ANALOG             (0x01U) /* ANI0 - ANI7 (all digital) */

/*
    Conversion result comparison upper limit setting register (ADUL)
*/
/* Upper bound (ADUL) value */
#define _AD_ADUL_VALUE               (0xFFU)
/* Upper bound (ADLL) value */
#define _AD_ADLL_VALUE               (0x00U)

/*-------------------------------------------------------------------------*
 * Constants:
 *-------------------------------------------------------------------------*/
#define ADC_CHANNEL_0         0
#define ADC_CHANNEL_1         1
#define ADC_CHANNEL_2         2
#define ADC_CHANNEL_3         3
#define ADC_CHANNEL_4         4
#define ADC_CHANNEL_5         5
#define ADC_CHANNEL_6         6
#define ADC_CHANNEL_7         7

/*-------------------------------------------------------------------------*
 * Prototypes:
 *-------------------------------------------------------------------------*/
void ADC_Start(void);
void ADC_SelectChannel(uint8_t channel);
uint32_t ADC_GetReading();

#endif // ADC_H_
/*-------------------------------------------------------------------------*
 * End of File:  ADC.h
 *-------------------------------------------------------------------------*/
