#pragma once
#include <stdint.h>

class EnvSensor_AHT {
public:
  bool begin();
  void tick(uint32_t nowMs);

  bool ok() const { return _ok; }
  float temperatureC() const { return _t; }
  float humidityRH() const { return _h; }

private:
  bool _ok = false;
  float _t = 0.0f;
  float _h = 0.0f;
  uint32_t _lastRead = 0;
};
