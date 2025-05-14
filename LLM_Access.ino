//

#include <WiFi.h>
#include <esp_http_client.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "i2s.h"
#include "tts.h"
#include "esp_eap_client.h"
#include "sd_access.h"
#include "append_file.h"
#include "base64.hpp"
#include <WebSocketsClient.h>
#include <UrlEncode.h>
#include <HardwareSerial.h>

/*const char* ssid = "your_ssid";
const char* password = "your_password";*/

const char* ssid = "HKU";
const char* username = "your_username";
const char* password = "your_password";

const char* api_key = "Bearer your_api_key" ;
const char* api_url = "https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation";

const char* appid_b = "your_appid";
const char* api_key_b = "your_api_key";
const char* api_url_b = "https://vop.baidu.com/pro_api";
const char* secret_key_b = "your_seecret_key";
const char* web_socket_url = "wss://vop.baidu.com/realtime_asr?sn=GCbnIqfRbTuqVe1tYxZ0q9cT"; 

void baiduTTS_Send(String text) {
  String access_token = getAccessToken(api_key_b, secret_key_b);
  if (access_token == "") {
    Serial.println("access_token is null");
    return;
  }
  if (text.length() == 0) {
    Serial.println("text is null");
    return;
  }
  const int per = 5;
  const int spd = 7;
  const int pit = 5;
  const int vol = 10;
  const int aue = 4;
  // 进行 URL 编码
  String encodedText = urlEncode(urlEncode(text));
  Serial.print(encodedText);
  Serial.print("\n");
  // URL http请求数据封装
  String url = "https://tsn.baidu.com/text2audio";
  const char* header[] = { "Content-Type", "Content-Length" };
  url += "?tok=" + access_token;
  url += "&tex=" + encodedText;
  url += "&per=" + String(per);
  url += "&spd=" + String(spd);
  url += "&pit=" + String(pit);
  url += "&vol=" + String(vol);
  url += "&aue=" + String(aue);
  url += "&cuid=esp32s3";
  url += "&lan=zh";
  url += "&ctp=1";
  // http请求创建
  HTTPClient http;
  http.begin(url);
  http.collectHeaders(header, 2);
  // http请求
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    if (httpResponseCode == HTTP_CODE_OK) {
      String contentType = http.header("Content-Type");
      Serial.println(contentType);
      if (contentType.startsWith("audio")) {
        Serial.println("合成成功");
        // 获取返回的音频数据流
        Stream* stream = http.getStreamPtr();
        uint8_t buffer[512];
        //uint8_t* byte_audio= (uint8_t*)ps_malloc(200000);

        size_t bytesRead = 0;

        // 设置timeout为200ms 避免最后出现杂音
        int ite = 1;
        stream->setTimeout(5000);

        while (http.connected() /*&&(bytesRead = stream->readBytes(buffer, sizeof(buffer))) > 0*/) {
          // 音频输出
          //playAudio(buffer, bytesRead);
          bytesRead = stream->readBytes(buffer, sizeof(buffer));
          if(ite > 2000){
            break;
          }
          Serial.printf("%d, ",bytesRead);
          if (bytesRead > 0) {
            size_t bytes_written = 0;
            i2s_write(I2S_NUM_0, (int16_t*)buffer, bytesRead, &bytes_written, portMAX_DELAY);
          }
          delay(2);
          ite++;
        }
        Serial.print("\n");
        // 清空I2S DMA缓冲区
        clearAudio();
      } else if (contentType.equals("application/json")) {
        Serial.println("合成出现错误");
      } else {
        Serial.println("未知的Content-Type");
      }
    } else {
      Serial.println("Failed to receive audio file");
    }
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// Play audio data using MAX98357A
/*void playAudio(uint8_t* audioData, size_t audioDataSize) {
  
}*/

void clearAudio(void) {
  // 清空I2S DMA缓冲区
  i2s_zero_dma_buffer(I2S_NUM_0);
  Serial.print("clearAudio");
}

String getAccessToken(const char* api_key_baidu, const char* secret_key_baidu) {
  String access_token = "";
  HTTPClient http;

  // 创建http请求
  http.begin("https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=" + String(api_key_baidu) + "&client_secret=" + String(secret_key_baidu));
  int httpCode = http.POST("");

  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, response);
    access_token = doc["access_token"].as<String>();

    Serial.printf("[HTTP] GET access_token: %s\n", access_token);
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

  return access_token;
}

