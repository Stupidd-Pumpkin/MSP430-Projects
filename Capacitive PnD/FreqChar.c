#include <msp430.h>
#include <msp430f2132.h>

unsigned long int d=40000, q=50000, k;

int main (void)
{
  WDTCTL = WDTPW + WDTHOLD;                     // Stop watchdog timer
  //BCSCTL1 = 0x06; 
  P1DIR |= 0x10;  P1SEL |= 0x10;
  //DCOCTL = 0x70;       
  /*BCSCTL1 =0x00; for(k=0;k<=d/10;k++);
  BCSCTL1 = 0x01; for(k=0;k<=d/10;k++);
  BCSCTL1 = 0x02; for(k=0;k<=d/10;k++);
  BCSCTL1 = 0x03; for(k=0;k<=d/10;k++);
  BCSCTL1 = 0x04; for(k=0;k<=d/10;k++);
  BCSCTL1 = 0x05; for(k=0;k<=d/10;k++);*/
  
  /*BCSCTL1 = 0x06; for(k=0;k<=d;k++); d=d*3; d=d/2;
  BCSCTL1 = 0x07; for(k=0;k<=d;k++); d=d*3; d=d/2;
  BCSCTL1 = 0x08; for(k=0;k<=d;k++); d=d*3; d=d/2;
  BCSCTL1 = 0x09; for(k=0;k<=d;k++); d=d*3; d=d/2;
  BCSCTL1 = 0x0A; for(k=0;k<=d;k++); d=d*3; d=d/2;
  BCSCTL1 = 0x0B; for(k=0;k<=d;k++); d=d*3; d=d/2;
  BCSCTL1 = 0x0C; for(k=0;k<=d;k++); d=d*3; d=d/2;
  BCSCTL1 = 0x0D; for(k=0;k<=d;k++); d=d*3; d=d/2;
  BCSCTL1 = 0x0E; for(k=0;k<=d;k++); d=d*3; d=d/2;*/
  
  /*BCSCTL1 = 0x08;
  DCOCTL = 0x10; for(k=0;k<=d;k++);
  DCOCTL = 0x20; for(k=0;k<=d;k++); 
  DCOCTL = 0x30; for(k=0;k<=d;k++);
  DCOCTL = 0x40; for(k=0;k<=d;k++);
  DCOCTL = 0x50; for(k=0;k<=d;k++);
  DCOCTL = 0x60; for(k=0;k<=d;k++);
  DCOCTL = 0x70; for(k=0;k<=d;k++);
  DCOCTL = 0x80; for(k=0;k<=d;k++);
  DCOCTL = 0x90; for(k=0;k<=d;k++);
  DCOCTL = 0xA0; for(k=0;k<=d;k++);
  DCOCTL = 0xB0; for(k=0;k<=d;k++);
  DCOCTL = 0xC0; for(k=0;k<=d;k++);
  DCOCTL = 0xD0; for(k=0;k<=d;k++);
  DCOCTL = 0xE0; for(k=0;k<=d;k++);
  DCOCTL = 0xF0; for(k=0;k<=d;k++);
  d=d*3; d=d/2;*/
  
  
  
  BCSCTL1 = 0x09;  
  DCOCTL = 0x10; for(k=0;k<=d;k++);
  DCOCTL = 0x20; for(k=0;k<=d;k++); 
  DCOCTL = 0x30; for(k=0;k<=d;k++);
  DCOCTL = 0x40; for(k=0;k<=d;k++);
  DCOCTL = 0x50; for(k=0;k<=d;k++);
  DCOCTL = 0x60; for(k=0;k<=d;k++);
  DCOCTL = 0x70; for(k=0;k<=d;k++);
  DCOCTL = 0x80; for(k=0;k<=d;k++);
  DCOCTL = 0x90; for(k=0;k<=d;k++);
  DCOCTL = 0xA0; for(k=0;k<=d;k++);
  DCOCTL = 0xB0; for(k=0;k<=d;k++);
  DCOCTL = 0xC0; for(k=0;k<=d;k++);
  DCOCTL = 0xD0; for(k=0;k<=d;k++);
  DCOCTL = 0xE0; for(k=0;k<=d;k++);
  DCOCTL = 0xF0; for(k=0;k<=d;k++);
  d=d*3; d=d/2;
  
  /*BCSCTL1 = 0x0A;
  DCOCTL = 0x20; for(k=0;k<=d;k++);
  //BCSCTL1 = 0x03; for(k=0;k<=q;k++);
  //BCSCTL1 = 0x0A;
  DCOCTL = 0x40; for(k=0;k<=d;k++);
  //BCSCTL1 = 0x03; for(k=0;k<=q;k++);
  //BCSCTL1 = 0x0A;
  DCOCTL = 0x60; for(k=0;k<=d;k++);
  //BCSCTL1 = 0x03; for(k=0;k<=q;k++);
  //BCSCTL1 = 0x0A;
  DCOCTL = 0x80; for(k=0;k<=d;k++);
  //BCSCTL1 = 0x03; for(k=0;k<=q;k++);
  //BCSCTL1 = 0x0A;
  DCOCTL = 0xA0; for(k=0;k<=d;k++);
  d=d*3; d=d/2;*/
  
  /*BCSCTL1 = 0x09; for(k=0;k<=d;k++);
  DCOCTL = 0x60; for(k=0;k<=d;k++);             //Max PDL
  DCOCTL = 0x70; for(k=0;k<=d;k++);*/
  
  for(;;){    
    BCSCTL1 = 0x04; DCOCTL = 0x00; for(k=0;k<=d;k++);
  }
}