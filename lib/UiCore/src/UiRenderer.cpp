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

void UiRenderer::drawHeader(uint32_t uptimeMs, uint8_t page, bool fault) {
  u8g2.setFont(u8g2_font_6x10_tf);

  char buf[24];
  fmtUptime(buf, sizeof(buf), uptimeMs);
  u8g2.drawStr(2, 10, buf);

  if (fault) {
    u8g2.drawStr(120, 10, "!");
  } else {
    char pbuf[8];
    snprintf(pbuf, sizeof(pbuf), "P%u", (unsigned)(page + 1));
    u8g2.drawStr(112, 10, pbuf);
  }

  u8g2.drawHLine(0, 12, 128);
}

void UiRenderer::drawBodyPages(const UiModel& m) {
  // Body area y=13..51
  u8g2.setFont(u8g2_font_unifont_t_korean2);

  char line1[32];
  char line2[32];

  switch (m.mainPage) {
    case 0: {
      if (m.sensorOk) {
        char tbuf[16], hbuf[16];
        fmtX10(tbuf, sizeof(tbuf), m.currentTemp_x10);
        fmtX10(hbuf, sizeof(hbuf), m.currentHum_x10);
        snprintf(line1, sizeof(line1), "온도:%5s C", tbuf);
        // currentHum_x10: "55.0" but we show integer percent by default for readability
        int humI = (int)(m.currentHum_x10 / 10);
        snprintf(line2, sizeof(line2), "습도:%5d %%", humI);
      } else {
        snprintf(line1, sizeof(line1), "온도: --.- C");
        snprintf(line2, sizeof(line2), "습도: -- %%");
      }
      break;
    }
    case 1: {
      // Targets (effective when AUTO)
      char tbuf[16], hbuf[16];
      fmtX10(tbuf, sizeof(tbuf), m.effectiveTargetTemp_x10);
      fmtX10(hbuf, sizeof(hbuf), m.effectiveTargetHum_x10);
      snprintf(line1, sizeof(line1), "목표:%5s C", tbuf);
      int humI = (int)(m.effectiveTargetHum_x10 / 10);
      snprintf(line2, sizeof(line2), "목표습도:%3d %%", humI);
      break;
    }
    case 2: {
      snprintf(line1, sizeof(line1), "모터:%3us/%3um",
               (unsigned)m.motorOnSec, (unsigned)m.motorOffMin);
      snprintf(line2, sizeof(line2), "히터:%s 가습:%s",
               m.heaterOn ? "ON" : "OFF",
               m.humidifierOn ? "ON" : "OFF");
      break;
    }
    default:
      snprintf(line1, sizeof(line1), "-");
      snprintf(line2, sizeof(line2), "-");
      break;
  }

  u8g2.drawUTF8(2, 28, line1);
  u8g2.drawUTF8(2, 48, line2);

  u8g2.setDrawColor(1);
  u8g2.drawHLine(0, 52, 128);
}

void UiRenderer::drawFooter(const UiModel& m) {
  const int y = 56;

  // icons 8x8
  u8g2.drawXBMP(0,  y, 8, 8, ICON_HEATER);
  u8g2.drawXBMP(18, y, 8, 8, ICON_MOTOR);
  u8g2.drawXBMP(36, y, 8, 8, ICON_FAN);
  u8g2.drawXBMP(54, y, 8, 8, ICON_HUMID);

  u8g2.setFont(u8g2_font_6x10_tf);

  // ON markers (invert small 10x10 box behind icon)
  auto mark = [&](int x, bool on) {
    if (!on) return;
    u8g2.setDrawColor(1);
    u8g2.drawBox(x-1, y-1, 10, 10);
    u8g2.setDrawColor(0);
    // redraw icon in invert
    if (x==0) u8g2.drawXBMP(x, y, 8, 8, ICON_HEATER);
    else if (x==18) u8g2.drawXBMP(x, y, 8, 8, ICON_MOTOR);
    else if (x==36) u8g2.drawXBMP(x, y, 8, 8, ICON_FAN);
    else if (x==54) u8g2.drawXBMP(x, y, 8, 8, ICON_HUMID);
    u8g2.setDrawColor(1);
  };

  mark(0,  m.heaterOn);
  mark(18, m.motorOn);
  mark(36, m.fanOn);
  mark(54, m.humidifierOn);

  // right side: D+day
  char buf[12];
  snprintf(buf, sizeof(buf), "D+%02u", (unsigned)m.incubationDay);
  u8g2.drawStr(92, 63, buf);
}

