#pragma once
#include <Arduino.h>
#include <Wire.h>

class NKFM_BatteryMAX17048 {
  private:
    TwoWire* _wire;
    uint8_t  _addr;
    float    _volts   = 0.0f;
    uint8_t  _percent = 0;
    float    _lastLogV   = -1.0f;
    uint8_t  _lastLogPct = 255;

  public:
    NKFM_BatteryMAX17048(TwoWire& wirePort = Wire, uint8_t addr = 0x36)
      : _wire(&wirePort), _addr(addr) {}

    bool update() {
      _wire->beginTransmission(_addr);
      _wire->write(0x02);
      if (_wire->endTransmission(false) != 0) return false;
      _wire->requestFrom(_addr, (uint8_t)4);
      if (_wire->available() < 4) return false;

      uint16_t rawV   = ((uint16_t)_wire->read() << 8) | _wire->read();
      uint16_t rawSoc = ((uint16_t)_wire->read() << 8) | _wire->read();
      _volts   = rawV * 0.000078125f;
      _percent = (uint8_t)constrain((rawSoc >> 8) + ((rawSoc & 0xFF) / 256.0f), 0.0f, 100.0f);
      return true;
    }

    bool hasSignificantChange() {
      if (_percent != _lastLogPct || fabs(_volts - _lastLogV) >= 0.02f) {
        _lastLogV = _volts;
        _lastLogPct = _percent;
        return true;
      }
      return false;
    }

    float   getVolts() const   { return _volts; }
    uint8_t getPercent() const { return _percent; }
};
