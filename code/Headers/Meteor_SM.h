/****************************************************************************
 
  Header file for meteor interaction
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef Meteor_SM_H
#define Meteor_SM_H

#include <stdint.h>
#include <stdbool.h>

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum {Idle_M, MeteorFalling, MeteorFlashing} Meteor_SMState_t;

// Public Function Prototypes

bool InitMeteor_SM (uint8_t Priority);
bool PostMeteor_SM(ES_Event_t ThisEvent);
ES_Event_t RunMeteor_SM(ES_Event_t ThisEvent);



#endif /* Meteor_SM_H */
