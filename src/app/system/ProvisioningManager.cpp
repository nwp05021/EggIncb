#include "ProvisioningManager.h"

#include <Arduino.h>
#include <WiFi.h>

// Arduino-ESP32 WiFi provisioning (BLE / SoftAP)
#include <WiFiProv.h>

// ESP-IDF helpers
#include <esp_wifi.h>

static const char* PROV_POP = "incubator";  // Proof-of-possession (change for production)
static const char* PROV_SERVICE_KEY = nullptr; // optional custom service key (usually nullptr)

static void buildServiceName(char* out, size_t n) {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  // Use last 3 bytes for readability
  snprintf(out, n, "INCUB-%02X%02X%02X", mac[3], mac[4], mac[5]);
}

bool ProvisioningManager::hasStaConfig() const {
  wifi_config_t cfg;
  memset(&cfg, 0, sizeof(cfg));
  if (esp_wifi_get_config(WIFI_IF_STA, &cfg) != ESP_OK) return false;
  return cfg.sta.ssid[0] != 0;
}

void ProvisioningManager::ensureWifiMode() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
}

void ProvisioningManager::begin() {
  if (_started) return;
  _started = true;

  ensureWifiMode();

  // Try normal Wi-Fi connect using stored creds (if any)
  WiFi.begin();

  startIfNeededOnBoot();
}

void ProvisioningManager::startIfNeededOnBoot() {
  // If there is no saved STA config, start provisioning immediately.
  // This is the "out-of-box" experience.
  if (!hasStaConfig()) {
    startBleProvisioning();
  }
}

void ProvisioningManager::startBleProvisioning() {
  ensureWifiMode();

  char serviceName[24];
  buildServiceName(serviceName, sizeof(serviceName));

  // Security:
  //  - WIFI_PROV_SECURITY_1 uses PoP + encryption (recommended).
  //  - For production, PoP must not be constant; derive from device/QR.
  WiFiProv.beginProvision(
    WIFI_PROV_SCHEME_BLE,
    WIFI_PROV_SCHEME_HANDLER_NONE,
    WIFI_PROV_SECURITY_1,
    PROV_POP,
    serviceName,
    PROV_SERVICE_KEY
  );

  _provisioning = true;
  _lastWifiCheckMs = 0;
}

void ProvisioningManager::stopProvisioning() {
  if (!_provisioning) return;
  // Intentionally do not call core/IDF stop APIs for maximum compatibility across Arduino-ESP32 versions.
  // Once Wi-Fi is connected, provisioning becomes irrelevant.
  _provisioning = false;
}

void ProvisioningManager::tick(uint32_t nowMs) {
  if (!_started) return;

  // Update wifi connected flag (cheap polling)
  if (nowMs - _lastWifiCheckMs >= 500) {
    _lastWifiCheckMs = nowMs;
    _wifiConnected = (WiFi.status() == WL_CONNECTED);
  }

  // If provisioning is active and Wi-Fi becomes connected, stop provisioning.
  if (_provisioning && _wifiConnected) {
    stopProvisioning();
  }

  // If not connected and not provisioning, but we *do* have saved config,
  // keep trying reconnect.
  if (!_wifiConnected && !_provisioning) {
    // WiFi.begin() is safe to call occasionally
    static uint32_t lastRetryMs = 0;
    if (nowMs - lastRetryMs > 5000) {
      lastRetryMs = nowMs;
      WiFi.begin();
    }
  }
}
