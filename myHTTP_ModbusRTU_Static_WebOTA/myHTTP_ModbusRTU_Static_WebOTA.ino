//-----------------------------
// myHTTP_ModbusRTU_StaticIP_WebOTA.ino
// 9apr2021
// psarod@gmail.com
//
// Sketch 1.8.10
//
// ESP01 GPIO0/GPIO2/GND ---- RX/TX/GND ttl-3v3 ---- to PZEM-016 V2 
// 
// read V,A,W,WH,Hz,PF,ALARM 
// print to esp01.serial  
//--------------------------------------

//============================================================
// PZEM-016 + Blynk
// https://solarduino.com/pzem-016-ac-energy-meter-online-monitoring-with-blynk-app/

//========================================================================
// PZEM-ESP Modify bypass RS485 ==> TTL UART
// https://github.com/Gio-dot/PZEM-016-OLED-2-OUT-ESPHome
//========================================================================

//------------------------------------------------
// Library Modbus-esp8266 v3.0.6
//
// github ----
// https://github.com/emelianov/modbus-esp8266
// https://github.com/emelianov/modbus-esp8266/archive/3.0.6.zip
//
// ABOUT ----
// The Most complete Modbus library for Arduino. 
// A library that allows your Arduino board 
// to communicate via Modbus protocol, 
// acting as a master, slave or both. 
// Supports network transport (Modbus TCP) 
// and Serial line/RS-485 (Modbus RTU). 
// Supports Modbus TCP Security for ESP8266/ESP32. 
//
// Q&A ---- 
// https://www.gitmemory.com/emelianov
// ------------------------------

//--------------------------------------
// esp-01 pin out
// pinno     pin name
// 8 7 6 5   3V3,   RST,   CH_PD, TX(1)
// 1 2 3 4   RX(3), GPIO0, GPIO2, GND  
//
//
//  GPIO0 = RX modbus software serial
//  GPIO2 = TX modbus software serial
//  GND   = GND serial
//---------------------------------------

//=============================================
// https://github.com/ayushsharma82/ElegantOTA
//
// ElegantOTA provides a beautiful interface 
// to upload Over the Air `.bin` updates 
// to your ESP Modules 
// with precise status and progress displayed over UI. 
// 
// This Library shows the current upload progress of your OTA 
// and once finished, it will display the status of your OTA. 
//==============================================


//____________________include_____________________
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ModbusRTU.h>
#include <ElegantOTA.h>



//____________________INIT_____________________
//--init Modbus, Serial2
SoftwareSerial Serial2(0, 2);  // (RX, TX)
ModbusRTU mbRTU;

//--PZEM
#include "PZEMcalc.h"

//--OTA
const char *OTAName = "myesp01";         // A name and a password for the OTA service
const char *OTAPassword = "esp01";


//--NTP timeClient
const long utcOffsetInSeconds = 7* 3600;
const char *timeserver = "asia.pool.ntp.org";
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, timeserver, utcOffsetInSeconds);

//---wifi
const char *ssid = "true_home2G_85c";
const char *password = "Hp2ArYCx";

const char* OTAid = "111222333";
const char* OTAusername = "sarodp";
const char* OTApassword = "12341234";

IPAddress myip(192, 168, 1, 249);
IPAddress mydns(192, 168, 1, 1);
IPAddress mygateway(192, 168, 1, 1);
IPAddress mysubnet(255, 255, 255, 0);


//--HTTP 
ESP8266WebServer server(80);  // Define a web server at port 80 for HTTP
#include "serverhandle.h"



//____________________SETUP_____________________

void setup() {
//--StartSerial
  delay(1000);
  Serial.begin(74800);

//--StartWiFi,OTA
  startWiFi_static();


//--StartNTP
  timeClient.begin();

//--Start Webserver+ElegantOTA
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  //start OTA
  ElegantOTA.setID(OTAid);  //default = esp chip id
  ElegantOTA.begin(&server,OTAusername,OTApassword);   
  
  server.begin();
  Serial.println("HTTP server started");

//--StartModbusRTU
  Serial2.begin(9600, SWSERIAL_8N1);
  mbRTU.begin(&Serial2);
  mbRTU.master();
  
}

//____________________LOOP_____________________

