#include "PageManager.h"
#include "UiRenderer.h"

void PageManager::push(Page* p) {
  if (!p) return;
  _stack.push_back(p);
  p->onEnter();
}

void PageManager::pop() {
  if (_stack.empty()) return;
  Page* p = _stack.back();
  p->onExit();
  _stack.pop_back();
}

void PageManager::replace(Page* p) {
  pop();
  push(p);
}

Page* PageManager::top() {
  if (_stack.empty()) return nullptr;
  return _stack.back();
}

void PageManager::handleEncoder(int delta) { if (auto* p = top()) p->onEncoder(delta); }
void PageManager::handleClick()            { if (auto* p = top()) p->onClick(); }
void PageManager::handleLongPress()        { if (auto* p = top()) p->onLongPress(); }
void PageManager::handleVeryLongPress()    { if (auto* p = top()) p->onVeryLongPress(); }

void PageManager::render(UiRenderer& r) { if (auto* p = top()) p->render(r); }
