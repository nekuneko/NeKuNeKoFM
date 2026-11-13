#pragma once
#include <Arduino.h>
#include <Wire.h>

class NKFM_BatteryMAX17048 {
  private:
    TwoWire* _wire;
    uint8_t  _addr;
    float    _volts      = 0.0f;
    uint8_t  _percent    = 0;
    float    _lastLogV   = -1.0f;
    uint8_t  _lastLogPct = 255;

    static uint8_t voltsToPercent(float v) {
      if (v >= 4.15f) return 100;
      if (v <= 3.30f) return 0;

      // Curva de descarga LiPo realista por tramos (voltaje -> porcentaje)
      struct Point { float v; uint8_t pct; };
      static constexpr Point curve[] = {
        { 4.15f, 100 },
        { 4.05f,  90 },
        { 3.95f,  80 },
        { 3.87f,  70 },
        { 3.82f,  60 },
        { 3.79f,  50 },
        { 3.76f,  40 },
        { 3.73f,  30 },
        { 3.68f,  20 },
        { 3.58f,  10 },
        { 3.45f,   5 },
        { 3.30f,   0 }
      };

      constexpr uint8_t n = sizeof(curve) / sizeof(*curve);
      for (uint8_t i = 0; i < n - 1; i++) {
        if (v >= curve[i + 1].v) {
          float ratio = (v - curve[i + 1].v) / (curve[i].v - curve[i + 1].v);
          return (uint8_t)(curve[i + 1].pct + ratio * (curve[i].pct - curve[i + 1].pct) + 0.5f);
        }
      }
      return 0;
    }

  public:
    NKFM_BatteryMAX17048(TwoWire& wirePort = Wire, uint8_t addr = 0x36)
      : _wire(&wirePort), _addr(addr) {}

    bool update() {
      _wire->beginTransmission(_addr);
      _wire->write(0x02); // Registro VCELL
      if (_wire->endTransmission(false) != 0) return false;
      _wire->requestFrom(_addr, (uint8_t)2);
      if (_wire->available() < 2) return false;

      uint16_t rawV = ((uint16_t)_wire->read() << 8) | _wire->read();
      _volts   = rawV * 0.000078125f;
      _percent = voltsToPercent(_volts);
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
