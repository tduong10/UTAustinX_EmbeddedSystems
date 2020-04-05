// BranchingFunctionsDelays.c Lab 6
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to 
// toggle an LED while a button is pressed and 
// turn the LED on when the button is released.  
// This lab will use the hardware already built into the LaunchPad.
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// built-in connection: PF0 connected to negative logic momentary switch, SW2
// built-in connection: PF1 connected to red LED
// built-in connection: PF2 connected to blue LED
// built-in connection: PF3 connected to green LED
// built-in connection: PF4 connected to negative logic momentary switch, SW1

#include "TExaS.h"

#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOF      0x00000020  // port F Clock Gating Control
#define GPIO_PORTF_LOCK_R				(*((volatile unsigned long *)0x40025520)) // APB. unlocks the port when = 0x4C4F434B. pg. 659-660 for register map
				// the AHB uses a full duplex parallel communication whereas the APB uses massive memory-I/O accesses
				// The AHB bus is much faster than APB. The AHB allows one clock cycle access to the peripherals. 
				// The APB is slower and its access time is minimum of two clock cycles. 

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void PortF_Init(volatile unsigned long delay);				// port F initialization
void Delay100ms(unsigned long time); 									// delay 100ms

// global 
//unsigned long switchInput;				// input from PF4
																	// (threw #268 error code)
																	// In C - you can declare variables only at the beginning of the function (not after any executable code)


int main(void) { 
	unsigned long volatile delay;
	unsigned long switchInput;			// In C - you can declare variables only at the beginning of the function (not after any executable code)
  TExaS_Init(SW_PIN_PF4, LED_PIN_PF2);  // activate grader and set system clock to 80 MHz
  // initialization goes here

	PortF_Init(delay);						// dummy variable since delay variable was included
  EnableInterrupts();           // enable interrupts for the grader
	
	GPIO_PORTF_DATA_R |= 0x04;							// The system starts with the LED ON (make PF2 =1). pg. 662 datasheet.
																					/* The GPIODATA register is the data register. In software control mode, values written in the
																					 * GPIODATA register are transferred onto the GPIO port pins if the respective pins have been
																					 * configured as outputs through the GPIO Direction (GPIODIR) register */
  while(1){
    // body goes here

		
		switchInput = GPIO_PORTF_DATA_R &0x10;    // read PF4 into SW1. & before a variable name means "use the address of this variable"
		if (!switchInput) {							 					// SW1 is pressed
			Delay100ms(1);													// set delay of 0.1 second
			GPIO_PORTF_DATA_R ^= 0x04;							// toggle the LED. didn't work with "switchInput = "
		}
		else {
			GPIO_PORTF_DATA_R = 0x04;								// turn on the blue LED. cannot use "switchInput = " because you cannot change the memory address of a variable
		}
  }
}

void PortF_Init(volatile unsigned long delay) {
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;   // 1) Activate clock for Port F. 2^5 = 32 then convert to hexa --> 0x20. -- pg. 340 datasheet
	delay = SYSCTL_RCGC2_R;        				  // allow time for clock to start
	// GPIO_PORTF_LOCK_R = 0x4C4F434B;   			// 2) unlock GPIO Port F
	GPIO_PORTF_AMSEL_R = 0x00;		 				  // 3) disable analog on PF
	GPIO_PORTF_PCTL_R = 0x00;      					// 4) GPIO clear bit PCTL
	GPIO_PORTF_DIR_R |= 0x04;								// 5) Set PF2 output. 2^2 = 4 then to hexa --> 0x04. pg. 663 datasheet.
																					/* Setting a bit in the GPIODIR register configures the corresponding pin to be an output,
																					 * while clearing a bit configures the corresponding pin to be an input.
																					 * All bits are cleared by a reset, meaning all GPIO pins are inputs by default. */
	GPIO_PORTF_PUR_R |= 0x10;								// enable PUR for PF4. 2^4 = 16 then to hexa --> 0x10. pg. 678 datasheet.
																					/* The GPIOPUR register is the pull-up control register. When a bit is set, a weak pull-up resistor on
                                           * the corresponding GPIO signal is enabled. Setting a bit in GPIOPUR automatically clears the
                                           * corresponding bit in the GPIO Pull-Down Select (GPIOPDR) */
	GPIO_PORTF_AFSEL_R = 0x00;							// 6) Clear bits in the alt function							
																					/* The GPIOAFSEL register is the mode control select register. If a bit is clear, the pin is used as a
																					 * GPIO and is controlled by the GPIO registers. Setting a bit in this register configures the
																					 * corresponding GPIO line to be controlled by an associated peripheral. */
	GPIO_PORTF_DEN_R |= 0x14;								// 7) enable digital pins PF4, PF2. 0x04 + 0x10 = 0x14. pg. 682 datasheet.


	/*
	Initialization of an I/O port

To initialize an I/O port for general use:

    1. Activate the clock for the port in the Run Mode Clock Gating Control Register 2 (RCGC2).
    2. Unlock the port (LOCK = 0x4C4F434B). This step is only needed for pins PC0-3, PD7 and PF0 on TM4C123GXL LaunchPad.
    3. Disable the analog function of the pin in the Analog Mode Select register (AMSEL), because we want to use the pin for digital I/O. If this pin is connected to the ADC or analog comparator, its corresponding bit in AMSEL must be set as 1. 
				In our case, this pin is used as digital I/O, so its corresponding bit must be set as 0.
    4. Clear bits in the port control register (PCTL) to select regular digital function. Each GPIO pin needs four bits in its corresponding PCTL register. Not every pin can be configured to every alternative function.
    5. Set its direction register (DIR). A DIR bit of 0 means input, and 1 means output.
    6. Clear bits in the alternate Function Select register (AFSEL).
    7. Enable digital port in the Digital Enable register (DEN).

    ** Please note that we need to add a short delay between activating the clock and setting the port registers.
		source link: http://shukra.cedt.iisc.ernet.in/edwiki/EmSys:Programming_the_GPIO_in_TM4C123
	*/
}

void Delay100ms(unsigned long time) {
	unsigned long i;
	while(time > 0) {
		i = 1333333;  	// this number means 100ms
		while (i > 0) {
			i--;
		}
		time--;					// decrements every 100 ms
	}
	/* In this case, While loops take 6 cycles to execute. The clock speed is 80 MHz.
		so Time Delay = 6 * no. of iterations required * time period of one clock cycle
		100 * 10^-3 = 6 *n* 1/(80*10^6)
		n = 1333333 */
}
