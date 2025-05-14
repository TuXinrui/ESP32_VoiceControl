#include "tts.h"
#include "serial_wrapper.h"

static esp_tts_handle_t *tts_handle = NULL;
static spi_flash_mmap_handle_t mmap_handle;

void Init_tts(void) {
    const esp_partition_t* part = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, 
        ESP_PARTITION_SUBTYPE_DATA_FAT, 
        "voice_data"
    );

    if (!part) {
        serial_print("tts error:partition failed\n");
        while(1) {  delay(1000);/* 错误处理 */ }
    }

    uint16_t* voicedata;
    esp_err_t err = esp_partition_mmap(
        part, 
        0, 
        part->size, 
        ESP_PARTITION_MMAP_DATA, 
        (const void**)&voicedata, 
        &mmap_handle
    );

    if(err != ESP_OK){
      serial_print("tts error:map to voice_data partition failed\n");
      while(1) {  delay(1000);/* 错误处理 */ }
    }

    esp_tts_voice_t *voice = esp_tts_voice_set_init(
        &esp_tts_voice_xiaole, 
        voicedata
    );
    
    tts_handle = esp_tts_create(voice);

    if (!tts_handle){
        serial_print("tts error:tts handle failed\n");
      while(1) {  delay(1000);/* 错误处理 */ }
    }
    serial_print("\ntts success\n");

    serial_print("Voice data address: ");
    serial_printp(voicedata);
    serial_print("\nVoice handle: ");
    serial_printp(tts_handle);
    serial_print("\n");
}

void audio_output(const char* text) {
    serial_print("Free heap before TTS: ");
    serial_printu(xPortGetFreeHeapSize());
    serial_print("\n");
    
    serial_print("Parsing text: ");
    serial_print(text);
    serial_print("\n");
    
    int parse_result = esp_tts_parse_chinese(tts_handle, text);
    serial_print("Parse result: ");
    serial_printd(parse_result);
    serial_print("\n");
    
    serial_print("Free heap after parse: ");
    serial_printu(xPortGetFreeHeapSize());
    serial_print("\n");
    
    if (parse_result) {
        int len[1] = {0};
        size_t bytes_written = 0;
        do {
            short *data = esp_tts_stream_play(tts_handle, len, 0);
            serial_print("dataptr:");
            serial_printp(data);
            serial_print(" len:");
            serial_printd(len[0]);
            serial_print("\n");
            i2s_write(I2S_NUM_0, data, len[0] * 2, &bytes_written, portMAX_DELAY);
            serial_print("Wave Data: ");
            for(int i=0; i<len[0]; i++){
                data[i] = data[i] * 1000;
                serial_printd(data[i]);
                serial_print(", ");
            }
            serial_print("\n");
        } while(len[0] > 0);
        i2s_zero_dma_buffer(0);
        serial_print("\n");
    } else {
        serial_print("Failed to parse Chinese text\n");
    }
    esp_tts_stream_reset(tts_handle);
    
    serial_print("Free heap after stream play: ");
    serial_printu(xPortGetFreeHeapSize());
    serial_print("\n");
}