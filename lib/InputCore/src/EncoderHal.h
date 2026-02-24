#pragma once
#include <stdint.h>

class EncoderHal {
public:
  virtual ~EncoderHal() = default;

  virtual void begin() = 0;

  // raw reads (must be fast; used in ISR)
  virtual int readA() = 0;
  virtual int readB() = 0;
  virtual int readBtn() = 0;

  virtual void attachABInterrupts(void (*isr)()) = 0;

  // critical section for isrDelta snapshot
  virtual void enterCritical() = 0;
  virtual void exitCritical() = 0;

  virtual uint32_t millisNow() = 0;
};
