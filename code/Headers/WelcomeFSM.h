/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef WelcomeFSM_H
#define WelcomeFSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h" 

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  InitPState, Idle_W, Welcome
}WelcomeState_t;

// Public Function Prototypes

bool InitWelcomeFSM(uint8_t Priority);
bool PostWelcomeFSM(ES_Event_t ThisEvent);
ES_Event_t RunWelcomeFSM(ES_Event_t ThisEvent);
WelcomeState_t QueryWelcomeState(void);


#endif /* WelcomeFSM_H */

