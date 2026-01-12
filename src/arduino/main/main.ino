// main.c - source code for APPS system
// Delveloped by Joseph Patrick
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP_Mail_Client.h>
#include <Stepper.h>
#include <HX711.h>

#define WIFI_SSID "Your Wi-Fi Name"
#define WIFI_PASSWORD "Your Wi-Fi Password"

#define EMAIL_FROM "Your email"
#define EMAIL_TO "Your email or elsewhere"
#define EMAIL_PASSWORD "Your email application password"
https://support.google.com/accounts/answer/185833?hl=en
#define EMAIL_SMTP_PORT YOUR_EMAIL_PROVIDERS_SMTP_PORT
#define EMAIL_SMTP_HOST "YOUR.EMAIL.PROVIDERS.SMTP.HOST"

#define STP_1 15
#define STP_2 2
#define STP_3 4
#define STP_4 16
#define PUMP_PIN 23

const String times[2] = {"07:00", "18:00"}; // times to output provisions

Stepper stepper(2048, STP_1, STP_2, STP_3, STP_4); // stepper step count & pins

SMTPSession smtp;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "north-america.pool.ntp.org", -18000, 60000);

void setup_wifi() { // connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) { // Wait for connection
    delay(500);
    Serial.print(".");
  }
  Serial.print("\n");
  Serial.print("Connected to the Wi-Fi network: ");
  Serial.println(WiFi.SSID());
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n\n---------------------------------------------"));
  setup_wifi();
  timeClient.begin();
  stepper.setSpeed(10);
  pinMode(PUMP_PIN, OUTPUT);
  timeClient.update();
}

void send_email(String text) {
  ESP_Mail_Session session;
  session.server.host_name = EMAIL_SMTP_HOST;
  session.server.port = EMAIL_SMTP_PORT;
  session.login.email = EMAIL_FROM;
  session.login.password = EMAIL_PASSWORD;
  SMTP_Message message;
  message.sender.email = EMAIL_FROM;
  message.subject = "APPS Notification";
  message.addRecipient("", EMAIL_TO);
  message.html.content = text;
  smtp.connect(&session);
  MailClient.sendMail(&smtp, &message);
}

void test_loop() {
  while (true) {
    digitalWrite(PUMP_PIN, HIGH);
    delay(1000);
    digitalWrite(PUMP_PIN, LOW);
    delay(1000);
    stepper.step(1024);
    delay(1000);
  }
}

void loop() {
  String time = timeClient.getFormattedTime().substring(0,5);
  Serial.println(time);
  test_loop();
  for (String t : times) { // loop through times and check if any of them are now
    if (t == time) {
      Serial.println("time triggered");
      // while (food is below threshold) {
      //   stepper.step(512);
      //   re-measure
      // }
      // if (food is empty) {
      //   send_email("Food is empty");
      // }
      //
      // while (water is below threshold) {
      //   digitalWrite(pump_pin, HIGH);
      //   re-measure;
      // }
      // digitalWrite(pump_pin, LOW);
      // if (water is empty) {
      //   send_email("Water is empty");
      // }

      delay(60000); // wait for the minute to be over so it doesn't re-trigger
      timeClient.update(); // update to current time
    }
  }
  delay(1000);
}
