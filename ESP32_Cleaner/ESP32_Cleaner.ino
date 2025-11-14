#define BLYNK_TEMPLATE_ID "YOUR_BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "SolarCleaner"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ---------------------- MOTOR DRIVER PINS ----------------------
#define STBY 18
#define BIN1 19
#define BIN2 22
#define PWMB 23

// ---------------------- IR SENSOR PINS --------------------------
#define IR_SENSOR_FORWARD 27
#define IR_SENSOR_BACKWARD 25

// ---------------------- WIFI CREDENTIALS ------------------------
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ---------------------- BLYNK CONFIG ----------------------------
#define BLYNK_BUTTON V0

// ---------------------- PWM CONFIG ------------------------------
const int pwmChannel = 1;
const int freq = 1000;
const int resolution = 8;
// Slow speed for ~20 RPM
const int slowDutyCycle = 60;

// ---------------------- WEB SERVER ------------------------------
WebServer server(80);

// ---------------------- STATE MACHINE ---------------------------
enum State { IDLE, MOVING_FORWARD, MOVING_BACKWARD };
State motionState = IDLE;

// ---------------------- MOTOR FUNCTIONS -------------------------
void moveForward() {
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  ledcWrite(PWMB, slowDutyCycle);
}

void moveBackward() {
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  ledcWrite(PWMB, slowDutyCycle);
}

void stopMotor() {
  ledcWrite(PWMB, 0);
}

// ---------------------- SETUP ----------------------
void setup() {
  Serial.begin(115200);

  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(IR_SENSOR_FORWARD, INPUT);
  pinMode(IR_SENSOR_BACKWARD, INPUT);

  ledcAttach(PWMB, freq, resolution);

  // WiFi setup
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected: " + WiFi.localIP().toString());

  // Web server routes
  server.on("/clean", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"error\":\"No data\"}");
      return;
    }
    String body = server.arg("plain");
    StaticJsonDocument<200> doc;
    deserializeJson(doc, body);
    const char* cmd = doc["command"];
    if (strcmp(cmd, "start") == 0) {
      motionState = MOVING_FORWARD;
      Serial.println("🟢 Cleaning started via HTTP");
      server.send(200, "application/json", "{\"status\":\"started\"}");
    } else if (strcmp(cmd, "stop") == 0) {
      motionState = IDLE;
      stopMotor();
      Serial.println("🛑 Cleaning stopped via HTTP");
      server.send(200, "application/json", "{\"status\":\"stopped\"}");
    }
  });
  server.begin();
  Serial.println("🌐 HTTP Server Started");

  // Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

// ---------------------- BLYNK HANDLER ----------------------
BLYNK_WRITE(BLYNK_BUTTON) {
  int v = param.asInt();
  if (v == 1) {
    motionState = MOVING_FORWARD;
    Serial.println("🚀 Cleaning started via Blynk");
  } else {
    motionState = IDLE;
    stopMotor();
    Serial.println("🛑 Cleaning stopped via Blynk");
  }
}

// ---------------------- MAIN LOOP ----------------------
void loop() {
  Blynk.run();
  server.handleClient();

  int frontIR = digitalRead(IR_SENSOR_FORWARD);
  int backIR = digitalRead(IR_SENSOR_BACKWARD);

  // ✅ Correct IR logic: 0 = Surface, 1 = Air
  bool frontSurface = (frontIR == LOW);
  bool frontAir     = (frontIR == HIGH);
  bool backSurface  = (backIR == LOW);
  bool backAir      = (backIR == HIGH);

  Serial.print("Front:");
  Serial.print(frontIR);
  Serial.print(" Back:");
  Serial.print(backIR);
  Serial.print(" | STATE: ");
  switch (motionState) {
    case IDLE: Serial.println("IDLE"); break;
    case MOVING_FORWARD: Serial.println("MOVING_FORWARD"); break;
    case MOVING_BACKWARD: Serial.println("MOVING_BACKWARD"); break;
  }

  switch (motionState) {
    case MOVING_FORWARD:
      moveForward();
      // Forward surface, back air → keep moving
      if (frontSurface && backAir) moveForward();
      // Both surface → keep moving
      else if (frontSurface && backSurface) moveForward();
      // Front air, back surface → reverse
      else if (frontAir && backSurface) {
        stopMotor();
        delay(300);
        Serial.println("↩ Front off surface — reversing");
        motionState = MOVING_BACKWARD;
      }
      break;

    case MOVING_BACKWARD:
      // Keep reversing until back IR detects air
      if (backSurface) {
        moveBackward();  // continue backward
        Serial.println("⬅ Back surface detected — continuing reverse");
      } else if (backAir) {
        stopMotor();
        motionState = IDLE;
        Serial.println("✅ Back IR detected air — cleaning done, waiting for next command");
      }
      break;

    case IDLE:
    default:
      stopMotor();
      break;
  }

  delay(120);
}