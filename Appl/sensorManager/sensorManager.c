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

#define SENSORMANAGER_TEMP_RES_PULL_UP_VAL        (uint32_t)15U
#define SENSORMANAGER_TEMP_RES_PULL_DOWN_VAL      (uint32_t)33U
#define SENSORMANAGER_TEMP_RES_REF_VAL            (uint32_t)10000U
#define SENSORMANAGER_TEMP_VREF_MV                (uint32_t)3300U
#define SENSORMANAGER_TEMP_VIN_MV                 (uint32_t)5000U

#define SENSORMANAGER_TEMP_ADC_MAX_RES            (uint32_t)4096U

#define SENSORMANAGER_TEMP_LOOKUP_TABLE_SIZE      101U

#define SENSORMANAGER_PRESSURE_RES_PULL_UP_VAL    (uint32_t)15U
#define SENSORMANAGER_PRESSURE_RES_PULL_DOWN_VAL  (uint32_t)33U
#define SENSORMANAGER_PRESSURE_VREF_MV            (uint32_t)3300U
#define SENSORMANAGER_PRESSURE_VIN_MV             (uint32_t)5000U

#define SENSORMANAGER_PRESSURE_ADC_MAX_RES        (uint32_t)4096U
#define SENSORMANAGER_PRESSURE_LEVEL_MAX_RES      (uint32_t)100U

#define SENSORMANAGER_ADC_CONV_CPLT_FLAG          (uint32_t)0x01

typedef enum
{
  SENSORMANAGER_CONV_STATE_INIT,
  SENSORMANAGER_CONV_STATE_MEASURE,
  SENSORMANAGER_CONV_STATE_CALC,
  SENSORMANAGER_CONV_STATE_RESULT,
} SensorManager_ConvStateType;



/*===============================================================================================
*                                       GLOBAL VARIABLES
===============================================================================================*/

extern osThreadId_t adcConvCbkTaskHandle;

static uint32_t sensorManager_convRawValue[2];
static uint8_t sensorManager_convAvailable;
static uint8_t sensorManager_convMeasureDone;

static SensorManager_ConvStateType sensorManager_convState;

static uint16_t sensorManager_tempVal;
static uint16_t sensorManager_presVal;

