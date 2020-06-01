// Seungjun Jeon
// Jonic Mecija   #014467048
// CECS 347 PROJECT 1
// September 24, 2018


                        
// 1. Pre-processor Directives Section
// Constant declarations to access port registers using 
// symbolic names instead of addresses
/*
#define NVIC_EN0_R              (*((volatile unsigned long *)0xE000E100))  // IRQ 0 to 31 Set Enable Register
#define NVIC_PRI7_R             (*((volatile unsigned long *)0xE000E41C))  // IRQ 28 to 31 Priority Register
#define GPIO_PORTF_IS_R         (*((volatile unsigned long *)0x40025404))
#define GPIO_PORTF_IBE_R        (*((volatile unsigned long *)0x40025408))
#define GPIO_PORTF_IEV_R        (*((volatile unsigned long *)0x4002540C))
#define GPIO_PORTF_IM_R         (*((volatile unsigned long *)0x40025410))
#define GPIO_PORTF_ICR_R        (*((volatile unsigned long *)0x4002541C))
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define NVIC_SYS_PRI3_R         (*((volatile unsigned long *)0xE000ED20))  // Sys. Handlers 12 to 15 Priority
#define NVIC_ST_CTRL_R          (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R        (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R       (*((volatile unsigned long *)0xE000E018))
#define GPIO_PORTF_RIS_R        (*((volatile unsigned long *)0x40025414))
	
#include "PLL.h"
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "PWM.h"


// 2. Declarations Section
//   Global Variables

double Duty;
int Direction = 0;
int speed = 0;


//   Function Prototypes
void PortF_Init(void);
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode


// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void)
{
	PWM0A_Init(40000,39999);
	PWM0B_Init(40000,39999);
	PLL_Init();
  PortF_Init();        		// Call initialization of portF			// Initialize Mode to Blue/Red
  EnableInterrupts();
	GPIO_PORTF_DATA_R = 0x02;
	
  while(1)
		{
			WaitForInterrupt();
		}
  
}


void GPIOPortF_Handler(void)
{
  if(GPIO_PORTF_RIS_R&0x01)  // SW2 touch
	{
														 // must clear flag or endless interrupts
    GPIO_PORTF_ICR_R = 0x01; // acknowledge flag ( aka clear flag Interrupt Clear Register)
		speed += 1;							 // if sw2 is touched increment 1 
		
		if (Direction == 0)      // BACKWARD
			{
				GPIO_PORTF_DATA_R = 0x02;
				if (speed == 0) 
					{
						Duty = 0;
					}
				else if (speed == 1) // First press 25% speed
					{
						Duty = 2.5;
					}
				else if (speed == 2) // Second press 50% speed
					{
						Duty = 5;
					}
				else								 // Third press 100% speed 
														 // Speed is back to 0. Cycle continues
					{
						Duty = 9.9;
						speed = -1;
					}
			}
			
		if (Direction == 1) 	   // FORWARD
			{
				if (speed == 0)
					{
						Duty = 0;
						speed = 1;
					}
				else if (speed == 1) // First switch 25% speed
					{
						Duty = 2.5;
						speed = 2;
					}
				else if (speed == 2) // Second switch 50% speed
					{
						Duty = 5;
					}
				else 								 // Third switch 100% speed 
														 // Speed is back to 0. Cycle continues
					{
						Duty = 9.9;
						speed = -1;
					}
			}	
	}
	
  if (GPIO_PORTF_RIS_R&0x10) // SW1 touch
		{  
			
			GPIO_PORTF_ICR_R = 0x10;  // acknowledge/clear flag4
			
			if (Direction == 0)						
				{
					Direction = 1;						// SET FORWARD
					GPIO_PORTF_DATA_R = 0x02; // RED LED
				}
				
			if (Direction == 1)
				{
					Direction = 0;						// SET BACKWARD
					GPIO_PORTF_DATA_R = 0x04; // BLUE LED
				}
		}

	PWM0A_Duty(4000*Duty);
	PWM0B_Duty(4000*Duty);

}


// period is 16-bit number of PWM clock cycles in one period (3<=period)
// period for PB6 and PB7 must be the same
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2 
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PB6/M0PWM0

// Port F initialization for two buttons and LEDs
void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;   // (a) activate clock for port F
  delay = SYSCTL_RCGC2_R;
  GPIO_PORTF_LOCK_R = 0x4C4F434B; // unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;         // allow changes to PF4,0
  GPIO_PORTF_DIR_R |= 0x0E;       // (c) make PF4,0 in (built-in button)
  GPIO_PORTF_AFSEL_R &= ~0x1F;    //     disable alt funct on PF4,0
  GPIO_PORTF_DEN_R |= 0x1F;       //     enable digital I/O on PF4,0
  GPIO_PORTF_PCTL_R = 0x00000000; //  configure PF4,0 as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x1F;    //     disable analog functionality on PF4,0
  GPIO_PORTF_PUR_R |= 0x11;       //     enable weak pull-up on PF4,0
  GPIO_PORTF_IS_R &= ~0x11;       // (d) PF4,PF0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;      //     PF4,PF0 is not both edges
  GPIO_PORTF_IEV_R &= ~0x11;      //     PF4,PF0 falling edge event
  GPIO_PORTF_ICR_R = 0x11;        // (e) clear flags 4,0
  GPIO_PORTF_IM_R |= 0x11;        // (f) arm interrupt on PF4,PF0
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00400000; // (g) priority 2
  NVIC_EN0_R = 0x40000000;        // (h) enable interrupt 30 in NVIC
}


// Subroutine to initialize port F pins for input and output
// PF4 and PF0 are input SW1 and SW2 respectively
// PF3,PF2,PF1 are outputs to the LED
// Inputs: None
// Outputs: None
// Notes: These five pins are connected to hardware on the LaunchPad

// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// sky blue -GB    0x0C
// white    RGB    0x0E
// pink     R-B    0x06
*/
// Seungjun Jeon
// Jonic Mecija   #014467048
// CECS 347 PROJECT 1
// October 8, 2018

                   
// 1. Pre-processor Directives Section
// Constant declarations to access port registers using 
// symbolic names instead of addresses
#define NVIC_EN0_R              (*((volatile unsigned long *)0xE000E100))  // IRQ 0 to 31 Set Enable Register
#define NVIC_PRI7_R             (*((volatile unsigned long *)0xE000E41C))  // IRQ 28 to 31 Priority Register
#define GPIO_PORTF_IS_R         (*((volatile unsigned long *)0x40025404))
#define GPIO_PORTF_IBE_R        (*((volatile unsigned long *)0x40025408))
#define GPIO_PORTF_IEV_R        (*((volatile unsigned long *)0x4002540C))
#define GPIO_PORTF_IM_R         (*((volatile unsigned long *)0x40025410))
#define GPIO_PORTF_ICR_R        (*((volatile unsigned long *)0x4002541C))
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define NVIC_SYS_PRI3_R         (*((volatile unsigned long *)0xE000ED20))  // Sys. Handlers 12 to 15 Priority
#define NVIC_ST_CTRL_R          (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R        (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R       (*((volatile unsigned long *)0xE000E018))
#define GPIO_PORTF_RIS_R        (*((volatile unsigned long *)0x40025414))
#include "Nokia5110.h"
#include "PLL.h"
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "PWM.h"
 #define RELEASED 0
