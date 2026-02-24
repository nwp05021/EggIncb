#pragma once
#include <Arduino.h>
#include <Wire.h>

// I2C bus recovery: If SDA is held low by a slave, try to free it by toggling SCL.
namespace I2cRecovery {

inline void busRecoveryGPIO(int sdaPin, int sclPin) {
  // Release I2C pins
  pinMode(sdaPin, INPUT_PULLUP);
  pinMode(sclPin, INPUT_PULLUP);
  delay(1);

  // If SDA is low, clock out 9 pulses on SCL to free a stuck slave
  if (digitalRead(sdaPin) == LOW) {
    pinMode(sclPin, OUTPUT);
    for (int i = 0; i < 9; i++) {
      digitalWrite(sclPin, HIGH);
      delayMicroseconds(5);
      digitalWrite(sclPin, LOW);
      delayMicroseconds(5);
    }
    pinMode(sclPin, INPUT_PULLUP);
    delay(1);
  }

  // Send a STOP condition manually: SDA low -> SCL high -> SDA high
  pinMode(sdaPin, OUTPUT);
  digitalWrite(sdaPin, LOW);
  delayMicroseconds(5);

  pinMode(sclPin, OUTPUT);
  digitalWrite(sclPin, HIGH);
  delayMicroseconds(5);

  pinMode(sdaPin, INPUT_PULLUP);
  delayMicroseconds(5);

  // Restore for Wire
  pinMode(sdaPin, INPUT_PULLUP);
  pinMode(sclPin, INPUT_PULLUP);
}

inline void configureWire(uint32_t clockHz, uint32_t timeoutMs) {
  // Wire timeout: prevents indefinite blocking on a bad bus
  // On ESP32 Arduino, setTimeOut exists in most builds.
  // If your core lacks it, this call will fail to compile; in that case comment it out.
  Wire.setClock(clockHz);
  Wire.setTimeOut(timeoutMs);
}

} // namespace I2cRecovery