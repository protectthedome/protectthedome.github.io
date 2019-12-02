/****************************************************************************
 Module
   AirLeakLib.c

 Revision
   1.0.1

 Description
   This module controls the 3 fans and reads the 3 hall switches. Uses the 
   ShiftRegisterWrite module to control the fans. 

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/9/19 13:14 ram     initial release
 
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


#include "BITDEFS.H"

//Libraries
#include "AirLeakLib.h"
#include "ShiftRegisterWrite.h"
#include "Servo.h"

#define ALL_BITS (0xff<<2) 
#define HALL_IO_PORT HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS))

#define HALL_SW0 BIT1HI
#define HALL_SW1 BIT2HI
#define HALL_SW2 BIT3HI

//Use pull ups?
#define USE_PULL_UP

/***Public Functions***/
void AirLeak_Init(void);
bool AirLeak_FanControl(uint8_t Which, UI_State_t state);
uint8_t AirLeak_QueryFanState(uint8_t Which);
void AirLeak_ReadHallSensors(uint8_t* data);
bool AirLeak_MoveGauge(uint8_t Position);

/***Module Level Variables***/

static uint8_t GaugePosition;

void AirLeak_Init(void)
{
  // Call SR_Init
	SR_Init();
  SR_Write(0);
  // Turn on PORTD
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;
	while((HWREG(SYSCTL_PRGPIO)&SYSCTL_PRGPIO_R3) != SYSCTL_PRGPIO_R3)
	{
	}
	// Intialise PD1, PD2, PD3 as digital inputs
  HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= (BIT1HI|BIT2HI|BIT3HI);
  HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) &= (BIT1LO&BIT2LO&BIT3LO);
  
	#ifdef USE_PULL_UP
	//Enable Pull-ups
	HWREG(GPIO_PORTD_BASE+GPIO_O_PUR) |= (BIT1HI|BIT2HI|BIT3HI);
	#endif
}


/***
AirLeak_FanControl Functions Description
	Arugements: Which fan, On/Off
		NOTE: Pass On/Off to the function. Defined as enum in header
	Returns: Status of execution
	turns on/off the corresponding fan
***/
bool AirLeak_FanControl(uint8_t Which, UI_State_t state)
{
	//Check valid fan selection
	if(Which >2)
	{
		return false;
	}
	
	//Create 8 bit uint with 1 in the LSB
	uint8_t FanValue = 1;
	
	//Create variable for SR
	uint8_t CurrentRegisterValue = SR_GetCurrentRegister();
	uint8_t NewRegisterValue;
	
	//Shift for the given fan selection
	FanValue = FanValue << Which;
	
	//If on selecgted
	if(state == On)
	{
		//OR with current SR register value
		NewRegisterValue = CurrentRegisterValue|FanValue;
	}
	//Esle if off
	else if(state == Off)
	{
		//Invert value bitwise
		FanValue = ~FanValue;
		//AND with current SR value
		NewRegisterValue = CurrentRegisterValue & FanValue;
  }
	//Endif
	//write to SR
	SR_Write(NewRegisterValue);

	return true;
}
/***
AirLead_ReadHallSensors Function Description
  Arugments: Array of uint8s, 3 long to store the value
  Returns nothing
***/

void AirLeak_ReadHallSensors(uint8_t* data)
{
	//Get PORTD
	uint8_t HallVals = HALL_IO_PORT;
	//Isolate the wanted values
	HallVals &= (HALL_SW0 | HALL_SW1 | HALL_SW2);
	//Assign to the array
	int index = 0;
	for(index=0;index<3;index++)
	{
		//Assign to array index. Mask bit 1 and shift right 1
		*data = (HallVals & BIT1HI) >> 1;
    data++;
		//Shift right 1
		HallVals = HallVals >> 1;
	}
	return;
}
	
bool AirLeak_MoveGauge(uint8_t position)
{
	//Call Servo function
	bool status = Servo_MoveO2Gauge(100-position);
	if(status) // Update gauge position if movement sucessful
	{
		GaugePosition = position;
	}
	return (status);
}

uint8_t AirLeak_QueryGauge(void)
{
	return(GaugePosition);
}
/*** 
AirLeak_QueryFanState Function Description
	Arguments: Which fan
	Returns: State of fan
		NOTE: 1 = on, 0 = off
	Uses the value that was last shifted out to determine it
***/

uint8_t AirLeak_QueryFanState(uint8_t Which)
{
	//Get SRimage and shift over by which fan
  uint8_t CurrentRegisterValue;
	CurrentRegisterValue = SR_GetCurrentRegister() >> Which;
	//grab LSB and return it
	return(CurrentRegisterValue&BIT0HI);
}
