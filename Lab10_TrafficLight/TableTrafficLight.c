// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016



/*
West/east: PB5=red light, PB4=yellow light, PB3=green light
South/north: PB2=red light, PB1=yellow light, PB0=green light

North/south car dectector: PE0
East/west car dectector: PE1
Pedestrian dectector: PE2

Don't walk light: PF1 (red LED)
Walk light: PF3 (green LED)
*/


// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****
unsigned short currentState, input;

struct State {
  // 6-bit output. one each light
	unsigned short portB_out;	
	unsigned short portF_out; 
	unsigned long Time;	// delay in 1ms 
	unsigned long Next[8]; // next state for inputs
};

typedef const struct State FSM;
// shortcuts for the different states
#define goW 0 // go west
#define waitW 1 // wait at west
#define goS 2 // go south
#define waitS 3 // wait south
#define goPed 4 // go pedestrians
#define flashON1 5 // flash don't walk
#define flashOFF1 6
#define flashON2 7
#define flashOFF2 8


FSM Traffic[9] = {
	//				output(two #'s), delay 			next states
	/* 0 */				{0x0C, 0x02, 100, {goW, goW, waitW, waitW, waitW, waitW, waitW, waitW}},
	/* 1 */				{0x14, 0x02, 50, {goS, goW, goS, goS, goPed, goPed, goS, goS}}, 
	/* 2 */				{0x21, 0x02, 100, {goS, waitS, goS, waitS, waitS, waitS, waitS, waitS}},
	/* 3 */				{0x22, 0x02, 50, {waitS, waitS, waitS, waitS, goPed, goPed, goPed, goPed}},
	/* 4 */				{0x24, 0x08, 100, {goPed, flashON1, flashON1, flashON1, goPed, flashON1, flashON1, flashON1}},
	/* 5 */				{0x24, 0x02, 20, {flashOFF1, flashOFF1, flashOFF1, flashOFF1, flashOFF1, flashOFF1, flashOFF1, flashOFF1}},
	/* 6 */				{0x24, 0x00, 20, {flashON2, flashON2, flashON2, flashON2, flashON2, flashON2, flashON2, flashON2}},
	/* 7 */				{0x24, 0x02, 20, {flashOFF2, flashOFF2, flashOFF2, flashOFF2, flashOFF2, flashOFF2, flashOFF2, flashOFF2}},
	/* 8 */				{0x24, 0x00, 20, {goPed, goW, goS, goW, goPed, goW, goS, goW}}
};

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void SysTick_Init(void); // SysTick initialization
void SysTick_Wait(unsigned long delay); // SysTick wait
void SysTick_Wait10ms(unsigned long delay);
void Ports_Init(void); // Initialize ports B, E, F


// ***** 3. Subroutines Section *****
void SysTick_Init() { 
	/*
	1. The SysTick STCTRL register enables the SysTick features.	
	2. The STRELOAD register specifies the start value to load into the SysTick Current Value (STCURRENT) register when the counter reaches 0.
	3. The STCURRENT register contains the current value of the SysTick counter.
	*/
	NVIC_ST_CTRL_R = 0x00; 	// 1. Clear the ENABLE (NVIC_ST_CTRL_R) bit to turn off SysTick during initialization.
	NVIC_ST_RELOAD_R = 0x00FFFFFF;	 // 2. Set the RELOAD (NVIC_ST_RELOAD_R) register. 0x00FF.FFFF max value
	NVIC_ST_CURRENT_R = 0x00; 	// 3. Write to the NVIC_ST_CURRENT_R value to clear the counter.
	NVIC_ST_CTRL_R = 0x05; 	// 4. Write the desired mode to the control register, NVIC_ST_CTRL_R. Enable CLK_SRC (2-bit) and SysTick(0-bit)
}

// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void SysTick_Wait(unsigned long delay){ 
  NVIC_ST_RELOAD_R = delay - 1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R &0x00010000) == 0) { // wait for count flag. Count=16-bit. pg. 138 datasheet
  }
}

// 800000*12.5ns equals 10ms
void SysTick_Wait10ms(unsigned long delay){
  unsigned long i;
  for(i=0; i<delay; i++){
    SysTick_Wait(800000);  // wait 10ms
  }
}


void Ports_Init() { 
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000032;     // 1) activate clock for Port B=0x02, E=0x10, and F=0x20
	delay = SYSCTL_RCGC2_R;           // allow time for clock to start
	
	// Port B - PB0-5. 6-traffic light
	GPIO_PORTB_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port B
  GPIO_PORTB_CR_R = 0x3F;           // allow changes to PB5=32, PB4=16, PB3=8, PB2=4, PB1=2, PB0=1. -> 63/16 = 3, r=15. 3/16 = 0, r=3 
	GPIO_PORTB_AMSEL_R = 0x00;        // 3) disable analog on PF
	GPIO_PORTB_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PB0-5
	GPIO_PORTB_DIR_R = 0x3F;          // 5) PB0-5 outputs
  GPIO_PORTB_AFSEL_R = 0x00;        // 6) disable alt funct
	GPIO_PORTB_PUR_R   = 0x00;        // disable pull-up resistor
	GPIO_PORTB_DEN_R = 0x3F;          // 7) enable digital I/O on PB0-5
	
	// Port E - PE0-2. 3 input sensor. one East and one North. one Walk.
	GPIO_PORTE_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port E
  GPIO_PORTE_CR_R = 0x07;           // allow changes to PE1=0x02, PE2=0x04, PE0=0x01. -> 0x07
	GPIO_PORTE_AMSEL_R = 0x00;        // 3) disable analog on PE
	GPIO_PORTE_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PE0-2
	GPIO_PORTE_DIR_R = 0x00;          // 5) PE0-2 inputs
  GPIO_PORTE_AFSEL_R = 0x00;        // 6) disable alt funct
	GPIO_PORTE_PUR_R   = 0x00;        // disable pull-up resistor
	GPIO_PORTE_DEN_R = 0x07;          // 7) enable digital I/O on PE0-2
	
	// Port F - PF1,3. pedestrian lights. 
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x0A;           // allow changes to PF1 = 0x02, PF3 = 0x08. -> 0x0A
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF1,3
  GPIO_PORTF_DIR_R = 0x0A;          // 5) PF1,3 outputs
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct
  GPIO_PORTF_PUR_R = 0x00;          // disable pull-up resistor
  GPIO_PORTF_DEN_R = 0x0A;          // 7) enable digital I/O on PF1,3

}


int main(void){ 
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
	Ports_Init(); // port B, E, F initialization
	SysTick_Init(); // SysTick initialization
  EnableInterrupts();
	currentState = goW; // initial state
  while(1){
		// set output for lights
    GPIO_PORTB_DATA_R = Traffic[currentState].portB_out;  
    GPIO_PORTF_DATA_R = Traffic[currentState].portF_out;  
		
		// set SysTick to current state time. The function SysTick_Wait10ms will wait 10ms times the parameter
		SysTick_Wait10ms(Traffic[currentState].Time);
		
		// get input
		input = GPIO_PORTE_DATA_R;
		
		// changes the current state based on input & current state
		currentState = Traffic[currentState].Next[input];
  }
}

