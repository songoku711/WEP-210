/*
 * FreeModbus Libary: RT-Thread Port
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial.c,v 1.60 2013/08/13 15:07:05 Armink $
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbconfig.h"

#include "usart.h"



/* ----------------------- Macro definitions --------------------------------*/

#define eMB_PORT_SERIAL_INSTANCE                  &huart1

#define eMB_PORT_SERIAL_RXBUF_SIZE                100U

/* serial transmit event */
#define EVENT_SERIAL_TRANS_START                  0x01

#define eMB_PORT_SERIAL_TX_MODE()                 { RS485_CS_GPIO_Port->ODR |= (uint32_t)RS485_CS_Pin; }
#define eMB_PORT_SERIAL_RX_MODE()                 { RS485_CS_GPIO_Port->ODR &= (uint32_t)(~((uint32_t)RS485_CS_Pin)); }

typedef struct
{
  uint8_t *buffer;
  uint16_t get_index;
  uint16_t put_index;
} Serial_fifo;



/* ----------------------- Static variables ---------------------------------*/

/* Serial hardware instance */
static UART_HandleTypeDef* eMB_pUartIns;

/* Serial event */
static osEventFlagsId_t eMB_PortSerialTxEvent;

/* Serial Tx task */
osThreadId_t eMB_PortSerialTxTaskHandle;

const osThreadAttr_t eMB_PortSerialTxTask_attributes = {
  .name = "eMBTxTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal7,
};

static uint8_t eMB_recvData;



/* ----------------------- Static functions ---------------------------------*/

static void eMB_PortSerialTxTask(void *parameter);
static void eMB_PortSerialRxCpltCallback(struct __UART_HandleTypeDef *huart);



/* ----------------------- Start implementation -----------------------------*/

BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
  /* Set serial instance */
  eMB_pUartIns = eMB_PORT_SERIAL_INSTANCE;
  
  /* Disable serial interrupt and register interrupt callback */
  __HAL_UART_DISABLE_IT(eMB_pUartIns, UART_IT_RXNE);
  __HAL_UART_DISABLE_IT(eMB_pUartIns, UART_IT_TC);
  HAL_UART_RegisterCallback(eMB_pUartIns, HAL_UART_RX_COMPLETE_CB_ID, eMB_PortSerialRxCpltCallback);
  
  /* Create serial transmitter event */
  eMB_PortSerialTxEvent = osEventFlagsNew(NULL);
  
  /* Create serial transmitter task */
  eMB_PortSerialTxTaskHandle = osThreadNew(eMB_PortSerialTxTask, NULL, &eMB_PortSerialTxTask_attributes);
  
  eMB_PORT_SERIAL_RX_MODE();
  
  return TRUE;
}

void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
  if (xRxEnable)
  {
    /* Enable RX interrupt */
    __HAL_UART_ENABLE_IT(eMB_pUartIns, UART_IT_RXNE);
    
    /* Switch 485 to receive mode */
    eMB_PORT_SERIAL_RX_MODE();
  }
  else
  {
    /* Switch 485 to transmit mode */
    eMB_PORT_SERIAL_TX_MODE();
    
    /* Disable RX interrupt */
    __HAL_UART_DISABLE_IT(eMB_pUartIns, UART_IT_RXNE);
  }
  
  if (xTxEnable)
  {
    /* Start serial transmit */
    osEventFlagsSet(eMB_PortSerialTxEvent, EVENT_SERIAL_TRANS_START);
  }
  else
  {
    /* Stop serial transmit */
    osEventFlagsClear(eMB_PortSerialTxEvent, EVENT_SERIAL_TRANS_START);
  }
}

void vMBPortClose(void)
{
  /* Do shit here */
}

BOOL xMBPortSerialPutByte(CHAR ucByte)
{
  while (!(eMB_pUartIns->Instance->SR & UART_FLAG_TXE));
  eMB_pUartIns->Instance->DR = ucByte;
  while (!(eMB_pUartIns->Instance->SR & UART_FLAG_TC));

  return TRUE;
}

BOOL xMBPortSerialGetByte(CHAR * pucByte)
{
  *pucByte = eMB_recvData;

  return TRUE;
}



/**
 * Software simulation serial transmit IRQ handler.
 *
 * @param parameter parameter
 */
static void eMB_PortSerialTxTask(void *parameter)
{
  while (1)
  {
    /* waiting for serial transmit start */
    osEventFlagsWait(eMB_PortSerialTxEvent, EVENT_SERIAL_TRANS_START, osFlagsWaitAny | osFlagsNoClear, osWaitForever);
    
    /* execute modbus callback */
    pxMBFrameCBTransmitterEmpty();
  }
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void eMB_PortSerialRxCpltCallback(UART_HandleTypeDef *huart)
{
  if (eMB_pUartIns->Instance->SR & UART_FLAG_RXNE)
  {
    eMB_recvData = eMB_pUartIns->Instance->DR & (uint8_t)0xFFU;

    /* execute modbus callback */
    pxMBFrameCBByteReceived();
  }
}
