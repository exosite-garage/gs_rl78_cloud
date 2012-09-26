/*-------------------------------------------------------------------------*
 * File:  UART2.c
 *-------------------------------------------------------------------------*
 * Description:
 *     FIFO driven UART2 driver for RL78.
 *-------------------------------------------------------------------------*/
#include <system/platform.h>
#include "SAU.h"
#include "UART2.h"

/*-------------------------------------------------------------------------*
 * Constants:
 *-------------------------------------------------------------------------*/
#ifndef UART2_RX_BUFFER_SIZE
#define UART2_RX_BUFFER_SIZE        128
#endif

#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE        128
#endif

#ifndef UART2_TX_INTERRUPT_PRIORITY
#define UART2_TX_INTERRUPT_PRIORITY 1U   // Low
#endif

#ifndef UART2_RX_INTERRUPT_PRIORITY
#define UART2_RX_INTERRUPT_PRIORITY 1U  // Low
#endif

#ifndef UART2_RE_INTERRUPT_PRIORITY
#define UART2_RE_INTERRUPT_PRIORITY 1U  // Low
#endif

/*-------------------------------------------------------------------------*
 * Globals:
 *-------------------------------------------------------------------------*/
/* Transmit FIFO buffer */
static uint8_t G_UART2_RXBuffer[UART2_RX_BUFFER_SIZE];
static uint16_t G_UART2_RXIn = 0;
static uint16_t G_UART2_RXOut = 0;

/* Transmit FIFO buffer */
static uint8_t G_UART2_TXBuffer[UART2_TX_BUFFER_SIZE];
static uint16_t G_UART2_TXIn = 0;
static uint16_t G_UART2_TXOut = 0;
static volatile bool G_UART2_TX_Empty;

static volatile T_SAUStatusError G_UART2_LastError = NONE;
static volatile uint16_t G_UART2_ErrorCount = 0;

