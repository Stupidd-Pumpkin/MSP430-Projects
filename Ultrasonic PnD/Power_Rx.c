// Use this code if you want to test Startup and Data Receiving functions.
// (1) On Startup goes into LPM3.5 after enabling RTC interrupt
// (2) RTC ISR: Get out of Low Power Mode
// (3) Compare 1.2V internal reference as V- input with P1.1 as V+ input
// (4) If comparator output is 0: compare 1.2V internal reference as V- input with P5.7 as V+ input
// (5)  If comparator output is 1: Turn off RTC and call Data_Rx() {Wait until received signal (P1.1) has a raising edge and receives data}
// (6)    Upon Exit from Data_Rx(): Call Data_Tx() {Wait until received signal (P1.1) has a falling edge and transmit data}
// (7) Else: stay in LPM3.5 and follow RTC ISR.
// Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430fr2476.h' to understand registers and their functions

#include <msp430fr2476.h>
#include <stdio.h>

unsigned int Mode, Steps, f_Max, ADC_Max, ADC_Out, Count, q, i, Data[100];
void initGpio (void); // GPIO initialization function
void SU_Setup (void); // Setup function during startup; Not used! 
void SU_Loop (void);  // Loop function during startup to evaluate the necessary power levels and only then proceed to data mode. Mode = 1
void Data_Rx (void);  // Data Receive Function. Mode = 2
void Data_Tx (void);  // Data Transmission Function. Mode = 3

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;   // Stop Watch Dog Timer
  initGpio();                 // Call the GPIO initialization function
  
  CSCTL4 |= SELA_1 | SELMS_3; // MCLK = SMCLK = VLO; ACLK = REFO
  Steps = 64;
  ADC_Max = 0;
  Count=0;
  Mode = 1;
  
  // Setup eCOMP for first comparision
  CPCTL0 &= ~(CPNSEL | CPPSEL);// Clear the  CPNSEL and CPPSEL bits first                        
  CPCTL0 |= CPNSEL_2;          // Select 1.2V internal reference as V- input, P1.1 as V+ input
  CPCTL0 |= CPPEN | CPNEN;     // Enable eCOMP input
  
  // Configure ADC
  ADCCTL0 |= ADCSHT_2 | ADCON; // ADCON, Sample and Hold time = 16 ADC clks
  ADCCTL1 |= ADCSHP | ADCSSEL_1;// ADCCLK = ACLK; sampling timer
  ADCCTL2 |= ADCSR;             // ADC buffer supports upto 50kbps instead of 200kbps
  ADCCTL2 &= ~ADCRES;           // clear ADCRES in ADCCTL i.e ADC resolution set to 8-bit as default 
  //ADCCTL2 |= ADCRES_2;        // ADC resolution set to 12-bit
  ADCMCTL0 |= ADCINCH_3;        // A3 (P1.3) is selected as ADC input; reference voltage for ADC (Vref) is set as DVCC
  
  // Initialize RTC
  SYSCFG2 |= RTCCKSEL;                    // Select ACLK as the source for Real Time clock
  
  // Configure RTC
  RTCMOD = 31;  // set the modulo for RTC source input; =31 for 1 sec
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
  
  PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode
}

void SU_Setup() // Currently this function is not used. If an acknowledgement pulse is needed soon after turning on the MCU, call this function in main()
{
  // (1) Get out of Low Power Mode,
  // (2) source the clocks with high frequency DCO,
  // (3) Send a Pulse on P5.1,
  // (4) Set back the clocks to VLO,
  // (5) Start RTC and Enter LPM3.5;
  
 // Configure RTC
  RTCMOD = 31;  // set the modulo for RTC source input; =31 for 1 sec
                // 1024/32768 * 32 = 1 sec; 32.768 kHz is the ACLK frequency

  // Pulse generation for acknowledgement of start up
  __bic_SR_register(0x0080);            // LPM3 --> LPM0
  CSCTL4 &= ~SELMS_3;                   // MCLK = SMCLK = DCO
  CSCTL3 |= SELREF_1;
  CSCTL1 |= DISMOD_1 | DCOFTRIM | DCORSEL_5 | DCOFTRIMEN_1;  
  // Modulation disabled,  frequency trimmed to highest value, DCOf = 16 MHz, Trimming enabled
  //CSCTL2 |= 0x00FF;
  //CSCTL2 &= ~0x7000;
  //__delay_cycles(3);
  __bic_SR_register(0x0040);            // enable FLL
  
  CSCTL0 |= 0x01FF;
  
  P5OUT |= BIT1;
  __delay_cycles(10);
  P5OUT &= ~BIT1;                       // A short pulse is sent through P5.1
  
  CSCTL4 |= SELMS_3;                    // MCLK = SMCLK = VLO
  
  // Initialize RTC
  RTCCTL = RTCSS_1 | RTCSR | RTCPS__1024 | RTCIE;
  
  __bis_SR_register(LPM3_bits | GIE);
}

