#include <stdio.h>
#include "SERCOM4_RS232.h"

int _write(FILE *f,char *buf, int n)
{
	int m = n;
	for(; n>0 ; n--)
	{
		UART_write(*buf++);
	}
	return m;
}

int _read(FILE *f, char *buf, int n)
{
	*buf = UART_read();
	if (*buf == '\r')
	{
		*buf = '\n';
		_write(f, "\r", 1);
	}
	_write(f, buf, 1);
	return 1;
}

int _close(FILE *f)
{
	return 0;
}

int _fstat(FILE *f, void *p)
{
	*((int *)p+4) = 0x81b6; //enable read/write
	return 0;
}

int _isatty(FILE *f)
{
	return 1;
}

int _lseek(FILE *f, int o, int w)
{
	return 0;
}

void* _sbrk(int i)
{
	return (void*) 0x20006000;
}
