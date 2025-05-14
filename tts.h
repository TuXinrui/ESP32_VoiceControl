#ifndef TTS_H
#define TTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_tts.h"
#include "esp_tts_voice_xiaole.h"
#include "i2s.h"
#include "esp_partition.h"
#include "esp_spi_flash.h"
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "serial_wrapper.h"

void Init_tts(void);
void audio_output(const char* text);


#ifdef __cplusplus
}
#endif

#endif