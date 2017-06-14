# ArduinoPcap
Create and send .pcap files using ESP8266/ESP32 and Arduino.

**Support me and my projects on [Patreon!](https://www.patreon.com/spacehuhn)**  
[<img width="200" alt="Support me on Patreon" src="https://c5.patreon.com/external/logo/downloads_wordmark_navy.png">](https://www.patreon.com/spacehuhn)  


## Installation

Download this project and extract the .zip file to the Arduino `libraries` folder.  
For more information please read [Installing Additional Arduino Libraries](https://www.arduino.cc/en/Guide/Libraries).  

### Dependencies

**If you don't use an ESP32, you will need the [SdFat](https://github.com/greiman/SdFat) library.**

Following Arduino libraries are required for the included examples:
- [Arduino Time Library](https://github.com/PaulStoffregen/Time)  
- [ESP8266](https://github.com/esp8266/Arduino)  
- [ESP32](https://github.com/espressif/arduino-esp32)  

For the Serialshark script:
  - [python3](https://www.python.org/downloads/)
  - [pyserial](https://github.com/pyserial/pyserial)  
  - [Wireshark](https://www.wireshark.org/#download)

**Please be aware that the Serialshark script doesn't work on Windows!**

## Usage

### General

To open one of the examples open Arduino and go to `File`>`Examples`>`Arduino Pcap`.  
You might want to adjust the settings in the sketch (settings may be different depending on the example):  
```
//===== SETTINGS =====//
#define CHANNEL 1
#define FILENAME "esp8266"
#define SAVE_INTERVAL 30 //save new file every 30s
#define CHANNEL_HOPPING true //if true it will scan on all channels
#define MAX_CHANNEL 11 //(only necessary if channelHopping is true)
#define HOP_INTERVAL 214 //in ms (only necessary if channelHopping is true)
```

Also note that when you use the ESP8266 most packets will be malformed or cut off at the end.  
I don't know why the promiscuous mode returns this mostly false data, but it's still good enough to get some MAC adresses and SSIDs. I used it successfully in various projects to [detect deauthentication packets](https://github.com/spacehuhn/DeauthDetector), [scan for client devices](https://github.com/spacehuhn/esp8266_deauther) and build a low cost WiFi [packet monitor](https://github.com/spacehuhn/PacketMonitor).  
But if you want something more reliable then have a look at the ESP32, its promiscuous mode is working way better.  

### Live capture using Wireshark

You also live capture the packets in Wireshark over the serial connection!  
For that be sure to have everything installed - see [Dependencies](#dependencies).  

Flash your device and be sure it is connected.  
Then start the `SerialShark.py` (it's in the `extras` folder) with `python3 SerialShark.py`.  
It will ask you about about the serial port (you have to find that out), the baudrate (be sure it's the same as in your sketch) and where to save the pcap file (wherever you want).  

### Saving onto a SD-Card

For that you will need a SD-Card (obviously). You can connect it direcly to the ESP8266/ESP32, but I recomment buying a SD-Card module/shield. Wire up the SPI connection (MOSI,MISO,SCL,CS,GND,VCC) to your module and basically you are ready to go!  
[If you don't know which pins to connect to which, just search for a pinout picture of your module]  

The included examples will save a new .pcap file every 30s. If you open the serial monitor in Arduino (baudrate is 115200), you will get some more information.  

## License

This project is licensed under the MIT License - see the [license file](LICENSE) for details.
