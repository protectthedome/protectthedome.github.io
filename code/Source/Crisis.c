/****************************************************************************
 Module
   Crisis.c

 Revision
   1.0.1

 Description
   This module controls the crisis LEDs and buzzer. Uses the SR8 module.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/9/19 14:10 ram     initial release
 
****************************************************************************/

// the common headers for C99 types 
#include <stdint.h>
#include <stdbool.h>

#include "BITDEFS.H"

// Libraries
#include "ShiftRegisterWrite.h"
#include "Crisis.h"

//definitions
#define AIRLEAK_LED BIT5HI
#define METEOR_LED BIT6HI
#define POWER_LED BIT7HI

#define BUZZER BIT4HI


/***Public Functions***/
void Crisis_Meteor(UI_State_t state);
void Crisis_AirLeak(UI_State_t state);
void Crisis_Power(UI_State_t state);
void Crisis_Buzzer(UI_State_t state);

/***
Crisis_Meteor function Description
	Arguments: On/Off
		NOTE: Pass "On" or "Off", type defines in header
	Returns nothing
	Affects the needed bit on the SR to get the desired crisis state
***/
void Crisis_Meteor(UI_State_t state)
{
	//get Current register val
	uint8_t NewValue= SR_GetCurrentRegister();
	
	//Mask to update bit based on state
	if(state == On)
	{
		NewValue |= METEOR_LED;
	}
	else if(state == Off)
	{
		NewValue &= (~METEOR_LED);
	}
	//Write to SR8
	SR_Write(NewValue);
	return;
}

/***
Crisis_AirLeak function Description
	Arguments: On/Off
		NOTE: Pass "On" or "Off", type defines in header
	Returns nothing
	Affects the needed bit on the SR to get the desired crisis state
***/
void Crisis_AirLeak(UI_State_t state)
{
	//get Current register val
	uint8_t NewValue= SR_GetCurrentRegister();
	
	//Mask to update bit based on state
	if(state == On)
	{
		NewValue |= AIRLEAK_LED;
	}
	else if(state == Off)
	{
		NewValue &= (~AIRLEAK_LED);
	}
	//Write to SR8
	SR_Write(NewValue);
	return;
}

/***
Crisis_Power function Description
	Arguments: On/Off
		NOTE: Pass "On" or "Off", type defines in header
	Returns nothing
	Affects the needed bit on the SR to get the desired crisis state
***/
void Crisis_Power(UI_State_t state)
{
	//get Current register val
	uint8_t NewValue= SR_GetCurrentRegister();
	
	//Mask to update bit based on state
	if(state == On)
	{
		NewValue |= POWER_LED;
	}
	else if(state == Off)
	{
		NewValue &= (~POWER_LED);
	}
	//Write to SR8
	SR_Write(NewValue);
	return;
}

/***
Crisis_Buzzer function Description
	Arguments: On/Off
		NOTE: Pass "On" or "Off", type defines in header
	Returns nothing
	Affects the needed bit on the SR to get the desired crisis state
***/
void Crisis_Buzzer(UI_State_t state)
{
	//get Current register val
	uint8_t NewValue= SR_GetCurrentRegister();
	
	//Mask to update bit based on state
	if(state == On)
	{
		NewValue |= BUZZER;
	}
	else if(state == Off)
	{
		NewValue &= (~BUZZER);
	}
	//Write to SR8
	SR_Write(NewValue);
	return;
}
