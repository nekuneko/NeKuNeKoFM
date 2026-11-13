#pragma once
#include <Arduino.h>
#include "NeKuNeKoFM.h"

typedef void (*NKFM_ButtonCallback)(uint8_t idx, const NKFM_ButtonDef& btn, bool isLongPress);

class NKFM_ButtonPad {
  private:
    const NKFM_ButtonDef* _btns;
    uint8_t               _count;
    uint16_t              _longMs;
    NKFM_ButtonCallback   _cb = nullptr;

    uint16_t      _stableState   = 0xFFFF;
    uint16_t      _lastRawState  = 0xFFFF;
    unsigned long _lastDeb       = 0;
    unsigned long _pressStart[16]= {0};

  public:
    NKFM_ButtonPad(const NKFM_ButtonDef* btns, uint8_t count, uint16_t longPressMs = 700)
      : _btns(btns), _count(min(count, (uint8_t)16)), _longMs(longPressMs) {}

    void begin(NKFM_ButtonCallback cb) {
      _cb = cb;
      if (!_btns) return;
      for (uint8_t i = 0; i < _count; i++) {
        pinMode(_btns[i].pin, INPUT_PULLUP);
      }
    }

    void poll(unsigned long now) {
      if (_count == 0 || !_btns) return;

      uint16_t raw = 0;
      for (uint8_t i = 0; i < _count; i++) {
        if (digitalRead(_btns[i].pin) == HIGH) raw |= (1U << i);
      }

      if (raw != _lastRawState) {
        _lastDeb = now;
        _lastRawState = raw;
      }

      if ((now - _lastDeb) >= 35 && raw != _stableState) {
        uint16_t changed = _stableState ^ raw;
        _stableState = raw;

        for (uint8_t i = 0; i < _count; i++) {
          if (changed & (1U << i)) {
            bool pressed = !(raw & (1U << i));
            if (pressed) {
              _pressStart[i] = now;
            } else if (_pressStart[i] > 0) {
              bool isLong = (now - _pressStart[i]) >= _longMs;
              _pressStart[i] = 0;
              if (_cb) _cb(i, _btns[i], isLong);
            }
          }
        }
      }
    }
};
