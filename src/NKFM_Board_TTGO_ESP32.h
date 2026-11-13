#pragma once
#include <Arduino.h>
#include "NeKuNeKoFM.h"

constexpr uint8_t NKFM_SI4705_ADDR      = 0x63;
constexpr uint8_t NKFM_MAX17048_ADDR    = 0x36;
constexpr uint8_t NKFM_SX1509_ADDR      = 0x3E;
constexpr uint8_t NKFM_BME280_ADDR      = 0x77;
constexpr uint8_t NKFM_SI7021_ADDR      = 0x40;


constexpr bool    NKFM_HAS_BATTERY      = true;
constexpr bool    NKFM_HAS_BOARD_LEDS   = true;
constexpr bool    NKFM_USE_EXT_ANTENNA  = false; // FMI (0x1107 = 0x0000): entrada FM para antena y auriculares

constexpr int NKFM_PIN_I2C_SDA      = 21;
constexpr int NKFM_PIN_I2C_SCL      = 22;

constexpr int NKFM_PIN_TFT_MOSI     = 23;
constexpr int NKFM_PIN_TFT_SCK      = 18;
constexpr int NKFM_PIN_TFT_CS       = 5;
constexpr int NKFM_PIN_TFT_DC       = 15;
constexpr int NKFM_PIN_TFT_RST      = -1;
constexpr int NKFM_PIN_TFT_BL       = 9;
constexpr bool NKFM_TFT_BL_ACT_LOW  = false;

constexpr int NKFM_PIN_RADIO_RST    = 25;
constexpr int NKFM_PIN_EN_SPEAKER   = 26;
constexpr int NKFM_PIN_EN_HP        = 14;

constexpr int NKFM_PIN_IR_RX        = 39;
constexpr int NKFM_PIN_IR_TX        = 27;
constexpr int NKFM_PIN_BAT_ADC      = 34;
constexpr int NKFM_PIN_INT_RTC      = 35;
constexpr int NKFM_PIN_INT_SX1509   = 10;

constexpr int NKFM_PIN_LED_BT       = 4;
constexpr int NKFM_PIN_LED_BUILTIN  = 2;

// Compensación del autocalentamiento de la PCB (ESP32 + TFT + Amplificador)
constexpr float NKFM_BME280_TEMP_OFFSET = -12.5f;
constexpr float NKFM_SI7021_TEMP_OFFSET = -14.5f;

constexpr uint8_t NKFM_DEFAULT_VOLUME   = 4;
constexpr uint8_t NKFM_DEFAULT_ROTATION = 1;

const NKFM_ButtonDef NKFM_BTNS[] = {
  // Canal | Etiqueta       | Acción Corta                | Acción Larga
  {  0,      "IO0 [B]",       NKFM_Action::PRESET_NEXT,    NKFM_Action::SEEK_UP        },
  {  1,      "IO1 [A]",       NKFM_Action::PRESET_PREV,    NKFM_Action::SEEK_DOWN      },
  {  2,      "IO2 [PROG-]",   NKFM_Action::VOL_UP,         NKFM_Action::MONO_TOGGLE    },
  {  3,      "IO3 [MENU]",    NKFM_Action::VOL_DOWN,       NKFM_Action::BASS_TOGGLE    },
  {  4,      "IO4 [PROG+]",   NKFM_Action::MUTE_TOGGLE,    NKFM_Action::DISPLAY_TOGGLE },
  {  5,      "IO5 [VOL+]",    NKFM_Action::FREQ_UP,        NKFM_Action::THEME_NEXT     },
  {  6,      "IO6 [MUTE]",    NKFM_Action::VIEW_TOGGLE,    NKFM_Action::ROTATION_NEXT  },
  {  7,      "IO7 [VOL-]",    NKFM_Action::FREQ_DOWN,      NKFM_Action::THEME_PREV     },
  {  8,      "IO8 [CH1]",     NKFM_Action::PRESET_1,       NKFM_Action::NONE           },
  {  9,      "IO9 [CH2]",     NKFM_Action::PRESET_2,       NKFM_Action::NONE           },
  { 10,      "IO10 [CH3]",    NKFM_Action::PRESET_3,       NKFM_Action::NONE           },
  { 11,      "IO11 [CH4]",    NKFM_Action::PRESET_4,       NKFM_Action::NONE           },
  { 12,      "IO12 [CH5]",    NKFM_Action::PRESET_5,       NKFM_Action::NONE           },
  { 13,      "IO13 [CH6]",    NKFM_Action::PRESET_6,       NKFM_Action::NONE           },
  { 14,      "IO14 [CH7]",    NKFM_Action::PRESET_7,       NKFM_Action::NONE           },
  { 15,      "IO15 [CH8]",    NKFM_Action::PRESET_8,       NKFM_Action::NONE           }
};
constexpr uint8_t NKFM_BTN_COUNT = sizeof(NKFM_BTNS) / sizeof(*NKFM_BTNS);

