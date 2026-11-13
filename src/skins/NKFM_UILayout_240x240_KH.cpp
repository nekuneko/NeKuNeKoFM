#include "NKFM_UILayout_240x240_KH.h"
#include <math.h>

// Paleta temática estilo Kingdom Hearts (Azul Cobalto, Cian Eléctrico y Dorado)
static constexpr uint16_t KH_BG        = 0x0007; // Azul medianoche profundo
static constexpr uint16_t KH_PANEL     = 0x010C; // Azul cobalto oscuro
static constexpr uint16_t KH_SELECT    = 0x02BF; // Azul brillante selección
static constexpr uint16_t KH_CYAN      = 0x07FF; // Borde cian
static constexpr uint16_t KH_GOLD      = 0xFFE0; // Corona y acentos dorados
static constexpr uint16_t KH_HP_GREEN  = 0x07E8; // Arco señal (VIT)
static constexpr uint16_t KH_MP_BLUE   = 0x3E7F; // Arco volumen (PM)
static constexpr uint16_t KH_DARK_ARC  = 0x10A4; // Fondo de arco vacío

void NKFM_UILayout_240x240_KH::drawCrownIcon(Adafruit_ST7789* tft, int16_t x, int16_t y, uint16_t col) {
  tft->fillTriangle(x,     y + 8, x,     y + 2, x + 3, y + 6, col);
  tft->fillTriangle(x + 2, y + 8, x + 5, y,     x + 8, y + 8, col);
  tft->fillTriangle(x + 7, y + 6, x + 10,y + 2, x + 10,y + 8, col);
  tft->fillRect(x, y + 8, 11, 2, col);
}

void NKFM_UILayout_240x240_KH::drawStar4(Adafruit_ST7789* tft, int16_t cx, int16_t cy, int16_t r, uint16_t col) {
  tft->fillTriangle(cx, cy - r, cx - 2, cy, cx + 2, cy, col);
  tft->fillTriangle(cx, cy + r, cx - 2, cy, cx + 2, cy, col);
  tft->fillTriangle(cx - r, cy, cx, cy - 2, cx, cy + 2, col);
  tft->fillTriangle(cx + r, cy, cx, cy - 2, cx, cy + 2, col);
}

void NKFM_UILayout_240x240_KH::drawArcBar(Adafruit_ST7789* tft, int16_t cx, int16_t cy,
                                          int16_t rIn, int16_t rOut, uint8_t pct,
                                          uint16_t fillCol, uint16_t emptyCol) {
  // Arco de 180° (desde -180° izquierda hasta 0° arriba-derecha en 24 segmentos)
  const uint8_t segs = 24;
  uint8_t activeSegs = (uint8_t)((pct * segs) / 100);

  for (uint8_t i = 0; i < segs; i++) {
    float a1 = (-180.0f + (i * 180.0f / segs)) * 0.0174533f;
    float a2 = (-180.0f + ((i + 1) * 180.0f / segs) - 1.5f) * 0.0174533f;

    int16_t x1i = cx + (int16_t)(cosf(a1) * rIn);
    int16_t y1i = cy + (int16_t)(sinf(a1) * rIn);
    int16_t x1o = cx + (int16_t)(cosf(a1) * rOut);
    int16_t y1o = cy + (int16_t)(sinf(a1) * rOut);

    int16_t x2i = cx + (int16_t)(cosf(a2) * rIn);
    int16_t y2i = cy + (int16_t)(sinf(a2) * rIn);
    int16_t x2o = cx + (int16_t)(cosf(a2) * rOut);
    int16_t y2o = cy + (int16_t)(sinf(a2) * rOut);

    uint16_t c = (i < activeSegs) ? fillCol : emptyCol;
    tft->fillTriangle(x1i, y1i, x1o, y1o, x2o, y2o, c);
    tft->fillTriangle(x1i, y1i, x2o, y2o, x2i, y2i, c);
  }
}

void NKFM_UILayout_240x240_KH::drawStatic(Adafruit_ST7789* tft) {
  tft->fillScreen(KH_BG);

  // Cartucho superior estilo "World Banner"
  tft->fillRoundRect(4, 24, 232, 62, 6, KH_PANEL);
  tft->drawRoundRect(4, 24, 232, 62, 6, KH_CYAN);
  drawStar4(tft, 14, 34, 5, KH_GOLD);
  drawStar4(tft, 225, 34, 5, KH_GOLD);

  // Línea separadora RadioText
  tft->drawFastHLine(10, 88, 220, KH_CYAN);

  // Cabecera inclinada del Command Menu (abajo izquierda)
  tft->fillRect(4, 114, 134, 16, KH_SELECT);
  tft->drawRect(4, 114, 134, 16, KH_CYAN);
  tft->setTextSize(1);
  tft->setTextColor(KH_GOLD);
  tft->setCursor(10, 118);
  tft->print(F("COMMAND // FM"));
}

void NKFM_UILayout_240x240_KH::clearRTBox(Adafruit_ST7789* tft) {
  tft->fillRect(4, 91, 232, 18, KH_BG);
}

