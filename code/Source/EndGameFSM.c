/****************************************************************************
 Module
   EndGameFSM.c

 Revision
   1.0.1

 Description
   State machine for end of game behavior (both win and lose)

 Notes
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "EndGameFSM.h"
#include "Tot.h"
#include "Crisis.h"
#include "Cannon.h"
#include "AirLeakLib.h"
#include "Meteor.h"
#include "PowerLib.h"
#include "ArcadeFSM.h"
#include "WelcomeFSM.h"




/*----------------------------- Module Defines ----------------------------*/
#define WIN_DELAY 200
#define LOSE_DELAY 100
#define LED_MOD 5
#define MAX( a, b ) ( ( a > b) ? a : b )
#define OFF_DELAY 500

#define TOTAL_NUMBER_LED 8 //total number of LED per bank (8)
#define LAST_LED TOTAL_NUMBER_LED //last LED is number 8, closer to the dome
#define FIRST_LED_BANK 0
#define MIDDLE_LED_BANK 1
#define LAST_LED_BANK 2
#define CANNON_CENTER_POS 50

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static EndGameState_t CurrentState;

// with the introductio n of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

// counter for blinking LEDs (win) or number of steps (lose)
static uint16_t counter;

// from 0-2, for cannon shooting fireworks
static uint8_t position;

// keeps track of whether LEDs/buzzer are on or off
static bool ToggleOn = true;

// true if cannon pos > 50, false if cannon pos < 50
// determines to which side the cannon falls
static bool CannonDirection;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitEndGameFSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

Author: Rachel Thomasson
****************************************************************************/
bool InitEndGameFSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = InitPState_E;
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
     PostEndGameFSM

 Parameters
     EF_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes
Author: Rachel Thomasson
****************************************************************************/
bool PostEndGameFSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunEndGameFSM

 Parameters
   ES_Event_t : ES_INIT, End_Of_Game, ES_TIMEOUT, Tot_Inserted

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Control events during End_Of_Game.
   Different LED pattern and actions for GAME_OVER and Win.


