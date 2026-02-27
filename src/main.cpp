#include <Arduino.h>
#include <esp_task_wdt.h>
#include "hal/I2cLock.h"
#include "app/IncubatorApp.h"

// 🔥 라이브러리가 요구하는 전역 심볼들
I2cLock g_i2cLock;

// App 인스턴스
IncubatorApp g_app;

void setup()
{
    esp_task_wdt_init(5, true);
    esp_task_wdt_add(NULL);

    g_app.begin();
}

void loop()
{
    g_app.tick();

    esp_task_wdt_reset();    
}