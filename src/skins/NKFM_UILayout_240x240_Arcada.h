#pragma once
#include "../NKFM_UILayout.h"

class NKFM_UILayout_240x240_Arcada : public NKFM_UILayout {
  public:
    uint16_t rawWidth() const override  { return 240; }
    uint16_t rawHeight() const override { return 240; }

    void drawStatic(Adafruit_ST7789* tft) override;
    void render(Adafruit_ST7789* tft, const UIState& st, uint8_t dirty) override;
    void tickScroll(Adafruit_ST7789* tft, UIState& st) override;
    void clearRTBox(Adafruit_ST7789* tft) override;
};
