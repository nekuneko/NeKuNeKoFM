#include "NKFM_UILayout_240x240.h"

void NKFM_UILayout_240x240::drawStatic(Adafruit_ST7789* tft) {
  tft->fillScreen(C_BG);
  tft->fillRect(0, 0, 240, 26, C_CARD);
  tft->drawFastHLine(0, 26, 240, C_BORDER);

  tft->fillRoundRect(4, 32, 232, 76, 8, C_CARD);
  tft->drawRoundRect(4, 32, 232, 76, 8, C_BORDER);

  tft->fillRoundRect(4, 114, 232, 82, 8, C_CARD);
  tft->drawRoundRect(4, 114, 232, 82, 8, C_BORDER);
  tft->fillRect(10, 171, 220, 18, C_BG);

  tft->fillRoundRect(4, 202, 232, 34, 8, C_CARD);
  tft->drawRoundRect(4, 202, 232, 34, 8, C_BORDER);
}

void NKFM_UILayout_240x240::clearRTBox(Adafruit_ST7789* tft) {
  tft->fillRect(10, 171, 220, 18, C_BG);
}

void NKFM_UILayout_240x240::tickScroll(Adafruit_ST7789* tft, UIState& st) {
  tft->fillRect(10, 171, 220, 18, C_BG);
  tft->setTextSize(2);
  tft->setCursor(st.scrollX, 172);
  tft->setTextColor(C_ACCENT2);
  tft->print(st.rt);

  tft->fillRect(0, 170, 10, 20, C_BG);
  tft->fillRect(230, 170, 10, 20, C_BG);
  tft->fillRect(4, 170, 5, 20, C_CARD);
  tft->fillRect(231, 170, 5, 20, C_CARD);
  tft->drawFastVLine(4, 170, 20, C_BORDER);
  tft->drawFastVLine(235, 170, 20, C_BORDER);

  if ((st.scrollX -= 4) < -(int16_t)st.rtPixelW) st.scrollX = 230;
}

