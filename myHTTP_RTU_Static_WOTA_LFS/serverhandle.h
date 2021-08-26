

void handleRoot000() {
  //server.send(200, "text/html", "<h1>You are connected</h1>");
  
  String message = "PZEM-016 Meter\n\n";
   message += "\nV = "+String(PZEMdata1.V,1) ;
   message += "\nA = "+String(PZEMdata1.A,3) ;
   message += "\nW = "+String(PZEMdata1.W,1) ;
   message += "\nWH = "+String(PZEMdata1.WH,0) ;
   message += "\nHz= "+String(PZEMdata1.HZ,1) ;
   message += "\nPF = "+String(PZEMdata1.PF,2) ;
  server.send(200, "text/plain", message);
}


void handleRoot() {
/*
 //--"led" ==> [PUSH] 
  digitalWrite (LED_BUILTIN, 0); //turn the built in LED on pin DO of NodeMCU on
  digitalWrite (ledPin, server.arg("led").toInt());
  ledState = digitalRead(ledPin);

 //-- [GET] <=== ledState,ledText, "led" 
 //-- Dynamically generate the LED toggle link, based on its current state (on or off)
  char ledText[80];
  
  if (ledState) {
    strcpy(ledText, "LED is on. <a href=\"/?led=0\">Turn it OFF!</a>");
  }

  else {
    strcpy(ledText, "LED is OFF. <a href=\"/?led=1\">Turn it ON!</a>");
  }
 
  ledState = digitalRead(ledPin);
//---------------------------------------------------
*/
  

//--brightness
//  int brightness = analogRead(A0);
//  brightness = (int)(brightness + 5) / 10; //converting the 0-1024 value to a (approximately) percentage value

//--uptime
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;


//--Build an HTML page to display on the web-server root address
//
//--snprintf help
// int snprintf( char *restrict buffer, size_t bufsz,
//              const char *restrict format, ... );
//--float to string
//  char floatString[10];
//  dtostrf(xf,4,2,floatString)

  char html[1000];
  snprintf ( html, 1000,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>My Home WiFi-Energy-Meter</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; font-size: 1.5em; Color: #000000; }\
      h2 { Color: #AA0000; }\
    </style>\
  </head>\
  <body>\
    <h2>My Home WiFi-Energy-Meter</h2>\
    <p>%.1fHz      %.2fPF</p>\
    <p>%.1fV       %.3fA</p>\
    <p>WATT: %.1f</p>\
    <p>KWH: %.3f</p>\
    <p>ALARM: %02d</p>\
    <p>TimeStamp: %s</p>\
    <p>UpTime: %02d:%02d:%02d</p>\
    <p>This page refreshes every 5 seconds. </p>\
    <p>Click <a href=\"javascript:window.location.reload();\">here</a> to refresh the page now.</p>\
    <p>Click <a href=\"http://%s/data\">data</a> to view History Energy Records.</p>\
    <p>Click <a href=\"http://%s/update\">upload</a> to upload new firmware.</p>\
  </body>\
</html>",

    PZEMdata1.HZ, PZEMdata1.PF,
    PZEMdata1.V, PZEMdata1.A,
    PZEMdata1.W,
    (PZEMdata1.WH*.001),
    PZEMdata1.ALARM,    
    PZEMdata1.tstampDT,
    hr, min % 60, sec % 60,
    WiFi.localIP().toString().c_str(),
    WiFi.localIP().toString().c_str()
  );

//--.send
  server.send ( 200, "text/html", html );
}


void handleData() {
//--uptime
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

//--Build an HTML page to display on the web-server root address
//
//--snprintf help
// int snprintf( char *restrict buffer, size_t bufsz,
//              const char *restrict format, ... );
//--float to string
//  char floatString[10];
//  dtostrf(xf,4,2,floatString)

  char html[1000];
  snprintf ( html, 1000,

"<html>\
  <head>\
    <title>My Home WiFi-Energy-Meter: HISTORY DATA</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; font-size: 1.5em; Color: #000000; }\
      h2 { Color: #AA0000; }\
    </style>\
  </head>\
  <body>\
    <h2>My Home WiFi-Energy-Meter: HISTORY DATA</h2>\
    <p>UpTime: %02d:%02d:%02d</p>\
    <p>This page refreshes every 5 seconds. </p>\
    <p>Click <a href=\"javascript:window.location.reload();\">here</a> to refresh the page now.</p>\
    <p>Click <a href=\"http://%s/update\">upload</a> to upload new firmware.</p>\
  </body>\
</html>",
  hr, min % 60, sec % 60,
  WiFi.localIP().toString().c_str()
  );
//--.send
  server.send ( 200, "text/html", html );

}


void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );

}
