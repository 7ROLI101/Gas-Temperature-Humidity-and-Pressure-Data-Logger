/***********************************************************************
  
   * File Name: lcd_dog_driver.c
   *
   * Date: 3/26/2020 
   * Author : Aaron Varghese
   * Version 1.0
   * Target: ATSAML21J18B
   * Target Hardware: SAML21 XPlained PRO
   
   This driver contains procedures to initialize and update
   DOG text based LCD display modules, including the EA DOG163W-A LCD
   modules configured with three (3) 16 characters display lines.

   The display module hardware interface uses a 1-direction, write only
   SPI interface. (See below for more information.)

   The display module software interface uses three (3) 16-byte
   data (RAM) based display buffers - One for each line of the display.
   (See below for more information.)
*/

/*look at page 43 of the EA DOG163W-A LCD data sheet for details on the 
schematic for SPI data transfer and the initialization process of the 
LCD screen

SERCOM1 is being used for the SPI data transfer
PA16 (PAD[0])---> MOSI
PA17 (PAD[1])---> SCK
PA18 (PAD[2])---> /SS
PA19 (PAD[3])--->MISO //NOT USED, since the LCD is write only

PB06 --->/RS
*/

#include "saml21j18b.h"
//for each of the lines in the LCD screen
unsigned char* ARRAY_PINCFG0 =(unsigned char*) &REG_PORT_PINCFG0;
unsigned char* ARRAY_PMUX0 = (unsigned char*) &REG_PORT_PMUX0;
char dsp_buff_1[17];
char dsp_buff_2[17];
char dsp_buff_3[17];

/*************************
NAME:        delay_30uS
ASSUMES:     nothing
CALLED BY:   init_dsp
DESCRIPTION: This procedure will generate a fixed delay of just over
             30 uS (assuming a 4 MHz clock).
********************************************************************/
void delay_30us(void)
{
	for(int i= 40;i>0;i--){ __asm("nop");	}
};

/*********************
NAME:        v_delay
ASSUMES:     Integers a and b= initial count values defining how many
             30uS delays will be called. This procedure can generate
             short delays (a = small #) or much longer delays (where
             b value is large).
RETURNS:     nothing
CALLED BY:   init_dsp, plus...
DESCRIPTION: This procedure will generate a variable delay for a fixed
             period of time based the values pasted in a and b.

a is the inner loop value, and b is the outer loop value

**********************************************************************/
void v_delay(int a,int b)
{
	for(;a>0;a--)
	{
		for(;b>0;b--){}
	}
};

/***********************
NAME:        delay_40mS
ASSUMES:     nothing
RETURNS:     nothing
MODIFIES:    N/A
CALLED BY:   init_dsp
DESCRIPTION: This procedure will generate a fixed delay of 
             40 mS.
********************************************************************/
void delay_40mS(void)
{
	v_delay(700,15000);
};

/************************
NAME:       init_spi_lcd
ASSUMES:    The LCD module is interfaced by SPI through SERCOM1
SERCOM1 is being used for the SPI data transfer
PA16 (PAD[0])---> MOSI
PA17 (PAD[1])---> SCK
PA18 (PAD[2])---> /SS
PA19 (PAD[3])--->MISO

PB06 ----> /RS

RETURNS:    N/A
MODIFIES:   N/A
CALLED BY:  init_dsp, update
DESCRITION: init SPI port for command and data writes to LCD via SPI
********************************************************************/
void init_spi_lcd(void)
{
//this initializes the SERCOM SPI unit
//REG_MCLK_AHBMASK |= 0x04; //APBC bus enabled by default
//REG_MCLK_APBCMASK|= 0x02; //SERCOM1 APBC bus clock enabled
//by default
//using generic clock generator 0 (4 MHz) for peripheral clock
REG_GCLK_PCHCTRL19 = 0x40;// enabling SERCOM1 core clock

ARRAY_PINCFG0[16] |= 1; //setting PMUX config
ARRAY_PINCFG0[17] |= 1; //setting PMUX config
ARRAY_PINCFG0[18] |= 1; //setting PMUX config
ARRAY_PINCFG0[19] |= 1; //setting PMUX config
ARRAY_PMUX0[8] = 0x22; //PA16 = MOSI, PA17 = SCK
ARRAY_PMUX0[9] = 0x22; //PA18 = SS, PA19 = MISO

REG_SERCOM1_SPI_CTRLA = 1; //software reset SERCOM1
while(REG_SERCOM1_SPI_CTRLA & 1){};//waiting for reset to finish
//MISO = PAD[3], MOSI = PAD[0], SCK = PAD[1], SS = PAD[2], SPI master
REG_SERCOM1_SPI_CTRLA = 0x3030000C;//CPOL and CPHA are 11
REG_SERCOM1_SPI_CTRLB = 0x2000; //master SS, 8-bit data
//SPI clock should be a maximum of 3.125 MHz 
//SPI clock is going to be written to 2MHz
REG_SERCOM1_SPI_BAUD = 0;
REG_SERCOM1_SPI_CTRLA|=2; //SERCOM1 enabled

//setting up /RS, and turning it off
REG_PORT_DIR1 |=64;
REG_PORT_OUT1 |=64;

};

