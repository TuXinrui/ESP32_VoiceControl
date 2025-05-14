#include "sd_access.h"


#define SCK_PIN   47  // VSPI的SCK引脚
#define MOSI_PIN  48  // VSPI的MOSI引脚
#define MISO_PIN  45  // VSPI的MISO引脚
#define CS_PIN    21   // VSPI的CS引脚

void SD_setup(const char* sd_act, const char* sd_para) {
  SPIClass hspi(HSPI); 
  hspi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  if (!SD.begin(CS_PIN, hspi)) {
    Serial.println("SD卡初始化失败！");
    while (1);
  }
  Serial.println("SD卡初始化成功！");
  if(sd_act == "create"){
    createFile(SD, sd_para);
  }
  else if(sd_act == "delete"){
    deleteFile(SD, sd_para);
  }
}



void createFile(fs::FS &fs, const char *path) {
  
  Serial.printf("Creating file: %s\n", path);
  
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to create file\n");
    return;
  }
  else{
    Serial.println("success to create file\n");
  }
  file.close(); // 必须关闭文件以保存
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}



// 打印SD卡目录的辅助函数
void printDirectory(File dir, int numTabs) {
  int i = 1;
  while(true) {
    File entry =  dir.openNextFile();
    if (!entry) break;
    for (uint8_t i=0; i<numTabs; i++) {
      Serial.print('\t');
    }
    if(i != 1){Serial.println(entry.name());}
    entry.close();
    i++;
  }
}


