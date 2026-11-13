#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Si7021.h>

#include <NeKuNeKoFM.h>
#include <NKFM_BoardConfig.h>
#include <NKFM_ActionDispatcher.h>
#include <NKFM_ReceiverSi4705.h>
#include <skins/NKFM_Skins_240x135.h>
#include <skins/NKFM_UILayout_240x135_Sensors.h>
#include <NKFM_UISerial.h>
#include <NKFM_IRSony.h>
#include <NKFM_ButtonPadSX1509.h>
#include <NKFM_BatteryMAX17048.h>

Adafruit_ST7789               tft(&SPI, NKFM_PIN_TFT_CS, NKFM_PIN_TFT_DC, NKFM_PIN_TFT_RST);
NKFM_UILayout_240x135_Sensors layoutSensors;
NKFM_ReceiverSi4705           radio(Wire, NKFM_SI4705_ADDR, NKFM_PIN_RADIO_RST, NKFM_HAS_BOARD_LEDS, NKFM_USE_EXT_ANTENNA);
NKFM_UISerial                 console(&Serial);
NKFM_IRSony                   remoteIR(NKFM_PIN_IR_RX);
NKFM_ButtonPadSX1509          buttons(Wire, NKFM_SX1509_ADDR, NKFM_PIN_INT_SX1509, NKFM_BTNS, NKFM_BTN_COUNT, 700);
NKFM_BatteryMAX17048          battery(Wire, NKFM_MAX17048_ADDR);

Adafruit_BME280 bme;
Adafruit_Si7021 si7021;
bool hasBME           = false;
bool hasSi7021        = false;
bool sensorViewActive = false;

int TFT_ROTATION = NKFM_DEFAULT_ROTATION;
unsigned long tRDS = 0, tBat = 0, tSig = 0, tEnv = 0;

void initTPA6130(bool enable) {
  Wire.beginTransmission(0x60);
  Wire.write(0x01);
  Wire.write(enable ? 0xC0 : 0x01);
  Wire.endTransmission();

  if (enable) {
    Wire.beginTransmission(0x60);
    Wire.write(0x02);
    Wire.write(0x36);
    Wire.endTransmission();
  }
}

void setAudioAmpsEnabled(bool enable) {
  digitalWrite(NKFM_PIN_EN_SPEAKER, enable ? HIGH : LOW);
  digitalWrite(NKFM_PIN_EN_HP,      enable ? HIGH : LOW);
  if (enable) delay(10);
  initTPA6130(enable);
}

float compensateHumidity(float rawHum, float rawTemp, float realTemp) {
  if (rawHum <= 0.0f) rawHum = 1.0f;
  float esRaw   = 6.112f * exp((17.62f * rawTemp) / (243.12f + rawTemp));
  float eActual = (rawHum / 100.0f) * esRaw;
  float esReal  = 6.112f * exp((17.62f * realTemp) / (243.12f + realTemp));
  return constrain((eActual / esReal) * 100.0f, 0.0f, 100.0f);
}

void updateEnvSensors(bool forceLog = false) {
  NKFM_EnvData env;
  env.hasBME = hasBME;
  if (hasBME) {
    float rawT  = bme.readTemperature();
    float rawH  = bme.readHumidity();
    env.bmeTemp = rawT + NKFM_BME280_TEMP_OFFSET;
    env.bmeHum  = compensateHumidity(rawH, rawT, env.bmeTemp);
    env.bmePres = bme.readPressure() / 100.0F;
    if (forceLog) {
      console.logTag("[ENV-BME]");
      Serial.print(env.bmeTemp, 1); Serial.print(F(" C | "));
      Serial.print(env.bmeHum, 0);  Serial.print(F(" % | "));
      Serial.print(env.bmePres, 1); Serial.println(F(" hPa"));
    }
  }

  env.hasSi7021 = hasSi7021;
  if (hasSi7021) {
    float rawT = si7021.readTemperature();
    float rawH = si7021.readHumidity();
    env.siTemp = rawT + NKFM_SI7021_TEMP_OFFSET;
    env.siHum  = compensateHumidity(rawH, rawT, env.siTemp);
    if (forceLog) {
      console.logTag("[ENV-SI]");
      Serial.print(env.siTemp, 1); Serial.print(F(" C | "));
      Serial.print(env.siHum, 0);  Serial.println(F(" %"));
    }
  }

  layoutSensors.setEnvData(env);
}

