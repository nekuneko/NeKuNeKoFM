#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include <NeKuNeKoFM.h>
#include <NKFM_BoardConfig.h>
#include <NKFM_ReceiverSi4705.h>
#include <NKFM_UILayout_240x240.h>
#include <NKFM_UISerial.h>
#include <NKFM_IRSony.h>
#include <NKFM_ButtonPad.h>
#include <NKFM_BatteryMAX17048.h>

Adafruit_ST7789       tft(&SPI1, NKFM_PIN_TFT_CS, NKFM_PIN_TFT_DC, NKFM_PIN_TFT_RST);
NKFM_UILayout_240x240 layout;
NKFM_ReceiverSi4705   radio(Wire, NKFM_SI4705_ADDR, NKFM_PIN_RADIO_RST, NKFM_HAS_BOARD_LEDS);
NKFM_UISerial         console(&Serial);
NKFM_IRSony           remoteIR(NKFM_PIN_IR_IRQ);
NKFM_ButtonPad        buttons(NKFM_BTNS, NKFM_BTN_COUNT, 700);
NKFM_BatteryMAX17048  battery(Wire, NKFM_MAX17048_ADDR);

int TFT_ROTATION = NKFM_DEFAULT_ROTATION;
unsigned long tRDS = 0, tBat = 0, tSig = 0;

// Observador central: sincroniza pantalla TFT y log Serie ante cualquier evento
void onRadioEvent(NKFM_UIEvent ev, int val, const char* source) {
  console.logEvent(ev, val, source);

  switch (ev) {
    case NKFM_UIEvent::FREQ_CHANGED:
      uiClearRDS();
      uiSetFrequency((RADIO_FREQ)val);
      uiSetSignal(radio.getRSSI(), radio.getSNR(), radio.isStereo());
      break;
    case NKFM_UIEvent::VOL_CHANGED:
    case NKFM_UIEvent::MUTE_CHANGED:
      uiSetVolume(radio.getVolume(), radio.isMuted());
      break;
    case NKFM_UIEvent::SIGNAL_CHANGED:
    case NKFM_UIEvent::MONO_CHANGED:
      uiSetSignal(radio.getRSSI(), radio.getSNR(), radio.isStereo());
      break;
    case NKFM_UIEvent::BASS_CHANGED:
      uiSetBassBoost(val != 0);
      break;
    case NKFM_UIEvent::THEME_SET:
      uiSetTheme((uint8_t)val);
      break;
    case NKFM_UIEvent::THEME_NEXT:
      uiNextTheme(val != 0);
      break;
    case NKFM_UIEvent::ROTATION_SET:
      TFT_ROTATION = val & 0x03;
      uiSetRotation(TFT_ROTATION);
      break;
    case NKFM_UIEvent::ROTATION_NEXT:
      TFT_ROTATION = (TFT_ROTATION + 1) & 0x03;
      uiSetRotation(TFT_ROTATION);
      break;
  }
}

void onButtonAction(uint8_t idx, int pin, bool isLong) {
  const char* src = isLong ? "BTN LONG" : "BTN";
  console.logTag("[BTN]"); Serial.print(NKFM_BTN_NAMES[idx]);
  Serial.println(isLong ? F(" -> LONG") : F(" -> SHORT"));

  if      (pin == NKFM_BTN_MUTE) { if (isLong) uiToggleDisplay(); else radio.toggleMute(src); }
  else if (pin == NKFM_BTN_NEXT) { if (isLong) radio.seek(true, src); else radio.nextPreset(src); }
  else if (pin == NKFM_BTN_PREV) { if (isLong) radio.seek(false, src); else radio.prevPreset(src); }
  else if (pin == NKFM_BTN_VUP)  { if (isLong) radio.stepFrequency(+1, src); else radio.changeVolume(+1, src); }
  else if (pin == NKFM_BTN_VDN)  { if (isLong) radio.stepFrequency(-1, src); else radio.changeVolume(-1, src); }
  else if (pin == NKFM_BTN_INFO) { if (isLong) radio.toggleBassBoost(src); else onRadioEvent(NKFM_UIEvent::THEME_NEXT, 0, src); }
  else if (pin == NKFM_BTN_ROT)  { if (isLong) radio.toggleMono(src); else onRadioEvent(NKFM_UIEvent::ROTATION_NEXT, 0, src); }
}

