/****************************************************************************
 Module
   ResetService.c

 Revision
   1.0.1

 Description
   This is the reset service that tracks a user input every 30 seconds
   Gen2 Events and Services Framework.

 Notes

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this service
*/
#include "ResetService.h"
#include <string.h>
#include "Tot.h"
#include "WelcomeFSM.h"

/* include header files for hardware access
*/
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

/* include header files for the framework
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"




/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;


// add a deferral queue for up to 3 pending deferrals +1 to allow for overhead
static ES_Event_t DeferralQueue[3+1];

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitResetService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 01/16/12, 10:00
****************************************************************************/
bool InitResetService ( uint8_t Priority )
{
  MyPriority = Priority;
  
  return true;
}




/****************************************************************************
 Function
     PostResetService

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostResetService( ES_Event_t ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunResetService

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   
 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunResetService( ES_Event_t ThisEvent )
{
  ES_Event_t NewEvent;
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT;
  
  switch (ThisEvent.EventType) {
    case Tot_Inserted:
		//Init new 30s timer
		ES_Timer_InitTimer(RESET_TIMER, 30000);
    printf("\rTOT Inserted");
	break;
    case New_Pot_Value: 
		//Init new 30s timer
		ES_Timer_InitTimer(RESET_TIMER, 30000);
    printf("\rNewPotValue\r\n");
    break;
    case Airflow_Plugged:
		//Init new 30s timer
		ES_Timer_InitTimer(RESET_TIMER, 30000);
    printf("\rAirPlugged\r\n");
    break;
	case Falling_Edge:
		//Init new 30s timer
		ES_Timer_InitTimer(RESET_TIMER, 30000);
  printf("\rFalling Edge\r\n");
	break;
	case Rising_Edge:
		//Init new 30s timer
		ES_Timer_InitTimer(RESET_TIMER, 30000);
  printf("\rRising Edge\r\n");
	break;
	case Cannon_Button_Down:
		//Init new 30s timer;
		ES_Timer_InitTimer(RESET_TIMER, 30000);
	break;
	case ES_TIMEOUT:
    printf("\rRESET\r\n");
		//Post reset event to all
		NewEvent.EventType = RST;
		ES_PostAll(NewEvent);
		//Post go welcome mode event to Welcome_SM
		NewEvent.EventType = Go_Welcome_Mode;
		PostWelcomeFSM(NewEvent);
    //TOT_Release();
	break;
  case End_Of_Game:
    ES_Timer_StopTimer(RESET_TIMER);
  break;
  }
 
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/


 
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

