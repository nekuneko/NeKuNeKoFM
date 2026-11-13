#pragma once
#include <Arduino.h>
#include "NeKuNeKoFM.h"

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

    // Mapa estándar de acciones del mando IR.
    // Devuelve true si la tecla tiene acción estándar asignada, o false si está libre.
    static bool dispatchDefault(NKFM_Key key, NKFM_Receiver& radio, NKFM_EventCallback onUIEvent = nullptr) {
      switch (key) {
        // --- 1. ASIGNADAS: Selección directa de Presets (1..10) ---
        case NKFM_Key::NUM_1: radio.selectPreset(0, "IR"); return true;
        case NKFM_Key::NUM_2: radio.selectPreset(1, "IR"); return true;
        case NKFM_Key::NUM_3: radio.selectPreset(2, "IR"); return true;
        case NKFM_Key::NUM_4: radio.selectPreset(3, "IR"); return true;
        case NKFM_Key::NUM_5: radio.selectPreset(4, "IR"); return true;
        case NKFM_Key::NUM_6: radio.selectPreset(5, "IR"); return true;
        case NKFM_Key::NUM_7: radio.selectPreset(6, "IR"); return true;
        case NKFM_Key::NUM_8: radio.selectPreset(7, "IR"); return true;
        case NKFM_Key::NUM_9: radio.selectPreset(8, "IR"); return true;
        case NKFM_Key::NUM_0: radio.selectPreset(9, "IR"); return true;

        // --- 2. ASIGNADAS: Control de Radio y Audio ---
        case NKFM_Key::UP:           radio.changeVolume(+1, "IR"); return true;
        case NKFM_Key::DOWN:         radio.changeVolume(-1, "IR"); return true;
        case NKFM_Key::LEFT:         radio.prevPreset("IR");       return true;
        case NKFM_Key::RIGHT:        radio.nextPreset("IR");       return true;
        case NKFM_Key::REWIND:       radio.stepFrequency(-1, "IR");return true;
        case NKFM_Key::FAST_FORWARD: radio.stepFrequency(+1, "IR");return true;
        case NKFM_Key::PREVIOUS:     radio.seek(false, "IR");      return true;
        case NKFM_Key::NEXT:         radio.seek(true, "IR");       return true;
        case NKFM_Key::OK:
        case NKFM_Key::PAUSE:        radio.toggleMute("IR");       return true;

        // --- 3. ASIGNADAS: Control de Pantalla (vía eventos UI) ---
        case NKFM_Key::POWER:
            if (onUIEvent) onUIEvent(NKFM_UIEvent::DISPLAY_TOGGLE, 0, "IR");
            return true;
        case NKFM_Key::SELECT:
            if (onUIEvent) onUIEvent(NKFM_UIEvent::ROTATION_NEXT, 0, "IR");
            return true;
        case NKFM_Key::SUBTITLES:
            if (onUIEvent) onUIEvent(NKFM_UIEvent::THEME_NEXT, 1, "IR");
            return true;
        case NKFM_Key::SCENE:
            if (onUIEvent) onUIEvent(NKFM_UIEvent::THEME_NEXT, 0, "IR");
            return true;

        // --- 4. SIN ASIGNAR (Libres para personalizar en el .ino) ---
        case NKFM_Key::MENU:
        case NKFM_Key::CLEAR:
        case NKFM_Key::STOP:
        case NKFM_Key::BOOKMARK:
        case NKFM_Key::EJECT:
        case NKFM_Key::MUSIC:
        case NKFM_Key::KEY_UNKNOWN:
        case NKFM_Key::NONE:
        default:
          return false;
      }
    }
};
