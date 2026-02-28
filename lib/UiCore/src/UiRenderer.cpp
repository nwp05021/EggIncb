#include "UiRenderer.h"
#include <Arduino.h>
#include <cstdio>
#include <cmath>

// 16x16 heater (flame silhouette)
static const unsigned char icon_heater_16x16[] U8X8_PROGMEM = {
  0x00,0x00,
  0x10,0x00,
  0x38,0x00,
  0x7C,0x00,
  0xFE,0x00,
  0x7C,0x00,
  0x38,0x00,
  0x10,0x00,
  0x10,0x00,
  0x38,0x00,
  0x7C,0x00,
  0x38,0x00,
  0x10,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00
};

// 16x16 fan (3 blade)
static const unsigned char icon_fan_16x16[] U8X8_PROGMEM = {
  0x00,0x00,
  0x18,0x00,
  0x3C,0x00,
  0x18,0x00,
  0x7E,0x00,
  0x18,0x00,
  0x3C,0x00,
  0x18,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00
};

// 16x16 motor (gear-like)
static const unsigned char icon_motor_16x16[] U8X8_PROGMEM = {
  0x18,0x00,
  0x3C,0x00,
  0x66,0x00,
  0x42,0x00,
  0x81,0x00,
  0x81,0x00,
  0x42,0x00,
  0x66,0x00,
  0x3C,0x00,
  0x18,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00
};

// 16x16 water drop
static const unsigned char icon_humid_16x16[] U8X8_PROGMEM = {
  0x10,0x00,
  0x38,0x00,
  0x7C,0x00,
  0xFE,0x00,
  0xFE,0x00,
  0x7C,0x00,
  0x38,0x00,
  0x10,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00,
  0x00,0x00
};

UiRenderer::UiRenderer()
: u8g2(U8G2_R0, U8X8_PIN_NONE) {}

void UiRenderer::begin() {
  static bool inited = false;
  if (inited) return;
  inited = true;

  u8g2.begin();
  u8g2.clearDisplay();
  u8g2.setContrast(200);

  u8g2.enableUTF8Print();
  u8g2.setFontMode(1);
}

void UiRenderer::fmtUptime(char* out, size_t outSz, uint32_t uptimeMs) {
  uint32_t s = uptimeMs / 1000;
  uint32_t m = s / 60;
  uint32_t h = m / 60;
  snprintf(out, outSz, "%02lu:%02lu:%02lu",
           (unsigned long)(h % 24),
           (unsigned long)(m % 60),
           (unsigned long)(s % 60));
}

void UiRenderer::fmtX10(char* out, size_t outSz, int16_t v_x10) {
  int16_t i = v_x10 / 10;
  int16_t f = abs(v_x10 % 10);
  snprintf(out, outSz, "%d.%d", (int)i, (int)f);
}

void UiRenderer::highlightRow(int yTop, int height) {
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, yTop, 128, height);
  u8g2.setDrawColor(0);
}

/* ---------------- Main (GrowBed-like) ---------------- */

void UiRenderer::drawHeader(const UiModel& m, uint32_t uptimeMs, bool fault) {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(2, 10, m.timeStr);

  // Right: alarm mark or Wi-Fi state + page
  char rbuf[16];
  if (fault || m.alarm) {
    snprintf(rbuf, sizeof(rbuf), "!");
  } else {
    const char* net = m.provisioning ? "BLE" : (m.wifiConnected ? "WiFi" : "----");
    snprintf(rbuf, sizeof(rbuf), "%s P%u", net, (unsigned)(m.mainPage + 1));
  }
  int rw = strW(u8g2, rbuf);
  u8g2.drawStr(126 - rw, 10, rbuf);

  // Divider
  u8g2.drawHLine(0, 12, 128);
}

