/* 
 * File:   sensorManager.c
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

#include "sensorManager.h"

#include "mb.h"

#include "adc.h"
#include "cmsis_os.h"



/*===============================================================================================
*                                       DEFINES AND MACROS
===============================================================================================*/

#define SENSORMANAGER_TEMPERATURE_OFFSET          0U
#define SENSORMANAGER_PRESSURE_OFFSET             1U

#define SENSORMANAGER_ADC_CONV_CPLT_FLAG          (uint32_t)0x01



/*===============================================================================================
*                                       GLOBAL VARIABLES
===============================================================================================*/

extern osThreadId_t adcConvCbkTaskHandle;

uint32_t adc_value[2];
static uint8_t adc_conv_state;



/*===============================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
===============================================================================================*/

void SensorManager_InternalTriggerConvSubMainFunction(void);
void SensorManager_InternalUpdateDataSubMainFunction(void);



/*===============================================================================================
*                                       LOCAL FUNCTIONS
===============================================================================================*/

void SensorManager_InternalTriggerConvSubMainFunction(void)
{
  static uint8_t adc_counter = (uint8_t)10U;
  
  if (adc_counter < (uint8_t)20U)
  {
    adc_counter++;
  }
  else
  {
    adc_counter = (uint8_t)0U;
    
    if (adc_conv_state == (uint8_t)0U)
    {
      HAL_ADC_Start_DMA(&hadc1, adc_value, 2U);
      
      adc_conv_state = (uint8_t)1U;
    }
  }
}

void SensorManager_InternalUpdateDataSubMainFunction(void)
{
  static uint8_t updateDataCnt = (uint8_t)0U;
  
  if (updateDataCnt < (uint8_t)20U)
  {
    updateDataCnt++;
  }
  else
  {
    updateDataCnt = (uint8_t)0U;
    
    if (adc_conv_state == (uint8_t)2U)
    {
      /* Update sensor value in input variables */
      eMBSlaveSetDataU16(MB_SLAVE_REG_TYPE_INPUT, SENSORMANAGER_TEMPERATURE_OFFSET, adc_value[SENSORMANAGER_TEMPERATURE_OFFSET]);
      eMBSlaveSetDataU16(MB_SLAVE_REG_TYPE_INPUT, SENSORMANAGER_PRESSURE_OFFSET, adc_value[SENSORMANAGER_PRESSURE_OFFSET]);
      
      adc_conv_state = (uint8_t)0U;
    }
  }
}



/*===============================================================================================
*                                       GLOBAL FUNCTIONS
===============================================================================================*/

void SensorManager_Init(void)
{
  adc_conv_state = (uint8_t)0U;
  
  /* Calibration ADC for more precise results */
  HAL_ADCEx_Calibration_Start(&hadc1);
}



/*=============================================================================================*/
void SensorManager_MainFunction(void)
{
  SensorManager_InternalTriggerConvSubMainFunction();
  SensorManager_InternalUpdateDataSubMainFunction();
}

/*=============================================================================================*/
void SensorManager_AdcConvMainFunction(void)
{
  uint32_t recvFlag;
  
  recvFlag = osThreadFlagsWait(SENSORMANAGER_ADC_CONV_CPLT_FLAG, osFlagsWaitAny, osWaitForever);
  
  if (recvFlag == SENSORMANAGER_ADC_CONV_CPLT_FLAG)
  {
    adc_conv_state = (uint8_t)2U;
  }
}



/*=============================================================================================*/
void SensorManager_AdcConvCallback(void)
{
  osThreadFlagsSet(adcConvCbkTaskHandle, SENSORMANAGER_ADC_CONV_CPLT_FLAG);
}



#ifdef  __cplusplus
}
#endif
