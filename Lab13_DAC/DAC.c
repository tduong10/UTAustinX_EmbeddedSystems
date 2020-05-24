// DAC.c
// Runs on LM4F120 or TM4C123, 
// edX lab 13 
// Implementation of the 4-bit digital to analog converter
// Daniel Valvano, Jonathan Valvano
// December 29, 2014
// Port B bits 3-0 have the 4-bit DAC

#include "DAC.h"
#include "..//tm4c123gh6pm.h"

// **************DAC_Init*********************
// Initialize 4-bit DAC 
// Input: none
// Output: none
void DAC_Init(void){
	unsigned long volatile delay;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; // activate portB
	delay = SYSCTL_RCGC2_R;			// allow time to finish activating
	GPIO_PORTB_AMSEL_R &= ~0x0F;	// no analog
	GPIO_PORTB_PCTL_R = 0x00; // clear PCTL register on PB3-0
	GPIO_PORTB_DIR_R |= 0x0F;			//PB0-3 output
	GPIO_PORTB_DR8R_R |= 0x0F;  // can drive up to 8mA out on PB3-0
	/*
The GPIODR8R register is the 8-mA drive control register. Each GPIO signal in the port can be
individually configured without affecting the other pads. When setting the DRV8 bit for a GPIO signal,
the corresponding DRV2 bit in the GPIODR2R register and DRV4 bit in the GPIODR4R register are
automatically cleared by hardware. The 8-mA setting is also used for high-current operation.
From TM4C123 Datasheet, page 673, GPIODR2R description: "By default, all GPIO pins have 2-mA drive."
	*/
	GPIO_PORTB_AFSEL_R &= ~0x0F; 	// disable alt function on PB0-3
	GPIO_PORTB_DEN_R |= 0x0F;	// enable digital I/O on PB0-3
}


// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Output: none
void DAC_Out(unsigned long data){
	// write value from sinewave array
  GPIO_PORTB_DATA_R = data;
}
