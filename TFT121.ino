/**
 *  TFT121LNPAY
 */

#include <Keypad.h>
#include <string.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <math.h>
#include <TFT_eSPI.h> 
#include "qrcode.h"

TFT_eSPI tft = TFT_eSPI(); 

///WIFI Setup
char wifiSSID[] = "<your_wifi_ssid>";
char wifiPASS[] = "<your_wifi_pass>";

//API Setup
String api_key = "<api_key_goes_here>"; // Can be found here: https://lnpay.co/dashboard/integrations
String wallet_key = "<wi_XXXXX_key_goes_here>"; // Can be found here: https://lnpay.co/dashboard/advanced-wallets


//Payment Setup
String memo = "M5 "; //memo suffix, followed by a random number
String nosats = "50";

//Endpoint setup
const char* api_endpoint = "https://lnpay.co/v1";
String invoice_create_endpoint = "/user/wallet/" + wallet_key + "/invoice";
String invoice_check_endpoint = "/user/lntx/"; //append LNTX ID to the end (e.g. /user/lntx/lntx_mfEKSse22)


String lntx_id;
String choice = "";

String key_val;
String cntr = "0";
String inputs = "";
int keysdec;
int keyssdec;
float temp;  
String fiat;
float satoshis;
float conversion;
bool settle = false;
String payreq = "";


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

  pinMode(21, OUTPUT);
  digitalWrite(21, LOW);
  
}

void loop() {
 
 reqinvoice(nosats);
 page_qrdisplay(payreq);
 checkpayment();
 int counta = 0;
 while (settle != true){
   checkpayment();
   key_val = "";
   inputs = "";
   if(counta >200){
    settle = true;
   }
   counta++;
   delay(1000);
  } 
  digitalWrite(21, HIGH);
  delay(10000);
  digitalWrite(21, LOW);
}


/**
 * Request an invoice
 */
void reqinvoice(String value){

  String payload;
  HTTPClient http;
  http.begin(api_endpoint + invoice_create_endpoint + "?fields=payment_request,id"); //Getting fancy to response size
  http.addHeader("Content-Type","application/json");
  http.addHeader("X-Api-Key",api_key);
  String toPost = "{  \"num_satoshis\" : " + nosats +", \"memo\" :\""+ memo + String(random(1,1000)) + "\"}";
  int httpCode = http.POST(toPost); //Make the request
  
  if (httpCode > 0) { //Check for the returning code
      payload = http.getString();
      Serial.println(payload);
    }
  else {
    Serial.println("Error on HTTP request");
  }
  http.end(); //Free the resources

  Serial.println(payload);

  
  const size_t capacity = JSON_OBJECT_SIZE(2) + 500;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, payload);
  
  const char* payment_request = doc["payment_request"]; 
  const char* id = doc["id"]; 
  payreq = (String) payment_request;
  lntx_id = (String) id;
  Serial.println(payreq);
  Serial.println(lntx_id);
}


void checkpayment(){
  String payload;
  HTTPClient http;
  http.begin(api_endpoint + invoice_check_endpoint + lntx_id + "?fields=settled"); //Getting fancy to response size
  http.addHeader("Content-Type","application/json");
  http.addHeader("X-Api-Key",api_key);
  int httpCode = http.GET(); //Make the request
  
  if (httpCode > 0) { //Check for the returning code
      payload = http.getString();
      Serial.println(payload);
    }
  else {
    Serial.println("Error on HTTP request");
  }
  http.end(); //Free the resources
  
  const size_t capacity = JSON_OBJECT_SIZE(2) + 500;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, payload);
  
  int settled = doc["settled"]; 
  Serial.println(settled);
  if (settled == 1){
    settle = true;
  }
  else{
    settle = false;
  }
}

void page_qrdisplay(String xxx)
{  
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

