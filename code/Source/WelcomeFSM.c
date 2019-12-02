/****************************************************************************
 Module
   WelcomeFSM.c

 Revision
   1.0.1

 Description
   This state machine specifies the behavior of our Arcade in Welcome Mode!

 Notes

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"

// Libraries
#include "WelcomeFSM.h"
#include "Meteor.h"
#include "Cannon.h"
#include "AirLeakLib.h"
#include "PowerLib.h"
#include "ArcadeFSM.h"
#include "Crisis.h"
#include "Servo.h"

/*----------------------------- Module Defines ----------------------------*/
#define STEP_DELAY 100
#define LED_MOD 2 // should be a power of 2

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
void StepCannonGauge(void);
void StepLED(void);

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static WelcomeState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

// Indiates whether cannon + gauges moving CW (true) or CCW (false)
static bool DirectionFlag;

// For LED timing
static uint16_t counter;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitWelcomeFSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     RPT, 11/8/19
****************************************************************************/
bool InitWelcomeFSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = InitPState;
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
     PostWelcomeFSM

 Parameters
     EF_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     RPT, 11/8/19
****************************************************************************/
bool PostWelcomeFSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunWelcomeFSM

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   RPT, 11/8/19
****************************************************************************/
ES_Event_t RunWelcomeFSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ES_Event_t NewEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch (CurrentState)
  {
    case InitPState:        // If current state is initial Psedudo State
    {
      if (ThisEvent.EventType == ES_INIT)    // only respond to ES_Init
      {
        CurrentState = Idle_W;
        NewEvent.EventType = Go_Welcome_Mode;
        PostWelcomeFSM(NewEvent);
      }
    }
    break;

    case Idle_W:
    {
      switch (ThisEvent.EventType)
      {
        case Go_Welcome_Mode: 
        {
          // set DirectionFlag to true (start moving CW)
          DirectionFlag = true;
          // Move the gauges to empty
          AirLeak_MoveGauge(0);
          PowerLib_MoveGauge(0);
          // Move cannon fully to one side
          Cannon_ManualPosition(0);
          // Turn all LEDs off
          Meteor_ClearAll();
          // Turn off Crisis LEDs
          Crisis_AirLeak(Off);
          Crisis_Meteor(Off);
          Crisis_Power(Off);
          // Reset Global Timer
          Servo_MoveTimer(0);
          printf("\rZereod\r\n");
          // set CurrentState to Welcome
          CurrentState = Welcome;
          // Start timer
          ES_Timer_InitTimer(WELCOME_TIMER, STEP_DELAY);
        }
        break;

        default:
          ;
      }
    }
    break;

    case Welcome:
    {
      switch (ThisEvent.EventType)
      {
        case Tot_Inserted: // define this in ES_Configure
        {
          Meteor_ClearAll();
          Crisis_Meteor(Off);
          Crisis_AirLeak(Off);
          Crisis_Power(Off);
          CurrentState = Idle_W;
        }
        break;

        case ES_TIMEOUT:
        //  printf("welcome");
          // turn off all LEDs
          Meteor_ClearAll();
          // Step cannon
          // Step o2 gauge
          // Step power gauge
          StepCannonGauge();
          // if counter % LED_MOD is 0
          if ( (counter % LED_MOD) == 0)
          {
            StepLED();
          }
          // init timer
          ES_Timer_InitTimer(WELCOME_TIMER, STEP_DELAY);
          // increment counter
          counter = counter + 1;

        default:
          
        
          ;
      }
    }
    break;
    default:
      ;
  }
  return ReturnEvent;
}

WelcomeState_t QueryWelcomeState(void)
{
  return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/
void StepCannonGauge(void)
{
  // NOTE: asumming cannon & gauges all at same position
  // get current cannon position
  uint8_t CurrentCannonPosition;
  CurrentCannonPosition = QueryCannonPosition();
  // initialize variable NewCannonPosition
  uint8_t NewCannonPosition;

  // if DirectionFlag = true (moving CW)
  if (DirectionFlag == true)
  {
    // if current servo pos == 100
    if (CurrentCannonPosition == 100)
    {
      // set DirectionFlag to false
      DirectionFlag = false;
      // decrement NewPosition
      NewCannonPosition = CurrentCannonPosition - 1;
      // move cannon + gauges to NewPosition
      Cannon_ManualPosition(NewCannonPosition);
      AirLeak_MoveGauge(NewCannonPosition);
      PowerLib_MoveGauge(NewCannonPosition);
    }
    else
    {
      // increment NewPosition
      NewCannonPosition = CurrentCannonPosition + 1;
      // move cannon + gauges to NewPosition
      Cannon_ManualPosition(NewCannonPosition);
      AirLeak_MoveGauge(NewCannonPosition);
      PowerLib_MoveGauge(NewCannonPosition);
    }
  }
  else
  {
    // if CurrentPosition = 0
    if (CurrentCannonPosition == 0)
    {
      // set DirectionFlag to true
      DirectionFlag = true;
      // increment NewPosition
      NewCannonPosition = CurrentCannonPosition + 1;
      // move cannon + gauges to NewPosition
      Cannon_ManualPosition(NewCannonPosition);
      AirLeak_MoveGauge(NewCannonPosition);
      PowerLib_MoveGauge(NewCannonPosition);

    }
    else {
      // decrement NewPosition
      NewCannonPosition = CurrentCannonPosition - 1;
      // move cannon + gauges to NewPosition
      Cannon_ManualPosition(NewCannonPosition);
      AirLeak_MoveGauge(NewCannonPosition);
      PowerLib_MoveGauge(NewCannonPosition);
    }
  }
}

void StepLED(void)
{
  // create variables to keep tack of which LED in which bank is on
  static uint8_t Bank0LED;
  static uint8_t Bank1LED;
  static uint8_t Bank2LED;

  if (counter == 0)
  {
    Bank0LED = 1;
    Bank1LED = 2;
    Bank2LED = 3;
  }
  // Turn on the 3 LEDs corresponding to current step
  Meteor_LightLEDBank(0, Bank0LED);
  Meteor_LightLEDBank(1, Bank1LED);
  Meteor_LightLEDBank(2, Bank2LED);

  // update which LED will be on for each bank
  Bank0LED = (Bank0LED%8) + 1;
  Bank1LED = (Bank1LED%8) + 1;
  Bank2LED = (Bank2LED%8) + 1;
}
