// Code to debug Clock speeds and continous ADC sampling.
// Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430fr2476.h' to understand registers and their functions

#include <msp430fr2476.h>
#include <stdio.h>

unsigned int Mode, q, i, Rx_Data[2000];
void initGpio (void);

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;     // Stop Watch Dog Timer
  initGpio();                   // Call the GPIO initialization function

  //CSCTL4 |= SELA_1 | SELMS_3;                     // MCLK = SMCLK = VLO; ACLK = REFO

  __bic_SR_register(0x0080);            // LPM3 --> LPM0
  CSCTL4 &= ~SELMS_3;                   //MCLK = SMCLK = DCO
  CSCTL3 |= SELREF_1;
  CSCTL1 |= DISMOD_1 | DCOFTRIM | DCORSEL_5 | DCOFTRIMEN_1;  
  // Modulation disabled,  frequency trimmed to highest value, DCOf = 16 MHz, Trimming enabled
  //CSCTL2 |= 0x00FF;
  //CSCTL2 &= ~0x7000;
  //__delay_cycles(3);
  __bic_SR_register(0x0040);                // enable FLL
  
  CSCTL0 |= 0x01FF;
  // CSCTL4 |= SELMS_3;                     // MCLK = SMCLK = VLO
   printf("1 \n");
  // Configure ADC
  ADCCTL0 |= ADCSHT_2 | ADCON;              // ADCON, S&H=16 ADC clks
  ADCCTL1 |= ADCSHP | ADCSSEL_2;            // ADCCLK = SMCLK; sampling timer
  //ADCCTL2 |= ADCSR;                         // ADC buffer supports upto 50kbps instead of 200kbps
  ADCCTL2 &= ~ADCRES;                       // clear ADCRES in ADCCTL i.e 8-bit 
  //ADCCTL2 |= ADCRES_2;                      // 12-bit
  ADCMCTL0 |= ADCINCH_3;                    // A3 ADC input select; Vref=DVCC
  //ADCIE |= ADCIE0;

  
  // Setup eCOMP for first comparision
  CPCTL0 &= ~(CPNSEL | CPPSEL);                       //Clear the  CPNSEL and CPPSEL bits first                        
  CPCTL0 |= CPNSEL_2;                         // Select 1.2V internal reference as V- input, P1.1 as V+ input
  CPCTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input
  CPCTL1 |= CPEN | CPMSEL;                  // Turn on eCOMP, in low power mode
  while(~CPCTL1 & CPOUT);
  CPCTL1 &= ~CPEN;                  // Turn off eCOMP
 //   printf("2 \n");
 //  __delay_cycles(1000);
 
  ADCCTL0 |= ADCENC | ADCSC;                                        // Sampling and conversion start
//   printf("3 \n");
for (i=0; i<2000; i++){
  while(ADCCTL1 & ADCBUSY);
   Rx_Data[i] = ADCMEM0;
    //printf("4 \n");
   // printf("%d \n",Rx_Data[i]);
    ADCCTL0 |= ADCENC | ADCSC;                                        // Sampling and conversion start
}

i=0;
while(ADCCTL1 & ADCBUSY);
//ADCCTL0 &= ~ADCENC;        
for (i=0; i<2000; i++){
  printf("%d \n",Rx_Data[i]);
}
  
   //Enter LPM3.5
  //PMMCTL0_H = PMMPW_H;                    // Open PMM Registers for write
  //PMMCTL0_L |= PMMREGOFF;                 // and set PMMREGOFF
  __bis_SR_register(LPM3_bits | GIE);                  // LPM3 and enable interrupts
}

void initGpio()
{
  // Configure GPIO
  P1DIR = 0xFF; 
  P2DIR = 0xFF;
  P3DIR = 0xFF;
  P4DIR = 0xFF;
  P5DIR = 0xFF;
  P6DIR = 0xFF;
  
  P1OUT = 0x00;
  P2OUT = 0x00;
  P3OUT = 0x00;
  P4OUT = 0x00;
  P5OUT = 0x00;
  P6OUT = 0x00;
  
  // Select eCOMP input function on P1.1/C0.0  
  P1SEL0 |= BIT1;
  P1SEL1 |= BIT1;
  // Select eCOMP input function on P2.2/C0.1
  P2SEL0 |= BIT2;
  P2SEL1 |= BIT2;
  
  P3SEL0 &= ~BIT4;
  P3SEL1 |= BIT4;
  // Select eCOMP input function on P5.7/C0.2
  P5SEL0 |= BIT7;
  P5SEL1 |= BIT7;
  // Select eCOMP input function on P6.0/C0.3
  P6SEL0 |= BIT0;
  P6SEL1 |= BIT0;
  // Select ADC input function on P1.3/A3
  P1SEL0 |= BIT3;
  P1SEL1 |= BIT3;
  // Output MCLK and SMCLK on P1.3 and P1.7
  P1SEL0 &= ~( BIT7); 
  P1SEL1 |=  BIT7;
  
  PM5CTL0 &= ~LOCKLPM5;
}

// ADC interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC_VECTOR))) ADC_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
    {
        case ADCIV_NONE:
            break;
        case ADCIV_ADCOVIFG:
            break;
        case ADCIV_ADCTOVIFG:
            break;
        case ADCIV_ADCHIIFG:
            break;
        case ADCIV_ADCLOIFG:
            break;
        case ADCIV_ADCINIFG:
            break;
        case ADCIV_ADCIFG:
            Rx_Data[i] = ADCMEM0;
            i=i+1;      
            ADCCTL0 |= ADCENC | ADCSC;   
            printf("4 \n");
            if (i == 1000){
            printf("5 \n");
            ADCCTL0 &= ~ADCENC;
            }
            break;
        default:
            break;
    }
}

