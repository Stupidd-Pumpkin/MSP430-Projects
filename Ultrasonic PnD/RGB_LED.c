// GPIO Test code. RGB LEDs on the evaluation board of msp430fr2xxx will blink in rotation
// Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430.h' to understand registers and their functions
#include <msp430.h>

volatile unsigned int i;                    // volatile to prevent optimization

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  P5DIR |= 0x03;                            
  P4DIR |= 0x80;                            
  PM5CTL0 &= ~LOCKLPM5;
  P5OUT = 0x00;
  P4OUT = 0x00;
  for (;;)
  {
    P5OUT ^= 0x02;                          
    i = 50000;                              // Delay
    do (i--);
    while (i != 0);
    P5OUT ^= 0x02;
    P4OUT ^= 0x80;            
    i = 50000;                              // Delay
    do (i--);
    while (i != 0);
    P4OUT ^= 0x80;
    P5OUT ^= 0x01;    
    i = 50000;                              // Delay
    do (i--);
    while (i != 0);
    P5OUT ^= 0x01;
  }
}