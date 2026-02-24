#include "EncoderHal_ArduinoEsp32.h"

EncoderHal_ArduinoEsp32::EncoderHal_ArduinoEsp32(int pinA, int pinB, int pinBtn)
: _a(pinA), _b(pinB), _btn(pinBtn) {}

void EncoderHal_ArduinoEsp32::begin() {
  pinMode(_a, INPUT_PULLUP);
  pinMode(_b, INPUT_PULLUP);
  pinMode(_btn, INPUT_PULLUP);
}

//int EncoderHal_ArduinoEsp32::readA() { return digitalRead(_a); }
//int EncoderHal_ArduinoEsp32::readB() { return digitalRead(_b); }
int EncoderHal_ArduinoEsp32::readA() {
  return gpio_get_level((gpio_num_t)_a);
}

int EncoderHal_ArduinoEsp32::readB() {
  return gpio_get_level((gpio_num_t)_b);
}

int EncoderHal_ArduinoEsp32::readBtn() { return digitalRead(_btn); } // LOW when pressed

void EncoderHal_ArduinoEsp32::attachABInterrupts(void (*isr)()) {
  attachInterrupt(digitalPinToInterrupt(_a), isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(_b), isr, CHANGE);
}

void EncoderHal_ArduinoEsp32::enterCritical() { portENTER_CRITICAL(&_mux); }
void EncoderHal_ArduinoEsp32::exitCritical()  { portEXIT_CRITICAL(&_mux); }
