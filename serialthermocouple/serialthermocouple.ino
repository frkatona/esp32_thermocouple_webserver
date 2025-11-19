#include <WiFi.h>
#include <WebServer.h>
#include "max6675.h"
#include "html.h"   // contains html_page

// MAX6675 pins (SPI2 example from your comment)
int thermoDO  = 12;   // MISO
int thermoCS  = 15;   // CS
int thermoCLK = 14;   // SCLK

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

WebServer server(80);

const char* ssid     = "APK_Thermocouple";
const char* password = "APK_Thermocouple";

// use floating point so we don’t throw away precision
double temp_C = 0.0;
double temp_F = 0.0;

unsigned long lastReadMs = 0;
const unsigned long readIntervalMs = 1000;  // 1 second

void MainPage() {
  String _html_page = html_page;   // html_page defined in html.h
  server.send(200, "text/html", _html_page);
}

void Web_Thermo() {
  // JSON array: ["<C>","<F>"]
  String data = "[\"" + String(temp_C, 2) + "\",\"" + String(temp_F, 2) + "\"]";
  server.send(200, "application/json", data);
}

void setup(void){
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Starting ESP32 Thermocouple AP...");

  // Start as Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.print("AP SSID: ");
  Serial.println(ssid);
  Serial.print("AP IP:   ");
  Serial.println(WiFi.softAPIP());

  // Routes
  server.on("/", MainPage);
  server.on("/readWeb_Thermo", Web_Thermo);
  server.begin();
  Serial.println("HTTP server started");

  // Small warm-up delay for MAX6675
  delay(500);
}

void loop(void){
  // Always handle HTTP clients as fast as possible
  server.handleClient();

  // Read the thermocouple every readIntervalMs using millis()
  unsigned long now = millis();
  if (now - lastReadMs >= readIntervalMs) {
    lastReadMs = now;

    temp_C = thermocouple.readCelsius();
    temp_F = thermocouple.readFahrenheit();

    Serial.print("°C = ");
    Serial.println(temp_C);
    Serial.print("°F = ");
    Serial.println(temp_F);
  }
}
