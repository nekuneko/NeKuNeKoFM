#pragma once
#include "NeKuNeKoFM.h"

class NKFM_ActionDispatcher {
  public:
    static void execute(NKFM_Action act, NKFM_Receiver& radio, NKFM_EventCallback onUIEvent, const char* src) {
      if (act >= NKFM_Action::PRESET_1 && act <= NKFM_Action::PRESET_10) {
        radio.selectPreset((uint8_t)act - (uint8_t)NKFM_Action::PRESET_1, src);
        return;
      }

      switch (act) {
        case NKFM_Action::PRESET_NEXT:    radio.nextPreset(src); break;
        case NKFM_Action::PRESET_PREV:    radio.prevPreset(src); break;
        case NKFM_Action::SEEK_UP:        radio.seek(true, src); break;
        case NKFM_Action::SEEK_DOWN:      radio.seek(false, src); break;
        case NKFM_Action::FREQ_UP:        radio.stepFrequency(+1, src); break;
        case NKFM_Action::FREQ_DOWN:      radio.stepFrequency(-1, src); break;
        case NKFM_Action::VOL_UP:         radio.changeVolume(+1, src); break;
        case NKFM_Action::VOL_DOWN:       radio.changeVolume(-1, src); break;
        case NKFM_Action::MUTE_TOGGLE:    radio.toggleMute(src); break;
        case NKFM_Action::MONO_TOGGLE:    radio.toggleMono(src); break;
        case NKFM_Action::BASS_TOGGLE:    radio.toggleBassBoost(src); break;
        case NKFM_Action::THEME_NEXT:     if (onUIEvent) onUIEvent(NKFM_UIEvent::THEME_NEXT, 1, src); break;
        case NKFM_Action::THEME_PREV:     if (onUIEvent) onUIEvent(NKFM_UIEvent::THEME_NEXT, 0, src); break;
        case NKFM_Action::SKIN_NEXT:      if (onUIEvent) onUIEvent(NKFM_UIEvent::SKIN_NEXT, 1, src); break;
        case NKFM_Action::SKIN_PREV:      if (onUIEvent) onUIEvent(NKFM_UIEvent::SKIN_NEXT, 0, src); break;
        case NKFM_Action::ROTATION_NEXT:  if (onUIEvent) onUIEvent(NKFM_UIEvent::ROTATION_NEXT, 1, src); break;
        case NKFM_Action::ROTATION_PREV:  if (onUIEvent) onUIEvent(NKFM_UIEvent::ROTATION_NEXT, 0, src); break;
        case NKFM_Action::DISPLAY_TOGGLE: if (onUIEvent) onUIEvent(NKFM_UIEvent::DISPLAY_TOGGLE, 0, src); break;
        case NKFM_Action::VIEW_TOGGLE:    if (onUIEvent) onUIEvent(NKFM_UIEvent::VIEW_TOGGLE, 0, src); break;

        default: break;
      }
    }
};
