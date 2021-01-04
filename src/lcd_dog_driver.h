#ifndef LCD_DOG_DRIVER_H_
#define LCD_DOG_DRIVER_H_

char dsp_buff_1[17], dsp_buff_2[17],dsp_buff_3[17];

void delay_30us(void);

void v_delay(int a,int b);

void delay_40mS(void);

void init_spi_lcd(void);

void lcd_spi_transmit_CMD(char command);

void lcd_spi_transmit_DATA(char data);

void init_lcd_dog(void);

void update_lcd_dog(void);


#endif




