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
 * File: $Id: porttimer.c,v 1.60 2013/08/13 15:07:05 Armink $
 */

/* ----------------------- Modbus includes ----------------------------------*/

#include "mb.h"
#include "mbconfig.h"

#include "tim.h"



/* ----------------------- Macro definitions --------------------------------*/

#define eMB_PORT_TIMER_INSTANCE                   &htim2



/* ----------------------- Static functions ---------------------------------*/

static void eMB_PortTimerElapsedCallback(TIM_HandleTypeDef *xTimer);



/* ----------------------- Start implementation -----------------------------*/

BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
  /* Timer frequency is set to 20 kHz (period 50us) when initialization */
  MODBUS_DEBUG("Init timer!\r\n");
  
  //__HAL_TIM_SetAutoreload(eMB_PORT_TIMER_INSTANCE, (usTim1Timerout50us - 1));
  
  HAL_TIM_RegisterCallback(eMB_PORT_TIMER_INSTANCE, HAL_TIM_PERIOD_ELAPSED_CB_ID, eMB_PortTimerElapsedCallback);
  
  return TRUE;
}

void vMBPortTimersEnable()
{
  MODBUS_DEBUG("Start timer!\r\n");
  
  __HAL_TIM_SetCounter(eMB_PORT_TIMER_INSTANCE, 0);
  HAL_TIM_Base_Start_IT(eMB_PORT_TIMER_INSTANCE);
}

void vMBPortTimersDisable()
{
  MODBUS_DEBUG("Stop timer!\r\n");
  
  HAL_TIM_Base_Stop_IT(eMB_PORT_TIMER_INSTANCE);
}



static void eMB_PortTimerElapsedCallback(TIM_HandleTypeDef *xTimer)
{
  HAL_TIM_Base_Stop_IT(eMB_PORT_TIMER_INSTANCE);
  
  (void)pxMBPortCBTimerExpired();
}
