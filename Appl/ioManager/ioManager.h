/* 
 * File:   ioManager.h
 * Author: Long
 *
 * Created on September 15, 2019, 11:06 AM
 */

#ifndef IOMANAGER_H
#define IOMANAGER_H

#ifdef  __cplusplus
extern "C" {
#endif



/*===============================================================================================
*                                         INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
===============================================================================================*/

#include "main.h"

#include "tim.h"
#include "gpio.h"
#include "cmsis_os.h"



/*===============================================================================================
*                                     DEFINES AND MACROS
===============================================================================================*/




/*===============================================================================================
*                                     FUNCTION PROTOTYPES
===============================================================================================*/

void IoManager_Init(void);

void IoManager_MainFunction(void);
void IoManager_ExtIrptMainFunction(void);

void IoManager_ExtIrptCallback(uint32_t arg);



#ifdef  __cplusplus
}
#endif

#endif  /* IOMANAGER_H */
