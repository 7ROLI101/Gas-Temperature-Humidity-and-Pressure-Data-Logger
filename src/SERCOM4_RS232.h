#ifndef SERCOM4_RS232_H_
#define SERCOM4_RS232_H_
void UART_init(void);
void UART_write(char data);
void delayMs(int n);
char UART_read(void);
#endif 

