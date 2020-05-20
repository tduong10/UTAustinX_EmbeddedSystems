// TuningFork.c Lab 12
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to create a squarewave at 440Hz.  
// There is a positive logic switch connected to PA3, PB3, or PE3.
// There is an output on PA2, PB2, or PE2. The output is 
//   connected to headphones through a 1k resistor.
// The volume-limiting resistor can be any value from 680 to 2000 ohms
// The tone is initially off, when the switch goes from
// not touched to touched, the tone toggles on/off.
//                   |---------|               |---------|     
// Switch   ---------|         |---------------|         |------
//
//                    |-| |-| |-| |-| |-| |-| |-|
// Tone     ----------| |-| |-| |-| |-| |-| |-| |---------------
//
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include "TExaS.h"
#include "..//tm4c123gh6pm.h"

// global variables
unsigned short tune = 0;
unsigned int prevSW;
unsigned int swNow;
	
// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode

// input from PA3, output from PA2, SysTick interrupts
void Sound_Init(void){ 
	// port A initialization
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000001; 	// 1) activate clock for PortA
	delay = SYSCTL_RCGC2_R; 				// allow time for clock to start
	GPIO_PORTA_LOCK_R = 0x4C4F434B; // 2) unlock GPIO portA
	GPIO_PORTA_CR_R = 0x0C; 				// allow changes to PA2-3
	GPIO_PORTA_AMSEL_R = 0x00;			// 3) disable analog on PA
	GPIO_PORTA_PCTL_R = 0x00000000;	// 4) PCTL GPIO on PA2-3
	GPIO_PORTA_DIR_R = 0x04;				// 5) PA2 out
	GPIO_PORTA_AFSEL_R = 0x00;			// 6) disable alt function
	GPIO_PORTA_PUR_R = 0x00;				// disable pull-up
	GPIO_PORTA_DEN_R = 0x0C; 				// 7) enable digital I/O
	// systick
	NVIC_ST_CTRL_R = 0;							// 1) disable SysTick during setup
	NVIC_ST_RELOAD_R = 90908;				// 2) reload value (assuming 80MHz)
	// freq = 1/T 
	// Reload Value = (clk*Time delay)-1 => (80E6 * 1/880) - 1 = 90908
	NVIC_ST_CURRENT_R = 0; 					// 3) any write to current clears it
	NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0   
  NVIC_ST_CTRL_R = 0x00000007; 		// 4) enable with core clock and interrupts
	// pg. 138 datasheet
	// enable clksrc 2^2 = 0x04, enable INTEN for interrupts 2^1=0x02, enable ENABLE bit 2^0=0x01
  EnableInterrupts();
}

// called at 880 Hz
void SysTick_Handler(void){
	// rising edge: when the input signal is transitioning from a low state (e.g. 0) to a high state (e.g. 1)
	// falling edge: when the input signal is transitioning from a high state (e.g. 1) to a low state (e.g. 0)
	/*
	The ISR is automatically called when the SysTick counter reaches 0, which should happen at 880Hz, and should perform the following tasks:

    Read the switch status and compared with the previous status
    If detects a transition from 0 to 1, changes the sound status
    If the sound status is ON, toggles the output pin (sound ON), otherwise put it to 0 (sound OFF).
	*/
	// periodic polling
	swNow = GPIO_PORTA_DATA_R &0x08; // read PA3

	if (tune == 0 && swNow > prevSW) // rising edge. if swNow pressed then tune on
	{
		tune = 1; 
	}
	else if (tune == 1 && swNow > prevSW) // falling edge. if tune on and pressed again then off
	{
		tune = 0;
	}
	
	if (tune == 0) // off
	{
		GPIO_PORTA_DATA_R &= ~0x04; // output sound off
	}
	else if (tune == 1) // on
	{
		GPIO_PORTA_DATA_R ^= 0x04; // toggle PA2 output (audio jack)
	}
	prevSW = swNow;	
}

int main(void){// activate grader and set system clock to 80 MHz
  TExaS_Init(SW_PIN_PA3, HEADPHONE_PIN_PA2,ScopeOn); 
  Sound_Init();         
  EnableInterrupts();   // enable after all initialization are done
	prevSW = GPIO_PORTA_DATA_R &0x08; // previous input of PA3
  while(1){
    // main program is free to perform other tasks
    // do not use WaitForInterrupt() here, it may cause the TExaS to crash
	}
}