void SU_Loop()
{
  // (1) Compare 1.2V internal reference as V- input with P1.1 as V+ input
  // (2) If comparator output is 0: compare 1.2V internal reference as V- input with P5.7 as V+ input
  // (3)  If comparator output is 1: Turn off RTC and call Data_Rx()
  // (4)    Upon Exit from Data_Rx(): Call Data_Tx()
  // (5) Else: stay in LPM3.5 and follow RTC ISR.

  Mode = 1;
  CPCTL1 |= CPEN | CPMSEL;            // Turn on eCOMP, in low power mode
  __delay_cycles(2);
  if ((CPCTL1 & CPOUT) == 0)          // If Comparator Output is 0 proceed to next step
  {
    CPCTL1 &= ~CPEN;                  // Turn off eCOMP
    // Setup eCOMP for second comparision
    CPCTL0 &= ~(CPNSEL | CPPSEL);     // Clear the  CPNSEL and CPPSEL bits first                        
    CPCTL0 |= CPNSEL_2 | CPPSEL_3;    // Select internal Low-Power 1.2 V reference as V- input, P5.7 as V+ input
    CPCTL0 |= CPPEN | CPNEN;          // Enable eCOMP input
    CPCTL1 |= CPEN | CPMSEL;          // Turn on eCOMP, in low power mode
    __delay_cycles(2);
    if ((CPCTL1 & CPOUT) == 1){
      RTCCTL &= ~RTCSS;
      P1OUT |= BIT2;
      Data_Rx();
      Data_Tx();
    }
  }
  CPCTL1 &= ~CPEN;                  // Turn off eCOMP
  
}

void Data_Rx()
{
  // (1) Compare 1.2V internal reference as V- input with P1.1 as V+ input
  // (2) Wait until received signal (P1.1) has a raising edge
  // (3) Compare P2.2 as V- input, P1.1 as V+ input repeatedly for 100 times (Input signal compared with it's envelope)
  // (4) The 100 bits of Data read is saved in the array data[] and displayed on P5.1
 

  Mode = 2;
  // Setup eCOMP for first comparision again
  CPCTL0 &= ~(CPNSEL | CPPSEL);     // Clear the  CPNSEL and CPPSEL bits first                        
  CPCTL0 |= CPNSEL_2;               // Select 1.2V internal reference as V- input, P1.1 as V+ input
  CPCTL0 |= CPPEN | CPNEN;          // Enable eCOMP input
  CPCTL1 |= CPEN | CPMSEL;          // Turn on eCOMP, in low power mode
  while(CPCTL1 & ~CPOUT);           // Wait until received signal has a raising edge
  CPCTL1 &= ~CPEN;                  // Turn off eCOMP
  
  // Setup eCOMP for incomping data comparision
  CPCTL0 &= ~(CPNSEL | CPPSEL);     //Clear the  CPNSEL and CPPSEL bits first                        
  CPCTL0 |= CPNSEL_1;               // Select P2.2 as V- input, P1.1 as V+ input
  CPCTL0 |= CPPEN | CPNEN;          // Enable eCOMP input
  
  for (i=0;i<100;i++){
    __delay_cycles(10);
    P1OUT ^= BIT0;
    if (CPOUT == 0){
      Data[i]=0;
      P5OUT &= ~BIT1;               // The Data Read is displayed on P5.1
    }
    else {
      Data[i]=1;
      P5OUT |= BIT1;
  }
  CPCTL1 &= ~CPEN;                  // Turn off eCOMP
}
}

void Data_Tx()
{
  // (1) Compare 1.2V internal reference as V- input with P1.1 as V+ input
  // (2) Wait until received signal (P1.1) has a falling edge
  // (3) Output the saved data on P5.0
 
  Mode = 3;
  // Setup eCOMP for first comparision
  CPCTL0 &= ~(CPNSEL | CPPSEL);     // Clear the  CPNSEL and CPPSEL bits first                        
  CPCTL0 |= CPNSEL_2;               // Select 1.2V internal reference as V- input, P1.1 as V+ input
  CPCTL0 |= CPPEN | CPNEN;          // Enable eCOMP input
  CPCTL1 |= CPEN | CPMSEL;          // Turn on eCOMP, in low power mode
  while(CPCTL1 & CPOUT);
  CPCTL1 &= ~CPEN;                  // Turn off eCOMP
  
  for (i=0;i<100;i++){
    __delay_cycles(10);
    P1OUT ^= BIT0;                  // For debugging purposes toggles P1.0 everytime
    P5OUT = (0x01 & Data[i]);
  }
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
  case  RTCIV_NONE:   break;// No interrupt
  case  RTCIV_RTCIF:        // RTC Overflow
    P1OUT ^= BIT0;          // For debugging purposes toggles P1.0 everytime RTC overflows
    SU_Loop();              // Enter Start Up Loop Function
    break;
  default: break;
  }
}
