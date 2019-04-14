#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#define USE_SERIAL Serial
int DPLINE_HOST_ID = 2; // Data Pipe Line HOST ID number
int Signal_V = (-50); // Working Signal Strength
String ESP_id[10];
String Sensor_name[10];
float Sensor_value[10];
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


ESP8266WiFiMulti WiFiMulti;

void setup() {
  Serial.begin(115200);
  pinMode(D7,OUTPUT);
  pinMode(D8,OUTPUT);
  digitalWrite(D7,LOW);
  digitalWrite(D8,LOW);
}

void loop() {
  digitalWrite(D7,HIGH);
  delay(5000);
  digitalWrite(D7,LOW);  
  String ESP_SSID;
  int numberOfNetworks = WiFi.scanNetworks();
  for(int i =0; i<numberOfNetworks; i++){
    Serial.print("Network name: ");
    Serial.println(WiFi.SSID(i));
    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(i));
    Serial.println("-----------------------");
    // except DPL_number>=300 bypass
    if ((WiFi.SSID(i).substring(0,3)=="ESP") && (WiFi.RSSI(i)>=Signal_V) && (DPL_number<DPL_max)) {
      Serial.println("Process...");
      ESP_SSID = WiFi.SSID(i);
      
      WiFi.mode(WIFI_STA);
      WiFi.begin(ESP_SSID.c_str(), "password");

      while(WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
      }
      Serial.println(".");
      // wait for WiFi connection
      if ((WiFi.status() == WL_CONNECTED)) {

        HTTPClient http;
        int post = 0;
        Serial.print("[HTTP] begin...\n");
        http.begin("http://www.esp.com/get");

        Serial.print("[HTTP] GET...\n");
        int httpCode = http.GET();
        if (httpCode > 0) {
          if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            int Sensor_number = 0;
            while(true) {
              int eline = payload.indexOf('\n');
              int colon = payload.indexOf(':');
              int comma = payload.indexOf(',');
              ESP_id[Sensor_number] = payload.substring(0,colon);
              Sensor_name[Sensor_number] = payload.substring(colon+1,comma);
              if (eline==(-1)) {
                Sensor_value[Sensor_number] = (payload.substring(comma+1)).toFloat();
              } else
              {
                Sensor_value[Sensor_number] = (payload.substring(comma+1,eline)).toFloat();
                payload = payload.substring(eline+1);                
              }
              Serial.print(ESP_id[Sensor_number]);
              Serial.print(':');
              Serial.print(Sensor_name[Sensor_number]);
              Serial.print('=');
              Serial.println(Sensor_value[Sensor_number]);
              // Fill Sensor Data to Data Pipe Line
              DPL_id[DPL_number] = ESP_id[Sensor_number];
              DPL_name[DPL_number] = Sensor_name[Sensor_number];
              DPL_value[DPL_number] = Sensor_value[Sensor_number];
              DPL_number++;
              // Check random number for Example
              if (Sensor_name[Sensor_number]=="number") {
                post=Sensor_value[Sensor_number];
              }
              Sensor_number++;
              if (eline==(-1)) {break;}
              if (Sensor_number>9) {break;}
              if (DPL_number>=DPL_max) {break;}
            }         
          }
        } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
        delay(10000);

        // Fill Example Data
        UPL_id[UPL_number] = ESP_SSID;
        UPL_name[UPL_number] = "number";
        UPL_value[UPL_number] = random(255);
        Serial.print("Example Data ");
        Serial.print(UPL_id[UPL_number]);
        Serial.print(':');
        Serial.print(UPL_name[UPL_number]);
        Serial.print('=');
        Serial.println(UPL_value[UPL_number]);
        UPL_number++;
        // Fill Example Data
        http.begin("http://www.esp.com/post");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        Serial.print("[HTTP] POST...\n");

        String PostData;
        for(i=0; i<UPL_number; i++) {
          if(ESP_SSID == UPL_id[i]) {
            String SUvalue = String(UPL_value[i]);
            PostData += UPL_name[i]+"="+SUvalue+"&";
            for(int j=i; j<UPL_number-1; j++) {
              UPL_id[j]=UPL_id[j+1];
              UPL_name[j]=UPL_name[j+1];
              UPL_value[j]=UPL_value[j+1];
            }
            UPL_id[UPL_number]="";
            UPL_name[UPL_number]="";
            UPL_value[UPL_number]=(-1);
            if(UPL_number>0) {
              UPL_number = UPL_number-1;
            }
          }
        }
        PostData = PostData.substring(0,PostData.length()-1); // Cut last &        
        
        httpCode = http.POST(PostData);
        if (httpCode > 0) {
          if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println(payload);
          }
        } else {
          Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();    
      }
      delay(10000);
    }
    // Data Pipe Line Section
    // DPLINE HOST ID+1 Next
    char pad[4]; // Max 4 digits
    sprintf(pad,"%04d",DPLINE_HOST_ID+1);
    String spad(pad);
    // except DPL_number>=1000 bypass
    if ((WiFi.SSID(i).substring(0,3)=="DPL") && (WiFi.SSID(i).substring(3,7)==spad) && DPL_number<DPL_max) {
      ESP_SSID = WiFi.SSID(i);
      
      WiFi.mode(WIFI_STA);
      WiFi.begin(ESP_SSID.c_str(), "password");
  
      // wait for WiFi connection
      if ((WiFi.status() == WL_CONNECTED)) {

        HTTPClient http;
        int post = 0;
        Serial.print("[HTTP] begin...\n");
        http.begin("http://www.esp.com/get");

        Serial.print("[HTTP] GET...\n");
        int httpCode = http.GET();
        if (httpCode > 0) {
          if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            // int DPL_number = 0;
            while(true) {
              int eline = payload.indexOf('\n');
              int colon = payload.indexOf(':');
              int comma = payload.indexOf(',');
              DPL_id[DPL_number] = payload.substring(0,colon);
              DPL_name[DPL_number] = payload.substring(colon+1,comma);
              if (eline==(-1)) {
                DPL_value[DPL_number] = (payload.substring(comma+1)).toFloat();
              } else
              {
                DPL_value[DPL_number] = (payload.substring(comma+1,eline)).toFloat();
                payload = payload.substring(eline+1);                
              }
              Serial.print(DPL_id[DPL_number]);
              Serial.print(':');
              Serial.print(DPL_name[DPL_number]);
              Serial.print('=');
              Serial.println(DPL_value[DPL_number]);
              // Check random number for Example
              if (DPL_name[DPL_number]=="number") {
                post=DPL_value[DPL_number];
              }
              DPL_number++;
              if (eline==(-1)) {break;}
              if (DPL_number>=DPL_max) {break;}
            }
          }
        } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
        delay(10000);
        String PostData;
        for(i=0; i<UPL_number; i++) {
          String SUvalue = String(UPL_value[i]);
          PostData += UPL_id[i]+"DPL"+UPL_name[i]+"="+SUvalue+"&";
        }
        PostData = PostData.substring(0,PostData.length()-1); // Cut last &
        http.begin("http://www.esp.com/post");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        Serial.print("[HTTP] POST...\n");
        httpCode = http.POST(PostData);
        if (httpCode > 0) {
          if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println(payload);
          }
          for (i=0; i<UPL_number; i++) {
            UPL_id[i] = "";
            UPL_name[i] = "";
            UPL_value[i] = 0.00;
          }
          UPL_number = 0;
        } else {
          Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();    
      }
      delay(10000);  
    }    
    // DPLINE my HOST ID
    char mpad[4]; // Max 4 digits
    sprintf(mpad,"%04d",DPLINE_HOST_ID);
    String mspad(mpad);
    if ((WiFi.SSID(i).substring(0,3)=="DPL") && (WiFi.SSID(i).substring(3,7)==mspad) && (UPL_number<=UPL_max)) {
      ESP_SSID = WiFi.SSID(i);
      
      WiFi.mode(WIFI_STA);
      WiFi.begin(ESP_SSID.c_str(), "password");
  
      // wait for WiFi connection
      if ((WiFi.status() == WL_CONNECTED)) {

        HTTPClient http;
        int post = 0;
        Serial.print("[HTTP] begin...\n");
        http.begin("http://www.esp.com/get");

        Serial.print("[HTTP] GET...\n");
        int httpCode = http.GET();
        if (httpCode > 0) {
          if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            while(true) {
              int eline = payload.indexOf('\n');
              int colon = payload.indexOf(':');
              int comma = payload.indexOf(',');
              UPL_id[UPL_number] = payload.substring(0,colon);
              UPL_name[UPL_number] = payload.substring(colon+1,comma);
              if (eline==(-1)) {
                UPL_value[UPL_number] = (payload.substring(comma+1)).toFloat();
              } else
              {
                UPL_value[UPL_number] = (payload.substring(comma+1,eline)).toFloat();
                payload = payload.substring(eline+1);                
              }
              Serial.print(UPL_id[UPL_number]);
              Serial.print(':');
              Serial.print(UPL_name[UPL_number]);
              Serial.print('=');
              Serial.println(UPL_value[UPL_number]);
              UPL_number++;
              if (eline==(-1)) {break;}
              if (UPL_number>=UPL_max) {break;}
            }
          }
        } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
        delay(10000);
        String PostData;
        for(i=0; i<DPL_number; i++) {
          String SDvalue = String(DPL_value[i]);
          PostData += DPL_id[i]+"DPL"+DPL_name[i]+"="+SDvalue+"&";
        }
        PostData = PostData.substring(0,PostData.length()-1); // Cut last &
        http.begin("http://www.esp.com/post");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        Serial.print("[HTTP] POST...\n");
        httpCode = http.POST(PostData);
        if (httpCode > 0) {
          if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println(payload);
          }
          for (i=0; i<DPL_number; i++) {
            DPL_id[i] = "";
            DPL_name[i] = "";
            DPL_value[i] = 0.00;
          }
          DPL_number = 0;
        } else {
          Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();    
      }
      delay(10000);  
    }      
  }
}
