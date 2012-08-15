//*************************************************************************************************
// MSP430 Reaction Game
//
// By: Dillon Nichols
// http://tinkeringetc.blogspot.com/p/msp430-reaction-game.html
//
// Created in Code Composer Studio v4.2.5
//
// Description: 
// Drives 8 LEDs using a shift register at a user selected rate 
// The object of the game is to stop the LEDs at a specific LED
//
// Special thanks to:
// * TI's example code for the ADC10 located at http://www.ti.com/litv/zip/slac463a
// * Andrew Morton's example of interfacing the 74HC595 shift register to the MSP430 located at
//   http://processors.wiki.ti.com/index.php/MSP430_Launchpad_Shift_Register
// * Christopher Chenoweth's delay function located at: http://www.threadabort.com/?p=26
// * The "SCIENTIFIC INSTRUMENTS USING THE TI MSP430" blog located at http://mspsci.blogspot.com/
//
// This work is licensed under a Creative Commons Attribution 3.0 Unported License.
//*************************************************************************************************
#include "msp430g2231.h"
 
// Define pins
#define POT   BIT1 	// POTENTIOMETER -> P1.1
#define DATA  BIT3 	// PIN 14 OF 74HC595 -> P1.3
#define CLOCK BIT4 	// PIN 11 OF 74HC595 -> P1.4
#define LATCH BIT5 	// PIN 12 OF 74HC595 -> P1.5
#define STOP  BIT6 	// GAME STOP / RESTART BUTTON -> P1.6
#define START BIT7	// LED START / RESET BUTTON -> P1.7

// Declare functions
void game (void);
void stop (void);
void restart (void);
void delay (unsigned int);
void pinWrite (unsigned int, unsigned char);
void pulseClock (void);
void shiftOut (unsigned char);

// Store time between LED flashes as a global variable so all functions can access it
int delayTime;		

// Uses potentiometer through the ADC10 to select how quickly LEDs will cycle
int main(void) {
	WDTCTL = WDTPW + WDTHOLD;					// Stop WDT
	ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE;	// ADC10ON, interrupt enabled
	ADC10CTL1 = INCH_1;							// input A1
	ADC10AE0 |= 0x02;							// P1.1 ADC input select
	P1DIR |= (DATA + CLOCK + LATCH);			// Setup shift register pins as outputs

  	// Reads ADC values and lights LEDS and sets the delay time corresponding to the input voltage
  	// Lower values on the ADC equals more delayTime
	for (;;) {
		ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
  		__bis_SR_register(CPUOFF + GIE);        // LPM0, ADC10_ISR will force exit
		
		if (ADC10MEM > 0x000) {
  			shiftOut(1 << 7);
  			delayTime = 100;  }
	  	if (ADC10MEM > 0x07F) {
  			shiftOut(1 << 6);
  			delayTime = 90;   }
	  	if (ADC10MEM > 0x0FF) {
		  	shiftOut(1 << 5);
		  	delayTime = 80;   }
	  	if (ADC10MEM > 0x17F) {
		  	shiftOut(1 << 4);
		  	delayTime = 70;   }
	  	if (ADC10MEM > 0x1FF) {
		  	shiftOut(1 << 3);
		  	delayTime = 60;   }
	  	if (ADC10MEM > 0x27F) {
		  	shiftOut(1 << 2);
		  	delayTime = 50;   }
		if (ADC10MEM > 0x2FF) {
		  	shiftOut(1 << 1);
		  	delayTime = 40;   }
		if (ADC10MEM > 0x37F) {
	    	shiftOut(1 << 0);
	    	delayTime = 30;   }
	  	if ((P1IN & START) == START) {	// When the START button is pressed, the game
	  		game();         }			// will start with the selected delayTime	
	}
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF);	// Clear CPUOFF bit from 0(SR)
}

