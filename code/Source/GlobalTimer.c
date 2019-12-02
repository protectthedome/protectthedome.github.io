/****************************************************************************
 Module
   GlobalTimer.c

 Revision
   1.0.1

 Description
   This is the global timer service

 Notes

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this service
*/
#include "GlobalTimer.h"
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
#include "ArcadeFSM.h"




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
#define DeltaTime 600

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint16_t counter = 0;
static uint8_t MyPriority;
static GlobalTimerState_t CurrentSMState;
static uint8_t CurrentTimePercentage = 0;

// add a deferral queue for up to 3 pending deferrals +1 to allow for overhead
static ES_Event_t DeferralQueue[3+1];

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitGlobalTimer

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
     T. Weber
****************************************************************************/
bool InitGlobalTimer ( uint8_t Priority )
{
  MyPriority = Priority;
  
  //Initialize State
  CurrentSMState = NotPlaying_GT;
  
  return true;
}




/****************************************************************************
 Function
     PostGlobalTimer

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
bool PostGlobalTimer( ES_Event_t ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunGlobalTimer

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   
 Author
   T. Weber
****************************************************************************/
ES_Event_t RunGlobalTimer( ES_Event_t ThisEvent )
{
  ES_Event_t NewEvent;
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT;
  
  switch (CurrentSMState) {
    case NotPlaying_GT: 
	//If TOT is inserted
	if (ThisEvent.EventType == Tot_Inserted) {
		//Initialize Timer
		ES_Timer_InitTimer(GLOBAL_TIMER, DeltaTime);
		//Move time display to beginning
		Servo_MoveTimer(BEGINNING_TIME);
		//Reset counter
		counter = 0;
		//Reset time
		 CurrentTimePercentage = 0;
		//Set next state
		CurrentSMState = Playing_GT;
			}
    break;
    case Playing_GT:
	//If reset event or end of game event
	if (ThisEvent.EventType == RST || ThisEvent.EventType == End_Of_Game) {
    //Reset counter
		counter = 0;
		//Reset time
		 CurrentTimePercentage = 0;
    Servo_MoveTimer(BEGINNING_TIME);
    ES_Timer_StopTimer(GLOBAL_TIMER);
		CurrentSMState = NotPlaying_GT;
		}
		//Or if we receive a timeout event
		else if (ThisEvent.EventType == ES_TIMEOUT) {
      
			//If count is max
			if (counter >= MAX_COUNT) {
        printf("\rMAXED");
				//Post a timeout event to Arcade_SM
				NewEvent.EventType = Global_Time_Out;
				PostArcadeFSM(NewEvent);
				//Set SM state
				CurrentSMState = NotPlaying_GT; 
				}
			//Otherwise, increment counter
			else {
				//Increment counter
				counter++;
				//Move time display
				CurrentTimePercentage++;
				//Servo_MoveTimer(BEGINNING_TIME+CurrentTimePercentage*DeltaTime);
        Servo_MoveTimer(counter);
				//Init timer
				ES_Timer_InitTimer(GLOBAL_TIMER, DeltaTime);
				}
			}
    break;
  }
 
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

 
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

