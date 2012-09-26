/*-------------------------------------------------------------------------*
 * File:  I2C.c
 *-------------------------------------------------------------------------*
 * Description:
 *      Simple implementation of the Renesas RL78 I2C Interface.
 *-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Includes:
 *---------------------------------------------------------------------------*/
#include <system/platform.h>
#include "I2C.h"

/*---------------------------------------------------------------------------*
 * Types:
 *---------------------------------------------------------------------------*/
typedef enum {
    I2C_STATE_START,
    I2C_STATE_ADDR_COMPLETE
} T_i2cState;

typedef enum {
    I2C_INTERRUPT_ICEEI,
    I2C_INTERRUPT_ICRXI,
    I2C_INTERRUPT_ICTXI,
    I2C_INTERRUPT_ICTEI,
} T_i2cInterrupt;

/*-------------------------------------------------------------------------*
 * Constants:
 *-------------------------------------------------------------------------*/
#define BIT_TDRE    (1<<7)
#define BIT_TEND    (1<<6)
#define BIT_RDRF    (1<<5)
#define BIT_NACKF   (1<<4)
#define BIT_STOP    (1<<3)
#define BIT_START   (1<<2)
#define BIT_AL      (1<<1)
#define BIT_TMOF    (1<<0)

#define I2C_MODE_READ           1
#define I2C_MODE_WRITE          0

/*-------------------------------------------------------------------------*
 * Constants:
 *-------------------------------------------------------------------------*/
#define DEBUG_I2C_TRACK_INTERRUPTS 0

/*-------------------------------------------------------------------------*
 * Globals:
 *-------------------------------------------------------------------------*/
static I2C_Request *G_I2C_Request;
static uint8_t G_I2C_Address;
static uint8_t *G_I2C_Data;
static uint8_t G_I2C_DataLen;
static uint8_t G_I2C_TXCount;
static uint8_t G_I2C_RXCount;
static void (*G_I2C_CompleteFunc)(void);
static T_i2cState G_I2C_State;
static volatile bool G_I2C_DoneFlag;

/*-------------------------------------------------------------------------*
 * Function Prototypes:
 *-------------------------------------------------------------------------*/
static void I2C_MasterHandler();

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
void I2C_SetSpeed(uint16_t aSpeed)
{
    uint32_t low, high;
    
    low = ((47 * RL78_MAIN_SYSTEM_CLOCK) / 100000) / aSpeed;
    high = ((53 * RL78_MAIN_SYSTEM_CLOCK) / 100000) / aSpeed;
    
    IICWL0 = (uint8_t)low;
    IICWH0 = (uint8_t)high;
}

/*---------------------------------------------------------------------------*
 * Routine:  I2C_Start
 *---------------------------------------------------------------------------*
 * Description:
 *      Start the I2C bus.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void I2C_Start(void)
{      
    IICA0EN = 1U; /* supply IICA0 clock */
    IICE0 = 0U; /* disable IICA0 operation */
    IICAMK0 = 1U; /* disable INTIICA0 interrupt */
    IICAIF0 = 0U; /* clear INTIICA0 interrupt flag */
    
    /* Set INTIICA0 low priority */
    IICAPR10 = 1U;
    IICAPR00 = 1U;
    
    /* Set SCLA0, SDAA0 pin */
    P6 &= 0xFCU;
    PM6 |= 0x03U;
    SMC0 = 0U;
    IICCTL01 &= (uint8_t)~_IICA_fCLK_HALF;
    SVA0 = _IICA0_MASTERADDRESS;
    STCEN0 = 1U;
    IICRSV0 = 1U;
    SPIE0 = 0U;
    WTIM0 = 1U;
    ACKE0 = 1U;
    IICAMK0 = 0U;
    IICE0 = 1U;
    LREL0 = 1U;
    
    /* Set SCLA0, SDAA0 pin */
    PM6 &= 0xFCU;
}

/*---------------------------------------------------------------------------*
 * Routine:  I2C_Stop
 *---------------------------------------------------------------------------*
 * Description:
 *      Stop the I2C bus.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void I2C_Stop(void)
{
    IICE0 = 0U;    /* disable IICA0 operation */
}

