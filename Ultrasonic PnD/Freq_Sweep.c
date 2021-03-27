// Use this code if you want to Perform Frequency Sweep.
// (1) Initiate RTC interrupt and go to LPM3.5
// (2) Perform 8-bit ADC on P1.1 input signal 64 times and save the maximum of them all
// Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430fr2476.h' to understand registers and their functions

#include <msp430.h>
#include <stdio.h>

void initGpio(void);
void ADC (void);

unsigned int ADC_Out, ADC_Max, f_Max, Count, Steps, i, j, q;

int main(void)
{
 WDTCTL = WDTPW | WDTHOLD;     // Stop Watch Dog Timer
 initGpio();                   // Call the GPIO initialization function
 
  Steps = 64;   // Number of ADC steps
  ADC_Max = 0;  // Initialize ADC_Max to zero
  Count=0;
  
  CSCTL4 |= 0x0203;                     // MCLK = SMCLK = VLO; ACLK = VLO
  
  // Configure ADC
  ADCCTL0 |= 0x0210;                                       // ADCON, S&H=16 ADC clks
  ADCCTL1 |= 0x0208;                                       // ADCCLK = ACLK; sampling timer
  ADCCTL2 &= ~0x0030;                                      // clear ADCRES in ADCCTL i.e ADC resolution set to 8-bit as default
  //ADCCTL2 |= 0x0020;                                     // ADC resolution set to 12-bit
  ADCMCTL0 |= 0x0003;                                      // A1 ADC input select; Vref=DVCC
  //ADCIE |= 0x0001;                                       // Enable ADC conv complete interrupt
  
  // Configure RTC
  RTCMOD = 63;  // set the modulo for RTC source input; =31 for 1 sec
                // 1024/32768 * 32 = 1 sec; 32.768 kHz is the ACLK frequency

  // Initialize RTC
  SYSCFG2 |= RTCCKSEL;                    // Select ACLK as RTC clock
  RTCCTL = RTCSS_1 | RTCSR | RTCPS__1024 | RTCIE;
  
  // Enter LPM3.5 mode and wait till an RTC interrupt is triggered
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
  
  // Select ADC input function on P1.1/A1
  P1SEL0 |= BIT3;
  P1SEL1 |= BIT3;
  // Output MCLK and SMCLK on P1.3 and P1.7
  //P1SEL0 &= ~(BIT3 | BIT7); 
  //P1SEL1 |= BIT3 | BIT7;
  // Output ACLK on P2.2
  //P2SEL0 &= ~BIT2;
  //P2SEL1 |= BIT2;
  
  // Disable the GPIO power-on default high-impedance mode
  // to activate previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;
}

 
void ADC()
{
  ADCCTL0 |= 0x0003;                                        // Sampling and conversion start
  
  while(ADCCTL1 & ADCBUSY);
  if (ADCMEM0 < ADC_Max){
    P5OUT |= BIT1;
    __delay_cycles(500);
    P5OUT &= ~BIT1;
  }
  else{
    P5OUT |= BIT0;
    __delay_cycles(500);
    P5OUT &= ~BIT0;
    ADC_Max = ADCMEM0;
    f_Max = Count;
  }
  printf("%d   ; %d  \n", ADC_Max, f_Max);
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
    Count++;
    ADC();
    if (Count > 63){
      RTCCTL &= ~RTCSS_3;
    }
    break;
  default: break;
  }
}
