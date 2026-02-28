#include "IncubatorMainPage.h"
#include "../UiRenderer.h"
#include "../PageManager.h"
#include <Arduino.h>

IncubatorMainPage::IncubatorMainPage(UiModel& m, PageManager& mgr, Page* settingsPage)
: _m(m), _mgr(mgr), _settings(settingsPage) {}

void IncubatorMainPage::onEncoder(int delta) {
  if (delta == 0) return;
  int dir = (delta > 0) ? 1 : -1;

  const uint8_t maxPage = 2; // P1..P3 currently
  int p = (int)_m.mainPage + dir;
  // Clamp at both ends (no wrap)
  if (p < 0) p = 0;
  if (p > (int)maxPage) p = (int)maxPage;
  _m.mainPage = (uint8_t)p;
}

void IncubatorMainPage::onClick() {
  if (_settings) _mgr.push(_settings);
}

void IncubatorMainPage::render(UiRenderer& r) {
  r.drawMain(_m, millis());
}
