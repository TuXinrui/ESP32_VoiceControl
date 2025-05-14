#ifndef SERIAL_WRAPPER_H
#define SERIAL_WRAPPER_H

#include <Arduino.h>

void serial_print(const char* message);
void serial_printHEX(unsigned char message);
void serial_printd(short a);
void serial_printp(void *ptr);
void serial_printu(uint32_t a);
void serial_printsize(size_t a);
#endif