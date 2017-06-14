/*
  ===========================================
       Copyright (c) 2017 Stefan Kremser
              github.com/spacehuhn
  ===========================================

  PLEASE NOTE:
    The ESP8266's promiscuous mode is usable, but will return mostly malformed packets and packets that are cut off after x bytes.
    This example is more of a proof-of-concept. If you want something more reliable, please have a look at the ESP32.
  
*/

/* include all necessary libraries */ 
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <TimeLib.h>
#include <SPI.h>
#include <SdFat.h>
#include <PCAP.h>

extern "C" {
  #include "user_interface.h"
}


//===== SETTINGS =====//
#define CHANNEL 1
#define FILENAME "esp8266"
#define SAVE_INTERVAL 30 //save new file every 30s
#define CHANNEL_HOPPING true //if true it will scan on all channels
#define MAX_CHANNEL 11 //(only necessary if channelHopping is true)
#define HOP_INTERVAL 214 //in ms (only necessary if channelHopping is true)


//===== Run-Time variables =====//
SdFat SD;
unsigned long lastTime = 0;
unsigned long lastChannelChange = 0;
int counter = 0;
int ch = CHANNEL;
bool fileOpen = false;

PCAP pcap = PCAP();


//===== FUNCTIONS =====//

/* will be executed on every packet the ESP8266 gets while beeing in promiscuous mode */
void sniffer(uint8_t *buf, uint16_t len) {
  if(fileOpen){
    uint32_t timestamp = now(); //current timestamp 
    uint32_t microseconds = (unsigned int)(micros() - millis() * 1000); //micro seconds offset (0 - 999)
    pcap.newPacketSD(timestamp, microseconds, len, buf); //write packet to file
  }
}

/* open a new file */
void openFile(){

  //searches for the next non-existent file name
  int c = 0;
  String filename = (String)FILENAME + ".pcap";
  while(SD.exists(filename.c_str())){
    filename = (String)FILENAME + "_" + (String)c + ".pcap";
    c++;
  }

  //set filename and open the file
  pcap.filename = filename;
  fileOpen = pcap.openFile(SD);

  Serial.println("opened: "+filename);

  //reset counter for saving every X seconds
  counter = 0;
}


//===== SETUP =====//
void setup() {
  
  /* start Serial */
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println("starting...");
  
  /* initialize SD card */
  if(!SD.begin()) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  openFile();
  
  /* setup wifi */
  wifi_set_opmode(STATION_MODE); //put the ESP into Station-Mode
  wifi_promiscuous_enable(0); //disable promiscuous first
  WiFi.disconnect(); //disconnect any WiFi connection just to be safe
  wifi_set_promiscuous_rx_cb(sniffer); //set callback function for our sniffer
  wifi_set_channel(ch); //switch to our selected channel
  wifi_promiscuous_enable(1); //enable promiscuous
  
  Serial.println("Sniffer started!");

}

//===== LOOP =====//
void loop() {
  unsigned long currentTime = millis();

  /* Channel Hopping */
  if(CHANNEL_HOPPING){
    if(currentTime - lastChannelChange >= HOP_INTERVAL){
      lastChannelChange = currentTime;
      ch++; //increase channel
      if(ch > MAX_CHANNEL) ch = 1;
      wifi_set_channel(ch); //switch to new channel
      Serial.println( "Switched to channel " + (String)ch );
    }
  }
  
  /* for every second */
  if(fileOpen && currentTime - lastTime >= 1000){
    pcap.flushFile(); //save file
    lastTime = millis(); //update time
    counter++; //add 1 to counter
  }

  /* when counter > 30s interval */
  if(fileOpen && counter >= SAVE_INTERVAL){
    pcap.closeFile(); //save & close the file
    fileOpen = false; //update flag
    Serial.println("==================");
    Serial.println(pcap.filename + " saved!");
    Serial.println("==================");
    openFile(); //open new file
  }
}