# UI Engine Design (GrowBed-like) - v0.2.0

## Goals
- GrowBed-feel input (stable detents + button debounce + long/very-long press)
- Product-like UI: acceleration, highlight, blinking edit
- Extensible navigation: PageStack

## Layers
- InputCore (lib): EncoderHal + EncoderLogic( ISR ) + EncoderController + EncoderAccel
- UiCore (lib): UiApp + PageManager + Pages + UiRenderer
- App (src): domain controllers + persistence + wiring

## PageStack
- PageManager holds a stack of Page*
- MainPage pushes SettingsPage
- SettingsPage pushes EditValuePage/ConfirmPage
- Long press = pop()

## Persistence policy
- UI calls UiCallbacks::onConfigChanged()
- main marks dirty, saves after 1s of inactivity (flash friendly)

## How to add a new page
1) Create `pages/FooPage.h/.cpp` inheriting Page
2) From any page: `_mgr.push(&fooPageInstance);`
3) Implement render + input handlers
