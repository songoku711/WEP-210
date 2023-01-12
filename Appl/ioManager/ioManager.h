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



/*===============================================================================================
*                                     DEFINES AND MACROS
===============================================================================================*/

#define IOMANAGER_SIGNAL_PIN_NUM                  4U

#define IOMANAGER_SIGNAL_IC_THREAD_FLAG           (uint32_t)0x01000000
#define IOMANAGER_SIGNAL_IC_THREAD_FLAG_MASK      (uint32_t)0x000000FF

#define IOMANAGER_SIGNAL_OC_THREAD_FLAG           (uint32_t)0x02000000
#define IOMANAGER_SIGNAL_OC_THREAD_FLAG_OFFSET    8U
#define IOMANAGER_SIGNAL_OC_THREAD_FLAG_MASK      (uint32_t)0x0000FF00

#define IOMANAGER_RELAY_PIN_NUM                   16U



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
