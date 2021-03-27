/*
CAPACITIVE POWER AND DATA TRANSMISSION USING RESONANCE
FREQUENCY MODULATION TECHNIQUE FOR DATA (610 kHz) AND POWER (1 MHz)
*/
//******************************************************************************
//
//               MSP430F2013
//            -----------------
//        /|\|              XIN|-
//         | |                 | 
//         --|RST          XOUT|-
//           |                 |
//           |       P1.4/SMCLK|-------->Digital FM out
//           |                 | 
//           |         RXD/P1.1|<--------UART IN (Any baud; 8N1)
//


#include <msp430.h>
#include <msp430f2132.h>

unsigned char delay, temp_data;

int main (void)
{
  WDTCTL = WDTPW + WDTHOLD;                      // Stop watchdog timer
  BCSCTL1 = 0x09; DCOCTL = 0x60;               // Frequency Setup //2.28MHz
  //BCSCTL1 = 0x05; DCOCTL = 0x60;                 // Half Frequency
  P1DIR |= 0x10;  P1SEL |= 0x10;                // SET THE PIN DIRECTION FOR SMCLK OUT (P1.4)
  P3DIR &= 0x00;  P3SEL &= 0x00;                // TX RX making them input ports                              
  
  for(;;)
  {
    temp_data = P3IN & BIT5;
    if(temp_data == 0)
      {BCSCTL1 = 0x07;  for(delay=0;delay<1;delay++);} // 1.2 MHz
    else
      {BCSCTL1 = 0x09; for(delay=0;delay<2;delay++);} // 2.28 MHz    
  }
}
