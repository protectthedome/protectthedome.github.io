/****************************************************************************
 Module
   ArcadeFSM.c

 Revision
   1.0.1

 Description
   This is the ArcadeFSM
   Gen2 Events and Services Framework.

 Notes

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this service
*/
#include "ArcadeFSM.h"
#include <string.h>
#include "Crisis.h"
#include "PowerCrank_SM.h"
#include "Tot.h"
#include "AirLeakLib.h"
#include "Meteor_SM.h"
#include "AirLeak_SM.h"
#include "Servo.h"
#include "Meteor.h"


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
#include "termio.h"




/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)
#define NUM_INTERACTIONS 8 //Not counting initial power regen interaction
#define WIN 1
#define GAME_OVER 0
#define DELAY_TIME 500

#define TOT_TIME 2000 //changed from 2000

#define AIRLEAK_INTERACTION 2
#define METEOR_INTERACTION 3

#define   TIME_BTW_INTERACTIONS (2.5*ONE_SEC)
#define BUZZER_TIME (ONE_SEC)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

bool TotChecker(void);



/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static ArcadeFSMState_t CurrentSMState;
UI_State_t CrisisLEDState;


static const uint8_t InteractionList[NUM_INTERACTIONS] = {METEOR_INTERACTION,
  METEOR_INTERACTION,AIRLEAK_INTERACTION,METEOR_INTERACTION,METEOR_INTERACTION,
  AIRLEAK_INTERACTION,METEOR_INTERACTION,METEOR_INTERACTION};

//Track index of InteractionList
static uint8_t InteractionIndex = 0;
UI_State_t BuzzerState;
static uint8_t LastTotState = 1;


// add a deferral queue for up to 3 pending deferrals +1 to allow for overhead
static ES_Event_t DeferralQueue[3+1];

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitArcadeFSM

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
bool InitArcadeFSM ( uint8_t Priority )
{
  MyPriority = Priority;

  //Initialize TOT
  TOT_HWInit();
  TOT_Block();

  Meteor_LightLEDBank(0, 0);

  //Intialzie Deferral Queue
  ES_InitDeferralQueueWith(DeferralQueue, ARRAY_SIZE(DeferralQueue));

  //Initialize AirLeak HW (includes crisis LEDs)
  AirLeak_Init();

  //Set SM currentstate
  CurrentSMState = NotPlaying;

  return true;
}




