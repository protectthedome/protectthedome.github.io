/****************************************************************************
 Module
   ShiftRegisterWrite_24.c

 Revision
   1.0.1

 Description
   This module acts as the low level interface to write to 3 cascaded 
   shift registers.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/5/19 19:51 ram     first pass
 
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

// readability defines
#define DATA GPIO_PIN_2

#define SCLK GPIO_PIN_3
#define SCLK_HI BIT3HI
#define SCLK_LO BIT3LO

#define RCLK GPIO_PIN_4
#define RCLK_LO BIT4LO
#define RCLK_HI BIT4HI

#define GET_MSB_IN_LSB(x) ((x & 0x800000)>>23)
#define ALL_BITS (0xff<<2) 
//   #define TEST
// an image of the last 8 bits written to the shift register
static uint32_t LocalRegisterImage=0;

// Create your own function header comment
void SR24_Init(void){

  // set up port A by enabling the peripheral clock, waiting for the 
  // peripheral to be ready and setting the direction
  // of PA2, PA3 & PA4 to output
  // Turn on PORTA
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
	// Wait for the port clock to come online
	while((HWREG(SYSCTL_PRGPIO)&SYSCTL_PRGPIO_R0) != SYSCTL_PRGPIO_R0)
	{
	}
	// Set PA2, PA3, and PA4 as digital outputs
	HWREG(GPIO_PORTA_BASE+GPIO_O_DEN) |= (BIT4HI|BIT3HI|BIT2HI);
	
	HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) |= (BIT4HI|BIT3HI|BIT2HI);
	// Leave the configuration for the above ports as totem-poles
	
  // start with the data & sclk lines low and the RCLK line high
	HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT2LO & SCLK_LO);
	HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) |= RCLK_HI;
	#ifdef TEST
	puts("\n\rShift Register Hardware Initialized.\n\r");
	#endif
	return;
}

// Create your own function header comment
uint32_t SR24_GetCurrentRegister(void){
  return LocalRegisterImage;
}

// Create your own function header comment
void SR24_Write(uint32_t NewValue){

  uint32_t BitCounter;
  LocalRegisterImage = NewValue; // save a local copy

	// lower the register clock
	HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) &= RCLK_LO;
	// Loop through all the bits in dataSR
	BitCounter = 24;
	LocalRegisterImage = NewValue; // copy into module var holder for read back
	int i=0;
	for (i=0;i<BitCounter; i++)
	{
		// Isolate the MSB of NewValue, put it into the LSB position and output to port
		// shift out the data while pulsing the serial clock 
		// Isolate the MSB of NewValue, put it into the LSB position and output to port
		if(NewValue&BIT23HI) 
		{
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) |= BIT2HI;
		}
		else{
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) &= BIT2LO;
		}
		// Pulse PA3
		HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) |= SCLK_HI; // raise SCLK
		HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) &= SCLK_LO; // lower SCLK
		// Shift dataSR left 1 bit
		NewValue = NewValue << 1; 
	}
	// Pulse PB2

// finish looping through bits in NewValue
// raise the register clock to latch the new data
	HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) |= RCLK_HI; 
	return;
}
