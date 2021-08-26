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


//____________________include_____________________
#include <time.h>
#include <coredecls.h>                  //   required for settimeofday_cb()
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ModbusRTU.h>
#include "myfscmd.h"
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


//--NTP 


//---wifi
const char *ssid = "true_home2G_85c";
const char *password = "Hp2ArYCx";

const char* OTAid = "111222333";
const char* OTAusername = "sarodp";
const char* OTApassword = "12341234";

IPAddress myip(192, 168, 1, 248);
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
  startNTP();

//---StartFS
  startFS(false);   // format=NO
  //startFS(true);  // format=YES  
  testFStimestamp();

//--Start Webserver+ElegantOTA
  server.on("/", handleRoot);
  server.on("/data",handleData);
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
    
    //--Tstamp
    struct tm *tmp ;               // NOTE: structure tm is defined in time.h
    char FORMATTED_TIME[50];       //   filled by strftime()

    time_t tnow = time(nullptr);     // get UNIX timestamp 
    tmp = localtime(&tnow);           // break down xtime     
                                                 //==> 25-03-2021 12:11:51
    strftime(FORMATTED_TIME, sizeof(FORMATTED_TIME), "%d-%m-%Y %H:%M:%S", tmp);  

    TstampHEX = tnow ;    //pzem.tstampHEX   01234567890123456789 
    TstampDT = FORMATTED_TIME;

    printDateTime(tnow);

  }

 //02b--mbRTU.task 
  mbRTU.task();
  
 //99---done 
  yield();      
}



//______________START______________________________

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


void startNTP() {
 // https://github.com/G6EJD/ESP_Simple_Clock_Functions
 // https://forum.arduino.cc/t/how-to-replace-sketch-with-http-server-with-ntp/606396/2
 
 //--config
  // implement NTP update of timekeeping (with automatic 60sec updates)
  // info to convert UNIX time to local time (including automatic DST update)
  // setenv("TZ", "EST+5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00", 1); 
  // register a callback (execute whenever an NTP update has occurred)

  configTime(0, 0, "asia.pool.ntp.org");  //NTP server
  setenv("TZ", "<+7>-7", 1);              //TimeZone GMT+7    
  tzset();
  settimeofday_cb(time_is_set);           //NTP callback every 60sec default

 //---- wait until NTP Time = OK
 time_t tnow,t = 0,yearmin ;
  Serial.println("....waiting NTP time start.....");
  yearmin = 3600*24*365*10;  // yearmin =1980 
  while (t < yearmin) {
    tzset();
    t = time(nullptr);
    delay(500);
    Serial.print(".");
  }
  tnow = time(nullptr);
  Serial.print("\nNTP Time = ");
  Serial.print(tnow);
  Serial.print("  ");
  printDateTime(tnow);
 }


boolean startFS(boolean flagFormat) {
//00--format FS
 if (flagFormat){
   Serial.println("Formatting LittleFS filesystem");
   LittleFS.format();
 }

//01--mount FS 
 if (!mountFS()) {
  Serial.println("ERROR mountFS !!!");
  return false;
 } 

//02--FSinfo
 printFSinfo();
 listDir("/");
 return true;
}


//__________________________functions____________________________

void time_is_set (void)
// callback routine - arrive here whenever a successful NTP update has occurred
{
  struct tm *tmp ;                      // NOTE: structure tm is defined in time.h  
  char UPDATE_TIME[50];                 // buffer for use by strftime()
    
  // display time when NTP update occurred
  time_t tnow = time(nullptr);          // get UNIX timestamp 
  tmp = localtime(&tnow);               // convert to local time and break down
  strftime(UPDATE_TIME, sizeof(UPDATE_TIME), "%T", tmp);  // extract just the 'time' portion
    
  Serial.print("\n-------- NTP update at ");
  Serial.print(UPDATE_TIME);
  Serial.println(" --------");
}


void printDateTime(time_t xtime)
{
  // use strftime() to display the date and time 
  // http://www.cplusplus.com/reference/ctime/strftime/
  // https://www.geeksforgeeks.org/strftime-function-in-c/ 

  //time_t tnow = time(nullptr);          // get UNIX timestamp 
  struct tm *tmp ;               // NOTE: structure tm is defined in time.h
  char FORMATTED_TIME[50];       //   filled by strftime()
  tmp = localtime(&xtime);       // break down xtime     
                                              //==> 25-03-2021 12:11:51
  strftime(FORMATTED_TIME, sizeof(FORMATTED_TIME), "%d-%m-%Y %H:%M:%S", tmp);  
  Serial.println(FORMATTED_TIME);
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
