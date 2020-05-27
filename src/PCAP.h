/*
  ===========================================
       Copyright (c) 2017 Stefan Kremser
              github.com/spacehuhn
  ===========================================
*/

#ifndef PCAP_h
#define PCAP_h

#include <Arduino.h>
#include "SPI.h"
#if defined(ESP32)
	#include "FS.h"
	#include "SD.h"
#else
	#include <SPI.h>
	#include <SdFat.h>
#endif

class PCAP
{
  public:
    PCAP();
	
    void startSerial();
#if defined(ESP32)
	bool openFile(fs::FS &fs);
    bool removeFile(fs::FS &fs);
#else
	bool openFile(SdFat &SD);
    bool removeFile(SdFat &SD);
#endif
	
    void flushFile();
    void closeFile();

    void newPacketSerial(uint32_t ts_sec, uint32_t ts_usec, uint32_t len, uint8_t* buf);
    void newPacketSD(uint32_t ts_sec, uint32_t ts_usec, uint32_t len, uint8_t* buf);
	
	String filename = "capture.cap";
	
    uint32_t magic_number = 0xa1b2c3d4;
    uint16_t version_major = 2;
    uint16_t version_minor = 4;
    uint32_t thiszone = 0;
    uint32_t sigfigs = 0;
    uint32_t snaplen = 2500;
    uint32_t network = 105;

  private:
    File file;

    void escape32(uint32_t n, uint8_t* buf);
    void escape16(uint16_t n, uint8_t* buf);
	
    void filewrite_16(uint16_t n);
    void filewrite_32(uint32_t n);
	
    void serialwrite_16(uint16_t n);
    void serialwrite_32(uint32_t n);
};

#endif

