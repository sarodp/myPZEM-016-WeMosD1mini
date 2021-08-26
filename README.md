myHTTP_ModbusRTU_StaticIP_WebOTA.ino
9apr2021
psarod@gmail.com

Sketch 1.8.10

Modbus Serial2 Wirings
--
ESP01 GPIO0/GPIO2/GND ---- RX/TX/GND ttl-3v3 ---- to PZEM-016 V2 
WEMOS D1 Mini 
ESP12 GPIO0/GPIO2/GND ---- RX/TX/GND ttl-3v3 ---- to PZEM-016 V2 
 
read V,A,W,WH,Hz,PF,ALARM 
print to esp01.serial  

PZEM-016
--
PZEM-016 + Blynk
 https://solarduino.com/pzem-016-ac-energy-meter-online-monitoring-with-blynk-app/

PZEM-016 modify
--
PZEM-ESP Modify bypass RS485 ==> TTL UART
 https://github.com/Gio-dot/PZEM-016-OLED-2-OUT-ESPHome
 
 https://community.home-assistant.io/t/pzem-016-with-esphome-oled-display-and-2-outputs/191409


Library Modbus-esp8266 v3.0.6
--
github ----
 https://github.com/emelianov/modbus-esp8266
 https://github.com/emelianov/modbus-esp8266/archive/3.0.6.zip

ABOUT ----
The Most complete Modbus library for Arduino. 
A library that allows your Arduino board 
to communicate via Modbus protocol, 
acting as a master, slave or both. 
Supports network transport (Modbus TCP) 
and Serial line/RS-485 (Modbus RTU). 
Supports Modbus TCP Security for ESP8266/ESP32. 

Q&A ---- 
 https://www.gitmemory.com/emelianov


ESP-01 or WEMOD D1 Mini (ESP-12) pinout
--
esp-01 pin out
pinno     pin name
8 7 6 5   3V3,   RST,   CH_PD, TX(1)
1 2 3 4   RX(3), GPIO0, GPIO2, GND  


GPIO0 = RX modbus software serial
GPIO2 = TX modbus software serial
GND   = GND serial


ElegantOTA
--
 https://github.com/ayushsharma82/ElegantOTA

ElegantOTA provides a beautiful interface 
to upload Over the Air `.bin` updates 
to your ESP Modules 
with precise status and progress displayed over UI. 

This Library shows the current upload progress of your OTA 
and once finished, it will display the status of your OTA. 

