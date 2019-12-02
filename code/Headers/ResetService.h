/****************************************************************************
 
  Header file for reset service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef ResetService_H
#define ResetService_H

#include <stdint.h>
#include <stdbool.h>

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"


// Public Function Prototypes

bool InitResetService (uint8_t Priority);
bool PostResetService(ES_Event_t ThisEvent);
ES_Event_t RunResetService(ES_Event_t ThisEvent);


#endif /* ResetService_H */
