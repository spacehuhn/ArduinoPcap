/*
  ===========================================
       Copyright (c) 2017 Stefan Kremser
              github.com/spacehuhn
  ===========================================
*/


/* include all necessary libraries */ 
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_wifi_internal.h"
#include "lwip/err.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include <Arduino.h>
#include <TimeLib.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <PCAP.h>


//===== SETTINGS =====//
#define CHANNEL 1
#define BAUD_RATE 921600
#define CHANNEL_HOPPING true //if true it will scan on all channels
#define MAX_CHANNEL 11 //(only necessary if channelHopping is true)
#define HOP_INTERVAL 214 //in ms (only necessary if channelHopping is true)

//===== Run-Time variables =====//
PCAP pcap = PCAP();
int ch = CHANNEL;
unsigned long lastChannelChange = 0;

//===== FUNCTIONS =====//

/* will be executed on every packet the ESP32 gets while beeing in promiscuous mode */
void sniffer(void *buf, wifi_promiscuous_pkt_type_t type){
  wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)pkt->rx_ctrl;
  
  uint32_t timestamp = now(); //current timestamp 
  uint32_t microseconds = (unsigned int)(micros() - millis() * 1000); //micro seconds offset (0 - 999)
  
  pcap.newPacketSerial(timestamp, microseconds, ctrl.sig_len, pkt->payload); //send packet via Serial  
}

esp_err_t event_handler(void *ctx, system_event_t *event){ return ESP_OK; }


//===== SETUP =====//
void setup() {

  /* start Serial */
  Serial.begin(BAUD_RATE);
  delay(2000);
  Serial.println();
  
  Serial.println("<<START>>");
  pcap.startSerial();

  /* setup wifi */
  nvs_flash_init();
  tcpip_adapter_init();
  ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );  
  ESP_ERROR_CHECK( esp_wifi_start() );
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(sniffer);
  wifi_second_chan_t secondCh = (wifi_second_chan_t)NULL;
  esp_wifi_set_channel(ch,secondCh);
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
      wifi_second_chan_t secondCh = (wifi_second_chan_t)NULL;
      esp_wifi_set_channel(ch,secondCh);
    }
  }
  
}