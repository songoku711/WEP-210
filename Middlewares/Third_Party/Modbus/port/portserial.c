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

/* ----------------------- Platform includes --------------------------------*/
#include "usart.h"

/* ----------------------- Macro definitions --------------------------------*/
#define MODBUS_SERIAL                       &huart1

#define MB_SIZE_MAX                         100U

/* serial transmit event */
#define EVENT_SERIAL_TRANS_START            (1 << 0)

#define RS485_TX_MODE                       { RS485_CS_GPIO_Port->ODR |= (uint32_t)RS485_CS_Pin; }
#define RS485_RX_MODE                       { RS485_CS_GPIO_Port->ODR &= (uint32_t)(~((uint32_t)RS485_CS_Pin)); }

typedef struct
{
  uint8_t *buffer;
  uint16_t get_index;
  uint16_t put_index;
} Serial_fifo;

/* ----------------------- Static variables ---------------------------------*/
/* software simulation serial transmit IRQ handler thread */
static TaskHandle_t thread_serial_soft_trans_irq = NULL;
/* serial event */
static osEventFlagsId_t event_serial;
/* modbus slave serial device */
static UART_HandleTypeDef *serial;
/*
 * Serial FIFO mode 
 */
uint8_t rx_buff[MB_SIZE_MAX];
Serial_fifo Slave_serial_rx_fifo;

/* ----------------------- Static functions ---------------------------------*/
static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);
static void serial_soft_trans_irq(void *parameter);
static void Slave_RxCpltCallback(struct __UART_HandleTypeDef *huart);
static int stm32_getc(void);
static int stm32_putc(char c);
static void Put_in_fifo(Serial_fifo *buff, uint8_t *putdata, int length);
static int Get_from_fifo(Serial_fifo *buff, uint8_t *getdata, int length);

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
  /* Set serial instance */
  (void)ucPORT;
  serial = MODBUS_SERIAL;
  
  /* Set serial configure */
  serial->Init.BaudRate = ulBaudRate;
  serial->Init.StopBits = UART_STOPBITS_1;
  
  switch (eParity)
  {
    case MB_PAR_ODD:
    {
      serial->Init.WordLength = UART_WORDLENGTH_9B;
      serial->Init.Parity = UART_PARITY_ODD;
      break;
    }
    case MB_PAR_EVEN:
    {
      serial->Init.WordLength = UART_WORDLENGTH_9B;
      serial->Init.Parity = UART_PARITY_EVEN;
      break;
    }
    case MB_PAR_NONE:
    default:
    {
      serial->Init.WordLength = UART_WORDLENGTH_8B;
      serial->Init.Parity = UART_PARITY_NONE;
      break;
    }
  }
  
  /* Serial re-initialization */
  if (HAL_UART_Init(serial) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Disable serial interrupt and register interrupt callback */
  __HAL_UART_DISABLE_IT(serial, UART_IT_RXNE);
  __HAL_UART_DISABLE_IT(serial, UART_IT_TC);
  HAL_UART_RegisterCallback(serial, HAL_UART_RX_COMPLETE_CB_ID, Slave_RxCpltCallback);

  /* software initialize */
  Slave_serial_rx_fifo.buffer = rx_buff;
  Slave_serial_rx_fifo.get_index = 0;
  Slave_serial_rx_fifo.put_index = 0;
  
  //event_serial = osEventFlagsNew(NULL); // id cannot be created with CMSISv2 interface here
  /* Create master send task */
  BaseType_t xReturn = xTaskCreate((TaskFunction_t)serial_soft_trans_irq,          /* Task entry function */
                                   (const char *)"slave trans",                    /* Task name */
                                   (uint16_t)512,                                  /* Task stack size */
                                   (void *)NULL,                                   /* Task entry function parameters */
                                   (UBaseType_t)5,                                 /* Priority of task */
                                   (TaskHandle_t *)&thread_serial_soft_trans_irq); /* Task control block pointer */

  if (xReturn == pdPASS)
  {
    MODBUS_DEBUG("xTaskCreate slave trans success\r\n");
  }
  
  RS485_RX_MODE;
  
  return TRUE;
}

