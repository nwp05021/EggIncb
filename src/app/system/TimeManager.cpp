#include "TimeManager.h"
#include <WiFi.h>
#include <time.h>
#include <Preferences.h>

static const char* NTP_SERVER = "pool.ntp.org";
static const long GMT_OFFSET = 9 * 3600;

Preferences prefs;

void TimeManager::begin()
{
    loadFromNVS();
    // Always try NTP sync on boot once Wi-Fi is connected.
    _forceSync = true;
}

void TimeManager::loadFromNVS()
{
    prefs.begin("time", true);
    _baseEpoch = prefs.getULong("epoch", 0);
    prefs.end();

    if (_baseEpoch > 0)
    {
        _baseMillis = millis();
        _synced = false;   // 🔥 중요: 아직 NTP 동기화 안됨
    }
}

void TimeManager::saveToNVS(time_t t)
{
    prefs.begin("time", false);
    prefs.putULong("epoch", t);
    prefs.end();
}

void TimeManager::loop()
{
    if (WiFi.status() != WL_CONNECTED) return;

    static bool ntpStarted = false;

    if (!ntpStarted)
    {
        configTime(GMT_OFFSET, 0,
                   "time.google.com",
                   "pool.ntp.org");
        ntpStarted = true;
    }

    if (_forceSync || !_synced || millis() - _lastSyncAttempt > 86400000UL)
    {
        struct tm timeinfo;

        if (getLocalTime(&timeinfo, 5000))   // 5초 타임아웃
        {
            time_t nowEpoch = mktime(&timeinfo);
            _baseEpoch = nowEpoch;
            _baseMillis = millis();
            _synced = true;
            _forceSync = false;
            saveToNVS(nowEpoch);

            Serial.println("[Time] NTP sync OK");
        }
        else
        {
            Serial.println("[Time] NTP failed");
        }

        _lastSyncAttempt = millis();
    }
}

time_t TimeManager::now()
{
    // If we have a base epoch (from NVS or NTP), we can provide a running clock.
    if (_baseEpoch == 0) return 0;

    unsigned long delta = (millis() - _baseMillis) / 1000;
    return (time_t)_baseEpoch + (time_t)delta;
}

void TimeManager::requestSync()
{
    _forceSync = true;
}

void TimeManager::getTimeString(char* buf, size_t len)
{
    time_t t = now();
    if (t == 0)
    {
        snprintf(buf, len, "--:--:--");
        return;
    }

    struct tm* ti = localtime(&t);
    snprintf(buf, len, "%02d:%02d:%02d",
             ti->tm_hour,
             ti->tm_min,
             ti->tm_sec);
}