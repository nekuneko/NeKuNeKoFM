#pragma once
#include "NeKuNeKoFM.h"

#if defined(__has_include)
  #if __has_include("UserBoardConfig.h")
    #include "UserBoardConfig.h"
    #define NKFM_USER_CONFIG_LOADED
  #endif
#endif

#ifndef NKFM_USER_CONFIG_LOADED

  #if defined(ARDUINO_ARCH_SAMD)
    constexpr uint8_t NKFM_SI4705_ADDR      = 0x63;
    constexpr uint8_t NKFM_MAX17048_ADDR    = 0x36;
    constexpr bool    NKFM_HAS_BATTERY      = true;
    constexpr bool    NKFM_HAS_BOARD_LEDS   = true;

    constexpr int8_t  NKFM_PIN_RADIO_RST    = FSC_IRQ;
    constexpr int8_t  NKFM_PIN_IR_IRQ       = FSP_IRQ;
    constexpr int8_t  NKFM_PIN_INT_GAUGE    = FSC_RST;
    constexpr int8_t  NKFM_PIN_INT_IMU      = 5;
    constexpr int8_t  NKFM_PIN_TFT_CS       = A9;
    constexpr int8_t  NKFM_PIN_TFT_DC       = A0;
    constexpr int8_t  NKFM_PIN_TFT_RST      = -1;
    constexpr int8_t  NKFM_PIN_TFT_BL       = DAC0;
    constexpr bool    NKFM_TFT_BL_ACT_LOW   = true;

    constexpr int NKFM_BTN_ROT  = 4;
    constexpr int NKFM_BTN_NEXT = 10;
    constexpr int NKFM_BTN_PREV = 12;
    constexpr int NKFM_BTN_VUP  = A7;
    constexpr int NKFM_BTN_VDN  = A6;
    constexpr int NKFM_BTN_INFO = 3;
    constexpr int NKFM_BTN_MUTE = 2;

    const int NKFM_BTNS[] = {
      NKFM_BTN_ROT, NKFM_BTN_NEXT, NKFM_BTN_PREV,
      NKFM_BTN_VUP, NKFM_BTN_VDN, NKFM_BTN_INFO, NKFM_BTN_MUTE
    };
    const char* const NKFM_BTN_NAMES[] = {
      "D4 (Rotar/Mono)", "D10 (Next/Seek+)", "D12 (Prev/Seek-)",
      "A7 (Vol+/Freq+)", "A6 (Vol-/Freq-)", "D3 (Tema/BB)", "D2 (Mute/Disp)"
    };
    constexpr uint8_t NKFM_BTN_COUNT = sizeof(NKFM_BTNS) / sizeof(*NKFM_BTNS);

  #elif defined(ESP32)
    constexpr uint8_t NKFM_SI4705_ADDR      = 0x63;
    constexpr uint8_t NKFM_MAX17048_ADDR    = 0x36;
    constexpr bool    NKFM_HAS_BATTERY      = false;
    constexpr bool    NKFM_HAS_BOARD_LEDS   = false;

    constexpr int8_t  NKFM_PIN_RADIO_RST    = 25;
    constexpr int8_t  NKFM_PIN_IR_IRQ       = 26;
    constexpr int8_t  NKFM_PIN_TFT_CS       = 5;
    constexpr int8_t  NKFM_PIN_TFT_DC       = 16;
    constexpr int8_t  NKFM_PIN_TFT_RST      = 17;
    constexpr int8_t  NKFM_PIN_TFT_BL       = 4;
    constexpr bool    NKFM_TFT_BL_ACT_LOW   = false;

    const int NKFM_BTNS[] = {};
    const char* const NKFM_BTN_NAMES[] = {};
    constexpr uint8_t NKFM_BTN_COUNT = 0;
  #endif

  constexpr uint8_t NKFM_DEFAULT_PRESET   = 0;
  constexpr uint8_t NKFM_DEFAULT_VOLUME   = 15;
  constexpr uint8_t NKFM_DEFAULT_THEME    = 0;
  constexpr uint8_t NKFM_DEFAULT_ROTATION = 3;
  constexpr int8_t  NKFM_TIMEZONE_OFFSET  = 1;

  const NKFM_Preset NKFM_DEFAULT_PRESETS[] = {
    {  9740, "Canal Sur Cadiz" }, {  9780, "Los 40 Jerez"    }, {  9850, "Gozadera"        },
    {  9900, "Canal Sur Jerez" }, {  9940, "RAI Andalucia"   }, {  9990, "Cadena Dial"     },
    { 10140, "Onda Cero"       }, { 10200, "Cadena COPE"     }, { 10230, "NeKu FM"         },
    { 10350, "RNE 1"           }, { 10410, "Radiole"         }, { 10440, "Radio Maria"     },
    { 10480, "Canal Fiesta"    }, { 10630, "RNE 5-CA"        }, { 10740, "Radio La Isla"   },
    { 10770, "Radio Chiclana"  }, { 10780, "Rad. Puerto Real"}, {  8770, "Hit FM"          },
    {  8810, "Europa FM"       }, {  8910, "Woman FM"        }, {  8940, "Los 40"          },
    {  8980, "Los 40 CL"       }, {  9030, "Onda Cero 2"     }, {  9080, "Cadena SER"      },
    {  9170, "Baby Radio"      }, {  9240, "Cadena 100"      }, {  9320, "Cadena SER+"     },
    {  9360, "AWradio"         }, {  9380, "BOOM FM"         }, {  9450, "Radio Clasica"   },
    {  9670, "Radio 3"         }
  };
  constexpr uint8_t NKFM_DEFAULT_PRESET_COUNT = sizeof(NKFM_DEFAULT_PRESETS) / sizeof(*NKFM_DEFAULT_PRESETS);

#endif