uint8_t* tobytearray(int16_t* input_audio){
  Serial.print("开始编码\n");
  size_t input_audio_len = 48000;
  uint8_t* byteArray = (uint8_t*)ps_malloc(96000);
  Serial.print("定义八位数组\n");
  for (size_t i = 0; i < input_audio_len; i++) {
    byteArray[i * 2] = (uint8_t)(input_audio[i] & 0xFF);       // 低字节
    byteArray[i * 2 + 1] = (uint8_t)((input_audio[i] >> 8) & 0xFF); // 高字节
  }
  Serial.print("转换为8位数组\n");
  //String encodedString = base64::encode(byteArray, input_audio_len * 2);
  //Serial.print("Base64: ");
  //Serial.println(byteArray);
  return byteArray;
}

String baidu_response(int16_t* input_audio){
  String recognizedText = "";
  String access_token = getAccessToken(api_key_b, secret_key_b);
  uint8_t* intput_audio_byte = tobytearray(input_audio);
  int audioDataSize = 96000;
  int audio_data_len = audioDataSize * sizeof(char) * 1.4;
  unsigned char* audioDataBase64 = (unsigned char*)ps_malloc(audio_data_len);
  if (!audioDataBase64) {
    Serial.println("Failed to allocate memory for audioDataBase64");
    return recognizedText;
  }
  // json包大小，由于需要将audioData数据进行Base64的编码，数据量会增大1/3
  int data_json_len = audioDataSize * sizeof(char) * 1.4;
  char* data_json = (char*)ps_malloc(data_json_len);
  if (!data_json) {
    Serial.println("Failed to allocate memory for data_json");
    return recognizedText;
  }
  // Base64 encode audio data
  //base64.encode(audioDataBase64, intput_audio_byte, audioDataSize);
  encode_base64(intput_audio_byte, audioDataSize, audioDataBase64);
  //String macAddress = WiFi.macAddress();
  memset(data_json, '\0', data_json_len);
  strcat(data_json, "{");
  strcat(data_json, "\"format\":\"pcm\",");
  strcat(data_json, "\"rate\":16000,");
  strcat(data_json, "\"dev_pid\":80001,");
  strcat(data_json, "\"channel\":1,");
  strcat(data_json, "\"cuid\":\"20040415133\",");
  strcat(data_json, "\"token\":\"");
  strcat(data_json, access_token.c_str());
  strcat(data_json, "\",");
  sprintf(data_json + strlen(data_json), "\"len\":%d,", audioDataSize);
  strcat(data_json, "\"speech\":\"");
  strcat(data_json, (const char*)audioDataBase64);
  strcat(data_json, "\"");
  strcat(data_json, "}");
  HTTPClient http;
  http.setTimeout(10000);
  http.begin(api_url_b);
  http.addHeader("Content-Type", "application/json");
  //http.addHeader("Authorization", api_key_b);
  int httpResponseCode = http.POST(data_json);
  Serial.print("http posted\n");
  if (httpResponseCode == HTTP_CODE_OK) {
    Serial.print("HTTP_CODE_OK\n");
    recognizedText = http.getString();
    http.end();
    Serial.println(recognizedText);
    // Parse JSON response
    DynamicJsonDocument jsonDoc(1024);
    deserializeJson(jsonDoc, recognizedText);
    String outputText = jsonDoc["result"];
    if (audioDataBase64) {
      free(audioDataBase64);
    }
    if (data_json) {
      free(data_json);
    }
    int len = outputText.length();
    String modified_text = outputText.substring(2, len-2);
    return modified_text;
    // Serial.println(outputText);
  } else {
    http.end();
    //Serial.printf("Error %i \n", httpResponseCode);
    Serial.printf("[HTTP] POST failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
    if (audioDataBase64) {
      free(audioDataBase64);
    }
    if (data_json) {
      free(data_json);
    }
    return "<error>\n";
  }

}


String qwen_response(String input_text){
  HTTPClient http;
  http.setTimeout(10000);
  http.begin(api_url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", api_key);
  String payload = "{\"model\":\"qwen-turbo\",
  \"input\":{\"messages\":[{\"role\": \"system\",\"content\": 
  \"当输入为将某种颜色的小球放到某个数字的位置时,你只需要回答两个汉字,第一个字是颜色,第二个字是数字。
  例如,输入把蓝色小球放到8号位时,你的回答应该是蓝八。若没有输入类似的指令，正常回答即可，回答严格控制在60字内\"},
  {\"role\": \"user\",\"content\": \"" + input_text + "\"}]}}";
  int httpResponseCode = http.POST(payload);
  if (httpResponseCode == 200) {
    String response = http.getString();
    http.end();
    Serial.println(response);
    // Parse JSON response
    DynamicJsonDocument jsonDoc(1024);
    deserializeJson(jsonDoc, response);
    String outputText = jsonDoc["output"]["text"];
    return outputText;
    // Serial.println(outputText);
  } else {
    http.end();
    //Serial.printf("Error %i \n", httpResponseCode);
    Serial.printf("[HTTP] POST failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
    return "<error>\n";
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 41, 40);
  Serial2.begin(115200, SERIAL_8N1, 5, 6);

  Init_i2s_read();
  Init_i2s_write(); //初始化音频输出模块
  Init_tts();
  //audio_output("你好");
  WiFi.begin (ssid,password);
  while(WiFi.status()!= WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  /*WiFi.mode(WIFI_STA);
  esp_eap_client_set_identity((uint8_t *)username, strlen(username)); // 设置身份
  esp_eap_client_set_username((uint8_t *)username, strlen(username)); // 设置用户名 --> 身份和用户名相同
  esp_eap_client_set_password((uint8_t *)password, strlen(password)); // 设置密码
  esp_wifi_sta_enterprise_enable();
  WiFi.begin(ssid);
  while(WiFi.status()!= WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }*/

  Serial.print("Connected, IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print("\n");
  //audio_output("你好");
  String response;
  response = qwen_response("你好千问");
  Serial.print(response);

  //audio_output("开始");
  Serial.println("Enter a prompt:");
  int16_t temp_sound[] = {0, 8000, 15000, 20000, 15000, 8000, 0, -8000, -15000, -20000, -15000, -8000};
  size_t bytes_written;
  
}

void offline_mode(){
  Serial.print("离线模式\n");
  String asr_command;
  size_t bytes_written;
  while(1){
    int16_t temp_sound[] = {0, 8000, 15000, 20000, 15000, 8000, 0, -8000, -15000, -20000, -15000, -8000};
    bytes_written = 0;
    for(int i = 0; i < 100; i++){
      i2s_write(I2S_NUM_0, temp_sound, 12 * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    }
    delay(200);
    bytes_written = 0;
    for(int i = 0; i < 100; i++){
      i2s_write(I2S_NUM_0, temp_sound, 12 * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    }
    delay(200);
    for(int i = 0; i < 100; i++){
      i2s_write(I2S_NUM_0, temp_sound, 12 * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    }
    delay(500);
    while (Serial1.available()) {
      Serial1.read();  // 逐字节读取并丢弃
    }
    while(!Serial1.available()){
      delay(100);
    }
    if(Serial1.available()){
      Serial.print("asr pro available\n");
      asr_command = Serial1.readStringUntil('\n');
      asr_command.trim();
      if(asr_command == "online"){
        Serial.print("在线模式\n");
        return;
      }
      else if(asr_command == "A"){
        Serial2.write(0x41);
        Serial2.write(0x0a);
        Serial.print("A\n");
      }
      else if(asr_command == "B"){
        Serial2.write(0x42); 
        Serial2.write(0x0a);
        Serial.print("B\n");
      }
      else if(asr_command == "C"){
        Serial2.write(0x43);
        Serial2.write(0x0a);
        Serial.print("C\n");
      } 
      else if(asr_command == "D" ){
        Serial2.write(0x44);
        Serial2.write(0x0a);
        Serial.print("D\n");

      }
      else if(asr_command == "E" ){
        Serial2.write(0x45);
        Serial2.write(0x0a);
        Serial.print("E\n");
      }
      else if(asr_command == "F" ){
        Serial2.write(0x46);
        Serial2.write(0x0a);
        Serial.print("F\n");
      }
    }
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:
  String inputText;
  String response;
  String asr_command;
  size_t bytes_written;
  while(1){
    bytes_written = 0;
    int16_t temp_sound[] = {0, 8000, 15000, 20000, 15000, 8000, 0, -8000, -15000, -20000, -15000, -8000};
    for(int i = 0; i < 100; i++){
      i2s_write(I2S_NUM_0, temp_sound, 12 * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    }
    delay(200);
    bytes_written = 0;
    for(int i = 0; i < 100; i++){
      i2s_write(I2S_NUM_0, temp_sound, 12 * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    }
    delay(200);
    for(int i = 0; i < 100; i++){
      i2s_write(I2S_NUM_0, temp_sound, 12 * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    }
    delay(500);
    while (Serial1.available()) {
      Serial1.read();  // 逐字节读取并丢弃
    }
    while(!Serial1.available()){
      delay(100);
    }
    if(Serial1.available()){
      Serial.print("asr pro available\n");
      asr_command = Serial1.readStringUntil('\n');
      asr_command.trim();
      if(asr_command == "wake"){
        Serial.print("唤醒\n");
      }
      else if(asr_command == "offline"){
        offline_mode();
        continue;
      }
      else{
        continue;
      }
    }
    
    //clearSerialBuffer(Serial1);
    //while(1){///for testing
    Serial.print("1秒后开始录音\n");
    delay(500);
    bytes_written = 0;
    
    for(int i = 0; i < 400; i++){
      i2s_write(I2S_NUM_0, temp_sound, 12 * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    }
    delay(500);
    int16_t* input_audio = record_audio();
    bytes_written = 0;
    i2s_write(I2S_NUM_0, input_audio, 96000, &bytes_written, portMAX_DELAY);
    inputText = "<error>";
    
    for(int i = 1; i<=5 ; i++){
      Serial.print("start ASR\n");
      inputText = baidu_response(input_audio);
      inputText.trim();
      if(inputText != "<error>" && inputText != ""){
        break;
      }
      else{
        int16_t temp_sound[] = {0, 8000, 15000, 20000, 15000, 8000, 0, -8000, -15000, -20000, -15000, -8000};
        for(int i = 0; i < 200; i++){
          bytes_written = 0;
          i2s_write(I2S_NUM_0, temp_sound, 12 * sizeof(int16_t), &bytes_written, portMAX_DELAY);
        }
        delay(300);
        for(int i = 0; i < 200; i++){
          bytes_written = 0;
          i2s_write(I2S_NUM_0, temp_sound, 12 * sizeof(int16_t), &bytes_written, portMAX_DELAY);
        }
        delay(300);
        if(i == 5){
          baiduTTS_Send("连续五次连接失败，重启对话");
        }
      }
    }
    //}
    if(inputText == "<error>" || inputText == ""){
      continue;
    }
    Serial.println("Input:"+inputText);
    Serial.print("\n");
    //tobase64(input_audio);
    /*SD_setup("create", "/record.wav");
    uint32_t input_size = sizeof(*input_audio);
    appendPcmToWav("/record.wav", input_audio, input_size );
    Serial.print("\n");*/
    //free(input_audio);
    //}
    //if (Serial.available()) {
    //inputText = Serial.readStringUntil('\n');
    
    //Serial.println("\nInput:"+inputText);
    response = qwen_response(inputText);
    //response = qwen_response(inputText);
    //const char* text = response.c_str();
    Serial.println("response: ");
    Serial.print(response);
    Serial.print("\n");

    if(response == "篮球二" || response == "篮二"|| response == "蓝二"){
      Serial2.write(0x41);
      Serial2.write(0x0a);
    }
    else if(response == "篮球三" || response == "篮三" || response == "蓝三"){
      Serial2.write(0x42); 
      Serial2.write(0x0a);
    }
    else if(response == "篮球四" || response == "篮四" || response == "蓝四"){
      Serial2.write(0x43);
      Serial2.write(0x0a);
    } 
    else if(response == "红二" ){
      Serial2.write(0x44);
      Serial2.write(0x0a);
    }
    else if(response == "红三" ){
      Serial2.write(0x45);
      Serial2.write(0x0a);
    }
    else if(response == "红四" ){
      Serial2.write(0x46);
      Serial2.write(0x0a);
    }
    else{
      baiduTTS_Send(response);
    }
    
    //audio_output(text);//音频输出
    Serial.println("Enter a prompt:");
  }
  //}
}
