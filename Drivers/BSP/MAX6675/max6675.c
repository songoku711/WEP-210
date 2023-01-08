/* 
 * File:   max6675.c
 * Author: Huy Long
 * Comments:  File contains definitions, structures and interfaces to MAX6675.
 * Revision history: 1.0
 */

#ifdef   __cplusplus
extern "C" {
#endif



/*===============================================================================================
*                                         INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
===============================================================================================*/

#include "max6675.h"
#include "spi.h"

#include "main.h"



/*===============================================================================================
*                                       DEFINES AND MACROS
===============================================================================================*/

#define MAX6675_SPI_INSTANCE                      hspi1

#ifndef MAX6675_CS_Pin
#define MAX6675_CS_Pin                            K_TEMP_CS_Pin
#endif

#ifndef MAX6675_CS_Port
#define MAX6675_CS_Port                           K_TEMP_CS_GPIO_Port
#endif



#if ((defined MAX6675_CS_Pin) && (defined MAX6675_CS_Port))
  #define __MAX6675_CS_SET()                      { MAX6675_CS_Port->ODR |= (uint32_t)MAX6675_CS_Pin; }
  #define __MAX6675_CS_RESET()                    { MAX6675_CS_Port->ODR &= (uint32_t)(~((uint32_t)MAX6675_CS_Pin)); }
#else
  #error "MAX6675 Chip Select port and pin are not defined"
#endif



/*===============================================================================================
*                                       LOCAL VARIABLES
===============================================================================================*/




/*===============================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
===============================================================================================*/




/*===============================================================================================
*                                       LOCAL FUNCTIONS
===============================================================================================*/




/*===============================================================================================
*                                       GLOBAL FUNCTIONS
===============================================================================================*/

/**
 * @brief      MAX6675 initialization function
 * @details    MAX6675 initialization function
 * @param[in]  None
 *
 * @return     HAL_StatusTypeDef
 *             HAL_OK - Initialization successful
 *             HAL_ERROR - Initialization failed
 */
HAL_StatusTypeDef MAX6675_Init
(
  void
)
{
  __MAX6675_CS_SET();
  
  return HAL_OK;
}



/*=============================================================================================*/
/**
 * @brief      Read temperature from MAX6675
 * @details    This function will read temperature from MAX6675
 * @param[in]  uint16_t*  temp         Pointer to output temperature
 *             uint8_t    degType      Temperature in Celsius (0) or Fahrenheit (1) degree
 *
 * @return     HAL_StatusTypeDef
 *             HAL_OK - Read successful
 *             HAL_ERROR - Read failed
 */
HAL_StatusTypeDef MAX6675_ReadTemp
(
  uint16_t*    temp,
  uint8_t      degType
)
{
  uint16_t result;
  uint8_t data[2];
  
  __MAX6675_CS_RESET();
  
  HAL_SPI_Receive(&MAX6675_SPI_INSTANCE, data, 2U, HAL_MAX_DELAY);
  
  __MAX6675_CS_SET();
  
  result  =  (uint16_t)data[0];
  result |= ((uint16_t)data[1] << 8U);
  
  /* Check if there is K thermocouple sensor attached */
  if (result & (uint16_t)0x04U)
  {
    return HAL_ERROR;
  }
  
  /* Shift right 3-bit and mask with 12-bit to get temperature */
  result = (result >> 3U) & (uint16_t)0x0FFF;
  
  /* Output temperature is in F degree */
  if (degType != (uint8_t)0U)
  {
    *temp = (result * (uint16_t)9U / (uint16_t)5U) + (uint16_t)32U;
  }
  else
  {
    *temp = result;
  }
  
  return HAL_OK;
}



#ifdef __cplusplus
}
#endif
