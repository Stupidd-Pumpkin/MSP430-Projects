// Final code for startup and frequency sweep.
// (1) Setup the variables -> Initiate RTC & enable its interrupt -> Go into LPM3.5
// (2) Interrupt Service Routine: if (mode == 1) enter SU_Loop; else enter FS_Loop
// (3) SU_Loop: perform power monitoring including hysteresis and enter FS_Setup if DVCC is sufficiently high
// (4) FS_Loop: perform freqency sweep and call SU_Loop in every step.
// Please bare with the redundancies in the code, for they have been incorporated to fit with the processing limitations of msp430fr2476 (sometimes ineffectively)
// Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430fr2476.h' to understand registers and their functions

#include <msp430fr2476.h>
#include <stdio.h>

unsigned int Mode, Steps, f_Max, ADC_Max, ADC_Out, Count, q;
void initGpio (void); // GPIO initialization function
void SU_Setup (void); // Equivalent to main function. Used when DVCC drops below threshold
void SU_Loop (void);  // Power monitoring function with hysterisis included
void FS_Setup (void); // Frequency sweep setup function. Used first time and everytime DVCC drops below threshold.
void FS_Loop (void);  // Frequency sweep function
void Pulse (void);    // Pulse generation function (Currently not used)
void Tx (void);       // Dummy Function

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;             // Stop Watch Dog Timer
  initGpio();                           // Call the GPIO initialization function
 
  CSCTL4 |= SELA_1 | SELMS_3;           // MCLK = SMCLK = VLO; ACLK = REFO
  Steps = 64;                           // Number of steps during frequency sweep
  ADC_Max = 0;                          // To track the maximum value of the ADC output during frequency sweep
  Count=0;                              // The current running step of the frequency sweep
  Mode = 1;
  
  // Setup eCOMP for first comparision
  CPCTL0 &= ~(CPNSEL | CPPSEL);         // Clear the  CPNSEL and CPPSEL bits first                        
  CPCTL0 |= CPNSEL_2;                   // Select 1.2V internal reference as V- input, P1.1 as V+ input
  CPCTL0 |= CPPEN | CPNEN;              // Enable eCOMP input
  
  // Configure ADC
  ADCCTL0 |= ADCSHT_2 | ADCON;          // ADCON, Sample and Hold time = 16 ADC clks
  ADCCTL1 |= ADCSHP | ADCSSEL_1;        // ADCCLK = ACLK; sampling timer
  ADCCTL2 |= ADCSR;                     // ADC buffer supports upto 50kbps instead of 200kbps
  ADCCTL2 &= ~ADCRES;                   // Clear ADCRES in ADCCTL i.e ADC resolution set to 8-bit as default
  //ADCCTL2 |= ADCRES_2;                  // ADC resolution set to 12-bit
  ADCMCTL0 |= ADCINCH_3;                // Select A3 (P1.3) as ADC input; Vref=DVCC
  
   // Configure RTC
  SYSCFG2 |= RTCCKSEL;                    // Select ACLK as RTC clock

  RTCMOD = 31;  // Set the modulo for RTC source input; =31 for 1 sec
                // 1024/32768 * 32 = 1 sec; 32.768 kHz is the ACLK frequency
  
  // Initialize RTC
  RTCCTL = RTCSS_1 | RTCSR | RTCPS__1024 | RTCIE;
  
 
  // Enter LPM3.5 mode and wait till an RTC interrupt is triggered
  //PMMCTL0_H = PMMPW_H;                    // Open PMM Registers for write
  //PMMCTL0_L |= PMMREGOFF;                 // Set PMMREGOFF
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

void SU_Setup() 
{
  Mode = 1;
  
  // Configure RTC
  RTCMOD = 31;  // Set the modulo for RTC source input; =31 for 1 sec
                // 1024/32768 * 32 = 1 sec; 32.768 kHz is the ACLK frequency
  
  // Pulse generation
  // (1) Get out of Low Power Mode,
  // (2) Source the clocks with high frequency DCO,
  // (3) Send a Pulse on P5.1,
  // (4) Set back the clocks to VLO.
  // (5) Enter LPM3.5;
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
  __delay_cycles(10);
  P5OUT &= ~BIT1;
  
  CSCTL4 |= SELMS_3;                     // MCLK = SMCLK = VLO
  
  // Initialize RTC
  RTCCTL = RTCSS_1 | RTCSR | RTCPS__1024 | RTCIE;
  
  __bis_SR_register(LPM3_bits | GIE);
}

void SU_Loop()
{
  CPCTL1 |= CPEN | CPMSEL;                    // Turn on eCOMP, in low power mode
  __delay_cycles(2);
  if ((CPCTL1 & CPOUT) == 0){
    CPCTL1 &= ~CPEN;                          // Turn off eCOMP
    // Setup eCOMP for second comparision
    CPCTL0 &= ~(CPNSEL | CPPSEL);             // Clear the  CPNSEL and CPPSEL bits first                        
    CPCTL0 |= CPNSEL_2 | CPPSEL_3;            // Select internal Low-Power 1.2 V reference as V- input, P5.7 as V+ input
    CPCTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input
    CPCTL1 |= CPEN | CPMSEL;                  // Turn on eCOMP, in low power mode
    __delay_cycles(2);
    if ((CPCTL1 & CPOUT) == 1){
      RTCCTL &= ~RTCSS;
      P1OUT |= BIT2;
      FS_Setup();
    }
  }
  CPCTL1 &= ~CPEN;                  // Turn off eCOMP
}

