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
#define FILENAME "esp32"
#define SAVE_INTERVAL 30 //save new file every 30s
#define CHANNEL_HOPPING true //if true it will scan on all channels
#define MAX_CHANNEL 11 //(only necessary if channelHopping is true)
#define HOP_INTERVAL 214 //in ms (only necessary if channelHopping is true)


//===== Run-Time variables =====//
unsigned long lastTime = 0;
unsigned long lastChannelChange = 0;
int counter = 0;
int ch = CHANNEL;
bool fileOpen = false;

PCAP pcap = PCAP();


//===== FUNCTIONS =====//

/* will be executed on every packet the ESP32 gets while beeing in promiscuous mode */
void sniffer(void *buf, wifi_promiscuous_pkt_type_t type){
  
  if(fileOpen){
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)pkt->rx_ctrl;
  
    uint32_t timestamp = now(); //current timestamp 
    uint32_t microseconds = (unsigned int)(micros() - millis() * 1000); //micro seconds offset (0 - 999)
    pcap.newPacketSD(timestamp, microseconds, ctrl.sig_len, pkt->payload); //write packet to file
    
  }
  
}

esp_err_t event_handler(void *ctx, system_event_t *event){ return ESP_OK; }


/* opens a new file */
void openFile(){

  //searches for the next non-existent file name
  int c = 0;
  String filename = "/" + (String)FILENAME + ".pcap";
  while(SD.open(filename)){
    filename = "/" + (String)FILENAME + "_" + (String)c + ".pcap";
    c++;
  }
  
  //set filename and open the file
  pcap.filename = filename;
  fileOpen = pcap.openFile(SD);

  Serial.println("opened: "+filename);

  //reset counter (counter for saving every X seconds)
  counter = 0;
}


//===== SETUP =====//
void setup() {
  
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  
  /* initialize SD card */
  if(!SD.begin()){
    Serial.println("Card Mount Failed");
    return;
  }
  
  uint8_t cardType = SD.cardType();
  
  if(cardType == CARD_NONE){
      Serial.println("No SD card attached");
      return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
      Serial.println("MMC");
  } else if(cardType == CARD_SD){
      Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
  } else {
      Serial.println("UNKNOWN");
  }

  int64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
    
  openFile();

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

  Serial.println("Sniffer started!");
}

void loop() {
  unsigned long currentTime = millis();
  
  /* Channel Hopping */
  if(CHANNEL_HOPPING){
    if(currentTime - lastChannelChange >= HOP_INTERVAL){
      lastChannelChange = currentTime;
      ch++; //increase channel
      if(ch > MAX_CHANNEL) ch = 1;
      wifi_second_chan_t secondCh = (wifi_second_chan_t)NULL;
      esp_wifi_set_channel(ch,secondCh);
    }
  }
  
	/* for every second */
  if(fileOpen && currentTime - lastTime > 1000){
    pcap.flushFile(); //save file
    lastTime = currentTime; //update time
    counter++; //add 1 to counter
  }
  /* when counter > 30s interval */
  if(fileOpen && counter > SAVE_INTERVAL){
    pcap.closeFile(); //save & close the file
    fileOpen = false; //update flag
    Serial.println("==================");
    Serial.println(pcap.filename + " saved!");
    Serial.println("==================");
    openFile(); //open new file
  }

}