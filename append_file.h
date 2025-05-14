#ifndef APPEND_FILE_H
#define APPEND_FILE_H

#include <SPI.h>
#include <SD.h>
#include <FS.h>

void appendPcmToWav( const char* filename, int16_t* pcmData, uint32_t sampleCount);
#endif