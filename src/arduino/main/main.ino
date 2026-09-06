// main.ino - source code for APPS system
// Delveloped by Joseph Patrick
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <HX711.h>
#define ENABLE_SMTP
#define ENABLE_DEBUG
#include <ReadyMail.h>
#include <credentials.h>
#include <time_convert.h>
#include <test.h>

bool production = true;

// Pin definitions
#define AUGER_PIN 12
#define PUMP_PIN 13
#define WATER_SCALE_SCK 26
#define WATER_SCALE_DOUT 25
#define FOOD_SCALE_SCK 14
#define FOOD_SCALE_DOUT 27

// see calibrate() to get DIVIDER and OFFSET
#define FOOD_SCALE_OFFSET -368370.33
#define FOOD_SCALE_DIVIDER 429.94
#define WATER_SCALE_OFFSET 164968.66
#define WATER_SCALE_DIVIDER 442.57

// amount of food and water until full
#define FOOD_THRESHOLD_G 60
#define WATER_THRESHOLD_G 200

// how much food/water to give per cycle
#define FOOD_PRECISION 250 // ms of auger moving 
#define WATER_PRECISION 9000 // ms of water delivery
// too many cycles = out of food/water 
#define CYCLE_LIMIT 15

HX711 water_scale;
HX711 food_scale;

WiFiClientSecure ssl_client;
SMTPClient smtp(ssl_client);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "north-america.pool.ntp.org", -18000, 60000);

void scanAndPrintNetworks() {
  Serial.println("Scanning networks...");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("No networks found.");
    return;
  }
  for (int i = 0; i < n; i++) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i));
    Serial.print(" dBm, ch ");
    Serial.print(WiFi.channel(i));
    Serial.print(")");
    if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) Serial.print(" [OPEN]");
    Serial.println();
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  // Optionally: WiFi.disconnect(false); // don't erase credentials unless you need to

  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const unsigned long timeout = 30000; // 30 seconds
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED) {
    unsigned long elapsed = millis() - start;
    if (elapsed >= timeout) {
      Serial.println("\nWiFi connection timed out.");
      Serial.print("Final status: ");
      Serial.println(wifiStatusToString(WiFi.status()));
      // Optionally reboot or go into a safe loop
      while (true) {
        delay(5000);
        Serial.println("No WiFi – halting.");
      }
    }

    Serial.print(".");
    delay(500); // faster feedback, less blocking

    wl_status_t s = WiFi.status();
    if (s == WL_CONNECT_FAILED || s == WL_NO_SSID_AVAIL) {
      Serial.println("\nAuthentication or SSID failure.");
      Serial.print("Status: ");
      Serial.println(wifiStatusToString(s));
      // Break or reboot; don't loop forever
      while (true) {
        delay(5000);
        Serial.println("WiFi auth failed – check SSID/password and 2.4GHz.");
      }
    }
  }

  Serial.println("\nConnected to the Wi-Fi network: " + String(WiFi.SSID()));
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n\r---------------------------------------------\n\r"));

  pinMode(PUMP_PIN, OUTPUT);

  pinMode(AUGER_PIN, OUTPUT);

  water_scale.begin(WATER_SCALE_DOUT, WATER_SCALE_SCK);
  food_scale.begin(FOOD_SCALE_DOUT, FOOD_SCALE_SCK);
  water_scale.set_offset(WATER_SCALE_OFFSET);
  food_scale.set_offset(FOOD_SCALE_OFFSET);
  water_scale.set_scale(WATER_SCALE_DIVIDER);
  food_scale.set_scale(FOOD_SCALE_DIVIDER);

  if (!production) test_loop(timeClient, food_scale, water_scale, AUGER_PIN, PUMP_PIN);

  scanAndPrintNetworks();
  connectWiFi();

  timeClient.begin();

  ssl_client.setInsecure();
}

void send_email(String body) {
  auto statusCallback = [](SMTPStatus status) {
    Serial.println(status.text);
  };

  int connect_counter = 0;
  for (int i = 0; i < 10; i++) {
    smtp.connect(EMAIL_SMTP_HOST, EMAIL_SMTP_PORT, statusCallback);
    if (smtp.isConnected()) break;
  }

  if (!smtp.isConnected()) return;

  smtp.authenticate(EMAIL_FROM, EMAIL_PASSWORD, readymail_auth_password);

  SMTPMessage msg;
  msg.headers.add(rfc822_from, EMAIL_FROM);
  msg.headers.add(rfc822_to, EMAIL_TO);
  msg.headers.add(rfc822_to, EMAIL_TO_2);
  msg.headers.add(rfc822_subject, "APPS Notification");
  msg.text.body(body);

  configTime(0, 0, "pool.ntp.org");
  while (time(nullptr) < 100000) delay(100);
  msg.timestamp = time(nullptr);

  smtp.send(msg);
}

void loop() {
  timeClient.update();
  String times[2] = {"08:00", "18:00"}; // times to output provisions
  if (is_DST(timeClient.getEpochTime())) { timeClient.setTimeOffset(-14400); }
  else { timeClient.setTimeOffset(-18000); }
  String time = timeClient.getFormattedTime().substring(0,5);
  delay(10000);
  Serial.println(time);
  for (String t : times) { // loop through times and check if any of them are now
    if (t == time) {
      // add food until full
      int counter = 0; // counter if food runs out
      int bowlAmmount = food_scale.get_units(10);
      while (bowlAmmount < FOOD_THRESHOLD_G && counter < CYCLE_LIMIT) {
        Serial.print("Dispensing food: ");
        Serial.println(bowlAmmount);
        digitalWrite(AUGER_PIN, HIGH);
        delay(FOOD_PRECISION);
        digitalWrite(AUGER_PIN, LOW);
        delay(1000);
        counter++;
        bowlAmmount = food_scale.get_units(10);
      }

      // send email if refill needed
      if (counter >= CYCLE_LIMIT) send_email("Food Container is empty or obstruction detected, please remedy ASAP");

      // add water until full
      counter = 0; // reset counter
      bowlAmmount = water_scale.get_units(10);
      while (water_scale.get_units(10) < WATER_THRESHOLD_G && counter < CYCLE_LIMIT) {
        Serial.print("Dispensing water: ");
        Serial.println(bowlAmmount);
        digitalWrite(PUMP_PIN, HIGH);
        delay(WATER_PRECISION);
        digitalWrite(PUMP_PIN, LOW);
        counter++;
        bowlAmmount = water_scale.get_units(10);
      }

      // send email if refill needed
      if (counter >= CYCLE_LIMIT) send_email("Water Resivoir is empty or obstruction detected, please remedy ASAP");

      delay(60000); // wait for the minute to be over so it doesn't re-trigger
      timeClient.update(); // update to current time via NTP
    }
  }
}
