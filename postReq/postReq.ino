#include <WiFi.h>
#include <HTTPClient.h>
  
const char* ssid = "q";
const char* password =  "12345678";
  
void setup() {
  
  Serial.begin(115200);
  delay(4000);   //Delay needed before calling the WiFi.begin
  
  WiFi.begin(ssid, password); 
  
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  
  Serial.println("Connected to the WiFi network");
  
}
  
void loop() {
  
 if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
  
   HTTPClient http;   
  
   http.begin("https://geepstracker.herokuapp.com/add/61b5e0b4b4b53c922f763799");  //Specify destination for HTTP request
//   http.addHeader("Content-Type", "text/plain");             //Specify content-type header
    // If you need an HTTP request with a content type: application/json, use the following:
   http.addHeader("Content-Type", "application/json");
   int httpResponseCode = http.POST("{\"longitude\":\"4\",\"latitude\":\"7\"}");
  
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
  
  delay(60000);  //Send a request every 10 seconds
  
}
