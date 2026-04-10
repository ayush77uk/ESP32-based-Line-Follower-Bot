#include <Arduino.h>
#include <../lib/config.h>
#include "../lib/ble/ble.h"

#include "ble_setup.h"

void led_blink_signal(){
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(200);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void ble_setup(const char* SERVICE_UUID,
    const char* CHARACTERISTIC_UUID,
    bool *on_state,
    int LEFT_EN, int RIGHT_EN,
    int LEFT_IN1, int LEFT_IN2,
    int RIGHT_IN1, int RIGHT_IN2,
    int* base_speed,
    float* Kp, float* Ki, float* Kd,
    int ledcChan_left, int ledcChan_right) {

BLEManager& ble = BLEManager::getInstance();
  ble.init("LFR_ESP_TEST");

//BLE wrapper
BLECharWrapper* dataChar = nullptr;
BLECharWrapper* rfidChar = nullptr;
uint32_t value = 0;



    // Set connection callbacks
    ble.onConnect([]() {
        Serial.println("Client connected!");
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
    });

    ble.onDisconnect([on_state, LEFT_EN, RIGHT_EN]() {
        Serial.println("Client disconnected, restarting advertising...");
        digitalWrite(LEFT_EN, LOW);
        digitalWrite(RIGHT_EN, LOW);
        *on_state = false;

        digitalWrite(LED_BUILTIN, LOW);
        BLEManager::getInstance().startAdvertising();
    });

    // Create service
    BLEServiceWrapper* service = ble.createService(SERVICE_UUID);

    // Create characteristic with builder pattern
    dataChar = service->addCharacteristic(CHARACTERISTIC_UUID)
        .withRead()
        .withWrite()
        .withNotify()
        .onWrite([base_speed, Kp, Ki, Kd,LEFT_IN1, LEFT_IN2, RIGHT_IN1, RIGHT_IN2, on_state, ledcChan_left, ledcChan_right]
            (uint8_t* data, size_t len) {
            if (len == 0 || len > 31) return;

            char buf[32];
            memcpy(buf, data, len);
            buf[len] = '\0';
            Serial.print("Received: ");
            Serial.println(buf);

            int new_base_speed = 0;
            if (sscanf(buf, "s%d", &new_base_speed) == 1){
              *base_speed = new_base_speed;
              led_blink_signal();

              Serial.printf("Base speed = %d\n", *base_speed);
            }

            float new_kp, new_ki, new_kd;
            if (sscanf(buf, "p%f", &new_kp) == 1){
              *Kp = new_kp;
              led_blink_signal();

              Serial.printf("P = %.2f\n", *Kp);
            }
            if (sscanf(buf, "i%f", &new_ki) == 1){
              *Ki = new_ki;
              led_blink_signal();

              Serial.printf("I = %.2f\n", *Ki);
            }
            if (sscanf(buf, "d%f", &new_kd) == 1){
              *Kd = new_kd;
              led_blink_signal();

              Serial.printf("D = %.2f\n", *Kd);
            }

            if(strcmp(buf, "1")==0){
              *on_state = !(*on_state);
              digitalWrite(LED_BUILTIN, *on_state);

              Serial.printf("Bot State : %s\n", (*on_state)? "ON":"OFF");

            //   if(*on_state){
            //   ledcWrite(ledcChan_left, *base_speed);
            //   ledcWrite(ledcChan_right, *base_speed);

            //   digitalWrite(LEFT_IN1, LOW);
            //   digitalWrite(LEFT_IN2, HIGH);

            //   digitalWrite(RIGHT_IN1, LOW);
            //   digitalWrite(RIGHT_IN2, HIGH);
              
            //   }
            //   else{
            //   ledcWrite(ledcChan_left, 0);
            //   ledcWrite(ledcChan_right, 0);
            //   }
            
            }

        })
        .build();

            service->start();
    ble.startAdvertising();

    Serial.println("BLE Server ready. Waiting for connections...");
}