#pragma once
#include "NKFM_IR.h"

#if defined(ARDUINO_ARCH_SAMD)
  #include <IRLibDecodeBase.h>
  #include <IRLib_P01_NEC.h>
  #include <IRLib_P02_Sony.h>
  #include <IRLib_P03_RC5.h>
  #include <IRLib_HashRaw.h>
  #include <IRLibCombo.h>
  #include <IRLibRecv.h>

class NKFM_IRSony : public NKFM_IRRemote {
  private:
    IRrecv    receiver;
    IRdecode  decoder;
    uint32_t  lastRawCode  = 0;
    unsigned long lastPacketMs = 0;
    unsigned long lastValidMs  = 0;

    static NKFM_Key mapSonyCode(uint32_t code) {
      switch (code) {
        case 0xA8B5B: return NKFM_Key::POWER;
        case 0x68B5B: return NKFM_Key::EJECT;

        case 0x00B92: return NKFM_Key::NUM_1;
        case 0x80B92: return NKFM_Key::NUM_2;
        case 0x40B92: return NKFM_Key::NUM_3;
        case 0xC0B92: return NKFM_Key::NUM_4;
        case 0x20B92: return NKFM_Key::NUM_5;
        case 0xA0B92: return NKFM_Key::NUM_6;
        case 0x60B92: return NKFM_Key::NUM_7;
        case 0xE0B92: return NKFM_Key::NUM_8;
        case 0x10B92: return NKFM_Key::NUM_9;
        case 0x90B92: return NKFM_Key::NUM_0;

        case 0x9EB92: return NKFM_Key::UP;
        case 0x5EB92: return NKFM_Key::DOWN;
        case 0xDEB92: return NKFM_Key::LEFT;
        case 0x3EB92: return NKFM_Key::RIGHT;

        case 0xD0B92: return NKFM_Key::OK;
        case 0xD8B92: return NKFM_Key::MENU;
        case 0x0AB5B: return NKFM_Key::SELECT;
        case 0xF0B92: return NKFM_Key::CLEAR;

        case 0x0CB92: return NKFM_Key::PREVIOUS;
        case 0x8CB92: return NKFM_Key::NEXT;
        case 0xCCB92: return NKFM_Key::REWIND;
        case 0x2CB92: return NKFM_Key::FAST_FORWARD;
        case 0x9CB92: return NKFM_Key::PAUSE;
        case 0x1CB92: return NKFM_Key::STOP;

        case 0x4CB92: return NKFM_Key::BOOKMARK;
        case 0x58B92: return NKFM_Key::SCENE;
        case 0xC6B92: return NKFM_Key::SUBTITLES;
        case 0x26B92: return NKFM_Key::MUSIC;

        default:      return NKFM_Key::KEY_UNKNOWN;
      }
    }

  public:
    NKFM_IRSony(uint8_t irqPin) :  receiver(irqPin) {}

    void begin() override {
      receiver.enableIRIn();
    }

    void flush() override {
      if (receiver.getResults()) {
        decoder.value = 0;
        receiver.enableIRIn();
      }
      unsigned long now = millis();
      lastPacketMs = now;
      lastValidMs  = now;
    }

    NKFM_Key poll(uint32_t *rawOut = nullptr) override {
      if (!receiver.getResults()) return NKFM_Key::NONE;

      NKFM_Key result = NKFM_Key::NONE;
      if (decoder.decode() && decoder.protocolNum == SONY && decoder.bits >= 12) {
        uint32_t code = decoder.value;
        unsigned long now = millis();
        unsigned long gap = now - lastPacketMs;
        lastPacketMs = now;

        // Teclas que permiten repetición continua al mantenerlas pulsadas (Volumen +/- y Frecuencia +/-)
        bool allowHold = (code == 0x9EB92 || code == 0x5EB92 || code == 0xCCB92 || code == 0x2CB92);

        if (code != lastRawCode) {
          if ((now - lastValidMs) >= 100) {
            lastRawCode = code;
            lastValidMs = now;
            if (rawOut) *rawOut = code;
            result = mapSonyCode(code);
          }
        } else if (allowHold) {
          if ((now - lastValidMs) >= 170) {
            lastValidMs = now;
            if (rawOut) *rawOut = code;
            result = mapSonyCode(code);
          }
        } else if (gap >= 450 && (now - lastValidMs) >= 550) {
          lastValidMs = now;
          if (rawOut) *rawOut = code;
          result = mapSonyCode(code);
        }
      }

      decoder.value = 0;
      receiver.enableIRIn();
      return result;
    }

    static const char* getKeyName(NKFM_Key k) {
      switch (k) {
        case NKFM_Key::POWER:        return "POWER";
        case NKFM_Key::EJECT:        return "EJECT";
        case NKFM_Key::NUM_0:        return "0";
        case NKFM_Key::NUM_1:        return "1";
        case NKFM_Key::NUM_2:        return "2";
        case NKFM_Key::NUM_3:        return "3";
        case NKFM_Key::NUM_4:        return "4";
        case NKFM_Key::NUM_5:        return "5";
        case NKFM_Key::NUM_6:        return "6";
        case NKFM_Key::NUM_7:        return "7";
        case NKFM_Key::NUM_8:        return "8";
        case NKFM_Key::NUM_9:        return "9";
        case NKFM_Key::UP:           return "UP";
        case NKFM_Key::DOWN:         return "DOWN";
        case NKFM_Key::LEFT:         return "LEFT";
        case NKFM_Key::RIGHT:        return "RIGHT";
        case NKFM_Key::OK:           return "OK";
        case NKFM_Key::MENU:         return "MENU";
        case NKFM_Key::SELECT:       return "SELECT";
        case NKFM_Key::CLEAR:        return "CLEAR";
        case NKFM_Key::PREVIOUS:     return "PREV";
        case NKFM_Key::NEXT:         return "NEXT";
        case NKFM_Key::REWIND:       return "REW";
        case NKFM_Key::FAST_FORWARD: return "FF";
        case NKFM_Key::PAUSE:        return "PAUSE";
        case NKFM_Key::STOP:         return "STOP";
        case NKFM_Key::BOOKMARK:     return "BOOKMARK";
        case NKFM_Key::SCENE:        return "SCENE";
        case NKFM_Key::SUBTITLES:    return "SUBTITLES";
        case NKFM_Key::MUSIC:        return "MUSIC";
        default:                     return "UNKNOWN";
      }
    }
};
#endif
