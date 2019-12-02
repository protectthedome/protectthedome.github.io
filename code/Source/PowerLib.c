/****************************************************************************
 Module
   PowerLib.c

 Revision
   1.0.1

 Description
   This module reads the comparator output and controls the power gauge

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/10/19 16:04 ram     initial release
 
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
#include "PowerLib.h"
#include "Servo.h"
#include "ServoDefs.h"

//Definitions
#define ALL_BITS (0xff<<2) 
#define POWER_IO_PORT HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS))
#define POWER_PIN BIT6HI;
#define READ_PIN(x) ((x &BIT6HI)>>6)

/***Public Functions***/
void PowerLib_HWInit(void);
uint8_t PowerLib_PinState(void);
bool PowerLib_MoveGauge(uint8_t position);
uint8_t PowerLib_QueryGauge(void);

/***Module Level Variables***/
static uint8_t GaugePosition;

void PowerLib_HWInit(void)
{
  //Turn on PORTD
  HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;
	while((HWREG(SYSCTL_PRGPIO)&SYSCTL_PRGPIO_R3) != SYSCTL_PRGPIO_R3)
	{
	}
	// Intialise PD6 as digital input
  HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= BIT6HI;
  HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) &= BIT6LO;
}


uint8_t PowerLib_PinState(void)
{
  //Get the value of PORTD and mask
  uint8_t PinValue;
  PinValue = POWER_IO_PORT;
  //shift the value over
  PinValue = READ_PIN(PinValue);
  //return the value
  return(PinValue);
}

bool PowerLib_MoveGauge(uint8_t position)
{
  //Cal the servo lib function
  bool status;
  status = Servo_MovePowerGauge(position);
  if(status) // Update gauge position if movement sucessful
  {
	  GaugePosition = position;
  }
  //return status
  return(status);
}

uint8_t PowerLib_QueryGauge(void)
{
	return(GaugePosition);
}