// Function that flashes the LEDs back and forth with a delay between LEDs of delayTime
// Waits for the STOP button to be pressed to stop the LED and win/(lose?) the game
void game(void){
	for (;;){
    	shiftOut(1 << 0);
      	delay(delayTime); 
      	if ((P1IN & STOP) == STOP) {stop();}
    	shiftOut(1 << 1);
      	delay(delayTime); 
      	if ((P1IN & STOP) == STOP) {stop();}
      	shiftOut(1 << 2);
      	delay(delayTime); 
      	if ((P1IN & STOP) == STOP) {stop();}
      	shiftOut(1 << 3);
      	delay(delayTime); 
      	if ((P1IN & STOP) == STOP) {stop();}
      	shiftOut(1 << 4);
      	delay(delayTime); 
      	if ((P1IN & STOP) == STOP) {stop();}
      	shiftOut(1 << 5);
      	delay(delayTime); 
      	if ((P1IN & STOP) == STOP) {stop();}
      	shiftOut(1 << 6);
      	delay(delayTime); 
      	if ((P1IN & STOP) == STOP) {stop();}
      	shiftOut(1 << 7);
      	delay(delayTime); 
      	if ((P1IN & STOP) == STOP) {stop();}
      	shiftOut(1 << 6);
      	delay(delayTime); 
      	if ((P1IN & STOP) == STOP) {stop();}
      	shiftOut(1 << 5);
      	delay(delayTime); 
      	if ((P1IN & STOP) == STOP) {stop();}
      	shiftOut(1 << 4);
      	delay(delayTime); 
      	if ((P1IN & STOP) == STOP) {stop();}
      	shiftOut(1 << 3);
      	delay(delayTime); 
      	if ((P1IN & STOP) == STOP) {stop();}
      	shiftOut(1 << 2);
      	delay(delayTime); 
      	if ((P1IN & STOP) == STOP) {stop();}
      	shiftOut(1 << 1);
      	delay(delayTime); 
      	if ((P1IN & STOP) == STOP) {stop();}
	}
}

// Ensures that the STOP button is released after finishing a game
void stop (void) {
	for (;;) {
		if ((P1IN & STOP) == STOP) {			// Waits to STOP button to be pressed
			delay(50);
  			if ((P1IN & STOP) != STOP) {		// Waits for STOP buton to be released so it 
  				restart();				
  			}
		}
	}
}

// Gives the user the option to restart at the same speed with the STOP button
// or select a new speed with the START button
void restart (void) {
	for (;;) {
		if ((P1IN & START) == START) {			// Waits to START button to be pressed
			delay(50);
	  		if ((P1IN & START) != START) {		// Waits for START buton to be released so it 
	  			main();							// does not interfere with the main function
	  		}
		}
		if ((P1IN & STOP) == STOP) {			// Waits to STOP button to be pressed
			delay(50);
	  		if ((P1IN & STOP) != STOP) {		// Waits for STOP buton to be released so it 
	  			game();							// does not interfere with the game function
	  		}
		}
	}
}


// Delays by the specified Milliseconds
void delay(unsigned int ms) {
 	while (ms--) {
		__delay_cycles(1000);
    }
}
 
// Writes a value to the specified bitmask/pin. Use built in defines
// when calling this, as the shiftOut() function does.
// All nonzero values are treated as "high" and zero is "low"
void pinWrite(unsigned int bit, unsigned char val) {
  	if (val){
    	P1OUT |= bit;
  	} else {
    	P1OUT &= ~bit;
  	}
}
 
// Pulse the clock pin
void pulseClock(void){
  	P1OUT |= CLOCK;
  	P1OUT ^= CLOCK;
}
 
// Take the given 8-bit value and shift it out, LSB to MSB
void shiftOut(unsigned char val){
  	//Set latch to low (should be already)
  	P1OUT &= ~LATCH;
 
  	char i;
 
  	// Iterate over each bit, set data pin, and pulse the clock to send it to the shift register
  	for (i = 0; i < 8; i++)  {
      	pinWrite(DATA, (val & (1 << i)));
      	pulseClock();
  	}
 
  	// Pulse the latch pin to write the values into the storage register
  	P1OUT |= LATCH;
  	P1OUT &= ~LATCH;
}
