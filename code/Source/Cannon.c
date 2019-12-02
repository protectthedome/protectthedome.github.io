/****************************************************************************
 Module
   Cannon.c

 Revision
   1.0.1

 Description
   This module reads the potentiometer, positions the cannon, vibrates the
   knob and lights up the tip LED on the cannnon. 
   This module uses the Servo library for positioning the cannon. this module
   uses the AD library to read the potentiometer.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/8/19 11:08  ram     initial release
 11/11/19 21:07 ram		Added manual positioning function
 
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
#include "Cannon.h"
#include "Servo.h"
#include "ServoDefs.h"
#include "ADMulti.h"
#include "PWM16Tiva.h"

//Definitions
#define ALL_BITS (0xff<<2) 
#define CANNON_IO_PORT HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA+ALL_BITS))

#define LED_OFF BIT6LO
#define LED_ON BIT6HI
#define VIBE_OFF BIT7LO
#define VIBE_ON BIT7HI
#define READ_BUTTON(x) ((x &BIT5HI)>>5) // will this work?

#define ADC_COUNT_MASK 0x0000FFFF

/***Private Functions Prototypes***/
static uint16_t TrimCount(uint16_t ADCount);
static uint8_t CalcPosition(uint16_t ADCount);

/***Public Functions Prototypes***/
void Cannon_HWInit(void);

void Cannon_LED(UI_State_t state);
void Cannon_Vibrate(UI_State_t state);

uint8_t Cannon_ReadButton(void);
uint16_t Cannon_GetPotValue(void);

bool Cannon_UserPosition(uint16_t ADCount);
bool Cannon_MoveToBank(uint8_t Bank);
bool Cannon_ManualPosition(uint8_t position);

uint8_t QueryCannonPosition(void);
uint8_t QueryBankPositions(uint8_t Bank);



/***Module Level Variables***/
static uint8_t CannonPosition;
static const uint8_t BankPositions[3] = 
{
	74, //Left Bank
	50, //Middle Bank
	27 //Right Bank
};

/***
Cannon_HWInit Function Description
	Arguements: None
	Returns nothing
	Intializes the output pins for controling the LED and vibration motor
	Intializes the input pin to read the button
	Intializes the pin to read the pot using ADC methods
	
***/
void Cannon_HWInit(void)
{
	// set up port C by enabling the peripheral clock, waiting for the 
	// peripheral to be ready and setting the direction
	// of PC5 to input, PC6 & PC7 to output
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
	while((HWREG(SYSCTL_PRGPIO)&SYSCTL_PRGPIO_R2) != SYSCTL_PRGPIO_R2)
	{
	}
	// Set PC6 and PC7 as digital outputs, PC5 as digital input
	HWREG(GPIO_PORTC_BASE+GPIO_O_DEN) |= (BIT5HI|BIT6HI|BIT7HI);
	HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) |= (BIT6HI|BIT7HI);
	HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) &= BIT5LO;
  
  //Enable Pull-up on PC5
  HWREG(GPIO_PORTC_BASE+GPIO_O_PUR) |= BIT5HI;
  
  //Cofigure open-drain on PC6 & PC7
  HWREG(GPIO_PORTF_BASE+GPIO_O_ODR) |= (BIT6HI|BIT7HI);
  
  //Turn everything off
  CANNON_IO_PORT &= LED_OFF;
  CANNON_IO_PORT &= VIBE_OFF;
	
	// setup the analog system to read one channel (PE4)
	ADC_MultiInit(1);
  
	return;
}

/***
Cannon_LED Function Description
	Arguements: state (on or off)
		NOTE: Use "On" or "Off". These are enums defined in the header
	Returns nothing
	If argument is on -> turn on LED
	If argument is off -> turn off LED
	
***/
void Cannon_LED(UI_State_t state)
{
	//If state = ON
	if(state == On)
	{
		// Set pin High
		CANNON_IO_PORT |= LED_ON;
	}
	//Else if state = OFF
	else if(state == Off)
	{
		// Set pin low
		CANNON_IO_PORT &= LED_OFF;
	}
	//end if
	return;
}

/***
Cannon_Vibrate Function Description
	Arguements: state (on or off)
		NOTE: Use "On" or "Off". These are enums defined in the header
	Returns nothing
	If argument is on -> turn on vibration
	If argument is off -> turn off vibration
	
***/
void Cannon_Vibrate(UI_State_t state)
{
	//If state = ON
	if(state == On)
	{
		// Set pin High
		CANNON_IO_PORT |= VIBE_ON;
	}
	//Else if state = OFF
	else if(state == Off)
	{
		// Set pin low
		CANNON_IO_PORT &= VIBE_OFF;
	}
	//end if
	return; 
}

