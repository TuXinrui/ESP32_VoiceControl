#ifndef I2S_H
#define I2S_H

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/i2s.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>
#include "serial_wrapper.h"

#define MAX98375_BCLK_IO1 8
#define MAX98375_LRCLK_IO1 19
#define MAX98375_DOUT_IO1 18

#define INMP441_BCLK_IO1 1
#define INMP441_LRCLK_IO1 2
#define INMP441_DIN_IO1 42

#define bufferLen 512
#define RECORD_DURATION 3 


void Init_i2s_write(void);
void Init_i2s_read(void);
int16_t* record_audio(void);



//void i2s_audio_play(const void *src, size_t size, TickType_t timeout);




#ifdef __cplusplus
}
#endif

#endif