# IncubatorC3 Framework v0.4.1

## Fixed
- Provisioning stop: use IDF `network_prov_mgr_stop_provisioning()` for compatibility with Arduino-ESP32 cores that don't expose `WiFiProv.endProvision()`.
