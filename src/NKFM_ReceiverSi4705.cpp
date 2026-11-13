#include "NKFM_ReceiverSi4705.h"

NKFM_ReceiverSi4705::NKFM_ReceiverSi4705(TwoWire& wirePort, uint8_t i2cAddr, int8_t rstPin,
                                         bool hasBoardLeds, bool useExtAntenna)
  : _wire(&wirePort), _addr(i2cAddr), _rstPin(rstPin),
    _hasBoardLeds(hasBoardLeds), _useExtAntenna(useExtAntenna) {}


void NKFM_ReceiverSi4705::waitCTS() {
  unsigned long t0 = millis();
  while (millis() - t0 < 100) {
    _wire->requestFrom(_addr, (uint8_t)1);
    if (_wire->available() && (_wire->read() & 0x80)) return;
    delay(1);
  }
}

void NKFM_ReceiverSi4705::sendCmd(uint8_t cmd, const uint8_t* args, uint8_t len) {
  _wire->beginTransmission(_addr);
  _wire->write(cmd);
  for (uint8_t i = 0; i < len; i++) _wire->write(*(args + i));
  _wire->endTransmission();
  waitCTS();
}

void NKFM_ReceiverSi4705::setProp(uint16_t prop, uint16_t val) {
  uint8_t args[5] = {
    0x00,
    (uint8_t)(prop >> 8), (uint8_t)(prop & 0xFF),
    (uint8_t)(val >> 8),  (uint8_t)(val & 0xFF)
  };
  sendCmd(0x12, args, 5);
}

void NKFM_ReceiverSi4705::syncBoardLeds() {
  if (!_hasBoardLeds) return;
  uint8_t ctl = 0x06; // Habilita IO2 (bit 2 = Rojo ON) e IO1 (bit 1 = Azul MUTE) como salidas
  uint8_t set = (_onLedState ? 0x04 : 0x00) | (_muteLedState ? 0x02 : 0x00);
  sendCmd(0x80, &ctl, 1);
  sendCmd(0x81, &set, 1);
}

void NKFM_ReceiverSi4705::setOnLed(bool on) {
  _onLedState = on;
  syncBoardLeds();
}

void NKFM_ReceiverSi4705::setMuteLed(bool on) {
  _muteLedState = on;
  syncBoardLeds();
}

void NKFM_ReceiverSi4705::blinkOnLed(uint16_t durationMs) {
  if (!_hasBoardLeds) return;
  bool prev = _onLedState;
  setOnLed(!prev);
  delay(durationMs);
  setOnLed(prev);
}

void NKFM_ReceiverSi4705::begin() {
  if (_rstPin >= 0) {
    pinMode(_rstPin, OUTPUT);
    digitalWrite(_rstPin, LOW);  delay(500);
    digitalWrite(_rstPin, HIGH); delay(500);
  }

  uint8_t pwrArgs[2] = { 0x10, 0x05 }; // 0x10: XOSCEN=1, CTSIEN=0, GPO2OEN=0 (libera IO2 para LED Rojo)
  sendCmd(0x01, pwrArgs, 2);
  delay(150);

  setProp(0x0201, 32768);  // REFCLK_FREQ = 32.768 kHz (cristal externo)
  setProp(0x0202, 0x0001); // REFCLK_PRESCALE = 1

  setProp(0x1100, 0x0001); // 50us Europa
  setProp(0x1107, _useExtAntenna ? 0x0001 : 0x0000); // 0x0001 = ANT externa, 0x0000 = FMI auriculares
  setProp(0x1400, 8750);
  setProp(0x1401, 10800);
  setProp(0x1402, 10);
  setProp(0x1302, 0x0000);
  setProp(0x1403, 0x0003); // Seek SNR = 3 dB
  setProp(0x1404, 0x000A); // Seek RSSI = 10 dBuV

  setProp(0x1800, 18);
  setProp(0x1801, 10);
  setProp(0x1804, 6);
  setProp(0x1805, 3);

  setProp(0x1500, 0x0003);
  setProp(0x1501, 0x0001);
  setProp(0x1502, 0x5501);

  hwSetVolume(_volume);
  setProp(0x4001, _muted ? 0x0003 : 0x0000);
  _muteLedState = _muted;
  syncBoardLeds(); // Arranca con IO2 (LED Rojo ON) apagado (_onLedState = false)
}


void NKFM_ReceiverSi4705::flushHardwareRDS() {
  uint8_t flushArg = 0x02;
  sendCmd(0x24, &flushArg, 1);
  _wire->requestFrom(_addr, (uint8_t)13);
  while (_wire->available()) (void)_wire->read();
  _rds.reset();
}

void NKFM_ReceiverSi4705::hwTuneTo(RADIO_FREQ freq) {
  flushHardwareRDS();

  uint8_t args[4] = {
    0x00,
    (uint8_t)((freq >> 8) & 0xFF),
    (uint8_t)(freq & 0xFF),
    0x00
  };
  sendCmd(0x20, args, 4);
  delay(50);
  waitCTS();

  _currentFreq = freq;
  updateSignal();
}

