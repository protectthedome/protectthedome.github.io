/****************************************************************************
 
  Header file for Power Crank service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef PowerCrank_SM_H
#define PowerCrank_SM_H

#include <stdint.h>
#include <stdbool.h>

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum { Cranking, NotCranking, Idle_PC } PowerCrank_SMState_t;

// Public Function Prototypes

bool InitPowerCrank_SM (uint8_t Priority);
bool PostPowerCrank_SM(ES_Event_t ThisEvent);
ES_Event_t RunPowerCrank_SM(ES_Event_t ThisEvent);

bool CheckPowerCrankEvents(void);

PowerCrank_SMState_t QueryPowerCrank_SM(void);

#endif /* PowerCrank_SM_H */
