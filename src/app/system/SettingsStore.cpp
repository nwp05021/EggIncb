#include "SettingsStore.h"
#include <stddef.h>
#include <Preferences.h>
#include <string.h>

static uint32_t crc32_sw(const uint8_t* data, size_t len) {
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (int b = 0; b < 8; ++b) {
      uint32_t mask = -(crc & 1u);
      crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }
  }
  return ~crc;
}

static Preferences prefs;

bool SettingsStore::begin(const char* nvsNamespace) {
  _ns = nvsNamespace;
  return true;
}

uint32_t SettingsStore::crc32(const uint8_t* data, size_t len) const {
  return crc32_sw(data, len);
}

bool SettingsStore::load(PersistedData& out) {
  memset(&out, 0, sizeof(out));

  if (!prefs.begin(_ns, true)) return false;
  size_t n = prefs.getBytesLength("blob");
  if (n != sizeof(PersistedData)) { prefs.end(); return false; }
  prefs.getBytes("blob", &out, sizeof(PersistedData));
  prefs.end();

  uint32_t expected = out.crc;
  out.crc = 0;
  uint32_t actual = crc32(reinterpret_cast<const uint8_t*>(&out), sizeof(PersistedData));
  out.crc = expected;

  return expected == actual;
}

bool SettingsStore::save(const PersistedData& in) {
  PersistedData tmp = in;
  tmp.crc = 0;
  tmp.crc = crc32(reinterpret_cast<const uint8_t*>(&tmp), sizeof(PersistedData));

  if (!prefs.begin(_ns, false)) return false;
  bool ok = prefs.putBytes("blob", &tmp, sizeof(PersistedData)) == sizeof(PersistedData);
  prefs.end();
  return ok;
}

void SettingsStore::factoryReset() {
  if (!prefs.begin(_ns, false)) return;
  prefs.clear();
  prefs.end();
}
