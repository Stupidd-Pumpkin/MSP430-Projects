// Use this code if you want to test only the data transmitting function of msp430fr2xxx
// Test to transmit saved data on P5.0 and P5.1
// Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430.h' to understand registers and their functions

#include <msp430fr2476.h>
#include <stdio.h>

unsigned int Mode, Steps, f_Max, ADC_Max, ADC_Out, Count, q, i;
// Pseudo Random Data is taken
int Data[100] = {1,1,1,0,0,0,1,0,0,1,1,0,0,0,1,1,1,1,1,1,1,1,0,0,1,0,0,1,1,1,1,1,0,1,1,0,0,0,0,0,1,1,0,1,0,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,0,1,1,1,0,0,1,0,1,1,0,0,0,1,1,1,0,1,0,0,0,0,1,1,0,0,0,0,1,0,1};
void initGpio (void);

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD; // Stop Watch Dog Timer
  initGpio();                   // Call the GPIO initialization function
  
  CSCTL4 |= SELA_1 | SELMS_3;                     // MCLK = SMCLK = VLO; ACLK = REFO
  Steps = 64;
  ADC_Max = 0;
  Count=0;
  
  P1OUT ^= BIT0;

  // (1) Get out of Low Power Mode,
  // (2) source the clocks with high frequency DCO,
  // (3) Send data at this high speed,
  // (4) Set back the clocks to VLO.

  __bic_SR_register(0x0080);            // LPM3 --> LPM0
  CSCTL4 &= ~SELMS_3;                   // MCLK = SMCLK = DCO
  CSCTL3 |= SELREF_1;                   // REFOCLK is used as the FLL reference clock
  CSCTL1 |= DISMOD_1 | DCOFTRIM | DCORSEL_5 | DCOFTRIMEN_1;  
  // Modulation disabled,  frequency trimmed to highest value, DCOf = 16 MHz, Trimming enabled
  //CSCTL2 |= 0x00FF;
  //CSCTL2 &= ~0x7000;
  //__delay_cycles(3);
  __bic_SR_register(0x0040);            // enable FLL
  
  CSCTL0 |= 0x01FF;
  for (i = 0; i <100; i++){
    if (Data[i] == 0);
      //__delay_cycles(6);
    else 
      P5OUT ^= 0x03;      
  }
  CSCTL4 |= SELMS_3;                     // MCLK = SMCLK = VLO
  __bis_SR_register(LPM3_bits | GIE);
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

