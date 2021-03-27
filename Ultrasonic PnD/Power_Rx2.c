// Use this code if you want to test Startup and Data Receiving functions.
// Works same as Data_Test1.c', except now,  the data received is oversampled and averaged to reduce error.
// Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430fr2476.h' to understand registers and their functions

#include <msp430fr2476.h>
#include <stdio.h>

unsigned int Mode, q, i, j, Samples, SPB, Bits, Count, ID, Config, Data_In[1000], Data[100], Rx_Data[1000];
void initGpio (void); // GPIO initialization function
//void SU_Setup (void); // Setup function during startup
//void FS_Setup (void); // Frequency Sweep Setup function
//void Data_Setup (void);
//void Rx (void);
//void Tx (void);
void Data_Rx (void);
void Data_Tx (void);
//void Mode_Select (unsigned int Config);
int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;     // Stop Watch Dog Timer
  initGpio();                   // Call the GPIO initialization function
 
  CSCTL4 |= SELA_1 | SELMS_3;   // MCLK = SMCLK = VLO; ACLK = REFO
  Mode = 7;
  
  Samples = sizeof(Data_In) / sizeof(Data_In[0]); //Total number of Samples 
  SPB = 10;     //Samples per bit
  Bits = Samples/SPB; // Number of bits
  ID = 11;
  Config = 7;
  
  // Setup eCOMP for first comparision
 // CPCTL0 &= ~(CPNSEL | CPPSEL);               //Clear the  CPNSEL and CPPSEL bits first                        
 // CPCTL0 |= CPNSEL_2;                        // Select 1.2V internal reference as V- input, P1.1 as V+ input
 // CPCTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input
  
  // Initialize RTC
  SYSCFG2 |= RTCCKSEL;                    // Select ACLK as RTC clock
  
  // Configure RTC
  RTCMOD = 63;  // set the modulo for RTC source input; =31 for 1 sec
                // 1024/32768 * 32 = 1 sec; 32.768 kHz is the ACLK frequency
  
  // Initialize RTC
  RTCCTL = RTCSS_1 | RTCSR | RTCPS__1024 | RTCIE;
  
  //Enter LPM3.5 mode and wait till an RTC interrupt is triggered
  //PMMCTL0_H = PMMPW_H;                    // Open PMM Registers for write
  //PMMCTL0_L |= PMMREGOFF;                 // and set PMMREGOFF
  __bis_SR_register(LPM3_bits | GIE);       // LPM3 and enable interrupts
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
  //P1SEL0 &= ~(BIT3 | BIT7); 
  //P1SEL1 |= BIT3 | BIT7;
  
  PM5CTL0 &= ~LOCKLPM5;
}

void Data_Rx()
{
  // (1) Compare 1.2V internal reference as V- input with P1.1 as V+ input
  // (2) If comparator output is 1: Turn off RTC and wait until comparator output has a raising edge 
  // (3)    To record data compare P2.2 as V- input, P1.1 as V+ input repeatedly (Input signal compared with it's envelope)
  // (5)    Data is oversampled and averaged to reduce error
  // (6)    Upon Exit from Data_Rx(): Call Data_Tx() {Wait until received signal (P1.1) has a falling edge and transmit data}
  // (7) Else: stay in LPM3.5 and follow RTC ISR.

  Mode = 2;
  // Setup eCOMP for first comparision
  CPCTL0 &= ~(CPNSEL | CPPSEL);             //Clear the  CPNSEL and CPPSEL bits first                        
  CPCTL0 |= CPNSEL_2;                       // Select 1.2V internal reference as V- input, P1.1 as V+ input
  CPCTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input
  CPCTL1 |= CPEN | CPMSEL;                  // Turn on eCOMP, in low power mode
  if (CPCTL1 && CPOUT == 1){
    RTCCTL &= ~RTCSS;
    while(CPCTL1 && CPOUT);
    while(~CPCTL1 && CPOUT);
    CPCTL1 &= ~CPEN;                        // Turn off eCOMP
  
  // Setup eCOMP for incomping data comparision
  CPCTL0 &= ~(CPNSEL | CPPSEL);             //Clear the  CPNSEL and CPPSEL bits first                        
  CPCTL0 |= CPNSEL_2;                       // Select P2.2 as V- input, P1.1 as V+ input
  CPCTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input
  CPCTL1 |= CPEN ;                          // Turn on eCOMP, in low power mode

  for (i=0;i<Samples;i++){
    //__delay_cycles(10);
    //P1OUT ^= BIT0;
    if (CPOUT == 0){
      Data_In[i]=0;
      P5OUT = CPOUT;
    }
    else {
      Data_In[i]=1;
      P5OUT = CPOUT;
  }
 }
  CPCTL1 &= ~CPEN;                  // Turn off eCOMP
  for (i=0; i<Bits;i++){
    Count = 0;
    for (j=0; j<SPB; j++){
    if (Data_In[i*SPB+j] == 1)
      Count++;
    }
    if (Count >= SPB/2)
      Data[i] = 1;
  }
   if (Data[0] + Data[1] + Data[2] + Data[3] == 11)
     Config = Data[4] + Data[5] + Data[6];
    //Mode_Select(Config);
   }
 CPCTL1 &= ~CPEN;                  // Turn off eCOMP
}

