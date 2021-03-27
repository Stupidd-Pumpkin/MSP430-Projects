// Preliminary code for startup and frequency sweep (no hysteresis and no averaged adc sampling)
  // (1) Compare 1.2V internal reference as V- input with P1.1 as V+ input
  // (2) If comparator output is 1: Turn off RTC and enter Frequency Sweep Mode; else: repeat
  // (3) In frequency mode: in every step convert the incoming signal and compare with previous values to obtain the f_max, and return to (1) after every step
// Mode 1 implies startup mode and mode 2 corresponds to frequency sweep
// Please bare with the redundancies in the code, for they have been incorporated to fit with the processing limitations of msp430fr2476 (sometimes ineffectively)
// Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430fr2476.h' to understand registers and their functions

#include <msp430fr2476.h>
#include <stdio.h>

unsigned int Mode, Steps, f_Max, ADC_Max, Count, q;
void initGpio (void);
void eCOMP (void);
void Pulse (void);
void Freq_Sweep (void);
void ADC (void);
void Tx (void);         // Dummy function just to debug
void Wait (void);

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                       // Stop Watch Dog Timer
  initGpio();                                     // Call the GPIO initialization function
  
  CSCTL4 |= SELA_1 | SELMS_3;                     // MCLK = SMCLK = VLO; ACLK = REFO
  Steps = 64;         // Number of steps during frequency sweep
  ADC_Max = 0;        // To track the maximum value of the ADC output during frequency sweep
  Count=0;            // The current running step of the frequency sweep
  
  // Setup eCOMP
  CPCTL0 &= ~CPNSEL;                              // Clear the  CPNSEL bits first                        
  CPCTL0 |= CPNSEL1;                              // Select internal 1.2V for V- terminal, P1.1 for V+ terminal
  CPCTL0 |= CPPEN | CPNEN;                        // Enable eCOMP input
 // CPDACCTL |= CPDACEN | CPDACREFS;              // Select on-chip VREF and enable DAC
  
  // Configure ADC
  ADCCTL0 |= 0x0210;                              // ADCON, Sample and Hold time = 16 ADC clks
  ADCCTL1 |= 0x0208;                              // ADCCLK = ACLK; sampling timer
  ADCCTL2 &= ~0x0030;                             // clear ADCRES in ADCCTL i.e ADC resolution set to 8-bit as default
  //ADCCTL2 |= 0x0020;                              // ADC resolution set to 12-bit
  ADCMCTL0 |= 0x0003;                             // Select A3 (P1.3) as ADC input; Vref=DVCC
  
    Mode = 1;
  //CPDACDATA |= 0x002D;                     // CPDACBUF1 = 1.5*45/64 =1.05V

  // Configure RTC
  SYSCFG2 |= RTCCKSEL;                    // Select ACLK as RTC clock

  RTCMOD = 31;  // Set the modulo for RTC source input; =31 for 1 sec
                // 1024/32768 * 32 = 1 sec; 32.768 kHz is the ACLK frequency
  
  // Initialize RTC
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
  
  // Select eCOMP input function on P1.1/C0.0  
  P1SEL0 |= BIT1;
  P1SEL1 |= BIT1;
  // Select ADC input function on P1.3/A3
  P1SEL0 |= BIT3;
  P1SEL1 |= BIT3;
  // Output MCLK and SMCLK on P1.3 and P1.7 (For debugging purposes)
  //P1SEL0 &= ~(BIT3 | BIT7); 
  //P1SEL1 |= BIT3 | BIT7;
  
  PM5CTL0 &= ~LOCKLPM5;
}

void Wait()
{ 
  // If the energy stored in the node goes below the threshold, go back to mode 1 and send an acknoledgement

  Mode = 1;
  //CPDACDATA |= 0x002D;                     // CPDACBUF1 = 1.5*45/64 =1.05V
  
  // Configure RTC
  RTCMOD = 31;  // Set the modulo for RTC source input; =31 for 1 sec
                // 1024/32768 * 32 = 1 sec; 32.768 kHz is the ACLK frequency
 
  // Initialize RTC
  RTCCTL = RTCSS_1 | RTCSR | RTCPS__1024 | RTCIE;
  
  //Enter LPM3.5
  //PMMCTL0_H = PMMPW_H;                    // Open PMM Registers for write
  //PMMCTL0_L |= PMMREGOFF;                 // and set PMMREGOFF
  __bis_SR_register(LPM3_bits | GIE);                  // LPM3 and enable interrupts
  
  __bic_SR_register(0x0080);            // LPM3 --> LPM0
  CSCTL4 &= ~SELMS_3;                    //MCLK = SMCLK = DCO
  CSCTL3 |= SELREF_1;
  CSCTL1 |= DISMOD_1 | DCOFTRIM | DCORSEL_5 | DCOFTRIMEN_1;  
  // Modulation disabled,  frequency trimmed to highest value, DCOf = 16 MHz, Trimming enabled
  //CSCTL2 |= 0x00FF;
  //CSCTL2 &= ~0x7000;
  //__delay_cycles(3);
  __bic_SR_register(0x0040);                // enable FLL
  
  CSCTL0 |= 0x01FF;
  
  P5OUT |= BIT1;
  P5OUT &= ~BIT1;
  
  CSCTL4 |= SELMS_3;                     // MCLK = SMCLK = VLO
  __bis_SR_register(LPM3_bits | GIE);
}

