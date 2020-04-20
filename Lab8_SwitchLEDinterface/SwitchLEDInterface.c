// ***** 0. Documentation Section *****
// SwitchLEDInterface.c for Lab 8
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to toggle an LED
// while a button is pressed and turn the LED on when the
// button is released.  This lab requires external hardware
// to be wired to the LaunchPad using the prototyping board.
// January 15, 2016
//      Jon Valvano and Ramesh Yerraballi

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void PortE_Init(volatile unsigned long delay); // PortE initialization
void Delay100ms(unsigned long time); // delay 100ms

// ***** 3. Subroutines Section *****

// PE0, PB0, or PA2 connected to positive logic momentary switch using 10k ohm pull down resistor
// PE1, PB1, or PA3 connected to positive logic LED through 470 ohm current limiting resistor
// To avoid damaging your hardware, ensure that your circuits match the schematic
// shown in Lab8_artist.sch (PCB Artist schematic file) or 
// Lab8_artist.pdf (compatible with many various readers like Adobe Acrobat).
int main(void){ 
//**********************************************************************
// The following version tests input on PE0 and output on PE1
//**********************************************************************
	unsigned long volatile delay;
	unsigned long switchInput;			// In C - you can declare variables only at the beginning of the function (not after any executable code)
  TExaS_Init(SW_PIN_PE0, LED_PIN_PE1, ScopeOn);  // activate grader and set system clock to 80 MHz
  
	PortE_Init(delay);						// dummy variable 
  EnableInterrupts();           // enable interrupts for the grader


  while(1){
		Delay100ms(1);
		switchInput = GPIO_PORTE_DATA_R &0x01;    // read input PE0 into SW1. 
		
		if (switchInput == 0x01) {
			GPIO_PORTE_DATA_R ^= 0x02;	// toggle LED
		}
		else {
			GPIO_PORTE_DATA_R = 0x02;		// turn LED on
		}
  }
  
}
/* System Requirements
In this lab you will build a switch interface that implements positive logic, and you will build an LED interface that implements positive logic.  
You will attach this switch and LED to your protoboard (the white piece with all the holes), and interface them to your TM4C123. 
Overall functionality of this system is similar to Lab 6, with five changes: 
1) the pin to which we connect the switch is moved to PE0, 2) you will have to remove the PUR initialization because pull up is no longer needed. 
3) the pin to which we connect the LED is moved to PE1, 4) the switch is changed from negative to positive logic, and 
5) you should decrease the delay so it flashes about 5 Hz. To flash at 5 Hz means the LED comes on 5 times per second. 
If the switch is pressed we turn on the LED for 100 ms, turn off the LED for 100 ms, and repeat.


1) Make PE1 an output and make PE0 an input.
2) The system starts with the LED on (make PE1 =1).
3) Wait about 100 ms
4) If the switch is pressed (PE0 is 1), then toggle the LED once, else turn the LED on.
5) Steps 3 and 4 are repeated over and over.
*/
void PortE_Init(volatile unsigned long delay) {
	SYSCTL_RCGC2_R |= 0x00000010;   				// 1) Activate clock for Port E. 2^4 = 16 then convert to hexa --> 0x10. -- pg. 465 datasheet
	delay = SYSCTL_RCGC2_R;        				  // allow time for clock to start
  GPIO_PORTE_AMSEL_R &= ~0x03;       		  // 3) disable analog function ON PE0, PE1
	GPIO_PORTE_PCTL_R = 0x00;      					// 4) GPIO clear bit PCTL
	GPIO_PORTE_DIR_R |= 0x02;								// 5) Set PE1 output. 
																					/* Setting a bit in the GPIODIR register configures the corresponding pin to be an output,
																					 * while clearing a bit configures the corresponding pin to be an input.
																					 * All bits are cleared by a reset, meaning all GPIO pins are inputs by default. */
	GPIO_PORTE_AFSEL_R = 0x00;							// 6) Clear bits in the alt function							
																					/* The GPIOAFSEL register is the mode control select register. If a bit is clear, the pin is used as a
																					 * GPIO and is controlled by the GPIO registers. Setting a bit in this register configures the
																					 * corresponding GPIO line to be controlled by an associated peripheral. */
	GPIO_PORTE_DEN_R |= 0x03;								// 7) enable digital pins PE1, PE0. 0x02 + 0x01 = 0x03
	GPIO_PORTE_DATA_R |= 0x02;							// The system starts with the LED ON (make PE1 =1).
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