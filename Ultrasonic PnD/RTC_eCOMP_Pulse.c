// Similar to 'Timer_ADC_Pulse.c', but uses real time clock instead of timer and enhanced comparator instead of ADC to drastically increase efficiency

#include <msp430fr2476.h>
#include <stdio.h>

void initGpio(void);
void eCOMP (void);
void pulse (void);

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                                // Stop WDT
  initGpio();
 
  CSCTL4 |= 0x0103;                     // MCLK=SMCLK=VLO; ACLK=VLO
  
  // Setup eCOMP
  CPCTL0 &= ~CPNSEL0;                       //Clear the  CPNSEL0 bit first                        
  CPCTL0 |= CPNSEL1 | CPNSEL2;              // Select DAC as input for V- terminal, external input  for V+ terminal
CPCTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input
  CPDACCTL |= CPDACEN;                      // Select VCC as reference and enable DAC
  CPDACDATA |= 0x0020;                      // CPDACBUF1=VCC *32/64
  
  // Configure RTC
  // 1024/32768 * 32 = 1 sec.
  RTCMOD = 31; // =31 for 1 sec
  
  // Initialize RTC
  SYSCFG2 |= RTCCKSEL;                    // Select ACLK as RTC clock
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
  
  // Select eCOMP input function on P1.1/C0
  P1SEL0 |= BIT1;
  P1SEL1 |= BIT1;
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


void eCOMP()
{
  
  CPCTL1 |= CPEN | CPMSEL;                  // Turn on eCOMP, in low power mode
  __delay_cycles(10);
  if ((CPCTL1 & 0x0001) == 1){
    P5OUT |= BIT0;
    pulse();
  }
  else {
    P5OUT &= ~BIT0;
  }
  CPCTL1 &= ~CPEN;                  // Turn off eCOMP
}

void pulse()
{
  __bic_SR_register(0x0080);            // LPM3 --> LPM0
  CSCTL4 |= 0x0003;                     // MCLK=SMCLK=DCO; ACLK=VLO
  
  CSCTL3 |= 0x0010;
  CSCTL1 |= 0x00FA;                             // 16 MHz
  CSCTL2 |= 0x00FF;
  //CSCTL2 &= ~0x7000;
  //__delay_cycles(3);
  __bic_SR_register(0x0040);                // enable FLL
  
  CSCTL0 |= 0x01FF;
  
  
  P5OUT |= BIT1;
  __delay_cycles(1);
  P5OUT &= ~BIT1;
  
  CSCTL5 |= 0x0100;                      // SMCLK off
  CSCTL4 &= ~0x0003;                     // MCLK=SMCLK=VLO; ACLK=VLO
  
  __bis_SR_register(0x00C0);
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
    eCOMP();
    break;
  default: break;
  }
}
