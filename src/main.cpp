#include <Arduino.h>
#include <WiFiNINA.h>
#include <LSM6DS3.h>

// Create WiFI AP 
char ssid[] = "Nano33";
char password[] = "sussusamogus";

WiFiServer server(80);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  asm(".global _printf_float"); // enable pretty-printing floats
  Serial.begin(115200);
  int8_t status = WiFi.beginAP(ssid, password);
  if (status != WL_AP_LISTENING) {
    Serial.println("Failed to setup access point");
    while (true);
  }
  Serial.println("Established hotspot, listening for TCP connections");
  server.begin();
  while(!IMU.begin()) {delay(10);}
  Serial.println("Established IMU");
}

void loop() {
  static float ax, ay, az, rx, ry, rz;
  static char print_buffer[100];
  Serial.println("about to call server.available...");
  WiFiClient client = server.available();
  Serial.println("obtained client!");
  if (client && client.connected()) {
    client.print("WiFi strength (RSSI): ");
    client.println(WiFi.RSSI());
  } 
  while (client && client.connected()) {
    Serial.println(client.connected());
    while (!IMU.accelerationAvailable() || !IMU.gyroscopeAvailable()) {
      delay(10);
    }
    if (!IMU.readAcceleration(ax, ay, az)) {
      Serial.println("Failed to read acceleration"); return;
    }
    if (!IMU.readGyroscope(rx, ry, rz)) {
      Serial.println("Failed to read gyroscope"); return;
    }
    snprintf(print_buffer, 100, "ax: %.2f, ay: %.2f, az: %.2f, rx: %.2f ry: %.2f, rz: %.2f", ax, ay, az, rx, ry, rz);
    client.println(print_buffer);
    while (client.available()) {
      char next = client.read();
      if (next == '1') {
        digitalWrite(LED_BUILTIN, HIGH);
      } else if (next == '0') {
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
    delay(500);
  } 
  if (client) {
    client.stop();
  }
  delay(500);
}

