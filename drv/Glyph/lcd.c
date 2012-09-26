/******************************************************************************
* DISCLAIMER

* This software is supplied by Renesas Electronics Corp. and is
* only intended for use with Renesas products.  

* No other uses are authorized.

* This software is owned by Renesas Electronics Corp. and is 
* protected under the applicable laws, including copyright laws.

* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES
* REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY,
* INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  ALL SUCH
* WARRANTIES ARE EXPRESSLY DISCLAIMED.

* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER 
* RENESAS ELECTRONICS CORP. NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR AND DIRECT, INDIRECT, SPECIAL, INCIDENTAL
* OR COSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE,
* EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE
* POSSIBILITIES OF SUCH DAMAGES.

* Renesas reserves the right, without notice, to make changes to this
* software and to discontinue availability of this software.
* By using this software, you agree to the additional terms and 
* conditions found by accessing the following link:
* http://www.renesas.com/disclaimer
*******************************************************************************/
/* Copyright (C) 2010. Renesas Electronics Corp., All Rights Reserved         */
/******************************************************************************
* File Name     : lcd.c
* Version       : 1.1
* Device(s)     : R5F562N8
* Tool-Chain    : Renesas RX Standard Toolchain 1.0.1
* OS            : None
* H/W Platform  : YRDKRX62N
* Description   : LCD Module utility functions.
*                 Written for RDK Okaya graphic LCD 
*                 Porting layer for so that demo code written for KS0066u 
*                 compatible LCD Module works with Okaya display.
*******************************************************************************
* History : DD.MMM.YYYY     Version     Description
*         : 01.Sep.2010     1.00        First release
*         : 02.Dec.2010     1.10        Second YRDK release
*******************************************************************************/

/******************************************************************************
* Project Includes
******************************************************************************/
/* Standard string manipulation & formatting functions */
#include <stdio.h>
#include <string.h>
/* Defines standard variable types used in this function */
#include <stdint.h>
/* Following header file provides function prototypes for LCD controlling
   functions & macro defines */
#include "lcd.h"
/* Graphics library support */
#include "Glyph_API.h"
#include "Glyph_cfg.h"


/******************************************************************************
Private global variables and functions
******************************************************************************/
T_glyphHandle G_lcd ;

/******************************************************************************
* Local Function Prototypes
******************************************************************************/

/******************************************************************************
* Function name : InitialiseLCD
* Description   : Initializes the LCD display. 
* Arguments     : none
* Return Value  : none
******************************************************************************/
void InitialiseLCD(void)
{
    if (GlyphOpen(&G_lcd, 0) == GLYPH_ERROR_NONE)
    {
        /* use Glyph full access direct functions */
        GlyphNormalScreen(G_lcd) ;
        GlyphSetFont(G_lcd, GLYPH_FONT_8_BY_8_SUBSET) ;
        GlyphClearScreen(G_lcd) ;
    }
}
/******************************************************************************
* End of function InitialiseDisplay
******************************************************************************/

/******************************************************************************
* Function name : ClearLCD
* Description   : Clears the LCD
* Arguments     : none
* Return Value  : none
******************************************************************************/
void ClearLCD(void)
{
    GlyphClearScreen(G_lcd) ;
}
/******************************************************************************
* End of function ClearLCD
******************************************************************************/

/******************************************************************************
* Function name : DisplayLCD
* Description   : This function controls the LCD writes.
*                 The display supports 8 lines with up to 12 characters per line
*                 Use the defines LCD_LINE1 to LCD_LINE8 to specfify the starting
*                 position.
*                 For example, to start at the 4th position on line 1:
*                     DisplayLCD(LCD_LINE1 + 4, "Hello")
* Arguments     : uint8_t position - line number of display
*                 const unit8_t * string - pointer to null terminated string
* Return Value  : none
******************************************************************************/
void DisplayLCD(uint8_t position, const uint8_t * string)
{
    uint8_t y = position - (position % 8);
    uint8_t xOffset = (position % 8)<<3 ;

    /* Draw text lines, 8 pixels high, 96 pixels wide */
    /* Clear the rectangle of this line */
    GlyphEraseBlock(G_lcd, xOffset, y, (95 - xOffset), y+7);
    GlyphSetXY(G_lcd, xOffset, y);
    GlyphString(G_lcd, (uint8_t *)string, strlen((void *)string));

}
/******************************************************************************
* End of function DisplayString
******************************************************************************/

