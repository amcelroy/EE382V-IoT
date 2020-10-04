// Seeed_HC12main.c
// Runs on MSP432
// Use the SysTick timer to request interrupts at a particular period.
// Jonathan Valvano
// August 19, 2020

/* This example accompanies the book
   "Embedded Systems: Introduction to Robotics,
   Jonathan W. Valvano, ISBN: 9781074544300, copyright (c) 2020
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2020, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/


// built-in LED1 connected to P1.0
// P1.0, P2.0 are an output to profiling scope/logic analyzer
// UCA2RXD (VCP receive) connected to P3.2
// UCA2TXD (VCP transmit) connected to P3.3
// HC12 pin connection
// 5 SET to GPIO      J2.18 from LaunchPad to HC12  (GPIO output){MSP432 P3.0}
// 4 SO to serial RxD J1.3 from HC12 to LaunchPad (UCA2TXD RxD){MSP432 P3.2}
// 3 SI to serial TxD J1.4 from LaunchPad to HC12  (UCA2TXD TxD){MSP432 P3.3}
// 2 GND
// 1 VCC 1N4004 diode to +5V Vin (plus side of diode on +5V)
// Power pin, the requirements of 3.2V to 5.5V
// DC power supply, the supply current is not less
// than 200mA. Note: If the module is to work
// for a long time in the transmit state, it is
// recommended that the power supply voltage
// of more than 4.5V when connected to a
// 1N4007/1N4004 diode, to avoid the module built-in
// LDO fever.
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "msp.h"
#include "iot_packet.h"
#include "../inc/CortexM.h"
#include "../inc/SysTickInts.h"
#include "../inc/LaunchPad.h"
#include "../inc/Clock.h"
#include "../inc/UART0.h"
#include "../inc/UART1.h"
volatile uint32_t Time,MainCount;
#define LEDOUT (*((volatile uint8_t *)(0x42098040)))
// P3->OUT is 8-bit port at 0x4000.4C22
// I/O address be 0x4000.0000+n, and let b represent the bit 0 to 7.
// n=0x4C22, b=0
// bit banded address is 0x4200.0000 + 32*n + 4*b
#define SET (*((volatile uint8_t *)(0x42098440)))
char HC12data;
// every 10ms
void SysTick_Handler(void){
  uint8_t in;
  LEDOUT ^= 0x01;       // toggle P1.0
  LEDOUT ^= 0x01;       // toggle P1.0
  Time = Time + 1;
  uint8_t ThisInput = LaunchPad_Input();   // either button
  if(ThisInput){
    if((Time%100) == 0){ // 1 Hz
      HC12data = HC12data^0x01; // toggle '0' to '1'
      if(HC12data == 0x31){
        printf("S1\n");
      }else{
        printf("S0\n");
      }
      UART1_OutChar(HC12data);
    }
  }
  in = UART1_InCharNonBlock();
  if(in){
    switch(in){
      case '0':
        printf("R0\n");
        LaunchPad_Output(0); // off
        break;
      case '1':
        printf("R1\n");
        LaunchPad_Output(BLUE);
        break;
    }
  }
  LEDOUT ^= 0x01;       // toggle P1.0
}

void HC12_ReadAllInput(void){uint8_t in;
// flush receiver buffer
  in = UART1_InCharNonBlock();
  while(in){
    UART0_OutChar(in);
    in = UART1_InCharNonBlock();
  }
}

void HC12_ReadForever(void){uint8_t in;
// flush receiver buffer
  in = UART1_InChar();
  while(1){
    UART0_OutChar(in);
    in = UART1_InChar();
  }
}


void HC12_Init(uint32_t baud){
  P3->SEL0 &= ~0x01;
  P3->SEL1 &= ~0x01;    // configure P3.0 as GPIO
  P3->DIR |= 0x01;      // make P3.0 out
  UART1_InitB(baud);    // serial port to HC12
  HC12data = '1';
  //************ configure the HC12 module**********************
  SET = 0;       // enter AT command mode
  Clock_Delay1ms(40);
  UART1_OutString("AT+B9600\n");  // UART baud rate set to 9600
  Clock_Delay1ms(50);
  HC12_ReadAllInput();
  UART1_OutString("AT+C007\n");   // channel 7 selected (001 to 100 valid)
  Clock_Delay1ms(50);
  HC12_ReadAllInput();
  UART1_OutString("AT+P8\n");    // highest power level (1 to 8)
  Clock_Delay1ms(50);
  HC12_ReadAllInput();
  UART1_OutString("AT+RF\n");    // read FU transmission mode (FU3)
  Clock_Delay1ms(50);
  HC12_ReadAllInput();
  UART1_OutString("AT+V\n");    // read firmware
  Clock_Delay1ms(50);
  HC12_ReadAllInput();
  SET = 1;  // exit AT command mode
  Clock_Delay1ms(200);
  HC12_ReadAllInput(); // remove any buffered input
  //************ configuration ended***********************
  printf("\nRF_XMT initialization done\n");
}
/**
 * main.c
 */

void sendTest(void)
{
    int i;
    uint8_t destination = 0x02;
    int totalSize = 26*4;
    struct IOT_TRANSACTION * t = AJP_beginTransmittal(destination, totalSize);
    printf("Successful connection. Begin sending data\r\n");
    for(i = 0; i < 26*4; i++)
    {
        AJP_sendByte(t, 'A'+i%26);
    }
    printf("Data Sent. Awaiting acknowledgement\r\n");
    AJP_endTransmittal(t);
    printf("\r\nFinished!\r\n\r\n");
}

void receiveTest(void)
{
    uint8_t in;
    struct IOT_TRANSACTION * t;
    printf("Awaiting Connection.\r\n");
    t = AJP_listen();

    while(AJP_receiveByte(t,&in) == 0){
        printf("%c\r\n",in);
    }
    Clock_Delay1ms(20);
    AJP_acknowledge(t);
    printf("\r\nFinished!\r\n");

}

void main(void){
  Clock_Init48MHz();        // running on crystal
  Time = MainCount = 0;
  SysTick_Init(480000,2);   // set up SysTick for 100 Hz interrupts
  LaunchPad_Init();         // P1.0 is red LED on LaunchPad
  UART0_Initprintf();       // serial port to PC for debugging
  EnableInterrupts();
  printf("\nAJP Protocol Demonstration\n");
  HC12_Init(UART1_BAUD_9600);

  //char in;

  while(1){
     //WaitForInterrupt();
#ifdef CONSOLE_DEBUG
     printf("\r\n\nPress any key to run test: \r\n");
     UART0_InChar();

#endif

#ifdef IOT_SEND
      sendTest();
#else
      receiveTest();
#endif

     /*
     int i;
     time_t start = time(0);
     for(i = 0; i < 500; i ++){
         UART1_OutChar('a' + i%26);
         in = UART1_InChar();
         if(in != 'a' + i%26)
         {
             if(in == '!')
             {
                 printf("!");
                 // in this case a timeout occurred
                 drop++;

             }
             else
             {
                 error++;
             }
         }
         printf("%d\r\n",i);

     }
     time_t stop = time(0);

     float elapsed = difftime(stop,start);
     UART0_OutChar(in);

     //HC12_ReadForever();

     // foreground thread
      *
      */

    MainCount++;
    }
}


