#pragma once
#include <Wire.h>
#include "NeKuNeKoFM.h"

class NKFM_ReceiverSi4705 : public NKFM_Receiver {
  private:
    TwoWire* _wire;
    uint8_t  _addr;
    int8_t   _rstPin;
    bool     _hasBoardLeds;
    bool     _useExtAntenna;
    bool     _onLedState   = false; // IO2 (LED Rojo ON) apagado por defecto para ahorrar batería
    bool     _muteLedState = false; // IO1 (LED Azul MUTE)

    void waitCTS();
    void sendCmd(uint8_t cmd, const uint8_t* args = nullptr, uint8_t len = 0);
    void setProp(uint16_t prop, uint16_t val);
    void syncBoardLeds();
    void flushHardwareRDS();
    RADIO_FREQ readTunedFreq();

  protected:
    void hwTuneTo(RADIO_FREQ freq) override;
    RADIO_FREQ hwSeek(bool seekUp) override;
    void hwStepFrequency(int8_t steps) override;
    void hwSetVolume(uint8_t vol) override;
    void hwSetMute(bool mute) override;
    void hwSetMonoMode(bool forceMono) override;
    void hwSetBassBoostMode(bool enable) override;

  public:
    NKFM_ReceiverSi4705(TwoWire& wirePort = Wire, uint8_t i2cAddr = 0x63, int8_t rstPin = -1,
                        bool hasBoardLeds = true, bool useExtAntenna = false);


    void begin() override;
    void updateSignal() override;
    void pollRDS() override;
    void printDebugStatus(Stream* out = nullptr) override;

    // Control independiente de los LEDs GPIO del Si4705 (IO2 = Rojo ON, IO1 = Azul MUTE)
    void setOnLed(bool on);
    void setMuteLed(bool on);
    void blinkOnLed(uint16_t durationMs = 25);
    bool isOnLedEnabled() const { return _onLedState; }
};