void UiRenderer::drawMain(const UiModel& m, uint32_t uptimeMs) {
  begin();
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);

  drawHeader(uptimeMs, m.mainPage, false);
  // connection / alarm indicators
  u8g2.setFont(u8g2_font_5x8_tf);
  if (m.alarm) {
    u8g2.setCursor(108, 8);
    u8g2.print("ALM");
  } else if (m.provisioning) {
    u8g2.setCursor(84, 8);
    u8g2.print("BLE");
  } else if (m.wifiConnected) {
    u8g2.setCursor(84, 8);
    u8g2.print("WiFi");
  }

  // connection indicators
  u8g2.setFont(u8g2_font_5x8_tf);
  if (m.provisioning) {
    u8g2.setCursor(84, 8);
    u8g2.print("BLE");
  } else if (m.wifiConnected) {
    u8g2.setCursor(84, 8);
    u8g2.print("WiFi");
  }

  drawBodyPages(m);
  drawFooter(m);

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
  u8g2.setCursor(0, 12);
  u8g2.print(title);

  // labels may include Korean
  u8g2.setFont(u8g2_font_unifont_t_korean2);

  u8g2.setFont(u8g2_font_6x10_tf);

  int maxPage = (itemCount - 1) / pageSize;
  u8g2.setCursor(100, 12);
  u8g2.print(page + 1);
  u8g2.print("/");
  u8g2.print(maxPage + 1);

  int start = page * pageSize;
  int end = start + pageSize;
  if (end > itemCount) end = itemCount;

  int row = 0;
  for (int i = start; i < end; ++i, ++row) {
    int y = 26 + row * 10;
    int yTop = y - 8;
    bool sel = (i == cursorAbs);

    u8g2.setDrawColor(1);
    if (sel) highlightRow(yTop, 10);

    u8g2.setCursor(0, y);
    u8g2.print(sel ? ">" : " ");
    u8g2.print(labels[i]);

    if (values && values[i]) {
      u8g2.setCursor(84, y);
      u8g2.print(values[i]);
    }

    if (sel) u8g2.setDrawColor(1);
  }

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

  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setCursor(0, 12);
  u8g2.print("Edit");

  u8g2.setFont(u8g2_font_unifont_t_korean2);
  u8g2.setCursor(0, 28);
  u8g2.print(label);

  u8g2.setCursor(0, 56);
  if (showValue) {
    u8g2.setFont(u8g2_font_logisoso24_tf);

    if (isBool) {
      u8g2.print(value ? "ON" : "OFF");
    } else if (isX10) {
      char buf[16];
      fmtX10(buf, sizeof(buf), (int16_t)value);
      u8g2.print(buf);
    } else {
      u8g2.print(value);
    }

    u8g2.setFont(u8g2_font_6x10_tf);
    if (unit && unit[0]) {
      u8g2.print(" ");
      u8g2.print(unit);
    }
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
  u8g2.setCursor(0, 12);
  u8g2.print(title);

  u8g2.setFont(u8g2_font_unifont_t_korean2);
  u8g2.setCursor(0, 28);
  u8g2.print(line1);

  u8g2.setCursor(0, 40);
  u8g2.print(line2);

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
    u8g2.setCursor(x + 12, 61);
    u8g2.print(noYes[i]);
    u8g2.setDrawColor(1);
  }

  u8g2.sendBuffer();
}
