// MeasurementOfDistance.c
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to periodically initiate a software-
// triggered ADC conversion, convert the sample to a fixed-
// point decimal distance, and store the result in a mailbox.
// The foreground thread takes the result from the mailbox,
// converts the result to a string, and prints it to the
// Nokia5110 LCD.  The display is optional.
// April 8, 2014

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

// this connection occurs in the USB debugging cable
// U0Rx (PA0) connected to serial port on PC
// U0Tx (PA1) connected to serial port on PC
// Ground connected ground in the USB cable

#include "UART.h"

// Slide pot pin 3 connected to +3.3V
// Slide pot pin 2 connected to PE1 and PD3
// Slide pot pin 1 connected to ground

#include "ADC.h"
#include "tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "TExaS.h"

void EnableInterrupts(void);  // Enable interrupts

unsigned char String2[]="abc";
unsigned char String[10];
unsigned long Distance; // units 0.001 cm
unsigned long ADCdata;  // 12-bit 0 to 4095 sample
unsigned long Flag;     // 1 means valid Distance, 0 means Distance is empty



//------------UART_Init------------
// Initialize the UART for 115200 baud rate (assuming 80 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART_Init(void){
	
	SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART1; // activate UART1
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOC; // activate port C
  UART1_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
  UART1_IBRD_R = 43;  // 80,000,000/(16*115,200)) = 43.40278
  UART1_FBRD_R = 26;  //6-bbit fraction, round(0.40278 * 64) = 26
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART1_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART1_CTL_R |= UART_CTL_UARTEN;       // enable UART
  GPIO_PORTC_AFSEL_R |= 0x30;           // enable alt funct on PC5-4
  GPIO_PORTC_DEN_R |= 0x30;             // enable digital I/O on PC5-4
                                        // configure PC5-4 as UART1
  GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0xFF00FFFF)+0x00220000;
  GPIO_PORTC_AMSEL_R &= ~0x30;          // disable analog functionality on PC5-4
}

//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART_OutChar(unsigned char data){
// as part of Lab 11, you modified this program to use UART0 instead of UART1
  while((UART0_FR_R&UART_FR_TXFF) != 0);
  UART0_DR_R = data;
}

//------------UART_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART_OutString(unsigned char buffer[]){
// as part of Lab 11 you implemented this function
	int i=0;
  while(buffer[i]) 
	{ UART_OutChar(buffer[i]); 
    i++; }
}



//-----------------------UART_ConvertDistance-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.001cm)
// Output: store the conversion in global variable String[10]
// Fixed format 1 digit, point, 3 digits, space, units, null termination
// Examples
//    4 to "0.004 cm"  
//   31 to "0.031 cm" 
//  102 to "0.102 cm" 
// 2210 to "2.210 cm"
//10000 to "*.*** cm"  any value larger than 9999 converted to "*.*** cm"
void UART_ConvertDistance(unsigned long n){

// as part of Lab 11 you implemented this function
int i=4;

	if(n>9999) 
	    for(i=0;i<5;i++) 
	   { 	if(i==1) 
			    String[i]='.';
			   else 
					  String[i]='*';
		 } 
	else 
		{
			    do
						{
              int digit=n%10;
							if(i==1) 
							 { String[i]='.';
							   i--; }
               String[i]=(char)(((int)'0')+digit);       
              n/=10;
              i--;
						}while (n > 0);
						
						while(i>-1) 
							{ 
							  if(i==1) 
							  { 
									 String[i]='.';
							     i--; 
								 }    
							  String[i]='0';    
				        i--; 
						   }
		 }	  
		String[5]=' ';
		String[6]='c';
		String[7]='m';
  
}

//********Convert****************
// Convert a 12-bit binary ADC sample into a 32-bit unsigned
// fixed-point distance (resolution 0.001 cm).  Calibration
// data is gathered using known distances and reading the
// ADC value measured on PE1.  
// Overflow and dropout should be considered 
// Input: sample  12-bit ADC sample
// Output: 32-bit distance (resolution 0.001cm)
unsigned long Convert(unsigned long sample){
  return 1.4*sample+218;
}

// Initialize SysTick interrupts to trigger at 40 Hz, 25 ms
void SysTick_Init(unsigned long period){
	
	NVIC_ST_CTRL_R = 0;           // disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;  // reload value for 40Hz frequency (assuming 80MHz) //1999999
															 //	If clock = 80M, cycle time = 1/80M = 12.5ns
															//Systick should interrupt every 1/40 = 25 ms ? SysTick interrupts to sample at 40 Hz
															//Systick count * 12.5ns should be = 25 ms
															//Scount = 25m/12.5n = 2000000
	
  NVIC_ST_CURRENT_R = 0;        // any write to current clears it
  NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0               
  NVIC_ST_CTRL_R = 0x00000007;  // enable with core clock and interrupts
	
}

// executes every 25 ms, collects a sample, converts and stores in mailbox
void SysTick_Handler(void){ 

ADCdata = ADC0_In(); 
Distance = Convert(ADCdata);	

Flag=1;
}	

int main(void){ 
	
  volatile unsigned long delay;
  TExaS_Init(ADC0_AIN1_PIN_PE2, SSI0_Real_Nokia5110_Scope);
	
// initialize ADC0, channel 1, sequencer 3
	 
	 ADC0_Init();       
	 
// initialize Nokia5110 LCD (optional)
	//Nokia5110_Init();

	UART_Init();
	
// initialize SysTick for 40 Hz interrupts

  SysTick_Init(2000000);
  EnableInterrupts();
	
  while(1){
		
// read mailbox
	Flag=0;	
		if(Flag==1)
		{	UART_ConvertDistance(Distance);
			
			Nokia5110_SetCursor(0, 0);
			Nokia5110_OutString(String); 
			//UART_OutString(String); UART_OutChar('\n'); //output to uart
		}    
	 
  }
}