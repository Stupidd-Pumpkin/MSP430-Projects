// Equivalent to 'Timer_ADC.c' function but now produces a high frequency pulse if input signal > Vcc/2
#include <msp430.h>
#include <msp430fr2476.h>
#include <stdio.h>

unsigned int ADC_Out, ADC_Ch, Timer_Count, q;

void pulse()
{
  __bic_SR_register(0x0080);            // LPM3 --> LPM0
  CSCTL3 |= 0x0010;
  CSCTL1 |= 0x00FA;                             // 16 MHz
  CSCTL2 |= 0x00FF;
  //CSCTL2 &= ~0x7000;
  //__delay_cycles(3);
  __bic_SR_register(0x0040);                // enable FLL
  
  CSCTL0 |= 0x01FF;
  CSCTL5 &= ~0x0100;                      // SMCLK on

   P5OUT |= 0x02;
  __delay_cycles(1);
  P5OUT &= ~0x02;

  CSCTL5 |= 0x0100;                      // SMCLK off
  
  __bis_SR_register(0x00C0);
}

void ADC()
{
    ADCCTL0 |= 0x0003;                                        // Sampling and conversion start
}


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                                // Stop WDT
    CSCTL4 |= 0x0200;                     // MCLK=SMCLK=DCO; ACLK=VLO
   // CSCTL5 |= 0x0100;                      // SMCLK off
    
    ADC_Ch=0x02;                                 //P1.1 configured as A1
    Timer_Count=0x03FF;                         //0xFFFF == 2s Delay
    //q=getchar();
    //printf("1 \n");
    // Configure GPIO
    P1DIR |= 0x89;
    P1SEL0 &= ~0x88; // MCLK and SMCLK output
    P1SEL1 |= 0x88;
    P1OUT &= ~0x01;                                         
    
    P2DIR |= 0x04;
    P2SEL0 &= ~0x04;
    P2SEL1 |= 0x04;
    
    P4DIR |= 0x80;
    P4OUT &= ~0x80;
    
    P5DIR |= 0x03;
    P5OUT &= ~0x03;
    
    
    // Configure ADC input Channel
    P1SEL0 |= ADC_Ch;
    P1SEL1 |= ADC_Ch;
    
    
    PM5CTL0 &= ~LOCKLPM5;                                    // Disable the GPIO power-on default high-impedance mode
    
    // Configure ADC
    ADCCTL0 |= 0x0210;                                       // ADCON, S&H=16 ADC clks
    ADCCTL1 |= 0x0200;                                       // ADCCLK = MODOSC; sampling timer
    ADCCTL2 &= ~0x0030;                                      // clear ADCRES in ADCCTL i.e 8-bit 
    //ADCCTL2 |= 0x0020;                                     // 12-bit
    ADCMCTL0 |= 0x0001;                                      // A1 ADC input select; Vref=DVCC
    ADCIE |= 0x0001;                                         // Enable ADC conv complete interrupt
    
    // Configure Timer_A
    TA0CCTL0 |= 0x0010;                             // TACCR0 interrupt enabled
    TA0CCR0 = Timer_Count;
    TA0CTL = 0x0114;                            // ACLK, up mode, clear TAR, enable interrupt 
    __bis_SR_register(0x00D8);                  // LPM3
    __no_operation();                           // For debugger
}

// ADC interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC_VECTOR))) ADC_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
    {
        case ADCIV_NONE:
            break;
        case ADCIV_ADCOVIFG:
            break;
        case ADCIV_ADCTOVIFG:
            break;
        case ADCIV_ADCHIIFG:
            break;
        case ADCIV_ADCLOIFG:
            break;
        case ADCIV_ADCINIFG:
            break;
        case ADCIV_ADCIFG:
            ADC_Out = ADCMEM0;            
            break;
        default:
            break;
    }
    if (ADC_Out < 0x7F)
        P4OUT &= ~0x80;                                  // Clear P4.7 LED off
    else{
        P4OUT |= 0x80;                                   // Set P4.7 LED on
        pulse();   
    }
}

// Timer A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif
{   P1OUT ^= 0x01;
    ADC();
    TA0CCR0 = Timer_Count;
}