/***
Cannon_ReadButton Function Description
	Arguements: none
	Returns 1 if pin high, 0 if low
	Reads PC5

***/
uint8_t Cannon_ReadButton(void)
{
	uint8_t CannonButtonVal;
	//Read the button pin
	CannonButtonVal = CANNON_IO_PORT;
	CannonButtonVal = READ_BUTTON(CannonButtonVal);
	//return pin value
	return CannonButtonVal;
}

/***
Cannon_GetPotValue Function Description
	Arguements: None
	Returns ADC Count
	Reads the voltage at the pot sweep, trims it, and returns the trimmed
	value for further use
***/
uint16_t Cannon_GetPotValue(void)
{
	//Intialize array to get count
	uint32_t ADC_Results[1];
	//Get ADC count and store to array
	ADC_MultiRead(ADC_Results);
	//Isolate the count and store to int
	uint32_t count32 = ADC_Results[0];
	count32 &= ADC_COUNT_MASK;
	uint16_t CountVal = (uint16_t)count32;
	//Call TrimCount
	CountVal = TrimCount(CountVal);
	// return the count
	return (CountVal);
}


/***
Cannon_UserPosition Function Description
	Arguments: Cannon Position Value
	returns status
	Takes the count value, converts it to a servo tick value and then sends
	the tick count to the servo library
  Updates current cannon position
	NOTE: This function is to be used for any analog movement. 
***/

bool Cannon_UserPosition(uint16_t ADCcount)
{
	//Convert ADC count to position
	uint8_t position;
	position = CalcPosition(ADCcount);
	//Call Servo_MoveCannon and give it the position
	bool PWMStatus;
	PWMStatus = Servo_MoveCannon(position);
	//If movement successful
	if(PWMStatus)
	{
		//update CannonPosition var
		CannonPosition = position;
	}
	//end if
	//Return status of PWM function
	return (PWMStatus);
}

/***
Cannon_MoveToBank Function Description
	Arguments: Bank Index
	returns status
	Takes the Bank index, finds the servo position associated with said bank
  and moves the cannon to the bank.
  Updates current cannon position
	NOTE: This function is to be used for bank-to-bank movement. 
***/
bool Cannon_MoveToBank(uint8_t Bank)
{
	//Set status to false
	bool PWMStatus = false;
	//If bank < 3
	if(Bank <3)
	{
		//Get bank position value
    uint8_t position;
		position = BankPositions[Bank];
		//Call Servo_MoveCannon with bank position
		PWMStatus = Servo_MoveCannon(position);
	}
  	//If movement successful
	if(PWMStatus)
	{
		//update CannonPosition var
		CannonPosition = BankPositions[Bank];
	}
	//end if
	
	//return status
	return (PWMStatus);
}

/***
Cannon_ManualPosition Function Description
	Arguments: position (0-100)
	returns status
	Takes a servo count value and moves the cannon to that servo count
	Updates current cannon position
	NOTE: This function is to be used for sweeping movement. 
***/
bool Cannon_ManualPosition(uint8_t position)
{
	// Call servo function
	bool status;
	status = Servo_MoveCannon(position);
	
	if(status)// Update position count if successful
	{
		CannonPosition = position;
	}
	//return status
	return(status);
}

/***
QueryCannonPosition
  Arguments: none
  returns module level variable CannonPosition
***/
uint8_t QueryCannonPosition(void)
{
	return (CannonPosition);
}

uint8_t QueryBankPositions(uint8_t Bank)
{
  int8_t PosValue;
  if(Bank>2)
  {
    PosValue = 0xFF;
  }
  else
  {
    PosValue = BankPositions[Bank];
  }
  return(PosValue);
  
}
/******************************************************************************/

/***
Cannon_TrimCount Function Description
	Arguments: ADCcount
	Returns trimmed count
	Takes the count value and adjusts it to match thresholds if value exceeds
	NOTE: This function is to be used for any analog movement. 
***/
static uint16_t TrimCount(uint16_t ADCount)
{
	uint16_t TrimmedValue;
	//If value is above higher threshold, set value to upper threshold
	if(ADCount > COUNT_UPPER_LIM)
	{
		TrimmedValue = COUNT_UPPER_LIM;
	}
	//If value is below lower threshold, set value to lower threshold
	else if(ADCount < COUNT_LOWER_LIM)
	{
		TrimmedValue = COUNT_LOWER_LIM;
	}
  else
  {
    	TrimmedValue = ADCount;
  }
	//return value
	return(TrimmedValue);
}

static uint8_t CalcPosition(uint16_t ADCount)
{
	double countRange;
	countRange = COUNT_UPPER_LIM - COUNT_LOWER_LIM;
	double offset = COUNT_LOWER_LIM;
	//Figure out count ratio
	double ratio;
	ratio = ((float)ADCount-offset)/(countRange);

	//Determine tick count based on ratio
	uint8_t ServoCount;
	ServoCount = ratio*100; 
	
	//return tick count
	return(ServoCount);
}
	
//----------------------------------End File------------------------------------//

