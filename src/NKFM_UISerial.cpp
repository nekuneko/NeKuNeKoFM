#include "NKFM_UISerial.h"

NKFM_UISerial::NKFM_UISerial(Stream* port) : _port(port) {}

void NKFM_UISerial::begin(const NKFM_SerialConfig& cfg, Stream* port) {
  _cfg = cfg;
  if (port) _port = port;
}

void NKFM_UISerial::logTag(const char* tag) const {
  if (!_port) return;
  unsigned long ms = millis();
  unsigned long s  = ms / 1000;
  char ts[16];
  snprintf(ts, sizeof(ts), "[%02lu:%02lu.%03lu] ", (s / 60) % 60, s % 60, ms % 1000);
  _port->print(ts);
  _port->print(tag);
  _port->print(' ');
}

void NKFM_UISerial::logRDS(const char* tag, const char* msg) const {
  if (!_port || !msg || *msg == '\0') return;
  logTag(tag);
  _port->println(msg);
}

void NKFM_UISerial::logBtnSnapshot(const char* fase, const int* pins, uint8_t count) const {
  if (!_port) return;
  logTag("[HW-CHK]");
  _port->print(fase);
  _port->print(F(" -> Pines: "));
  for (uint8_t i = 0; i < count; i++) {
    _port->print(digitalRead(pins[i]));
  }
  _port->println();
}

void NKFM_UISerial::logEvent(NKFM_UIEvent event, int value, const char* source) const {
  if (!_port || !_cfg.tuner) return;

  switch (event) {
    case NKFM_UIEvent::FREQ_CHANGED: {
      logTag("[RADIO]");
      _port->print(source); _port->print(F(" -> "));
      _port->print(value / 100.0f, 2); _port->print(F(" MHz"));
      int8_t idx = _cfg.tuner->getPresetIndex();
      const NKFM_Preset* list = _cfg.tuner->getPresets();
      if (idx >= 0 && list) {
        _port->print(F(" [P")); _port->print(idx + 1);
        _port->print(F(": ")); _port->print(list[idx].name); _port->print(']');
      }
      _port->println();
      break;
    }
    case NKFM_UIEvent::VOL_CHANGED:
      logTag("[AUDIO]");
      _port->print(source); _port->print(F(" -> Volumen: "));
      _port->print(value); _port->println(F("/15"));
      break;

    case NKFM_UIEvent::MUTE_CHANGED:
      logTag("[AUDIO]");
      _port->print(source);
      _port->println(value ? F(" -> MUTE ON") : F(" -> MUTE OFF"));
      break;

    case NKFM_UIEvent::MONO_CHANGED:
      logTag("[CMD]");
      _port->print(source);
      _port->println(value ? F(" -> Modo MONO forzado") : F(" -> Modo ESTEREO automatico"));
      break;

    case NKFM_UIEvent::BASS_CHANGED:
      logTag("[AUDIO]");
      _port->print(source);
      _port->println(value ? F(" -> Bass Boost (75us): ON") : F(" -> Bass Boost (50us): OFF"));
      break;

    default:
      break;
  }
}

void NKFM_UISerial::printHelpMenu() const {
  if (!_port || !_cfg.tuner) return;
  uint8_t pCount = _cfg.tuner->getPresetCount();

  _port->println(F("\n=========================================================="));
  _port->print(F("   ")); _port->println(_cfg.boardTitle ? _cfg.boardTitle : "NEKUNEKO FM RADIO");
  _port->println(F("=========================================================="));
  _port->println(F("  [FILA QWERTY]"));
  _port->println(F("    q / w     -> Bajar / Subir frecuencia (-0.1 / +0.1 MHz)"));
  _port->println(F("    e         -> Alternar Estereo / Mono"));
  _port->println(F("    r[0..3]   -> Rotar pantalla (ej: r o r2)"));
  if (_cfg.maxThemes > 0) {
    _port->print(F("    t[0..")); _port->print(_cfg.maxThemes - 1); _port->println(F("]   -> Cambiar tema (ej: t o t4)"));
  }
  _port->println(F("    i         -> Informe de senal y bateria"));
  if (pCount > 0) {
    _port->print(F("    p[1..")); _port->print(pCount); _port->println(F("]  -> Listar presets (p) o ir al N (ej: p3)"));
    _port->println(F("    a / s     -> Preset anterior / siguiente"));
  }
  _port->println(F("  [FILA ASDF]"));
  _port->println(F("    d         -> Debug registros crudos del sintonizador"));
  _port->println(F("    fNNNNN    -> Frecuencia directa (ej: f9740)"));
  _port->println(F("    h / ?     -> Mostrar este menu de ayuda"));
  _port->println(F("  [FILA ZXCV]"));
  _port->println(F("    z / x     -> Busqueda automatica (Seek - / +)"));
  _port->println(F("    c / v     -> Bajar / Subir volumen (0..15)"));
  _port->println(F("    b         -> Alternar Bass Boost / De-enfasis 75us"));
  _port->println(F("    m         -> Silenciar / Activar audio (Mute)"));
  _port->println(F("=========================================================="));
}

