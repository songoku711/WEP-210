/* 
 * File:   hc595.h
 * Author: Huy Long
 * Comments:  File contains definitions, structures and interfaces to 74HC595.
 * Revision history: 1.0
 */

#ifndef HC595_H
#define HC595_H

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
);



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
);



#ifdef __cplusplus
}
#endif

#endif /* HC595_H */