#define PRESSED 1   

// 2. Declarations Section
//   Global Variables
int Direction;
int speed;
int dutyA;
int dutyB;
int flag;
unsigned char pressed=0, released = 1;
unsigned char prev_s, current_s, prev_s1, current_s1;
unsigned char count=0;

//   Function Prototypes
void PortF_Init(void);
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode
void PortE_Init(void);
void Delay(void);


// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable

int main(void)
{
	PWM0A_Init(40000, 39999);
	PWM0B_Init(40000, 1);
	PLL_Init();
  PortF_Init();						    // Call initialization of Port F
	PortE_Init();						    // Call initialization of port E
	speed = 0;
	Direction = 1;						 	// Direction is initially forward 
	dutyA = 39999;
  dutyB = 1;
  current_s=RELEASED;
  prev_s = current_s;	
  current_s1=RELEASED;
	Nokia5110_Init();
  prev_s1 = current_s1;	
	GPIO_PORTE_DATA_R = 0x01;
	GPIO_PORTF_DATA_R = 0x02;   // Initialize Mode to Blue/Red
  EnableInterrupts();
  Nokia5110_Clear();
  Nokia5110_OutString("Direction:                          Duty Cycle:   ");
  while(1)
		{		if (prev_s != current_s) {
			Delay();
			prev_s = current_s;
			 prev_s1= current_s1;}
				else{		PWM0A_Duty(dutyA);
						PWM0B_Duty(dutyB);}		}	
}

