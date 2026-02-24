#pragma once
#include "UiModel.h"
#include "UiIcons.h"
#include <U8g2lib.h>

class UiRenderer {
public:
  UiRenderer();

  void begin();

  // Main (GrowBed-like layout)
  void drawMain(const UiModel& m, uint32_t uptimeMs);

  // Settings menu with pagination and highlight (unchanged usage)
  void drawSettingsMenu(const UiModel& m,
                        const char* title,
                        const char* const* labels,
                        const char* const* values,
                        int itemCount,
                        int cursorAbs,
                        int page,
                        int pageSize);

  // Value edit page (blink by showValue flag)
  void drawEditValue(const char* label,
                     const char* unit,
                     int32_t value,
                     bool showValue,
                     bool isBool,
                     bool isX10);

  // confirm page (No/Yes)
  void drawConfirm(const char* title,
                   const char* line1,
                   const char* line2,
                   int cursor01);

private:
  // 1.3 OLED commonly SH1106. Change to SSD1306 if needed.
  U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;

  void drawHeader(uint32_t uptimeMs, uint8_t page, bool fault);
  void drawBodyPages(const UiModel& m);
  void drawFooter(const UiModel& m);

  static void fmtUptime(char* out, size_t outSz, uint32_t uptimeMs);
  static void fmtX10(char* out, size_t outSz, int16_t v_x10);
  void highlightRow(int yTop, int height);
};
