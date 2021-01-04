/*
 * final_program.c
 *
 * Created: 5/2/2020 3:40:08 PM
 * Author : Aaron
 This program uses the BME680 sensor
 to output the temperature, pressure, 
 humidity and gas resistance measurements.
 For this program, we will be using the 
 BME680 sensor API that is given by Bosch to 
 help make the calculations and everything 
 easier to use and follow.This code is also 
 supposed to output T,H, and P before the
 pushbutton press, and then H,P,G after the
 pushbutton press.
 */ 


#include "saml21j18b.h"
#include "lcd_dog_driver.h"
#include "SERCOM4_RS232.h"
#include "bme680.h"
#include "stdio.h"
#include "console_io_support.h"
#include "stdint.h"

unsigned char * ARRAY_PINCFG0 = (unsigned char*) & REG_PORT_PINCFG0;
unsigned char * ARRAY_PMUX0 = (unsigned char*) & REG_PORT_PMUX0;

//bme680 needed functions
void init_spi_bme680(void);
void user_delay_ms(uint32_t period);
uint8_t spi_transfer(uint8_t data);
int8_t user_spi_read(uint8_t dev_id, uint8_t reg_addr, uint8_t* reg_data, uint16_t len);
int8_t user_spi_write(uint8_t dev_id, uint8_t reg_addr, uint8_t* reg_data, uint16_t len);

//switching screens
void pushbuttonpress(_Bool *point, struct bme680_field_data data);
void tph(struct bme680_field_data data);
void phg(struct bme680_field_data data);




int main(void)
{
	UART_init();
	init_lcd_dog();
	init_spi_bme680();
	
	//initializing the bme680 device structure
	struct bme680_dev sensor;
	//sensor.dev_id = 0;
	sensor.intf = BME680_SPI_INTF;
	sensor.read = user_spi_read;
	sensor.write = user_spi_write;
	sensor.delay_ms = user_delay_ms;
	sensor.amb_temp = 25;
	int8_t rslt = BME680_OK;
	rslt = bme680_init(&sensor);
	
	uint8_t set_required_settings;
	
	//setting temp, pressure and humidity settings
 	sensor.tph_sett.os_hum = BME680_OS_2X;
	sensor.tph_sett.os_pres = BME680_OS_4X;
	sensor.tph_sett.os_temp = BME680_OS_8X;
	sensor.tph_sett.filter = BME680_FILTER_SIZE_3;
	
	//setting the remaining gas sensor settings and link
	//the heating profile
	sensor.gas_sett.run_gas = BME680_ENABLE_GAS_MEAS;
	//creating a ramp heat waveform in 3 steps
	sensor.gas_sett.heatr_temp = 200;//degrees Celsius
	sensor.gas_sett.heatr_dur = 85; //milliseconds
	
	//selecting power mode
	sensor.power_mode = BME680_FORCED_MODE;
	
	//setting the required sensor settings needed
	set_required_settings = (BME680_OST_SEL)|(BME680_OSP_SEL)|
	(BME680_OSH_SEL)|(BME680_FILTER_SEL)|(BME680_GAS_SENSOR_SEL);
	
	//setting the desired sensor settings
	rslt = bme680_set_sensor_settings(set_required_settings,&sensor);
	//set the power mode
	rslt = bme680_set_sensor_mode(&sensor);
	
	//getting the measurement duration needed so it the sensor 
	//could sleep or wait until the measurement is complete 
	 uint16_t meas_period;
	 bme680_get_profile_dur(&meas_period,&sensor);

	 //state = 0 will be tph code, state = 1 will be phg code
	 _Bool state = 0;
	 //says  if pb is pressed
	 _Bool pbpress = 0;
	 _Bool* point = &state;
	 
	 REG_PORT_DIR0 &= ~(0x04);
	 ARRAY_PINCFG0[2] |=6;
	 REG_PORT_OUT0 = 0x04;
	
	 struct bme680_field_data data;
    

	while (1) 
    {
		//wait until the measurement is ready
		user_delay_ms(meas_period);
		rslt = bme680_get_sensor_data(&data, &sensor);
		printf("T: %.2f degC, P: %.2f hPa, H %.2f %%rH ", data.temperature / 100.0f,
		data.pressure / 100.0f, data.humidity / 1000.0f );
		
		//avoid using measurements from an unstable heating setup
		if((data.status & BME680_GASM_VALID_MSK))
		{
			printf(", G: %ld ohms", data.gas_resistance);
		}
		
		printf("\r\n");
		


		//this is the code for outputting to the LCD
		//PA02 is used for the pushbutton press
		if(!(REG_PORT_IN0&0x04))
		//if it is pressed, then wait for it to be released
		{
			//while it is still pressed, then just output current state
			//the current state will be taken, and then the next state will 
			//be calculated assuming that the pushbutton is still being held
			while(!(REG_PORT_IN0&0x04))
			{
				//in case of state 0, output tph
				if(state==0)
				{
					tph(data);
					init_spi_bme680();
					// Trigger the next measurement if you would like to read data out
					// continuously
					if(sensor.power_mode == BME680_FORCED_MODE)
					{
						rslt = bme680_set_sensor_mode(&sensor);					}
					user_delay_ms(meas_period);
					rslt = bme680_get_sensor_data(&data, &sensor);
					printf("T: %.2f degC, P: %.2f hPa, H %.2f %%rH ", data.temperature / 100.0f,
					data.pressure / 100.0f, data.humidity / 1000.0f );
		
					//avoid using measurements from an unstable heating setup
					if((data.status & BME680_GASM_VALID_MSK))
					{
					printf(", G: %ld ohms", data.gas_resistance);
					}
					printf("\r\n");
				}
				
				//in case of state 1, output phg
				else if (state == 1)
				{
					phg(data);
					init_spi_bme680();
					// Trigger the next measurement if you would like to read data out
					// continuously
					if(sensor.power_mode == BME680_FORCED_MODE)
					{
					rslt = bme680_set_sensor_mode(&sensor);
					}
					user_delay_ms(meas_period);
					rslt = bme680_get_sensor_data(&data, &sensor);
					printf("T: %.2f degC, P: %.2f hPa, H %.2f %%rH ", data.temperature / 100.0f,
					data.pressure / 100.0f, data.humidity / 1000.0f );
					//avoid using measurements from an unstable heating setup
					if((data.status & BME680_GASM_VALID_MSK))
					{
					printf(", G: %ld ohms", data.gas_resistance);
					}
					printf("\r\n");
				}
				
			}
			//if it is released, update pbpress and change state
			if((REG_PORT_IN0&0x04))
			{
				pbpress=1;
				pushbuttonpress(point,data);
				pbpress = 0;
			}
		}
		
		//check which state is currently on and then move on to 
		//calculate the next result
		if(state==0)
		{
			//state 0 is task 1 code
			tph(data);
		}
		else
		{
			//state 1 is task 2 code
			phg(data);
		}

		init_spi_bme680();
		// Trigger the next measurement if you would like to read data out
		// continuously
		if(sensor.power_mode == BME680_FORCED_MODE)
		{
			rslt = bme680_set_sensor_mode(&sensor);		}	
    }
}





