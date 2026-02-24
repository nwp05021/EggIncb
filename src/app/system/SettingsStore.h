#pragma once
#include <stdint.h>
#include <stddef.h>
#include "PersistedData.h"

class SettingsStore {
public:
  bool begin(const char* nvsNamespace = "incubator");
  bool load(PersistedData& out);
  bool save(const PersistedData& in);
  void factoryReset();

private:
  const char* _ns = nullptr;
  uint32_t crc32(const uint8_t* data, size_t len) const;
};
