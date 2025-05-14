#include <Arduino.h>

extern "C" {
  void serial_print(const char* message) {
    Serial.print(message);
  }
  void serial_printHEX(unsigned char message){
    Serial.print(message, HEX);
  }
  void serial_printd(short a){
    Serial.printf("%d",a);
  }
  void serial_printp(void *ptr){
    Serial.printf("%p",ptr);
  }
  void serial_printu(uint32_t a){
    Serial.printf("%u",a);
  }
  void serial_printsize(size_t a){
    Serial.printf("%d",a);
  }
}