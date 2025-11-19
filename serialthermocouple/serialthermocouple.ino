#include <WiFi.h>
#include <WebServer.h>
#include "max6675.h"
#include "html.h"   // contains html_page

// MAX6675 pins (SPI2 example)
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

// onboard LED (on many ESP32 dev boards this is GPIO 2;
// LED_BUILTIN should be defined by the core)
const int ledPin = LED_BUILTIN;

void MainPage() {
  String _html_page = html_page;   // html_page defined in html.h
  server.send(200, "text/html", _html_page);
}

void Web_Thermo() {
  // Take a sample *when this endpoint is called* (i.e. when the browser polls)
  digitalWrite(ledPin, HIGH);                 // LED on while sampling
  temp_C = thermocouple.readCelsius();
  temp_F = thermocouple.readFahrenheit();
  digitalWrite(ledPin, LOW);                  // LED off after sample

  Serial.print("Sample -> °C = ");
  Serial.print(temp_C);
  Serial.print(" | °F = ");
  Serial.println(temp_F);

  // JSON array: ["<C>","<F>"]
  String data = "[\"" + String(temp_C, 2) + "\",\"" + String(temp_F, 2) + "\"]";
  server.send(200, "application/json", data);
}

void setup(void){
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Starting ESP32 Thermocouple AP...");

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

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

  // No blocking delays here; sampling happens in Web_Thermo()
}
  