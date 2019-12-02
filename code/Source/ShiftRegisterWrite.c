/****************************************************************************
 Module
   ShiftRegisterWrite.c

 Revision
   1.0.1

 Description
   This module acts as the low level interface to a write only shift register.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/11/15 19:55 jec     first pass
 
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
#define DATA GPIO_PIN_0

#define SCLK GPIO_PIN_1
#define SCLK_HI BIT1HI
#define SCLK_LO BIT1LO

#define RCLK GPIO_PIN_2
#define RCLK_LO BIT2LO
#define RCLK_HI BIT2HI

#define GET_MSB_IN_LSB(x) ((x & 0x80)>>7)
#define ALL_BITS (0xff<<2) 
//   #define TEST
// an image of the last 8 bits written to the shift register
static uint8_t LocalRegisterImage=0;

// Create your own function header comment
void SR_Init(void){

  // set up port B by enabling the peripheral clock, waiting for the 
  // peripheral to be ready and setting the direction
  // of PB0, PB1 & PB2 to output
  // Turn on PORTB
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	// Wait for the port clock to come online
	while((HWREG(SYSCTL_PRGPIO)&SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1)
	{
	}
	// Set PB0, PB1, and PB2 as digital outputs
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (BIT2HI|BIT1HI|BIT0HI);
	
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (BIT2HI|BIT1HI|BIT0HI);
	// Leave the configuration for the above ports as totem-poles
	
  // start with the data & sclk lines low and the RCLK line high
	HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT0LO & SCLK_LO);
	HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) |= RCLK_HI;
	#ifdef TEST
	puts("\n\rShift Register Hardware Initialized.\n\r");
	#endif
	return;
}

// Create your own function header comment
uint8_t SR_GetCurrentRegister(void){
  return LocalRegisterImage;
}

// Create your own function header comment
void SR_Write(uint8_t NewValue){

  uint8_t BitCounter;
  LocalRegisterImage = NewValue; // save a local copy

	// lower the register clock
	HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) &= RCLK_LO;
	// Loop through all the bits in dataSR
	BitCounter = 8;
	LocalRegisterImage = NewValue; // copy into module var holder for read back
	int i=0;
	for (i=0;i<BitCounter; i++)
	{
		// Isolate the MSB of NewValue, put it into the LSB position and output to port
		// shift out the data while pulsing the serial clock 
		// Isolate the MSB of NewValue, put it into the LSB position and output to port
		if(NewValue&BIT7HI) 
		{
			HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) |= BIT0HI;
		}
		else{
			HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) &= BIT0LO;
		}
		// Pulse PBO
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) |= SCLK_HI; // raise SCLK
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) &= SCLK_LO; // lower SCLK
		// Shift dataSR left 1 bit
		NewValue = NewValue << 1; 
	}
	// Pulse PB2

// finish looping through bits in NewValue
// raise the register clock to latch the new data
	HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) |= RCLK_HI; 
	return;
}


/* Test Harness for SR module*/

#ifdef TEST
#include "termio.h"
int main(void)
{
	TERMIO_Init();

  printf("testing SR module\r\n");
	SR_Init(); 
	while(!kbhit()){
		// Set Q0 high without affecting other bits
		SR_Write(LocalRegisterImage |= BIT0HI);
		
		// Set Q0 low without affecting other bits
		SR_Write(LocalRegisterImage &= BIT0LO);
	}
	return 0;
}	
#endif
