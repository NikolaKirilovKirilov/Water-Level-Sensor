#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// === USER-CONFIGURABLE CONSTANTS ===
const int EMPTY_VOLUME_CM = 50;  // Distance from sensor to max level of water in cm
const float TANK_DEPTH_METERS = 4;  // Set your tank depth here
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
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Change address to 0x3F if needed

float measureDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);  // Timeout 30ms
  float distance = duration * 0.034 / 2;
  
  // Cap unreasonable values
  if (distance <= 2 || distance > TANK_DEPTH_CM + 10) {
    return -1;  // Invalid measurement
  }

  return distance;
}

void writeOnLCD(float distance)
{
  lcd.clear();
  if (distance == -1) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error");
  } else {
    int level_cm = TANK_DEPTH_CM - distance - EMPTY_VOLUME_CM;
    level_cm = max(0, min(level_cm, TANK_DEPTH_CM));
    float percentage = (level_cm * 100.0) / TANK_DEPTH_CM;

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
}

void triggerRelays(float distance)
{

}

void setup() {
  Serial.begin(115200);

  // Setup pins for HC-SR04
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
  writeOnLCD(distance);
  triggerRelays(distance);
  

  delay(500);
}