/****************************************************************************
 
  Header file for Arcade FSM
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef ArcadeFSM_H
#define ArcadeFSM_H

#include <stdint.h>
#include <stdbool.h>

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum 
{
  NotPlaying, 
  Playing, 
  WaitForNext,  
  WaitForTotRelease
} ArcadeFSMState_t;

// Public Function Prototypes
#define WIN 1
#define GAME_OVER 0

bool InitArcadeFSM (uint8_t Priority);
bool PostArcadeFSM(ES_Event_t ThisEvent);
ES_Event_t RunArcadeFSM(ES_Event_t ThisEvent);

//Tot inserted event checker? Not sure if we want to have it here or in EventCheckers.c
bool TotChecker(void);
#endif /* ArcadeFSM_H */

