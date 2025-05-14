#include "i2s.h"
#include "serial_wrapper.h"

//int16_t input_audio_buffer[80000];


void Init_i2s_write(void) {
    i2s_config_t i2s_config_write = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
        //.communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = bufferLen,
        .use_apll = false,
        .tx_desc_auto_clear= true, 
        .fixed_mclk=-1    
    };
    
    if(ESP_OK != i2s_driver_install(I2S_NUM_0, &i2s_config_write, 0, NULL)){
      serial_print("speaker install i2s driver failed\n");
      return;
    }
    
    i2s_pin_config_t pin_config_speaker = {
        .bck_io_num = MAX98375_BCLK_IO1,
        .ws_io_num = MAX98375_LRCLK_IO1,
        .data_out_num = MAX98375_DOUT_IO1,
        .data_in_num = I2S_PIN_NO_CHANGE,
    };
    if(ESP_OK != i2s_set_pin(I2S_NUM_0, &pin_config_speaker)){
      serial_print("speaker i2s pin set failed\n");
      return;
    }
    serial_print("speaker i2s install: success\n");
    i2s_set_clk(I2S_NUM_0, 16000, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
    i2s_start(I2S_NUM_0);
}

void Init_i2s_read(void) {
    i2s_config_t i2s_config_read = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
        //.communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = bufferLen,
        .use_apll = false,   
    };
    
    if(ESP_OK != i2s_driver_install(I2S_NUM_1, &i2s_config_read, 0, NULL)){
      serial_print("microphone install i2s driver failed\n");
      return;
    }
    
    i2s_pin_config_t pin_config_mic = {
        .bck_io_num = INMP441_BCLK_IO1,
        .ws_io_num = INMP441_LRCLK_IO1,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = INMP441_DIN_IO1,
    };
    if(ESP_OK != i2s_set_pin(I2S_NUM_1, &pin_config_mic)){
      serial_print("microphone i2s pin set failed\n");
      return;
    }
    serial_print("\nmicrophone i2s install: success\n");
    //i2s_set_clk(I2S_NUM_0, 16000, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
    i2s_start(I2S_NUM_1);
}

int16_t *record_audio(void) {
    size_t total_samples = 16000 * RECORD_DURATION;
    size_t buffer_size = total_samples * sizeof(int16_t);  // 16 位数据
    serial_printsize(buffer_size);
    serial_print("\n");
    int16_t *audio_buffer = (int16_t*) malloc(buffer_size);  // 动态分配内存
    if (!audio_buffer) {
      serial_print("内存分配失败！\n");
      return NULL;
    }
    size_t bytes_read;
    size_t total_bytes = 0;
    int16_t *ptr = audio_buffer;
    // 获取开始时间戳
    TickType_t start_time = xTaskGetTickCount();
    while (1) {
        size_t remaining_bytes = buffer_size - total_bytes;
        if (remaining_bytes == 0) {
            break;  // 缓冲区已满
        }
        size_t read_len = (bufferLen < remaining_bytes) ? bufferLen : remaining_bytes;
        // 从 I2S 读取数据
        i2s_read(I2S_NUM_1, ptr, read_len , &bytes_read, portMAX_DELAY);

        ptr += bytes_read / sizeof(int16_t);  // 移动缓冲区指针
        total_bytes += bytes_read;
        const TickType_t max_duration_ticks = pdMS_TO_TICKS(RECORD_DURATION * 1000);

        // 检查是否达到 3 秒
        TickType_t elapsed_time = (xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS / 1000;
        if (elapsed_time >= RECORD_DURATION) {
            break;
        }
        if (xTaskGetTickCount() - start_time >= max_duration_ticks) {
            break;
        }
        // 短延防止任务饥饿
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    serial_print("录音完成！总数据量(字节):");
    serial_printsize(total_bytes);
    delay(500);
    serial_print("\n");
    //audio_play(audio_buffer);
    return(audio_buffer);
}

/*void audio_play(int16_t *audio_buffer){
  int tmp_len = 
}*/

/*void i2s_audio_play(const void *src, size_t size, TickType_t timeout) {
    size_t bytes_written = 0;
    i2s_write(I2S_NUM_0, src, size, &bytes_written, timeout);
}*/