/**
 *  PIN MAP for ESP32 NODEMCU-32S, other ESP32 dev boards will vary
 *  Keypad (12-32
 *  1.8 128/160 TFT PIN MAP: [VCC - 5V, GND - GND, CS - GPIO5, Reset - GPIO16, AO (DC) - GPI17, SDA (MOSI) - GPIO23, SCK - GPIO18, LED - 3.3V]
 */

#include <Keypad.h>
#include <string.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <math.h>
#include <TFT_eSPI.h> 
#include "qrcode.h"

TFT_eSPI tft = TFT_eSPI(); 

//Details to change
char wifiSSID[] = "YOUR-WIFI";
char wifiPASS[] = "YOUR-PASS";
const char* lntxbothost = "paywall.link";
String invoicekey = "YOUR-PAYWALL-KEY"; 
String invoicenum = "INVOICE-NUMBER"; //Create a paywall, go to "Details" of the paywall https://paywall.link/dashboard/paywalls, number in the URL, ie 931 from https://paywall.link/link/view?id=931 
String memo = "Q1 "; //memo suffix, followed by a random number
String on_currency = "BTCGBP"; //currency can be changed here ie BTCUSD BTCGBP etc
String payid;

String pubkey;
String totcapacity;
const char* payment_request;
bool certcheck = false;

//LNTXBOT DETAILS
//
int httpsPort = 443;


String choice;

String on_sub_currency = on_currency.substring(3);

  String key_val;
  String cntr = "0";
  String inputs;
  int keysdec;
  int keyssdec;
  float temp;  
  String fiat;
  float satoshis;
  String nosats;
  float conversion;
  String postid;
  String data_id;
  String data_lightning_invoice_payreq = "";
  String data_status;
  bool settle = false;
  String payreq;
  String hash;
  String virtkey;
  

void setup() {
  tft.begin();
  Serial.begin(115200);
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(3);
 
  //connect to local wifi            
  WiFi.begin(wifiSSID, wifiPASS);   
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if(i >= 5){
     tft.fillScreen(TFT_BLACK);
     tft.setCursor(55, 20);
     tft.setTextSize(1);
     tft.setTextColor(TFT_RED);
     tft.println("WIFI NOT CONNECTED");
    }
    delay(1000);
    i++;
  }

  page_nodecheck();
  on_rates();
//  nodecheck();
}

void loop() {

        addinvoice(nosats);

        showAddress(payreq);
        checkpayment();
        
        int counta = 0;
         while (settle != true){
           counta++;

            checkpayment();
             if (settle == true){
              tft.fillScreen(TFT_BLACK);
              tft.setCursor(52, 40);
              tft.setTextSize(1);
              tft.setTextColor(TFT_GREEN);
              tft.println("COMPLETE");
              delay(1000);
              cntr = true;
             }

           if(counta >20){
            settle = true;
            tft.fillScreen(TFT_BLACK);
            cntr = true;
           }
           delay(1000);
         }
}
      
      else if (virtkey == "*"){
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.setTextColor(TFT_WHITE);
        key_val = "";
        inputs = "";  
        nosats = "";
        virtkey = "";
        cntr = "2";
      }
      displaysats();    
    }
  }
}



void page_processing()
{ 
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(49, 40);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.println("PROCESSING");
 
}

void page_nodecheck()
{ 
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(49, 40);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.println("INITIALISING");
 
}




//OPENNODE REQUESTS

void on_rates(){
  WiFiClientSecure client;
  if (!client.connect("api.opennode.co", httpsPort)) {
    return;
  }

  String url = "/v1/rates";
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.opennode.co\r\n" +
               "User-Agent: ESP32\r\n" +
               "Connection: close\r\n\r\n");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {

      break;
    }
  }
  String line = client.readStringUntil('\n');
    const size_t capacity = 169*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(168) + 3800;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, line);
    conversion = doc["data"][on_currency][on_currency.substring(3)]; 

}


void addinvoice(String nosats){

 WiFiClientSecure client;
  
  if (!client.connect(lntxbothost, httpsPort)) {
    Serial.println("fail");
    return;
    
  }
  
  String topost = "{  \"num_satoshis\" : " + nosats +", \"memo\" :\""+ memo + String(random(1,1000)) + "\"}";
  
  String url = "/v1/user/paywall/" + invoicenum + "/invoice";
  client.print(String("POST ") + url + "?access-token=" + invoicekey + " HTTP/1.1\r\n" +
                "Host: " + lntxbothost + "\r\n" +
                "User-Agent: ESP32\r\n" +
                "Content-Type: application/json\r\n" +
                "Connection: close\r\n" +
                "Content-Length: " + topost.length() + "\r\n" +
                "\r\n" + 
                topost + "\n");

  
  while (client.connected()) {
    String line = client.readStringUntil('\n');
   // Serial.println(line);
    if (line == "\r") {
      break;
    }
  }
  
  String line = client.readStringUntil('\n');
  line = client.readStringUntil('\n');
  Serial.println(line);

  
const size_t capacity = JSON_OBJECT_SIZE(17) + 500;

DynamicJsonDocument doc(capacity);
deserializeJson(doc, line);

const char* payment_request = doc["request"]; 
int id = doc["id"]; 
payreq = (String)payment_request;
payid = (String)id;
Serial.println(payreq);
Serial.println(payid);

}


void checkpayment(){

 WiFiClientSecure client;
  
  if (!client.connect(lntxbothost, httpsPort)) {
    Serial.println("fail");
    return;
    
  }
 
  String url = "/v1/user/invoice/" + payid;
  client.print(String("GET ") + url + "?access-token=" + invoicekey + " HTTP/1.1\r\n" +
                "Host: " + lntxbothost + "\r\n" +
                "User-Agent: ESP32\r\n" +
                "Content-Type: application/json\r\n" +
               "Connection: close\r\n\r\n");
    Serial.println(String("GET ") + url + "?access-token=" + invoicekey + " HTTP/1.1\r\n" +
                "Host: " + lntxbothost + "\r\n" +
                "User-Agent: ESP32\r\n" +
                "Content-Type: application/json\r\n" +
               "Connection: close\r\n\r\n");
  


  
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    
    if (line == "\r") {
      break;
      Serial.println(line);
    }
  }
  
  String line = client.readStringUntil('\n');
  line = client.readStringUntil('\n');
  Serial.println(line);
const size_t capacity = JSON_OBJECT_SIZE(9) + 400;
DynamicJsonDocument doc(capacity);


deserializeJson(doc, line);

int settled = doc["settled"]; 
Serial.println(settled);
if (settled == 1){
  settle = true;
}
else{
  settle = false;
}
}

void showAddress(String XXX){
  tft.fillScreen(TFT_WHITE);
  XXX.toUpperCase();
 const char* addr = XXX.c_str();
 Serial.println(addr);
  int qrSize = 12;
  int sizes[17] = { 14, 26, 42, 62, 84, 106, 122, 152, 180, 213, 251, 287, 331, 362, 412, 480, 504 };
  int len = String(addr).length();
  for(int i=0; i<17; i++){
    if(sizes[i] > len){
      qrSize = i+1;
      break;
    }
  }
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(qrSize)];
  qrcode_initText(&qrcode, qrcodeData, qrSize, ECC_LOW, addr);
  Serial.println(qrSize );
 
  float scale = 2;

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if(qrcode_getModule(&qrcode, x, y)){       
        tft.drawRect(15+3+scale*x, 3+scale*y, scale, scale, TFT_BLACK);
      }
      else{
        tft.drawRect(15+3+scale*x, 3+scale*y, scale, scale, TFT_WHITE);
      }
    }
  }
}