static const uint32_t tempResLookupTable[SENSORMANAGER_TEMP_LOOKUP_TABLE_SIZE] =
{
  32754,   /*   0 C */ 
  31124,   /*   1 C */ 
  29584,   /*   2 C */ 
  28130,   /*   3 C */ 
  26755,   /*   4 C */ 
  25456,   /*   5 C */ 
  24227,   /*   6 C */ 
  23065,   /*   7 C */ 
  21965,   /*   8 C */ 
  20923,   /*   9 C */ 
  19938,   /*  10 C */ 
  19004,   /*  11 C */ 
  18119,   /*  12 C */ 
  17280,   /*  13 C */ 
  16485,   /*  14 C */ 
  15731,   /*  15 C */ 
  15016,   /*  16 C */ 
  14337,   /*  17 C */ 
  13693,   /*  18 C */ 
  13081,   /*  19 C */ 
  12500,   /*  20 C */ 
  11948,   /*  21 C */ 
  11423,   /*  22 C */ 
  10925,   /*  23 C */ 
  10451,   /*  24 C */ 
  10000,   /*  25 C */ 
   9570,   /*  26 C */ 
   9162,   /*  27 C */ 
   8773,   /*  28 C */ 
   8403,   /*  29 C */ 
   8051,   /*  30 C */ 
   7715,   /*  31 C */ 
   7395,   /*  32 C */ 
   7090,   /*  33 C */ 
   6799,   /*  34 C */ 
   6522,   /*  35 C */ 
   6257,   /*  36 C */ 
   6005,   /*  37 C */ 
   5764,   /*  38 C */ 
   5534,   /*  39 C */ 
   5314,   /*  40 C */ 
   5104,   /*  41 C */ 
   4904,   /*  42 C */ 
   4713,   /*  43 C */ 
   4530,   /*  44 C */ 
   4355,   /*  45 C */ 
   4187,   /*  46 C */ 
   4027,   /*  47 C */ 
   3874,   /*  48 C */ 
   3728,   /*  49 C */ 
   3588,   /*  50 C */ 
   3454,   /*  51 C */ 
   3325,   /*  52 C */ 
   3202,   /*  53 C */ 
   3084,   /*  54 C */ 
   2971,   /*  55 C */ 
   2863,   /*  56 C */ 
   2759,   /*  57 C */ 
   2660,   /*  58 C */ 
   2564,   /*  59 C */ 
   2473,   /*  60 C */ 
   2385,   /*  61 C */ 
   2301,   /*  62 C */ 
   2220,   /*  63 C */ 
   2143,   /*  64 C */ 
   2068,   /*  65 C */ 
   1997,   /*  66 C */ 
   1928,   /*  67 C */ 
   1862,   /*  68 C */ 
   1798,   /*  69 C */ 
   1738,   /*  70 C */ 
   1679,   /*  71 C */ 
   1623,   /*  72 C */ 
   1568,   /*  73 C */ 
   1516,   /*  74 C */ 
   1466,   /*  75 C */ 
   1418,   /*  76 C */ 
   1372,   /*  77 C */ 
   1327,   /*  78 C */ 
   1284,   /*  79 C */ 
   1243,   /*  80 C */ 
   1203,   /*  81 C */ 
   1164,   /*  82 C */ 
   1127,   /*  83 C */ 
   1092,   /*  84 C */ 
   1058,   /*  85 C */ 
   1024,   /*  86 C */ 
    993,   /*  87 C */ 
    962,   /*  88 C */ 
    932,   /*  89 C */ 
    904,   /*  90 C */ 
    876,   /*  91 C */ 
    849,   /*  92 C */ 
    824,   /*  93 C */ 
    799,   /*  94 C */ 
    775,   /*  95 C */ 
    752,   /*  96 C */ 
    730,   /*  97 C */ 
    708,   /*  98 C */ 
    687,   /*  99 C */ 
    667,   /* 100 C */
};



/*===============================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
===============================================================================================*/

void SensorManager_InternalCalcTemp(uint32_t adcVal, uint16_t* tempVal);
void SensorManager_InternalCalcPres(uint32_t adcVal, uint16_t* presVal);

void SensorManager_InternalTriggerConvSubMainFunction(void);
void SensorManager_InternalUpdateDataSubMainFunction(void);



/*===============================================================================================
*                                       LOCAL FUNCTIONS
===============================================================================================*/

