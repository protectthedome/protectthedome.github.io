/****************************************************************************
 Module
   TOT.c

 Revision
   1.0.1

 Description
   This module looks for the TOT on insertion and controls the release of it

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/9/19 14:10 ram     initial release
 
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
#include "TOT.h"
#include "Servo.h"


//defintitions
#define ALL_BITS (0xff<<2) 
#define IR_PORT HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS))

#define IR_PIN BIT0HI

#define OPEN_POS 43
#define CLOSED_POS 0

/***Public Functions***/
void TOT_HWInit(void);
uint8_t TOT_Detect(void);
bool TOT_Release(void);
bool TOT_Block(void);

void TOT_HWInit(void)
{
	//turn on portD
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;
	while((HWREG(SYSCTL_PRGPIO)&SYSCTL_PRGPIO_R3) != SYSCTL_PRGPIO_R3)
	{
	}
	//Set PD0 as digital input
	HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= BIT0HI;
	HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) &= BIT0LO;
  
  //Enable Pull-up on PD0
  HWREG(GPIO_PORTD_BASE+GPIO_O_PUR) |= BIT0HI;
	return;
}

uint8_t TOT_Detect(void)
{
	//Get PORTD value
	uint8_t PortVal;
	PortVal = IR_PORT;
	//Return value at bit0
	return(PortVal & IR_PIN);
}

bool TOT_Release(void)
{
	//call TOTmove servo with needed tick count to open gate
	bool status;
	status = Servo_MoveTOT(OPEN_POS);
	//return status of PWM
	return(status);
}

bool TOT_Block(void)
{
	//call TOTmove servo with needed tick count to close gate
	bool status;
	status = Servo_MoveTOT(CLOSED_POS);
	//return status of PWM
	return(status);
}

