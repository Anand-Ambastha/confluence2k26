// ============================================================
// SMART SAFETY HELMET (FINAL WORKING VERSION)
// ============================================================

#define BLYNK_TEMPLATE_ID "TMPL3SJyrLOuo"
#define BLYNK_TEMPLATE_NAME "Smart Helmet"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <MPU6050.h>
#include <Adafruit_BMP280.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// ---- TinyML ----
#include "model.h"
#include <TensorFlowLite_ESP32.h>

#include <tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h>
#include <tensorflow/lite/experimental/micro/micro_interpreter.h>
#include <tensorflow/lite/experimental/micro/micro_error_reporter.h>
#include <tensorflow/lite/schema/schema_generated.h>

#include <cmath>

// -------- CREDENTIALS --------
char auth[] = "OHik1IXJWVapBvjac_f9ZUmKKjdutE2n";
char ssid[] = "Excitel_Benzo4";
char pass[] = "Deter@y@";

// -------- PINS --------
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define MQ2_PIN 34
#define MQ7_PIN 32
#define MQ135_PIN 33
#define SW420_PIN 18
#define BUZZER_PIN 26
#define SOS_BTN_PIN 5

// -------- OLED --------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------- OBJECTS --------
DHT dht(DHT_PIN, DHT_TYPE);
MPU6050 mpu;
Adafruit_BMP280 bmp;
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

// -------- VARIABLES --------
float temperature, pressure;
int mq2Value, mq7Value, mq135Value;
bool vibrationDetected = false;
bool sosTriggered = false;
double gpsLat = 0, gpsLng = 0;

// -------- STATE --------
enum Status { SAFE, WARNING, CRITICAL };
Status currentStatus = SAFE;

// -------- BUZZER --------
unsigned long lastBeep = 0;
bool buzzerState = false;

// -------- TinyML --------
constexpr int tensorArenaSize = 15 * 1024;
uint8_t tensorArena[tensorArenaSize];

tflite::MicroInterpreter* interpreter;
TfLiteTensor* input;
TfLiteTensor* output;

// ============================================================
// MODEL SETUP (FIXED)
// ============================================================
void setupModel() {

  const tflite::Model* model_tflite = tflite::GetModel(model);

  static tflite::MicroErrorReporter micro_error_reporter;
  tflite::ErrorReporter* error_reporter = &micro_error_reporter;

  static tflite::ops::micro::AllOpsResolver resolver;

  static tflite::MicroInterpreter static_interpreter(
    model_tflite,
    resolver,
    tensorArena,
    tensorArenaSize,
    error_reporter
  );

  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("Tensor allocation failed!");
    while (1);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SOS_BTN_PIN, INPUT_PULLUP);
  pinMode(SW420_PIN, INPUT);

  Wire.begin(21, 22);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);

  dht.begin();
  mpu.initialize();
  bmp.begin(0x76);

  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(300);

  Blynk.config(auth);

  setupModel();

  Serial.println("System Ready");
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  Blynk.run();

  readSensors();
  readGPS();
  runTinyML();
  applyOverrides();
  handleBuzzer();
  updateOLED();
  sendToBlynk();

  delay(1000);
}

// ============================================================
// SENSOR READ
// ============================================================
void readSensors() {
  temperature = dht.readTemperature();
  if (isnan(temperature)) temperature = 0;

  pressure = bmp.readPressure() / 100.0;

  mq2Value = analogRead(MQ2_PIN);
  mq7Value = analogRead(MQ7_PIN);
  mq135Value = analogRead(MQ135_PIN);

  vibrationDetected = digitalRead(SW420_PIN);

  if (digitalRead(SOS_BTN_PIN) == LOW) {
    sosTriggered = true;
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
// ML SYSTEM
// ============================================================
void runTinyML() {

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float acc = sqrt(pow(ax/16384.0,2) +
                   pow(ay/16384.0,2) +
                   pow(az/16384.0,2));

  static float prevAcc = 1.0;
  float jerk = abs(acc - prevAcc);
  prevAcc = acc;

  bool noMove = (abs(ax) < 3000 && abs(ay) < 3000);

  // ---- NORMALIZATION ----
  input->data.f[0] = mq2Value / 768.62;
  input->data.f[1] = mq7Value / 768.58;
  input->data.f[2] = mq135Value / 897.15;
  input->data.f[3] = temperature / 57.69;
  input->data.f[4] = acc / 4.70;
  input->data.f[5] = jerk / 3.26;
  input->data.f[6] = noMove;

  interpreter->Invoke();

  float safe = output->data.f[0];
  float warn = output->data.f[1];
  float crit = output->data.f[2];

  if (crit > warn && crit > safe)
    currentStatus = CRITICAL;
  else if (warn > safe)
    currentStatus = WARNING;
  else
    currentStatus = SAFE;
}

// ============================================================
// OVERRIDES
// ============================================================
void applyOverrides() {

  if (vibrationDetected) {
    currentStatus = CRITICAL;
    Blynk.logEvent("vibration_alert");
  }

  if (sosTriggered) {
    currentStatus = CRITICAL;
    Blynk.logEvent("sos");
    sosTriggered = false;
  }

  if (currentStatus == CRITICAL) {
    Blynk.logEvent("critical_emergency");

    if (gpsLat != 0) {
      Blynk.virtualWrite(V10, gpsLat);
      Blynk.virtualWrite(V11, gpsLng);
    }
  }
}

// ============================================================
// OLED
// ============================================================
void updateOLED() {
  display.clearDisplay();

  display.setCursor(0, 0);
  display.print("T: "); display.print(temperature);

  display.setCursor(0, 10);
  display.print("CO: "); display.print(mq7Value);

  display.setCursor(0, 20);
  display.print("AIR: "); display.print(mq135Value);

  display.setCursor(0, 30);
  display.print("P: "); display.print(pressure);

  display.setCursor(0, 50);
  display.print("STATUS: ");

  if (currentStatus == SAFE) display.print("SAFE");
  else if (currentStatus == WARNING) display.print("WARN");
  else display.print("CRIT");

  display.display();
}

// ============================================================
// BUZZER
// ============================================================
void handleBuzzer() {

  switch (currentStatus) {

    case SAFE:
      digitalWrite(BUZZER_PIN, LOW);
      break;

    case WARNING:
      if (millis() - lastBeep > 700) {
        buzzerState = !buzzerState;
        digitalWrite(BUZZER_PIN, buzzerState);
        lastBeep = millis();
      }
      break;

    case CRITICAL:
      digitalWrite(BUZZER_PIN, HIGH);
      break;
  }
}

// ============================================================
// BLYNK
// ============================================================
void sendToBlynk() {
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, mq7Value);
  Blynk.virtualWrite(V2, mq2Value);
  Blynk.virtualWrite(V3, mq135Value);
  Blynk.virtualWrite(V4, pressure);

  if (gpsLat != 0) {
    Blynk.virtualWrite(V10, gpsLat);
    Blynk.virtualWrite(V11, gpsLng);
  }
}