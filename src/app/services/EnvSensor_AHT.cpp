#include "EnvSensor_AHT.h"
#include <Adafruit_AHTX0.h>
#include "hal/I2cLock.h"

static Adafruit_AHTX0 aht;

extern I2cLock g_i2cLock;

bool EnvSensor_AHT::begin() {
  _ok = aht.begin();
  _lastRead = 0;
  return _ok;
}

void EnvSensor_AHT::tick(uint32_t nowMs) {
  if (!_ok) return;
  if (nowMs - _lastRead < 1000) return;
  _lastRead = nowMs;

  sensors_event_t humidity, temp;
  if (g_i2cLock.lock(50)) {
    aht.getEvent(&humidity, &temp);
    g_i2cLock.unlock();

    _t = temp.temperature;
    _h = humidity.relative_humidity;

  }  
}