/****************************************************************************
 Function
     PostArcadeFSM

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
bool PostArcadeFSM( ES_Event_t ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunArcadeFSM

 Parameters
   ES_Event_t : Tot_Inserted, End_Of_Game, RST, TIME_OUT, Interaction_Completed
   Global_Time_Out

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Controls which is the current active game (power crank, air leak, or meteor)
   Controls the warning LED and BUZZER
   Controls the Release of TOT
   Control End_Of_Game due Global_Time_Out
 Notes

 Author
   Trey Weber
****************************************************************************/
ES_Event_t RunArcadeFSM( ES_Event_t ThisEvent )
{
  ES_Event_t NewEvent;
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT;

  switch (CurrentSMState) {
    case NotPlaying:
	//If TOT is inserted
	if (ThisEvent.EventType == Tot_Inserted) {
    Meteor_LightLEDBank(0, 0);
		//Start Low Power interaction
		NewEvent.EventType = Low_Power;
    PostPowerCrank_SM(NewEvent);
		//Change state to playing
		CurrentSMState = Playing;
		//Turn on Crisis LED
		CrisisLEDState = On;
		Crisis_Power(CrisisLEDState);
    ES_Timer_InitTimer(INTERACTION_TIMER, DELAY_TIME);
	}

    break;
    case Playing:
	//If we get a reset event or end of game
	if (ThisEvent.EventType == RST || ThisEvent.EventType == End_Of_Game) {
		//Reset state to not playing
		InteractionIndex = 0;
    ES_Timer_InitTimer(INTERACTION_TIMER, TOT_TIME);
    TOT_Release();
		CurrentSMState = WaitForTotRelease;
    BuzzerState = Off;
		Crisis_Buzzer(BuzzerState);
	}
	//Otherwise if we get a buzzer timeout
	else if (ThisEvent.EventType == ES_TIMEOUT) {
		//Turn off buzzer
		BuzzerState = Off;
		Crisis_Buzzer(BuzzerState);
		}
	//Otherwise if an interaction is complete
	else if (ThisEvent.EventType == Interaction_Completed) {
		//Turn off warning LEDs
		CrisisLEDState = Off;

		Crisis_Power(CrisisLEDState);
		Crisis_AirLeak(CrisisLEDState);
		Crisis_Meteor(CrisisLEDState);

		//Initialize timer (few seconds)
     ES_Timer_InitTimer(INTERACTION_TIMER, TIME_BTW_INTERACTIONS);

     CurrentSMState = WaitForNext;
	}
	//Otherwise if we get a global timeout
	else if (ThisEvent.EventType == Global_Time_Out) {
		//End of Game
		NewEvent.EventType = End_Of_Game;
    InteractionIndex = 0;

		//If power crank is Idle (query Power crank SM)
		PowerCrank_SMState_t CurrentPowerState = QueryPowerCrank_SM();
		if (CurrentPowerState == Idle_PC) {
			//Post win
			NewEvent.EventParam = WIN;
			ES_PostAll(NewEvent);
		}
		//Else....
		else {
			NewEvent.EventParam = GAME_OVER;
			ES_PostAll(NewEvent);
		}
	 ES_Timer_InitTimer(INTERACTION_TIMER, TOT_TIME);
    CurrentSMState = WaitForTotRelease;
	}
    break;
	case WaitForNext:
	//Check for Timeout
	if (ThisEvent.EventType == ES_TIMEOUT) {
     BuzzerState = Off;
		 Crisis_Buzzer(BuzzerState);
		//Check if we have reached end of event list
		if (InteractionIndex == NUM_INTERACTIONS) {
			//We have run out of events. User wins
			NewEvent.EventType = End_Of_Game;
			NewEvent.EventParam = WIN;
      ES_PostAll(NewEvent);
      InteractionIndex = 0;
      ES_Timer_InitTimer(INTERACTION_TIMER, TOT_TIME);
      CurrentSMState = WaitForTotRelease;
      TOT_Release();
		}
			//Check if next event in list is meteor
		else if (InteractionList[InteractionIndex] == METEOR_INTERACTION) {
			//Is meteor interaction
			//Post meteor interaction
			NewEvent.EventType = Meteor;
			PostMeteor_SM(NewEvent);
			//Increment Interaction Index
			InteractionIndex++;
			//Turn on warning LED
			CrisisLEDState = On;
			Crisis_Meteor(CrisisLEDState);

      CurrentSMState = Playing;
		}
		//Otherwise, check if Air leak event
		else if (InteractionList[InteractionIndex] == AIRLEAK_INTERACTION) {
			//Post Air leak event
			NewEvent.EventType = Leak_Develops;
			PostAirLeak_SM(NewEvent);
			//Increment InteractionIndex
			InteractionIndex++;
			//Turn on warning LED
			CrisisLEDState = On;
			Crisis_AirLeak(CrisisLEDState);
      CurrentSMState = Playing;

      //Init buzzer & timer
      BuzzerState = On;
      Crisis_Buzzer(BuzzerState);
			ES_Timer_InitTimer(BUZZER_TIMER, BUZZER_TIME);
		}
		//Return Error
		else  {
		//One of the above conditions was not satisfied
		ReturnEvent.EventType = ES_ERROR;
		}
	}
	//Else check for RST or End of Game
	else if (ThisEvent.EventType == RST || ThisEvent.EventType == End_Of_Game) {
	  	//Reset state to not playing
		InteractionIndex = 0;
    ES_Timer_InitTimer(INTERACTION_TIMER, TOT_TIME);
    BuzzerState = Off;
    //Release Tot
    TOT_Release();
		Crisis_Buzzer(BuzzerState);
		CurrentSMState = WaitForTotRelease;
	}
	//Else if we get a global timeout event
	else if (ThisEvent.EventType == Global_Time_Out) {
		//End of Game
		NewEvent.EventType = End_Of_Game;
    InteractionIndex = 0;
		//If power crank is Idle (query Power crank SM)
		PowerCrank_SMState_t CurrentPowerState = QueryPowerCrank_SM();
		if (CurrentPowerState == Idle_PC) {
			//Post win
			NewEvent.EventParam = WIN;
			ES_PostAll(NewEvent);
		}
		//Else....
		else {
			NewEvent.EventParam = GAME_OVER;
			ES_PostAll(NewEvent);
		}
    ES_Timer_InitTimer(INTERACTION_TIMER, TOT_TIME);
    CurrentSMState = WaitForTotRelease;
	}
  break;

    case WaitForTotRelease:
	//If TOT is inserted
	if (ThisEvent.EventType == ES_TIMEOUT)
    {
      //Block the TOT channel
      TOT_Block();
      //Set State to not playing
      CurrentSMState = NotPlaying;
      ES_RecallEvents(MyPriority, DeferralQueue);
	}
      else if (ThisEvent.EventType == Tot_Inserted)
    {
      ES_DeferEvent(DeferralQueue, ThisEvent);
    }
	break;
  }

  return ReturnEvent;
}
/***************************************************************************
 service event checking functions
 ***************************************************************************/
 /***
TotChecker Function Description
 	Arguments: none
 	Returns: bool. True if ther is a new event
 	This function check for the insertion of the tot

 	Author: Trey Weber
 ***/

bool TotChecker(void) {
  ES_Event_t NewEvent;
  bool ReturnVal = false;

  //Get current TOT state
  uint8_t CurrentTotState = TOT_Detect();
  //Report change only if we aren't playing
  if (CurrentSMState==NotPlaying){
    //Check if state has changed
    if (CurrentTotState != LastTotState) {
      //Check if high or low
      if (CurrentTotState != 1) {
        //Tot detected, let all these damn services know!
        NewEvent.EventType = Tot_Inserted;
        ES_PostAll(NewEvent);
          }
      //Else TOT has been ejected into outer space
      //Do Nothing!
      ReturnVal = true; //Return successfully
    }
}
  //Set new state
  LastTotState = CurrentTotState;

  //Return
  return ReturnVal;
}
/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
