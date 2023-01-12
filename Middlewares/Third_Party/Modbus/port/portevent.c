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
 * File: $Id: portevent.c,v 1.60 2013/08/13 15:07:05 Armink $
 */

/* ----------------------- Modbus includes ----------------------------------*/

#include "mb.h"
#include "mbconfig.h"



/* ----------------------- Static variables ---------------------------------*/

static osEventFlagsId_t eMB_OsEvent;



/* ----------------------- Start implementation -----------------------------*/

BOOL xMBPortEventInit(void)
{
  eMB_OsEvent = osEventFlagsNew(NULL);

  if (eMB_OsEvent == NULL)
  {
    MODBUS_DEBUG("xMBPortEventInit ERR=%d!\r\n", eMB_OsEvent);
    return FALSE;
  }
  
  return TRUE;
}

BOOL xMBPortEventPost(eMBEventType eEvent)
{
  MODBUS_DEBUG("Set event=%d!\r\n", eEvent);
  osEventFlagsSet(eMB_OsEvent, eEvent);
  
  return TRUE;
}

BOOL xMBPortEventGet(eMBEventType *eEvent)
{
  uint32_t recvEvent;
  
  /* waiting forever OS event */
  MODBUS_DEBUG("Wait for event...\r\n");
  recvEvent = osEventFlagsWait(eMB_OsEvent, EV_READY | EV_FRAME_RECEIVED | EV_EXECUTE | EV_FRAME_SENT,
                                 osFlagsWaitAny, MODBUS_EVENT_TIMEOUT);
  MODBUS_DEBUG("Received event=%d\r\n", recvEvent);
  
  /* the enum type couldn't convert to int type */
  switch (recvEvent)
  {
    case EV_READY:
      *eEvent = EV_READY;
      break;
    case EV_FRAME_RECEIVED:
      *eEvent = EV_FRAME_RECEIVED;
      break;
    case EV_EXECUTE:
      *eEvent = EV_EXECUTE;
      break;
    case EV_FRAME_SENT:
      *eEvent = EV_FRAME_SENT;
      break;
    default:
      return FALSE;
  }
  
  return TRUE;
}
