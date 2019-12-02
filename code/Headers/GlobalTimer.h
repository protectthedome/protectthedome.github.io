/****************************************************************************
 
  Header file for GlobalTimer
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef GlobalTimer_H
#define GlobalTimer_H

#include <stdint.h>
#include <stdbool.h>

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum {Playing_GT, NotPlaying_GT} GlobalTimerState_t;

// Public Function Prototypes

bool InitGlobalTimer (uint8_t Priority);
bool PostGlobalTimer(ES_Event_t ThisEvent);
ES_Event_t RunGlobalTimer(ES_Event_t ThisEvent);


#endif /* GlobalTimer_H */