void GPIOPortF_Handler(void)
{  unsigned long In1; 
		unsigned long In2;

 			In2 = GPIO_PORTF_DATA_R&0x01; // read PF4:sw1 into In
			if((In2 == 0x00)&& (current_s1==RELEASED)){ // zero means SW1 is pressed
						current_s1=PRESSED;
			}
			
			if ((In2 == 0x01)&& (current_s1==PRESSED)){		
					current_s1=RELEASED;			// SW2 touch, control moving direction of car
						GPIO_PORTF_ICR_R = 0x01;	
					if (Direction == 0)	 // forwards
						{ GPIO_PORTE_DATA_R = 0x01;
							Direction = 1;
							if (speed == 1)      // 0%
								{
								dutyA = 39999;
								dutyB = 1;
								GPIO_PORTF_DATA_R = 0x02;
								Nokia5110_SetCursor(3, 2);
								Nokia5110_OutString("Forward ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("    ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("0%");
								}
									
						
						else if (speed == 2)      // 25%
							{
								dutyA = 25000;
								dutyB = 15000;
								GPIO_PORTF_DATA_R = 0x08;
								
								Nokia5110_SetCursor(3, 2);
								Nokia5110_OutString("Forward ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("    ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("25%");
							}
						else if (speed == 3) // 50%
							{
								dutyA = 20000;
								dutyB = 20000;
								GPIO_PORTF_DATA_R = 0x08;
								Nokia5110_SetCursor(3, 2);
								Nokia5110_OutString("Forward ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("    ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("50%");
							}
						else 	if (speed==0)					 //100%
							{
								dutyA=15000;
								dutyB=25000;
								GPIO_PORTF_DATA_R = 0x08;
								Nokia5110_SetCursor(3, 2);
								Nokia5110_OutString("Forward ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("    ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("100%");

							}
						}
				else	if (Direction ==1) {
							Direction = 0;
							GPIO_PORTE_DATA_R = 0x02;
							if (speed == 1)      // 0%
							{
								dutyA = 1;
								dutyB = 39999;
								GPIO_PORTF_DATA_R = 0x02;
								Nokia5110_SetCursor(3, 2);
								Nokia5110_OutString("Backward");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("    ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("0%");
							}
									
						
						else if (speed == 2)      // 25%
							{
								dutyA = 15000;
								dutyB = 25000;
								GPIO_PORTF_DATA_R = 0x04;
								Nokia5110_SetCursor(3, 2);
								Nokia5110_OutString("Backward");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("    ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("25%");

							}
						else if (speed == 3) // 50%
							{
								dutyA = 20000;
								dutyB = 20000;
								GPIO_PORTF_DATA_R = 0x04;
								Nokia5110_SetCursor(3, 2);
								Nokia5110_OutString("Backward");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("    ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("50%");
							}
						else 	if(speed ==0)			 //100%
							{
								dutyA=25000;
								dutyB=15000;
								GPIO_PORTF_DATA_R = 0x04;
								Nokia5110_SetCursor(3, 2);
								Nokia5110_OutString("Backward");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("    ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("100%");
						
							}	
				}
						}
				
	// SW1 touch, increase/change speed of car


				In1 = GPIO_PORTF_DATA_R&0x10; // read PF4:sw1 into In
			if((In1 == 0x00)&& (current_s==RELEASED)){ // zero means SW1 is pressed
						current_s=PRESSED;
			}
			
			if ((In1 == 0x10)&& (current_s==PRESSED)){		
					current_s=RELEASED;			
						speed += 1;
						GPIO_PORTF_ICR_R = 0x10;
						if(Direction ==0){
						if (speed == 1)      // 0%
							{
								dutyA = 1;
								dutyB = 39999;
								GPIO_PORTF_DATA_R = 0x02;
								Nokia5110_SetCursor(3, 2);
								Nokia5110_OutString("Backward");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("    ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("0%");
							}
									
						
						else if (speed == 2)      // 25%
							{
								dutyA = 15000;
								dutyB = 25000;
								GPIO_PORTF_DATA_R = 0x04;
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("25%");
							}
						else if (speed == 3) // 50%
							{
								dutyA = 20000;
								dutyB = 20000;
								GPIO_PORTF_DATA_R = 0x04;
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("50%");
							}
						else 								 //100%
							{
								dutyA = 39999;
								dutyB = 1;
								speed = 0;
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("100%");
							}
						}	
				else	if (Direction ==1){
							if (speed == 1)      // 0%
								{
								dutyA = 39999;
								dutyB = 1;
								GPIO_PORTF_DATA_R = 0x02;
								Nokia5110_SetCursor(3, 2);
								Nokia5110_OutString("Forward ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("    ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("0%");
								}
									
						
						else if (speed == 2)      // 25%
							{
								dutyA = 25000;
								dutyB = 15000;
								GPIO_PORTF_DATA_R = 0x08;
								Nokia5110_SetCursor(3, 2);
								Nokia5110_OutString("Forward ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("25%");
							}
						else if (speed == 3) // 50%
							{
								dutyA = 20000;
								dutyB = 20000;
								Nokia5110_SetCursor(3, 2);
								Nokia5110_OutString("Forward ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("50%");
							}
						else 								 //100%
							{
								dutyA = 1;
								dutyB = 39999;
								speed = 0;
								GPIO_PORTF_DATA_R = 0x08;
								Nokia5110_SetCursor(3, 2);
								Nokia5110_OutString("Forward ");
								Nokia5110_SetCursor(5, 5);          // five leading spaces, bottom row
								Nokia5110_OutString("100%");
							}
			}	
						}	
}
// period is 16-bit number of PWM clock cycles in one period (3<=period)
// period for PB6 and PB7 must be the same
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2 
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PB6/M0PWM0

// Port F initialization for two buttons and LEDs
void PortF_Init(void)
{ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;   // (a) activate clock for port F
  delay = SYSCTL_RCGC2_R;
  GPIO_PORTF_LOCK_R = 0x4C4F434B; // unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;         // allow changes to PF4,0
  GPIO_PORTF_DIR_R |= 0x0E;       // (c) make PF4,0 in (built-in button)
  GPIO_PORTF_AFSEL_R &= ~0x1F;    //     disable alt funct on PF4,0
  GPIO_PORTF_DEN_R |= 0x1F;       //     enable digital I/O on PF4,0
  GPIO_PORTF_PCTL_R = 0x00000000; //  configure PF4,0 as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x1F;    //     disable analog functionality on PF4,0
  GPIO_PORTF_PUR_R |= 0x11;       //     enable weak pull-up on PF4,0
  GPIO_PORTF_IS_R &= ~0x11;       // (d) PF4,PF0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;      //     PF4,PF0 is not both edges
  GPIO_PORTF_IEV_R &= ~0x11;      //     PF4,PF0 falling edge event
  GPIO_PORTF_ICR_R = 0x11;        // (e) clear flags 4,0
  GPIO_PORTF_IM_R |= 0x11;        // (f) arm interrupt on PF4,PF0
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00400000; // (g) priority 2
  NVIC_EN0_R = 0x40000000;        // (h) enable interrupt 30 in NVIC
}

void PortE_Init(void)
{
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x10;      // 1)  E 
  delay = SYSCTL_RCGC2_R;      // 2) no need to unlock
  GPIO_PORTE_AMSEL_R &= ~0x03; // 3) disable analog function on PE2-0
  GPIO_PORTE_PCTL_R &= ~0x000000FF; // 4) enable regular GPIO
  GPIO_PORTE_DIR_R |= 0x03;    // 5) outputs on PE1-0
  GPIO_PORTE_AFSEL_R &= ~0x03; // 6) regular function on PE2-0
  GPIO_PORTE_DEN_R |= 0x03; 
} 

void Delay(void){unsigned long volatile time;
  time = 727240*20/91;  // 0.01sec * 20 for 0.01 sec
  while(time){
		time--;
  }
}

// Subroutine to initialize port F pins for input and output
// PF4 and PF0 are input SW1 and SW2 respectively
// PF3,PF2,PF1 are outputs to the LED
// Inputs: None
// Outputs: None
// Notes: These five pins are connected to hardware on the LaunchPad

// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// sky blue -GB    0x0C
// white    RGB    0x0E
// pink     R-B    0x06
