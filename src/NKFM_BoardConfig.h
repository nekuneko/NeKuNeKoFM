#pragma once
#include "NeKuNeKoFM.h"

// Si el usuario tiene un archivo "UserBoardConfig.h" en la carpeta de su sketch, se carga completo
#if defined(__has_include)
  #if __has_include("UserBoardConfig.h")
    #include "UserBoardConfig.h"
    #define NKFM_USER_CONFIG_LOADED
  #endif
#endif

#ifndef NKFM_USER_CONFIG_LOADED

  // ==========================================================================
  // 1. Preferencias y Emisoras Comunes (Sobrescribibles vía UserBoardConfig.h)
  // ==========================================================================
  constexpr uint8_t NKFM_DEFAULT_PRESET  = 0;
  constexpr uint8_t NKFM_DEFAULT_THEME   = 0;
  constexpr int8_t  NKFM_TIMEZONE_OFFSET = 1;

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

  // ==========================================================================
  // 2. Perfil de Hardware según Arquitectura / Placa
  // ==========================================================================
  #if defined(ARDUINO_ARCH_SAMD)
    #include "NKFM_Board_KittenDisplay.h"
  #elif defined(ESP32)
    #include "NKFM_Board_TTGO_ESP32.h"
  #endif

#endif
