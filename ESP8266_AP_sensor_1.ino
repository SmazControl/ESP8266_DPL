/****************************
 * ESP8266 DPL              *
 * ESP8266 AP Sensor        *
 *                          *
 * 14 Apr 2019              *
 * Supot Sawangpiriyakij    *
 * Bunnavit Sawangpiriyakij *
 ****************************/
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 2);
DNSServer dnsServer;
ESP8266WebServer webServer(80);
String Id;
#define UPL_max 100
String UPL_name[UPL_max];
float UPL_value[UPL_max];
int UPL_number = 0;
int delay_time = 10000;

void handlePost() {
  digitalWrite(D7,HIGH);
  int i = 0;
  for (i = 0; i < webServer.args(); i++) {
    UPL_name[i] = webServer.argName(i);
    String S_value = webServer.arg(i);
    UPL_value[i] = S_value.toFloat();
    Serial.println("UPL "+webServer.argName(i)+","+S_value);
  }
  webServer.send(200, "text/plain", "UPL Post done");

  for (int j = i; j<UPL_max; j++) {
    if (UPL_name[j]=="delay") {
      delay_time = UPL_value[j];
    }
    
    // UPL command code
    //
    //
    //
    // UPL command code

    // clear UPL
    UPL_name[j] = "";
    UPL_value[j] = -1;
  }  
  delay(delay_time);
  digitalWrite(D7,LOW);
}

void handleGet() {
  digitalWrite(D8,HIGH);

  // Example Sensor Data
  String message = Id+":number,"+String(random(256));
  message += "\n";
  message += Id+":humidity,47.46";
  Serial.println(message);
  // Example Sensor Data

  // Your Sensor Collect Data Code here...
  //
  //
  //        Y o u r    C o d e
  //
  //
  // Your Sensor Collect Data Code here...
  
  webServer.send(200, "text/plain", message);
  delay(delay_time);
  digitalWrite(D8,LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(D7,OUTPUT);
  pinMode(D8,OUTPUT);
  digitalWrite(D7,LOW);
  digitalWrite(D8,LOW);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  Id = "ESP"+String(ESP.getChipId());
  WiFi.softAP(Id.c_str(),"password");
  Serial.println("AP Web Server Start...");
  Serial.println(Id);
  Serial.println(WiFi.localIP());
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(DNS_PORT, "www.esp.com", apIP);
  webServer.on("/post", handlePost); 
  webServer.on("/get", handleGet);
  webServer.onNotFound([]() {
    Serial.println("Hello World!");
    String message = "Hello World!\n\n";
    message += "URI: ";
    message += webServer.uri();

    webServer.send(200, "text/plain", message);
  });
  webServer.begin();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}
