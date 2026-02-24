#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class I2cLock {
public:
  void begin() {
    if (!_m) _m = xSemaphoreCreateMutex();
  }

  bool lock(uint32_t timeoutMs = 50) {
    if (!_m) return true;
    return xSemaphoreTake(_m, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
  }

  void unlock() {
    if (_m) xSemaphoreGive(_m);
  }

private:
  SemaphoreHandle_t _m = nullptr;
};