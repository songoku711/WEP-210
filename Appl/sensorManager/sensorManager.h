/* 
 * File:   sensorManager.h
 * Author: Long
 *
 * Created on September 15, 2019, 11:06 AM
 */

#ifndef SENSORMANAGER_H
#define SENSORMANAGER_H

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




/*===============================================================================================
*                                     FUNCTION PROTOTYPES
===============================================================================================*/

void SensorManager_Init(void);

void SensorManager_MainFunction(void);
void SensorManager_AdcConvMainFunction(void);

void SensorManager_AdcConvCallback(void);



#ifdef  __cplusplus
}
#endif

#endif  /* SENSORMANAGER_H */
