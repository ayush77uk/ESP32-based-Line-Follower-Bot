#include <Arduino.h>
#include "../lib/config.h"
#include "../lib/ble/ble.h"
#include "ble_setup.h"

#define LED_PIN LED_BUILTIN
const char SERVICE_UUID[]        = "4fafc202-1fb5-459e-8fcc-c5c9c331914b";
const char CHARACTERISTIC_UUID[] = "cec5483e-36e1-4688-b7f5-ea07361b26a8";

bool on_state = false;

//===========IR sensor pins===================
#define SR2 36
#define SR1 39
#define S0  34
#define SL1 35
#define SL2 32

//===========motor driver pins================
#define LEFT_IN1  33
#define LEFT_IN2  25
#define RIGHT_IN1 26
#define RIGHT_IN2 27
#define LEFT_EN   12
#define RIGHT_EN  13

//===========PID Constants====================
float Kp = 300;
float Ki = 0;
float Kd = 40;

float error      = 0;
float last_error = 0;
float correction = 0;
float integral   = 0;
float derivative = 0;
float last_time  = 0;
float dt         = 0;

//===========Motor speed======================
int base_speed = 200;
int maxSpeed   = 255;

int left_motor  = 0;
int right_motor = 0;

//===========LEDC variables===================
int ledcChan_left  = 0;
int ledcChan_right = 1;
int ledcFreq = 5000;
int ledcRes  = 8;

//===========Analog threshold=================
// BLACK (on line)  = analogRead < threshold
// WHITE (off line) = analogRead > threshold
// Print raw values over serial to calibrate:
// threshold = (white_reading + black_reading) / 2
int threshold = 3000;

bool ledState = false;

bool black_track = true;
//=============================================


void setMotor(int leftSpeed, int rightSpeed) {
  leftSpeed  = constrain(leftSpeed,  -maxSpeed, maxSpeed);
  rightSpeed = constrain(rightSpeed, -maxSpeed, maxSpeed);

  // Left Motor
  if (leftSpeed >= 0) {
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, HIGH);
    ledcWrite(ledcChan_left, leftSpeed);
  } else {
    digitalWrite(LEFT_IN1, HIGH);
    digitalWrite(LEFT_IN2, LOW);
    ledcWrite(ledcChan_left, -leftSpeed);
  }

  // Right Motor
  if (rightSpeed >= 0) {
    digitalWrite(RIGHT_IN1, LOW);
    digitalWrite(RIGHT_IN2, HIGH);
    ledcWrite(ledcChan_right, rightSpeed);
  } else {
    digitalWrite(RIGHT_IN1, HIGH);
    digitalWrite(RIGHT_IN2, LOW);
    ledcWrite(ledcChan_right, -rightSpeed);
  }
}

//=============================================

// Returns true if sensor is on BLACK line
bool onLine(int pin) {
  return analogRead(pin) < threshold;
}

float get_error() {
  float sum   = 0;
  float total = 0;

  // Each sensor is weighted by how strongly it sees the line.
  // (threshold - raw) gives a higher weight to sensors more centered on line.
  // Positions: SL2=-2, SL1=-1, S0=0, SR1=1, SR2=2

  if (onLine(S0))  { float w = abs(threshold - analogRead(S0));  sum += 0  * w; total += w; }
  if (onLine(SL1)) { float w = abs(threshold - analogRead(SL1)); sum += -1 * w; total += w; }
  if (onLine(SL2)) { float w = abs(threshold - analogRead(SL2)); sum += -2 * w; total += w; }
  if (onLine(SR1)) { float w = abs(threshold - analogRead(SR1)); sum +=  1 * w; total += w; }
  if (onLine(SR2)) { float w = abs(threshold - analogRead(SR2)); sum +=  2 * w; total += w; }

  if (total == 0) return last_error;  // all sensors off line, hold last known error

  return sum / total;
}

//=============================================

void pid_control() {
  dt = (millis() - last_time) / 1000.0;
  if (dt <= 0) dt = 0.001;
  last_time = millis();

  integral += error * dt;
  integral = constrain(integral, -50, 50);

  derivative = (error - last_error) / dt;
  last_error = error;

  correction = (Kp * error) + (Ki * integral) + (Kd * derivative);

  left_motor  = constrain(base_speed + correction, -maxSpeed, maxSpeed);
  right_motor = constrain(base_speed - correction, -maxSpeed, maxSpeed);
}

bool allBlack() {
  return onLine(S0) && onLine(SL1) && onLine(SL2)
      && onLine(SR1) && onLine(SR2);
}


//=============================================

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  pinMode(S0,  INPUT);
  pinMode(SL1, INPUT);
  pinMode(SL2, INPUT);
  pinMode(SR1, INPUT);
  pinMode(SR2, INPUT);

  pinMode(LEFT_IN1,  OUTPUT);
  pinMode(LEFT_IN2,  OUTPUT);
  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);
  pinMode(LEFT_EN,   OUTPUT);
  pinMode(RIGHT_EN,  OUTPUT);

  digitalWrite(LEFT_EN,  LOW);
  digitalWrite(RIGHT_EN, LOW);

  last_time = millis();

  ledcSetup(ledcChan_left,  ledcFreq, ledcRes);
  ledcSetup(ledcChan_right, ledcFreq, ledcRes);
  ledcAttachPin(LEFT_EN,  ledcChan_left);
  ledcAttachPin(RIGHT_EN, ledcChan_right);

  ble_setup(SERVICE_UUID,
    CHARACTERISTIC_UUID,
    &on_state,
    LEFT_EN, RIGHT_EN,
    LEFT_IN1, LEFT_IN2,
    RIGHT_IN1, RIGHT_IN2,
    &base_speed,
    &Kp, &Ki, &Kd,
    ledcChan_left, ledcChan_right);

  Serial.println("LFR ready.");
}

//=============================================

void loop() {
  if (on_state) {

    if (allBlack()) {
      // Intersection detected — ignore PID, drive straight through
      error      = 0;
      derivative = 0;
      integral   = 0;
      setMotor(base_speed, base_speed);
    }
    else {
      error = get_error();
      pid_control();
      setMotor(left_motor, right_motor);
    }
    
  }
  else {
    setMotor(0, 0);
  }
}