/****************************************************************************
 Module
   PowerCrank_SM.c

 Revision
   1.0.1

 Description
   This is the first service for the Test Harness under the 
   Gen2 Events and Services Framework.

 Notes

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this service
*/
#include "PowerCrank_SM.h"
#include <string.h>
#include "Servo.h"
#include "PowerLib.h"
#include "ArcadeFSM.h"
#include "Cannon_SM.h"
#include "ResetService.h"

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

#define MAX_COUNT 100
//Tune the following two values to span proper servo range
#define BEGINNING_TIME 0
#define END_TIME 100
#define DeltaTime (END_TIME-BEGINNING_TIME)/100

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

bool CheckPowerCrankEvents(void);


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static PowerCrank_SMState_t CurrentSMState;
static uint16_t counter = 0;
static uint8_t LastInputState = 0;
static uint8_t CurrentTimePercentage = 0;

// add a deferral queue for up to 3 pending deferrals +1 to allow for overhead
static ES_Event_t DeferralQueue[3+1];

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitPowerCrank_SM

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
bool InitPowerCrank_SM ( uint8_t Priority )
{
  MyPriority = Priority;
  
  //Initialize hardware
  PowerLib_HWInit();
 
  CurrentSMState = Idle_PC;
  
  return true;
}



/****************************************************************************
 Function
     PostPowerCrank_SM

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
	Trey Weber     ****************************************************************************/
bool PostPowerCrank_SM( ES_Event_t ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunPowerCrank_SM

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   
 Author
	Trey Weber   ****************************************************************************/
ES_Event_t RunPowerCrank_SM( ES_Event_t ThisEvent )
{
  ES_Event_t NewEvent;
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT;
  
  switch (CurrentSMState) {
    case NotCranking: 
	//Check for Falling edge
	if (ThisEvent.EventType == Falling_Edge) {
		//Initialize timer
		ES_Timer_InitTimer(CRANK_TIMER, 50);
		//Update current state
		CurrentSMState = Cranking;
	}
	else if (ThisEvent.EventType == RST || ThisEvent.EventType == End_Of_Game) {
		CurrentSMState = Idle_PC;
		counter = 0;
		CurrentTimePercentage = 0;
	}	
  else if (ThisEvent.EventType == ES_TIMEOUT) {
		//Check if we are below max count
		if (counter < MAX_COUNT) {
			//Increment counter
			counter++;
			//Increment power gauge
			CurrentTimePercentage++;
			PowerLib_MoveGauge(counter);
		}
		//else we have reached maximum count
		else {
		//Post interaction complete to arcade SM
		NewEvent.EventType = Interaction_Completed;
		PostArcadeFSM(NewEvent);
		//Post Power complete to cannon SM
		NewEvent.EventType = Power_Complete;
		PostCannon_SM(NewEvent);
		//Reset counter
		counter = 0;
    //Reset state
    CurrentSMState = Idle_PC;
		//Reset servo position
		CurrentTimePercentage = 0;
		}
	}
	break;
	case Cranking:
	//Check for reset or end of game
	if (ThisEvent.EventType == RST || ThisEvent.EventType == End_Of_Game) {
		CurrentSMState = Idle_PC;
		counter = 0;
		CurrentTimePercentage = 0;
	}
	//else check for rising edge
	else if (ThisEvent.EventType == Rising_Edge) {
		//Update SM state
		CurrentSMState = NotCranking;

	}
	//else if we get a timeout
	else if (ThisEvent.EventType == ES_TIMEOUT) {
		//Check if we are below max count
		if (counter < (MAX_COUNT)) {
			//Increment counter
			counter++;
			//Increment power gauge
			CurrentTimePercentage++;
			PowerLib_MoveGauge(counter);
			//Init timer
			ES_Timer_InitTimer(CRANK_TIMER, 30);
		}
		//else we have reached maximum count
		else {
		//Post interaction complete to arcade SM
		NewEvent.EventType = Interaction_Completed;
		PostArcadeFSM(NewEvent);
		//Post Power complete to cannon SM
		NewEvent.EventType = Power_Complete;
		PostCannon_SM(NewEvent);
		//Reset counter
		counter = 0;
    //Reset state
    CurrentSMState = Idle_PC;
		//Reset servo position
		CurrentTimePercentage = 0;
		}
	}
	break;
	case Idle_PC:
	//Power regen interaction begin
	if (ThisEvent.EventType == Low_Power) {
		//Check if state is low 
		uint8_t PinState = PowerLib_PinState();
		if (PinState == 1) {
			//Update state
			CurrentSMState = NotCranking;
		}
		//Else it is high
		else {
			//Update state
			CurrentSMState = Cranking;
			//Init timer
			ES_Timer_InitTimer(CRANK_TIMER, 30);
		}
	}
	//Else check for TOT Inserted
	else if (ThisEvent.EventType == Tot_Inserted) {
		//Set gauge to min
		PowerLib_MoveGauge(BEGINNING_TIME);
		counter = 0;
		CurrentTimePercentage = 0;
	}
	
    break;
  }
 
  return ReturnEvent;
}

PowerCrank_SMState_t QueryPowerCrank_SM(void) {
	return CurrentSMState;
}

/***************************************************************************
 private functions
 ***************************************************************************/
 /***
CheckPowerCrankEvents Function Description
 	Arguments: none
 	Returns: bool. True if ther is a new event
 	This function check for the rotation of the power crank


 	Author: Trey Weber
 ***/

 
bool CheckPowerCrankEvents(void){
  ES_Event_t NewEvent;
  bool ReturnVal = false;

  //Get current input state
  uint8_t CurrentInputState = PowerLib_PinState();  
  //Check if state has changed
  if (CurrentInputState != LastInputState) {
    if (CurrentInputState != 0) { //Found rising edge
      NewEvent.EventType = Rising_Edge;
      PostPowerCrank_SM(NewEvent);
      PostResetService(NewEvent);
    }
    else { //Found falling edge
      NewEvent.EventType = Falling_Edge;
      PostPowerCrank_SM(NewEvent);
      PostResetService(NewEvent);
    }
  ReturnVal = true; //Return successfully
  }
 
  //Set new state
  LastInputState = CurrentInputState;
  
  //Return
  return ReturnVal;
}
 
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