void onRadioEvent(NKFM_UIEvent ev, int val, const char* source) {
  console.logEvent(ev, val, source);

  switch (ev) {
    case NKFM_UIEvent::FREQ_CHANGED:
      uiClearRDS();
      uiSetFrequency((RADIO_FREQ)val);
      uiSetSignal(radio.getRSSI(), radio.getSNR(), radio.isStereo());
      break;
    case NKFM_UIEvent::VOL_CHANGED:
      uiSetVolume(radio.getVolume(), radio.isMuted());
      break;
    case NKFM_UIEvent::MUTE_CHANGED:
      uiSetVolume(radio.getVolume(), radio.isMuted());
      setAudioAmpsEnabled(!radio.isMuted());
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
    case NKFM_UIEvent::SKIN_SET:
      sensorViewActive = false;
      uiSetSkin((uint8_t)val);
      break;
    case NKFM_UIEvent::SKIN_NEXT:
      sensorViewActive = false;
      uiNextSkin(val != 0);
      break;
    case NKFM_UIEvent::ROTATION_SET:
      TFT_ROTATION = val & 0x03;
      uiSetRotation(TFT_ROTATION);
      break;
    case NKFM_UIEvent::ROTATION_NEXT:
      TFT_ROTATION = (TFT_ROTATION + ((val != 0) ? 1 : 3)) & 0x03;
      uiSetRotation(TFT_ROTATION);
      break;
    case NKFM_UIEvent::DISPLAY_TOGGLE:
      uiToggleDisplay();
      break;
    case NKFM_UIEvent::VIEW_TOGGLE:
      sensorViewActive = !sensorViewActive;
      if (sensorViewActive) {
        updateEnvSensors(false);
        uiSetLayout(layoutSensors);
      } else {
        uiSetSkin(uiGetSkin());
      }
      uiRender();
      break;
    default:
      break;
  }
}

void onSX1509Button(uint8_t idx, const NKFM_ButtonDef& btn, bool isLong) {
  (void)idx;
  const char* src = isLong ? "SX1509 LONG" : "SX1509";
  console.logTag("[BTN]"); Serial.print(btn.label);
  Serial.println(isLong ? F(" -> LARGA") : F(" -> CORTA"));

  NKFM_ActionDispatcher::execute(isLong ? btn.longAct : btn.shortAct, radio, onRadioEvent, src);
}

void reportSensorsAndBattery(bool forceLog) {
  if (battery.update()) {
    uiSetBattery(battery.getVolts(), battery.getPercent());
    if (forceLog || battery.hasSignificantChange()) {
      console.logTag("[BAT]"); Serial.print(battery.getVolts(), 2);
      Serial.print(F(" V (")); Serial.print(battery.getPercent()); Serial.println(F("%)"));
    }
  }

  if (forceLog || sensorViewActive) {
    updateEnvSensors(forceLog);
  }
}

void reportCallback() { reportSensorsAndBattery(true); }

void setup() {
  Wire.begin(NKFM_PIN_I2C_SDA, NKFM_PIN_I2C_SCL);
  Serial.begin(115200);
  unsigned long wait = millis();
  while (!Serial && (millis() - wait < 2000));

  pinMode(NKFM_PIN_LED_BUILTIN, OUTPUT);
  pinMode(NKFM_PIN_LED_BT,      OUTPUT);
  digitalWrite(NKFM_PIN_LED_BUILTIN, LOW);
  digitalWrite(NKFM_PIN_LED_BT,      LOW);

  pinMode(NKFM_PIN_EN_SPEAKER, OUTPUT);
  pinMode(NKFM_PIN_EN_HP,      OUTPUT);
  setAudioAmpsEnabled(true);

  hasBME    = bme.begin(NKFM_BME280_ADDR, &Wire);
  hasSi7021 = si7021.begin();

  buttons.begin(onSX1509Button);

  uiInit(tft, *NKFM_SKINS[0], TFT_ROTATION, NKFM_PIN_TFT_BL, NKFM_TFT_BL_ACT_LOW);
  uiSetSkins(NKFM_SKINS, NKFM_SKIN_COUNT);
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
  sCfg.onReportBat = reportCallback;
  sCfg.boardTitle  = "TTGO ESP32-PICO-D4 - NEKUNEKO FM";
  console.begin(sCfg);

  radio.selectPreset(NKFM_DEFAULT_PRESET, "ARRANQUE");
  reportSensorsAndBattery(true);
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
    console.logTag("[IR]"); Serial.print(F("Sony 0x")); Serial.print(rawIR, HEX);
    Serial.print(F(" -> ")); Serial.println(NKFM_IRSony::getKeyName(key));

    if      (key == NKFM_Key::MUSIC) radio.tuneTo(9450, "IR");
    else if (key == NKFM_Key::MENU)  NKFM_ActionDispatcher::execute(NKFM_Action::SKIN_NEXT, radio, onRadioEvent, "IR");
    else                             NKFM_IRRemote::dispatchDefault(key, radio, onRadioEvent);
    remoteIR.flush();
  }

  if (now - tRDS >= 40)    { tRDS = now; radio.pollRDS(); }
  if (now - tSig >= 2000)  { tSig = now; radio.updateSignal(); uiSetSignal(radio.getRSSI(), radio.getSNR(), radio.isStereo()); }
  if (now - tBat >= 30000) { tBat = now; reportSensorsAndBattery(false); }
  if (sensorViewActive && (now - tEnv >= 2000)) { tEnv = now; updateEnvSensors(false); }

  uiTickClock(now);
  if (!sensorViewActive) uiTickScroll(now);
  uiRender();
}
