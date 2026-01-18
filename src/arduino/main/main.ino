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
// https://support.google.com/accounts/answer/185833?hl=en
#define EMAIL_SMTP_PORT YOUR_EMAIL_PROVIDERS_SMTP_PORT
#define EMAIL_SMTP_HOST "YOUR.EMAIL.PROVIDERS.SMTP.HOST"

#define STP_1 15
#define STP_2 2
#define STP_3 4
#define STP_4 16
#define PUMP_PIN 23
#define WATER_SCALE_SCK 5
#define WATER_SCALE_DOUT 18
#define FOOD_SCALE_SCK 19
#define FOOD_SCALE_DOUT 21
// see calibrate() to get DIVIDER and OFFSET
#define WATER_SCALE_OFFSET 142368
#define WATER_SCALE_DIVIDER 439.5
#define FOOD_SCALE_OFFSET -391764
#define FOOD_SCALE_DIVIDER 429.58

Stepper stepper(2048, STP_1, STP_2, STP_3, STP_4); // stepper step count & pins

HX711 water_scale;
HX711 food_scale;

SMTPSession smtp;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "north-america.pool.ntp.org", -18000, 60000);

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n\n---------------------------------------------"));

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

  timeClient.begin();
  timeClient.update();

  stepper.setSpeed(10);

  pinMode(PUMP_PIN, OUTPUT);

  water_scale.begin(WATER_SCALE_DOUT, WATER_SCALE_SCK);
  food_scale.begin(FOOD_SCALE_DOUT, FOOD_SCALE_SCK);
  water_scale.set_offset(WATER_SCALE_OFFSET);
  food_scale.set_offset(FOOD_SCALE_OFFSET);
  water_scale.set_scale(WATER_SCALE_DIVIDER);
  food_scale.set_scale(FOOD_SCALE_DIVIDER);
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

void calibrate(HX711 scale) {
  if (scale.is_ready()) {
    scale.set_scale(1);
    scale.set_offset(0);
    Serial.println("Offset... remove any weights from the scale.");
    delay(8000);
    Serial.print("Offset: ");
    long offset = scale.read_average(30);
    Serial.println(offset);
    Serial.println("Place a known weight on the scale...");
    delay(8000);
    Serial.print("Scale: ");
    long known = scale.get_units(30);
    Serial.print(known - offset);
    Serial.println(" / KNOWN WEIGHT = SCALE");
  }
  else {
    Serial.println("HX711 not found.");
  }
  delay(1000);
}

void test_loop() {
  while (true) {
    //delay(10000);
    //Serial.println("Calibrating Food Scale");
    //calibrate(food_scale);
    //Serial.println("Calibrating Water Scale");
    //calibrate(water_scale);
    //digitalWrite(PUMP_PIN, HIGH);
    //delay(1000);
    //digitalWrite(PUMP_PIN, LOW);
    //delay(1000);
    //stepper.step(1024);
    //delay(1000);
    Serial.print("Water: ");
    Serial.println(water_scale.get_units(10));
    delay(1001);
    Serial.print("Food: ");
    Serial.println(food_scale.get_units(10));
    delay(1000);
  }
}

void loop() {
  test_loop();
  String times[2] = {"07:00", "18:00"}; // times to output provisions
  String time = timeClient.getFormattedTime().substring(0,5);
  Serial.println(time);
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
