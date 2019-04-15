/*********************************
 * ESP8266 DPL                   *
 * ESP8266 AP Clone WiFi and JS  *
 *                               *
 * 14 Apr 2019                   *
 * Supot Sawangpiriyakij         *
 * Bunnavit Sawangpiriyakij      *
 *********************************/
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <FS.h>   //Include File System Headers
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 2);
DNSServer dnsServer;
ESP8266WebServer webServer(80);
String Id = "ServerExample";
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
int RCV_number = 0;

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
      RCV_number++;
    }
  }
  webServer.send(200, "text/plain", "DPL Post done");

  // for Example
  // We move all DPL data back to UPL data
  // in this section
  digitalWrite(D7,HIGH);
  for (int j = 0; j<DPL_number; j++) {
    UPL_id[UPL_number] = DPL_id[j];
    UPL_name[UPL_number] = DPL_name[j];
    UPL_value[UPL_number] = DPL_value[j];
    UPL_number++;
  }
  // and clear DPL data
  for(int j = 0; j<DPL_max; j++) {
    DPL_id[j] = "";
    DPL_name[j] = "";
    DPL_value[j] = -1;
  }
  DPL_number = 0;
  // for Example

  delay(delay_time);
  digitalWrite(D8,LOW);
}

void handleGet() {
  Serial.println("Handle Get");
  digitalWrite(D7,HIGH);
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

void handleRoot() {
  String html;
  html+="<!doctype html>";
  html+="<html>";
  html+="<head>";
  html+="    <meta charset='utf-8' />";
  html+="    <title>Counter</title>";
  html+="    <meta name='viewport' content='width=device-width'>";
  html+="    <style>";
  html+="    .container {";
  html+="        width: 450px;";
  html+="        margin: 0 auto;";
  html+="        text-align: center;";
  html+="    }";
  html+="    .gauge {";
  html+="        width: 450px;";
  html+="        height: 450px;";
  html+="    }";
  html+="    a:link.button,";
  html+="    a:active.button,";
  html+="    a:visited.button,";
  html+="    a:hover.button {";
  html+="        margin: 30px 5px 0 2px;";
  html+="        padding: 7px 13px;";
  html+="    }";
  html+="    </style>";
  html+="</head>";
  html+="<body>";
  html+="    <div class='container'>";
  html+="        <div id='g1' class='gauge'></div>";
  html+="        <a href='#' id='g1_refresh'>Refresh</a>";
  html+="    </div>";
  html+="    <script src='raphael-2.1.4.min.js'></script>";
  html+="    <script src='justgage.js'></script>";
  html+="    <script>";
  html+="    var g1;";
  html+="    document.addEventListener('DOMContentLoaded', function(event) {";
  html+="        g1 = new JustGage({";
  html+="            id: 'g1',";
  html+="            value: "+String(RCV_number+100)+",";
  html+="            min: 0,";
  html+="            max: 1000,";
  html+="            donut: true,";
  html+="            gaugeWidthScale: 0.6,";
  html+="            counter: true,";
  html+="            hideInnerShadow: true";
  html+="        });";
  html+="        document.getElementById('g1_refresh').addEventListener('click', function() {";
  html+="            g1.refresh("+String(RCV_number+100)+");";
  html+="        });";
  html+="    });";
  html+="    </script>";
  html+="</body>";
  html+="</html>";
  webServer.send(200, "text/html", html);
}

void handleWebRequests(){
  Serial.println("SPIFF");
  if(loadFromSpiffs(webServer.uri())) return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";
  for (uint8_t i=0; i<webServer.args(); i++){
    message += " NAME:"+webServer.argName(i) + "\n VALUE:" + webServer.arg(i) + "\n";
  }
  webServer.send(404, "text/plain", message);
  Serial.println(message);
}

void setup() {
  Serial.begin(115200);
  SPIFFS.begin();
  Serial.println("File System Initialized");
  pinMode(D7,OUTPUT);
  pinMode(D8,OUTPUT);
  digitalWrite(D7,LOW);
  digitalWrite(D8,LOW);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(Id.c_str(),"password");
  Serial.println("");
  Serial.println("AP Web Server Start...");
  Serial.println(Id);
  Serial.println(WiFi.softAPIP());
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(DNS_PORT, Id_URL, apIP);
  webServer.on("/post", handlePost); 
  webServer.on("/get", handleGet);
  webServer.on("/", handleRoot);
  webServer.onNotFound(handleWebRequests);
  webServer.begin();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}

bool loadFromSpiffs(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";
 
  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".html")) dataType = "text/html";
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (webServer.hasArg("download")) dataType = "application/octet-stream";
  if (webServer.streamFile(dataFile, dataType) != dataFile.size()) {
  }
 
  dataFile.close();
  return true;
}
