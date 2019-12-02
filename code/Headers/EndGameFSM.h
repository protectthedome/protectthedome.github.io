/****************************************************************************

  Header file for EndGameFSM

 ****************************************************************************/

#ifndef EndGameFSM_H
#define EndGameFSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  Idle_E, HappyDance, SadDance, InitPState_E
}EndGameState_t;

// Public Function Prototypes

bool InitEndGameFSM(uint8_t Priority);
bool PostEndGameFSM(ES_Event_t ThisEvent);
ES_Event_t RunEndGameFSM(ES_Event_t ThisEvent);

#endif /* EndGameFSM_H */
