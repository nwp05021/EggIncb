# Porting UI Engine to other Beds (LineBed/GrowBed/CloudBed)

This UI engine is designed to be portable:
- Only requires: EncoderEvents + UiRenderer backend + UiModel

## What you keep
- lib/InputCore (encoder feel)
- lib/UiCore (page stack)
- UiModel + callbacks pattern

## What you swap per device
- UiRenderer backend (OLED/TFT)
- Model fields (device-specific sensors/actuators)
- Pages (menus and edit items)
