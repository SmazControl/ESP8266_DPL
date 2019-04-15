/*********************************
 * ESP8266 DPL                   *
 * ESP8266 AP Extender           *
 *                               *
 * 15 Apr 2019                   *
 * Supot Sawangpiriyakij         *
 * Bunnavit Sawangpiriyakij      *
 *********************************
 * Data Pipe Line HOST ID number */
/**/ int DPLINE_HOST_ID = 1;   /**
 *********************************/

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 2);
DNSServer dnsServer;
ESP8266WebServer webServer(80);
String Id = "DPL";
String Id_URL = "www.esp.com";
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
int delay_time = 10000;

void handlePost() {
  Serial.println("Handle Post");
  digitalWrite(D8,HIGH);
  for (int i = 0; i < webServer.args(); i++) {
    Serial.println("DPL "+String(webServer.argName(i))+","+String(webServer.arg(i)));
    String S_name = webServer.argName(i);
    int colon = S_name.indexOf('DPL');
    if (colon != (-1)) {
      DPL_id[i] = S_name.substring(0,colon-2);
      DPL_name[i] = S_name.substring(colon+1);
      String S_value = webServer.arg(i);
      DPL_value[i] = S_value.toFloat();
      DPL_number++;
    }
  }
  webServer.send(200, "text/plain", "DPL Post done");
  if (DPL_number == 0) {
    digitalWrite(D8,LOW);    
  }
}

void handleGet() {
  Serial.println("Handle Get");
  String message;
  for (int i = 0; i<UPL_number; i++) {
    message += UPL_id[i]+":"+UPL_name[i]+","+String(UPL_value[i])+"\n";
    Serial.println("UPL "+UPL_id[i]+":"+UPL_name[i]+","+String(UPL_value[i]));
  }
  message = message.substring(0,message.length()-1);
  webServer.send(200, "text/plain", message);
  for(int j = 0; j<UPL_max; j++) {
    UPL_id[j] = "";
    UPL_name[j] = "";
    UPL_value[j] = -1;
  }
  UPL_number = 0;
  delay(delay_time);
  digitalWrite(D7,LOW);
}

void handlePostNext() {
  Serial.println("Handle Post Next");
  digitalWrite(D7,HIGH);
  for (int i = 0; i < webServer.args(); i++) {
    Serial.println("UPL "+String(webServer.argName(i))+","+String(webServer.arg(i)));
    String S_name = webServer.argName(i);
    int colon = S_name.indexOf('UPL');
    if (colon != (-1)) {
      UPL_id[i] = S_name.substring(0,colon-2);
      UPL_name[i] = S_name.substring(colon+1);
      String S_value = webServer.arg(i);
      UPL_value[i] = S_value.toFloat();
      UPL_number++;
    }
  }
  webServer.send(200, "text/plain", "UPL Post Next done");
  if (UPL_number == 0) {
    digitalWrite(D7,LOW);    
  }
}

void handleGetNext() {
  Serial.println("Handle Get Next");
  String message;
  for (int i = 0; i<DPL_number; i++) {
    message += DPL_id[i]+":"+DPL_name[i]+","+String(DPL_value[i])+"\n";
    Serial.println("DPL "+DPL_id[i]+":"+DPL_name[i]+","+String(DPL_value[i]));
  }
  message = message.substring(0,message.length()-1);
  webServer.send(200, "text/plain", message);
  for(int j = 0; j<DPL_max; j++) {
    DPL_id[j] = "";
    DPL_name[j] = "";
    DPL_value[j] = -1;
  }
  DPL_number = 0;
  delay(delay_time);
  digitalWrite(D8,LOW);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  char pad[4]; // Max 4 digits
  sprintf(pad,"%04d",DPLINE_HOST_ID);
  String spad(pad);
  Id += spad;
  WiFi.softAP(Id.c_str(),"password");
  Serial.println("AP Web Server Start...");
  Serial.println(Id);
  Serial.println(WiFi.softAPIP());
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(DNS_PORT, Id_URL, apIP);
  webServer.on("/post", handlePost); 
  webServer.on("/get", handleGet);
  webServer.on("/pnext", handlePostNext); 
  webServer.on("/gnext", handleGetNext);  
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
