#pragma once
#include <Arduino.h>
#include "NKFM_RDSParser.h"

typedef uint16_t RADIO_FREQ;

struct NKFM_Preset {
  RADIO_FREQ  freq;
  const char* name;
};

enum class NKFM_UIEvent : uint8_t {
  FREQ_CHANGED,
  VOL_CHANGED,
  MUTE_CHANGED,
  SIGNAL_CHANGED,
  MONO_CHANGED,
  BASS_CHANGED,
  THEME_SET,
  THEME_NEXT,
  ROTATION_SET,
  ROTATION_NEXT,
  DISPLAY_TOGGLE
};



typedef void (*NKFM_EventCallback)(NKFM_UIEvent event, int value, const char* source);

// ============================================================================
// Contrato Base RX (Dueño único del estado, presets y notificaciones)
// ============================================================================
class NKFM_Receiver {
  protected:
    NKFM_RDSParser     _rds;
    RADIO_FREQ         _currentFreq = 9740;
    uint8_t            _volume      = 8;
    bool               _muted       = false;
    bool               _forceMono   = false;
    bool               _bassBoost   = false;
    uint8_t            _rssi        = 0;
    uint8_t            _snr         = 0;
    bool               _stereo      = false;

    const NKFM_Preset* _presets     = nullptr;
    uint8_t            _presetCount = 0;
    int8_t             _presetIdx   = -1;
    NKFM_EventCallback _onEvent     = nullptr;

    void notify(NKFM_UIEvent ev, int val = 0, const char* src = "") {
      if (_onEvent) _onEvent(ev, val, src);
    }

    void syncPresetIndex(RADIO_FREQ freq) {
      _presetIdx = -1;
      if (!_presets) return;
      for (uint8_t i = 0; i < _presetCount; i++) {
        if ((_presets + i)->freq == freq) {
          _presetIdx = (int8_t)i;
          break;
        }
      }
    }

    // Métodos puros de hardware que implementa cada driver concreto
    virtual void hwTuneTo(RADIO_FREQ freq) = 0;
    virtual RADIO_FREQ hwSeek(bool seekUp) = 0;
    virtual void hwStepFrequency(int8_t steps) = 0;
    virtual void hwSetVolume(uint8_t vol) = 0;
    virtual void hwSetMute(bool mute) = 0;
    virtual void hwSetMonoMode(bool forceMono) = 0;
    virtual void hwSetBassBoostMode(bool enable) = 0;

  public:
    virtual ~NKFM_Receiver() {}

    virtual void begin() = 0;
    virtual void updateSignal() = 0;
    virtual void pollRDS() = 0;
    virtual void printDebugStatus(Stream* out = nullptr) { (void)out; }

    // Configuración de presets y observadores
    void setPresets(const NKFM_Preset* list, uint8_t count) {
      _presets = list;
      _presetCount = count;
      syncPresetIndex(_currentFreq);
    }
    void setEventCallback(NKFM_EventCallback cb) { _onEvent = cb; }
    void setRDSCallbacks(RDSServiceCallback onPS, RDSTextCallback onRT, RDSTimeCallback onTime, RDSPtyCallback onPTY) {
      _rds.setCallbacks(onPS, onRT, onTime, onPTY);
    }

    // Acciones únicas de alto nivel (Botones, IR, Serie, Web)
    void tuneTo(RADIO_FREQ freq, const char* source = "SYS") {
      hwTuneTo(freq);
      _currentFreq = freq;
      syncPresetIndex(freq);
      notify(NKFM_UIEvent::FREQ_CHANGED, _currentFreq, source);
    }

    RADIO_FREQ seek(bool seekUp, const char* source = "SYS") {
      RADIO_FREQ f = hwSeek(seekUp);
      _currentFreq = f;
      syncPresetIndex(f);
      notify(NKFM_UIEvent::FREQ_CHANGED, _currentFreq, source);
      return f;
    }

    void stepFrequency(int8_t steps, const char* source = "SYS") {
      hwStepFrequency(steps);
      syncPresetIndex(_currentFreq);
      notify(NKFM_UIEvent::FREQ_CHANGED, _currentFreq, source);
    }

