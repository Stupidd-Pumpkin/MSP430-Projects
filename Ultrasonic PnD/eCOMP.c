// Use this code if you want to test only the enhanced comparator (eCOMP) module of msp430fr2xxx
// Compare Vcc/2 as V- input with P1.1 as V+ input and set P5.0 equal to the comparator output
// Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430.h' to understand registers and their functions

#include <msp430.h>
int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT
  
  P5DIR |= 0x03;                            
  P4DIR |= 0x80;                            
  
  P1DIR |= 0x89;
  P1OUT &= ~0x01; 
  P5OUT &= ~0x03;
  P4OUT &= ~0x80;
  
  
  // Configure Comparator input & output
  P1SEL0 |= BIT1;                           // Select eCOMP input function on P1.1/C0
  P1SEL1 |= BIT1;
 
  PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings


  // Setup eCOMP
  CPCTL0 &= ~CPNSEL0;                       //Clear the  CPNSEL0 bit first                        
  CPCTL0 |= CPNSEL1 | CPNSEL2;              // Select DAC as input for V- terminal
  CPCTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input
  CPDACCTL |= CPDACEN;                      // Select VCC as reference and enable DAC
  CPDACDATA |= 0x0020;                      // CPDACBUF1=VCC *32/64
  CPCTL1 |= CPEN;                           // Turn on eCOMP, in low power mode

  while (1){
  
  
  if ((CPCTL1 & 0x0001) == 0){
          P5OUT &= ~0x01;
        }
  else {
          P5OUT |= 0x01;
        }
  }
 // __bis_SR_register(LPM3_bits);             // Enter LPM3
 // __no_operation();                         // For debug
}
