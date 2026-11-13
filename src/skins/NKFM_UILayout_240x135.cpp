#include "NKFM_UILayout_240x135.h"

void NKFM_UILayout_240x135::drawStatic(Adafruit_ST7789* tft) {
  tft->fillScreen(C_BG);
  tft->fillRect(0, 0, 240, 20, C_CARD);
  tft->drawFastHLine(0, 20, 240, C_BORDER);

  tft->fillRoundRect(2, 23, 236, 54, 6, C_CARD);
  tft->drawRoundRect(2, 23, 236, 54, 6, C_BORDER);

  tft->fillRoundRect(2, 80, 236, 53, 6, C_CARD);
  tft->drawRoundRect(2, 80, 236, 53, 6, C_BORDER);
  tft->fillRect(6, 113, 228, 17, C_BG);
}

void NKFM_UILayout_240x135::clearRTBox(Adafruit_ST7789* tft) {
  tft->fillRect(6, 113, 228, 17, C_BG);
}

void NKFM_UILayout_240x135::tickScroll(Adafruit_ST7789* tft, UIState& st) {
  tft->fillRect(6, 113, 228, 17, C_BG);
  tft->setTextSize(2);
  tft->setCursor(st.scrollX, 114);
  tft->setTextColor(C_ACCENT2);
  tft->print(st.rt);

  tft->fillRect(0, 112, 6, 19, C_BG);
  tft->fillRect(234, 112, 6, 19, C_BG);
  tft->fillRect(2, 112, 4, 19, C_CARD);
  tft->fillRect(234, 112, 4, 19, C_CARD);

  if ((st.scrollX -= 4) < -(int16_t)st.rtPixelW) st.scrollX = 230;
}