void NKFM_UILayout_240x240_KH::tickScroll(Adafruit_ST7789* tft, UIState& st) {
  tft->fillRect(4, 91, 232, 18, KH_BG);
  tft->setTextSize(2);
  tft->setCursor(st.scrollX, 92);
  tft->setTextColor(KH_CYAN);
  tft->print(st.rt);
  if ((st.scrollX -= 4) < -(int16_t)st.rtPixelW) st.scrollX = 230;
}

void NKFM_UILayout_240x240_KH::render(Adafruit_ST7789* tft, const UIState& st, uint8_t dirty) {
  // 1. Barra superior: Reloj + Batería
  if (dirty & (D_TIME | D_BATTERY)) {
    tft->fillRect(0, 0, 240, 20, KH_BG);
    tft->setTextSize(2);
    tft->setCursor(8, 3);
    tft->setTextColor(st.synced ? KH_HP_GREEN : KH_GOLD);
    if (st.hour < 10) tft->print('0'); tft->print(st.hour);
    tft->print(':');
    if (st.min < 10) tft->print('0'); tft->print(st.min);

    if (st.hasBat) {
      tft->setTextSize(1);
      tft->setTextColor(KH_CYAN);
      tft->setCursor(168, 7);
      tft->print(st.vbat, 2); tft->print("V ");
      tft->print(st.batPct); tft->print('%');
    }
  }

  // 2. Cartucho de Frecuencia + Nombre RDS + Menú de Comandos (Presets navegables)
  if (dirty & (D_FREQ | D_NAME | D_PTY)) {
    // Frecuencia gigante
    tft->fillRect(24, 28, 192, 32, KH_PANEL);
    tft->setTextSize(4);
    tft->setTextColor(KH_GOLD);
    tft->setCursor(32, 30);
    tft->print(st.freq / 100); tft->print('.');
    if ((st.freq % 100) < 10) tft->print('0');
    tft->print(st.freq % 100);

    tft->setTextSize(2);
    tft->setTextColor(KH_CYAN);
    tft->setCursor(184, 42);
    tft->print(F("MHz"));

    // Nombre RDS o Preset
    tft->fillRect(10, 63, 220, 18, KH_PANEL);
    int8_t pIdx = UIGfx::findPresetIndex(st, st.freq);
    const char* title = (*st.name != '\0') ? st.name
                      : (pIdx >= 0 && st.presets) ? (st.presets + pIdx)->name
                      : "SINTONIA LIBRE";
    UIGfx::drawFittedText(tft, title, 12, 64, 216, 16, 2, 0xFFFF);

    // Command Menu (4 filas de presets alrededor del actual)
    tft->fillRect(4, 132, 146, 104, KH_BG);
    if (st.presets && st.presetCount > 0) {
      int8_t baseIdx = (pIdx >= 0) ? pIdx : 0;
      for (uint8_t row = 0; row < 4; row++) {
        uint8_t idx = (baseIdx + row) % st.presetCount;
        int16_t ry = 133 + (row * 25);
        bool selected = (row == 0);

        int16_t rx = selected ? 10 : 4;
        uint16_t bgCol = selected ? KH_SELECT : KH_PANEL;
        uint16_t bdCol = selected ? KH_GOLD   : KH_CYAN;

        tft->fillRoundRect(rx, ry, 134, 22, 4, bgCol);
        tft->drawRoundRect(rx, ry, 134, 22, 4, bdCol);

        if (selected) {
          drawCrownIcon(tft, rx + 4, ry + 6, KH_GOLD);
          UIGfx::drawFittedText(tft, (st.presets + idx)->name, rx + 19, ry + 3, 110, 16, 1, 0xFFFF);
        } else {
          UIGfx::drawFittedText(tft, (st.presets + idx)->name, rx + 8, ry + 3, 120, 16, 1, KH_CYAN);
        }
      }
    }
  }

  // 3. Anillo Curvo VIT/PM (Señal + Volumen) en esquina inferior derecha
  if (dirty & (D_SIGNAL | D_VOL)) {
    tft->fillRect(152, 122, 86, 114, KH_BG);

    int16_t cx = 228;
    int16_t cy = 224;

    // Arco exterior: Señal RSSI (0..60 -> 0..100%)
    uint8_t sigPct = constrain(map((int)st.rssi, 0, 55, 0, 100), 0, 100);
    drawArcBar(tft, cx, cy, 56, 68, sigPct, KH_HP_GREEN, KH_DARK_ARC);

    // Arco interior: Volumen (0..15 -> 0..100%)
    uint8_t volPct = st.muted ? 0 : (uint8_t)((st.vol * 100) / 15);
    drawArcBar(tft, cx, cy, 40, 52, volPct, st.muted ? 0xF800 : KH_MP_BLUE, KH_DARK_ARC);

    // Centro del anillo: Número de volumen + icono Estéreo/Mute
    UIGfx::drawStereoIcon(tft, 198, 192, st.stereo);
    tft->setTextSize(2);
    tft->setTextColor(st.muted ? 0xF800 : KH_GOLD);
    tft->setCursor(198, 210);
    if (st.muted) {
      tft->print(F("MT"));
    } else {
      if (st.vol < 10) tft->print('0');
      tft->print(st.vol);
    }
  }
}
