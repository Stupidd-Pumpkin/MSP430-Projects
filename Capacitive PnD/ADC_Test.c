#include <msp430.h>
#include <msp430f2132.h>

unsigned long int delay, k = 40000;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE; // ADC10ON, interrupt enabled
  ADC10AE0 |= 0x01;                         // P2.0 ADC option select

  P1DIR |= 0x11;                            // Set P1.0 to output direction
  P3DIR |= 0xFF;
  
  for (;;)
  {
    ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
    __bis_SR_register(GIE);                // LPM0, ADC10_ISR will force exit
    P3OUT = ADC10MEM;
    P1OUT ^= 0x10;
    for(delay=0; delay<k; delay++);
  }
}

// ADC10 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC10_VECTOR))) ADC10_ISR (void)
#else
#error Compiler not supported!
#endif
{
    P1OUT ^= 0x01;
}
