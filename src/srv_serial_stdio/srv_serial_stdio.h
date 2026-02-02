#ifndef SRV_SERIAL_STDIO_H
#define SRV_SERIAL_STDIO_H

#include <stdio.h>

int srv_serial_put_char(char ch, FILE *f);
int srv_serial_get_char(FILE *f);

void srv_serial_stdio_setup();

#endif // SRV_SERIAL_STDIO_H