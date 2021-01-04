#ifndef CONSOLE_IO_SUPPORT_H_
#define CONSOLE_IO_SUPPORT_H_

#include <stdio.h>
#include "SERCOM4_RS232.h"

int _write(FILE *f,char *buf, int n);

int _read(FILE *f, char *buf, int n);

int _close(FILE *f);

int _fstat(FILE *f, void *p);

int _isatty(FILE *f);

int _lseek(FILE *f, int o, int w);

void* _sbrk(int i);

#endif