void eCOMP()
{
  // (1) Compare 1.2V internal reference as V- input with P1.1 as V+ input
  // (2) If comparator output is 1: Turn off RTC and enter Frequency Sweep Mode
  // (4)    Upon Exit from Data_Rx(): Call Data_Tx()
  // (5) Else: stay in LPM3.5 and follow RTC ISR.
  
  CPCTL1 |= CPEN | CPMSEL;                  // Turn on eCOMP, in low power mode
  if ((CPCTL1 & CPOUT) == 1){
    RTCCTL &= ~RTCSS;
    P1OUT |= BIT2;
    Freq_Sweep();
  }
  CPCTL1 &= ~CPEN;                  // Turn off eCOMP
}

void Freq_Sweep()
{
  // Enables Frequency Sweep and acknowledges the main hub with a pulse.

  Mode = 2;
  //CPDACDATA |= 0x0028;                     // CPDACBUF1 = 1.5*40/64 = 0.95V  
  
  // Configure RTC
  RTCMOD = 63;  // set the modulo for RTC source input; =31 for 1 sec
                // 1024/32768 * 32 = 1 sec; 32.768 kHz is the ACLK frequency
 
  // Initialize RTC
  RTCCTL = RTCSS_1 | RTCSR | RTCPS__1024 | RTCIE; 
  
  // (1) Get out of Low Power Mode,
  // (2) Source the clocks with high frequency DCO,
  // (3) Send a Pulse on P5.1,
  // (4) Set back the clocks to VLO.
  // (5) Enter LPM3.5;
  __bic_SR_register(0x0080);            // LPM3 --> LPM0
  CSCTL4 &= ~SELMS_3;                    // MCLK = SMCLK = DCO
  CSCTL3 |= SELREF_1;
  CSCTL1 |= DISMOD_1 | DCOFTRIM | DCORSEL_5 | DCOFTRIMEN_1;  
  // Modulation disabled,  frequency trimmed to highest value, DCOf = 16 MHz, Trimming enabled
  //CSCTL2 |= 0x00FF;
  //CSCTL2 &= ~0x7000;
  //__delay_cycles(3);
  __bic_SR_register(0x0040);                // enable FLL
  
  CSCTL0 |= 0x01FF;
  
  P5OUT |= BIT1;
  P5OUT &= ~BIT1;
  
  CSCTL4 |= SELMS_3;                     // MCLK=SMCLK=VLO
  __bis_SR_register(LPM3_bits | GIE);
}

void Pulse()
{
  __bic_SR_register(0x0080);          // LPM3 --> LPM0
  CSCTL4 &= ~SELMS_3;                 // MCLK = SMCLK = DCO
  CSCTL3 |= SELREF_1;
  CSCTL1 |= DISMOD_1 | DCOFTRIM | DCORSEL_5 | DCOFTRIMEN_1;  
  // Modulation disabled,  frequency trimmed to highest value, DCOf = 16 MHz, Trimming enabled
  //CSCTL2 |= 0x00FF;
  //CSCTL2 &= ~0x7000;
  //__delay_cycles(3);
  __bic_SR_register(0x0040);          // enable FLL
  
  CSCTL0 |= 0x01FF;
  
  P5OUT |= BIT1;
  P5OUT &= ~BIT1;
  
  CSCTL4 |= SELMS_3;                  // MCLK = SMCLK = VLO
  __bis_SR_register(LPM3_bits | GIE);
}

void ADC(){
  
  ADCCTL0 |= 0x0003;                  // Sampling and conversion start
  
  while(ADCCTL1 & ADCBUSY);
  if (ADCMEM0 > ADC_Max){             // Store only the max value and it's corresponding frequency
    ADC_Max = ADCMEM0;
    f_Max = Count;
  }
}

void Tx(){
  Mode = 3;
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
    P1OUT ^= BIT0;                    // For debugging purposes
    if (Mode == 2)                    // Mode = 2 implies frequency sweep is initiated                 
    {
      Count++;
      ADC();                          // Perform ADC on the incoming signal
      if (Count == Steps - 1){        // Once count = 63, stop the sweep and enter transmission mode
        RTCCTL &= ~RTCSS;             // Stop Real Time clock  
        Pulse();                      // Send an acknowledgement pulse to confirm all the 64 steps were processed
        Tx();                         // Dummy function
      }
      CPCTL1 |= CPEN | CPMSEL;        // Turn on eCOMP, in low power mode
      __delay_cycles(10);
      if ((CPCTL1 & CPOUT) == 0){     // After every step, check if the storage capacitor has enough energy stored
        RTCCTL &= ~RTCSS;
        P1OUT &= ~BIT2;
        Wait();                       
      }
      CPCTL1 &= ~CPEN;                  // Turn off eCOMP
    }
    else
      eCOMP();
    break;
  default: break;
  }
}