    void selectPreset(uint8_t idx, const char* source = "SYS") {
      if (!_presets || _presetCount == 0) return;
      uint8_t safeIdx = idx % _presetCount;
      _presetIdx = (int8_t)safeIdx;
      tuneTo((_presets + safeIdx)->freq, source);
    }

    void nextPreset(const char* source = "SYS") {
      if (!_presets || _presetCount == 0) return;
      int8_t next = (_presetIdx < (int8_t)_presetCount - 1) ? _presetIdx + 1 : 0;
      selectPreset((uint8_t)next, source);
    }

    void prevPreset(const char* source = "SYS") {
      if (!_presets || _presetCount == 0) return;
      int8_t prev = (_presetIdx > 0) ? _presetIdx - 1 : (int8_t)_presetCount - 1;
      selectPreset((uint8_t)prev, source);
    }

    void setVolume(uint8_t vol, const char* source = "SYS") {
      _volume = constrain(vol, (uint8_t)0, (uint8_t)15);
      hwSetVolume(_volume);
      notify(NKFM_UIEvent::VOL_CHANGED, _volume, source);
    }

    void changeVolume(int delta, const char* source = "SYS") {
      uint8_t nextVol = (uint8_t)constrain((int)_volume + delta, 0, 15);
      if (nextVol != _volume) setVolume(nextVol, source);
      else notify(NKFM_UIEvent::VOL_CHANGED, _volume, source);
    }

    void setMute(bool mute, const char* source = "SYS") {
      _muted = mute;
      hwSetMute(_muted);
      notify(NKFM_UIEvent::MUTE_CHANGED, _muted ? 1 : 0, source);
    }

    void toggleMute(const char* source = "SYS") { setMute(!_muted, source); }

    void setMonoMode(bool forceMono, const char* source = "SYS") {
      _forceMono = forceMono;
      hwSetMonoMode(_forceMono);
      notify(NKFM_UIEvent::MONO_CHANGED, _forceMono ? 1 : 0, source);
    }

    void toggleMono(const char* source = "SYS") { setMonoMode(!_forceMono, source); }

    void setBassBoostMode(bool enable, const char* source = "SYS") {
      _bassBoost = enable;
      hwSetBassBoostMode(_bassBoost);
      notify(NKFM_UIEvent::BASS_CHANGED, _bassBoost ? 1 : 0, source);
    }

    void toggleBassBoost(const char* source = "SYS") { setBassBoostMode(!_bassBoost, source); }

    // Getters de estado
    RADIO_FREQ         getFrequency() const   { return _currentFreq; }
    uint8_t            getVolume() const      { return _volume; }
    bool               isMuted() const        { return _muted; }
    bool               isMonoForced() const   { return _forceMono; }
    bool               isBassBoost() const    { return _bassBoost; }
    uint8_t            getRSSI() const        { return _rssi; }
    uint8_t            getSNR() const         { return _snr; }
    bool               isStereo() const       { return _stereo; }
    const NKFM_Preset* getPresets() const     { return _presets; }
    uint8_t            getPresetCount() const { return _presetCount; }
    int8_t             getPresetIndex() const { return _presetIdx; }
};

// ============================================================================
// Contrato Base TX (Futura estación transmisora FM: Si4713, QN8027...)
// ============================================================================
class NKFM_Station {
  protected:
    RADIO_FREQ _txFreq    = 10000;
    uint8_t    _txPower   = 115;
    bool       _txEnabled = false;
    uint8_t    _pty       = 10;
    uint16_t   _piCode    = 0xE200;

  public:
    virtual ~NKFM_Station() {}
    virtual void begin() = 0;
    virtual void setTxFrequency(RADIO_FREQ freq) = 0;
    virtual void setTxPower(uint8_t dBuV, uint8_t antCap = 0) = 0;
    virtual void setTxEnabled(bool enable) = 0;
    virtual void beginRDS(uint16_t piCode = 0xE200, uint8_t pty = 10) = 0;
    virtual void setRDSStationName(const char* psName) = 0;
    virtual void setRDSRadioText(const char* radioText) = 0;

    RADIO_FREQ getTxFrequency() const { return _txFreq; }
    uint8_t    getTxPower() const     { return _txPower; }
    bool       isTxEnabled() const    { return _txEnabled; }
};
