// Use this code if you want to test Analog to Digital Converter (ADC) module periodically sampling based on timer rollover in msp430fr2xxx
// 8-bit ADC conversion of the analog input given to P1.1 is performed and the ADC output is compared with Vcc/2
// If P1.1 analog input < Vcc/2: P4.7 LED is turned off. Else: P4.7 LED is turned on
// Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430fr2476.h' to understand registers and their functions

#include <msp430.h>
#include <msp430fr2476.h>

unsigned int ADC_Out, ADC_Ch, Timer_Count;

void ADC()
{
        // Configure ADC
    ADCCTL0 |= 0x0210;      // ADCON, S&H=16 ADC clks
    ADCCTL1 |= 0x0200;      // ADCCLK = MODOSC; sampling timer
    ADCCTL2 &= ~0x0030;     // clear ADCRES in ADCCTL i.e ADC resolution set to 8-bit as default
    //ADCCTL2 |= 0x0020;      // ADC resolution set to 12-bit
    ADCMCTL0 |= 0x0001;     // A1 (P1.1) is selected as ADC input; reference voltage for ADC (Vref) is set as DVCC
    ADCIE |= 0x0001;        // Enable ADC conv complete interrupt
    ADCCTL0 |= 0x0003;      // Sampling and conversion start
    if (ADC_Out < 0x7F)
        P4OUT &= ~0x80;     // Clear P4.7 LED off
    else
        P4OUT |= 0x80;      // Set P4.7 LED on
}


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;// Stop Watchdog Timer
    CSCTL4 = 0x0200;        // MCLK=SMCLK=DCO; ACLK=VLO
    
    ADC_Ch=0x02;            // P1.1 configured as A1
    Timer_Count=0x03FF;     // 0xFFFF == 2s Delay
    
    // Configure GPIO
    P1DIR |= 0x01;                                           
    P1OUT &= ~0x01;                                         
    P2DIR |= 0x04;
    P4DIR |= 0x80;
    P4OUT &= ~0x80;
    
    
    // Configure ADC Channel
    P1SEL0 |= ADC_Ch;
    P1SEL1 |= ADC_Ch;
    P2SEL0 &= ~0x04;
    P2SEL1 |= 0x04;
    
    PM5CTL0 &= ~LOCKLPM5;                                    // Disable the GPIO power-on default high-impedance mode
    
    
    // Configure Timer_A
    TA0CCTL0 |= 0x0010;                             // TACCR0 interrupt enabled
    TA0CCR0 = Timer_Count;
    TA0CTL = 0x0114;                            // ACLK, count mode, clear TAR, enable interrupt 
    __bis_SR_register(0x00C8);                  // LPM3
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
            //__bic_SR_register_on_exit(0x00C0);            // Clear CPUOFF bit from LPM0
            break;
        default:
            break;
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
{
    //__bis_SR_register(0x00C8);                  // LPM3
    P1OUT ^= 0x01;
    ADC();
    TA0CCR0 = Timer_Count;                    
}
