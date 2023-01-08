/* 
 * File:   max6675.h
 * Author: Huy Long
 * Comments:  File contains definitions, structures and interfaces to MAX6675.
 * Revision history: 1.0
 */

#ifndef MAX6675_H
#define MAX6675_H

#ifdef __cplusplus
extern "C" {
#endif



/*===============================================================================================
*                                         INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
===============================================================================================*/

#include "stm32f1xx_hal.h"



/*===============================================================================================
*                                       DEFINES AND MACROS
===============================================================================================*/





/*===============================================================================================
*                                     FUNCTION PROTOTYPES
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
);



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
);



#ifdef __cplusplus
}
#endif

#endif /* MAX6675_H */
