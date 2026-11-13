#pragma once
#include "../NKFM_UILayout.h"

struct NKFM_EnvData {
  bool  hasBME    = false;
  float bmeTemp   = 0.0f;
  float bmeHum    = 0.0f;
  float bmePres   = 0.0f;

  bool  hasSi7021 = false;
  float siTemp    = 0.0f;
  float siHum     = 0.0f;
};

class NKFM_UILayout_240x135_Sensors : public NKFM_UILayout {
  private:
    NKFM_EnvData _env;

  public:
    void setEnvData(const NKFM_EnvData& data) {
      _env = data;
      uiMarkDirty(D_ALL);
    }

    uint16_t rawWidth() const override  { return 135; }
    uint16_t rawHeight() const override { return 240; }

    void drawStatic(Adafruit_ST7789* tft) override;
    void render(Adafruit_ST7789* tft, const UIState& st, uint8_t dirty) override;
    void tickScroll(Adafruit_ST7789* tft, UIState& st) override { (void)tft; (void)st; }
    void clearRTBox(Adafruit_ST7789* tft) override              { (void)tft; }
};
