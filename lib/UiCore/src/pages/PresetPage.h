#pragma once

#include "../Page.h"

#include <stdint.h>

class PageManager;
class UiApp;
class UiRenderer;
class ConfirmPage;

class PresetPage : public Page {
public:
  PresetPage(PageManager& mgr, UiApp* app);

  void bindPreset(uint8_t* presetId,
                  int16_t* dayTempTable_x10,
                  int16_t* dayHumTable_x10);

  void onEnter() override;
  void onEncoder(int delta) override;
  void onClick() override;
  void onLongPress() override;
  void render(UiRenderer& r) override;

  // Called via ConfirmPage thunk functions
  void applyPreset(uint8_t id);

private:
  void ensureVisible();

  PageManager& _mgr;
  UiApp* _app = nullptr;
  ConfirmPage* _confirm = nullptr;

  uint8_t* _pPresetId = nullptr;
  int16_t* _pDayTemp = nullptr;
  int16_t* _pDayHum = nullptr;

  int _cursor = 0;
  int _page = 0;

  static constexpr int PAGE_SIZE = 4;
};