void Data_Tx()
{
  Mode = 2;
  if (Data[0] + Data[1] + Data[2] + Data[3] == 11) {

    
// Setup eCOMP for first comparision
//  CPCTL0 &= ~(CPNSEL | CPPSEL);                       //Clear the  CPNSEL and CPPSEL bits first                        
//  CPCTL0 |= CPNSEL_2;                         // Select 1.2V internal reference as V- input, P1.1 as V+ input
//  CPCTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input
//  CPCTL1 |= CPEN | CPMSEL;                  // Turn on eCOMP, in low power mode
//  while(CPCTL1 & CPOUT);
// CPCTL1 &= ~CPEN;                  // Turn off eCOMP
  
  for (i=0;i<100;i++){
    __delay_cycles(10);
    P1OUT ^= BIT0;
    if (Data[i] == 0)
      P5OUT & = ~BIT1;
    else
      P5OUT |= BIT1;
  }
 }
}

// void SU_Setup ()
// {
//   Mode = 7;
// }

// void FS_Setup ()
// {
//   Mode = 1;
// }

// void Data_Setup ()
// {
//   Mode = 2;
// }

// void Rx ()
// {
//   Mode = 3;
//     __delay_cycles(1000);
  
//    // Setup eCOMP for first comparision
//   CPCTL0 &= ~(CPNSEL | CPPSEL);                       //Clear the  CPNSEL and CPPSEL bits first                        
//   CPCTL0 |= CPNSEL_2;                         // Select 1.2V internal reference as V- input, P1.1 as V+ input
//   CPCTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input
//   CPCTL1 |= CPEN | CPMSEL;                  // Turn on eCOMP, in low power mode
//   while(~CPCTL1 && CPOUT);
//   CPCTL1 &= ~CPEN;                  // Turn off eCOMP
   
//    // Configure ADC
//   ADCCTL0 |= ADCSHT_2 | ADCON;                                       // ADCON, S&H=16 ADC clks
//   ADCCTL1 |= ADCSHP | ADCSSEL_1;                                       // ADCCLK = ACLK; sampling timer
//   ADCCTL2 |= ADCSR;                       // ADC buffer supports upto 50kbps instead of 200kbps
//   ADCCTL2 &= ~ADCRES;                                      // clear ADCRES in ADCCTL i.e 8-bit 
//   //ADCCTL2 |= ADCRES_2;                                     // 12-bit
//   ADCMCTL0 |= ADCINCH_3;                                      // A3 ADC input select; Vref=DVCC

//   for (i=0; i<1000; i++){
//     ADCCTL0 |= ADCENC | ADCSC;                                        // Sampling and conversion start
//     while(ADCCTL1 & ADCBUSY);
//     Rx_Data = ADCMEM0;
// }

// }

// void Tx ()
// {
//   Mode = 4;
//     __delay_cycles(1000);
  
//    // Setup eCOMP for first comparision
//   CPCTL0 &= ~(CPNSEL | CPPSEL);                       //Clear the  CPNSEL and CPPSEL bits first                        
//   CPCTL0 |= CPNSEL_2;                         // Select 1.2V internal reference as V- input, P1.1 as V+ input
//   CPCTL0 |= CPPEN | CPNEN;                  // Enable eCOMP input
//   CPCTL1 |= CPEN | CPMSEL;                  // Turn on eCOMP, in low power mode
//   while(~CPCTL1 && CPOUT);
//   CPCTL1 &= ~CPEN;                  // Turn off eCOMP
  
//     __delay_cycles(100);
  
//   // Pulse generation
//   __bic_SR_register(0x0080);            // LPM3 --> LPM0
//   CSCTL4 &= ~SELMS_3;                    //MCLK=SMCLK=DCO
//   CSCTL3 |= SELREF_1;
//   CSCTL1 |= DISMOD_1 | DCOFTRIM | DCORSEL_5 | DCOFTRIMEN_1;  
//   // Modulation disabled,  frequency trimmed to highest value, DCOf = 16 MHz, Trimming enabled
//   //CSCTL2 |= 0x00FF;
//   //CSCTL2 &= ~0x7000;
//   //__delay_cycles(3);
//   __bic_SR_register(0x0040);                // enable FLL
  
//   CSCTL0 |= 0x01FF;
  
//   for (i=0; i<5; i++){
//   P5OUT |= BIT1;
//   P5OUT &= ~BIT1;
//   }
  
//   CSCTL4 |= SELMS_3;                     // MCLK=SMCLK=VLO
//   __bis_SR_register(LPM3_bits | GIE);
// }

// void Mode_Select(unsigned int Config){
// switch(Config)
// {
// case 1: FS_Setup(); 
// case 2: Data_Setup();
// case 3: Rx();
// case 4: Tx();
// default: SU_Setup();
// }
// }


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
    if (P1OUT && BIT0)
      Data_Rx();
    else 
      Data_Tx();
    break;
  default: break;
  }
}
