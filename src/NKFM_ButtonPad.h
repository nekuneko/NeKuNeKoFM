#pragma once
#include <Arduino.h>

typedef void (*NKFM_ButtonCallback)(uint8_t index, int pin, bool isLongPress);

class NKFM_ButtonPad {
  private:
    const int*          _pins;
    uint8_t             _count;
    uint16_t            _longMs;
    NKFM_ButtonCallback _cb = nullptr;

    uint8_t       _stableState   = 0xFF;
    uint8_t       _lastRawState  = 0xFF;
    unsigned long _lastDeb       = 0;
    unsigned long _pressStart[8] = {0};

  public:
    NKFM_ButtonPad(const int* pins, uint8_t count, uint16_t longPressMs = 700)
      : _pins(pins), _count(min(count, (uint8_t)8)), _longMs(longPressMs) {}

    void begin(NKFM_ButtonCallback cb) {
      _cb = cb;
      for (uint8_t i = 0; i < _count; i++) pinMode(_pins[i], INPUT_PULLUP);
    }

    void poll(unsigned long now) {
      if (_count == 0) return;
      uint8_t raw = 0;
      for (uint8_t i = 0; i < _count; i++) {
        if (digitalRead(_pins[i]) == HIGH) raw |= (1U << i);
      }
      if (raw != _lastRawState) { _lastDeb = now; _lastRawState = raw; }
      if ((now - _lastDeb) >= 35 && raw != _stableState) {
        uint8_t changed = _stableState ^ raw;
        _stableState = raw;
        for (uint8_t i = 0; i < _count; i++) {
          if (changed & (1U << i)) {
            bool pressed = !(raw & (1U << i));
            if (pressed) {
              _pressStart[i] = now;
            } else if (_pressStart[i] > 0) {
              bool isLong = (now - _pressStart[i]) >= _longMs;
              _pressStart[i] = 0;
              if (_cb) _cb(i, _pins[i], isLong);
            }
          }
        }
      }
    }
};
