# Code for ES32 based Line Follower Bot (Work in Progress)

## Overview
This project implements a line follower robot using an ESP32, a 5-channel IR sensor array, and an L298 motor driver. The robot uses a PID control system for smooth line tracking and allows real-time tuning of speed and PID parameters via Bluetooth Low Energy (BLE).

## Micro-Controller Description
* Board : ESP32 DOIT Devkit V1
* CPU: Dual-core Xtensa LX6
* Clock speed: Up to 240 MHz
* SRAM: ~520 KB
* Flash: 4 MB

## Hardware Requirements
* ESP32 DOIT DevKit V1
* Smart Elex 5-channel IR sensor array
* L298N motor driver
* 2 DC motors
* Ball caster wheel
* Power supply (battery pack)

## Pin Configuration

### IR Sensor Connections

```
IR1 (Leftmost)  → GPIO 32
IR2             → GPIO 35
IR3 (Center)    → GPIO 34
IR4             → GPIO 39
IR5             → GPIO 36
```

### Motor Driver (L298N)

```
LEFT_IN1   → GPIO 33
LEFT_IN2   → GPIO 25
RIGHT_IN1  → GPIO 26
RIGHT_IN2  → GPIO 27

LEFT_ENABLE    → GPIO 12  (PWM)
RIGHT_ENABLE   → GPIO 13  (PWM)
```

## Working Principle
- IR sensors detect line position.
- Error is calculated using weighted average, to detect at how much offset the center IR sensor is from the line.
- PID controller logic processes the error and computes correction.
- Motor speeds are adjusted according to the correction to counter the error.

## BLE Commands (Format : Text UTF-8)
+ s<number> : Sets the base speed of bot
+ p<number> : Sets P (Proprtional constant)
+ i<number> : Sets I (Integral constant)
+ d<number> : Sets D (Derivative constant)

//Use any BLE mobile app (Recommended : "nrf Connect") to send commands to the esp32 over BLE