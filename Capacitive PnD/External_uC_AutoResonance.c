//CAPACITIVE POWER AND DATA TRANSMISSION USING RESONANCE FREQUENCY MODULATION TECHNIQUE FOR DATA

#include <msp430.h>
#include <msp430f2132.h>

unsigned int adc_max = 0, adc1, adc2, adc3, adc4, adc5, adc;                // Initializing ADC Parameters
unsigned int bcs, dco, bcs_nonres = 0x06, dco_sweep, dco_max=0x60, bcs_sweep, bcs_max=0x09;   // Initalizing Clock Parameters
unsigned long int d0, d1, d2, d3, delay1 = 35, delay2 = 140, delay_adc, delay_off = 50000, freq;  // Initializing Delay Counters
int flag = 0, i, j, k[8];                                       // Intializing Counters and flags
unsigned int br = 1200, br_int1 = 8, br_int2 = 0, br_frac = 14; // Initializing Baud Rate Parameters for UART
char data, calibrate_char = 'U';                                // Character to store Data received from Simblee         

//LUT can also be used for sourcing the Baud Rate Constants for higher data rates when SMCLK is going to be used
unsigned long int lut[10][7] =                       // Frequency Lookup Table //Add more rows of data!
{ 
  {239e3,254e3,272e3,292e3,315e3,342e3,375e3},
  {335e3,358e3,384e3,412e3,444e3,483e3,527e3},
  {471e3,501e3,535e3,574e3,620e3,673e3,736e3},
  {656e3,699e3,745e3,800e3,861e3,935e3,1.022e6},
  {930e3,990e3,1.057e6,1.131e6,1.222e6,1.323e6,1.445e6},
  {1.316e6,1.398e6,1.491e6,1.597e6,1.718e6,1.867e6,2.033e6},
  {1.892e6,2.011e6,2.141e6,2.291e6,2.463e6,2.670e6,2.911e6},
  {2.711e6,2.877e6,3.058e6,3.276e6,3.511e6,3.800e6,4.142e6},
  {3.471e6,3.685e6,3.924e6,4.202e6,4.521e6,4.902e6,5.355e6},
  {4.760e6,5.060e6,5.380e6,5.753e6,6.181e6,6.702e6,7.302e6},  
};

void br_const()                                    // Baud Rate Constants Estimator Functions (Select a baud rate based on SMCLK frequency): Optional
{                        
  float div = freq/br;                             // freq is a global variable with the Resonant Frequency Value
  br_int1 = (int) div;
  br_frac = ((int)((div - br_int1)*8))*2;
  if(br_int1>255)
  {
    br_int2 = br_int1/256; 
    br_int1 = br_int1 - (256*br_int2);
  }
  else
    br_int2 = 0;
}

void auto_resonance()                             // Function to generate Auto-resonant Frequency on SMCLK
{
  delay_adc = 500;                                // Delay Initialization
  for(bcs_sweep=3;bcs_sweep<=12;bcs_sweep++)      // Sweep BSCTL1 between 0x03 and 0x0C; 
  {    
    BCSCTL1 = bcs_sweep;                          // Intiatlize and update BCSCTL
    for(dco_sweep=32;dco_sweep<=160;dco_sweep=dco_sweep+32)     // Sweep DCOCTL between 0x20 and 0xA0 
    {      
      DCOCTL = dco_sweep;                         // Intiatlize and update DCOCTL
      for(d1=0; d1<delay_adc*100; d1++);              // Delay to stabilize voltage
      
      //Repeat ADC measurement 3 times at this particular frequency (for measuring max V and check for a resonance)
      ADC10CTL0 |= ENC + ADC10SC;                 // Start Conversion
      while (ADC10CTL1 & ADC10BUSY);              // Wait till conversion is complete
      adc1 = ADC10MEM;                            // Read an ADC converted value
      for(d1=0; d1<delay_adc; d1++);              // Delay to stabilize a particular AC voltage from a patch at a frequency 
   
      ADC10CTL0 |= ENC + ADC10SC;                 // Start Conversion
      while (ADC10CTL1 & ADC10BUSY);              // Wait till conversion is complete
      adc2 = ADC10MEM;
      for(d1=0; d1<delay_adc; d1++);              // Delay to stabilize voltage
      
      ADC10CTL0 |= ENC + ADC10SC;                 // Start Conversion
      while (ADC10CTL1 & ADC10BUSY);              // Wait till conversion is complete
      adc3 = ADC10MEM;
      for(d1=0; d1<delay_adc; d1++);              // Delay to stabilize voltage
      
      ADC10CTL0 |= ENC + ADC10SC;                 // Start Conversion
      while (ADC10CTL1 & ADC10BUSY);              // Wait till conversion is complete
      adc4 = ADC10MEM;
      for(d1=0; d1<delay_adc; d1++);              // Delay to stabilize voltage
      
      ADC10CTL0 |= ENC + ADC10SC;                 // Start Conversion
      while (ADC10CTL1 & ADC10BUSY);              // Wait till conversion is complete
      adc5 = ADC10MEM;

     adc = adc1 + adc2 + adc3 + adc4 + adc5;
      if(adc > adc_max)                           // Update segment if peak is found
      {                   
        adc_max = adc;                            // Current ADC value replaces the existing ADC maximum
        bcs_max = bcs_sweep;                      // Corresponding Clock frequency parameters are also stored
        dco_max = dco_sweep;   
      }            
    }
    delay_adc = delay_adc*3; delay_adc = delay_adc/2;  // Delay is increased by 1.5 times everytime BCSCTL is increased to the next value
  }
  
  bcs = bcs_max; dco = dco_max;                   // Setting BCS, DCO and BCS_NonRes(Half of Resonant Frequency) values 
  BCSCTL1 = bcs; DCOCTL = dco;                   // Setting SMCLK at Resonant Frequency   
  
  freq  =  lut[bcs_max-3][(dco_max/32)];        // Estimates Delays for byte-transfer with the found Maximum BCS and DCO values
  delay1 = ((long int) (2.291e6*140/freq))/10;
  if(bcs_max > 8)
  {
    bcs_nonres = (bcs_max - 2);                // Non-resonant frequency is half of resonant frequency
    delay2 = delay1*2;                         // Multiplied by 2
  }  
  else
  {
    bcs_nonres = (bcs_max + 2);               // Non-resonant frequency is double of resonant frequency
    delay2 = delay1/2;                        // Divided by 2
  }  
  
  //Additional: Add on the fine tuning by taking average of the values in the lookup table
}

