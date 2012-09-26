/******************************************************************************
* DISCLAIMER

* This software is supplied by Renesas Electronics Corp. and is
* only intended for use with Renesas products.  

* No other uses are authorized.

* This software is owned by Renesas Electronics Corp. and is 
* protected under the applicable laws, including copyright laws.

* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES
* REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY,
* INCLUDING BUT NOT LIMITED TO WAWRRANTIES OF MERCHANTABILITY, 
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
/* Copyright (C) 2010. Renesas Electronics Corp., All Rights Reserved       */
/******************************************************************************
* File Name     : lcd.h
* Version       : 1.1
* Device(s)     : R5F562N8
* Tool-Chain    : Renesas RX Standard Toolchain 1.0.1
* OS            : None
* H/W Platform  : YRDKRX62N
* Description   : Provides variable and function delarations for LCD.c file.
* Limitations   : None
*******************************************************************************
* History : DD.MMM.YYYY     Version     Description
*         : 01.Sep.2010     1.00        First release
*         : 02.Dec.2010     1.10        Second YRDK release
*******************************************************************************/

/* Multiple inclusion prevention macro */
#ifndef LCD_H
#define LCD_H

/*******************************************************************************
* Project Includes
*******************************************************************************/
/* Defines standard integer variable types used in this file */
#include <stdint.h>

/*******************************************************************************
* Macro Definitions
*******************************************************************************/
#define LCD_LINE1       0
#define LCD_LINE2       8
#define LCD_LINE3       16
#define LCD_LINE4       24
#define LCD_LINE5       32
#define LCD_LINE6       40
#define LCD_LINE7       48
#define LCD_LINE8       56

/******************************************************************************
* Global Function Prototypes
******************************************************************************/
/* LCD initialisation function declaration */
void InitialiseLCD(void);
/* Update display function declaration */
void DisplayLCD(uint8_t, const uint8_t *);
/* LCD write function delcaration */
void LCD_write(uint8_t, uint8_t);
/* Clear LCD function delcaration */
void ClearLCD(void);

/* End of multiple inclusion prevention macro */
#endif
