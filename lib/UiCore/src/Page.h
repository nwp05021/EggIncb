#pragma once
class UiRenderer;

class Page {
public:
  virtual ~Page() = default;
  virtual void onEnter() {}
  virtual void onExit() {}
  virtual void onEncoder(int delta) {}
  virtual void onClick() {}
  virtual void onLongPress() {}
  virtual void onVeryLongPress() {}
  virtual void render(UiRenderer& r) = 0;
};