/*---------------------------------------------------------------------------*
 * Routine:  I2C_SetSpeed
 *---------------------------------------------------------------------------*
 * Description:
 *      Set the I2C speed in kHz.
 * Inputs:
 *      uint16_t aSpeed -- kHz speed of I2C bus.
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void UART2_SetBaudRate(uint32_t baud)
{
    uint16_t fCLK_devisor;
    uint32_t baud_devisor;
    
    /*  Calculate division ratio of the operation clock to be stored in bits 15:9 
    of the SDRmn register. */
    
    baud_devisor = ((RL78_MAIN_SYSTEM_CLOCK / baud / 2) - 1);
    
    /* increase the fCLK devisor each time the baud rate is divided until it fits */
    for(fCLK_devisor = 0; fCLK_devisor<12; fCLK_devisor++)
    {
        /* check if baud_devisor is greater than 7 bits */
        if(baud_devisor > 127)
        {
            baud_devisor = baud_devisor/2;
        }
        else
        {
            SPS1 = fCLK_devisor & 0xF;   /* Serial clock select register */
            SDR10 = baud_devisor<<9;
            SDR11 = baud_devisor<<9;
            break;
        }
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  UART2_Start
 *---------------------------------------------------------------------------*
 * Description:
 *      Each time the timer fires off an interrupt, this routine catches it
 *      and passes it to the given interrupt service routine.
 * Inputs:
 *      uint32_t baud -- baud rate (e.g. 115200 baud), or bits per second
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void UART2_Start(uint32_t baud)
{
    /* Reset FIFO buffers */
    G_UART2_RXIn = G_UART2_RXOut = 0;
    G_UART2_TXIn = G_UART2_TXOut = 0;
    G_UART2_TX_Empty = true;  
  
    /* supply SAU0 clock */
    SAU1EN = 1U;
    NOP();
    NOP();
    NOP();
    NOP();
  
    /* disable UART2 receive and transmit */
    ST1 |= _SAU_CH1_STOP_TRG_ON | _SAU_CH0_STOP_TRG_ON;    
    
    /* Turn Off Interrupts */
    STMK2 = 1U;    /* disable INTST0 interrupt */
    STIF2 = 0U;    /* clear INTST0 interrupt flag */
    SRMK2 = 1U;    /* disable INTSR0 interrupt */
    SRIF2 = 0U;    /* clear INTSR0 interrupt flag */
    SREMK2 = 1U;    /* disable INTSRE0 interrupt */
    SREIF2 = 0U;    /* clear INTSRE0 interrupt flag */
    
    STPR12 = UART2_TX_INTERRUPT_PRIORITY;   /* Set INTST0 priority */
    STPR02 = UART2_TX_INTERRUPT_PRIORITY;
    SRPR12 = UART2_RX_INTERRUPT_PRIORITY;   /* Set INTSR0 priority */
    SRPR02 = UART2_RX_INTERRUPT_PRIORITY;
    SREPR12 = UART2_RE_INTERRUPT_PRIORITY;   /* Set INTSRE0 priority */
    SREPR02 = UART2_RE_INTERRUPT_PRIORITY;    
    
    /* Configure TX0 */
    SMR10 = _SAU_SMRMN_INITIALVALUE | _SAU_CLOCK_SELECT_CK00 | _SAU_TRIGGER_SOFTWARE |
            _SAU_MODE_UART | _SAU_TRANSFER_END;
    SCR10 = _SAU_TRANSMISSION | _SAU_INTSRE_MASK | _SAU_PARITY_NONE | _SAU_LSB | _SAU_STOP_1 |
            _SAU_LENGTH_8;
    
    NFEN0 |= _SAU_RXD0_FILTER_ON;
    
    /* Configure RX0 */
    SIR11 = _SAU_SIRMN_FECTMN | _SAU_SIRMN_PECTMN | _SAU_SIRMN_OVCTMN;    /* clear error flag */
    SMR11 = _SAU_SMRMN_INITIALVALUE | _SAU_CLOCK_SELECT_CK00 | _SAU_TRIGGER_RXD | _SAU_EDGE_FALL |
            _SAU_MODE_UART | _SAU_TRANSFER_END;
    SCR11 = _SAU_RECEPTION | _SAU_INTSRE_ENABLE | _SAU_PARITY_NONE | _SAU_LSB | _SAU_STOP_1 |
            _SAU_LENGTH_8;
    
    SO1 |= _SAU_CH0_DATA_OUTPUT_1;
    SOL1 |= _SAU_CHANNEL0_NORMAL;   /* output level normal */
    SOE1 |= _SAU_CH0_OUTPUT_ENABLE;   /* enable UART2 output */
    
    UART2_SetBaudRate(baud);
    
    PM1 |= 0x10U;   /* Set RxD2 pin */
    
    P1 |= 0x08U;    /* Set TxD2 pin */
    PM1 &= 0xF7U;
    
    STIF2 = 0U;    /* clear INTST0 interrupt flag */
	STMK2 = 0U;    /* enable INTST0 interrupt */
	SRIF2 = 0U;    /* clear INTSR0 interrupt flag */
	SRMK2 = 0U;    /* enable INTSR0 interrupt */
	SREIF2 = 0U;	/* clear INTSRE0 interrupt flag */
	SREMK2 = 0U;	/* enable INTSRE0 interrupt */
	SO1 |= _SAU_CH0_DATA_OUTPUT_1;    /* output level normal */
	SOE1 |= _SAU_CH0_OUTPUT_ENABLE;    /* enable UART2 output */
	
    /* enable UART2 receive and transmit */
    SS1 |= _SAU_CH1_START_TRG_ON | _SAU_CH0_START_TRG_ON;
}

/*---------------------------------------------------------------------------*
 * Routine:  UART2_Stop
 *---------------------------------------------------------------------------*
 * Description:
 *      Stop the SCI2 from processing the serial UART by turning off the
 *      interrupt processing.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void UART2_Stop(void)
{
    /* disable UART2 receive and transmit */
    ST1 |= _SAU_CH1_STOP_TRG_ON | _SAU_CH0_STOP_TRG_ON;
    
	SOE1 &= ~_SAU_CH0_OUTPUT_ENABLE;	/* disable UART2 output */
	STMK2 = 1U;	/* disable INTST0 interrupt */
	STIF2 = 0U;	/* clear INTST0 interrupt flag */
	SRMK2 = 1U;	/* disable INTSR0 interrupt */
	SRIF2 = 0U;	/* clear INTSR0 interrupt flag */
	SREMK2 = 1U;	/* disable INTSRE0 interrupt */
	SREIF2 = 0U;	/* clear INTSRE0 interrupt flag */
}

