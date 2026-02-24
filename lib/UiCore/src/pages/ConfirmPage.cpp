#include "ConfirmPage.h"
#include "../UiRenderer.h"
#include "../PageManager.h"

ConfirmPage::ConfirmPage(PageManager& mgr) : _mgr(mgr) {}

void ConfirmPage::configure(const char* title,const char* line1,const char* line2,void (*onYes)()) {
  _title = title ? title : "";
  _l1 = line1 ? line1 : "";
  _l2 = line2 ? line2 : "";
  _onYes = onYes;
  _cursor = 0;
}

void ConfirmPage::onEncoder(int delta) {
  if (delta == 0) return;
  _cursor = (_cursor == 0) ? 1 : 0;
}

void ConfirmPage::onClick() {
  if (_cursor == 1 && _onYes) _onYes();
  _mgr.pop();
}

void ConfirmPage::onLongPress() { _mgr.pop(); }

void ConfirmPage::render(UiRenderer& r) {
  r.drawConfirm(_title, _l1, _l2, _cursor);
}
