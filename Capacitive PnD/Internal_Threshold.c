//CAPACITIVE POWER AND DATA TRANSMISSION USING RESONANCE FREQUENCY MODULATION TECHNIQUE FOR DATA

#include <msp430.h>
#include <msp430f2132.h>

unsigned long int i,k;              
unsigned int delay, delay_max = 1000, pulse_gap = 2500, flag = 0, flag2 = 0, tol = 5;
unsigned long int count0 = 0, count1 = 0, threshold = 94, jump = 2, jump_diff = 1, count_max = 5000, calib_delay;
char data;
unsigned long int count, count_max2 = 500;

void anodic_no_del()                             // Anodic First No Delay Stimulation
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

void anodic_del()                                // Anodic First + Delay Stimulation
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

void cathodic_no_del()                           // Cathodic First No Delay Stimulation
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

void cathodic_del()                              // Cathodic First + Delay Stimulation
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

void threshold_SPI()
{
  P3OUT |= 0x01;                                 // Enable Chip Select
  while (!(IFG2 & UCB0TXIFG));                   // USCI_B0 TX buffer ready?
  for(delay=0;delay<1000;delay++);
  UCB0TXBUF = threshold;                         // Byte to SPI TXBUF
  for(delay=0;delay<1000;delay++);
  P3OUT &= ~0x01;                                // Disable Chip Select
}

// Still this portion of the code has to be modified and integrated with a Timer so that the synchronziation can happen with consecutive calibration bytes
void calibrate_threshold()
{
  P3SEL &= ~0x20;                                // Disable UART RXD on P3.5
  P3DIR &= ~0x20;                                // Enable P3.5 as an input port
  for(i=0; i<10; i++)                           // 10 Pulses
  {
    P1OUT ^= 0x02;      //
    while(P3IN & BIT5);                          // Wait till first zero arrives
    P1OUT ^= 0x02;      //
    for(;;)
    {
      count0++; 
      if (count0 == count_max) break;
      if(P3IN & BIT5) break;
    }    
    P1OUT ^= 0x02;      //
    while(P3IN & BIT5)
    {                                            // Counting length of high bit
      count1++;  
      if (count1 == count_max) break;
    }
    
    if(count0>count1-tol & count0<count1+tol)    // Tolerance band
    {
      P1OUT ^= 0x01;                             
      break;                                     // Stop any further calibration
    }
    else if (count0>count1) 
    {
      threshold = threshold + jump;              // Threshold Change
      P1OUT ^= 0x04;    //
    } 
    else if (count1>count0)  
    {
      threshold = threshold - jump; 
      P1OUT ^= 0x08;    //
    }
    threshold_SPI();                            // Threshold update to AD5165
    P1OUT ^= 0x02;      //
    //jump = jump - jump_diff;
    for(calib_delay=0; calib_delay<1000; calib_delay++);
    
  }  
  P3SEL |= 0x20;                                // Re-enable UART RXD port
}

int main(void){  
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
  DCOCTL = CALDCO_1MHZ;  
  
  P1DIR |= 0x0F;  P1OUT = 0x00;             // Set P1 to output direction
  P1DIR |= 0x10;  P1SEL |= 0x10;            // SET THE PIN DIRECTION FOR SMCLK OUT (P1.4)
  
  P2DIR |= 0x01;
  P2SEL |= 0x01;                            // ACLK on P2.0  
  BCSCTL3 |= 0x20;                          // ACLK setup to VLO

  P3SEL |= 0x0A;                            // P3.3 = UCB0CLK, P3.1 = UCB0SIMO //P3.0
  P3DIR |= 0x01;                            // P3.0 output direction
  UCB0CTL0 |= UCMSB + UCMST + UCSYNC + UCMODE0 + UCCKPL;
  UCB0CTL1 |= UCSSEL_2;                     // SMCLK
  UCB0BR0 = 100;                            // Divide by 100   
  UCB0BR1 = 0;
  UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI_B0 state machine**
  
  threshold_SPI();                          // Initialization Threshold at 1.1V
  
  // Initial Calibration Code
  P3SEL &= ~0x20; P3DIR &= ~0x20;          //Turn P3.5 as an input port
  
  for(;;)
  {    
    for(count = 0; count<=count_max2; count++)
    {
      if(P3IN & BIT5) 
      {
        P1OUT ^= 0x01; //
        break;
      }
      if(count == count_max2) 
      {
        P1OUT ^= 0x02;  //
        flag2 = 1;
      }
    }
    if(flag2 == 1) 
    {
      P1OUT ^= 0x04;    //
      for(;;)
      {
        if(P3IN & BIT5) 
          break;
      }
      P1OUT ^= 0x08;    //
      break;
    }
  }    
  for(delay=0; delay<100;delay++);          
  P1OUT ^= 0x02;        //
  calibrate_threshold();
  P1OUT ^= 0x01;        //
  
  
  //P3SEL |= 0x20;                            // Enable UART RXD Port
  P3SEL |= 0x30;
  UCA0CTL1 |= UCSSEL_1;                     // ACLK
  UCA0BR0 = 8;                              // 10.55 kHz Baud Rate: 1200
  UCA0BR1 = 0;                              // 10.55 kHz Baud Rate: 1200
  UCA0MCTL = 0x0E;                          // Modulation UCBRSx = 7
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI_A0 state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
  
  //P3DIR |= 0x14;                            //Stimulator Outputs reset P3.2 and P3.4
  //P3OUT &= ~0x14;                           //Output pins set to zero initially
  
  P1OUT ^= 0x02;                             //Test
  
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
      case '3': calibrate_threshold(); break;                           
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
    while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
    UCA0TXBUF = UCA0RXBUF;                    // TX -> RXed character
    data = UCA0RXBUF;
    //flag = 1;
}