/*---------------------------------------------------------------------------*
 * Routine:  I2C_Write
 *---------------------------------------------------------------------------*
 * Description:
 *      Write out an I2C request and then call the callback function
 *      when complete (success or failure).
 * Inputs:
 *      I2C_Request *iRequest -- I2C request structure with write info
 *      void (*callback)(void) -- Routine to call upon completion.
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void I2C_Write(I2C_Request *iRequest, void(*callback)(void))
{
    uint16_t wait;

    /* Don't process if the bus is busy! */
    if (1U == IICBSY0)
    {
        /* Check bus busy */
        IICAMK0 = 0U;  /* enable INTIIA0 interrupt */
        G_I2C_Request->iStatus = I2C_NOT_READY;
        return;
    } 
    
    /* Don't process if the start or stop triggers are set! */
    if ((1U == SPT0) || (1U == STT0))
    {
        /* Check trigger */
        IICAMK0 = 0U;  /* enable INTIICA0 interrupt */
        G_I2C_Request->iStatus = I2C_ERROR;
        return;
    }
    
    G_I2C_Request = iRequest;
    G_I2C_CompleteFunc = callback;
    G_I2C_Address = (G_I2C_Request->iAddr << 1) | I2C_MODE_WRITE;
    G_I2C_Data = (uint8_t *)G_I2C_Request->iWriteData;
    G_I2C_DataLen = G_I2C_Request->iWriteLength;
    G_I2C_TXCount = G_I2C_Request->iWriteLength;
    G_I2C_Request->iStatus = I2C_BUSY;
    
    /* Set the speed */
    I2C_SetSpeed(G_I2C_Request->iSpeed);
    
    STT0 = 1U; /* send IICA0 start condition */
    IICAMK0 = 0U;  /* enable INTIICA0 interrupt */
    
    /* Wait a bit of time to if busy or low sda line */
    wait = 100;
    while (wait){
        wait--;
    }

    G_I2C_State = I2C_STATE_START;
    IICA0 = G_I2C_Address; /* send address */
}

/*---------------------------------------------------------------------------*
 * Routine:  I2C_Read
 *---------------------------------------------------------------------------*
 * Description:
 *      Do a I2C request for read and then call the callback function
 *      when complete (success or failure).
 * Inputs:
 *      I2C_Request *iRequest -- I2C request structure with read info
 *      void (*callback)(void) -- Routine to call upon completion.
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void I2C_Read(I2C_Request *iRequest, void(*callback)(void))
{
    uint16_t wait;

    /* Don't process if the bus is busy! */
    if (1U == IICBSY0)
    {
        /* Check bus busy */
        IICAMK0 = 0U;  /* enable INTIIA0 interrupt */
        G_I2C_Request->iStatus = I2C_NOT_READY;
        return;
    } 
    
    /* Don't process if the start or stop triggers are set! */
    if ((1U == SPT0) || (1U == STT0))
    {
        /* Check trigger */
        IICAMK0 = 0U;  /* enable INTIICA0 interrupt */
        G_I2C_Request->iStatus = I2C_ERROR;
        return;
    }
    
    G_I2C_Request = iRequest;
    G_I2C_CompleteFunc = callback;
    G_I2C_Address = (G_I2C_Request->iAddr << 1) | I2C_MODE_READ;
    G_I2C_Data = G_I2C_Request->iReadData;
    G_I2C_DataLen = G_I2C_Request->iReadLength;
    G_I2C_RXCount = 0;
    G_I2C_Request->iStatus = I2C_BUSY;

    /* Set the speed */
    I2C_SetSpeed(G_I2C_Request->iSpeed);
    
    STT0 = 1U; /* set IICA0 start condition */
    IICAMK0 = 0U;  /* enable INTIIA0 interrupt */
    
    /* Wait a bit of time to if busy or low sda line */
    wait = 100;
    while (wait){
        wait--;
    }
    
    /* Set parameter */
    G_I2C_State = I2C_STATE_START;
    IICA0 = G_I2C_Address; /* receive address */
}

