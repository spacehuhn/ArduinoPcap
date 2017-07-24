#include "PCAP.h"

/* Create PCAP instance */
PCAP::PCAP() {
}

/* send file header to Serial */
void PCAP::startSerial(){
  serialwrite_32(magic_number);
  serialwrite_16(version_major);
  serialwrite_16(version_minor);
  serialwrite_32(thiszone);
  serialwrite_32(sigfigs);
  serialwrite_32(snaplen);
  serialwrite_32(network);
}

#if defined(ESP32)
	/* open file on SD card (when exists) */
	bool PCAP::openFile(fs::FS &fs){
	  if(fs.exists(filename.c_str())) removeFile(fs);
	  file = fs.open(filename, FILE_WRITE);
	  if(file) {
		filewrite_32(magic_number);
		filewrite_16(version_major);
		filewrite_16(version_minor);
		filewrite_32(thiszone);
		filewrite_32(sigfigs);
		filewrite_32(snaplen);
		filewrite_32(network);
		return true;
	  }
	  return false;
	}

	/* remove file from SD card */
	bool PCAP::removeFile(fs::FS &fs){
	  return fs.remove(filename.c_str());
	}
#else
	/* open file on SD card (when exists) */
	bool PCAP::openFile(SdFat &SD){
	  if(SD.exists(filename.c_str())) removeFile(SD);
	  file = SD.open(filename, FILE_WRITE);
	  if(file) {
		filewrite_32(magic_number);
		filewrite_16(version_major);
		filewrite_16(version_minor);
		filewrite_32(thiszone);
		filewrite_32(sigfigs);
		filewrite_32(snaplen);
		filewrite_32(network);
		return true;
	  }
	  return false;
	}

	/* remove file from SD card */
	bool PCAP::removeFile(SdFat &SD){
	  return SD.remove(filename.c_str());
	}
#endif


/* save changes */
void PCAP::flushFile(){
  if(file) file.flush();
}

/* close file */
void PCAP::closeFile(){
  if(file) file.close();
}

/* write packet to Serial */
void PCAP::newPacketSerial(uint32_t ts_sec, uint32_t ts_usec, uint32_t len, uint8_t* buf){
  uint32_t orig_len = len;
  uint32_t incl_len = len;
  
#if defined(ESP32)
  if(incl_len > snaplen) incl_len = snaplen; /* safty check that the packet isn't too big (I ran into problems with the ESP8266 here) */
#endif
  serialwrite_32(ts_sec);
  serialwrite_32(ts_usec);
  serialwrite_32(incl_len);
  serialwrite_32(orig_len);
  
  Serial.write(buf, incl_len);
}

/* write packet to file */
void PCAP::newPacketSD(uint32_t ts_sec, uint32_t ts_usec, uint32_t len, uint8_t* buf){
  if(file){
    
    uint32_t orig_len = len;
    uint32_t incl_len = len;
    //if(incl_len > snaplen) incl_len = snaplen; /* safty check that the packet isn't too big (I ran into problems here) */
    
    filewrite_32(ts_sec);
    filewrite_32(ts_usec);
    filewrite_32(incl_len);
    filewrite_32(orig_len);
  
    file.write(buf, incl_len);
  }
}

/* converts a 32 bit integer into 4 bytes */
void PCAP::escape32(uint32_t n, uint8_t* buf){
  buf[0] = n;
  buf[1] = n >>  8;
  buf[2] = n >> 16;
  buf[3] = n >> 24;
}

/* converts a 16 bit integer into 2 bytes */
void PCAP::escape16(uint16_t n, uint8_t* buf){
  buf[0] = n;
  buf[1] = n >>  8;
}

/* writes a 32 bit integer onto the SD card */
void PCAP::filewrite_32(uint32_t n){
  uint8_t _buf[4];
  escape32(n, _buf);
  file.write(_buf, 4);
}

/* writes a 16 bit integer onto the SD card */
void PCAP::filewrite_16(uint16_t n){
  uint8_t _buf[2];
  escape16(n, _buf);
  file.write(_buf, 2);
}

/* writes a 32 bit integer to Serial */
void PCAP::serialwrite_32(uint32_t n){
  uint8_t _buf[4];
  escape32(n, _buf);
  Serial.write(_buf, 4);
}

/* writes a 16 bit integer to Serial */
void PCAP::serialwrite_16(uint16_t n){
  uint8_t _buf[2];
  escape16(n, _buf);
  Serial.write(_buf, 2);
}