/*---------------------------------------------------------------------------*
 * Routine:  UART2_ReceiveByte
 *---------------------------------------------------------------------------*
 * Description:
 *      Attempt to retrieve a waiting byte in the receive FIFO.  If returned,
 *      true is returned, else false.
 * Inputs:
 *      uint8_t *aByte -- Pointer to place to store received byte
 * Outputs:
 *      bool -- true if returned, else false
 *---------------------------------------------------------------------------*/
bool UART2_ReceiveByte(uint8_t *aByte)
{
    bool found = false;

    /* Disable interrupts while a check is made */
    SRMK2 = 1U;

    /* Check to see if any bytes have been placed in the FIFO and */
    /* are waiting to be pulled out. */
    if (G_UART2_RXIn != G_UART2_RXOut) {
        *aByte = G_UART2_RXBuffer[G_UART2_RXOut++];
        if (G_UART2_RXOut >= UART2_RX_BUFFER_SIZE)
            G_UART2_RXOut = 0;
        found = true;
    }

    /* Allow receive interrupts to continue processing */
    SRMK2 = 0U;

    return found;
}

/*---------------------------------------------------------------------------*
 * Routine:  UART2_SendByte
 *---------------------------------------------------------------------------*
 * Description:
 *      Put a byte in the output buffer if there is room in the transmit
 *      FIFO, or return false.
 * Inputs:
 *      uint8_t aByte -- Byte to place in output buffer
 * Outputs:
 *      bool -- true if byte placed in transmit output buffer, else false.
 *---------------------------------------------------------------------------*/
bool UART2_SendByte(uint8_t aByte)
{
    bool placed = false;
    uint16_t next;

    /* Disable transmit interrupts while touching the transmit FIFO */
    STMK2 = 1U;
        
    /* Is the transmit FIFO empty and no interrupts started? */
    if (G_UART2_TX_Empty) {
        /* Note that TX is now active and ready for more bytes */
        G_UART2_TX_Empty = false;
        TXD2 = aByte;
        placed = true;
    } else {
        /* The transmit interrupts are active and will take bytes */
        /* from the FIFO on the next interrupt. */
        /* Calculate where the next in position is in the FIFO */
        next = G_UART2_TXIn+1;
        if (next >= UART2_TX_BUFFER_SIZE)
            next = 0;

        /* Is there room in the transmit FIFO? */
        if (next != G_UART2_TXOut) {
            /* There is room, place a byte in the FIFO */
            G_UART2_TXBuffer[G_UART2_TXIn] = aByte;
            G_UART2_TXIn = next;
            placed = true;
        } else {
            /* There is no room in the FIFO, return the failed case */
            placed = false;
        }
    }

    /* Allow transmit interrupts to continue processing */
    STMK2 = 0U;

    return placed;
}

/*---------------------------------------------------------------------------*
 * Routine:  UART2_SendData
 *---------------------------------------------------------------------------*
 * Description:
 *      Attempts to send a group of data to the SCI2 transmit buffer.
 *      Returns the number of bytes actually sent.
 * Inputs:
 *      uint8_t *aData -- Pointer to bytes to send out
 *      uint32_t aLen -- Number of bytes to send
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
uint32_t UART2_SendData(const uint8_t *aData, uint32_t aLen)
{
    uint32_t i;

    /* Send each byte one at a time unless out of space */
    for (i = 0; i < aLen; ++i) {
        if (!UART2_SendByte(aData[i]))
            break;
    }

    /* Return the number of bytes that did get into the transmit FIFO */
    return i;
}

