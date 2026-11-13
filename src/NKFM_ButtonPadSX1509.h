#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "NKFM_ButtonPad.h"

class NKFM_ButtonPadSX1509 {
  private:
    TwoWire*              _wire;
    uint8_t               _addr;
    int                   _intPin;
    const NKFM_ButtonDef* _btns;
    uint8_t               _count;
    uint16_t              _longMs;
    NKFM_ButtonCallback   _cb = nullptr;

    uint16_t      _stableState   = 0xFFFF;
    uint16_t      _lastRawState  = 0xFFFF;
    unsigned long _lastDeb       = 0;
    unsigned long _pressStart[16]= {0};

    void writeReg(uint8_t reg, uint8_t val) {
      _wire->beginTransmission(_addr);
      _wire->write(reg);
      _wire->write(val);
      _wire->endTransmission();
    }

    uint16_t readData16() {
      _wire->beginTransmission(_addr);
      _wire->write(0x10); // RegDataB (IO15..IO8) seguido de RegDataA (IO7..IO0)
      if (_wire->endTransmission(false) != 0) return _stableState;
      _wire->requestFrom(_addr, (uint8_t)2);
      if (_wire->available() < 2) return _stableState;
      uint8_t hi = _wire->read();
      uint8_t lo = _wire->read();
      return ((uint16_t)hi << 8) | lo;
    }

  public:
    NKFM_ButtonPadSX1509(TwoWire& wirePort, uint8_t addr, int intPin,
                         const NKFM_ButtonDef* btns, uint8_t count, uint16_t longPressMs = 700)
      : _wire(&wirePort), _addr(addr), _intPin(intPin),
        _btns(btns), _count(min(count, (uint8_t)16)), _longMs(longPressMs) {}

    void begin(NKFM_ButtonCallback cb) {
      _cb = cb;
      if (_intPin >= 0) pinMode(_intPin, INPUT_PULLUP);

      // Software reset del SX1509
      writeReg(0x7D, 0x12);
      writeReg(0x7D, 0x34);
      delay(5);

      // Configurar los 16 canales como entradas con pull-up interno activo
      writeReg(0x0E, 0xFF); // RegDirB
      writeReg(0x0F, 0xFF); // RegDirA
      writeReg(0x06, 0xFF); // RegPullUpB
      writeReg(0x07, 0xFF); // RegPullUpA

      // Habilitar interrupciones por cambio de flanco
      writeReg(0x12, 0x00); // RegInterruptMaskB
      writeReg(0x13, 0x00); // RegInterruptMaskA
      writeReg(0x14, 0xFF); // RegSenseHighB
      writeReg(0x15, 0xFF); // RegSenseLowB
      writeReg(0x16, 0xFF); // RegSenseHighA
      writeReg(0x17, 0xFF); // RegSenseLowA

      // Limpiar flags de interrupción iniciales
      writeReg(0x18, 0xFF);
      writeReg(0x19, 0xFF);
      _stableState = _lastRawState = readData16();
    }

    void poll(unsigned long now) {
      if (_count == 0 || !_btns) return;

      if (_intPin >= 0 && digitalRead(_intPin) == HIGH && _stableState == 0xFFFF && _lastRawState == 0xFFFF) {
        return;
      }

      uint16_t raw = readData16();
      writeReg(0x18, 0xFF);
      writeReg(0x19, 0xFF);

      if (raw != _lastRawState) {
        _lastDeb = now;
        _lastRawState = raw;
      }

      if ((now - _lastDeb) >= 35 && raw != _stableState) {
        uint16_t changed = _stableState ^ raw;
        _stableState = raw;

        for (uint8_t i = 0; i < _count; i++) {
          uint8_t ch = (uint8_t)(_btns[i].pin & 0x0F); // Canal físico IO0..IO15 configurado en NKFM_BTNS
          if (changed & (1U << ch)) {
            bool pressed = !(raw & (1U << ch));
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
