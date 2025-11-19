// this example is public domain. enjoy!
// https://learn.adafruit.com/thermocouple/

/*
____________________________________________________
 SPI pin name | ESP32 pin (SPI2) | ESP32 pin (SPI3)|
----------------------------------------------------
     CS      |       15         |        5       |
     SCLK    |       14         |      18        |
     MISO    |       12         |      19        |
     MOSI    |       13         |      23        |
----------------------------------------------------
*/

#include <WiFi.h>
#include <WebServer.h>
#include "html.h"
#include "max6675.h"

int thermoDO = 12;
int thermoCS = 15;
int thermoCLK = 14;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

WebServer server(80);

const char* ssid = "APK_Thermocouple";         /*Enter Your SSID*/
const char* password = "APK_Thermocouple"; /*Enter Your Password*/

long temp_C, temp_F;

void MainPage() {
  String _html_page = html_page;              /*Read The HTML Page*/
  server.send(200, "text/html", _html_page);  /*Send the code to the web server*/
}

void Web_Thermo() {
  String data = "[\"" + String(temp_C) + "\",\"" + String(temp_F) + "\"]";
  server.send(200, "application/json", data);  // or "text/plain"
}

void setup(void){
  Serial.begin(115200);

  // Start ESP32 as Wi-Fi Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.print("Access Point started: ");
  Serial.println(ssid);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Web server routes
  server.on("/", MainPage);                 // HTML page
  server.on("/readWeb_Thermo", Web_Thermo); // JSON temperature endpoint
  server.begin();
  Serial.println("HTTP server started");

  delay(1000);
}


void loop(void){
  // For the MAX6675 to update, you must delay AT LEAST 250ms between reads!
  temp_C = thermocouple.readCelsius();    /*Read Temperature on °C*/
  temp_F = thermocouple.readFahrenheit(); /*Read Temperature on °F*/
  Serial.print("°C = "); 
  Serial.println(temp_C);
  Serial.print("°F = ");
  server.handleClient();
  delay(1000);                            /*Wait for 1000mS*/
}