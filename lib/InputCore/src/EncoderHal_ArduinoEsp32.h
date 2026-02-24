#pragma once
#include "EncoderHal.h"
#include <Arduino.h>
#include "driver/gpio.h"

class EncoderHal_ArduinoEsp32 : public EncoderHal {
public:
  EncoderHal_ArduinoEsp32(int pinA, int pinB, int pinBtn);

  void begin() override;

  int readA() override;
  int readB() override;
  int readBtn() override;

  void attachABInterrupts(void (*isr)()) override;

  void enterCritical() override;
  void exitCritical() override;

  uint32_t millisNow() override { return ::millis(); }

private:
  int _a, _b, _btn;
  portMUX_TYPE _mux = portMUX_INITIALIZER_UNLOCKED;
};
