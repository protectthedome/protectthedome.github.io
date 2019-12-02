/****************************************************************************
 Module
   Cannon_SM.c

 Revision
   1.0.1

 Description
   This is the first service for cannon state machine under the
   Gen2 Events and Services Framework.

 Notes

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this service
*/
#include "Cannon_SM.h"
#include <string.h>
#include <stdlib.h>
#include "Cannon.h"
#include "Servo.h"


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
uint16_t DELTA = 3*(COUNT_UPPER_LIM-COUNT_LOWER_LIM)/100.0;


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
bool PotChecker(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static Cannon_SMState_t CurrentSMState;
static uint16_t CurrentCannonADCount;


// add a deferral queue for up to 3 pending deferrals +1 to allow for overhead
static ES_Event_t DeferralQueue[3+1];

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitCannon_SM

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
bool InitCannon_SM ( uint8_t Priority )
{
  
  ES_Event_t ThisEvent;
  MyPriority = Priority;
  
  //Init cannon hardware
  Cannon_HWInit();
 
  //Initialize cannon position
  CurrentCannonADCount = Cannon_GetPotValue();
  Cannon_UserPosition(CurrentCannonADCount);  
  
  //Set SM currentstate
  CurrentSMState = IdleNotPlaying;
  
  //Intialzie Deferral Queue
  ES_InitDeferralQueueWith(DeferralQueue, ARRAY_SIZE(DeferralQueue));
  
    // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService(MyPriority, ThisEvent) == true)
  {
    return true;
  }
  else
  {
    return false;
  }
  
}




/****************************************************************************
 Function
     PostCannon_SM

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
bool PostCannon_SM( ES_Event_t ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunCannon_SM

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
ES_Event_t RunCannon_SM( ES_Event_t ThisEvent )
{
  //ES_Event_t NewEvent;
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT;
  UI_State_t CannonState;
  
  
  switch (CurrentSMState) {
    case IdleNotPlaying: 
	//If power has been regenerated
	if (ThisEvent.EventType == Power_Complete) {
		//Update SM State
		CurrentSMState = IdlePlaying;
	}
    break;
	case IdlePlaying:
	//If RST or end of game event
	if (ThisEvent.EventType == RST || ThisEvent.EventType == End_Of_Game) {
		//Update SM state
		CurrentSMState = IdleNotPlaying;
	}
	//else if meteor is destroyed
	else if (ThisEvent.EventType == Meteor_Destroyed) {
    //printf("\rMeteor Destroyed\r\n"); 
		//Start timer
		ES_Timer_InitTimer(CANNON_TIMER, 1500);
		//Start vibratory motor
		CannonState = On;
		Cannon_Vibrate(CannonState);
    
		//Update current state
		CurrentSMState = KnobVibrating;
	}
	//else if cannon button is pressed
	else if (ThisEvent.EventType == Cannon_Button_Down) {
		//Start timer
		ES_Timer_InitTimer(CANNON_TIMER, 500);
		//Turn on LED
		CannonState = On;
		Cannon_LED(CannonState);
		//Update SM state
		CurrentSMState = LEDLit;
	}
	//Else if we receive a new pot value
	else if(ThisEvent.EventType == New_Pot_Value) {
     //printf("move to new pot value %u\n\r",ThisEvent.EventParam);
		//Position cannon
		Cannon_UserPosition(ThisEvent.EventParam);		
	}
	break;
	case LEDLit:
	//If RST or end of game event
	if (ThisEvent.EventType == RST || ThisEvent.EventType == End_Of_Game) {
		//Update SM state
		CurrentSMState = IdleNotPlaying;
		//Turn off LED
		CannonState = Off;
		Cannon_LED(CannonState);
	}
	//else if we get a timeout
	else if (ThisEvent.EventType == ES_TIMEOUT) {
		//Update SM state
		CurrentSMState = IdlePlaying;
		//Turn off LED
		CannonState = Off;
		Cannon_LED(CannonState);
    ES_RecallEvents(MyPriority, DeferralQueue);
	}
  else if (ThisEvent.EventType == Meteor_Destroyed)
    {
      ES_DeferEvent(DeferralQueue, ThisEvent);
    }
	break;
	case KnobVibrating:
		if (ThisEvent.EventType == RST || ThisEvent.EventType == End_Of_Game) {
		//Update SM state
		CurrentSMState = IdleNotPlaying;
		//Turn off vibratory motor 
		CannonState = Off;
		Cannon_Vibrate(CannonState);
	}
	//else if we get a timeout
	else if (ThisEvent.EventType == ES_TIMEOUT) {
		//Update SM state
		CurrentSMState = IdlePlaying;
		//Turn off vibratory motor
		CannonState = Off;
		Cannon_Vibrate(CannonState);
	}
	break;
  }
 
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

bool PotChecker(void) {
  ES_Event_t NewEvent;
  bool ReturnVal = false;
  
  //Get AD Pot value
  uint16_t ADPotVal = Cannon_GetPotValue();
  
  //If pot has been turned more than 1%
  if (abs(ADPotVal-CurrentCannonADCount) > DELTA) {
	  //printf("\rNew Pot Value!\r\n");
    //Post to service
	  NewEvent.EventType = New_Pot_Value;
	  NewEvent.EventParam = ADPotVal;
	  PostCannon_SM(NewEvent);
	  ReturnVal = true;
	  //Update AD value
	  CurrentCannonADCount = ADPotVal;
  }
   
   return ReturnVal;
}


 
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