//Functions for the BME680 sensor
/************************
NAME:       init_spi_bme680
ASSUMES:    The BME680 sensor is interfaced by SPI through SERCOM1
SERCOM1 is being used for the SPI data transfer
PA16 (PAD[0])---> SDI
PA17 (PAD[1])---> SCK
PA19 (PAD[3])--->SD0
PB07 ---->/CSB

RETURNS:    N/A
MODIFIES:   N/A
CALLED BY:
DESCRITION: init SPI port for communication with the BME680
********************************************************************/
void init_spi_bme680(void)
{
	//this initializes the SERCOM SPI unit
	//REG_MCLK_AHBMASK |= 0x04; //APBC bus enabled by default
	//REG_MCLK_APBCMASK|= 0x02; //SERCOM1 APBC bus clock enabled
	//by default
	//using generic clock generator 0 (4 MHz) for peripheral clock
	REG_GCLK_PCHCTRL19 = 0x40;// enabling SERCOM1 core clock

	ARRAY_PINCFG0[16] |= 1; //setting PMUX config
	ARRAY_PINCFG0[17] |= 1; //setting PMUX config
	ARRAY_PINCFG0[19] |= 1; //setting PMUX config
	ARRAY_PMUX0[8] = 0x22; //PA16 = SDO, PA17 = SCK
	ARRAY_PMUX0[9] = 0x20; //PA19 = SDI

	REG_SERCOM1_SPI_CTRLA = 1; //software reset SERCOM1
	while(REG_SERCOM1_SPI_CTRLA & 1){};//waiting for reset to finish
	//SDI = PAD[0], SDO = PAD[3], SCK = PAD[1], using PB07 for /CSB
	REG_SERCOM1_SPI_CTRLA = 0x3030000C;//CPOL and CPHA are 11
	REG_SERCOM1_SPI_CTRLB = 0x020000; //8-bit data
	//SPI clock is going to be written to 2MHz
	REG_SERCOM1_SPI_BAUD = 0;
	REG_SERCOM1_SPI_CTRLA|=2; //SERCOM1 enabled

	//setting up /CSB, and turning it off
	REG_PORT_DIR1 |=128;
	REG_PORT_OUTSET1 = 128;
}



/************************
NAME:       user_delay_ms()
ASSUMES:    An integer value is given that specifies amount of ms
delay that is needed

RETURNS:    N/A
MODIFIES:   N/A
CALLED BY:  N/A
DESCRITION: delay that lasts (uint32_t) period ms long
********************************************************************/
void user_delay_ms(uint32_t period)
{
	for(;period>0;period--)
	{
		for(int i=0;i<199;i++)
		{
			__asm("nop");
		}
	}
}



