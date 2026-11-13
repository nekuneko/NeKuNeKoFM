#pragma once

#include <Arduino.h>
#include "NeKuNeKoFM.h"

typedef void (*BatteryReportCallback)();

struct NKFM_SerialConfig {
  NKFM_Receiver*        tuner        = nullptr;
  uint8_t               maxThemes    = 9;
  NKFM_EventCallback    onUIEvent    = nullptr;
  BatteryReportCallback onReportBat  = nullptr;
  const char*           boardTitle   = "NEKUNEKO FM RADIO CONSOLE";
};

class NKFM_UISerial {
  private:
    Stream*           _port = nullptr;
    NKFM_SerialConfig _cfg;

    bool    _parsingInt = false;
    char    _cmd        = 0;
    int16_t _val        = 0;
    bool    _hasDigits  = false;

    void notifyUI(NKFM_UIEvent ev, int val = 0) const {
      if (_cfg.onUIEvent) _cfg.onUIEvent(ev, val, "SERIE");
    }
    void runCommand(char cmd, int16_t value);

  public:
    NKFM_UISerial(Stream* port = nullptr);

    void setPort(Stream* port) { _port = port; }
    void begin(const NKFM_SerialConfig& cfg, Stream* port = nullptr);
    void poll();

    void printHelpMenu() const;
    void printPresetList() const;
    void logEvent(NKFM_UIEvent event, int value, const char* source) const;
    void logRDS(const char* tag, const char* msg) const;

    void logTag(const char* tag) const;
    void logBtnSnapshot(const char* fase, const int* pins, uint8_t count) const;
};
