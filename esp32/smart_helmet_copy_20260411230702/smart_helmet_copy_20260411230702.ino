// ============================================================
// SMART SAFETY HELMET (FINAL STABLE VERSION)
// ============================================================

#define BLYNK_TEMPLATE_ID "  "
#define BLYNK_TEMPLATE_NAME "Smart Helmet"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <TinyGPS++.h>
#include <Wire.h>

// -------- CREDENTIALS --------
char auth[] = " ";
char ssid[] = " ";
char pass[] = " ";

// -------- PINS --------
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define MQ2_PIN 34
#define MQ7_PIN 32
#define MQ135_PIN 33
#define O2_PIN 36
#define SW420_PIN 18
#define BUZZER_PIN 26
#define SOS_BTN_PIN 5

// -------- THRESHOLDS --------
#define CO_THRESHOLD 300
#define GAS_THRESHOLD 400
#define AIR_QUALITY_THRESH 400
#define O2_LOW_THRESHOLD 850
#define TEMP_MAX 40.0
#define FALL_THRESHOLD 2.5

// -------- OLED --------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------- OBJECTS --------
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_BMP280 bmp;
MPU6050 mpu;
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

// -------- VARIABLES --------
float temperature, humidity, pressure, altitude;
int mq2Value, mq7Value, mq135Value, o2Value;
float o2Percent;
bool sosTriggered = false;
bool fallDetected = false;
bool vibrationDetected = false;
double gpsLat = 0, gpsLng = 0;

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SOS_BTN_PIN, INPUT_PULLUP);
  pinMode(SW420_PIN, INPUT);

  Wire.begin(21, 22);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
  }

  display.clearDisplay();
  display.setCursor(10, 20);
  display.println("Smart Helmet");
  display.display();
  delay(2000);

  // Sensors
  dht.begin();

  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 not detected!");
  }

  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 not detected!");
  }

  // GPS
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);

  // WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  // Blynk
  Blynk.config(auth);
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  Blynk.run();
  readSensors();
  checkSafety();
  updateOLED();
  sendToBlynk();
  readGPS();
  delay(2000);
}

// ============================================================
// SENSOR READ
// ============================================================
void readSensors() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  pressure = bmp.readPressure();
  altitude = bmp.readAltitude(1013.25);

  mq2Value = analogRead(MQ2_PIN);
  mq7Value = analogRead(MQ7_PIN);
  mq135Value = analogRead(MQ135_PIN);
  o2Value = analogRead(O2_PIN);

  o2Percent = map(o2Value, 0, 4095, 0, 250) / 10.0;

  vibrationDetected = digitalRead(SW420_PIN);

  if (digitalRead(SOS_BTN_PIN) == LOW) {
    sosTriggered = true;
    triggerAlarm("SOS");
  }

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float gForce = sqrt(pow(ax/16384.0,2) +
                      pow(ay/16384.0,2) +
                      pow(az/16384.0,2));

  if (gForce > FALL_THRESHOLD || gForce < 0.3) {
    fallDetected = true;
    triggerAlarm("FALL");
  }
}

// ============================================================
// SAFETY CHECK
// ============================================================
void checkSafety() {
  bool danger = false;

  if (mq7Value > CO_THRESHOLD) {
    danger = true;
    Blynk.logEvent("co_alert");
  }

  if (mq2Value > GAS_THRESHOLD) {
    danger = true;
    Blynk.logEvent("gas_alert");
  }

  if (mq135Value > AIR_QUALITY_THRESH) {
    danger = true;
  }

  if (o2Value < O2_LOW_THRESHOLD) {
    danger = true;
    Blynk.logEvent("o2_alert");
  }

  if (temperature > TEMP_MAX) {
    danger = true;
  }

  if (vibrationDetected) {
    danger = true;
    Blynk.logEvent("vibration_alert");
  }

  if (sosTriggered) {
    danger = true;
    Blynk.logEvent("sos");
    sosTriggered = false;
  }

  if (fallDetected) {
    danger = true;
    Blynk.logEvent("fall_alert");
    fallDetected = false;
  }

  digitalWrite(BUZZER_PIN, danger ? HIGH : LOW);
}

// ============================================================
// OLED
// ============================================================
void updateOLED() {
  display.clearDisplay();

  display.setCursor(0, 0);
  display.print("T:");
  display.print(temperature);

  display.setCursor(0, 10);
  display.print("H:");
  display.print(humidity);

  display.setCursor(0, 20);
  display.print("CO:");
  display.print(mq7Value);

  display.setCursor(0, 30);
  display.print("O2:");
  display.print(o2Percent);

  display.display();
}

// ============================================================
// BLYNK
// ============================================================
void sendToBlynk() {
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, mq7Value);
  Blynk.virtualWrite(V3, mq2Value);
  Blynk.virtualWrite(V4, mq135Value);
  Blynk.virtualWrite(V5, o2Percent);
  Blynk.virtualWrite(V6, pressure / 100);
  Blynk.virtualWrite(V7, altitude);

  if (gpsLat != 0) {
    Blynk.virtualWrite(V10, gpsLat);
    Blynk.virtualWrite(V11, gpsLng);
  }
}

// ============================================================
// GPS
// ============================================================
void readGPS() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid()) {
    gpsLat = gps.location.lat();
    gpsLng = gps.location.lng();
  }
}

// ============================================================
// ALARM
// ============================================================
void triggerAlarm(String msg) {
  Serial.println("ALARM: " + msg);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(300);
  digitalWrite(BUZZER_PIN, LOW);
}