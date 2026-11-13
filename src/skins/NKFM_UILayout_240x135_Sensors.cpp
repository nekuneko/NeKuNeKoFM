#include "NKFM_UILayout_240x135_Sensors.h"
#include <math.h>

void NKFM_UILayout_240x135_Sensors::drawStatic(Adafruit_ST7789* tft) {
  if (!tft) return;
  tft->fillScreen(C_BG);
  tft->drawRoundRect(1, 1, 238, 133, 5, C_BORDER);
  tft->drawFastHLine(4, 20, 232, C_BORDER);
  tft->drawFastHLine(4, 98, 232, C_BORDER);
}

void NKFM_UILayout_240x135_Sensors::render(Adafruit_ST7789* tft, const UIState& st, uint8_t dirty) {
  if (!tft || dirty == 0) return;

  if (dirty & D_FULL) {
    drawStatic(tft);
  }

  // 1. Cabecera compacta: Título + Frecuencia FM en segundo plano
  if (dirty & (D_FULL | D_FREQ | D_TIME)) {
    tft->fillRect(5, 3, 230, 15, C_BG);
    tft->setTextSize(1);
    tft->setTextColor(C_ACCENT, C_BG);
    tft->setCursor(8, 7);
    tft->print(F("ESTACION AMBIENTAL"));

    char freqBuf[20];
    snprintf(freqBuf, sizeof(freqBuf), "FM %.2f", st.freq / 100.0f);
    tft->setTextColor(C_GOLD, C_BG);
    tft->setCursor(180, 7);
    tft->print(freqBuf);
  }

  // 2. Bloque Principal Protagonista: BME280 (x=4..236, y=23..95)
  if (dirty & (D_FULL | D_ALL)) {
    tft->fillRoundRect(4, 23, 232, 72, 4, C_CARD);
    tft->drawRoundRect(4, 23, 232, 72, 4, C_BORDER);

    tft->setTextSize(1);
    tft->setTextColor(C_ACCENT2, C_CARD);
    tft->setCursor(10, 27);
    tft->print(F("BME280 PRINCIPAL"));

    if (_env.hasBME) {
      char buf[24];

      // Temperatura gigante en tamaño 4 (izquierda)
      tft->setTextSize(4);
      tft->setTextColor(C_TEXT, C_CARD);
      tft->setCursor(10, 42);
      snprintf(buf, sizeof(buf), "%.1f", _env.bmeTemp);
      tft->print(buf);
      tft->setTextSize(2);
      tft->print(F("C"));

      // Humedad y Presión en tamaño 2 (derecha)
      tft->setTextSize(2);
      tft->setTextColor(C_GOOD, C_CARD);
      tft->setCursor(134, 38);
      snprintf(buf, sizeof(buf), "%.0f%% HR", _env.bmeHum);
      tft->print(buf);

      tft->setTextColor(C_TEXT, C_CARD);
      tft->setCursor(134, 64);
      snprintf(buf, sizeof(buf), "%.0f hPa", _env.bmePres);
      tft->print(buf);
    } else {
      tft->setTextSize(2);
      tft->setTextColor(C_MUTED, C_CARD);
      tft->setCursor(35, 50);
      tft->print(F("BME280 N/D"));
    }

    // 3. Franja Inferior: Si7021 compacto + Punto de Rocío y Altitud
    tft->fillRect(5, 101, 230, 30, C_BG);
    tft->setTextSize(1);

    // Línea 1 inferior: Sensor secundario Si7021
    tft->setTextColor(C_ACCENT2, C_BG);
    tft->setCursor(8, 104);
    tft->print(F("Si7021:"));
    tft->setTextColor(C_TEXT, C_BG);
    if (_env.hasSi7021) {
      char siBuf[24];
      snprintf(siBuf, sizeof(siBuf), " %.1f C (HR %.0f%%)", _env.siTemp, _env.siHum);
      tft->print(siBuf);
    } else {
      tft->print(F(" N/D"));
    }

    // Línea 2 inferior: Punto de Rocío y Altitud estimada (solo si hay BME280)
    if (_env.hasBME) {
      float hSafe = (_env.bmeHum < 1.0f) ? 1.0f : _env.bmeHum;
      float gamma = logf(hSafe / 100.0f) + (17.62f * _env.bmeTemp) / (243.12f + _env.bmeTemp);
      float dewPoint = (243.12f * gamma) / (17.62f - gamma);
      float altitude = 44330.0f * (1.0f - powf(_env.bmePres / 1013.25f, 0.1903f));

      char calcBuf[38];
      snprintf(calcBuf, sizeof(calcBuf), "Rocio: %.1fC  |  Alt: %.0fm", dewPoint, altitude);
      tft->setTextColor(C_MUTED, C_BG);
      tft->setCursor(8, 118);
      tft->print(calcBuf);
    }
  }
}
