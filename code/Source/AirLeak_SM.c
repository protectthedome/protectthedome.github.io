/****************************************************************************
 Module
  AirLeak_SM.c

 Revision
   1.

 Description
   State machine for Air Leak interaction

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/08/19 19:00 gab      pseudo-code
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this service
*/

#include "AirLeak_SM.h"
#include "AirLeakLib.h"
#include "ResetService.h"
#include "ArcadeFSM.h"

#include <stdio.h>
#include <stdlib.h>
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
#define ONE_SEC 1000 // this time assume a 1.000mS/tick timing
#define MAX_TIME (ONE_SEC*10) //Define maximum time to plugg the leak (10 seconds)
#define STEP_TIME (ONE_SEC/2) //Define Time between steps
#define N_STEPS (MAX_TIME/STEP_TIME) //Define total number of steps
#define MAX_OXIGEN_LEVEL 100 //Define maximum oxigen level
#define STEP_OXIGEN MAX_OXIGEN_LEVEL/N_STEPS //Define step size of oxigen gauge



/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

static void SaveHallData(void) ;
static uint8_t RandomExclusive(uint16_t SysTime);


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static AirLeakState_t CurrentState;

static uint8_t Fan=3; //Active Fan (0,1,2, initiate to an invalid number)
static uint8_t HallData[3];//Hall sensors data
static uint8_t previous_HallData[3];//Hall sensors data
static uint8_t MachineCounter;
static uint8_t Indices[] = {0, 1, 2};


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitializeAirLeak

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any
     other required initialization for this service
 Notes

 Author
    Gabriela Bravo, 11/08/19, 19:04
****************************************************************************/
bool InitAirLeak_SM( uint8_t Priority )
{
  //save initial state of hall sensors
  AirLeak_ReadHallSensors(previous_HallData);

  //Initialize the MyPriority variable with the passed in parameter.
  MyPriority = Priority;

  //Set CurrentState to be Idle_A
  CurrentState=Idle_A;

  return true;
}

