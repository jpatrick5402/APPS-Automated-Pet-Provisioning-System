// main.c - source code for APPS system
// Delveloped by Joseph Patrick
#include <WiFi.h>
#include <WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Stepper.h>
#include <ESP_Mail_Client.h>

const String times[] = {"07:00", "18:00"};

const int pump_pin = 32;

Stepper stepper(2048, 13,12,14,27); // stepper step count & pins

const char* ssid = "";
const char* wifi_password = "";

const char* from_email = "";
const char* email_password = "";
const char* to_email = "";

WebServer server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "north-america.pool.ntp.org", -18000, 60000);

void setup_wifi() { // connect to WiFi
  WiFi.begin(ssid, wifi_password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) { // Wait for connection
    delay(500);
    Serial.print(".");
  }
  Serial.print("\n");
  Serial.print("\nConnected to the Wi-Fi network: ");
  Serial.println(WiFi.SSID());
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void setup_email() { // send an email to admin if error or empty

}

void render_root() {
  char* content = "<html><p>Hello There</p></html>";
  server.send(200, "text/html", content);
}

void setup_web_server() {
  server.on("/", render_root);
  Serial.println("Starting web server");
  server.begin();
  Serial.println("Web server initialized");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n---------------------------------------------");
  setup_wifi();
  setup_email();
  setup_web_server();
  timeClient.begin();
  stepper.setSpeed(10);
  pinMode(pump_pin, OUTPUT);
}

void loop() {
  server.handleClient();
  timeClient.update(); // update and store the current time
  String time = timeClient.getFormattedTime().substring(0,5);
  Serial.println(time);

  bool isActivationTime = false;
  for (String t : times) { // loop through times and check if any of them are now
    if (t == time) isActivationTime = true;
  }

  if (isActivationTime) {
    Serial.println("time triggered");
    // while (food is below threshold) {
    //   stepper.step(512);
    //   re-measure
    // }
    //
    // while (water is below threshold) {
    //   digitalWrite(pump_pin, HIGH);
    //   re-measure;
    // }
    // digitalWrite(pump_pin, LOW);

    delay(60000); // wait for the minute to be over so it doesn't re-trigger
  } else {
    delay(3000); // wait 3 seconds to check if we're now in trigger minute
                 // 3 seconds because of web server handleClient()
  }
}