Author: Rachel Thomasson
****************************************************************************/
ES_Event_t RunEndGameFSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch (CurrentState)
  {
    case InitPState_E:        // If current state is initial Psedudo State
    {
      if (ThisEvent.EventType == ES_INIT)    // only respond to ES_Init
      {
        // now put the machine into the actual initial state
        CurrentState = Idle_E;
      }
    }
    break;

    case Idle_E:
    {
      switch (ThisEvent.EventType)
      {
        case End_Of_Game:

        {
          if (ThisEvent.EventParam == WIN)
          {
            // Start timer
            ES_Timer_InitTimer(ENDOFGAME_TIMER, WIN_DELAY);
            // initialize counter to number of LEDs (8)
            counter = TOTAL_NUMBER_LED;
            // initialize position to 0
            position = FIRST_LED_BANK;
            // move cannon to the first position
            Cannon_MoveToBank(position);
            // set current state to HappyDance
            CurrentState = HappyDance;
            printf("Going into win mode \n\r");
          }
          else if (ThisEvent.EventParam == GAME_OVER)
          {
            // start timer
            ES_Timer_InitTimer(ENDOFGAME_TIMER, LOSE_DELAY);
            // determine how far from upright the cannon currently is
            int8_t CannonPosFromUp;
            CannonPosFromUp = (CANNON_CENTER_POS - QueryCannonPosition());
            // decide which way cannon should fall
            if (CannonPosFromUp > 0)
            {
              CannonDirection = true; // should fall to the right
            }
            else
            {
              CannonDirection = false; // should fall to the left
            }
            // set CannonPosFromUp to absolute value
            CannonPosFromUp = abs(CannonPosFromUp);
            // set counter = max of current gauge level or cannon position from upright
            counter = MAX(MAX(AirLeak_QueryGauge(), PowerLib_QueryGauge()), CannonPosFromUp);
            // set CurrentState to SadDance
            CurrentState = SadDance;
            printf("Going into sad mode \n\r");

          }

        }
        break;
        default:
          ;
      }  // end switch on CurrentEvent
    }
    break;

    case HappyDance:
    {
      switch (ThisEvent.EventType)
      {
        case ES_TIMEOUT:

        {
          if (counter > 0)
          {
            if (counter == TOTAL_NUMBER_LED)
            {
              // light up cannon LED
              Cannon_LED(On);
            }
            else{
              Cannon_LED(Off);
            }
            // move LED away from dome by 1
            Meteor_LightLEDBank(position, counter);
            // decrement counter by 1
            counter--;
            // init timer
            ES_Timer_InitTimer(ENDOFGAME_TIMER, WIN_DELAY);
          }
          else if ( (counter == 0) && (position < (LAST_LED_BANK+1)) )
          {
            // increment position
            position = position + 1;
            // move cannon to next position
            Cannon_MoveToBank(position);
            // reset counter
            counter = TOTAL_NUMBER_LED;
            if (position == (LAST_LED_BANK+1))
            {
                // turn off all LEDs
                Meteor_ClearAll();
                Crisis_Meteor(Off);
                Crisis_AirLeak(Off);
                Crisis_Power(Off);
                // wait a hot sec
                ES_Timer_InitTimer(ENDOFGAME_TIMER, OFF_DELAY); //I changed off_delay to zero for testing. It possibly may be interfering with existing timer?
            }
            else
            {
                // init timer
                ES_Timer_InitTimer(ENDOFGAME_TIMER, WIN_DELAY);
            }
          }
          else if (position == (LAST_LED_BANK+1))
          {
            // post Go_Welcome_Mode to WelcomeFSM
            ES_Event_t NextEvent;
            NextEvent.EventType = Go_Welcome_Mode;
            PostWelcomeFSM(NextEvent);
            printf("Going into welcome mode \n\r");
            // set CurrentState to Idle_E
            CurrentState = Idle_E;
          }
        }
        break;
        case Tot_Inserted:
        {
          Meteor_ClearAll();
          Crisis_Meteor(Off);
          Crisis_AirLeak(Off);
          Crisis_Power(Off);
          CurrentState = Idle_E;

        }
        break;
        default:
          ;
      }  // end switch on CurrentEvent
    }
    break;

    case SadDance:
    {
      switch (ThisEvent.EventType)
      {
        case ES_TIMEOUT:
        {
          if (counter == 0)
          {
            // post Go_Welcome_Mode to WelcomeFSM
            ES_Event_t NextEvent;
            NextEvent.EventType = Go_Welcome_Mode;
            PostWelcomeFSM(NextEvent);
            // set CurrentState to Idle_E
            CurrentState = Idle_E;
            printf("Going into welcome mode \n\r");

          }
          else if (counter > 0)
          {
            // move cannon down
            uint16_t NewCannonPos = QueryCannonPosition();
            if (CannonDirection == true)
            {
              NewCannonPos = NewCannonPos - 1;
              Cannon_ManualPosition(NewCannonPos);
            }
            else
            {
              NewCannonPos = NewCannonPos + 1;
              Cannon_ManualPosition(NewCannonPos);
            }
            // move gauges down
            AirLeak_MoveGauge(AirLeak_QueryGauge() - 1);
            PowerLib_MoveGauge(PowerLib_QueryGauge() - 1);

            if (ToggleOn)
            {
              // toggle LEDs touching the dome
              Meteor_LightLEDBank(FIRST_LED_BANK, LAST_LED);
              Meteor_LightLEDBank(MIDDLE_LED_BANK, LAST_LED);
              Meteor_LightLEDBank(LAST_LED_BANK, LAST_LED);
              // toggle indicator lights
              Crisis_Meteor(On);
              Crisis_AirLeak(On);
              Crisis_Power(On);
              // switch ToggleOn to False
              ToggleOn = false;
            }
            else
            {
              // toggle LEDs touching the dome
              Meteor_ClearAll();
              // toggle indicator lights
              Crisis_Meteor(Off);
              Crisis_AirLeak(Off);
              Crisis_Power(Off);
              // switch ToggleOn to True
              ToggleOn = true;
            }
            // decrement counter
            counter = counter - 1;
            if (counter == 0)
            {
                // turn off all LEDs
                Meteor_ClearAll();
                Crisis_Meteor(Off);
                Crisis_AirLeak(Off);
                Crisis_Power(Off);
                // wait a hot sec
                ES_Timer_InitTimer(ENDOFGAME_TIMER, OFF_DELAY);
            }
            else
            {
                // init timer
                ES_Timer_InitTimer(ENDOFGAME_TIMER, LOSE_DELAY);
            }
          }
        }
        break;
        case Tot_Inserted:
        {
          Meteor_ClearAll();
          Crisis_Meteor(Off);
          Crisis_AirLeak(Off);
          Crisis_Power(Off);
          CurrentState = Idle_E;
        }
        break;
        default:
          ;
      }  // end switch on CurrentEvent
    }
    break;

    default:
      ;
  }                                   // end switch on Current State
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
