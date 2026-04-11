#include <Wire.h>
#include <math.h>
#include <DHTesp.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <MPU6050.h>

// ===== PIN DEFINITIONS =====
#define DHT_PIN 15

#define MQ2_PIN 34
#define MQ7_PIN 32
#define O2_PIN 35   // ✅ fixed

#define BUTTON_PIN 5
#define BUTTON2_PIN 18

#define BUZZER_PIN 26
#define LED_PIN 2

// ===== OBJECTS =====
DHTesp dht;
MPU6050 mpu;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== GPS =====
float gpsLat = 28.6139;
float gpsLng = 77.2090;

// ===== THRESHOLDS =====
#define GAS_THRESHOLD 2000
#define CO_THRESHOLD 2000
#define O2_LOW 1500
#define TEMP_MAX 40
#define FALL_THRESHOLD 2.5

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  Wire.begin(21, 22);

  dht.setup(DHT_PIN, DHTesp::DHT22);
  delay(2000);

  mpu.initialize();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  Serial.println("SYSTEM READY");
}

void loop() {

  // ===== DHT =====
  TempAndHumidity data = dht.getTempAndHumidity();

  // retry once if failed
  if (isnan(data.temperature) || isnan(data.humidity)) {
    delay(1000);
    data = dht.getTempAndHumidity();
  }

  if (isnan(data.temperature) || isnan(data.humidity)) {
    Serial.println("DHT ERROR!");
    return;
  }

  float temp = data.temperature;
  float hum  = data.humidity;

  // ===== ANALOG =====
  int gas = analogRead(MQ2_PIN);
  int co  = analogRead(MQ7_PIN);
  int o2  = analogRead(O2_PIN);

  // ===== MPU =====
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float gForce = sqrt(pow(ax/16384.0,2) +
                      pow(ay/16384.0,2) +
                      pow(az/16384.0,2));

  // ===== CONDITIONS =====
  bool danger = false;

  if (temp > TEMP_MAX) danger = true;
  if (gas > GAS_THRESHOLD) danger = true;
  if (co > CO_THRESHOLD) danger = true;
  if (o2 < O2_LOW) danger = true;

  if (gForce > FALL_THRESHOLD || gForce < 0.3) {
    Serial.println("FALL DETECTED");
    danger = true;
  }

  if (digitalRead(BUTTON_PIN) == LOW) danger = true;
  if (digitalRead(BUTTON2_PIN) == LOW) danger = true;

  // ===== OUTPUT =====
  digitalWrite(BUZZER_PIN, danger);
  digitalWrite(LED_PIN, danger);

  // ===== SERIAL =====
  Serial.print("Temp: "); Serial.println(temp);
  Serial.print("Humidity: "); Serial.println(hum);
  Serial.print("GPS: ");
  Serial.print(gpsLat); Serial.print(", ");
  Serial.println(gpsLng);

  // ===== OLED =====
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0,0);
  display.print("T:");
  display.print(temp);

  display.setCursor(0,10);
  display.print("H:");
  display.print(hum);

  display.setCursor(0,20);
  display.print("Gas:");
  display.print(gas);

  display.setCursor(0,30);
  display.print("CO:");
  display.print(co);

  display.setCursor(0,40);
  display.print("Lat:");
  display.print(gpsLat,1);

  display.setCursor(0,50);
  display.print("Lng:");
  display.print(gpsLng,1);

  display.display();

  delay(2000);
}