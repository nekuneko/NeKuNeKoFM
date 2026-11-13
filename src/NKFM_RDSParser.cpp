#include "NKFM_RDSParser.h"

NKFM_RDSParser::NKFM_RDSParser() {
  reset();
}

void NKFM_RDSParser::reset() {
  memset(_rdsPS, ' ', 8);
  _rdsPS[8] = '\0';
  memset(_rdsPS_buf, ' ', 8);
  _rdsPS_buf[8] = '\0';
  _psMask = 0;

  memset(_rdsRT, ' ', 64);
  _rdsRT[64] = '\0';
  _rdsLastAB = 255;
  _rtMask    = 0;
  _rtEndMask = 0xFFFF;
  _rtReady   = false;
}

void NKFM_RDSParser::setCallbacks(RDSServiceCallback onPS, RDSTextCallback onRT, RDSTimeCallback onTime, RDSPtyCallback onPTY) {
  _onPS   = onPS;
  _onRT   = onRT;
  _onTime = onTime;
  _onPTY  = onPTY;
}

char NKFM_RDSParser::rdsCharToGFX(uint8_t c) {
  if (c >= 0x20 && c <= 0x7E) return (char)c;
  switch (c) {
    case 0x91: return (char)0xA0; // á
    case 0x92: return (char)0x82; // é
    case 0x93: return (char)0xA1; // í
    case 0x94: return (char)0xA2; // ó
    case 0x95: return (char)0xA3; // ú
    case 0x97: return (char)0xA4; // ñ
    case 0xB7: return (char)0xA5; // Ñ
    case 0x99: return (char)0x81; // ü
    case 0xB9: return (char)0x9A; // Ü
    case 0x9B: return (char)0x87; // ç
    case 0xBB: return (char)0x80; // Ç
    case 0x8E: return (char)0xAD; // ¡
    case 0xBF: return (char)0xA8; // ¿
    case 0xD1: return 'A';
    case 0xD2: return 'E';
    case 0xD3: return 'I';
    case 0xD4: return 'O';
    case 0xD5: return 'U';
    default:   return 0;
  }
}

void NKFM_RDSParser::feedGroup(uint16_t b1, uint16_t b2, uint16_t b3, uint16_t b4,
                               uint8_t errB, uint8_t errC, uint8_t errD) {
  (void)b1;
  if (errB == 3) return;

  uint8_t groupType = (b2 >> 12) & 0x0F;
  bool versionB     = (b2 & 0x0800) != 0;
  uint8_t pty       = (b2 >> 5) & 0x1F;

  if (_onPTY) _onPTY(pty);

  if (groupType == 0 && errB == 0 && errD == 0) {
    uint8_t seg = b2 & 0x03;
    uint8_t idx = seg * 2;
    char c1 = rdsCharToGFX((b4 >> 8) & 0xFF);
    char c2 = rdsCharToGFX(b4 & 0xFF);

    if (c1 && c2) {
      _rdsPS_buf[idx]     = c1;
      _rdsPS_buf[idx + 1] = c2;
      _psMask |= (1U << seg);

      if (_psMask == 0x0F) {
        memcpy(_rdsPS, _rdsPS_buf, 8);
        _rdsPS[8] = '\0';
        if (_onPS) _onPS(_rdsPS);
        _psMask = 0;
      }
    }
  }
  else if (groupType == 2 && !versionB && errB == 0 && errC == 0 && errD == 0) {
    uint8_t ab  = (b2 >> 4) & 0x01;
    uint8_t seg = b2 & 0x0F;
    uint8_t idx = seg * 4;

    if (ab != _rdsLastAB) {
      memset(_rdsRT, ' ', 64);
      _rdsRT[64] = '\0';
      _rdsLastAB = ab;
      _rtMask    = 0;
      _rtEndMask = 0xFFFF;
      _rtReady   = false;
    }

    if (!_rtReady) {
      uint8_t ch[4] = {
        (uint8_t)((b3 >> 8) & 0xFF), (uint8_t)(b3 & 0xFF),
        (uint8_t)((b4 >> 8) & 0xFF), (uint8_t)(b4 & 0xFF)
      };

      for (uint8_t i = 0; i < 4; i++) {
        if (ch[i] == '\r') {
          _rdsRT[idx + i] = '\0';
          _rtEndMask = (uint16_t)((1UL << (seg + 1)) - 1);
          break;
        }
        char mapped = rdsCharToGFX(ch[i]);
        if ((idx + i) < 64) {
          _rdsRT[idx + i] = mapped ? mapped : ' ';
        }
      }

      _rtMask |= (uint16_t)(1U << seg);

      if ((_rtMask & _rtEndMask) == _rtEndMask) {
        _rtReady = true;
        if (_onRT) _onRT(_rdsRT);
      }
    }
  }
  else if (groupType == 4 && !versionB && errB == 0 && errC == 0 && errD == 0) {
    uint8_t hour   = ((b3 & 0x01) << 4) | ((b4 >> 12) & 0x0F);
    uint8_t minute = (b4 >> 6) & 0x3F;
    int8_t offset  = (b4 & 0x1F);
    if (b4 & 0x20) offset = -offset;
    int totalMin = (hour * 60) + minute + (offset * 30);
    if (totalMin < 0) totalMin += 1440;
    uint8_t localH = (totalMin / 60) % 24;
    uint8_t localM = totalMin % 60;
    if (_onTime) _onTime(localH, localM);
  }
}
