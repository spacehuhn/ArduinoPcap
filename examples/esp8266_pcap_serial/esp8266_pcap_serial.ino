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
#include <PCAP.h>

extern "C" {
  #include "user_interface.h"
}


//===== SETTINGS =====//
#define CHANNEL 1
#define BAUD_RATE 115200
#define CHANNEL_HOPPING true //if true it will scan on all channels
#define MAX_CHANNEL 11 //(only necessary if channelHopping is true)
#define HOP_INTERVAL 214 //in ms (only necessary if channelHopping is true)

//===== Run-Time variables =====//
PCAP pcap = PCAP();
int ch = CHANNEL;
unsigned long lastChannelChange = 0;

//===== FUNCTIONS =====//

/* will be executed on every packet the ESP8266 gets while beeing in promiscuous mode */
void sniffer(uint8_t *buf, uint16_t len) {
  uint32_t timestamp = now(); //current timestamp 
  uint32_t microseconds = (unsigned int)(micros() - millis() * 1000); //micro seconds offset (0 - 999)
  pcap.newPacketSerial(timestamp, microseconds, len, buf); //write packet to file
}

//===== SETUP =====//
void setup() {
  
  /* start Serial */
  Serial.begin(BAUD_RATE);
  delay(2000);
  Serial.println();

  Serial.println("<<START>>");
  pcap.startSerial();
  
  /* setup wifi */
  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  WiFi.disconnect();
  wifi_set_promiscuous_rx_cb(sniffer);
  wifi_set_channel(ch);
  wifi_promiscuous_enable(1);
}

//===== LOOP =====//
void loop() {
  /* Channel Hopping */
  if(CHANNEL_HOPPING){
    unsigned long currentTime = millis();
    if(currentTime - lastChannelChange >= HOP_INTERVAL){
      lastChannelChange = currentTime;
      ch++; //increase channel
      if(ch > MAX_CHANNEL) ch = 1;
      wifi_set_channel(ch); //switch to new channel
    }
  }
}
