#pragma once
#include <Arduino.h>
#include "NeKuNeKoFM.h"

constexpr uint8_t NKFM_SI4705_ADDR      = 0x63;
constexpr uint8_t NKFM_MAX17048_ADDR    = 0x36;
constexpr bool    NKFM_HAS_BATTERY      = true;
constexpr bool    NKFM_HAS_BOARD_LEDS   = true;
constexpr bool    NKFM_USE_EXT_ANTENNA  = false; // Antena por cable auriculares (FMI)

constexpr int NKFM_PIN_RADIO_RST    = FSC_IRQ;
constexpr int NKFM_PIN_IR_IRQ       = FSP_IRQ;
constexpr int NKFM_PIN_INT_GAUGE    = FSC_RST;
constexpr int NKFM_PIN_INT_IMU      = 5;
constexpr int NKFM_PIN_TFT_CS       = A9;
constexpr int NKFM_PIN_TFT_DC       = A0;
constexpr int NKFM_PIN_TFT_RST      = -1;
constexpr int NKFM_PIN_TFT_BL       = DAC0;
constexpr bool NKFM_TFT_BL_ACT_LOW  = true;

constexpr uint8_t NKFM_DEFAULT_VOLUME   = 15;
constexpr uint8_t NKFM_DEFAULT_ROTATION = 3;

const NKFM_ButtonDef NKFM_BTNS[] = {
  // Pin  | Etiqueta   | Acción Corta                | Acción Larga
  { 4,    "D4 [ROT]",   NKFM_Action::ROTATION_NEXT, NKFM_Action::MONO_TOGGLE    },
  { 10,   "D10 [NEXT]", NKFM_Action::PRESET_NEXT,   NKFM_Action::SEEK_UP        },
  { 12,   "D12 [PREV]", NKFM_Action::PRESET_PREV,   NKFM_Action::SEEK_DOWN      },
  { A7,   "A7 [VOL+]",  NKFM_Action::VOL_UP,        NKFM_Action::FREQ_UP        },
  { A6,   "A6 [VOL-]",  NKFM_Action::VOL_DOWN,      NKFM_Action::FREQ_DOWN      },
  { 3,    "D3 [INFO]",  NKFM_Action::THEME_NEXT,    NKFM_Action::BASS_TOGGLE    },
  { 2,    "D2 [MUTE]",  NKFM_Action::MUTE_TOGGLE,   NKFM_Action::DISPLAY_TOGGLE }
};
constexpr uint8_t NKFM_BTN_COUNT = sizeof(NKFM_BTNS) / sizeof(*NKFM_BTNS);
