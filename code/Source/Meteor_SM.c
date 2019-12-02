/****************************************************************************
 Module
   Meteor_SM.c

 Revision
   1.0.1

 Description
   This is the meteor service under the
   Gen2 Events and Services Framework.

 Notes

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this service
*/
#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>

#include "Meteor_SM.h"
#include <string.h>
#include "Meteor.h"
#include "Cannon.h"
#include "Cannon_SM.h"
#include "ArcadeFSM.h"

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
#define DESTROYED_DELAY 100
#define FALL_TIME 1000-(150*(MachineCounter-1))
#define BLINK_COUNT_MAX 7

//Percentage error allowance (on one side) for cannon localization
#define ERROR 5
#define MAX_COUNT 8

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static uint8_t RandomNext(void);
static uint8_t GetStartingBank(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static Meteor_SMState_t CurrentSMState;
static uint16_t counter = 0;
static uint8_t BankNum = 3; //Initialize to an invalid LED bank
static uint8_t blinkcount = 0;
static uint8_t MachineCounter = 0;

// add a deferral queue for up to 3 pending deferrals +1 to allow for overhead
static ES_Event_t DeferralQueue[3+1];

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMeteor_SM

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
bool InitMeteor_SM ( uint8_t Priority )
{
  MyPriority = Priority;
  
  //Initialize meteor hardware
  Meteor_HWInit();
  
  //Set SM currentstate
  CurrentSMState = Idle_M;
  
  
  return true;
}




/****************************************************************************
 Function
     PostMeteor_SM

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
bool PostMeteor_SM( ES_Event_t ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunMeteor_SM

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
ES_Event_t RunMeteor_SM( ES_Event_t ThisEvent )
{
  ES_Event_t NewEvent;
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT;
  
  switch (CurrentSMState) {
    case Idle_M: 
	//If event is begin meteor interaction
	if (ThisEvent.EventType == Meteor) {
		//Reset count
		counter = 0;
    
		//Init timer
		ES_Timer_InitTimer(METEOR_TIMER, FALL_TIME);
		//Get "random" number
    if(MachineCounter == 0)
    {
    //BankNum = ES_Timer_GetTime() % 3;
      BankNum = GetStartingBank();
    }
    else
    {
      BankNum = RandomNext();
    }
    //printf("\rBankNum: %u\r\n", BankNum);
		//Turn on top LED of bank
		Meteor_LightLEDBank(BankNum, counter + 1);
		//Update current state
		CurrentSMState = MeteorFalling;
    //Increment Machine counter
    //printf("\rMeteor MachineCounter: %u\r\n", MachineCounter);
    MachineCounter++;
    
	}
  else if(ThisEvent.EventType == End_Of_Game || ThisEvent.EventType == RST)
  {
    MachineCounter = 0;
    //printf("\rMeteor MachineCounter: %u\r\n", MachineCounter);
  }
    break;
	case MeteorFalling:
	//Check for reset or end of game event
	if (ThisEvent.EventType == RST || ThisEvent.EventType == End_Of_Game) {
		//Update current state
		CurrentSMState = Idle_M;
		//Reset counter
		counter = 0;
		//Clear all LEDs
		Meteor_ClearAll();
		//Reset BankNum
		BankNum = 3;
	}
	//else if cannon button down
	else if (ThisEvent.EventType == Cannon_Button_Down) {
		//Check guard condition, meteor in range
		uint8_t CannonPos = QueryCannonPosition();
		//Get bank % position value
		uint8_t BankValue = QueryBankPositions(BankNum);
		 if ((CannonPos < BankValue + ERROR) && (CannonPos > BankValue - ERROR)) {
			//Post meteor destroyed to Meteor SM and cannon SM
		  ThisEvent.EventType = Meteor_Destroyed;
      PostCannon_SM(ThisEvent);
      //printf("\rI posted!\r\n");
      PostMeteor_SM(ThisEvent);
		}
	}
	//else if we get a timeout event
	else if (ThisEvent.EventType == ES_TIMEOUT) {
		//If counter is less than max
		if (counter < MAX_COUNT) {
		//Increment counter
		counter++;
		//Decrement LED
		Meteor_LightLEDBank(BankNum, counter + 1);
		//Init timer
    //printf("\rFall Time: %i\r\n", FALL_TIME);
		ES_Timer_InitTimer(METEOR_TIMER, FALL_TIME);
		}
		//else we have reached max count
		else {
		//Reset counter
		counter = 0;
		//Reset BankNum to invalid number
		//BankNum = 3;
		//Update state
		CurrentSMState = Idle_M;
		//Clear LEDs
		Meteor_ClearAll();
		//Post end of game (game over)
		NewEvent.EventType = End_Of_Game;
		NewEvent.EventParam = GAME_OVER;
		ES_PostAll(NewEvent);
		}
	}
	//Else if meteor is destroyed
	else if (ThisEvent.EventType == Meteor_Destroyed) {
		//Reset counter
		//counter = 0;
		//Update state
		CurrentSMState = MeteorFlashing;
    //CurrentSMState = Idle_M;
    ES_Timer_InitTimer(METEOR_TIMER, DESTROYED_DELAY);
    Meteor_LightLEDBank(BankNum, counter + 1);
		//Clear all LEDs
		//Meteor_ClearAll();
		//Reset BankNum to invalid number
		//BankNum = 3;
		//Post interaction complete to Arcade SM
	
	}
	break;
  case MeteorFlashing:
    if (ThisEvent.EventType == ES_TIMEOUT) {
      //printf("meteor timeout");
      if (blinkcount <= BLINK_COUNT_MAX) {
        //printf("1\r\n");
        //we are still flashing
        if ((blinkcount % 2) == 0) {
          //turn off
          Meteor_LightLEDBank(BankNum, 0);
        }
        else {
          //turn on
          Meteor_LightLEDBank(BankNum, counter + 1);
        }
        blinkcount++;
        ES_Timer_InitTimer(METEOR_TIMER, DESTROYED_DELAY);
      }
      else {
         //printf("2");
        //we are done flashing
        //Reset counter
        counter = 0;
        blinkcount = 0;
        //Update state
        CurrentSMState = Idle_M;
        Meteor_ClearAll();
        //Reset BankNum to invalid number
        //BankNum = 3;
        NewEvent.EventType = Interaction_Completed;
        PostArcadeFSM(NewEvent);
      }
    }
  break;
  }
 
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static uint8_t RandomNext(void)
{
  uint8_t ReturnVal = 0xFF; //intialize
  //Get System time
  uint16_t SysTime = ES_Timer_GetTime();
  // See if value is odd or even
  uint8_t odd = SysTime%2;
  //Make decision about next bank num based current bankNum
  switch (BankNum)
  {
    case 0:
    {
      if(odd)
      {
        ReturnVal = 1;
      }
      else
      {
        ReturnVal = 2;
      }
      break;
    }
    case 1:
    {
      if(odd)
      {
        ReturnVal = 0;
      }
      else
      {
        ReturnVal = 2;
      }
      break;
    }
    case 2:
    {
      if(odd)
      {
        ReturnVal = 0;
      }
      else
      {
        ReturnVal = 1;
      }
      break;
    }
  }
    //Return value
    return(ReturnVal);
}

static uint8_t GetStartingBank(void)
{
	//Get current cannon position
	uint8_t CannonPos = QueryCannonPosition();
	//find the distance from each bank
	uint8_t Distances [3] = {0,0,0};
	int8_t index, diff;
	for(index = 0; index < 3; index++)
	{
		diff = (int8_t)(CannonPos - QueryBankPositions(index));
		Distances[index] = abs(diff);
	}
	//find the maximum distance
	uint8_t MaxLoc = 0; 
	//Start loop, run 2 times
	for(index = 0; index<2; index++)
	{
		//compare n+1 to n
		//if n+1 bigger, set max to n+1
		if(Distances[index+1] > Distances[index])
		{
			MaxLoc = index + 1;
		}
		//End if
	}
	//End loop
	//Return bank that is the farthest
	return(MaxLoc);
}
       
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

