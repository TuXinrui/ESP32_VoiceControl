#ifndef SD_ACCESS_H
#define SD_ACCESS_H


#include <SPI.h>
#include <SD.h>
#include <FS.h>


void SD_setup(const char* sd_act, const char* sd_para);
void createFile(fs::FS &fs, const char *path);
void deleteFile(fs::FS &fs, const char * path);
void appendFile(fs::FS &fs, const char * path, uint16_t * message);

#endif