void UiRenderer::drawBodyPages(const UiModel& m) {
  // Clean body area (no frames)
  char buf[32];

  switch (m.mainPage) {
    case 0: {
      // Live
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(2, 24, "TEMP");
      u8g2.drawStr(86, 24, "HUM");

      if (m.sensorOk) {
        char tbuf[16];
        fmtX10(tbuf, sizeof(tbuf), m.currentTemp_x10);

        // Temp (big)
        u8g2.setFont(u8g2_font_logisoso24_tf);
        u8g2.drawStr(2, 50, tbuf);
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(56, 50, "C");

        // Hum (medium)
        int humI = (int)(m.currentHum_x10 / 10);
        u8g2.setFont(u8g2_font_logisoso16_tf);
        snprintf(buf, sizeof(buf), "%d%%", humI);
        u8g2.drawStr(86, 48, buf);
      } else {
        u8g2.setFont(u8g2_font_logisoso24_tf);
        u8g2.drawStr(2, 50, "--.-");
        u8g2.setFont(u8g2_font_logisoso16_tf);
        u8g2.drawStr(86, 48, "--%");
      }
      break;
    }

    case 1: {
      // Targets
      char tEff[16], tHys[16];
      fmtX10(tEff, sizeof(tEff), m.effectiveTargetTemp_x10);
      fmtX10(tHys, sizeof(tHys), m.tempHyst_x10);

      int hEff = (int)(m.effectiveTargetHum_x10 / 10);
      int hHys = (int)(m.humHyst_x10 / 10);

      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(2, 24, "TGT");

      // Main values
      u8g2.setFont(u8g2_font_logisoso16_tf);
      u8g2.drawStr(2, 46, tEff);
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(38, 46, "C");

      u8g2.setFont(u8g2_font_logisoso16_tf);
      snprintf(buf, sizeof(buf), "%d", hEff);
      u8g2.drawStr(66, 46, buf);
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(90, 46, "%");

      // Sub line
      u8g2.setFont(u8g2_font_6x10_tf);
      snprintf(buf, sizeof(buf), "HYS %sC %d%%", tHys, hHys);
      u8g2.drawStr(30, 24, buf);

      break;
    }

    case 2: {
      // System (include network)
      u8g2.setFont(u8g2_font_6x10_tf);
      // Keep enough bottom margin (status bar starts at y=52)
      u8g2.drawStr(2, 22, "SYSTEM");

      // Line 1: network
      const char* net = m.provisioning ? "BLE-PROV" : (m.wifiConnected ? "WiFi:ON" : "WiFi:OFF");
      u8g2.drawStr(2, 34, net);

      // Line 2: motor schedule
      snprintf(buf, sizeof(buf), "Motor %us/%um", (unsigned)m.motorOnSec, (unsigned)m.motorOffMin);
      u8g2.drawStr(2, 46, buf);

      // (3-line layout only; last line at y=46 to avoid clipping)
      break;
    }
  }
}

void UiRenderer::drawStatusBar(const UiModel& m, uint32_t uptimeMs)
{
  (void)uptimeMs;

  const int footerTop = 52;
  const int iconY = 53;

  u8g2.drawHLine(0, footerTop - 1, 128);

  // ---- HEATER (원 안 H + 깜박임) ----
  {
    int x = 2;

    if (m.heaterOn && (millis()/400)%2==0)
    {
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(x+3, iconY+10, "H");
    }
    else
    {
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(x+3, iconY+10, "H");
    }
  }

  // ---- MOTOR (시소) ----
  {
    int x = 22;
    u8g2.drawDisc(x+6, iconY+8, 1);

    if (m.motorOn)
    {
      if ((millis()/600)%2==0)
        u8g2.drawLine(x+1, iconY+4, x+11, iconY+7);
      else
        u8g2.drawLine(x+1, iconY+7, x+11, iconY+4);
    }
    else
    {
      u8g2.drawLine(x+1, iconY+6, x+11, iconY+6);
    }
  }

  // ---- FAN (2프레임 회전) ----
  {
    int x = 42;
    u8g2.drawDisc(x+6, iconY+6, 1);

    if (m.fanOn)
    {
      if ((millis()/200)%2==0) {
        u8g2.drawLine(x+6,iconY+1,x+6,iconY+11);
        u8g2.drawLine(x+1,iconY+6,x+11,iconY+6);
      } else {
        u8g2.drawLine(x+2,iconY+2,x+10,iconY+10);
        u8g2.drawLine(x+2,iconY+10,x+10,iconY+2);
      }
    }
    else
    {
      u8g2.drawLine(x+6,iconY+1,x+6,iconY+11);
      u8g2.drawLine(x+1,iconY+6,x+11,iconY+6);
    }
  }

  // ---- HUMID (점 3개 상승) ----
  {
    int x = 62;

    // 기준 막대
    u8g2.drawLine(x+2, iconY+10, x+10, iconY+10);

    if (m.humidifierOn)
    {
      uint8_t frame = (millis()/200)%6;
      int offsets[3] = { frame, (frame+2)%6, (frame+4)%6 };

      for (int i=0;i<3;i++)
      {
        int dy = offsets[i];
        u8g2.drawDisc(x+3+i*3, iconY+9-dy, 1);
      }
    }
  }

  // ---- D+ ----
  char buf[10];
  {
    uint8_t d = (m.elapsedDay > 0) ? m.elapsedDay : 1;
    // If elapsedDay is not available, fall back to DAY setting.
    snprintf(buf, sizeof(buf), "D+%02u", (unsigned)d);

    int tw = strW(u8g2, buf);
    int x = 128 - tw - 4;

    // 얇은 박스 추가
    u8g2.drawFrame(x - 3, 52, tw + 6, 12);
    u8g2.drawStr(x, 62, buf);    
  }
}

void UiRenderer::drawMain(const UiModel& m, uint32_t uptimeMs) {
  begin();
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);

  drawHeader(m, uptimeMs, false);
  drawBodyPages(m);
  drawStatusBar(m, uptimeMs);

  u8g2.setDrawColor(1);
  u8g2.sendBuffer();
}