void checkBattery(bool forceLog) {
  if (!battery.update()) return;
  uiSetBattery(battery.getVolts(), battery.getPercent());
  if (forceLog || battery.hasSignificantChange()) {
    console.logTag("[BAT]"); Serial.print(battery.getVolts(), 2);
    Serial.print(F(" V (")); Serial.print(battery.getPercent()); Serial.println(F("%)"));
  }
}

void reportBatCallback() { checkBattery(true); }

void setup() {
  Wire.begin();
  Serial.begin(115200);
  unsigned long wait = millis();
  while (!Serial && (millis() - wait < 2500));

  pinMode(NKFM_PIN_INT_GAUGE, INPUT_PULLUP);
  pinMode(NKFM_PIN_INT_IMU,   INPUT_PULLUP);
  pinMode(NKFM_PIN_IR_IRQ,    INPUT_PULLUP);

  buttons.begin(onButtonAction);

  uiInit(tft, layout, TFT_ROTATION, NKFM_PIN_TFT_BL, NKFM_TFT_BL_ACT_LOW);
  uiSetPresets(NKFM_DEFAULT_PRESETS, NKFM_DEFAULT_PRESET_COUNT);
  uiSetTimezoneOffset(NKFM_TIMEZONE_OFFSET);
  uiSetTheme(NKFM_DEFAULT_THEME);
  uiInitClock(atoi(__TIME__), atoi(__TIME__ + 3), atoi(__TIME__ + 6));

  radio.setPresets(NKFM_DEFAULT_PRESETS, NKFM_DEFAULT_PRESET_COUNT);
  radio.setEventCallback(onRadioEvent);
  radio.setRDSCallbacks(DisplayServiceName, DisplayRT, DisplayTime, DisplayPTY);
  radio.setVolume(NKFM_DEFAULT_VOLUME, "ARRANQUE");
  radio.begin();

  NKFM_SerialConfig sCfg;
  sCfg.tuner       = &radio;
  sCfg.maxThemes   = THEME_COUNT;
  sCfg.onUIEvent   = onRadioEvent;
  sCfg.onReportBat = reportBatCallback;
  sCfg.boardTitle  = "KITTEN DISPLAY 1.3 - NEKUNEKO FM";
  console.begin(sCfg);

  radio.selectPreset(NKFM_DEFAULT_PRESET, "ARRANQUE");
  checkBattery(true);
  uiRender();

  remoteIR.begin();
  console.printHelpMenu();
}

void loop() {
  unsigned long now = millis();

  console.poll();
  buttons.poll(now);

  uint32_t rawIR = 0;
  NKFM_Key key = remoteIR.poll(&rawIR);
  if (key != NKFM_Key::NONE) {
    radio.blinkOnLed(25);
    if (key == NKFM_Key::POWER)      uiToggleDisplay();
    else if (key == NKFM_Key::MUSIC) radio.tuneTo(9450, "IR");
    else                             NKFM_IRRemote::dispatchDefault(key, radio, onRadioEvent);
    remoteIR.flush();
  }


  if (now - tRDS >= 40)    { tRDS = now; radio.pollRDS(); }
  if (now - tSig >= 2000)  { tSig = now; radio.updateSignal(); uiSetSignal(radio.getRSSI(), radio.getSNR(), radio.isStereo()); }
  if (now - tBat >= 30000) { tBat = now; checkBattery(false); }

  uiTickClock(now);
  uiTickScroll(now);
  uiRender();
  if (key != NKFM_Key::NONE) remoteIR.flush();
}
