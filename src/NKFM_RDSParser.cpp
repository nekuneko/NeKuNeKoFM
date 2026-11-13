#include "NKFM_RDSParser.h"

NKFM_RDSParser::NKFM_RDSParser() {
  reset();
}

char NKFM_RDSParser::decodeCP437(uint8_t c) {
  if (c >= 0x20 && c <= 0x7E) return (char)c;
  switch (c) {
    case 0x80: case 0x87: return 'c';
    case 0x81: case 0x9A: return 'u';
    case 0x82: case 0x90: case 0x88: case 0x89: case 0x8A: return 'e';
    case 0x83: case 0x84: case 0x85: case 0x86: case 0x8E: case 0x8F: case 0xA0: return 'a';
    case 0x8B: case 0x8C: case 0x8D: case 0xA1: return 'i';
    case 0x91: case 0x92: return 'a';
    case 0x93: case 0x94: case 0x95: case 0x99: case 0xA2: return 'o';
    case 0x96: case 0x97: return 'u';
    case 0xA3: return 'u';
    case 0xA4: return 'n';
    case 0xA5: return 'N';
    case 0xE1: return 's';
    default:   return ' ';
  }
}

void NKFM_RDSParser::reset() {
  memset(_psName1, ' ', 8);
  _psName1[8] = '\0';
  memset(_psName2, ' ', 8);
  _psName2[8] = '\0';
  memset(_programServiceName, 0, sizeof(_programServiceName));

  memset(_rdstext, 0, sizeof(_rdstext));
  memset(_lastSentRT, 0, sizeof(_lastSentRT));
  _lastTextIDX = 0;
  _textAB      = 0;
  _lastTextAB  = 255;
  _rtMask      = 0;
  _rtEndSeg    = 255;

  _lastMinutes = 0;
  _lastPTY     = 255;

  if (_sendServiceName) _sendServiceName("");
  if (_sendText)        _sendText("");
  if (_sendPTY)         _sendPTY(0);
}


void NKFM_RDSParser::feedGroup(uint16_t b1, uint16_t b2, uint16_t b3, uint16_t b4,
                               uint8_t errB, uint8_t errC, uint8_t errD) {
  (void)b1;
  if (errB > 1) return;

  uint8_t rdsGroupType = 0x0A | ((b2 & 0xF000) >> 8) | ((b2 & 0x0800) >> 11);

  uint8_t pty = (b2 >> 5) & 0x1F;
  if (pty != _lastPTY) {
    _lastPTY = pty;
    if (_sendPTY) _sendPTY(pty);
  }

  switch (rdsGroupType) {
    case 0x0A:
    case 0x0B: {
      if (errD > 1) break;
      uint8_t idx = 2 * (b2 & 0x0003);
      char c1 = decodeCP437((b4 >> 8) & 0xFF);
      char c2 = decodeCP437(b4 & 0xFF);

      if ((_psName1[idx] == c1) && (_psName1[idx + 1] == c2)) {
        _psName2[idx]     = c1;
        _psName2[idx + 1] = c2;
        _psName2[8]       = '\0';

        if (idx == 6 && strcmp(_psName1, _psName2) == 0) {
          if (strcmp(_psName2, _programServiceName) != 0) {
            strcpy(_programServiceName, _psName2);
            if (_sendServiceName) _sendServiceName(_programServiceName);
          }
        }
      }
      if ((_psName1[idx] != c1) || (_psName1[idx + 1] != c2)) {
        _psName1[idx]     = c1;
        _psName1[idx + 1] = c2;
        _psName1[8]       = '\0';
      }
      break;
    }

    case 0x2A: {
      // Exigir 0 errores en bloques de texto C y D para evitar caracteres corruptos
      if (errC > 0 || errD > 0) break;

      uint8_t seg = (b2 & 0x000F); // Segmento 0..15 (4 caracteres cada uno)
      uint8_t idx = 4 * seg;
      _textAB = (b2 & 0x0010);

      // Si cambia el flag A/B de la emisora, reiniciar el ensamblador de frase
      if (_textAB != _lastTextAB) {
        _lastTextAB = _textAB;
        memset(_rdstext, 0, sizeof(_rdstext));
        _rtMask   = 0;
        _rtEndSeg = 255;
      }

      uint8_t raw[4] = {
        (uint8_t)((b3 >> 8) & 0xFF),
        (uint8_t)(b3 & 0xFF),
        (uint8_t)((b4 >> 8) & 0xFF),
        (uint8_t)(b4 & 0xFF)
      };

      for (uint8_t i = 0; i < 4; i++) {
        if (raw[i] == 0x0D || raw[i] == 0x00) {
          _rdstext[idx + i] = '\0';
          _rtEndSeg = seg; // Marca este segmento como el final oficial de la frase
          break;
        }
        _rdstext[idx + i] = decodeCP437(raw[i]);
      }
      _rdstext[64] = '\0';
      _rtMask |= (1U << seg);

      // Determinar cuál es el último segmento esperado
      uint8_t targetSeg = (_rtEndSeg != 255) ? _rtEndSeg : 15;

      // Verificar que TODOS los segmentos desde el 0 hasta targetSeg estén recibidos sin huecos
      uint16_t requiredMask = (targetSeg == 15) ? 0xFFFF : ((1U << (targetSeg + 1)) - 1U);
      if ((_rtMask & requiredMask) == requiredMask) {
        // Limpiar espacios finales antes de comparar y enviar
        char cleanBuf[66];
        strncpy(cleanBuf, _rdstext, 64);
        cleanBuf[64] = '\0';
        int len = strlen(cleanBuf);
        while (len > 0 && cleanBuf[len - 1] == ' ') {
          cleanBuf[--len] = '\0';
        }

        if (len > 0 && strcmp(cleanBuf, _lastSentRT) != 0) {
          strcpy(_lastSentRT, cleanBuf);
          if (_sendText) _sendText(_lastSentRT);
        }
      }
      break;
    }

    case 0x4A: {
      if (errC > 0 || errD > 0) break;
      uint32_t mjd = ((uint32_t)(b2 & 0x0003) << 15) | ((uint32_t)(b3 & 0xFFFE) >> 1);
      if (mjd < 58849) break;

      uint16_t minutes = ((b4 >> 6) & 0x003F);
      uint16_t hours   = ((b3 & 0x0001) << 4) | ((b4 >> 12) & 0x000F);
      int8_t   off     = (b4 & 0x001F);
      if (b4 & 0x0020) off = -off;

      int32_t totalMins = (int32_t)hours * 60 + (int32_t)minutes + (int32_t)off * 30;
      while (totalMins < 0)     totalMins += 1440;
      while (totalMins >= 1440) totalMins -= 1440;

      uint16_t cleanTime = (uint16_t)totalMins;
      if (cleanTime != _lastMinutes) {
        _lastMinutes = cleanTime;
        if (_sendTime) _sendTime((uint8_t)(cleanTime / 60), (uint8_t)(cleanTime % 60));
      }
      break;
    }

    default:
      break;
  }
}
