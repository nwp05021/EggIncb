#include "UiRenderer.h"
#include <Arduino.h>
#include <cstdio>
#include <cmath>

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

  // Left: time (uptime HH:MM)
  char tbuf[16];
  fmtUptime(tbuf, sizeof(tbuf), uptimeMs);
  // show HH:MM for a calmer header
  char hhmm[6];
  hhmm[0]=tbuf[0]; hhmm[1]=tbuf[1]; hhmm[2]=':'; hhmm[3]=tbuf[3]; hhmm[4]=tbuf[4]; hhmm[5]=0;
  u8g2.drawStr(2, 10, hhmm);

  // Right: alarm mark or mode + page
  const bool autoMode = (m.scheduleMode == 0);
  const char* mode = autoMode ? "AUTO" : "MAN";

  char rbuf[12];
  if (fault || m.alarm) {
    snprintf(rbuf, sizeof(rbuf), "! %s", mode);
  } else {
    snprintf(rbuf, sizeof(rbuf), "%s P%u", mode, (unsigned)(m.mainPage + 1));
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
      char tSet[16], tEff[16], tHys[16];
      fmtX10(tSet, sizeof(tSet), m.targetTemp_x10);
      fmtX10(tEff, sizeof(tEff), m.effectiveTargetTemp_x10);
      fmtX10(tHys, sizeof(tHys), m.tempHyst_x10);

      int hSet = (int)(m.targetHum_x10 / 10);
      int hEff = (int)(m.effectiveTargetHum_x10 / 10);
      int hHys = (int)(m.humHyst_x10 / 10);

      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(2, 24, "SET");

      // Main values
      u8g2.setFont(u8g2_font_logisoso16_tf);
      u8g2.drawStr(2, 46, tSet);
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(38, 46, "C");

      u8g2.setFont(u8g2_font_logisoso16_tf);
      snprintf(buf, sizeof(buf), "%d", hSet);
      u8g2.drawStr(66, 46, buf);
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(90, 46, "%");

      // Sub line
      u8g2.setFont(u8g2_font_6x10_tf);
      if (m.scheduleMode == 0) {
        snprintf(buf, sizeof(buf), "AUTO %sC %d%%", tEff, hEff);
      } else {
        snprintf(buf, sizeof(buf), "HYS  %sC %d%%", tHys, hHys);
      }
      u8g2.drawStr(2, 34, buf);

      break;
    }

    case 2: {
      // System
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(2, 24, "SYSTEM");
      snprintf(buf, sizeof(buf), "Motor %us/%um", (unsigned)m.motorOnSec, (unsigned)m.motorOffMin);
      u8g2.drawStr(2, 36, buf);
      snprintf(buf, sizeof(buf), "Heat %s  Hum %s", m.heaterOn ? "ON" : "OFF", m.humidifierOn ? "ON" : "OFF");
      u8g2.drawStr(2, 48, buf);
      break;
    }
  }
}

void UiRenderer::drawStatusBar(const UiModel& m, uint32_t uptimeMs) {
  (void)uptimeMs;
  // Bottom row: 4 clear blocks (HEAT/MOTOR/FAN/HUM). Active = inverted.
  const int yTop = 54;
  const int h = 10;
  u8g2.setDrawColor(1);
  u8g2.drawHLine(0, yTop, 128);

  struct Item { const char* label; bool on; };
  Item items[4] = {
    {"HEAT",  m.heaterOn},
    {"MOTOR", m.motorOn},
    {"FAN",   m.fanOn},
    {"HUM",   m.humidifierOn},
  };

  u8g2.setFont(u8g2_font_6x10_tf);
  for (int i = 0; i < 4; ++i) {
    int x = i * 32;
    if (items[i].on) {
      u8g2.setDrawColor(1);
      u8g2.drawBox(x, yTop + 1, 32, h);
      u8g2.setDrawColor(0);
    } else {
      u8g2.setDrawColor(1);
      u8g2.drawFrame(x, yTop + 1, 32, h);
    }

    int lw = strW(u8g2, items[i].label);
    int tx = x + (32 - lw) / 2;
    u8g2.drawStr(tx, 63, items[i].label);
    u8g2.setDrawColor(1);
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
