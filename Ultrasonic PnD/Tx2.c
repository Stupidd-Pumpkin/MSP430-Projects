// Use this code if you want to test only the data transmitting function of msp430fr2xxx
// Works same as Data_Tx_Test1.c', except now, data is transmitted repeatedly with RTC interrupt
// Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430.h' to understand registers and their functions

#include <msp430fr2476.h>
#include <stdio.h>

unsigned int Mode, Steps, f_Max, ADC_Max, ADC_Out, Count, q, i;
// Pseudo Random Data is taken
int Data[100] = {1,1,1,0,0,0,1,0,0,1,1,0,0,0,1,1,1,1,1,1,1,1,0,0,1,0,0,1,1,1,1,1,0,1,1,0,0,0,0,0,1,1,0,1,0,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,0,1,1,1,0,0,1,0,1,1,0,0,0,1,1,1,0,1,0,0,0,0,1,1,0,0,0,0,1,0,1}
void initGpio (void);

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;  // Stop Watch Dog Timer
  initGpio();                   // Call the GPIO initialization function
 
  CSCTL4 |= SELA_1 | SELMS_3;                     // MCLK=SMCLK=VLO; ACLK=REFO
  Steps = 64;
  ADC_Max = 0;
  Count=0;
  
  // Setup eCOMP for first comparision
  CPCTL0 &= ~(CPNSEL | CPPSEL);                       //Clear the  CPNSEL and CPPSEL bits first                        
  CPCTL0 |= CPNSEL_1;                         // Select P2.2 as V- input, P1.1 as V+ input
  CPCTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input
  
  // Configure ADC
  ADCCTL0 |= ADCSHT_2 | ADCON;                                       // ADCON, S&H=16 ADC clks
  ADCCTL1 |= ADCSHP | ADCSSEL_1;                                       // ADCCLK = ACLK; sampling timer
  ADCCTL2 |= ADCSR;                       // ADC buffer supports upto 50kbps instead of 200kbps
  ADCCTL2 &= ~ADCRES;                                      // clear ADCRES in ADCCTL i.e 8-bit 
  //ADCCTL2 |= ADCRES_2;                                     // 12-bit
  ADCMCTL0 |= ADCINCH_3;                                      // A3 ADC input select; Vref=DVCC
  
  // Initialize RTC
  SYSCFG2 |= RTCCKSEL;                    // Select ACLK as RTC clock
  
  Mode = 1;
  // Configure RTC
  RTCMOD = 31;  // set the modulo for RTC source input; =31 for 1 sec
                // 1024/32768 * 32 = 1 sec; 32.768 kHz is the ACLK frequency
  
  // Initialize RTC
  RTCCTL = RTCSS_1 | RTCSR | RTCPS__1024 | RTCIE;
  
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
  //P1SEL0 &= ~(BIT3 | BIT7); 
  //P1SEL1 |= BIT3 | BIT7;
  
  PM5CTL0 &= ~LOCKLPM5;
}

  
  // RTC interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(RTC_VECTOR))) RTC_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(RTCIV,RTCIV_RTCIF))
  {
  case  RTCIV_NONE:   break;          // No interrupt
  case  RTCIV_RTCIF:                  // RTC Overflow
    P1OUT ^= BIT0;
  // (1) Get out of Low Power Mode,
  // (2) source the clocks with high frequency DCO,
  // (3) Send data at this high speed,
  // (4) Set back the clocks to VLO.

  __bic_SR_register(0x0080);            // LPM3 --> LPM0
  CSCTL4 &= ~SELMS_3;                    //MCLK=SMCLK=DCO
  CSCTL3 |= SELREF_1;                    // REFOCLK is used as the FLL reference clock
  CSCTL1 |= DISMOD_1 | DCOFTRIM | DCORSEL_0 | DCOFTRIMEN_1;  
  // Modulation disabled,  frequency trimmed to highest value, DCOf = 16 MHz, Trimming enabled
  //CSCTL2 |= 0x00FF;
  //CSCTL2 &= ~0x7000;
  //__delay_cycles(3);
  __bic_SR_register(0x0040);                // enable FLL
  
  CSCTL0 |= 0x01FF;
  
  P5OUT |= 0x03;
  

  P5OUT &= ~0x03;
  
  __delay_cycles(6);
  
   P5OUT |= 0x03;

  
  __delay_cycles(6);
  
  
  __delay_cycles(6);
  
  P5OUT &= ~0x03;
  
  P5OUT |= 0x03;

  
  __delay_cycles(6);
  
  __delay_cycles(6);
  
  P5OUT &= ~0x03;

  __delay_cycles(6);
  
  __delay_cycles(6);

  for (i = 0; i <100; i++){
    if Data[i] == 0
      __delay_cycles(6);
    else 
      P5OUT ^= 0x03;      
  }
  CSCTL4 |= SELMS_3;                     // MCLK=SMCLK=VLO
  __bis_SR_register(LPM3_bits | GIE);
    break;
  default: break;
  }
}
