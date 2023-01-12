/* 
 * File:   ioManager.c
 * Author: Long
 *
 * Created on September 15, 2019, 11:06 AM
 */

#ifdef  __cplusplus
extern "C" {
#endif



/*===============================================================================================
*                                         INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
===============================================================================================*/

#include "ioManager.h"

#include "mb.h"

#include "hc595.h"

#include "tim.h"
#include "gpio.h"
#include "cmsis_os.h"



/*===============================================================================================
*                                       DEFINES AND MACROS
===============================================================================================*/

#define IOMANAGER_SIGNAL_INVERTER_ERR_OFFSET      0U
#define IOMANAGER_SIGNAL_IMBALANCE_ERR_OFFSET     1U
#define IOMANAGER_SIGNAL_DOOROPEN_ERR_OFFSET      2U

#define IOMANAGER_RELAY_ADDR                      0U
#define IOMANAGER_RELAY_BYTE_SIZE                 IOMANAGER_RELAY_PIN_NUM / 8

/**
 * @brief      Signal alive time period
 *
 */
#define SIGNAL_ALIVE_PERIOD                       (uint16_t)1000U

/**
 * @brief      Signal timer period
 *
 */
#define SIGNAL_TIMER_PERIOD                       (uint16_t)10000U



/*===============================================================================================
*                                       GLOBAL VARIABLES
===============================================================================================*/

extern osMessageQueueId_t ioInputStateQueueHandle;

static uint16_t currentFallingEdgeCount[IOMANAGER_SIGNAL_PIN_NUM];
static uint16_t lastFallingEdgeCount[IOMANAGER_SIGNAL_PIN_NUM];

static uint8_t firstFallingEdgeDetected[IOMANAGER_SIGNAL_PIN_NUM];

static uint16_t firstCompareValue[IOMANAGER_SIGNAL_PIN_NUM];
static uint16_t currentCompareValue[IOMANAGER_SIGNAL_PIN_NUM];

GPIO_PinState signalStateAtInitialStep[IOMANAGER_SIGNAL_PIN_NUM];
uint8_t signalStateLock[IOMANAGER_SIGNAL_PIN_NUM];

uint8_t signalState[IOMANAGER_SIGNAL_PIN_NUM];

uint8_t relayState[IOMANAGER_RELAY_BYTE_SIZE];



/*===============================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
===============================================================================================*/

void IoManager_InternalUpdateDataSubMainFunction(void);
void IoManager_InternalControlRelaySubMainFunction(void);



/*===============================================================================================
*                                       LOCAL FUNCTIONS
===============================================================================================*/

void IoManager_InternalUpdateDataSubMainFunction(void)
{
  static uint8_t updateDataCnt = (uint8_t)5U;
  uint16_t temp;
  
  if (updateDataCnt < (uint8_t)10U)
  {
    updateDataCnt++;
  }
  else
  {
    updateDataCnt = (uint8_t)0U;
    
    /* Update signal in discrete input variables */
    eMBSlaveSetDiscreteInput(IOMANAGER_SIGNAL_INVERTER_ERR_OFFSET, signalState[IOMANAGER_SIGNAL_INVERTER_ERR_OFFSET]);
    eMBSlaveSetDiscreteInput(IOMANAGER_SIGNAL_IMBALANCE_ERR_OFFSET, signalState[IOMANAGER_SIGNAL_IMBALANCE_ERR_OFFSET]);
    eMBSlaveSetDiscreteInput(IOMANAGER_SIGNAL_DOOROPEN_ERR_OFFSET, signalState[IOMANAGER_SIGNAL_DOOROPEN_ERR_OFFSET]);
    
    /* Get relay state from holding input variables */
    eMBSlaveGetDataU16(MB_SLAVE_REG_TYPE_HOLDING, IOMANAGER_RELAY_ADDR, &temp);
    
    relayState[0] = (uint8_t)((temp & (uint16_t)0xFF00) >> 8U);
    relayState[1] = (uint8_t)(temp & (uint16_t)0x00FF);
  }
}

/*=============================================================================================*/
void IoManager_InternalControlRelaySubMainFunction(void)
{
  static uint8_t ctrlRelayCnt = (uint8_t)0U;
  
  if (ctrlRelayCnt < (uint8_t)10U)
  {
    ctrlRelayCnt++;
  }
  else
  {
    ctrlRelayCnt = (uint8_t)0U;
    
    /* Control relay */
    (void)HC595_WriteByte(0U, relayState, IOMANAGER_RELAY_BYTE_SIZE);
  }
}



/*===============================================================================================
*                                       GLOBAL FUNCTIONS
===============================================================================================*/

void IoManager_Init(void)
{
  uint8_t index;
  
  /* Initialize signal variables */
  for (index = (uint8_t)0U; index < IOMANAGER_SIGNAL_PIN_NUM; index++)
  {
    if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(GPIOx_hwSignalCfg[index].hwSignalPort, GPIOx_hwSignalCfg[index].hwSignalPin))
    {
      signalState[index] = GPIO_PIN_SET;
      signalStateAtInitialStep[index] = GPIO_PIN_SET;
    }
    else
    {
      signalState[index] = GPIO_PIN_RESET;
      signalStateAtInitialStep[index] = GPIO_PIN_RESET;
    }
    
    currentFallingEdgeCount[index] = (uint8_t)0U;
    lastFallingEdgeCount[index] = (uint8_t)0U;
    
    firstFallingEdgeDetected[index] = (uint8_t)1U;
    
    signalStateLock[index] = (uint8_t)0U;
  }
  
  /* Initialize relay variables */
  for (index = (uint8_t)0U; index < IOMANAGER_RELAY_BYTE_SIZE; index++)
  {
    relayState[index] = (uint8_t)0x00U;
  }
  
  /* Start signal input capture */
  TIMx_StartInputCapture();
}