/****************************************************************************
 Function
     PostAirLeakSM

 Parameters
     ES_Event_t ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     Gabriela Bravo, 11/08/19, 19:27
****************************************************************************/
bool PostAirLeak_SM( ES_Event_t ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunAirLeakSM

 Parameters
   ES_Event_t : the event to process. This event will be one of:



 Returns
   ES_NO_EVENT

 Description



 Notes

 Author
   Gabriela Bravo, 11/08/19, 19:20
****************************************************************************/
ES_Event_t RunAirLeak_SM( ES_Event_t ThisEvent )
{
  ES_Event_t ReturnEvent;
  ES_Event_t NewEvent;
  ReturnEvent.EventType=ES_NO_EVENT;

    switch (CurrentState)
{
  case Idle_A ://		CurrentState is Idle_A
  {//if tot inserted
    if (ThisEvent.EventType == Tot_Inserted){
      //set oxygen gauge to max
      AirLeak_MoveGauge(MAX_OXIGEN_LEVEL);
      //Indices[] equal {0, 1, 2}
      uint8_t index;
      for(index=0; index <3; index++)
      {
        Indices[index] = index;
      }
    }//end if

    //if Leak_Develops
     if(ThisEvent.EventType == Leak_Develops)
      {

        uint16_t SysTime;
        //Use system time to randomly select the fan
        SysTime = ES_Timer_GetTime();
        Fan = RandomExclusive(SysTime);
        //printf("\rFan: %u\r\n", Fan);
        // Turn the fan on
        AirLeak_FanControl(Fan, On);
        //Start timer
        ES_Timer_InitTimer(AIRLEAK_TIMER, STEP_TIME);
		CurrentState = FanOn;
		//Increment MachineCounter
		MachineCounter++;
      }
      else if(ThisEvent.EventType == End_Of_Game || ThisEvent.EventType == RST)
      {
        MachineCounter = 0;
        //printf("\rMachineCounter: %u\r\n", MachineCounter);
      }
     }

  break;
    //End Idle_A state
  case FanOn ://CurrentState is On
  {
    uint8_t Oxigen_Level = AirLeak_QueryGauge();
    //If TIME_OUT and Oxygen_Level greater than 0
    if (ThisEvent.EventType ==ES_TIMEOUT && Oxigen_Level>0){
      //Decrement oxygen gauge
      AirLeak_MoveGauge(Oxigen_Level - STEP_OXIGEN);
      //init timer
      ES_Timer_InitTimer(AIRLEAK_TIMER, STEP_TIME);
    }//end if

    //If TIME_OUT Oxygen_Level less or equal to 0
    else if (ThisEvent.EventType ==ES_TIMEOUT && Oxigen_Level<=0){
      //Post to all End_Of_Game with parameter Game_Over
      NewEvent.EventType=End_Of_Game;
      NewEvent.EventParam = GAME_OVER;
      ES_PostAll(NewEvent);
      //Turn off fan
      AirLeak_FanControl(Fan,Off);
      //move to Idle_A state
      CurrentState = Idle_A;
      //set Fan to invalid
      Fan=3;
    }//end if


    //If AirflowPlugged
    if(ThisEvent.EventType ==Airflow_Plugged){
      //Turn off Fan
      AirLeak_FanControl(Fan,Off);
      //Post Interaction_Completed to Arcade_SM
      NewEvent.EventType = Interaction_Completed;
      PostArcadeFSM(NewEvent);

      //Set oxygen gauge to maximum
      AirLeak_MoveGauge(MAX_OXIGEN_LEVEL);
      //move to Idle_A state
      CurrentState = Idle_A;
      //set Fan to invalid
      //Fan=3;
    }//end if

    //if RST
    else if(ThisEvent.EventType == RST){
      //Turn off fan
      AirLeak_FanControl(Fan,Off);
      //move to Idle_A state
      CurrentState = Idle_A;
      //FanIndex = 0;
      MachineCounter = 0;
      //set Fan to invalid
      Fan=3;
    }//end if

    //If End_Of_Game
    else if(ThisEvent.EventType == End_Of_Game){
      //Turn off fan
      AirLeak_FanControl(Fan,Off);
      //move to Idle_A state
      CurrentState = Idle_A;
      //FanIndex = 0;
      MachineCounter = 0;
      printf("\rMachineCounter: %u\r\n", MachineCounter);
      //set Fan to invalid
      Fan=3;
    }//end if
}
    break;



}
//Return ES_NO_EVENT
return ReturnEvent;

}

/***************************************************************************
 private functions
 ***************************************************************************/
/****************************************************************************
 Function
    AirPlugChecker

 Parameters
   None

 Returns
   bool. True is a new event has been published

 Description

 Notes

 Author
Gabriela Bravo, 11//19, 16:00
****************************************************************************/
 bool AirPlugChecker(void) {
   ES_Event_t ReturnEvent;
   bool ReturnVal = false;

   //read hall sensors data
   AirLeak_ReadHallSensors(HallData);

	 //if active leak plugged, post to Airleak_SM
   if(Fan<3){
     //printf("checking %u\n\r",HallData[Fan]);
     if(HallData[Fan]==0 ){
       //post that air leak was plugged
       ReturnEvent.EventType=Airflow_Plugged;
       PostAirLeak_SM(ReturnEvent);
       //save hall data as previous state
       SaveHallData();
       //return true
       ReturnVal = true;
     }
 }
   //if any of the sensors changed state, count it as a new interaction
   if ((HallData[0]!=previous_HallData[0]) || (HallData[1]!=previous_HallData[1]) ||
     (HallData[2]!=previous_HallData[2]) ) {
      //post to reset service
     ReturnEvent.EventType=Airflow_Plugged;
     PostResetService(ReturnEvent);
     //return true
     SaveHallData();
     ReturnVal = true;
   }

   //return
     return ReturnVal;
 }

/****************************************************************************
 Function
    SaveHallData

 Parameters
   None

 Returns
   Fucntion that copy array with halldata info to Previous_HallData

 Description

 Notes

 Author
 Gabriela Bravo, 11//19, 19:00
****************************************************************************/
static void SaveHallData(void) {
  //copy to array
	int index = 0;
	for(index=0;index<3;index++)
	{
		//copy by element
		previous_HallData[index] = HallData[index];
 }
}

/***
RandomExclusive Function Description
	Arguments: System Clock Time
	Returns: Fan Number
	This function picks a random fan from the list, return that value, and
	then "removes" that fan from the list so it cannot be picked again.

	Author: R. Merchant
***/
static uint8_t RandomExclusive(uint16_t SysTime)
{
    uint8_t ReturnVal;
    //create static array to store indices
    int8_t counter = 3 - MachineCounter;
    printf("\rCounter: %u\r\n", counter);
    //Create int to track position in the array
    uint8_t index;
    //If count is valid
    if(counter > 0)
    {
        // Get mod of the division by the counter
        index = SysTime%counter;
        // Get array element at mod result
       ReturnVal = Indices[index];
        //Start loop to remove the element and push
        //everything above it down.
        uint8_t lpcnt;
        for (lpcnt = index ; lpcnt < 3; lpcnt++)
        {
            //Over write n element with n+1
            Indices[lpcnt] = Indices[lpcnt+1];
        }
    } // End if
    return(ReturnVal);
}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
