// Use this code if you want to test only the Analog to Digital Converter (ADC) module of msp430fr2xxx
// 12-bit ADC conversion of the analog input given to P1.1 is performed and the ADC output is compared with Vcc/4
// If P1.1 analog input < Vcc/4: P1.0 LED is turned off. Else: P1.0 LED is turned on.
// Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430fr2476.h' to understand registers and their functions

#include <msp430fr2476.h> 										  // Each register location is associated with it's corresponding name in this header file

unsigned int ADC_Out;										// ADC output

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                               // Stop Watch Dog Timer (necessary to keep the code running)

    // Configure P1.0 as output pin
    P1DIR |= BIT0;                                          // Set P1.0/LED to output direction
    P1OUT |= BIT0;                                          // P1.0 out off

    // Configure P1.1 as ADC A1 input
    P1SEL0 |= BIT1;
    P1SEL1 |= BIT1;

    PM5CTL0 &= ~LOCKLPM5;                                   // Disable the GPIO power-on default high-impedance mode

    // Configure ADC
    ADCCTL0 |= 0x0210;                             			// ADCON, Sample and Hold time = 16 ADC clks
    ADCCTL1 |= 0x0200;                                      // ADCCLK = MODOSC; sampling timer
    ADCCTL2 &= ~0x0030;                                     // clear ADCRES in ADCCTL i.e ADC resolution set to 8-bit as default
    ADCCTL2 |= 0x0020;                                     	// ADC resolution set to 12-bit
    ADCMCTL0 |= 0x0001;                                   	// A1 (P1.1) is selected as ADC input; reference voltage for ADC (Vref) is set as DVCC
    ADCIE |= 0x0001;                                        // Enable ADC conv complete interrupt

    while(1) 												// Forever loop
    {
        ADCCTL0 |= 0x0003;                           		// Start sampling and conversion
        __bis_SR_register(0x0018);                 		 	// Go to LPM0, ADC_ISR will force exit
        __no_operation();                                   // For debug only
        if (ADC_Out < 0x3FF)								// Compare ADC with DVCC*0x3FF/0xFFF
            P1OUT &= ~0x01;                                 // Clear P1.0 LED off
        else
            P1OUT |= 0x01;                                   // Set P1.0 LED on
        __delay_cycles(5000);
    }
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
            __bic_SR_register_on_exit(0x0010);            // Clear CPUOFF bit from LPM0
            break;
        default:
            break;
    }
}