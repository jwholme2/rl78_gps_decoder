/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only 
* intended for use with Renesas products. No other uses are authorized. This 
* software is owned by Renesas Electronics Corporation and is protected under 
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING 
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT 
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
* AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR 
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE 
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software 
* and to discontinue the availability of this software.  By using this software, 
* you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2011, 2013 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_cg_serial_user.c
* Version      : Applilet3 for RL78/G14 V2.01.00.02 [09 Aug 2013]
* Device(s)    : R5F104PJ
* Tool-Chain   : IAR Systems iccrl78
* Description  : This file implements device driver for Serial module.
* Creation Date: 2/14/2014
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_serial.h"
/* Start user code for include. Do not edit comment generated here */
#include "queues.h"
#include "rtc_sched.h"
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
extern uint8_t * gp_uart1_tx_address;         /* uart1 send buffer address */
extern uint16_t  g_uart1_tx_count;            /* uart1 send data number */
extern uint8_t * gp_uart1_rx_address;         /* uart1 receive buffer address */
extern uint16_t  g_uart1_rx_count;            /* uart1 receive data number */
extern uint16_t  g_uart1_rx_length;           /* uart1 receive data length */
extern uint8_t * gp_csi21_rx_address;         /* csi21 receive buffer address */
extern uint16_t  g_csi21_rx_length;           /* csi21 receive data length */
extern uint16_t  g_csi21_rx_count;            /* csi21 receive data count */
extern uint8_t * gp_csi21_tx_address;         /* csi21 send buffer address */
extern uint16_t  g_csi21_send_length;         /* csi21 send data length */
extern uint16_t  g_csi21_tx_count;            /* csi21 send data count */
/* Start user code for global. Do not edit comment generated here */
volatile uint8_t G_UART_SendingData = 0;
volatile uint8_t G_UART_ReceivingData = 0;
volatile uint8_t G_SPI_SendingData = 0;
volatile uint8_t G_SPI_ReceivingData = 0;
volatile uint8_t G_IIC_SendingData = 0;
volatile uint8_t G_IIC_ReceivingData = 0;
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: r_uart1_interrupt_receive
* Description  : This function is INTSR1 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
#pragma vector = INTSR1_vect
__interrupt static void r_uart1_interrupt_receive(void)
{
    uint8_t rx_data;
    uint8_t err_type;
    
    //If error occured, handle
    err_type = (uint8_t)(SSR03 & 0x0007U);
    SIR03 = (uint16_t)err_type;

    if (err_type != 0U)
    {
      //call error handler, but does nothing as our FSM will handle errors
        r_uart1_callback_error(err_type);
    }
    
    //If we get here, no errors    
    rx_data = RXD1;
    
    //If this is the end character we are looking for, 
    //go ahead and call handler
    if (rx_data == '\r') {
      r_uart1_callback_receiveend();
    }
      
    //If the enqueue fails
    if ((Q_Enqueue(&rx_q, (unsigned char) rx_data))==0) {
      //call overflow, but do nothing as our FSM will take care of this
      r_uart1_callback_softwareoverrun(rx_data);
    } 
     
}

/***********************************************************************************************************************
* Function Name: r_uart1_interrupt_send
* Description  : This function is INTST1 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
#pragma vector = INTST1_vect
__interrupt static void r_uart1_interrupt_send(void)
{
  
  if (!Q_Empty(&tx_q) ) {
      TXD1 = Q_Dequeue(&tx_q);
  } else {
        r_uart1_callback_sendend();
    }
  
}

/***********************************************************************************************************************
* Function Name: r_uart1_callback_receiveend
* Description  : This function is a callback function when UART1 finishes reception.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_uart1_callback_receiveend(void)
{
    /* Start user code. Do not edit comment generated here */
  //JWH
  //We received all the data...
  //Tell the scheduler to run the decodeTask
  Run_TaskN(1);
  G_UART_ReceivingData=0;
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_uart1_callback_softwareoverrun
* Description  : This function is a callback function when UART1 receives an overflow data.
* Arguments    : rx_data -
*                    receive data
* Return Value : None
***********************************************************************************************************************/
static void r_uart1_callback_softwareoverrun(uint16_t rx_data)
{
    /* Start user code. Do not edit comment generated here */
    //Do nothing
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_uart1_callback_sendend
* Description  : This function is a callback function when UART1 finishes transmission.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_uart1_callback_sendend(void)
{
    /* Start user code. Do not edit comment generated here */
  G_UART_SendingData = 0;
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_uart1_callback_error
* Description  : This function is a callback function when UART1 reception error occurs.
* Arguments    : err_type -
*                    error type value
* Return Value : None
***********************************************************************************************************************/
static void r_uart1_callback_error(uint8_t err_type)
{
    /* Start user code. Do not edit comment generated here */
  //Do nothing. 
    /* End user code. Do not edit comment generated here */
}


/***********************************************************************************************************************
* Function Name: r_csi21_interrupt
* Description  : This function is INTCSI21 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
#pragma vector = INTCSI21_vect
__interrupt static void r_csi21_interrupt(void)
{
    uint8_t err_type;
    uint8_t sio_dummy;

    err_type = (uint8_t)(SSR11 & _0001_SAU_OVERRUN_ERROR);
    SIR11 = (uint16_t)err_type;

    if (1U == err_type)
    {
        r_csi21_callback_error(err_type);    /* overrun error occurs */
    }
    else
    {
        if (g_csi21_tx_count > 0U)
        {
            if (0U != gp_csi21_rx_address)
            {
                *gp_csi21_rx_address = SIO21;
                gp_csi21_rx_address++;
            }
            else
            {
                sio_dummy = SIO21;
            }

            if (0U != gp_csi21_tx_address)
            {
                SIO21 = *gp_csi21_tx_address;
                gp_csi21_tx_address++;
            }
            else
            {
                SIO21 = 0xFFU;
            }

            g_csi21_tx_count--;
        }
        else 
        {
            if (0U == g_csi21_tx_count)
            {
                if (0U != gp_csi21_rx_address)
                {
                    *gp_csi21_rx_address = SIO21;
                }
                else
                {
                    sio_dummy = SIO21;
                }
            }

            r_csi21_callback_sendend();    /* complete send */
            r_csi21_callback_receiveend();    /* complete receive */
        }
    }
}

/***********************************************************************************************************************
* Function Name: r_csi21_callback_receiveend
* Description  : This function is a callback function when CSI21 finishes reception.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_csi21_callback_receiveend(void)
{
    /* Start user code. Do not edit comment generated here */
	   G_SPI_ReceivingData = 0;
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_csi21_callback_error
* Description  : This function is a callback function when CSI21 reception error occurs.
* Arguments    : err_type -
*                    error type value
* Return Value : None
***********************************************************************************************************************/
static void r_csi21_callback_error(uint8_t err_type)
{
    /* Start user code. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_csi21_callback_sendend
* Description  : This function is a callback function when CSI21 finishes transmission.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_csi21_callback_sendend(void)
{
    /* Start user code. Do not edit comment generated here */
	  G_SPI_SendingData = 0;
		/* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */





/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
