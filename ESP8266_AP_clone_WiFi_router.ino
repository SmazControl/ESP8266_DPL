#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);
String Id;
#define DPL_max 300
String DPL_id[DPL_max];
String DPL_name[DPL_max];
float DPL_value[DPL_max];
int DPL_number = 0;
#define UPL_max 100
String UPL_id[UPL_max];
String UPL_name[UPL_max];
float UPL_value[UPL_max];
int UPL_number = 0;


void handlePost() {
  String message = "Number of args received:";
  message += webServer.args();  
  message += "\n";                  
  for (int i = 0; i < webServer.args(); i++) {
    message += "Arg nº" + (String)i + " –> ";
    message += webServer.argName(i) + ": ";
    message += webServer.arg(i) + "\n";
    if(webServer.argName(i)=="number") {
      Serial.println("Received "+String(webServer.arg(i)));
    }
    String S_name = webServer.argName(i);
    int colon = S_name.indexOf('DPL');
    DPL_id[i] = S_name.substring(0,colon);
    DPL_name[i] = S_name.substring(colon+3);
    String S_value = webServer.arg(i);
    DPL_value[i] = S_value.toFloat();
  }
  for (int j = 0; j<DPL_max; j++) {
    DPL_id[j] = "";
    DPL_name[j] = "";
    DPL_value[j] = -1;
  }
  webServer.send(200, "text/plain", message);
}

void handleGet() {
  String message;
  for (int i = 0; i<UPL_number; i++) {
    message = UPL_id[i]+":"+UPL_name[i]+","+String(UPL_value[i]);
  }
  Serial.println("Send "+message);
  webServer.send(200, "text/plain", message);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  Id = "ESP"+String(ESP.getChipId());
  WiFi.softAP(Id.c_str(),"password");
  Serial.println("AP Web Server Start...");
  Serial.println(Id);
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
