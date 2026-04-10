#include <Arduino.h>
#include "../lib/config.h"
#include "../lib/ble/ble.h"
#include "ble_setup.h"

#define LED_PIN LED_BUILTIN
const char SERVICE_UUID[]   =     "4fafc202-1fb5-459e-8fcc-c5c9c331914b";
const char CHARACTERISTIC_UUID[] = "cec5483e-36e1-4688-b7f5-ea07361b26a8";


bool on_state = false;

//===========IR sensor pins===================
#define SR2 36
#define SR1 39
#define S0 34
#define SL1 35
#define SL2 32

//motor driver pins===========================
#define LEFT_IN1 33
#define LEFT_IN2 25
#define RIGHT_IN1 26
#define RIGHT_IN2 27

#define LEFT_EN 12
#define RIGHT_EN 13

//PID Constants==============================
float Kp = 30;
float Ki = 5;
float Kd = 20;

float error = 0;
float last_error = 0;
float correction = 0;
float integral = 0;
float derivative = 0;

float last_time = 0;
float dt = 0;

//Motor speed===================================
int base_speed = 70;
int maxSpeed = 255;

int left_motor = 0;
int right_motor = 0;

//ledc variables =============================================
int ledcChan_left = 0;
int ledcChan_right = 1;
int ledcFreq = 5000; //Hz
int ledcRes = 8;     //8 bit (0-255)

bool ledState=false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  //ir and motor driver pins setup=======================
  pinMode(S0, INPUT);
  pinMode(SL1, INPUT);
  pinMode(SL2, INPUT);
  pinMode(SR1, INPUT);
  pinMode(SR2, INPUT);

  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);

  pinMode(LEFT_EN, OUTPUT);
  pinMode(RIGHT_EN, OUTPUT);

  digitalWrite(LEFT_EN, LOW);
  digitalWrite(RIGHT_EN, LOW);

  last_time = millis();

  //ledc setup for motors pwm ============================================

  ledcSetup(ledcChan_left, ledcFreq, ledcRes);
  ledcSetup(ledcChan_right, ledcFreq, ledcRes);
  ledcAttachPin(LEFT_EN, ledcChan_left);
  ledcAttachPin(RIGHT_EN, ledcChan_right);

  //BLE SETUP=============================================================
  ble_setup(SERVICE_UUID,
    CHARACTERISTIC_UUID,
    &on_state,
    LEFT_EN, RIGHT_EN,
    LEFT_IN1, LEFT_IN2,
    RIGHT_IN1, RIGHT_IN2,
    &base_speed,
    &Kp, &Ki, & Kd,
    ledcChan_left, ledcChan_right);
}

//=========================================================================

float get_error(){
  int sum = 0, count = 0;

  if(digitalRead(S0)==LOW){sum += 0; count++;}
  if(digitalRead(SL1)==LOW){sum += -1; count++;}
  if(digitalRead(SL2)==LOW){sum += -2; count++;}
  if(digitalRead(SR1)==LOW){sum += 1; count++;}
  if(digitalRead(SR2)==LOW){sum += 2; count++;}

  if(count==0) return last_error;

  return (float)sum/count;
}

void setMotor(int leftSpeed, int rightSpeed) {
  leftSpeed = constrain(leftSpeed, -maxSpeed, maxSpeed);
  rightSpeed = constrain(rightSpeed, -maxSpeed, maxSpeed);

  // Left Motor
  if (leftSpeed >= 0) {
    digitalWrite(LEFT_IN1, HIGH);
    digitalWrite(LEFT_IN2, LOW);
    ledcWrite(ledcChan_left, leftSpeed);
  } else {
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, HIGH);
    ledcWrite(ledcChan_left, -leftSpeed);
  }

  // Right Motor
  if (rightSpeed >= 0) {
    digitalWrite(RIGHT_IN1, HIGH);
    digitalWrite(RIGHT_IN2, LOW);
    ledcWrite(ledcChan_right, rightSpeed);
  } else {
    digitalWrite(RIGHT_IN1, LOW);
    digitalWrite(RIGHT_IN2, HIGH);
    ledcWrite(ledcChan_right, -rightSpeed);
  }
}

void pid_control(){
  dt = (millis()-last_time)/1000.0;
  if(dt <= 0) dt = 0.001;
  last_time = millis();

  integral += error*dt;
  integral = constrain(integral, -50, 50);

  derivative = (error-last_error)/dt;
  last_error = error;

  correction = (Kp*error) + (Ki*integral) + (Kd*derivative);

  left_motor = constrain(base_speed + correction, -maxSpeed, maxSpeed);
  right_motor = constrain(base_speed - correction, -maxSpeed, maxSpeed);

}

void loop() {
   
    //     // BLE notifications
    // static unsigned long lastUpdate = 0;
    // if (millis() - lastUpdate > 1000) {
    //     if (BLEManager::getInstance().isConnected()) {
    //         dataChar->setValue(value);
    //         dataChar->notify();
    //         value++;
    //         lastUpdate = millis();
    //     }
    // }
  if(on_state){
    error = get_error();
    pid_control();
    setMotor(left_motor, right_motor);
  }
  else setMotor(0,0);
}