void NKFM_UISerial::printPresetList() const {
  if (!_port || !_cfg.tuner) return;
  const NKFM_Preset* list = _cfg.tuner->getPresets();
  uint8_t count = _cfg.tuner->getPresetCount();
  int8_t act = _cfg.tuner->getPresetIndex();
  if (!list || count == 0) return;

  _port->println(F("\n--- LISTA DE PRESETS GUARDADOS ---"));
  for (uint8_t i = 0; i < count; i++) {
    char line[64];
    snprintf(line, sizeof(line), "  [%02d] %3d.%02d MHz - %s%s",
             i + 1, list[i].freq / 100, list[i].freq % 100, list[i].name,
             ((int8_t)i == act) ? "  <-- ACTIVA" : "");
    _port->println(line);
  }
  _port->println(F("----------------------------------\n"));
}

void NKFM_UISerial::runCommand(char cmd, int16_t value) {
  if (!_port || !_cfg.tuner) return;

  switch (cmd) {
    case 'q': case 'Q': _cfg.tuner->stepFrequency(-1, "SERIE 'q'"); break;
    case 'w': case 'W': _cfg.tuner->stepFrequency(+1, "SERIE 'w'"); break;
    case 'e': case 'E': _cfg.tuner->toggleMono("SERIE 'e'"); break;
    case 'r': case 'R':
      if (value >= 0) notifyUI(NKFM_UIEvent::ROTATION_SET, value % 4);
      else            notifyUI(NKFM_UIEvent::ROTATION_NEXT, 0);
      break;
    case 't': case 'T':
      if (value >= 0) notifyUI(NKFM_UIEvent::THEME_SET, value);
      else            notifyUI(NKFM_UIEvent::THEME_NEXT, 1);
      break;
    case 'i': case 'I': {
      _cfg.tuner->updateSignal();
      notifyUI(NKFM_UIEvent::SIGNAL_CHANGED, 0);
      logTag("[SENAL]"); _port->print(F("RSSI:")); _port->print(_cfg.tuner->getRSSI());
      _port->print(F(" SNR:")); _port->print(_cfg.tuner->getSNR());
      _port->println(_cfg.tuner->isStereo() ? F(" STEREO") : F(" MONO"));
      if (_cfg.onReportBat) _cfg.onReportBat();
      break;
    }
    case 'p': case 'P':
      if (value >= 1) _cfg.tuner->selectPreset((uint8_t)(value - 1), "SERIE 'pN'");
      else printPresetList();
      break;
    case 'a': case 'A': _cfg.tuner->prevPreset("SERIE 'a'"); break;
    case 's': case 'S': _cfg.tuner->nextPreset("SERIE 's'"); break;
    case 'd': case 'D': _cfg.tuner->printDebugStatus(_port); break;
    case 'f': case 'F':
      if (value >= 8750 && value <= 10800) _cfg.tuner->tuneTo((RADIO_FREQ)value, "SERIE 'f'");
      else { logTag("[ERR]"); _port->println(F("Usa f8750..f10800")); }
      break;
    case 'h': case 'H': case '?': printHelpMenu(); break;
    case 'z': case 'Z': _cfg.tuner->seek(false, "SERIE 'z'"); break;
    case 'x': case 'X': _cfg.tuner->seek(true, "SERIE 'x'"); break;
    case 'c': case 'C': _cfg.tuner->changeVolume(-1, "SERIE 'c'"); break;
    case 'v': case 'V': _cfg.tuner->changeVolume(+1, "SERIE 'v'"); break;
    case 'b': case 'B': _cfg.tuner->toggleBassBoost("SERIE 'b'"); break;
    case 'm': case 'M': _cfg.tuner->toggleMute("SERIE 'm'"); break;
    default: logTag("[WARN]"); _port->print(F("Comando no reconocido: ")); _port->println(cmd); break;
  }
}

void NKFM_UISerial::poll() {
  if (!_port) return;

  while (_port->available() > 0) {
    char c = _port->read();
    if (c == '\r' || c == '\n') {
      if (_parsingInt) {
        runCommand(_cmd, _hasDigits ? _val : -1);
        _parsingInt = false; _cmd = 0; _val = 0; _hasDigits = false;
      }
    } else if (!_parsingInt) {
      if (c > ' ') {
        _cmd = c; _val = 0; _hasDigits = false;
        char u = toupper(_cmd);
        if (u == 'F' || u == 'R' || u == 'T' || u == 'P') _parsingInt = true;
        else { runCommand(_cmd, -1); _cmd = 0; }
      }
    } else {
      if (c >= '0' && c <= '9') {
        _val = (_val * 10) + (c - '0');
        _hasDigits = true;
      } else {
        runCommand(_cmd, _hasDigits ? _val : -1);
        _parsingInt = false; _cmd = 0; _val = 0; _hasDigits = false;
        if (c > ' ') {
          char u = toupper(c);
          if (u == 'F' || u == 'R' || u == 'T' || u == 'P') { _cmd = c; _parsingInt = true; }
          else runCommand(c, -1);
        }
      }
    }
  }
}
