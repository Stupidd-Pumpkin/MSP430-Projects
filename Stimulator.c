#include <msp430.h>

volatile unsigned int i;                    // volatile to prevent optimization

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  P1DIR |= 0xFF;                            // Set P1.0 to output direction
  P1OUT  = 0x00;
  
  for (;;)
  {
    P1OUT |= 0x04;                          // Toggle P1.0 using exclusive-OR
    i = 100;                              // Delay
    do (i--); 
    while (i != 0);
    
    P1OUT &= ~0x04;
    P1OUT |= 0x08;                          // Toggle P1.0 using exclusive-OR
    i = 100;                              // Delay
    do (i--);
    while (i != 0);
    
    P1OUT &= ~0x08;
    i = 250;                              // Delay
    do (i--);
    while (i != 0);
    
  }
}