RADIO_FREQ NKFM_ReceiverSi4705::hwSeek(bool seekUp) {
  flushHardwareRDS();

  uint8_t ack = 0x01;
  sendCmd(0x22, &ack, 1);
  delay(10);

  uint8_t arg = seekUp ? 0x0C : 0x04;
  sendCmd(0x21, &arg, 1);

  unsigned long tStart = millis();
  while (millis() - tStart < 3500) {
    delay(40);
    sendCmd(0x14, nullptr, 0);
    _wire->requestFrom(_addr, (uint8_t)1);
    if (_wire->available() && (_wire->read() & 0x01)) break;
  }

  sendCmd(0x22, &ack, 1);
  _wire->requestFrom(_addr, (uint8_t)8);
  uint8_t resp[8] = {0};
  for (uint8_t i = 0; i < 8 && _wire->available(); i++) *(resp + i) = _wire->read();

  _currentFreq = ((uint16_t)(*(resp + 2)) << 8) | (*(resp + 3));
  updateSignal();
  return _currentFreq;
}

RADIO_FREQ NKFM_ReceiverSi4705::readTunedFreq() {
  uint8_t arg = 0x00;
  sendCmd(0x22, &arg, 1);
  _wire->requestFrom(_addr, (uint8_t)8);
  uint8_t resp[8] = {0};
  for (uint8_t i = 0; i < 8 && _wire->available(); i++) *(resp + i) = _wire->read();
  _currentFreq = ((uint16_t)(*(resp + 2)) << 8) | (*(resp + 3));
  return _currentFreq;
}

void NKFM_ReceiverSi4705::hwStepFrequency(int8_t steps) {
  RADIO_FREQ cur = readTunedFreq();
  if (cur < 8750 || cur > 10800) cur = _currentFreq;

  int32_t nextFreq = (int32_t)cur + ((int32_t)steps * 10);
  if (nextFreq > 10800) nextFreq = 8750;
  if (nextFreq < 8750)  nextFreq = 10800;

  hwTuneTo((RADIO_FREQ)nextFreq);
}

void NKFM_ReceiverSi4705::hwSetVolume(uint8_t vol) {
  setProp(0x4000, map(vol, 0, 15, 0, 63));
}

void NKFM_ReceiverSi4705::hwSetMute(bool mute) {
  setProp(0x4001, mute ? 0x0003 : 0x0000);
  _muteLedState = mute;
  syncBoardLeds();
}

void NKFM_ReceiverSi4705::hwSetMonoMode(bool forceMono) {
  setProp(0x1800, forceMono ? 127 : 18);
  setProp(0x1801, forceMono ? 127 : 10);
  updateSignal();
}

void NKFM_ReceiverSi4705::hwSetBassBoostMode(bool enable) {
  setProp(0x1100, enable ? 0x0002 : 0x0001);
}

void NKFM_ReceiverSi4705::updateSignal() {
  uint8_t arg = 0x00;
  sendCmd(0x23, &arg, 1);
  _wire->requestFrom(_addr, (uint8_t)8);
  uint8_t resp[8] = {0};
  for (uint8_t i = 0; i < 8 && _wire->available(); i++) *(resp + i) = _wire->read();

  _stereo = ((*(resp + 3)) & 0x80) != 0;
  _rssi   = *(resp + 4);
  _snr    = *(resp + 5);
}

void NKFM_ReceiverSi4705::pollRDS() {
  uint8_t arg = 0x01;
  sendCmd(0x24, &arg, 1);
  _wire->requestFrom(_addr, (uint8_t)13);

  uint8_t r[13] = {0};
  for (uint8_t i = 0; i < 13 && _wire->available(); i++) *(r + i) = _wire->read();

  if (*(r + 3) == 0) return;

  uint8_t errB = (*(r + 12) >> 4) & 0x03;
  uint8_t errC = (*(r + 12) >> 2) & 0x03;
  uint8_t errD = (*(r + 12)) & 0x03;

  uint16_t b1 = ((uint16_t)(*(r + 4))  << 8) | (*(r + 5));
  uint16_t b2 = ((uint16_t)(*(r + 6))  << 8) | (*(r + 7));
  uint16_t b3 = ((uint16_t)(*(r + 8))  << 8) | (*(r + 9));
  uint16_t b4 = ((uint16_t)(*(r + 10)) << 8) | (*(r + 11));

  _rds.feedGroup(b1, b2, b3, b4, errB, errC, errD);
}

void NKFM_ReceiverSi4705::printDebugStatus(Stream* out) {
  if (!out) return;
  uint8_t arg = 0x00;
  sendCmd(0x23, &arg, 1);
  _wire->requestFrom(_addr, (uint8_t)8);
  out->print(F("[DEBUG] RSQ Raw: "));
  while (_wire->available()) {
    out->print(F("0x")); out->print(_wire->read(), HEX); out->print(' ');
  }
  out->println();
}
