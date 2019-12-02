/****************************************************************************
 Module
   ServoModule.c

 Revision
   1.0.1

 Description
   This module acts as the low level interface to move the servo motors using
   PWM functions

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/7/19 8:03 ram     initial release

****************************************************************************/
// the common headers for C99 types
#include <stdint.h>
#include <stdbool.h>

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "termio.h"

//include PWM module
#include "PWM16Tiva.h"

// For redability
#include "ServoDefs.h"

/***Private Functions***/
static uint16_t Count2Ticks(uint8_t ServoCount, uint16_t HiLimit, uint16_t LoLimit);
static uint16_t Period2Tick(uint8_t Period_ms);

/***Public Functions***/
bool Servo_HWInit(void);
bool Servo_MoveCannon(uint8_t ServoCount);
bool Servo_MoveO2Gauge(uint8_t ServoCount);
bool Servo_MoveTOT(uint8_t ServoCount);
bool Servo_MoveTimer(uint8_t ServoCount);
bool Servo_MovePowerGauge(uint8_t ServoCount);
uint8_t QueryServoPosition(uint8_t Which);

/***Module Variables***/
uint8_t ServoPositions[5]; // each element for each servo

/***
Servo_HWInit Function Description
	Arguements: None
	Return: PWM Status
	Following function intializes the needed number of PWM channels and sets
	the needed period for each group. To define the period, update the value
	in ServoDefs.h. Period values are in ms
***/
bool Servo_HWInit(void)
{
	// Intialize the channels
	bool InitStatus = PWM_TIVA_Init(NUM_SERVOS);

	// Set period for the cannon and timer servo
	uint16_t Group0Period = Period2Tick(CANNON_TIMER_PERIOD_MS);
	InitStatus &= PWM_TIVA_SetPeriod(Group0Period,CANNON_TIMER_GROUP);

	// Set period for gauge servos
	uint16_t Group1Period = Period2Tick(GAUGE_GROUP_PERIOD_MS);
	InitStatus &= PWM_TIVA_SetPeriod(Group1Period, GAUGE_GROUP);

	// Set period for TOT servo
	uint16_t Group2Period = Period2Tick(TOT_GROUP_PERIOD_MS);
	InitStatus &= PWM_TIVA_SetPeriod(Group2Period, TOT_GROUP);

	// Return state of intialization
	return(InitStatus);
}
/*** Servo_Move[Name] Function Description
	Argument: ServoCount
		Note: Enter a vlue between 0-100. If a value outside this range is
		entered, the function will return false without executing anything.
	Return: PWM Status
	The following functions all behave the same way. They take a count value,
	which reperesents the percentage of motion you want to move them between
	the servo's defined max and min positions (defined in ServoDefs).
***/
bool Servo_MoveCannon(uint8_t ServoCount){
	// Check input range
	if(ServoCount > 100)
	{
		return false;
	}
	// Convert ServoCount to Ticks
	uint16_t TickCount;
	TickCount = Count2Ticks(ServoCount, CANNON_HI_LIM, CANNON_LO_LIM);

	// Set pulse width on cannon PWM channel
	bool status;
	status = PWM_TIVA_SetPulseWidth(TickCount, CANNON_SERVO);

	// Update position value if motion successful
	if(status)
	{
		ServoPositions[CANNON_SERVO] = ServoCount;
	}
	// Retun status
	return(status);
}

bool Servo_MoveTimer(uint8_t ServoCount){
	// Check input range
	if(ServoCount > 100)
	{
		return false;
	}
	// Convert ServoCount to Ticks
	uint16_t TickCount;
	TickCount = Count2Ticks(ServoCount, TIMER_HI_LIM, TIMER_LO_LIM);

	// Set pulse width on TIMER_SERVO PWM channel
	bool status;
	status = PWM_TIVA_SetPulseWidth(TickCount, TIMER_SERVO);

	// Update position value if motion successful
	if(status)
	{
		ServoPositions[TIMER_SERVO] = ServoCount;
	}
	// Retun status
	return(status);
}

bool Servo_MoveO2Gauge(uint8_t ServoCount){
	// Check input range
	if(ServoCount > 100)
	{
		return false;
	}
	// Convert ServoCount to Ticks
	uint16_t TickCount;
	TickCount = Count2Ticks(ServoCount, O2_GAUGE_HI_LIM, O2_GAUGE_LO_LIM);
	// Set pulse width on O2_GAUGE_SERVO PWM channel
	bool status;
	status = PWM_TIVA_SetPulseWidth(TickCount, O2_GAUGE_SERVO);

	// Update position value if motion successful
	if(status)
	{
		ServoPositions[O2_GAUGE_SERVO] = ServoCount;
	}
	// Retun status
	return(status);

}

bool Servo_MovePowerGauge(uint8_t ServoCount){
	// Check input range
	if(ServoCount > 100)
	{
		return false;
	}
	// Convert ServoCount to Ticks
	uint16_t TickCount;
	TickCount = Count2Ticks(ServoCount, POWER_GAUGE_HI_LIM, POWER_GAUGE_LO_LIM);

	// Set pulse width on cannon PWM channel
	bool status;
	status = PWM_TIVA_SetPulseWidth(TickCount, POWER_GAUGE_SERVO);

	// Update position value if motion successful
	if(status)
	{
		ServoPositions[POWER_GAUGE_SERVO] = ServoCount;
	}
	// Retun status
	return(status);
}

bool Servo_MoveTOT(uint8_t ServoCount){
	// Check input range
	if(ServoCount > 100)
	{
		return false;
	}
	// Convert ServoCount to Ticks
	uint16_t TickCount;
	TickCount = Count2Ticks(ServoCount, TOT_HI_LIM, TOT_LO_LIM);

	// Set pulse width on cannon PWM channel
	bool status;
	status = PWM_TIVA_SetPulseWidth(TickCount, TOT_SERVO);

	// Update position value if motion successful
	if(status)
	{
		ServoPositions[TOT_SERVO] = ServoCount;
	}
	// Retun status
	return(status);
}

/*** QueryServoPosition Function Description
	Arguement: Which servo
	Returns: Current count
	This function returns what the last count value that was written to the
	servo. It does not give you a tick value!
***/
uint8_t QueryServoPosition(uint8_t Which)
{
	//Create holder, set to 0xFF by defualt
	uint8_t CurrentPosition;
	CurrentPosition = 0xFF;
	//If Which < NUM_SERVOS
	if(Which < NUM_SERVOS)
	{
		//Get needed element
		CurrentPosition = ServoPositions[Which];
	//End if
	}
	//Return position value
	return (CurrentPosition);
}
/******************************************************************************/

static uint16_t Count2Ticks(uint8_t ServoCount, uint16_t HiLimit, uint16_t LoLimit)
{
	//Intialize variables
	uint16_t TickRange, TickCount;
	// Determine the tick range of the servo
	TickRange = HiLimit - LoLimit;
	// Get ther pecentage of the tick range
	double ratio = (double)ServoCount/100.0;
	//TickAddition = ;
	// Perform y=mx+b, m = range, x=ration, b = lolimit
	TickCount = ratio*TickRange+LoLimit;
  //printf("\rPosition ticks %u\r\n", TickCount);
	return (TickCount);
}

static uint16_t Period2Tick(uint8_t Period_ms)
{
	//Convert to us
	uint16_t Period_us = Period_ms*1000;
	//Multiply by TICK_US (float math)
	uint16_t TickCount;
	TickCount = ((double)Period_us)/TICK_US;
	//return tick value
	return(TickCount);
}
//----------------------------End of File-----------------------------------//
