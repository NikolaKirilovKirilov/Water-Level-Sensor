#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// === USER-CONFIGURABLE CONSTANTS ===
const int EMPTY_VOLUME_CM = 50;  // Distance from sensor to max level of water in cm
const float TANK_DEPTH_METERS = 4;  // tank depth here
const int TANK_DEPTH_CM = TANK_DEPTH_METERS * 100;

// === Pin Configuration ===
#define TRIG_PIN 27
#define ECHO_PIN 26

#define RELAY1_PIN 17
#define RELAY2_PIN 5
#define RELAY3_PIN 18
#define RELAY4_PIN 19
#define RELAY5_PIN 21
#define RELAY6_PIN 22

// === LCD Configuration ===
LiquidCrystal_I2C lcd(0x27, 4, 16);  // Change address to 0x3F if needed

float measureDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);  // Timeout 30ms
  float distance = duration * 0.034 / 2;
  
  // Cap unreasonable values
  if (distance <= 2 || distance > TANK_DEPTH_CM + EMPTY_VOLUME_CM) {
    return -1;  // Invalid measurement
  }
  return distance;
}

float calcFillPercentage(float distance) {
  if (distance == -1) return -1;  // Invalid measurement
  int level_cm = TANK_DEPTH_CM - distance - EMPTY_VOLUME_CM;
  level_cm = max(0, min(level_cm, TANK_DEPTH_CM));
  return (level_cm * 100.0) / TANK_DEPTH_CM;
}

void writeOnLCD(float distance, float percentage)
{
  static float lastDistance = -999;  // Cache last values
  static float lastPercentage = -999;

  // Only update if values changed or first run
  if (distance == lastDistance && percentage == lastPercentage) {
    return;  // Skip update if no change
  }

  lastDistance = distance;
  lastPercentage = percentage;

  lcd.clear();
  if (distance == -1) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error");
    return;
  }

  int level_cm = TANK_DEPTH_CM - distance - EMPTY_VOLUME_CM;
  level_cm = max(0, min(level_cm, TANK_DEPTH_CM));

  // Line 0: Raw Data
  lcd.setCursor(0, 0);
  lcd.print("Depth: ");
  lcd.print(TANK_DEPTH_CM);
  lcd.print("cm");

  // Line 1: Level in cm
  lcd.setCursor(0, 1);
  lcd.print("Level: ");
  lcd.print(level_cm);
  lcd.print(" cm     ");

  // Line 2: Percentage
  lcd.setCursor(0, 2);
  lcd.print("Percent: ");
  lcd.print((int)percentage);
  lcd.print("%      ");

  // Line 3: Visual bar
  int bars = map(level_cm, 0, TANK_DEPTH_CM, 0, 20);
  lcd.setCursor(0, 3);
  for (int i = 0; i < 20; i++) {
    lcd.print(i < bars ? char(255) : ' ');
  }

  // Debug to Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm, Level: ");
  Serial.print(level_cm);
  Serial.print(" cm, Percent: ");
  Serial.print(percentage);
  Serial.println(" %");
}

void triggerRelays(float distance, float percentage = -1)
{
  if (distance == -1) {
    // Sensor error, turn off all relays
    digitalWrite(RELAY1_PIN, LOW);
    digitalWrite(RELAY2_PIN, LOW);
    digitalWrite(RELAY3_PIN, LOW);
    digitalWrite(RELAY4_PIN, LOW);
    digitalWrite(RELAY5_PIN, LOW);
    digitalWrite(RELAY6_PIN, LOW);
    return;
  }

  // Relay logic based on percentage thresholds
  digitalWrite(RELAY1_PIN, percentage >= 10 ? LOW : HIGH);
  digitalWrite(RELAY2_PIN, percentage >= 30 ? LOW : HIGH);
  digitalWrite(RELAY3_PIN, percentage >= 50 ? LOW : HIGH);
  digitalWrite(RELAY4_PIN, percentage >= 70 ? LOW : HIGH);
  digitalWrite(RELAY5_PIN, percentage >= 90 ? LOW : HIGH);
  digitalWrite(RELAY6_PIN, percentage >= 100 ? LOW : HIGH);
}

void setup() {
  Serial.begin(115200);

  // Initialize pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  pinMode(RELAY5_PIN, OUTPUT);
  pinMode(RELAY6_PIN, OUTPUT);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Water Level Monitor");
  delay(1000);
  lcd.clear();
}

void loop() {
  float distance = measureDistanceCM();
  float percentage = calcFillPercentage(distance);
  writeOnLCD(distance, percentage);
  triggerRelays(distance, percentage);

  delay(500);
}