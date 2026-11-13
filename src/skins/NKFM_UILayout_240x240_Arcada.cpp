#include "NKFM_UILayout_240x240_Arcada.h"

void NKFM_UILayout_240x240_Arcada::drawStatic(Adafruit_ST7789* tft) {
  tft->fillScreen(C_BG);
  // Dos líneas divisorias ultrafinas estilo consola portátil
  tft->drawFastHLine(8, 22, 224, C_BORDER);
  tft->drawFastHLine(8, 72, 224, C_BORDER);
  tft->drawFastHLine(8, 218, 224, C_BORDER);
}

void NKFM_UILayout_240x240_Arcada::clearRTBox(Adafruit_ST7789* tft) {
  tft->fillRect(8, 221, 224, 17, C_BG);
}

void NKFM_UILayout_240x240_Arcada::tickScroll(Adafruit_ST7789* tft, UIState& st) {
  tft->fillRect(8, 221, 224, 17, C_BG);
  tft->setTextSize(2);
  tft->setCursor(st.scrollX, 222);
  tft->setTextColor(C_MUTED);
  tft->print(st.rt);

  // Máscaras limpias en los bordes laterales
  tft->fillRect(0, 220, 8, 19, C_BG);
  tft->fillRect(232, 220, 8, 19, C_BG);

  if ((st.scrollX -= 4) < -(int16_t)st.rtPixelW) st.scrollX = 230;
}

void NKFM_UILayout_240x240_Arcada::render(Adafruit_ST7789* tft, const UIState& st, uint8_t dirty) {
  // 1. Barra superior minimalista: Hora | Señal | Volumen | Batería
  if (dirty & (D_TIME | D_SIGNAL | D_VOL | D_BATTERY)) {
    tft->fillRect(8, 3, 224, 17, C_BG);

    // Hora a la izquierda
    tft->setTextSize(2);
    tft->setCursor(8, 4);
    tft->setTextColor(st.synced ? C_GOOD : C_MUTED);
    if (st.hour < 10) tft->print('0'); tft->print(st.hour);
    tft->print(':');
    if (st.min < 10) tft->print('0'); tft->print(st.min);

    // Barras de cobertura compactas
    uint8_t bars = (st.rssi >= 42) ? 4 : (st.rssi >= 30) ? 3 : (st.rssi >= 18) ? 2 : (st.rssi >= 10) ? 1 : 0;
    for (uint8_t b = 0; b < 4; b++) {
      uint8_t bh = 4 + (b * 3);
      tft->fillRect(82 + (b * 5), 18 - bh, 3, bh, (b < bars) ? C_GOOD : C_DARK);
    }

    // Volumen
    UIGfx::drawSpeakerIcon(tft, 114, 4, st.muted, st.muted ? C_WARN : C_TEXT);
    tft->setTextSize(2);
    tft->setCursor(136, 4);
    tft->setTextColor(st.muted ? C_WARN : C_TEXT);
    if (st.vol < 10) tft->print('0'); tft->print(st.vol);

    // Batería a la derecha
    if (st.hasBat) {
      tft->setTextSize(1);
      tft->setCursor(182, 8);
      tft->setTextColor(C_GOLD);
      tft->print(st.vbat, 2); tft->print('V');
    }
  }

  // 2. Cabecera de Sintonía Activa (Frecuencia grande + RDS PS)
  if (dirty & (D_FREQ | D_NAME | D_PTY)) {
    tft->fillRect(8, 26, 224, 43, C_BG);

    // Frecuencia grande limpia
    tft->setTextSize(3);
    tft->setCursor(10, 28);
    tft->setTextColor(C_GOLD);
    tft->print(st.freq / 100); tft->print('.');
    if ((st.freq % 100) < 10) tft->print('0');
    tft->print(st.freq % 100);

    tft->setTextSize(1);
    tft->setCursor(106, 42);
    tft->setTextColor(C_MUTED);
    tft->print(F("MHz"));

    // Indicador Estéreo / Mono a la derecha
    UIGfx::drawStereoIcon(tft, 195, 32, st.stereo);
    tft->setTextSize(1);
    tft->setCursor(216, 35);
    tft->setTextColor(C_ACCENT2);
    tft->print(curThemeIdx);

    // Nombre RDS debajo de la frecuencia
    const char* rdsTitle = (*st.name != '\0') ? st.name : "FM STEREO RECEIVER";
    UIGfx::drawFittedText(tft, rdsTitle, 10, 54, 220, 14, 1, C_ACCENT2);

    // 3. Lista Vertical Estilo Arcada (6 filas + scrollbar lateral)
    tft->fillRect(8, 76, 224, 139, C_BG);

    if (st.presets && st.presetCount > 0) {
      int8_t curIdx = UIGfx::findPresetIndex(st, st.freq);
      if (curIdx < 0) curIdx = 0;

      const uint8_t visibleRows = 6;
      // Ventana deslizante centrada en el elemento activo
      int16_t startIdx = curIdx - 2;
      if (startIdx < 0) startIdx = 0;
      if (startIdx > (int16_t)st.presetCount - visibleRows) {
        startIdx = max(0, (int16_t)st.presetCount - visibleRows);
      }

      for (uint8_t r = 0; r < visibleRows; r++) {
        uint8_t idx = startIdx + r;
        if (idx >= st.presetCount) break;

        int16_t y = 77 + (r * 23);
        bool isSel = ((int8_t)idx == curIdx);

        if (isSel) {
          // Barra de selección sólida estilo Arcada (alto contraste)
          tft->fillRoundRect(8, y, 214, 20, 4, C_ACCENT);
          tft->fillTriangle(13, y + 6, 13, y + 14, 19, y + 10, C_BG);
          UIGfx::drawFittedText(tft, (st.presets + idx)->name, 24, y + 2, 142, 16, 1, C_BG);

          // Frecuencia a la derecha en texto inverso
          char fBuf[10];
          snprintf(fBuf, sizeof(fBuf), "%.1f", (st.presets + idx)->freq / 100.0f);
          tft->setTextSize(1);
          tft->setTextColor(C_BG);
          tft->setCursor(184, y + 6);
          tft->print(fBuf);
        } else {
          // Fila normal limpia sin recuadro
          UIGfx::drawFittedText(tft, (st.presets + idx)->name, 24, y + 2, 142, 16, 1, C_TEXT);

          char fBuf[10];
          snprintf(fBuf, sizeof(fBuf), "%.1f", (st.presets + idx)->freq / 100.0f);
          tft->setTextSize(1);
          tft->setTextColor(C_MUTED);
          tft->setCursor(184, y + 6);
          tft->print(fBuf);
        }
      }

      // Scrollbar lateral estilo Arcada (x=226..230, y=77..213)
      int16_t trackH = 136;
      tft->drawFastVLine(228, 77, trackH, C_DARK);
      int16_t thumbH = max(14, (trackH * visibleRows) / st.presetCount);
      int16_t thumbY = 77 + ((trackH - thumbH) * curIdx) / max(1, (int)st.presetCount - 1);
      tft->fillRoundRect(226, thumbY, 5, thumbH, 2, C_ACCENT);
    }
  }
}