void byte_transfer()                             // Transfer a Byte from External Chip to the Internal Chip (FSK Code)
{
  BCSCTL1 = bcs_nonres;                             //Start Bit
  for(d2=0; d2<delay1; d2++);
  
  for(i=0; i<=7; i++)
  {
    if(k[i] == 1)
    {
      BCSCTL1 = bcs;                               //Resonant Frequency
      for(d2=0; d2<delay2; d2++);
    }
    else
    {
      BCSCTL1 = bcs_nonres;                       //Non-Resonant Frequency
      for(d2=0; d2<delay1; d2++);
    }
  }
  BCSCTL1 = bcs;                                  // Stop Bit
  for(d2=0; d2<delay2; d2++);
}

void calibrate_intialize()                       // Function to send 01010101 and calibrate internal chip's comparator threshold
{                  
  for(i=0; i<=7; i++)
    k[i] = (calibrate_char >> i) & 0x01;         // calibrate_char = 'U'
  for(d3=0; d3<delay2*50; d3++);
  for(j=0; j<10; j++)                            // Run 10 times
  {                       
    for(d3=0; d3<delay2*100; d3++);
    byte_transfer();
  }
}

int main (void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  
  P1DIR |= 0x0F;  P1OUT = 0x00;             // Test pins
  
  P1DIR |= 0x10;  P1SEL |= 0x10;            // SET THE PIN DIRECTION FOR SMCLK OUT (P1.4)            

  // Auto-Resonance Segment
  ADC10CTL0 = ADC10SHT_2 + ADC10ON;         // ADC10ON and ADC samples at 16 x ADC10CLKs
  ADC10AE0 |= 0x01;                         // P2.0 ADC option select 
  auto_resonance();                         // Brings the system to resonance
  ADC10CTL0 &= ~ADC10ON;                    // Turn off ADC

  
/* //Uncomment this portion of the code when Calibration of Internal Threshold is required
  P1SEL &= ~0x10;                           // Turn SMCLK OFF
  for(d0=0; d0<delay_off; d0++);            // Delay to Power Off Internal uC completely
  
  P1SEL |= 0x10;                            // Turn SMCLK ON
  calibrate_intialize();                    // Calibration Initialization in Synchronization with Internal uC
  */
    
  P2DIR |= 0xFF;                            // UART Testing
  P2OUT = 0xF0;
  BCSCTL3 |= 0x20;                          // ACLK setup to VLO

  //Code to run UART on ACLK
  P3SEL |= 0x30;                            // Enable UART TXD and RXD Port
  UCA0CTL1 |= UCSSEL_1;                     // ACLK
  UCA0BR0 = br_int1;                        // 10.55 kHz Baud Rate: 1200
  UCA0BR1 = br_int2;                        // 10.55 kHz Baud Rate: 1200
  UCA0MCTL = br_frac;                       // Modulation UCBRSx = 7
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

/*
  // Optional Code to run UART on SMCLK    
  P3SEL |= 0x30;                            // Enable UART TXD and RXD Port
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  //// Fixed Baud Rate Constants for some of the commonly used SMCLK frequency setups
  UCA0BR0 = 117;                            // 2.291MHz Baud Rate: 1200
  UCA0BR1 = 7;                              // 2.291MHz Baud Rate: 1200
  UCA0MCTL = 0x02;                          // Modulation UCBRSx = 1
  UCA0BR0 = 108;                            // 745kHz Baud Rate: 1200
  UCA0BR1 = 2;                              // 745kHz Baud Rate: 1200
  UCA0MCTL = 0x0E;                          // Modulation UCBRSx = 7
  ///// 
  //Use the Four Lines of Code to have the Baud Rate adjusted to the current SMCLK frequency setup
  br_const();
  UCA0BR0 = br_int1;                        // Adjustable Baud Rate: 1200
  UCA0BR1 = br_int2;                        // Adjustable Baud Rate: 1200
  UCA0MCTL = br_frac;                       // Adjustable Modulation UCBRSx
  
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt  
*/
  __bis_SR_register(GIE);                   //Interrupts enabled
 
 for(;;)
 {
    if(flag==1)
    {
      //P2OUT ^= 0xFF;                        // Test Pins
      for(i=0; i<=7; i++)
        k[i] = (data >> i) & 0x01;      
      byte_transfer();      
      if (data == '3')                      // Perform AutoResonance and Calibration
      {
        auto_resonance();
        //calibrate_intialize();            // Uncomment this function to run the Internal Threshold calibration
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
  while (!(IFG2&UCA0TXIFG));             // USCI_A0 TX buffer ready?
  UCA0TXBUF = UCA0RXBUF;                 // TX -> RXed character
  data = UCA0RXBUF;                      // Shift Character received to Data
  flag = 1;                              // Increment Flag                          
}