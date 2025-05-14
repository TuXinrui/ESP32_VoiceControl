#include <SD.h>
#include <SPI.h>
#include <FS.h>

#define SCK_PIN   47  // VSPI的SCK引脚
#define MOSI_PIN  48  // VSPI的MOSI引脚
#define MISO_PIN  45  // VSPI的MISO引脚
#define CS_PIN    21   // VSPI的CS引脚

// 将PCM数据追加到WAV文件，如果文件不存在则创建
void appendPcmToWav (const char* filename, int16_t* pcmData, uint32_t numSamples) {
  SPIClass hspi(HSPI); 
  // 初始化自定义SPI
  hspi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  // 初始化SD卡，使用自定义的SPI实例和CS引脚
  if (!SD.begin(CS_PIN, hspi)) {
    Serial.println("SD卡初始化失败！");
    while (1);
  }
  Serial.println("SD卡初始化成功！");
  const uint32_t sampleRate = 16000;
  const uint16_t numChannels = 1;
  const uint32_t newDataBytes = numSamples * sizeof(int16_t);

  if (SD.exists(filename)) {
    File file = SD.open(filename, FILE_WRITE);
    if (!file) return;

    struct {
      uint8_t riff[4], wave[4], fmt[4], data[4];
    } identifiers;
    
    file.seek(0);
    if (file.read(identifiers.riff, 4) != 4 || memcmp(identifiers.riff, "RIFF", 4) != 0) {
      file.close();
      return;
    }

    file.seek(8);
    if (file.read(identifiers.wave, 4) != 4 || memcmp(identifiers.wave, "WAVE", 4) != 0) {
      file.close();
      return;
    }

    file.seek(12);
    if (file.read(identifiers.fmt, 4) != 4 || memcmp(identifiers.fmt, "fmt ", 4) != 0) {
      file.close();
      return;
    }

    file.seek(36);
    if (file.read(identifiers.data, 4) != 4 || memcmp(identifiers.data, "data", 4) != 0) {
      file.close();
      return;
    }

    // 读取原数据大小
    uint32_t existingDataSize;
    file.seek(40);
    file.read((uint8_t*)&existingDataSize, sizeof(existingDataSize));
    
    // 计算新大小
    uint32_t newRiffSize = 36 + existingDataSize + newDataBytes;
    uint32_t newDataSize = existingDataSize + newDataBytes;

    // 写入新数据
    file.seek(44 + existingDataSize);
    file.write((uint8_t*)pcmData, newDataBytes);

    // 更新头信息
    file.seek(4);
    file.write((uint8_t*)&newRiffSize, sizeof(newRiffSize));
    file.seek(40);
    file.write((uint8_t*)&newDataSize, sizeof(newDataSize));

    file.close();
  } else {
    File file = SD.open(filename, FILE_WRITE);
    if (!file) return;

    // 构造WAV文件头
    uint8_t wavHeader[44] = {'R','I','F','F'};
    
    // RIFF大小 (文件总大小 - 8)
    uint32_t riffSize = 36 + newDataBytes; 
    memcpy(wavHeader+4, &riffSize, 4);
    
    memcpy(wavHeader+8, "WAVEfmt ", 8); // "WAVE" + "fmt "
    
    uint32_t fmtSize = 16;
    memcpy(wavHeader+16, &fmtSize, 4);
    
    uint16_t audioFormat = 1;
    memcpy(wavHeader+20, &audioFormat, 2);
    
    memcpy(wavHeader+22, &numChannels, 2);
    
    memcpy(wavHeader+24, &sampleRate, 4);
    
    uint32_t byteRate = sampleRate * numChannels * sizeof(int16_t);
    memcpy(wavHeader+28, &byteRate, 4);
    
    uint16_t blockAlign = numChannels * sizeof(int16_t);
    memcpy(wavHeader+32, &blockAlign, 2);
    
    uint16_t bitsPerSample = 16;
    memcpy(wavHeader+34, &bitsPerSample, 2);
    
    memcpy(wavHeader+36, "data", 4);
    memcpy(wavHeader+40, &newDataBytes, 4);

    file.write(wavHeader, 44);
    file.write((uint8_t*)pcmData, newDataBytes);
    file.close();
  }
}




