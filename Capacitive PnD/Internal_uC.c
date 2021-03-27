//CAPACITIVE POWER AND DATA TRANSMISSION USING RESONANCE FREQUENCY MODULATION TECHNIQUE FOR DATA

#include <msp430.h>
#include <msp430f2132.h>

unsigned long int i,k;           
unsigned int delay, delay_max = 200, pulse_gap = 500, flag = 0, threshold = 105;
char data;

void anodic_no_del()
{
    P1OUT ^= 0x01;                               // Test
    P3OUT |= 0x10;                               // P3.5 set
    for(delay=0;delay<delay_max;delay++);        // Delay
    P3OUT &= ~0x10;                              // P3.5 reset
    P3OUT |= 0x04;                               // P3.2 set
    for(delay=0;delay<delay_max;delay++);        // Delay
    P3OUT &= ~0x04;                              // P3.2 reset
    for(delay=0;delay<delay_max;delay++);        // Delay
}

void anodic_del()
{
    P1OUT ^= 0x02;                               // Test
    P3OUT |= 0x10;                               // P3.5 set
    for(delay=0;delay<delay_max;delay++);        // Delay
    P3OUT &= ~0x10;                              // P3.5 reset
    for(delay=0;delay<delay_max;delay++);        // Delay
    P3OUT |= 0x04;                               // P3.2 set
    for(delay=0;delay<delay_max;delay++);        // Delay
    P3OUT &= ~0x04;                              // P3.2 reset
    for(delay=0;delay<delay_max;delay++);        // Delay
}

void cathodic_no_del()
{
    P1OUT ^= 0x04;                               // Test
    P3OUT |= 0x04;                               // P3.2 set
    for(delay=0;delay<delay_max;delay++);        // Delay
    P3OUT &= ~0x04;                              // P3.2 reset
    P3OUT |= 0x10;                               // P3.5 set
    for(delay=0;delay<delay_max;delay++);        // Delay
    P3OUT &= ~0x10;                              // P3.5 reset
    for(delay=0;delay<delay_max;delay++);        // Delay
}

void cathodic_del()
{
    P1OUT ^= 0x08;                               // Test
    P3OUT |= 0x04;                               // P3.2 set
    for(delay=0;delay<delay_max;delay++);        // Delay
    P3OUT &= ~0x04;                              // P3.2 reset
    for(delay=0;delay<delay_max;delay++);        // Delay      
    P3OUT |= 0x10;                               // P3.5 set
    for(delay=0;delay<delay_max;delay++);        // Delay
    P3OUT &= ~0x10;                              // P3.5 reset
    for(delay=0;delay<delay_max;delay++);        // Delay
}

int main(void)
{  
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  BCSCTL1 = CALBC1_1MHZ;                    // Set SMCLK at 1MHz
  DCOCTL = CALDCO_1MHZ;  
  
  P1DIR |= 0x0F;                            // Set P1 to output direction
  P1DIR |= 0x10;  P1SEL |= 0x10;            // SET THE PIN DIRECTION FOR SMCLK OUT (P1.4)
  
  P2DIR |= 0x01;
  P2SEL |= 0x01;                            // ACLK on P2.0  
  BCSCTL3 |= 0x20;                          // ACLK setup to VLO

  P3SEL |= 0x20;                            // Enable UART RXD Port
  UCA0CTL1 |= UCSSEL_1;                     // ACLK
  UCA0BR0 = 8;                              // 10.55 kHz Baud Rate: 1200
  UCA0BR1 = 0;                              // 10.55 kHz Baud Rate: 1200
  UCA0MCTL = 0x0E;                          // Modulation UCBRSx = 7
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI_A0 state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

  
  P3SEL |= 0x0A;                            // P3.3 = UCB0CLK, P3.1 = UCB0SIMO //P3.0
  P3DIR |= 0x01;                            // P3.0 output direction
  UCB0CTL0 |= UCMSB + UCMST + UCSYNC + UCMODE0 + UCCKPL;
  UCB0CTL1 |= UCSSEL_2;                     // SMCLK
  UCB0BR0 = 100;                            // Divide by 100   
  UCB0BR1 = 0;
  UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI_B0 state machine**
  
  P3OUT |= 0x01;                             // Enable Chip Select
  while (!(IFG2 & UCB0TXIFG));              // USCI_B0 TX buffer ready?
  for(delay=0;delay<1000;delay++);
  UCB0TXBUF = threshold;                    // Byte to SPI TXBUF //01101011
  for(delay=0;delay<1000;delay++);
  P3OUT &= ~0x01;                           // Disable Chip Select
  
  P3DIR |= 0x14;                            //Stimulator Outputs reset P3.2 and P3.4
  P3OUT &= ~0x14;                           //Output pins set to zero initially
  
   __bis_SR_register(GIE);                  //Interrupts enabled
   
  for (;;)
  {
    if (flag==1)
    {
      switch(data)
      {
      case 's': anodic_no_del(); break;
      case 'K': anodic_del(); break;
      case 'Z': cathodic_no_del(); break;
      case 'N': cathodic_del(); break;
      case '+': anodic_no_del();  for(delay=0;delay<pulse_gap;delay++);   // All Stimulations 
        anodic_del();    for(delay=0;delay<pulse_gap;delay++);
        cathodic_no_del();  for(delay=0;delay<pulse_gap;delay++);
        cathodic_del();  for(delay=0;delay<pulse_gap;delay++); break;
      //case '3': calibrate_threshold(); break;                           
      }
      flag = 0;
    }
  }  
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{ 
    //while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
    //UCA0TXBUF = UCA0RXBUF;                    // TX -> RXed character
    data = UCA0RXBUF;
    flag = 1;
}