/*---------------------------------------------------------------------------*
 * Routine:  UART2_SendDataBlock
 *---------------------------------------------------------------------------*
 * Description:
 *      Send a group of data to the SCI2 transmit buffer.  Block until
 *      all data is sent.
 * Inputs:
 *      uint8_t *aData -- Pointer to bytes to send out
 *      uint32_t aLen -- Number of bytes to send
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void UART2_SendDataBlock(const uint8_t *aData, uint32_t aLen)
{
    /* Send each byte one at a time unless out of space */
    uint32_t i;

    for (i = 0; i < aLen; ++i) {
        while (!UART2_SendByte(aData[i]));
    }
}
/*---------------------------------------------------------------------------*
 * Routine:  UART2_IsTransmitEmpty
 *---------------------------------------------------------------------------*
 * Description:
 *      Determine if all bytes in the transmit FIFO buffer of SCI2 have
 *      all been sent.
 * Inputs:
 *      void
 * Outputs:
 *      bool - true if all bytes transmmitted, else false.
 *---------------------------------------------------------------------------*/
bool UART2_IsTransmitEmpty(void)
{
    /* Is the FIFO empty? */
    return G_UART2_TX_Empty;
}

/*---------------------------------------------------------------------------*
 * Interrupt Routine:  UART2_TX_ISRHandler
 *---------------------------------------------------------------------------*
 * Description:
 *      Interrupt handler to process bytes being sent from the transmit
 *      FIFO.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
#pragma vector = INTST2_vect
__interrupt void UART2_TX_ISRHandler(void)
{
    /* Clear the interrupt as the interrupt has been processed */
    STIF2 = 0U;	/* clear INTST0 interrupt flag */

    /* Is more data waiting to be sent? */
    if (G_UART2_TXIn != G_UART2_TXOut) {
        /* Send another byte and update the FIFO position */
        TXD2 = G_UART2_TXBuffer[G_UART2_TXOut++];
        if (G_UART2_TXOut >= UART2_TX_BUFFER_SIZE)
            G_UART2_TXOut = 0;
    } else {
        /* No data to send, mark transmitting as done. */
        /* This flag is needed so that the first byte is sent to */
        /* 'prime the pump' for interrupts. */
        G_UART2_TX_Empty = true;
    }
}

/*---------------------------------------------------------------------------*
 * Interrupt Routine:  UART2_RX_ISRHandler
 *---------------------------------------------------------------------------*
 * Description:
 *      Interrupt handler processes incoming bytes and places them in the
 *      receive FIFO buffer.  If there is not enough room, the bytes
 *      will be lost.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
#pragma vector = INTSR2_vect
__interrupt void UART2_RX_ISRHandler(void)
{
    uint8_t c;
    uint32_t next;

    /* Grab the byte immediately */
    c = RXD2;

    /* Is there room in the FIFO? */
    next = G_UART2_RXIn+1;
    if (next >= UART2_RX_BUFFER_SIZE)
        next = 0;
    if (next != G_UART2_RXOut) {
        /* There is room to place the byte.  Place it and move up a byte. */
        G_UART2_RXBuffer[G_UART2_RXIn] = c;
        G_UART2_RXIn = next;
    } else {
        /* The buffer is overrunning and we are losing bytes now. */
    }
    
    /* Done with the receive interrupt */
    SRIF0 = 0U;	/* clear INTSR0 interrupt flag */
}

/*---------------------------------------------------------------------------*
 * Interrupt Routine:  UART2_RXError_ISRHandler
 *---------------------------------------------------------------------------*
 * Description:
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
#pragma vector = INTSRE2_vect
__interrupt void UART2_RXError_ISRHandler(void)
{
    /* log error */
    if(SSR11 & 1)
        G_UART2_LastError = FRAMING_ERROR;
    else if(SSR11 & 2)
        G_UART2_LastError = PARITY_ERROR;
    else if(SSR11 & 4)
        G_UART2_LastError = OVERRUN_ERROR;
    else
        G_UART2_LastError = UNKNOWN_ERROR;
    
    G_UART2_ErrorCount++;
    
    /* clear error flag */
    SIR11 = _SAU_SIRMN_FECTMN | _SAU_SIRMN_PECTMN | _SAU_SIRMN_OVCTMN;
    
    /* clear INTSRE0 interrupt flag */
    SREIF2 = 0U;
}

/*-------------------------------------------------------------------------*
 * End of File:  UART2.c
 *-------------------------------------------------------------------------*/
