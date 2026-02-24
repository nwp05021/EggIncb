# IncubatorC3 Framework v0.4.0

## Added
- BLE Wi-Fi Provisioning (Arduino-ESP32 WiFiProv):
  - Auto-start provisioning on boot if no STA credentials are saved.
  - Stops provisioning automatically when Wi-Fi connects.
  - Reconnect retry loop when credentials exist.
- UI header indicator:
  - "BLE" while provisioning
  - "WiFi" when connected

## How to provision
- Use Espressif "ESP BLE Provisioning" mobile app.
- Device name: INCUB-xxxxxx (last 3 bytes of MAC)
- PoP: incubator