//--init Loop
uint32_t time_start = millis();
uint32_t time_tick[4] = { 150,300,450,600 };
boolean  flagRead[3]  = { false };

void loop() {

//01--web.handle, OTA.handle
  server.handleClient();


//02a--mbRTU.read... 
  uint32_t time_diff = millis() - time_start;
  
  if ((time_diff > time_tick[0]) and (!flagRead[0]) and (!mbRTU.slave())) { 
    //--Read1 Iregs
    mbRTU.readIreg(1, 0, Iregs, 10, cbIread);
    flagRead[0] = true; 
  }
  /* 
  if ((time_diff > time_tick[1])  and (!flagRead[1]) and(!mbRTU.slave())) { 
    //--Read2 Hregs
    mbRTU.readHreg(1, 0, Hregs, 7, cbHread_debug);
    flagRead[1] = true; 
  }

  if ((time_diff > time_tick[2])  and (!flagRead[2]) and(!mbRTU.slave())) { 
    //--Read3 Iregs
    mbRTU.readIreg(1, 0, Iregs, 10, cbIread_debug);
    flagRead[2] = true;     
  }
  */
  if (time_diff > time_tick[3])  { 
    //--Reset time_start,flagRead..
    time_start = millis();   
    flagRead[0] = false; 
    flagRead[1] = false; 
    flagRead[2] = false; 
    
    //--timeclient
    if(timeClient.update()) {
      TstampHEX = timeClient.getEpochTime();    //pzem.tstampHEX   01234567890123456789 
      //TstampDT = timeClient.getFormattedDate(); //pzem.tstamptDT yyyy-mm-ddThh:mm:ssZ
      String xstrDT =  timeClient.getFormattedDate();
      TstampDT = xstrDT.substring(0,10) + ' ' + xstrDT.substring(11,19);
      print_ntptime();
    }

  }

 //02b--mbRTU.task 
  mbRTU.task();
  
 //99---done 
  yield();      
}


void startWiFi_static() {
// The argument order for ESP is not the same as for Arduino. 
// 1) at least 3 args must always be given.
// 2) must be after WiFi.begin(xx,xx);
// bool WiFi.config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, 
//                  IPAddress dns1 = (uint32_t)0x00000000, 
//                  IPAddress dns2 = (uint32_t)0x00000000);

  //--.begin
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
 
  WiFi.mode(WIFI_STA);
  //WiFi.config(xlocal_ip, xgateway, xsubnet, xdns1,xdns2);  //dns option not try
  //WiFi.config(xlocal_ip, xgateway, xsubnet, xdns1);  //dns options compile.OK but run.ERROR
  //WiFi.config(xlocal_ip, xgateway, xsubnet);  //nodns compile.OK but run.ERROR
  //WiFi.config(xlocal_ip, xdns,xgateway, xsubnet);  //my options both.OK **********

  
  WiFi.begin(ssid, password);
  if (!WiFi.config(myip, mydns, mygateway, mysubnet)) {
    Serial.println("Wifi.config StaticIP Error");}
  
  //--wait connected
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
 
  //--.localIP
  Serial.print("\nIP Address: ");
  Serial.println(WiFi.localIP());
}

void print_ntptime() {
 //--full date-time
  Serial.print(".getEpochTime--HEX == ");
  Serial.println(timeClient.getEpochTime(),HEX);
  
  Serial.print(".getFormattedDate  == ");
  Serial.println(timeClient.getFormattedDate());

  Serial.print(".getFormattedTime  == ");
  Serial.println(timeClient.getFormattedTime());
}




/*
void startWiFiMulti() { // Try to connect to some given access points. Then wait for a connection
  wifiMulti.addAP(ssidAP1, passwordAP1);   // add Wi-Fi networks you want to connect to
  //wifiMulti.addAP(ssidAP2, passwordAP2);

  Serial.println("Connecting");
  while (wifiMulti.run() != WL_CONNECTED) {  // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
  }
  Serial.println("\r\n");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());             // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.print(WiFi.localIP());            // Send the IP address of the ESP8266 to the computer
  Serial.println("\r\n");
}


void startMDNS() { // Start the mDNS responder
  MDNS.begin(mdnsName);                        // start the multicast domain name server
  Serial.print("mDNS responder started: http://");
  Serial.print(mdnsName);
  Serial.println(".local");
}
*/
