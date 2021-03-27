// Works similar to Power.c but needs an initial wake up with an external interrupt. Useful if fully external power monitoring system is used.
// Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430fr2476.h' to understand registers and their functions

#include <msp430.h>
#include <stdio.h>

void initGpio(void);
void ADC (void);
void Freq_Sweep (void);
void Ack (void);
void Tx_Fd (void);
void Rx_Fu (void);

unsigned int ADC_Out, ADC_Max, f_Max, Count, Steps, i, j, q;

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                                // Stop WDT
   
  if (SYSRSTIV == SYSRSTIV_LPM5WU)        // MSP430 just woke up from LPMx.5
    Freq_Sweep();
  else{    
      
  P1DIR &= ~(BIT2);                   // Configure P1.3 as input direction pin
  P1OUT &= ~BIT2;                      // Configure P1.3 as pulled-down
  P1REN |= BIT2;                      // P1.3 pull up/down register enable
  P1IES &= ~BIT2;                      // P1.3 Low-High edge
  P1IFG = 0;                          // Clear all P1 interrupt flags
  P1IE |= BIT2;                       // P1.3 interrupt enabled
  
  RTCCTL = 0;                                              // Disable RTC
  //Enter LPM4.5
  PMMCTL0_H = PMMPW_H;                    // Open PMM Registers for write
  PMMCTL0_L |= PMMREGOFF;                 // and set PMMREGOFF
  __bis_SR_register(LPM4_bits | GIE);                  // LPM3 and enable interrupts
}
}

void Freq_Sweep(){
  
  //Configure GPIO
  initGpio();
    
  Steps = 64;
  ADC_Max = 0;
  Count=0;
  
  // Select ADC input function on P1.1/A1
  P1SEL0 |= BIT3;
  P1SEL1 |= BIT3;
  
  CSCTL4 |= 0x0203;                     // MCLK=SMCLK=VLO; ACLK=VLO
  
  // Configure ADC
  ADCCTL0 |= 0x0210;                                       // ADCON, S&H=16 ADC clks
  ADCCTL1 |= 0x0208;                                       // ADCCLK = ACLK; sampling timer
  ADCCTL2 &= ~0x0030;                                      // clear ADCRES in ADCCTL i.e 8-bit 
  //ADCCTL2 |= 0x0020;                                     // 12-bit
  ADCMCTL0 |= 0x0003;                                      // A1 ADC input select; Vref=DVCC
  //ADCIE |= 0x0001;                                         // Enable ADC conv complete interrupt
  
  // Configure RTC
  RTCMOD = 63;  // Set the modulo for RTC source input; = 31 for 1 sec
                // 1024/32768 * 32 = 1 sec; 32.768 kHz is the ACLK frequency
  
  // Initialize RTC
  SYSCFG2 |= RTCCKSEL;                    // Select ACLK as RTC clock
  RTCCTL = RTCSS_1 | RTCSR | RTCPS__1024 | RTCIE;
  
  //Enter LPM3
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

void Ack()
{
 __bic_SR_register(SCG1);            // LPM3 --> LPM0
  CSCTL3 |= 0x0010;
  CSCTL1 |= 0x00FA;                             // 16 MHz
  CSCTL2 |= 0x00FF;
  //CSCTL2 &= ~0x7000;
  __delay_cycles(3);
  __bic_SR_register(SCG0);                // enable FLL
  
  CSCTL0 |= 0x01FF;

  P5OUT |= 0x02;
  P5OUT &= ~0x02;

  __bis_SR_register(SCG0 + SCG1);
}

void Tx_Fd()
{
  
}

void Rx_Fu()
{
  
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
    if (Count>63){
      RTCCTL &= ~RTCSS_3;
      Ack();
      Tx_Fd();
    }
    break;
  default: break;
  }
}