/*---------------------------------------------------------------------------*
 * Routine:  I2C_IsBusy
 *---------------------------------------------------------------------------*
 * Description:
 *      Return true if I2C bus is busy with any transaction.
 * Inputs:
 *      void
 * Outputs:
 *      bool -- true if busy, else false
 *---------------------------------------------------------------------------*/
bool I2C_IsBusy(void)
{
    return (1U == IICBSY0) ? true : false;
}

/*---------------------------------------------------------------------------*
 * Routine:  I2C_InterruptHandler
 *---------------------------------------------------------------------------*
 * Description:
 *      This function is INTIICA0 interrupt service routine.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
#pragma vector = INTIICA0_vect
__interrupt static void I2C_InterruptHandler(void)
{
    if ((IICS0 & _IICA_STATUS_MASTER) == 0x80U)
    {
        I2C_MasterHandler();
    }
}

/*---------------------------------------------------------------------------*
 * Routine:  I2C_MasterHandler
 *---------------------------------------------------------------------------*
 * Description:
 *      This function is IICA0 master handler.
 * Inputs:
 *      void
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
static void I2C_MasterHandler(void)
{
    /* Control for communication */
    if ((0U == IICBSY0) && (G_I2C_TXCount != 0U))
    {
        G_I2C_Request->iStatus = I2C_STOP;
    }
    /* Control for sended address */
    else
    {
        if (G_I2C_State != I2C_STATE_ADDR_COMPLETE)
        {
            if (1U == ACKD0)
            {
                G_I2C_State = I2C_STATE_ADDR_COMPLETE;
                
                /* Master send control */
                if (1U == TRC0)
                {
                    WTIM0 = 1U;
                    
                    if (G_I2C_TXCount > 0U)
                    {
                        IICA0 = *G_I2C_Data;
                        G_I2C_Data++;
                        G_I2C_TXCount--;
                    }
                    else
                    {
                        /* Send End */
                        SPT0 = 1U;  /* trigger stop condition */
                        if (G_I2C_CompleteFunc)
                            G_I2C_CompleteFunc();
                        
                        G_I2C_Request->iStatus = I2C_OK;
                    }
                }
                /* Master receive control */
                else
                {
                    ACKE0 = 1U;
                    WTIM0 = 0U;
                    WREL0 = 1U;
                }
            }
            else
            {
                G_I2C_Request->iStatus = I2C_NAK;
            }
        }
        else
        {
            /* Master send control */
            if (1U == TRC0)
            {
                if ((0U == ACKD0) && (G_I2C_TXCount != 0U))
                {
                    G_I2C_Request->iStatus = I2C_NAK;
                }
                else
                {
                    if (G_I2C_TXCount > 0U)
                    {
                        IICA0 = *G_I2C_Data;
                        G_I2C_Data++;
                        G_I2C_TXCount--;
                    }
                    else
                    {
                        /* Send End */
                        SPT0 = 1U;  /* trigger stop condition */
                        if (G_I2C_CompleteFunc)
                            G_I2C_CompleteFunc();
                        
                        G_I2C_Request->iStatus = I2C_OK;
                    }
                }
            }
            /* Master receive control */
            else
            {
                if (G_I2C_RXCount < G_I2C_DataLen)
                {
                    *G_I2C_Data = IICA0;
                    G_I2C_Data++;
                    G_I2C_RXCount++;
                    
                    if (G_I2C_RXCount == G_I2C_DataLen)
                    {
                        ACKE0 = 0U;
                        WREL0 = 1U;
                        WTIM0 = 1U;
                    }
                    else
                    {
                        WREL0 = 1U;
                    }
                }
                else
                {
                    /* Receive End */
                    SPT0 = 1U;  /* trigger stop condition */
                    if (G_I2C_CompleteFunc)
                        G_I2C_CompleteFunc();
                    
                    G_I2C_Request->iStatus = I2C_OK;
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------*
 * End of File:  I2C.c
 *-------------------------------------------------------------------------*/