/************************
NAME:       spi_transfer()
ASSUMES:    1) The BME680 sensor is interfaced by SPI through SERCOM1
SERCOM1 is being used for the SPI data transfer.
2) Also assumes that data is given as a parameter to transfer


RETURNS:    N/A
MODIFIES:   N/A
CALLED BY: spi_read_BME680, spi_write_BME680
DESCRITION: transfers data between the sensor and SERCOM1
********************************************************************/
uint8_t spi_transfer(uint8_t data)
{
	while(!(REG_SERCOM1_SPI_INTFLAG&0x01)){}
	REG_SERCOM1_SPI_DATA = data;
	while(!(REG_SERCOM1_SPI_INTFLAG&0x04)){}
	return REG_SERCOM1_SPI_DATA;
}



/************************
NAME:       user_spi_read()
ASSUMES:
1) Address of register to read from is given
2) Chip ID is given (no need to do anything for it in our case)
3) Pointer to data is given
4) Amount of bytes needed to be written sequentially is given

RETURNS:    data in the address and rslt
MODIFIES:   N/A
CALLED BY:  N/A
DESCRITION: Returns a result that says if the read function was
done successfully
********************************************************************/
int8_t user_spi_read(uint8_t dev_id, uint8_t reg_addr, uint8_t* reg_data, uint16_t len)
{
	//operation is done successfully
	int8_t rslt = 0;
	
	//enabling the BME680
	REG_PORT_OUTCLR1 = 128;
	
	//control byte for the transfer
	spi_transfer(reg_addr|(1<<7));	
	//this will read through each address and increment automatically
	for(uint16_t i = 0;i<len;i++)
	{
		*reg_data = spi_transfer(0x00);
		reg_data++;
	}
	//disabling the BME680
	REG_PORT_OUTSET1 = 128;
	return rslt;
}
//
/************************
NAME:       user_spi_write()
ASSUMES:
1) Address of register to write to is given
2) Chip ID is given (no need to do anything for it in our case)
3) Pointer to data is given
4) Amount of bytes needed to be written sequentially is given

RETURNS:    rslt, which says that the operation is successful
MODIFIES:   N/A
CALLED BY:  N/A
DESCRITION: writes specified data to a specific register
********************************************************************/
int8_t user_spi_write(uint8_t dev_id, uint8_t reg_addr, uint8_t* reg_data, uint16_t len)
{
	//operation is done successfully
	int8_t rslt =0;
	
	//enabling the BME680
	REG_PORT_OUTCLR1 = 128;
	
	spi_transfer(reg_addr);
	for(uint8_t i = 0;i<len;i++)
		{
			spi_transfer(*reg_data);
			reg_data++;
		}
	//disabling the BME680
	REG_PORT_OUTSET1 = 128;
	return rslt;
}


//Functions to output the TPH and PHG to separate pages 
//when pushbutton is pressed
/************************
NAME:       tph()
ASSUMES:

RETURNS:    N/A
MODIFIES:   N/A
CALLED BY:  N/A
DESCRITION: When a pushbutton is pressed, the screen will change 
the output to temperature, pressure and humidity
********************************************************************/
void tph(struct bme680_field_data data)
{
	sprintf(dsp_buff_1,"T:%.2f %cC    ",data.temperature / 100.0f,0xDF);
	sprintf(dsp_buff_2, "P:%0.2f hPa   ",data.pressure / 100.0f);
	sprintf(dsp_buff_3, "H:%.2f %%rH   ",data.humidity / 1000.0f);
	update_lcd_dog();
}



/************************
NAME:       phg()
ASSUMES:

RETURNS:    N/A
MODIFIES:   N/A
CALLED BY:  N/A
DESCRITION: When a pushbutton is pressed, the screen will change
the output to pressure, humidity and gas 
********************************************************************/
void phg(struct bme680_field_data data)
{
	sprintf(dsp_buff_1, "P:%0.2f hPa   ",data.pressure / 100.0f);
	sprintf(dsp_buff_2,"H:%.2f %%rH    ",data.humidity / 1000.0f);
	sprintf(dsp_buff_3, "G:%ld ohms    ", data.gas_resistance);
	update_lcd_dog();
}


/***********************
NAME:        pushbuttonpress()
ASSUMES:    
RETURNS:     nothing
MODIFIES:    N/A
CALLED BY:   main()
DESCRIPTION: Change the current state and update the screen with
current state values.
********************************************************************/
void pushbuttonpress(_Bool *point,struct bme680_field_data data)
{
	//if state 0 is selected, switch to state 1
	if(*point == 0)
	{
		*point = 1;
		phg(data);
	}
	//else, if state 1 is selected, switch to state 0
	else
	{
		*point = 0;
		tph(data);
	}
}