void FS_Setup()
{
  Mode = 2;

  // Configure RTC
  // 1024/32768 * 32 = 1 sec.
  RTCMOD = 63; // =31 for 1 sec
  
  // Initialize RTC
  RTCCTL = RTCSS_1 | RTCSR | RTCPS__1024 | RTCIE; 
  
  // Pulse generation
  // (1) Get out of Low Power Mode,
  // (2) Source the clocks with high frequency DCO,
  // (3) Send a Pulse on P5.1,
  // (4) Set back the clocks to VLO.
  // (5) Enter LPM3.5;
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
  
  P5OUT |= BIT3;
  P5OUT &= ~BIT3;
  
  CSCTL4 |= SELMS_3;                     // MCLK = SMCLK = VLO
  __bis_SR_register(LPM3_bits | GIE);
}

void FS_Loop()
{
  
  RTCCTL &= ~RTCSS;
  // Setup eCOMP for first comparision
  CPCTL0 &= ~(CPNSEL | CPPSEL);                       //Clear the  CPNSEL and CPPSEL bits first                        
  CPCTL0 |= CPNSEL_2 | CPPSEL_4;                         // Select 1.2V internal reference as V- input, P6.0 as V+ input
  CPCTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input

  CPCTL1 |= CPEN | CPMSEL;                  // Turn on eCOMP, in low power mode
  __delay_cycles(2);

  if ((CPCTL1 & CPOUT) == 1){
    CPCTL1 &= ~CPEN;                  // Turn off eCOMP
    Count++; 
      
  	// Setup eCOMP for first comparision
  	CPCTL0 &= ~(CPNSEL | CPPSEL);                       //Clear the  CPNSEL and CPPSEL bits first                        
  	CPCTL0 |= CPNSEL_2;                         // Select 1.2V internal reference as V- input, P1.1 as V+ input
  	CPCTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input
  
  	CPCTL1 |= CPEN | CPMSEL;                  // Turn on eCOMP, in low power mode
  	while(CPCTL1 & CPOUT);
  	CPCTL1 &= ~CPEN;                  // Turn off eCOMP
  	__delay_cycles(5);
  
  	// 3 point averaged ADC sampling
  	__delay_cycles(6000);
  	ADCCTL0 |= ADCENC | ADCSC;                                        // Sampling and conversion start
  	while(ADCCTL1 & ADCBUSY);
  	ADC_Out = ADCMEM0;   
  	__delay_cycles(6000);

  	ADCCTL0 |= ADCENC | ADCSC;                                        // Sampling and conversion start
  	while(ADCCTL1 & ADCBUSY);
  	ADC_Out = ADC_Out + ADCMEM0; 
  	__delay_cycles(6000);

  	ADCCTL0 |= ADCENC | ADCSC;                                        // Sampling and conversion start
  	while(ADCCTL1 & ADCBUSY);
  	ADC_Out = ADC_Out + ADCMEM0;
    
  	CPCTL1 |= CPEN | CPMSEL;                  // Turn on eCOMP, in low power mode
  	while(CPCTL1 & CPOUT);
  	CPCTL1 &= ~CPEN;                  // Turn off eCOMP
  	__delay_cycles(5);

  	// Configure RTC
  	// 1024/32768 * 32 = 1 sec.
  	RTCMOD = 63; // =31 for 1 sec
  
  	if (ADC_Out > ADC_Max){
  		ADC_Max = ADC_Out;
  		f_Max = Count;     
  
  		// Pulse generation
  		// (1) Get out of Low Power Mode,
  		// (2) Source the clocks with high frequency DCO,
  		// (3) Send a Pulse on P5.1,
  		// (4) Set back the clocks to VLO.
  		// (5) Enter LPM3.5;
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
    }
   
  	if (count< 63){
  		// Initialize RTC
  		RTCCTL = RTCSS_1 | RTCSR | RTCPS__1024 | RTCIE; 
  		__bis_SR_register(LPM3_bits | GIE);
  	}
	
	__bis_SR_register(LPM3_bits | GIE);
}
  else {
    CPCTL1 &= ~CPEN;                  // Turn off eCOMP
    SU_Setup();
  }
}


void Pulse() // Currently not used
{
  // (1) Get out of Low Power Mode,
  // (2) Source the clocks with high frequency DCO,
  // (3) Send a Pulse on P5.1,
  // (4) Set back the clocks to VLO.
  // (5) Enter LPM3.5;
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

void Tx()
{
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
    P1OUT ^= BIT0;        // For debugging purposes
    if (Mode == 2){
      FS_Loop();  
    }
    else
      SU_Loop();
    break;
  default: break;
  }
}
