#pragma once
#include <Arduino.h>
#include "NeKuNeKoFM.h"
#include "NKFM_ActionDispatcher.h"

enum class NKFM_Key : uint8_t {
  NONE = 0,
  POWER, EJECT,
  NUM_0, NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9,
  UP, DOWN, LEFT, RIGHT,
  OK, MENU, SELECT, CLEAR,
  PREVIOUS, NEXT, REWIND, FAST_FORWARD, PAUSE, STOP,
  BOOKMARK, SCENE, SUBTITLES, MUSIC,
  KEY_UNKNOWN
};

class NKFM_IRRemote {
  public:
    virtual ~NKFM_IRRemote() {}
    virtual void begin() = 0;
    virtual void flush() = 0;
    virtual NKFM_Key poll(uint32_t *rawOut = nullptr) = 0;

    // Mapa estándar de acciones del mando IR usando NKFM_ActionDispatcher.
    // Devuelve true si la tecla tiene acción asignada, o false si está libre para el .ino.
    static bool dispatchDefault(NKFM_Key key, NKFM_Receiver& radio, NKFM_EventCallback onUIEvent = nullptr) {
      NKFM_Action act = NKFM_Action::NONE;

      switch (key) {
        // --- 1. ASIGNADAS: Selección directa de Presets (1..10) ---
        case NKFM_Key::NUM_1:        act = NKFM_Action::PRESET_1;       break;
        case NKFM_Key::NUM_2:        act = NKFM_Action::PRESET_2;       break;
        case NKFM_Key::NUM_3:        act = NKFM_Action::PRESET_3;       break;
        case NKFM_Key::NUM_4:        act = NKFM_Action::PRESET_4;       break;
        case NKFM_Key::NUM_5:        act = NKFM_Action::PRESET_5;       break;
        case NKFM_Key::NUM_6:        act = NKFM_Action::PRESET_6;       break;
        case NKFM_Key::NUM_7:        act = NKFM_Action::PRESET_7;       break;
        case NKFM_Key::NUM_8:        act = NKFM_Action::PRESET_8;       break;
        case NKFM_Key::NUM_9:        act = NKFM_Action::PRESET_9;       break;
        case NKFM_Key::NUM_0:        act = NKFM_Action::PRESET_10;      break;

        // --- 2. ASIGNADAS: Control de Radio y Audio ---
        case NKFM_Key::UP:           act = NKFM_Action::VOL_UP;         break;
        case NKFM_Key::DOWN:         act = NKFM_Action::VOL_DOWN;       break;
        case NKFM_Key::LEFT:         act = NKFM_Action::PRESET_PREV;    break;
        case NKFM_Key::RIGHT:        act = NKFM_Action::PRESET_NEXT;    break;
        case NKFM_Key::REWIND:       act = NKFM_Action::FREQ_DOWN;      break;
        case NKFM_Key::FAST_FORWARD: act = NKFM_Action::FREQ_UP;        break;
        case NKFM_Key::PREVIOUS:     act = NKFM_Action::SEEK_DOWN;      break;
        case NKFM_Key::NEXT:         act = NKFM_Action::SEEK_UP;        break;
        case NKFM_Key::OK:
        case NKFM_Key::PAUSE:        act = NKFM_Action::MUTE_TOGGLE;    break;

        // --- 3. ASIGNADAS: Control de Pantalla (vía eventos UI) ---
        case NKFM_Key::POWER:        act = NKFM_Action::DISPLAY_TOGGLE; break;
        case NKFM_Key::EJECT:        act = NKFM_Action::VIEW_TOGGLE;    break;
        case NKFM_Key::SUBTITLES:    act = NKFM_Action::THEME_NEXT;     break;
        case NKFM_Key::SCENE:        act = NKFM_Action::THEME_PREV;     break;
        case NKFM_Key::SELECT:       act = NKFM_Action::ROTATION_NEXT;  break;
        case NKFM_Key::STOP:         act = NKFM_Action::SKIN_NEXT;      break;

        // --- 4. SIN ASIGNAR (Libres para personalizar en el .ino) ---
        case NKFM_Key::MENU:
        case NKFM_Key::CLEAR:
        case NKFM_Key::BOOKMARK:
        case NKFM_Key::MUSIC:
        case NKFM_Key::KEY_UNKNOWN:
        case NKFM_Key::NONE:
        default:
          return false;
      }

      NKFM_ActionDispatcher::execute(act, radio, onUIEvent, "IR");
      return true;
    }
};
