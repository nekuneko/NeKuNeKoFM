#pragma once
#include <Arduino.h>
#include "NKFM_RDSParser.h"

typedef uint16_t RADIO_FREQ; // Frecuencia en decenas de kHz (ej. 9740 = 97.40 MHz)

struct NKFM_Preset {
  RADIO_FREQ  freq;
  const char* name;
};

enum class NKFM_Action : uint8_t {
  NONE = 0,
  PRESET_NEXT,
  PRESET_PREV,
  SEEK_UP,
  SEEK_DOWN,
  FREQ_UP,
  FREQ_DOWN,
  VOL_UP,
  VOL_DOWN,
  MUTE_TOGGLE,
  MONO_TOGGLE,
  BASS_TOGGLE,
  THEME_NEXT,
  THEME_PREV,
  SKIN_NEXT,
  SKIN_PREV,
  ROTATION_NEXT,
  ROTATION_PREV,
  DISPLAY_TOGGLE,
  VIEW_TOGGLE,
  // Accesos directos a memorias 1..10 (botones físicos e IR)
  PRESET_1, PRESET_2, PRESET_3, PRESET_4, PRESET_5,
  PRESET_6, PRESET_7, PRESET_8, PRESET_9, PRESET_10
};

struct NKFM_ButtonDef {
  int         pin;
  const char* label;
  NKFM_Action shortAct;
  NKFM_Action longAct;
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
  SKIN_SET,
  SKIN_NEXT,
  ROTATION_SET,
  ROTATION_NEXT,
  DISPLAY_TOGGLE,
  VIEW_TOGGLE
};

typedef void (*NKFM_EventCallback)(NKFM_UIEvent event, int value, const char* source);

// ============================================================================
// CLASE BASE DE RADIO (Propiedades comunes RX y TX)
// ============================================================================
class NKFM_RadioBase {
  protected:
    RADIO_FREQ         _currentFreq = 8750;
    const NKFM_Preset* _presets     = nullptr;
    uint8_t            _presetCount = 0;
    int                _presetIdx   = 0;
    NKFM_EventCallback _onEvent     = nullptr;

    void notify(NKFM_UIEvent ev, int val, const char* source) {
      if (_onEvent) _onEvent(ev, val, source);
    }

    int findPresetIndex(RADIO_FREQ freq) const {
      if (!_presets) return -1;
      for (uint8_t i = 0; i < _presetCount; i++) {
        if (_presets[i].freq == freq) return (int)i;
      }
      return -1;
    }

  public:
    virtual ~NKFM_RadioBase() {}
    virtual void begin() = 0;

    void setEventCallback(NKFM_EventCallback cb) { _onEvent = cb; }

    void setPresets(const NKFM_Preset* list, uint8_t count) {
      _presets     = list;
      _presetCount = count;
    }

    RADIO_FREQ         getFrequency() const   { return _currentFreq; }
    const NKFM_Preset* getPresets() const     { return _presets; }
    uint8_t            getPresetCount() const { return _presetCount; }
    int                getPresetIndex() const { return _presetIdx; }

    const char* getPresetName(uint8_t idx) const {
      if (!_presets || idx >= _presetCount) return "";
      return _presets[idx].name;
    }

    const char* getCurrentPresetName() const {
      int idx = findPresetIndex(_currentFreq);
      return (idx >= 0) ? _presets[idx].name : "";
    }
};

// ============================================================================
// CONTRATO PARA RECEPTORES FM (RX: Si4705, RDA5807...)
// ============================================================================
class NKFM_Receiver : public NKFM_RadioBase {
  protected:
    NKFM_RDSParser _rds;
    uint8_t        _volume    = 10;
    bool           _muted     = false;
    bool           _forceMono = false;
    bool           _bassBoost = false;
    uint8_t        _rssi      = 0;
    uint8_t        _snr       = 0;
    bool           _stereo    = false;

    virtual void hwTuneTo(RADIO_FREQ freq) = 0;
    virtual RADIO_FREQ hwSeek(bool seekUp) = 0;
    virtual void hwStepFrequency(int8_t steps) = 0;
    virtual void hwSetVolume(uint8_t vol) = 0;
    virtual void hwSetMute(bool mute) = 0;
    virtual void hwSetMonoMode(bool forceMono) = 0;
    virtual void hwSetBassBoostMode(bool enable) = 0;

  public:
    virtual void updateSignal() = 0;
    virtual void pollRDS() = 0;
    virtual void printDebugStatus(Stream* out = nullptr) { (void)out; }

    void setRDSCallbacks(NKFM_RDSServiceNameCb psCb, NKFM_RDSTextCb rtCb, NKFM_RDSTimeCb timeCb, NKFM_RDSPtyCb ptyCb) {
      _rds.attachServiceNameCallback(psCb);
      _rds.attachTextCallback(rtCb);
      _rds.attachTimeCallback(timeCb);
      _rds.attachPTYCallback(ptyCb);
    }

    const NKFM_RDSParser& getRDS() const { return _rds; }

    void tuneTo(RADIO_FREQ freq, const char* source = "SYS") {
      if (freq < 8750 || freq > 10800) return;
      _currentFreq = freq;
      int idx = findPresetIndex(freq);
      if (idx >= 0) _presetIdx = idx;
      hwTuneTo(freq);
      notify(NKFM_UIEvent::FREQ_CHANGED, (int)_currentFreq, source);
    }

