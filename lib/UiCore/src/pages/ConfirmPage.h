#pragma once
#include "../Page.h"

class PageManager;

class ConfirmPage : public Page {
public:
  ConfirmPage(PageManager& mgr);

  void configure(const char* title,
                 const char* line1,
                 const char* line2,
                 void (*onYes)());

  void onEncoder(int delta) override;
  void onClick() override;
  void onLongPress() override;
  void render(class UiRenderer& r) override;

private:
  PageManager& _mgr;

  const char* _title = "";
  const char* _l1 = "";
  const char* _l2 = "";
  void (*_onYes)() = nullptr;

  int _cursor = 0; // 0 No, 1 Yes
};
