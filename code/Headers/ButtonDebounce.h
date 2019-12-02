/****************************************************************************
 
  Header file for button debounce
  based on the Gen 2 Events and Debounces Framework

 ****************************************************************************/

#ifndef ButtonDebounce_H
#define ButtonDebounce_H

#include <stdint.h>
#include <stdbool.h>

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum { Debounce, Ready2Sample, } ButtonState_t;

// Public Function Prototypes

bool InitButtonDebounce (uint8_t Priority);
bool PostButtonDebounce(ES_Event_t ThisEvent);
ES_Event_t RunButtonDebounce(ES_Event_t ThisEvent);

bool CheckButtonEvents(void);


#endif /* ButtonDebounce_H */