void SensorManager_InternalCalcTemp(uint32_t adcVal, uint16_t* tempVal)
{
  static uint8_t notFirstTimeLookup = (uint8_t)0U;
  static uint16_t sensorManager_curTempVal;
  float volTemp;
  float tempResTemp;
  uint32_t tempRes;
  
  /* Calculate input voltage from NTC sensor */
  /* Vin(mV) = 3300 * (adcVal / 4096) * ((R1 + R2) / R2) */
  volTemp = (float)(adcVal * SENSORMANAGER_TEMP_VREF_MV * (SENSORMANAGER_TEMP_RES_PULL_UP_VAL + SENSORMANAGER_TEMP_RES_PULL_DOWN_VAL));
  volTemp = volTemp / (float)(SENSORMANAGER_TEMP_RES_PULL_DOWN_VAL * SENSORMANAGER_TEMP_ADC_MAX_RES);
  
  /* Calculate NTC sensor resistance */
  /* Res(Ohm) = (5000 * Rref) / Vin(mV) - Rref */
  tempResTemp = (float)(SENSORMANAGER_PRESSURE_VIN_MV * SENSORMANAGER_TEMP_RES_REF_VAL) / volTemp;
  tempRes = (uint32_t)tempResTemp - SENSORMANAGER_TEMP_RES_REF_VAL;
  
  /* Check if NTC resistance is out of look-up table range */
  if (tempRes > tempResLookupTable[0])
  {
    notFirstTimeLookup = (uint8_t)0U;
    *tempVal = (uint8_t)0U;
  }
  else if (tempRes < tempResLookupTable[SENSORMANAGER_TEMP_LOOKUP_TABLE_SIZE - 1])
  {
    notFirstTimeLookup = (uint8_t)0U;
    *tempVal = (uint8_t)100U;
  }
  else
  {
    /* Find NTC temperature by searching in look-up table array */
    if (notFirstTimeLookup)
    {
      if ((tempResLookupTable[sensorManager_curTempVal - 1] > tempRes) && (tempResLookupTable[sensorManager_curTempVal] <= tempRes))
      {
        *tempVal = sensorManager_curTempVal;
      }
      else if (tempResLookupTable[sensorManager_curTempVal - 1] <= tempRes)
      {
        for (uint8_t index = (uint8_t)sensorManager_curTempVal - (uint8_t)1U; index > (uint8_t)0U; index++)
        {
          if ((tempResLookupTable[index - 1] > tempRes) && (tempResLookupTable[index] <= tempRes))
          {
            sensorManager_curTempVal = (uint16_t)index;
            *tempVal = sensorManager_curTempVal;
            
            break;
          }
        }
      }
      else if (tempResLookupTable[sensorManager_curTempVal] > tempRes)
      {
        for (uint8_t index = (uint8_t)sensorManager_curTempVal; index < SENSORMANAGER_TEMP_LOOKUP_TABLE_SIZE; index++)
        {
          if ((tempResLookupTable[index] > tempRes) && (tempResLookupTable[index + 1] <= tempRes))
          {
            sensorManager_curTempVal = (uint16_t)index;
            *tempVal = sensorManager_curTempVal;
            
            break;
          }
        }
      }
      else
      {
        /* Should never enter here */
      }
    }
    else
    {
      for (uint8_t index = (uint8_t)1U; index < SENSORMANAGER_TEMP_LOOKUP_TABLE_SIZE; index++)
      {
        if ((tempResLookupTable[index - 1] > tempRes) && (tempResLookupTable[index] <= tempRes))
        {
          sensorManager_curTempVal = index;
          *tempVal = sensorManager_curTempVal;
          
          notFirstTimeLookup = (uint8_t)1U;
          
          break;
        }
      }
    }
  }
}

/*=============================================================================================*/
void SensorManager_InternalCalcPres(uint32_t adcVal, uint16_t* presVal)
{
  float volTemp;
  float presValTemp;
  
  /* Calculate input voltage from pressure sensor */
  /* Vin(mV) = 3300 * (adcVal / 4096) * ((R1 + R2) / R2) */
  volTemp = (float)(adcVal * SENSORMANAGER_PRESSURE_VREF_MV * (SENSORMANAGER_PRESSURE_RES_PULL_UP_VAL + SENSORMANAGER_PRESSURE_RES_PULL_DOWN_VAL));
  volTemp = volTemp / (float)(SENSORMANAGER_PRESSURE_RES_PULL_DOWN_VAL * SENSORMANAGER_PRESSURE_ADC_MAX_RES);
  
  /* Calculate pressure level */
  /* presVal = (Vin(mV) / 5000) * 100 */
  presValTemp = (float)(volTemp * (float)SENSORMANAGER_PRESSURE_LEVEL_MAX_RES) / (float)SENSORMANAGER_PRESSURE_VIN_MV;
  
  if ((presValTemp - (float)0.5f) < (uint16_t)presValTemp)
  {
    *presVal = (uint16_t)presValTemp;
  }
  else
  {
    *presVal = (uint16_t)presValTemp + (uint16_t)1U;
  }
}



