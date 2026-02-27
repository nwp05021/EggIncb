#pragma once
#include <Arduino.h>

class TimeManager {
public:
    void begin();
    void loop();

    bool isSynced() const;
    void getTimeString(char* buf, size_t len);
    time_t now();

    void requestSync();

private:
    void loadFromNVS();
    void saveToNVS(time_t t);

    bool _synced = false;
    time_t _baseEpoch = 0;
    unsigned long _baseMillis = 0;
    unsigned long _lastSyncAttempt = 0;

    bool _forceSync = true; // 부팅시 시간 동기화
};