void NKFM_UILayout_240x135::render(Adafruit_ST7789* tft, const UIState& st, uint8_t dirty) {
  if (dirty & D_TIME) {
    tft->fillRect(6, 2, 64, 16, C_CARD);
    tft->setTextSize(2); tft->setCursor(6, 2);
    tft->setTextColor(st.synced ? C_GOOD : C_WARN);
    if (st.hour < 10) tft->print('0'); tft->print(st.hour);
    tft->print(':');
    if (st.min < 10) tft->print('0'); tft->print(st.min);
  }

  if (dirty & D_SIGNAL) {
    tft->fillRect(74, 2, 24, 16, C_CARD);
    UIGfx::drawStereoIcon(tft, 77, 3, st.stereo);

    tft->fillRect(102, 2, 24, 16, C_CARD);
    uint8_t bars = (st.rssi >= 42) ? 4 : (st.rssi >= 30) ? 3 : (st.rssi >= 18) ? 2 : (st.rssi >= 10) ? 1 : 0;
    for (uint8_t b = 0; b < 4; b++) {
      uint8_t bh = 4 + (b * 3);
      tft->fillRect(103 + (b * 6), 18 - bh, 4, bh, (b < bars) ? C_GOOD : C_DARK);
    }
  }

  if ((dirty & D_BATTERY) && st.hasBat) {
    tft->fillRect(128, 2, 48, 16, C_CARD);

    uint16_t bCol = (st.batPct > 40) ? C_GOOD : (st.batPct > 15) ? C_GOLD : C_WARN;
    // Cuerpo de pila (16x10 px) + borne positivo
    tft->drawRect(129, 5, 16, 10, C_TEXT);
    tft->fillRect(145, 7, 2, 6, C_TEXT);

    uint8_t fillW = map(constrain(st.batPct, 0, 100), 0, 100, 0, 12);
    if (fillW > 0) tft->fillRect(131, 7, fillW, 6, bCol);

    // Porcentaje al lado (ej: "85%")
    tft->setTextSize(1);
    tft->setTextColor(bCol, C_CARD);
    tft->setCursor(149, 6);
    tft->print(st.batPct);
    tft->print('%');
  }


  if (dirty & D_VOL) {
    tft->fillRect(178, 2, 58, 16, C_CARD);
    UIGfx::drawSpeakerIcon(tft, 180, 3, st.muted, st.muted ? C_WARN : C_TEXT);
    tft->setTextSize(2); tft->setCursor(204, 3);
    tft->setTextColor(st.muted ? C_WARN : C_ACCENT2);
    if (st.vol < 10) tft->print('0'); tft->print(st.vol);
  }

  if (dirty & D_FREQ) {
    tft->fillRect(6, 26, 150, 26, C_CARD);
    tft->setTextSize(3); tft->setCursor(8, 28); tft->setTextColor(C_GOLD);
    tft->print(st.freq / 100); tft->print('.');
    if ((st.freq % 100) < 10) tft->print('0');
    tft->print(st.freq % 100);

    tft->fillRect(158, 26, 76, 26, C_CARD);
    int8_t pIdx = UIGfx::findPresetIndex(st, st.freq);
    if (pIdx >= 0) {
      tft->fillRoundRect(160, 29, 36, 18, 4, C_DARK);
      tft->setTextSize(2); tft->setCursor(162, 31); tft->setTextColor(C_ACCENT);
      tft->print("P"); if (pIdx + 1 < 10) tft->print('0'); tft->print(pIdx + 1);
    }
    tft->setTextSize(2); tft->setCursor(199, 31); tft->setTextColor(C_MUTED); tft->print("MHz");

    // Dial panorámico con marcas de escala, puntos de presets y aguja robusta
    tft->fillRect(6, 53, 228, 22, C_CARD);
    tft->drawFastHLine(12, 64, 216, C_BORDER);

    for (uint16_t f = 8800; f <= 10800; f += 200) {
      int16_t x = map(f, 8750, 10800, 12, 228);
      bool major = ((f / 100) % 4 == 0);
      tft->drawFastVLine(x, major ? 59 : 61, major ? 10 : 6, major ? C_MUTED : C_BORDER);
    }

    // Puntos indicadores de las emisoras pregrabadas (presets)
    if (st.presets && st.presetCount > 0) {
      for (uint8_t i = 0; i < st.presetCount; i++) {
        RADIO_FREQ pf = (st.presets + i)->freq;
        if (pf >= 8750 && pf <= 10800) {
          int16_t px = map(pf, 8750, 10800, 12, 228);
          tft->fillCircle(px, 70, 1, C_ACCENT2);
        }
      }
    }

    // Aguja indicadora de frecuencia actual (triángulo + barra vertical de 2px)
    int16_t needleX = map(constrain(st.freq, 8750, 10800), 8750, 10800, 12, 228);
    tft->fillTriangle(needleX - 4, 54, needleX + 4, 54, needleX, 59, C_ACCENT);
    tft->fillRect(needleX - 1, 59, 2, 13, C_TEXT);
  }


  if (dirty & D_NAME) {
    tft->fillRect(6, 83, 160, 16, C_CARD);
    if (*st.name != '\0') {
      UIGfx::drawFittedText(tft, st.name, 8, 83, 156, 16, 2, C_TEXT);
    } else {
      int8_t pIdx = UIGfx::findPresetIndex(st, st.freq);
      const char* label = (pIdx >= 0 && st.presets) ? (st.presets + pIdx)->name : "Sin nombre RDS";
      UIGfx::drawFittedText(tft, label, 8, 83, 156, 16, 2, C_ACCENT2);
    }
  }

  if (dirty & D_PTY) {
    tft->fillRect(166, 83, 68, 16, C_CARD);
    if (st.bassBoost) { tft->fillRoundRect(168, 83, 22, 15, 3, C_ACCENT); tft->setTextColor(C_DARK); }
    else              { tft->drawRoundRect(168, 83, 22, 15, 3, C_BORDER); tft->setTextColor(C_MUTED); }
    tft->setTextSize(1); tft->setCursor(173, 87); tft->print("BB");

    UIGfx::drawBrushIcon(tft, 195, 83, C_ACCENT2, C_GOLD);
    tft->setTextSize(2); tft->setCursor(214, 83); tft->setTextColor(C_ACCENT2); tft->print(curThemeIdx);
  }
}