/*=============================================================================================*/
void SensorManager_InternalTriggerConvSubMainFunction(void)
{
  static uint8_t adc_counter = (uint8_t)0U;
  static uint8_t adc_timeout = (uint8_t)0U;
  
  if (adc_counter < (uint8_t)20U)
  {
    adc_counter++;
  }
  else
  {
    adc_counter = (uint8_t)0U;
    
    sensorManager_convAvailable = (uint8_t)1U;
  }
  
  switch (sensorManager_convState)
  {
    case SENSORMANAGER_CONV_STATE_INIT:
    {
      if (sensorManager_convAvailable != (uint8_t)0U)
      {
        sensorManager_convAvailable = (uint8_t)0U;
        
        HAL_ADC_Start_DMA(&hadc1, sensorManager_convRawValue, 2U);
        
        adc_timeout = (uint8_t)0U;
        sensorManager_convState = SENSORMANAGER_CONV_STATE_MEASURE;
      }
      
      break;
    }
    case SENSORMANAGER_CONV_STATE_MEASURE:
    {
      if (adc_timeout < (uint8_t)200U)
      {
        adc_timeout++;
        
        if (sensorManager_convMeasureDone != (uint8_t)0U)
        {
          sensorManager_convMeasureDone = (uint8_t)0U;
          
          adc_timeout = (uint8_t)0U;
          sensorManager_convState = SENSORMANAGER_CONV_STATE_CALC;
        }
      }
      else
      {
        adc_timeout = (uint8_t)0U;
        sensorManager_convState = SENSORMANAGER_CONV_STATE_INIT;
      }
      
      break;
    }
    case SENSORMANAGER_CONV_STATE_CALC:
    {
      SensorManager_InternalCalcTemp(sensorManager_convRawValue[SENSORMANAGER_TEMPERATURE_OFFSET], &sensorManager_tempVal);
      SensorManager_InternalCalcPres(sensorManager_convRawValue[SENSORMANAGER_PRESSURE_OFFSET], &sensorManager_presVal);
      
      sensorManager_convState = SENSORMANAGER_CONV_STATE_RESULT;
      
      break;
    }
    case SENSORMANAGER_CONV_STATE_RESULT:
    {
      /* Update sensor value in input variables */
      eMBSlaveSetDataU16(MB_SLAVE_REG_TYPE_INPUT, SENSORMANAGER_TEMPERATURE_OFFSET, sensorManager_tempVal);
      eMBSlaveSetDataU16(MB_SLAVE_REG_TYPE_INPUT, SENSORMANAGER_PRESSURE_OFFSET, sensorManager_presVal);
      
      sensorManager_convState = SENSORMANAGER_CONV_STATE_INIT;
      
      break;
    }
    default:
    {
      sensorManager_convState = SENSORMANAGER_CONV_STATE_INIT;
      
      break;
    }
  }
}



/*===============================================================================================
*                                       GLOBAL FUNCTIONS
===============================================================================================*/

void SensorManager_Init(void)
{
  sensorManager_convAvailable = (uint8_t)1U;
  sensorManager_convMeasureDone = (uint8_t)0U;
  sensorManager_convState = SENSORMANAGER_CONV_STATE_INIT;
  
  /* Calibration ADC for more precise results */
  HAL_ADCEx_Calibration_Start(&hadc1);
}



/*=============================================================================================*/
void SensorManager_MainFunction(void)
{
  SensorManager_InternalTriggerConvSubMainFunction();
}

/*=============================================================================================*/
void SensorManager_AdcConvMainFunction(void)
{
  uint32_t recvFlag;
  
  recvFlag = osThreadFlagsWait(SENSORMANAGER_ADC_CONV_CPLT_FLAG, osFlagsWaitAny, osWaitForever);
  
  if (recvFlag == SENSORMANAGER_ADC_CONV_CPLT_FLAG)
  {
    sensorManager_convMeasureDone = (uint8_t)1U;
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
