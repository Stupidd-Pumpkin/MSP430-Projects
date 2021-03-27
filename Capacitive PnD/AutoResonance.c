//Code to tune the Capacitive Link to its Resonant State

#include <msp430.h>
#include <msp430f2132.h>

unsigned int adc_max = 0, dco, dco_max=0x60, bcs, bcs_max=0x03, adc1, adc2, adc3, adc;
unsigned long int delay, k = 1000;

int main (void)
{
  WDTCTL = WDTPW + WDTHOLD;                      // Stop watchdog timer
  P1DIR |= 0x10;  P1SEL |= 0x10;                // SET THE PIN DIRECTION FOR SMCLK OUT (P1.4)
  ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE;   // ADC10ON and ADC samples at 16 x ADC10CLKs
  ADC10AE0 |= 0x01;                             // P2.0 ADC option select 
  P1DIR |= 0x01;
  P1OUT = 0x00;
  
  //Portion of finding the maximum ADC input value and corresponding frequency
  
  for(bcs=6;bcs<=12;bcs++){
    BCSCTL1 = bcs;                              // Intiatlize and update BCSCTL
    for(dco=32;dco<=160;dco=dco+32){
      
      DCOCTL = dco;                             // Intiatlize and update DCOCTL
      for(delay=0; delay<k; delay++);           // Delay to stabilize voltage
      
      ADC10CTL0 |= ENC + ADC10SC;               // Start Conversion
      while (ADC10CTL1 & ADC10BUSY);            // Wait till conversion is complete
      adc1 = ADC10MEM;
      for(delay=0; delay<k; delay++);           // Delay to stabilize voltage
      
      ADC10CTL0 |= ENC + ADC10SC;               // Start Conversion
      while (ADC10CTL1 & ADC10BUSY);            // Wait till conversion is complete
      adc2 = ADC10MEM;
      for(delay=0; delay<k; delay++);           // Delay to stabilize voltage
      
      ADC10CTL0 |= ENC + ADC10SC;               // Start Conversion
      while (ADC10CTL1 & ADC10BUSY);            // Wait till conversion is complete
      adc3 = ADC10MEM;

     adc = adc1 + adc2 + adc3;
      if(adc > adc_max){                   // Update segment if peak is found
        adc_max = adc; 
        bcs_max = bcs;
        dco_max = dco;   
        P1OUT ^= 0x01; 
      }            
    }
  k = k*3; k = k/2;
  
  }
  for(delay=0; delay<k; delay++);                // Delay
  BCSCTL1=bcs_max ; DCOCTL=dco_max;             //Setting the Peak PDL Resonant Frequency
  P1OUT ^= 0x01; 
}