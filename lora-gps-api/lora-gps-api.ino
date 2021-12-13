/*****************************************
  ESP32 GPS VKEL 9600 Bds
This version is for T22_v01 20190612 board
As the power management chipset changed, it
require the axp20x library that can be found
https://github.com/lewisxhe/AXP202X_Library
You must import it as gzip in sketch submenu
in Arduino IDE
This way, it is required to power up the GPS
module, before trying to read it.

Also get TinyGPS++ library from: 
https://github.com/mikalhart/TinyGPSPlus
******************************************/
#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <axp20x.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h> 
#include <ArduinoJson.h>
#include <Arduino_JSON.h>
const char* ssid = "q";
const char* password =  "12345678";
#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND    868E6
String latitude;
String longitude;
TinyGPSPlus gps;
HardwareSerial GPS(1);
AXP20X_Class axp;

int counter = 0; 
void setup()
{
  Serial.begin(9600);
  delay(4000);   //Delay needed before calling the WiFi.begin

  Wire.begin(21, 22);
  if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS)) {
    Serial.println("AXP192 Begin PASS");
  } else {
    Serial.println("AXP192 Begin FAIL");
  }
  axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
  axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
  axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
  GPS.begin(9600, SERIAL_8N1, 34, 12);   //17-TX 18-RX

    while (!Serial);

  Serial.println("LoRa Sender");
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);  


  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
   LoRa.receive();
  Serial.println("init ok");
  
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  
  Serial.println("Connected to the WiFi network");
}

void loop()
{
  if(WiFi.status()== WL_CONNECTED){  
  Serial.print("Sending packet: ");
 
  String latitude = String(gps.location.lat(),5);
  String longitude = String(gps.location.lng(),4);
  Serial.print("Latitude  : ");
  Serial.println(gps.location.lat(), 5);
  Serial.print("Longitude : ");
  Serial.println(gps.location.lng(), 4);
  Serial.println(counter);
  

  // send packet
  LoRa.beginPacket();
  LoRa.print("Longitude is: ");
  LoRa.print(gps.location.lng(), 4);
   LoRa.print("Latitude is: ");
  LoRa.print(gps.location.lat(), 5);
  LoRa.endPacket();

  counter++;

    
  //Check WiFi connection status
       
    StaticJsonDocument<200> doc;
    // Add values in the document
    //
    doc["longitude"] = longitude;
    doc["latitude"] =latitude ;
       String requestBody;
    serializeJson(doc, requestBody);
   HTTPClient http;   
   
     
   
   http.begin("https://geepstracker.herokuapp.com/add/61b7946f19ea920268bb6dbd");  //Specify destination for HTTP request
//   http.addHeader("Content-Type", "text/plain");             //Specify content-type header
    // If you need an HTTP request with a content type: application/json, use the following:
   http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(requestBody);
//   int httpResponseCode = http.POST("{\"longitude\":\"4Gps\",\"latitude\":\"7Gps\"}");
  
//   int httpResponseCode = http.POST("POSTING from ESP32");   //Send the actual POST request
  
   if(httpResponseCode>0){
  
    String response = http.getString();                       //Get the response to the request
  
    Serial.println(httpResponseCode);   //Print return code
    Serial.println(response);           //Print request answer
  
   }else{
  
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  
   }
  
   http.end();  //Free resources
  
 }else{
  
    Serial.println("Error in WiFi connection");   
  
 }
  
//  delay(10000);  

  smartDelay(10000);//Send a request every 10 seconds

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (GPS.available())
      gps.encode(GPS.read());
  } while (millis() - start < ms);
}