void NKFM_UILayout_240x240::render(Adafruit_ST7789* tft, const UIState& st, uint8_t dirty) {
  if (dirty & D_TIME) {
    tft->fillRect(6, 5, 64, 16, C_CARD);
    tft->setTextSize(2); tft->setCursor(6, 5);
    tft->setTextColor(st.synced ? C_GOOD : C_WARN);
    if (st.hour < 10) tft->print('0'); tft->print(st.hour);
    tft->print(':');
    if (st.min < 10) tft->print('0'); tft->print(st.min);
  }

  if (dirty & D_SIGNAL) {
    tft->fillRect(74, 4, 24, 18, C_CARD);
    UIGfx::drawStereoIcon(tft, 77, 6, st.stereo);

    tft->fillRect(102, 5, 24, 16, C_CARD);
    uint8_t bars = (st.rssi >= 42) ? 4 : (st.rssi >= 30) ? 3 : (st.rssi >= 18) ? 2 : (st.rssi >= 10) ? 1 : 0;
    for (uint8_t b = 0; b < 4; b++) {
      uint8_t bh = 4 + (b * 3);
      tft->fillRect(103 + (b * 6), 21 - bh, 4, bh, (b < bars) ? C_GOOD : C_DARK);
    }

    tft->fillRect(196, 35, 38, 39, C_CARD);
    tft->setTextSize(2);
    uint16_t rssiCol = (st.rssi >= 30) ? C_GOOD : (st.rssi >= 18) ? C_TEXT : C_WARN;
    tft->setCursor(197, 37); tft->setTextColor(C_MUTED); tft->print("R");
    tft->setTextColor(rssiCol); tft->print(st.rssi);

    uint16_t snrCol = (st.snr >= 8) ? C_GOOD : (st.snr >= 6) ? C_TEXT : C_WARN;
    tft->setCursor(197, 57); tft->setTextColor(C_MUTED); tft->print("S");
    tft->setTextColor(snrCol); tft->print(st.snr);
  }

  if ((dirty & D_BATTERY) && st.hasBat) {
    tft->fillRect(130, 4, 106, 18, C_CARD);
    uint16_t batCol = (st.batPct < 20) ? C_WARN : (st.batPct >= 50) ? C_GOOD : C_ACCENT;

    tft->setTextSize(2); tft->setCursor(132, 5); tft->setTextColor(C_TEXT); tft->print(st.vbat, 2);
    tft->setTextSize(1); tft->setCursor(181, 11); tft->setTextColor(C_MUTED); tft->print("V");

    tft->drawRoundRect(194, 6, 36, 14, 2, C_MUTED);
    tft->fillRect(230, 10, 2, 6, C_MUTED);
    uint8_t fillW = map(constrain(st.batPct, 0, 100), 0, 100, 0, 32);
    if (fillW > 0) tft->fillRect(196, 8, fillW, 10, batCol);

    tft->setCursor(st.batPct == 100 ? 199 : 203, 9);
    tft->setTextColor(st.batPct > 45 ? C_BG : C_TEXT);
    tft->print(st.batPct); tft->print("%");
  }

  if (dirty & D_FREQ) {
    tft->fillRect(8, 36, 143, 36, C_CARD);
    tft->setTextSize(4); tft->setCursor(8, 40); tft->setTextColor(C_GOLD);
    tft->print(st.freq / 100); tft->print('.');
    if ((st.freq % 100) < 10) tft->print('0');
    tft->print(st.freq % 100);

    tft->fillRect(152, 35, 43, 39, C_CARD);
    tft->setTextSize(2);
    int8_t pIdx = UIGfx::findPresetIndex(st, st.freq);
    if (pIdx >= 0) {
      tft->fillRoundRect(153, 35, 41, 18, 4, C_DARK);
      tft->setCursor(156, 37); tft->setTextColor(C_ACCENT);
      tft->print("P"); if (pIdx + 1 < 10) tft->print('0'); tft->print(pIdx + 1);
    }
    tft->setCursor(156, 57); tft->setTextColor(C_MUTED); tft->print("MHz");

    tft->fillRect(10, 74, 220, 30, C_CARD);
    tft->drawFastHLine(14, 88, 212, C_BORDER);
    for (uint16_t f = 8800; f <= 10800; f += 200) {
      int16_t x = map(f, 8750, 10800, 14, 226);
      bool major = ((f / 100) % 4 == 0);
      tft->drawFastVLine(x, major ? 82 : 85, major ? 12 : 6, major ? C_MUTED : C_BORDER);
      if (major) {
        tft->setTextSize(1); tft->setCursor(x - (f >= 10000 ? 8 : 5), 96);
        tft->setTextColor(C_MUTED); tft->print(f / 100);
      }
    }
    if (st.presets) {
      for (uint8_t i = 0; i < st.presetCount; i++) {
        int16_t px = map(constrain((st.presets + i)->freq, 8750, 10800), 8750, 10800, 14, 226);
        tft->fillCircle(px, 80, 1, C_ACCENT);
      }
    }
    int16_t needleX = map(constrain(st.freq, 8750, 10800), 8750, 10800, 14, 226);
    tft->fillTriangle(needleX - 4, 75, needleX + 4, 75, needleX, 81, C_ACCENT);
    tft->drawFastVLine(needleX, 79, 15, C_TEXT);
  }

  if (dirty & D_NAME) {
    tft->fillRect(10, 120, 220, 24, C_CARD);
    if (*st.name != '\0') {
      UIGfx::drawFittedText(tft, st.name, 12, 120, 216, 24, 3, C_TEXT);
    } else {
      int8_t pIdx = UIGfx::findPresetIndex(st, st.freq);
      const char* label = (pIdx >= 0 && st.presets) ? (st.presets + pIdx)->name : "Sin nombre RDS";
      UIGfx::drawFittedText(tft, label, 12, 120, 216, 24, 3, C_ACCENT2);
    }
  }

  if (dirty & D_PTY) {
    tft->fillRect(10, 148, 220, 20, C_CARD);
    UIGfx::drawMusicNoteIcon(tft, 12, 150, C_ACCENT);
    UIGfx::drawFittedText(tft, NKFM_RDSParser::getPTYName(st.pty), 32, 148, 136, 20, 2, C_MUTED);

    if (st.bassBoost) { tft->fillRoundRect(170, 150, 22, 16, 3, C_ACCENT); tft->setTextColor(C_DARK); }
    else              { tft->drawRoundRect(170, 150, 22, 16, 3, C_BORDER); tft->setTextColor(C_MUTED); }
    tft->setTextSize(1); tft->setCursor(175, 154); tft->print("BB");

    UIGfx::drawBrushIcon(tft, 198, 151, C_ACCENT2, C_GOLD);
    tft->setTextSize(2); tft->setCursor(216, 151); tft->setTextColor(C_ACCENT2); tft->print(curThemeIdx);
  }

  if (dirty & D_VOL) {
    tft->fillRect(10, 208, 220, 22, C_CARD);
    UIGfx::drawSpeakerIcon(tft, 12, 212, st.muted, st.muted ? C_WARN : C_TEXT);
    uint16_t barColor = st.muted ? C_WARN : C_ACCENT2;
    for (uint8_t s = 0; s < 15; s++) {
      tft->fillRoundRect(36 + (s * 10), 211, 7, 16, 2, (s < st.vol) ? barColor : C_DARK);
    }
    tft->setTextSize(2); tft->setCursor(192, 212); tft->setTextColor(st.muted ? C_WARN : C_TEXT);
    if (st.vol < 10) tft->print('0'); tft->print(st.vol);
  }
}
