/****************************************************************************
 Module
     AirLeak.h
 Description
     header file for the AirLeak
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/08/19 18:44 gab      pseudo code
*****************************************************************************/


#ifndef AirLeak_SM_H
#define AirLeak_SM_H
// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"  


// typedefs for the states
// State definitions for use with the query function
typedef enum { Idle_A, FanOn} AirLeakState_t ;

// Public Function Prototypes

bool InitAirLeak_SM( uint8_t Priority );
bool PostAirLeak_SM( ES_Event_t ThisEvent );
ES_Event_t RunAirLeak_SM( ES_Event_t ThisEvent );
bool AirPlugChecker(void);



#endif /* AirLeakSM_H */