/*********************************
NAME:       lcd_spi_transmit_CMD
ASSUMES:    command = byte for LCD.
            SPI port are already configured.
RETURNS:    N/A
MODIFIES:   N/A
CALLED BY:  init_dsp, update
DESCRIPTION: outputs a byte passed in r16 via SPI port. Waits for data
            to be written by SPI port before continuing.
*********************************************************************/
void lcd_spi_transmit_CMD(char command)
{
	REG_PORT_OUTCLR1 |=64;//clearing /RS --> command
	while(!(REG_SERCOM1_SPI_INTFLAG&1)) {}//wait until transmission is done
	REG_SERCOM1_SPI_DATA = command;
	while(!(REG_SERCOM1_SPI_INTFLAG&1)) {}//wait until transmission is done

};

/*********************************
NAME:       lcd_spi_transmit_DATA
ASSUMES:    data = byte to transmit to LCD.
            SPI port is configured.
RETURNS:    N/A
MODIFIES:   N/A
CALLED BY:  init_dsp, update
DESCRIPTION: outputs a byte passed in r16 via SPI port. Waits for
            data to be written by SPI port before continuing.
*****************************************************************/
void lcd_spi_transmit_DATA(char data)
{
	REG_PORT_OUTSET1 |=64; //setting /RS --> data
	while(!(REG_SERCOM1_SPI_INTFLAG&1)) {}//wait until transmission is done
	REG_SERCOM1_SPI_DATA = data;
	while(!(REG_SERCOM1_SPI_INTFLAG&1)) {}//wait until transmission is done
};

/************************
NAME:       init_lcd_dog
ASSUMES:    nothing
RETURNS:    nothing
MODIFIES:   R16, R17
CALLED BY:  main application
DESCRITION: inits DOG module LCD display for SPI (serial) operation.
NOTE:  Can be used as is with MCU clock speeds of 4MHz or less.
********************************************************************/
void init_lcd_dog(void)
{
	//initialize the LCD DOG SPI protocol for the SAML21J18B
	init_spi_lcd();

	//delay of 40ms so VDD is stable
	delay_40mS();
	
	//function set 1
	lcd_spi_transmit_CMD(0x39);
	delay_30us();
	
	//function set 2
	lcd_spi_transmit_CMD(0x39);
	delay_30us(); 
	
	//setting the bias value/internal osc frequency
	lcd_spi_transmit_CMD(0x1E);
	delay_30us();
	
	//contrast set
	//~77 for 5V
	//~7F for 3.3V
	lcd_spi_transmit_CMD(0x7F);
	delay_30us();
	
	//POWER/ICON/CONTRAST control
	//0x50 nominal for 5V
	//0x55 for 3.3V
	lcd_spi_transmit_CMD(0x55);
	delay_30us();
	
	//follower control
	lcd_spi_transmit_CMD(0x6C);
	//delay 200ms
	delay_40mS();
	delay_40mS();
	delay_40mS();
	delay_40mS();
	delay_40mS();
	
	//display ON/OFF control
	
	lcd_spi_transmit_CMD(0x0C);//display on, cursor off, blink off
	delay_30us();
	
	
	lcd_spi_transmit_CMD(0x01);//clears display, cursor home
	delay_30us();
	
	//entry mode
	lcd_spi_transmit_CMD(0x06);//clear display
	delay_30us();
};

/**************************
NAME:       update_lcd_dog
ASSUMES:    display buffers loaded with display data
RETURNS:    N/A
MODIFIES:   N/A

DESCRIPTION: Updates the LCD display lines 1, 2, and 3, using the
  contents of dsp_buff_1, dsp_buff_2, and dsp_buff_3, respectively.
*******************************************************************/
void update_lcd_dog(void)
{
	//initializing the SPI port for the LCD
	init_spi_lcd();
	
//sending line 1 to the LCD module
	
	//initialize the DDRAM address counter to the first line(00H-0FH)
	lcd_spi_transmit_CMD(0x80);
	delay_30us();
	//putting in the data into dsp_buff_1
	for(int i=0;i<16;i++)
	{
		lcd_spi_transmit_DATA(dsp_buff_1[i]);
		delay_30us();	
	}

//sending line 2 to the LCD module

//initialize the DDRAM address counter to the second line(10H-1FH)
lcd_spi_transmit_CMD(0x90);
delay_30us();
//putting in the data from dsp_buff_2
for(int i=0;i<16;i++)
{
	lcd_spi_transmit_DATA(dsp_buff_2[i]);
	delay_30us();
}	

//sending line 3 to the LCD module

//initialize the DDRAM address counter to the third line(20H-2FH)
lcd_spi_transmit_CMD(0xA0);
delay_30us();
//putting in the data into dsp_buff_1
for(int i=0;i<16;i++)
{
	lcd_spi_transmit_DATA(dsp_buff_3[i]);
	delay_30us();
}
};

