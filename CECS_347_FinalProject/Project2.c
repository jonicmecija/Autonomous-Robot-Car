// Seungjun Jeon
// Jonic Mecija   #014467048
// CECS 347 Final Pr
// November 5, 2018
                  
// 1) make sure lcd can print anything
// 2) make sure you can get adc
// 3) lookup
									
// 1. Pre-processor Directives Section
// Constant declarations to access port registers using 
// symbolic names instead of addresses
	
#include "tm4c123gh6pm.h"
#include "Nokia5110.h"
#include <stdint.h>
#include "PLL.h"
#include "PWM.h"
#include "ADCSWTrigger.h"

// Function prototypes

void PortB_Init(void);
void delay(double time);
unsigned int getPercentage(unsigned long ADCvalue);
unsigned int getAbs(int n);
unsigned int Table(unsigned long ADCvalue);

void PortB_Init(void){
	volatile unsigned long delay;;
  SYSCTL_RCGC2_R |= 0x00000002;     // 1) B clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTB_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTB_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTB_DIR_R = 0x0F;          // 5) PB2-output 
  GPIO_PORTB_AFSEL_R = 0x00;        // 6) no alterna=te function
  GPIO_PORTB_DEN_R = 0x0F;          // 7) enable digital pins PB2-PB0    
	GPIO_PORTB_DATA_R = 0x00;          // Initialize PB0, PB2 high
}

unsigned int getPercentage(unsigned long ADCvalue){
	unsigned percentage = ADCvalue/40;
	if (percentage >= 100)	percentage = 100;
	return percentage;
}
unsigned int getAbs(int n) 
{ 
  int const mask = n >> (sizeof(int) * 8 - 1); 
  return ((n + mask) ^ mask); 
} 


unsigned int Table(unsigned long ADCvalue){
	//innaccurate after 45cm
	int adcOutput[15] = {2930, 2144, 1640, 1350, 1150,  928,  900, 880, 840, 790, 650, 630, 530};
	int distance[15] =  {  10,   15,   20,   25,   30,   35,   40,  45,  50,  55,  60,  65,  70};
	unsigned int closest = getAbs(ADCvalue-adcOutput[0]);
	unsigned int val = distance[0];
	unsigned int i = 0;
	
	for (i = 0; i < 15; i++){
		if (  (getAbs(adcOutput[i]-ADCvalue)) < closest ){
				closest = getAbs(ADCvalue-adcOutput[i]);
				val = distance[i];
		}
	}
	return val;
}



int main(void){
	unsigned long potentiometer, left, right, percent;
	ADC_Init298();
	PortB_Init();
	Nokia5110_Init();
	PWM0A_Init(40000);         // initialize PWM0, 1000 Hz
  PWM0B_Init(40000);         // initialize PWM0, 1000 Hz
	PLL_Init();           // bus clock at 80 MHz
  while(1){
		ADC_In298(&potentiometer, &left, &right); // sample AIN2(PE1), AIN9 (PE4), AIN8 (PE5)
		percent = getPercentage(potentiometer);
		
	
		if(Table(left) < 30){
			PWM0A_Duty(percent);
				PWM0B_Duty(0);
				}
				
		
		else if (Table(right) < 30){
			PWM0A_Duty(0);
			PWM0B_Duty(percent);
			}
		else{
			PWM0A_Duty(percent);
			PWM0B_Duty(percent);
		}
		Nokia5110_Clear();
		Nokia5110_OutString("L:");
		Nokia5110_OutUDec(Table(left));
	
		Nokia5110_SetCursor(0, 2);
		Nokia5110_OutString("R:");
		Nokia5110_OutUDec(Table(right));
	
		Nokia5110_SetCursor(0, 4);
		Nokia5110_OutString("Duty Cycle:");
		Nokia5110_OutUDec(percent);
	//	delay(1);
		//PWM0A_Duty(percent);
		//PWM0B_Duty(percent);
  }

}
void delay( double time)    // This function provides delay in terms of seconds
{
		//Roughly 1 second delay on 16MHz TM4C
    unsigned char i,j,k,l;
 
    for(i = 0; i < time; i++){
        for(j=0; j<250; j++){
					for(k=0; k< 250; k++){
						for (l=0; l< 60; l++){
						}
					}
			
				}
		}
}

