/* 
 * File:   hc595.c
 * Author: Huy Long
 * Comments:  File contains definitions, structures and interfaces to 74HC595.
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

#include "hc595.h"
#include "spi.h"

#include "main.h"



/*===============================================================================================
*                                       DEFINES AND MACROS
===============================================================================================*/

#define HC595_SPI_INSTANCE                        hspi2

#ifndef HC595_CS_Pin
#define HC595_CS_Pin                              HC595_CS_Pin
#endif

#ifndef HC595_CS_Port
#define HC595_CS_Port                             HC595_CS_GPIO_Port
#endif



#if ((defined HC595_CS_Pin) && (defined HC595_CS_Port))
  #define __HC595_CS_SET()                        { HC595_CS_Port->ODR |= (uint32_t)HC595_CS_Pin; }
  #define __HC595_CS_RESET()                      { HC595_CS_Port->ODR &= (uint32_t)(~((uint32_t)HC595_CS_Pin)); }
#else
  #error "HC595 Chip Select port and pin are not defined"
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
 * @brief      HC595 initialization function
 * @details    HC595 initialization function
 * @param[in]  None
 *
 * @return     HAL_StatusTypeDef
 *             HAL_OK - Initialization successful
 *             HAL_ERROR - Initialization failed
 */
HAL_StatusTypeDef HC595_Init
(
  void
)
{
  __HC595_CS_SET();
  
  return HAL_OK;
}



/*=============================================================================================*/
/**
 * @brief      HC595 write array of bytes function
 * @details    This function will write array of bytes consecutively
 *             into HC595
 * @param[in]  uint8_t    instance     HC595 instance
 *             uint8_t*   data         Pointer to array of bytes
 *             uint8_t    length       Number of byte data to be written
 *
 * @return     HAL_StatusTypeDef
 *             HAL_OK - Write successful
 *             HAL_ERROR - Write failed
 */
HAL_StatusTypeDef HC595_WriteByte
(
  uint8_t      instance,
  uint8_t*     data,
  uint8_t      length
)
{
  HAL_StatusTypeDef retVal;
  
  (void)instance;
  
  __HC595_CS_RESET();
  
  retVal = HAL_SPI_Transmit(&HC595_SPI_INSTANCE, data, length, HAL_MAX_DELAY);
  
  __HC595_CS_SET();
  
  return retVal;
}



#ifdef __cplusplus
}
#endif
