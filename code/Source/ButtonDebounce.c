/****************************************************************************
 Module
   ButtonDebounce.c

 Revision
   1.0.1

 Description
   This is the first service for the button debounce under the
   Gen2 Events and Services Framework.

 Notes

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this service
*/
#include "ButtonDebounce.h"
#include <string.h>
#include "Cannon.h"
#include "Cannon_SM.h"
#include "ResetService.h"
#include "Meteor_SM.h"

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

bool CheckButtonEvents(void);


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
//Initialize to low (up)
static uint8_t LastButtonState = 0;
static ButtonState_t CurrentSMState;



// add a deferral queue for up to 3 pending deferrals +1 to allow for overhead
static ES_Event_t DeferralQueue[3+1];

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitButtonDebounce

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
     Trey Weber
****************************************************************************/
bool InitButtonDebounce ( uint8_t Priority )
{
  MyPriority = Priority;

  
  //Set SM currentstate
  CurrentSMState = Ready2Sample;
  
  return true;
}




/****************************************************************************
 Function
     PostButtonDebounce

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     Trey Weber
****************************************************************************/
bool PostButtonDebounce( ES_Event_t ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunButtonDebounce

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Executes code based on what events are posted to the service
 Notes
   
 Author
   Trey Weber
****************************************************************************/
ES_Event_t RunButtonDebounce( ES_Event_t ThisEvent )
{
  ES_Event_t NewEvent;
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT;
  
  switch (CurrentSMState) {
    case Ready2Sample:
      //If this event is a button up event
    if (ThisEvent.EventType == Button_Up) {
      //Update SM state
      CurrentSMState = Debounce;
      //Start timer
      ES_Timer_InitTimer(DEBOUNCE_TIMER, 50);
    } else
	if (ThisEvent.EventType == Button_Down){
      //Post Cannon_Button_Down
      NewEvent.EventType = Cannon_Button_Down;
      PostCannon_SM(NewEvent);
      PostMeteor_SM(NewEvent);
	    PostResetService(NewEvent);
	  //Update SM state
      CurrentSMState = Debounce;
      //Start timer
      ES_Timer_InitTimer(DEBOUNCE_TIMER, 50);
    }
    break;
    case Debounce:
      if (ThisEvent.EventType == ES_TIMEOUT) {
        //Move back into ready2sample
        CurrentSMState = Ready2Sample;
      }
    break;
  }
 
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
 /***
CheckButtonEvents Function Description
 	Arguments: none
 	Returns: bool. True if ther is a new event
 	This function check for a button press event

 	Author: Trey Weber
 ***/



bool CheckButtonEvents(void) {
  ES_Event_t NewEvent;
  bool ReturnVal = false;
  
  //Set current button state
  uint8_t CurrentButtonState = Cannon_ReadButton();
  
  //If different from last state
  if (CurrentButtonState != LastButtonState) {
    //Set return val
    ReturnVal = true;
    //If state is down
    if (CurrentButtonState == 0) {
      //Post event
      NewEvent.EventType = Button_Down;
      PostButtonDebounce(NewEvent);
    } else { 
      //Post event
      NewEvent.EventType = Button_Up;
      PostButtonDebounce(NewEvent);
    }
  }
  //Set last state to current state and return
  LastButtonState = CurrentButtonState;
  return ReturnVal;
}

 
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

