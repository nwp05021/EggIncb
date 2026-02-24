#include "I2cHal.h"
#include <Wire.h>
#include "I2cRecovery.h"

static int g_sdaPin = -1;
static int g_sclPin = -1;

void I2cHal::begin(int sda, int scl) {
  g_sdaPin = sda;
  g_sclPin = scl;

  // Attempt recovery BEFORE Wire begins (in case bus is stuck after brownout/glitch)
  I2cRecovery::busRecoveryGPIO(sda, scl);

  Wire.begin(sda, scl);

  // 안정성 우선: 100kHz + 짧은 timeout
  // (원하면 400kHz로 올리되, 문제 재현되는지 반드시 확인)
  I2cRecovery::configureWire(200000, 20);
}

// 필요하면 어디서든 호출할 수 있게 보조 함수 하나 추가 (선택)
bool I2cHal::recoverBus() {
  if (g_sdaPin < 0 || g_sclPin < 0) return false;
  I2cRecovery::busRecoveryGPIO(g_sdaPin, g_sclPin);
  // Wire는 유지, 다음 트랜잭션부터 정상화 기대
  return true;
}