    void selectPreset(uint8_t index, const char* source = "PRESET") {
      if (!_presets || index >= _presetCount) return;
      _presetIdx = index;
      tuneTo(_presets[index].freq, source);
    }

    void nextPreset(const char* source = "PRESET") {
      if (!_presets || _presetCount == 0) return;
      _presetIdx = (_presetIdx + 1) % _presetCount;
      tuneTo(_presets[_presetIdx].freq, source);
    }

    void prevPreset(const char* source = "PRESET") {
      if (!_presets || _presetCount == 0) return;
      _presetIdx = (_presetIdx > 0) ? _presetIdx - 1 : (int)_presetCount - 1;
      tuneTo(_presets[_presetIdx].freq, source);
    }

    void seek(bool seekUp, const char* source = "SEEK") {
      _currentFreq = hwSeek(seekUp);
      int idx = findPresetIndex(_currentFreq);
      if (idx >= 0) _presetIdx = idx;
      notify(NKFM_UIEvent::FREQ_CHANGED, (int)_currentFreq, source);
    }

    void stepFrequency(int8_t steps, const char* source = "STEP") {
      hwStepFrequency(steps);
      int idx = findPresetIndex(_currentFreq);
      if (idx >= 0) _presetIdx = idx;
      notify(NKFM_UIEvent::FREQ_CHANGED, (int)_currentFreq, source);
    }

    void setVolume(uint8_t vol, const char* source = "VOL") {
      _volume = constrain(vol, (uint8_t)0, (uint8_t)15);
      hwSetVolume(_volume);
      if (_muted && _volume > 0) {
        _muted = false;
        hwSetMute(false);
        notify(NKFM_UIEvent::MUTE_CHANGED, 0, source);
      }
      notify(NKFM_UIEvent::VOL_CHANGED, (int)_volume, source);
    }

    void changeVolume(int8_t delta, const char* source = "VOL") {
      int next = constrain((int)_volume + delta, 0, 15);
      setVolume((uint8_t)next, source);
    }

    void setMute(bool mute, const char* source = "MUTE") {
      _muted = mute;
      hwSetMute(_muted);
      notify(NKFM_UIEvent::MUTE_CHANGED, _muted ? 1 : 0, source);
    }

    void toggleMute(const char* source = "MUTE") { setMute(!_muted, source); }

    void setMono(bool forceMono, const char* source = "AUDIO") {
      _forceMono = forceMono;
      hwSetMonoMode(_forceMono);
      notify(NKFM_UIEvent::MONO_CHANGED, _forceMono ? 1 : 0, source);
    }

    void toggleMono(const char* source = "AUDIO") { setMono(!_forceMono, source); }

    void setBassBoost(bool enable, const char* source = "AUDIO") {
      _bassBoost = enable;
      hwSetBassBoostMode(_bassBoost);
      notify(NKFM_UIEvent::BASS_CHANGED, _bassBoost ? 1 : 0, source);
    }

    void toggleBassBoost(const char* source = "AUDIO") { setBassBoost(!_bassBoost, source); }

    uint8_t getVolume() const    { return _volume; }
    bool    isMuted() const      { return _muted; }
    bool    isForcedMono() const { return _forceMono; }
    bool    isBassBoost() const  { return _bassBoost; }
    uint8_t getRSSI() const      { return _rssi; }
    uint8_t getSNR() const       { return _snr; }
    bool    isStereo() const     { return _stereo && !_forceMono; }
};

// ============================================================================
// CONTRATO PARA TRANSMISORES FM (TX: Si4713, QN8027...)
// ============================================================================
class NKFM_Station : public NKFM_RadioBase {
  protected:
    uint8_t _txPower = 115; // dBuV (88..115)
    char    _ps[9]   = "NEKUNEKO";
    char    _rt[65]  = "NeKuNeKo FM Station";

    virtual void hwSetTxFrequency(RADIO_FREQ freq) = 0;
    virtual void hwSetTxPower(uint8_t dBuV) = 0;
    virtual void hwSetTxRDS(const char* ps, const char* rt) = 0;

  public:
    void setFrequency(RADIO_FREQ freq, const char* source = "TX") {
      if (freq < 8750 || freq > 10800) return;
      _currentFreq = freq;
      hwSetTxFrequency(freq);
      notify(NKFM_UIEvent::FREQ_CHANGED, (int)_currentFreq, source);
    }

    void setTxPower(uint8_t dBuV, const char* source = "TX") {
      _txPower = constrain(dBuV, (uint8_t)88, (uint8_t)115);
      hwSetTxPower(_txPower);
      notify(NKFM_UIEvent::SIGNAL_CHANGED, (int)_txPower, source);
    }

    void setRDSInfo(const char* ps, const char* rt) {
      if (ps) { strncpy(_ps, ps, 8); _ps[8] = '\0'; }
      if (rt) { strncpy(_rt, rt, 64); _rt[64] = '\0'; }
      hwSetTxRDS(_ps, _rt);
    }

    uint8_t     getTxPower() const { return _txPower; }
    const char* getTxPS() const      { return _ps; }
    const char* getTxRT() const      { return _rt; }
};