void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
  if (xRxEnable)
  {
    /* Enable RX interrupt */
    __HAL_UART_ENABLE_IT(serial, UART_IT_RXNE);
    
    /* Switch 485 to receive mode */
    MODBUS_DEBUG("RS485_RX_MODE\r\n");
    RS485_RX_MODE;
  }
  else
  {
    /* Switch 485 to transmit mode */
    MODBUS_DEBUG("RS485_TX_MODE\r\n");
    RS485_TX_MODE;
    
    /* Disable RX interrupt */
    __HAL_UART_DISABLE_IT(serial, UART_IT_RXNE);
  }
  
  if (xTxEnable)
  {
    /* Start serial transmit */
    osEventFlagsSet(event_serial, EVENT_SERIAL_TRANS_START);
  }
  else
  {
    /* Stop serial transmit */
    osEventFlagsClear(event_serial, EVENT_SERIAL_TRANS_START);
  }
}

void vMBPortClose(void)
{
  HAL_UART_DeInit(serial);
}

BOOL xMBPortSerialPutByte(CHAR ucByte)
{
  stm32_putc(ucByte);
  return TRUE;
}

BOOL xMBPortSerialGetByte(CHAR * pucByte)
{
  Get_from_fifo(&Slave_serial_rx_fifo, (uint8_t*)pucByte, 1);
  return TRUE;
}

/* 
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
void prvvUARTTxReadyISR(void)
{
  pxMBFrameCBTransmitterEmpty();
}

/* 
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR(void)
{
  pxMBFrameCBByteReceived();
}

/**
 * Software simulation serial transmit IRQ handler.
 *
 * @param parameter parameter
 */
static void serial_soft_trans_irq(void* parameter)
{
  /*Create UART TX event handle */
  event_serial = osEventFlagsNew(NULL);
  
  if (NULL != event_serial)
  {
    MODBUS_DEBUG("Slave event_serial Event creat success id=%d\r\n", event_serial);
  }
  else
  {
    MODBUS_DEBUG("Slave event_serial Event creat faild err=%d\r\n", event_serial);
  }
  
  while (1)
  {
    /* waiting for serial transmit start */
    osEventFlagsWait(event_serial, EVENT_SERIAL_TRANS_START, osFlagsWaitAny | osFlagsNoClear, osWaitForever);
    /* execute modbus callback */
    prvvUARTTxReadyISR();
  }
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void Slave_RxCpltCallback(UART_HandleTypeDef *huart)
{
  int ch = -1;
  
  while (1)
  {
    ch = stm32_getc();
    if (ch == -1)
        break;
    Put_in_fifo(&Slave_serial_rx_fifo, (uint8_t *)&ch, 1);
  }
  
  prvvUARTRxISR();
}

static int stm32_putc(char c)
{
  while (!(serial->Instance->SR & UART_FLAG_TXE));
  serial->Instance->DR = c;
  while (!(serial->Instance->SR & UART_FLAG_TC));

  return TRUE;
}

static int stm32_getc(void)
{
  int ch;
  ch = -1;
  if (serial->Instance->SR & UART_FLAG_RXNE)
  {
    ch = serial->Instance->DR & 0xff;
  }
  
  return ch;
}

/*put  bytes in buff*/
static void Put_in_fifo(Serial_fifo *buff, uint8_t *putdata, int length)
{
  portDISABLE_INTERRUPTS();
  while (length--)
  {
    buff->buffer[buff->put_index] = *putdata;
    buff->put_index += 1;
    if (buff->put_index >= MB_SIZE_MAX)
      buff->put_index = 0;
    /* if the next position is read index, discard this 'read char' */
    if (buff->put_index == buff->get_index)
    {
      buff->get_index += 1;
      if (buff->get_index >= MB_SIZE_MAX)
        buff->get_index = 0;
    }
  }
  portENABLE_INTERRUPTS();
}

/*get  bytes from buff*/
static int Get_from_fifo(Serial_fifo *buff, uint8_t *getdata, int length)
{
  int size = length;
  /* read from software FIFO */
  while (length)
  {
    int ch;
    /* disable interrupt */
    portDISABLE_INTERRUPTS();
    if (buff->get_index != buff->put_index)
    {
      ch = buff->buffer[buff->get_index];
      buff->get_index += 1;
      if (buff->get_index >= MB_SIZE_MAX)
        buff->get_index = 0;
    }
    else
    {
      /* no data, enable interrupt and break out */
      portENABLE_INTERRUPTS();
      break;
    }
    *getdata = ch & 0xff;
    getdata++;
    length--;
    /* enable interrupt */
    portENABLE_INTERRUPTS();
  }
  return size - length;
}
