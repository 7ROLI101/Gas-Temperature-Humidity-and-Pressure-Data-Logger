/*

 * File Name: SERCOM4_echo_rs232.c
 *
 * Date: 3/26/2020
 * Author : Aaron Varghese
 * Version 1.0
 * Target: ATSAML21J18B
 * Target Hardware: SAML21 XPlained PRO
 
 Description:
 This program is to test out the RS232
 capabilities of the ATSAML21J18B.
 PB08------->Tx
 PB09------->Rx
 */ 


#include "saml21j18b.h"
unsigned char* ARRAY_PINCFG1 = (unsigned char*) &REG_PORT_PINCFG1;
unsigned char* ARRAY_PMUX1 = (unsigned char*) &REG_PORT_PMUX1;


/************************
NAME:       UART_init()
ASSUMES:    PB08 and PB09 are not being used
SERCOM4 is being used for the RS232 communication
 PB08------->Tx
 PB09------->Rx

RETURNS:    N/A
MODIFIES:   N/A
DESCRITION: initializes SERCOM4 for RS232 communication.
********************************************************************/
void UART_init(void)
{
	//Clock setup for the SAML21J18B
	//REG_MCLK_AHBMASK |= 0x04;  //APBC bus enabled by default
	//REG_MCLK_APBCMASK |= 0x10; //enabling the APBC bus clock
	//for the SERCOM4

	//Generic Clock Generator 0 will be used as the source of
	//the SERCOM4 core clock
	REG_GCLK_PCHCTRL22 |= 0x40;

	//configure port pins Tx & Rx
	//enable the PMUX
	ARRAY_PINCFG1[8] |= 1;
	ARRAY_PINCFG1[9] |= 1;
	//PB08--->PAD[0] of SERCOM4
	//PB09--->PAD[1] of SERCOM4
	ARRAY_PMUX1[4] = 0x33;

	//Configure the SERCOM4
	REG_SERCOM4_USART_CTRLA |= 1;
	//wait for reset to complete
	while((REG_SERCOM4_USART_SYNCBUSY&1)){}
	//LSB first, async, no parity, PAD[1]->Rx, PAD[0]->Tx, BAUD
	//uses fraction, 8x oversampling, internal clock
	REG_SERCOM4_USART_CTRLA |= 0x40106004;

	//enable Tx, Rx, one stop bit, 8 bit
	REG_SERCOM4_USART_CTRLB = 0x30000;

	// (4MHz)/(8*9600) = 52.08
	REG_SERCOM4_USART_BAUD = 52;
	//enable the SERCOM4 peripheral
	REG_SERCOM4_USART_CTRLA |= 2;

	//waiting for enable to complete
	while(REG_SERCOM4_USART_SYNCBUSY&2){}
}

/************************
NAME:       UART_write()
ASSUMES:    N/A
RETURNS:    N/A
MODIFIES:   SERCOM4's data register
DESCRITION: transfers a character out to the RS232 communication line
********************************************************************/
void UART_write(char data)
{
	//waiting for the the data register to be empty
	while(!(REG_SERCOM4_USART_INTFLAG&1)){};
	REG_SERCOM4_USART_DATA = data;
}

/************************
NAME:       UART_read()
ASSUMES:    N/A
RETURNS:    N/A
MODIFIES:   N/A
DESCRITION: reads in a character from the RS232 communication line
********************************************************************/
char UART_read(void)
{
	//waiting for transfer of incoming data to be fully sent
	while(!(REG_SERCOM4_USART_INTFLAG&4)){};
	//read the value of the data sent
	return REG_SERCOM4_USART_DATA;
}

/************************
NAME:       delayMs()
ASSUMES:    an integer n is given in the parameter
RETURNS:    N/A
MODIFIES:   N/A
DESCRITION: sets an n Ms delay
********************************************************************/
void delayMs(int n)
{
	for(;n>0;n--)
	{
		for(int i=0;i<199;i++)
		{
			__asm("nop");
		}
	}
}
