#pragma once
#include "../NKFM_UILayout.h"

class NKFM_UILayout_240x240_KH : public NKFM_UILayout {
  private:
    void drawCrownIcon(Adafruit_ST7789* tft, int16_t x, int16_t y, uint16_t col);
    void drawStar4(Adafruit_ST7789* tft, int16_t cx, int16_t cy, int16_t r, uint16_t col);
    void drawArcBar(Adafruit_ST7789* tft, int16_t cx, int16_t cy, int16_t rIn, int16_t rOut,
                    uint8_t pct, uint16_t fillCol, uint16_t emptyCol);

  public:
    uint16_t rawWidth() const override  { return 240; }
    uint16_t rawHeight() const override { return 240; }

    void drawStatic(Adafruit_ST7789* tft) override;
    void render(Adafruit_ST7789* tft, const UIState& st, uint8_t dirty) override;
    void tickScroll(Adafruit_ST7789* tft, UIState& st) override;
    void clearRTBox(Adafruit_ST7789* tft) override;
};
