// Use this code if you want to test pulse width modulation using timer modules of msp430fr2xxx
// Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430.h' to understand registers and their functions

#include <msp430.h>
#include <msp430fr2476.h>
#include <stdio.h>

volatile unsigned int i, y, x;                    // volatile to prevent optimization

unsigned int  Timer_Count0, Timer_Count1;

int main(void)
{
  i = 0;
  x = 0x0001;
  Timer_Count0 = 0x002F;                       
  Timer_Count1 = 0x0001;                       
  
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  P5DIR |= 0x03;                            
  P4DIR |= 0x80;                            
  
  P1DIR |= 0x89;
  P1SEL0 &= ~0x88;                          // MCLK and SMCLK output for debugging
  P1SEL1 |= 0x88;
  P1SEL0 |= BIT1;                           // Select eCOMP input function on P1.1/C0
  P1SEL1 |= BIT1;

  P1OUT &= ~0x01;
  P5OUT &= ~0x03;
  P4OUT &= ~0x80;
  
  PM5CTL0 &= ~LOCKLPM5;
  
  __bis_SR_register(SCG0);                // disable FLL
  CSCTL3 |= 0x0010;
  CSCTL1 |= 0x00FA;                       // 16 MHz
  CSCTL2 |= 0x00FF;
  //CSCTL2 &= ~0x7000;
  //__delay_cycles(3);
  __bic_SR_register(SCG0);                // enable FLL
   
  CSCTL0 |= 0x01FF;
  CSCTL4 |= 0x0200;                       // MCLK = SMCLK = DCO; ACLK = VLO
  CSCTL5 &= ~0x0100;                      // SMCLK on
  
  // Setup eCOMP
  CPCTL0 &= ~CPNSEL0;                     // Clear the  CPNSEL0 bit first                        
  CPCTL0 |= CPNSEL1 | CPNSEL2;            // Select DAC as input for V- terminal
  CPCTL0 |= CPPEN | CPNEN;                // Enable eCOMP input
  CPDACCTL |= CPDACEN;                    // Select VCC as reference and enable DAC
  CPDACDATA |= 0x0020;                    // CPDACBUF1 = VCC *32/64
  CPCTL1 |= CPHSEL_3;                     //  Hystresis selection
      
  // Configure Timer_0
    TA0CCTL0 |= 0x0010;                   // TA0CCR0 interrupt enabled
    TA0CCR0 = Timer_Count0;
    
    TA0CCTL1 |= 0x0010;                   // TA0CCR1 interrupt enabled
    TA0CCR1 = Timer_Count1;
    
    TA0CTL = 0x0214;                      // SMCLK, up mode, clear TAR, enable interrupt 
    
    __bis_SR_register(0x00C8);                  // LPM3
 
}


// TimerA0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TimerA0 (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) TimerA0 (void)
#else
#error Compiler not supported!
#endif
{
      P5OUT |= 0x02; 
      TA0CCR0 = Timer_Count0;
      TA0CCR1 = Timer_Count1;
      i += 1;
      if (i == 200){
        CPCTL1 |= CPEN;         // Turn on eCOMP
        i = 0;
        if ((CPCTL1 & 0x0001) == 0){
          Timer_Count1 -= 0x0001;
          P5OUT ^= 0x01;
        }
        else {
          Timer_Count1 -= 0x0001;
          P1OUT ^= 0x01;
        }
      }
        
}

// TimerA1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TimerA1 (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A1_VECTOR))) TimerA1 (void)
#else
#error Compiler not supported!
#endif
{
      P5OUT &= ~0x02;   
}