/* ---------------- Settings Menu ---------------- */

void UiRenderer::drawSettingsMenu(const UiModel& m,
                                  const char* title,
                                  const char* const* labels,
                                  const char* const* values,
                                  int itemCount,
                                  int cursorAbs,
                                  int page,
                                  int pageSize) {
  (void)m;
  begin();
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);

  u8g2.setFont(u8g2_font_6x10_tf);
  // Header
  u8g2.drawStr(2, 10, title);
  int maxPage = (itemCount - 1) / pageSize;
  char pbuf[8];
  snprintf(pbuf, sizeof(pbuf), "%d/%d", page + 1, maxPage + 1);
  int pw = strW(u8g2, pbuf);
  u8g2.drawStr(126 - pw, 10, pbuf);
  u8g2.drawHLine(0, 12, 128);

  // menu rows use small font for 4-line layout
  u8g2.setFont(u8g2_font_6x10_tf);

  int start = page * pageSize;
  int end = start + pageSize;
  if (end > itemCount) end = itemCount;

  int row = 0;
  for (int i = start; i < end; ++i, ++row) {
    int y = 24 + row * 10;
    int yTop = y - 8;
    bool sel = (i == cursorAbs);

    u8g2.setDrawColor(1);
    if (sel) {
      u8g2.drawBox(0, yTop, 128, 10);
      u8g2.setDrawColor(0);
    }

    u8g2.setCursor(6, y);
    u8g2.print(labels[i]);

    if (values && values[i]) {
      int vw = strW(u8g2, values[i]);
      u8g2.setCursor(126 - vw, y);
      u8g2.print(values[i]);
    }

    if (sel) u8g2.setDrawColor(1);
  }

  // Subtle frame for the list area
  u8g2.setDrawColor(1);
  u8g2.drawFrame(0, 14, 128, 50);

  u8g2.sendBuffer();
}

void UiRenderer::drawEditValue(const char* label,
                               const char* unit,
                               int32_t value,
                               bool showValue,
                               bool isBool,
                               bool isX10) {
  begin();
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);

  // Header
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(2, 10, "EDIT");
  if (unit && unit[0]) {
    int uw = strW(u8g2, unit);
    u8g2.drawStr(126 - uw, 10, unit);
  }
  u8g2.drawHLine(0, 12, 128);

  // Label (UTF8)
  u8g2.setFont(u8g2_font_unifont_t_korean2);
  u8g2.drawUTF8(4, 28, label);

  // Boxed value area
  const int boxX = 8, boxY = 32, boxW = 112, boxH = 28;
  u8g2.setDrawColor(1);
  u8g2.drawFrame(boxX, boxY, boxW, boxH);

  if (showValue) {
    // Invert inside box for focus
    u8g2.drawBox(boxX + 1, boxY + 1, boxW - 2, boxH - 2);
    u8g2.setDrawColor(0);

    char vbuf[20];
    if (isBool) {
      snprintf(vbuf, sizeof(vbuf), "%s", value ? "ON" : "OFF");
    } else if (isX10) {
      fmtX10(vbuf, sizeof(vbuf), (int16_t)value);
    } else {
      snprintf(vbuf, sizeof(vbuf), "%ld", (long)value);
    }

    u8g2.setFont(u8g2_font_logisoso24_tf);
    int vw = strW(u8g2, vbuf);
    int vx = 64 - vw / 2;
    u8g2.drawStr(vx, 56, vbuf);
    u8g2.setDrawColor(1);
  }

  u8g2.sendBuffer();
}

void UiRenderer::drawConfirm(const char* title,
                             const char* line1,
                             const char* line2,
                             int cursor01) {
  begin();
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);

  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(2, 10, title);
  u8g2.drawHLine(0, 12, 128);

  u8g2.setFont(u8g2_font_unifont_t_korean2);
  u8g2.drawUTF8(4, 30, line1);
  u8g2.drawUTF8(4, 44, line2);

  const char* noYes[2] = {"No", "Yes"};

  int yTop = 50, h = 14;
  for (int i = 0; i < 2; ++i) {
    int x = (i == 0) ? 10 : 70;
    int w = 40;
    if (cursor01 == i) {
      u8g2.setDrawColor(1);
      u8g2.drawBox(x, yTop, w, h);
      u8g2.setDrawColor(0);
    } else {
      u8g2.setDrawColor(1);
      u8g2.drawFrame(x, yTop, w, h);
    }
    // center label
    u8g2.setFont(u8g2_font_6x10_tf);
    int lw = strW(u8g2, noYes[i]);
    u8g2.setCursor(x + (w - lw) / 2, 61);
    u8g2.print(noYes[i]);
    u8g2.setDrawColor(1);
  }

  u8g2.sendBuffer();
}
