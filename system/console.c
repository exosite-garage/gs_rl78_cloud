/*-------------------------------------------------------------------------*
 * File:  console.c
 *-------------------------------------------------------------------------*
 * Description:
 *     
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*
 * Includes:
 *-------------------------------------------------------------------------*/
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <drv/UART0.h>

/*---------------------------------------------------------------------------*
 * Routine:  ConsolePrintf
 *---------------------------------------------------------------------------*
 * Description:
 *      Output a formatted string of text to the console.
 * Inputs:
 *      const char *format -- Format of string to output
 *      ... -- 0 or more parameters
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void ConsolePrintf(const char *format, ...)
{
    static volatile uint8_t buffer[256];
    volatile uint8_t *p;
    va_list args;

    /* Start processing the arguments */
    va_start(args, format);

    /* Output the parameters into a string */
    vsprintf((char *)buffer, format, args);

    /* Output the string to the console */
    p = buffer;
    while (*p) {
        if (*p == '\n')
            UART0_SendByte('\r');
        UART0_SendByte(*p);
        if (*p == '\n')
            while (!UART0_IsTransmitEmpty()) {
            }
        p++;
    }

    /* End processing of the arguments */
    va_end(args);
}

/*---------------------------------------------------------------------------*
 * Routine:  ConsoleSendString
 *---------------------------------------------------------------------------*
 * Description:
 *      Send a string to the console.
 * Inputs:
 *      const char *string -- string of characters to output on console
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void ConsoleSendString(const char *string)
{
    const char *p = string;

    /* Process all characters in string */
    while (*p) {
        if (*p == '\n')
            UART0_SendByte('\r');
        UART0_SendByte(*p);
        if (*p == '\n')
            while (!UART0_IsTransmitEmpty()) {
            }
        p++;
    }
}

/*-------------------------------------------------------------------------*
 * End of File:  console.c
 *-------------------------------------------------------------------------*/
