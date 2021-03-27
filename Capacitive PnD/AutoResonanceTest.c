//Code to tune the Capacitive Link to its Resonant State

#include <msp430.h>
#include <msp430f2132.h>

unsigned int adc_max = 0, adc1, adc2, adc3, adc;
unsigned int bcs, dco, bcs_nonres = 0x06, dco_sweep, dco_max=0x60, bcs_sweep, bcs_max=0x03;   
unsigned int d0, d1, d2, d3, delay1 = 35, delay2 = 140, delay_adc = 10000;
int flag = 0, i, j, k[8]; 
unsigned int br = 1200, br_int1 = 8, br_int2 = 0, br_frac = 14; // Initializing Baud Rate Parameters for UART
char data, calibrate_char = 'U';                                // Character to store Data received from Simblee

void byte_transfer(){
  BCSCTL1 = bcs_nonres;                            //Start Bit
  for(d2=0; d2<delay1; d2++);
  
  for(i=0; i<=7; i++){
    if(k[i] == 1){
      BCSCTL1 = bcs;                       //Resonant Frequency
      for(d2=0; d2<delay2; d2++);
    }
    else{
      BCSCTL1 = bcs_nonres;                       //Non-Resonant Frequency
      for(d2=0; d2<delay1; d2++);
    }
  }
  BCSCTL1 = bcs;                           // Stop Bit
  for(d2=0; d2<delay2; d2++);
}

void calibrate_intialize(){                   // Function to send 01010101 and calibrate internal chip's comparator threshold
  for(i=0; i<=7; i++)
    k[i] = (calibrate_char >> i) & 0x01;      // calibrate_char = 'U'
  for(d3=0; d3<delay2*50; d3++);
  for(j=0; j<20; j++){                        // Run 10 times
    for(d3=0; d3<delay2*100; d3++);
    byte_transfer();
  }
}

int main (void){
  WDTCTL = WDTPW + WDTHOLD;                      // Stop watchdog timer
  P1DIR |= 0x10;  P1SEL |= 0x10;                // SET THE PIN DIRECTION FOR SMCLK OUT (P1.4)
  ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE;   // ADC10ON and ADC samples at 16 x ADC10CLKs
  ADC10AE0 |= 0x01;                             // P2.0 ADC option select 
  P1DIR |= 0x0F;
  P1OUT = 0x00;
  
  //Portion of finding the maximum ADC input value and corresponding frequency
  
  for(bcs_sweep=8;bcs_sweep<=9;bcs_sweep++){    
    BCSCTL1 = bcs_sweep;                              // Intiatlize and update BCSCTL
    for(dco_sweep=32;dco_sweep<=160;dco_sweep=dco_sweep+32){      
      DCOCTL = dco_sweep;                             // Intiatlize and update DCOCTL
      for(d1=0; d1<delay_adc; d1++);           // Delay to stabilize voltage
      
      ADC10CTL0 |= ENC + ADC10SC;               // Start Conversion
      while (ADC10CTL1 & ADC10BUSY);            // Wait till conversion is complete
      adc1 = ADC10MEM;
      for(d1=0; d1<delay_adc; d1++);           // Delay to stabilize voltage
      
      ADC10CTL0 |= ENC + ADC10SC;               // Start Conversion
      while (ADC10CTL1 & ADC10BUSY);            // Wait till conversion is complete
      adc2 = ADC10MEM;
      for(d1=0; d1<delay_adc; d1++);           // Delay to stabilize voltage
      
      ADC10CTL0 |= ENC + ADC10SC;               // Start Conversion
      while (ADC10CTL1 & ADC10BUSY);            // Wait till conversion is complete
      adc3 = ADC10MEM;

     adc = adc1 + adc2 + adc3;
      if(adc > adc_max){                   // Update segment if peak is found
        adc_max = adc; 
        bcs_max = bcs_sweep;
        dco_max = dco_sweep;   
        P1OUT ^= 0x02; 
      }            
    }
  delay_adc = delay_adc*3; delay_adc = delay_adc/2;  
  }
  P1OUT ^= 0x04;
  for(d1=0; d1<delay_adc; d1++);       // Delay
               
  bcs = bcs_max; dco = dco_max; bcs_nonres = bcs_max - 2;  
  BCSCTL1 = bcs; DCOCTL = dco;
  P1OUT ^= 0x01; 
  ////////////////////////////////////////////////
  
  P2DIR |= 0xFF;                            // UART Testing
  P2OUT = 0xF0;
  //P2SEL |= 0x01;                            // ACLK on P2.0  
  BCSCTL3 |= 0x20;                          // ACLK setup to VLO

  //Code to run UART on ACLK
  P3SEL |= 0x30;                            // Enable UART TXD and RXD Port
  UCA0CTL1 |= UCSSEL_1;                     // ACLK
  UCA0BR0 = br_int1;                        // 10.55 kHz Baud Rate: 1200
  UCA0BR1 = br_int2;                        // 10.55 kHz Baud Rate: 1200
  UCA0MCTL = br_frac;                       // Modulation UCBRSx = 7
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

  //__bis_SR_register(GIE);                   //Interrupts enabled
  
  P1OUT ^= 0x01;
 
 for(;;){
    if(flag==1){
      P2OUT ^= 0xFF;
      for(i=0; i<=7; i++)
        k[i] = (data >> i) & 0x01;      
      //byte_transfer();      
      if (data == '3')
        //calibrate_intialize();
        flag = 0;
    }
  }
 
}


/*
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{ 
  while (!(IFG2&UCA0TXIFG));             // USCI_A0 TX buffer ready?
  UCA0TXBUF = UCA0RXBUF;                 // TX -> RXed character
  data = UCA0RXBUF;                      // Shift Character received to Data
  flag = 1;                              // Increment Flag                          
}
*/