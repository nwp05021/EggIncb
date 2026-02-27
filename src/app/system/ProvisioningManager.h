#pragma once
#include <stdint.h>

class ProvisioningManager {
public:
  void begin();
  void tick(uint32_t nowMs);

  // Start BLE provisioning explicitly (e.g., from UI)
  void startBleProvisioning();

  // Stop provisioning session (if active)
  void stopProvisioning();

  // Clear stored Wi-Fi credentials (STA config) and reboot
  void resetProvisioning();

  bool isProvisioning() const { return _provisioning; }
  bool isWifiConnected() const { return _wifiConnected; }

private:
  bool _started = false;
  bool _provisioning = false;
  bool _wifiConnected = false;

  uint32_t _lastWifiCheckMs = 0;

  bool hasStaConfig() const;
  void ensureWifiMode();

  void startIfNeededOnBoot();
};
