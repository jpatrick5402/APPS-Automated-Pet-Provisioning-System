// main.c - source code for APPS system
// Delveloped by Joseph Patrick
#include <WiFi.h>
#include <WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Stepper.h>
#include <ESP_Mail_Client.h>

Stepper stepper(2048, 19, 21, 22, 23);

const char* ssid = "";
const char* wifi_password = "";

const char* from_email = "";
const char* email_password = "";
const char* to_email = "";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "north-america.pool.ntp.org", -18000, 60000);

void setup_wifi() { // connect to WiFi
  WiFi.begin(ssid, wifi_password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) { // Wait for connection
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConnected to the Wi-Fi network: ");
  Serial.println(WiFi.SSID());
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void setup_email() { // send an email to admin if error or empty

}

void setup_web_server() {

}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  setup_email();
  setup_web_server();
  timeClient.begin();
  stepper.setSpeed(10);
}

void loop() {
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  stepper.step(1024);
  delay(1000);
}