/*=============================================================================================*/
void IoManager_MainFunction(void)
{
  IoManager_InternalUpdateDataSubMainFunction();
  IoManager_InternalControlRelaySubMainFunction();
}

/*=============================================================================================*/
void IoManager_ExtIrptMainFunction(void)
{
  uint32_t recvFlag;
  uint8_t signalIdx;
  
  osMessageQueueGet(ioInputStateQueueHandle, (void *)&recvFlag, NULL, osWaitForever);
  
  if (recvFlag & IOMANAGER_SIGNAL_IC_THREAD_FLAG)
  {
    signalIdx = (uint8_t)(recvFlag & IOMANAGER_SIGNAL_IC_THREAD_FLAG_MASK);
    
    /* Increase number of falling edge counted */
    currentFallingEdgeCount[signalIdx]++;
    
    /* First time falling edge detected */
    if ((uint8_t)0U != firstFallingEdgeDetected[signalIdx])
    {
      /* Get current capture value */
      firstCompareValue[signalIdx] = __HAL_TIM_GET_COMPARE(TIMx_hwSignalCfg[signalIdx].hwTimerIns, TIMx_hwSignalCfg[signalIdx].hwTimerICChn);
      
      currentCompareValue[signalIdx] = ((firstCompareValue[signalIdx] + SIGNAL_ALIVE_PERIOD) % SIGNAL_TIMER_PERIOD);
      
      /* Set compare value for output compare channel */
      __HAL_TIM_SET_COMPARE(TIMx_hwSignalCfg[signalIdx].hwTimerIns, TIMx_hwSignalCfg[signalIdx].hwTimerOCChn, currentCompareValue[signalIdx]);
    
      /* Enable the output compare channel interrupt */
      __HAL_TIM_ENABLE_IT(TIMx_hwSignalCfg[signalIdx].hwTimerIns, TIMx_hwSignalCfg[signalIdx].hwTimerOCIrpt);
      
      /* Enable the output compare channel */
      TIM_CCxChannelCmd((TIMx_hwSignalCfg[signalIdx].hwTimerIns)->Instance, TIMx_hwSignalCfg[signalIdx].hwTimerOCChn, TIM_CCx_ENABLE);
      
      firstFallingEdgeDetected[signalIdx] = (uint8_t)0U;
    }
  }
  
  if (recvFlag & IOMANAGER_SIGNAL_OC_THREAD_FLAG)
  {
    signalIdx = (uint8_t)((recvFlag & IOMANAGER_SIGNAL_OC_THREAD_FLAG_MASK) >> IOMANAGER_SIGNAL_OC_THREAD_FLAG_OFFSET);
    
    /* Check if current falling edge count is greater than last one,
       or the signal is in low level */
    if ( \
        (currentFallingEdgeCount[signalIdx] > lastFallingEdgeCount[signalIdx]) || \
        (GPIO_PIN_RESET == ((GPIOx_hwSignalCfg[signalIdx].hwSignalPort)->IDR & GPIOx_hwSignalCfg[signalIdx].hwSignalPin)) \
       )
    {
      /* Set signal is ON */
      signalState[signalIdx] = GPIO_PIN_SET;
      
      /* Update falling edge count value */
      lastFallingEdgeCount[signalIdx] = currentFallingEdgeCount[signalIdx];

      /* Update compare value */
      currentCompareValue[signalIdx] = ((currentCompareValue[signalIdx] + SIGNAL_ALIVE_PERIOD) % SIGNAL_TIMER_PERIOD);
      
      /* Set new compare value to start next compare */
      __HAL_TIM_SET_COMPARE(TIMx_hwSignalCfg[signalIdx].hwTimerIns, TIMx_hwSignalCfg[signalIdx].hwTimerOCChn, currentCompareValue[signalIdx]);
    }
    else
    {
      /* Set signal is OFF */
      signalState[signalIdx] = GPIO_PIN_RESET;
      
      /* Disable the output compare channel */
      TIM_CCxChannelCmd((TIMx_hwSignalCfg[signalIdx].hwTimerIns)->Instance, TIMx_hwSignalCfg[signalIdx].hwTimerOCChn, TIM_CCx_DISABLE);
      
      /* Disable the output compare channel interrupt */
      __HAL_TIM_DISABLE_IT(TIMx_hwSignalCfg[signalIdx].hwTimerIns, TIMx_hwSignalCfg[signalIdx].hwTimerOCIrpt);

      /* Reset falling edge count */
      lastFallingEdgeCount[signalIdx] = (uint16_t)0U;
      currentFallingEdgeCount[signalIdx] = (uint16_t)0U;
      
      /* Reset all compare value */
      firstCompareValue[signalIdx] = (uint16_t)0U;
      currentCompareValue[signalIdx] = (uint16_t)0U;
      
      firstFallingEdgeDetected[signalIdx] = (uint8_t)1U;
    }
  }
}



/*=============================================================================================*/
void IoManager_ExtIrptCallback(uint32_t arg)
{
  osMessageQueuePut(ioInputStateQueueHandle, (const void *)&arg, 0U, 0U);
}



#ifdef  __cplusplus
}
#endif
