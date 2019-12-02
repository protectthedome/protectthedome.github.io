/****************************************************************************
 
  Header file for the cannon SM
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef Cannon_SM_H
#define Cannon_SM_H

#include <stdint.h>
#include <stdbool.h>

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum { IdleNotPlaying, IdlePlaying, KnobVibrating, LEDLit } Cannon_SMState_t;

// Public Function Prototypes

bool InitCannon_SM (uint8_t Priority);
bool PostCannon_SM(ES_Event_t ThisEvent);
ES_Event_t RunCannon_SM(ES_Event_t ThisEvent);

bool PotChecker(void);

#endif /* Cannon_SM_H */
