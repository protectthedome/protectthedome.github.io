/****************************************************************************
 Module
   Meteor.c

 Revision
   1.0.1

 Description
   This module lights up the trace LEDs for the meteors. Utilizes the low level
   ShiftRegisterWrtie_24 module

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/7/19 22:24 ram     initial release
 
****************************************************************************/

// the common headers for C99 types 
#include <stdint.h>
#include <stdbool.h>
#include "termio.h"

#include "BITDEFS.H"

//ShiftRegisterWrtie_24
#include "ShiftRegisterWrite_24.h"

//Definitions
#define BANK0_MASK 0x00FFFF00
#define BANK1_MASK 0x00FF00FF
#define BANK2_MASK 0x0000FFFF

/***Public Functions***/
void Meteor_HWInit(void);
bool Meteor_LightLEDBank(uint8_t BankNum, uint8_t LEDNum);
void Meteor_ClearAll(void);


/***
Meteor_HWInit Function Description
	Arugments: None
	Returns: None
	Call the SR24 init function to intialize hardware to communicate with the 
	cascaded registers
***/
void Meteor_HWInit(void)
{
	//Call SR24_HWInit fucntion
	SR24_Init();
  SR24_Write(0);
	return;
}

/***
Meteor_LightLEDBank Function Description
	Arugments: BankNum, LEDNum
		BankNum: the bank to work with
		LEDNum: LED in the bank wished to light, 0 means all off
	Returns: Status
		If State = false, invalid parameters where given
	Figures out what value to write to SR24 to get the right LED lit. Will use 
	masks to preserve the lighting on any other trace
***/
bool Meteor_LightLEDBank(uint8_t BankNum, uint8_t LEDNum)
{
	//Set return val to false as defualt
	bool ReturnVal = false;
	//If LEDNum > 8 and Bank num  > 2
	if((LEDNum>8)||(BankNum>2))
	{
		//set returnVal to false
		ReturnVal = false;
		//return
		return(false);
	}
	//End if
	//Set return val to true
	ReturnVal = true;
	//Get what is currently in the register
	uint32_t NewSR24Value = SR24_GetCurrentRegister();
	//Create basemask variable (uint 32bit)
	uint32_t BaseMask = 0;
	//Select mask based on bank
	switch (BankNum){
		case 0:
		BaseMask = BANK0_MASK;
		break;
		case 1:
		BaseMask = BANK1_MASK;
		break;
		case 2:
		BaseMask = BANK2_MASK;
		break;
	}
	//Apply mask to register value to zero out the bank
	NewSR24Value &= BaseMask;
	//if LEDNum > 0
	if(LEDNum > 0)
	{
		//Create a uint 32bit variable and make it 1
		uint32_t LEDValue = 1;
		//Shift the 1 left by banknum*8+LEDnum
		uint8_t shiftval;
		shiftval = (BankNum*8)+LEDNum-1;
		LEDValue = LEDValue << shiftval; 
		//OR with masked register value
		NewSR24Value |= LEDValue;
		
	}
	//end if	//call SR24_write to write value to shift register
	SR24_Write(NewSR24Value);
	//Return the return val
	return (ReturnVal);
}

/***
Meteor_ClearAll Function Description
	Arugments: None
	Returns: None
	Calls clear bank for each bank
***/
void Meteor_ClearAll(void)
{
	//Write 0 to SR24
	uint32_t Clear =0;
	SR24_Write(Clear);
	return;
}

/******************************************************************************/
