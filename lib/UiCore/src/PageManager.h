#pragma once
#include <vector>
#include "Page.h"

class PageManager {
public:
  void push(Page* p);
  void pop();
  void replace(Page* p);

  Page* top();

  void handleEncoder(int delta);
  void handleClick();
  void handleLongPress();
  void handleVeryLongPress();

  void render(class UiRenderer& r);

private:
  std::vector<Page*> _stack;
};
