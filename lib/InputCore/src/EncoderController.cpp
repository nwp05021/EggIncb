#include "EncoderController.h"

EncoderController* EncoderController::instance = nullptr;

EncoderController::EncoderController(EncoderHal& hal_) : hal(hal_) {}

void EncoderController::begin(const EncoderConfig& cfg) {
  instance = this;

  hal.begin();
  uint8_t initialAB = (uint8_t)((hal.readA() << 1) | hal.readB());

  int initialBtn = hal.readBtn();

  logic.begin(cfg, initialAB, initialBtn);
  hal.attachABInterrupts(isrRouter);
}

EncoderEvents EncoderController::poll() {
  hal.enterCritical();
  logic.takeIsrDeltaSnapshot();
  hal.exitCritical();

  uint32_t now = hal.millisNow();
  int btnRaw = hal.readBtn();
  return logic.poll(now, btnRaw);
}

void IRAM_ATTR EncoderController::isrRouter() {
  if (instance) instance->handleIsr();
}

void IRAM_ATTR EncoderController::handleIsr() {
  uint8_t ab = (uint8_t)((hal.readA() << 1) | hal.readB());
  logic.onIsrAB(ab);
}
