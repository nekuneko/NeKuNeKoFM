#include "NKFM_UILayout.h"

uint8_t curThemeIdx = 0;

static Adafruit_ST7789 *tft    = nullptr;
static NKFM_UILayout   *layout = nullptr;
static UIState          st;

static int8_t  _blPin       = -1;
static bool    _blActiveLow = true;
static int8_t  _tzOffset    = 1;
static uint8_t dirtyFlags   = D_FULL | D_ALL;
static bool    displayOn    = true;

static uint8_t       internalH      = 0;
static uint8_t       internalM      = 0;
static uint8_t       internalS      = 0;
static unsigned long internalTClock = 0;
static bool          internalSynced = false;
static unsigned long tScroll        = 0;

static NKFM_UILayout* const* _skins            = nullptr;
static uint8_t               _skinCount        = 0;
static uint8_t               _curSkin          = 0;
static unsigned long         _lastSkinChangeMs = 0;

int8_t UIGfx::findPresetIndex(const UIState& s, RADIO_FREQ f) {
  if (!s.presets) return -1;
  for (uint8_t i = 0; i < s.presetCount; i++) {
    if ((s.presets + i)->freq == f) return i;
  }
  return -1;
}

void UIGfx::drawSpeakerIcon(Adafruit_ST7789* d, int16_t x, int16_t y, bool muted, uint16_t col) {
  d->fillRect(x, y + 4, 4, 6, col);
  d->fillTriangle(x + 4, y + 4, x + 9, y, x + 9, y + 13, col);
  if (muted) {
    d->drawLine(x + 12, y + 3, x + 18, y + 10, C_WARN);
    d->drawLine(x + 18, y + 3, x + 12, y + 10, C_WARN);
  } else {
    d->drawFastVLine(x + 12, y + 4, 6, col);
    d->drawFastVLine(x + 15, y + 2, 10, col);
  }
}

void UIGfx::drawBrushIcon(Adafruit_ST7789* d, int16_t x, int16_t y, uint16_t handleCol, uint16_t tipCol) {
  d->drawLine(x + 1, y + 12, x + 7, y + 6, handleCol);
  d->drawLine(x + 2, y + 13, x + 8, y + 7, handleCol);
  d->drawLine(x + 1, y + 13, x + 7, y + 7, handleCol);
  d->fillRect(x + 7, y + 4, 3, 3, C_TEXT);
  d->fillTriangle(x + 13, y + 1, x + 8, y + 4, x + 10, y + 6, tipCol);
  d->fillCircle(x + 11, y + 3, 2, tipCol);
}

void UIGfx::drawStereoIcon(Adafruit_ST7789* d, int16_t x, int16_t y, bool stereo) {
  if (stereo) {
    d->drawCircle(x + 5, y + 6, 5, C_GOOD);
    d->drawCircle(x + 11, y + 6, 5, C_GOOD);
  } else {
    d->drawCircle(x + 8, y + 6, 5, C_MUTED);
  }
}

void UIGfx::drawMusicNoteIcon(Adafruit_ST7789* d, int16_t x, int16_t y, uint16_t col) {
  d->fillCircle(x + 3, y + 11, 3, col);
  d->fillCircle(x + 11, y + 9, 3, col);
  d->drawFastVLine(x + 5, y + 2, 9, col);
  d->drawFastVLine(x + 6, y + 2, 9, col);
  d->drawFastVLine(x + 13, y, 9, col);
  d->drawFastVLine(x + 14, y, 9, col);
  d->drawLine(x + 5, y + 2, x + 14, y, col);
  d->drawLine(x + 5, y + 3, x + 14, y + 1, col);
}

void UIGfx::drawFittedText(Adafruit_ST7789* d, const char *str, int16_t x, int16_t y, int16_t maxW, int16_t boxH, uint8_t maxSize, uint16_t color) {
  if (!str || *str == '\0') return;
  uint8_t len = strlen(str);
  uint8_t chosenSize = 1;
  for (uint8_t s = maxSize; s >= 1; s--) {
    if ((int16_t)(len * 6 * s) <= maxW) { chosenSize = s; break; }
  }
  int16_t drawY = y + max(0, (boxH - (8 * chosenSize)) / 2);
  d->setTextSize(chosenSize);
  d->setCursor(x, drawY);
  d->setTextColor(color);

  uint8_t maxChars = maxW / (6 * chosenSize);
  if (len <= maxChars) d->print(str);
  else {
    char temp[40];
    uint8_t copyLen = min((uint8_t)(sizeof(temp) - 1), maxChars);
    strncpy(temp, str, copyLen);
    *(temp + copyLen) = '\0';
    d->print(temp);
  }
}

void uiInit(Adafruit_ST7789 &display, NKFM_UILayout &activeLayout, uint8_t rotation, int8_t backlightPin, bool blActiveLow) {
  tft = &display;
  layout = &activeLayout;
  _blPin = backlightPin;
  _blActiveLow = blActiveLow;

  if (_blPin >= 0) {
    pinMode(_blPin, OUTPUT);
    digitalWrite(_blPin, _blActiveLow ? LOW : HIGH);
  }
  tft->init(layout->rawWidth(), layout->rawHeight());
  tft->invertDisplay(true);
  tft->setSPISpeed(24000000);
  tft->fillScreen(C_BG);
  tft->setRotation(rotation);
  tft->enableTearing(false);
  tft->setTextWrap(false);
  tft->cp437(true);
  dirtyFlags = D_FULL | D_ALL;
}

void uiSetLayout(NKFM_UILayout &activeLayout) {
  layout = &activeLayout;
  dirtyFlags = D_FULL | D_ALL;
}



void uiSetPresets(const NKFM_Preset* presetList, uint8_t count) {
  st.presets = presetList;
  st.presetCount = count;
}

void uiSetTimezoneOffset(int8_t hoursOffset) { _tzOffset = hoursOffset; }
uint8_t uiNextTheme(bool forward) {
  if (forward) curThemeIdx = (curThemeIdx + 1) % THEME_COUNT;
  else         curThemeIdx = (curThemeIdx + THEME_COUNT - 1) % THEME_COUNT;
  uiMarkDirty(D_FULL | D_ALL);
  return curThemeIdx;
}

uint8_t uiSetTheme(uint8_t idx) { curThemeIdx = idx % THEME_COUNT; uiMarkDirty(D_FULL | D_ALL); return curThemeIdx; }
void uiMarkDirty(uint8_t flags) { dirtyFlags |= flags; }
bool uiIsDisplayOn() { return displayOn; }

void uiToggleDisplay() {
  if (!tft) return;
  displayOn = !displayOn;
  if (_blPin >= 0) digitalWrite(_blPin, (displayOn == _blActiveLow) ? LOW : HIGH);
  tft->enableDisplay(displayOn);
}

void uiSetRotation(uint8_t rotation) {
  if (!tft) return;
  tft->setRotation(rotation);
  dirtyFlags = D_FULL | D_ALL;
}

void uiSetFrequency(RADIO_FREQ freq) { if (st.freq != freq) { st.freq = freq; dirtyFlags |= D_FREQ; } }
void uiSetSignal(uint8_t rssi, uint8_t snr, bool stereo) {
  if (st.rssi != rssi || st.snr != snr || st.stereo != stereo) {
    st.rssi = rssi; st.snr = snr; st.stereo = stereo; dirtyFlags |= D_SIGNAL;
  }
}

void uiSetServiceName(const char *name) {
  char buf[12];
  strncpy(buf, name, sizeof(buf) - 1);
  *(buf + sizeof(buf) - 1) = '\0';
  int len = strlen(buf);
  while (len > 0 && *(buf + len - 1) == ' ') *(buf + --len) = '\0';
  if (strcmp(buf, st.name) != 0) { strcpy(st.name, buf); dirtyFlags |= D_NAME; }
}

void uiSetRadioText(const char *text) {
  while (*text == ' ') text++;
  if (*text == '\0') return;
  String s(text); s.trim();
  if (s.length() == 0 || strcmp(s.c_str(), st.rt) == 0) return;
  bool wasEmpty = (*st.rt == '\0');
  strncpy(st.rt, s.c_str(), sizeof(st.rt) - 1);
  *(st.rt + sizeof(st.rt) - 1) = '\0';
  st.rtPixelW = strlen(st.rt) * 12;
  if (wasEmpty) st.scrollX = 230;
}

void uiClearRDS() {
  *st.name = '\0'; *st.rt = '\0'; st.pty = 0; st.scrollX = 230; st.rtPixelW = 0;
  dirtyFlags |= (D_FREQ | D_NAME | D_PTY | D_SIGNAL);
  if (tft && layout) layout->clearRTBox(tft);
}

void uiSetPTY(uint8_t pty) { if (pty > 31) pty = 0; if (st.pty != pty) { st.pty = pty; dirtyFlags |= D_PTY; } }
void uiSetVolume(uint8_t vol, bool muted) { if (st.vol != vol || st.muted != muted) { st.vol = vol; st.muted = muted; dirtyFlags |= D_VOL; } }
void uiSetBassBoost(bool active) { if (st.bassBoost != active) { st.bassBoost = active; dirtyFlags |= D_PTY; } }

void uiSetTime(uint8_t h, uint8_t m, uint8_t s, bool synced) {
  (void)s;
  uint8_t safeH = h % 24, safeM = m % 60;
  if (st.hour != safeH || st.min != safeM || st.synced != synced) {
    st.hour = safeH; st.min = safeM; st.synced = synced; dirtyFlags |= D_TIME;
  }
}

void uiSetBattery(float volts, uint8_t percent) {
  st.hasBat = true;
  if (abs(st.vbat - volts) >= 0.01f || st.batPct != percent) {
    st.vbat = volts; st.batPct = percent; dirtyFlags |= D_BATTERY;
  }
}

void uiInitClock(uint8_t h, uint8_t m, uint8_t s) {
  internalH = h % 24; internalM = m % 60; internalS = s % 60;
  internalTClock = millis(); internalSynced = false;
  uiSetTime(internalH, internalM, internalS, false);
}

void uiTickClock(unsigned long now) {
  if (now >= internalTClock && (now - internalTClock >= 1000)) {
    internalTClock = now;
    if (++internalS >= 60) {
      internalS = 0;
      if (++internalM >= 60) { internalM = 0; internalH = (internalH + 1) % 24; }
      uiSetTime(internalH, internalM, internalS, internalSynced);
    }
  }
}

void uiTickScroll(unsigned long now) {
  if (!tft || !layout || !displayOn || *st.rt == '\0' || (now - tScroll < 35)) return;
  tScroll = now;
  layout->tickScroll(tft, st);
}

void uiRender() {
  if (!tft || !layout || !displayOn || dirtyFlags == 0) return;
  bool wasFull = (dirtyFlags & D_FULL) != 0;
  if (wasFull) { layout->drawStatic(tft); dirtyFlags = D_ALL; }
  layout->render(tft, st, dirtyFlags);
  dirtyFlags = 0;
  if (wasFull) _lastSkinChangeMs = millis(); // <-- Reinicia el bloqueo AL TERMINAR de pintar
}

void DisplayServiceName(const char *name) { uiSetServiceName(name); }
void DisplayRT(const char *text)          { uiSetRadioText(text); }
void DisplayPTY(uint8_t pty)              { uiSetPTY(pty); }

void DisplayTime(uint8_t h, uint8_t m) {
  if (h >= 24 || m >= 60) return;
  uint8_t newH = (h + _tzOffset + 24) % 24;
  int diff = abs(((int)newH * 60 + (int)m) - ((int)internalH * 60 + (int)internalM));
  if (diff > 720) diff = 1440 - diff;
  if (!internalSynced || diff > 15) {
    internalH = newH; internalM = m; internalS = 0;
    internalTClock = millis(); internalSynced = true;
    uiSetTime(internalH, internalM, 0, true);
  }
}

void uiSetSkins(NKFM_UILayout* const* skinList, uint8_t count) {
  _skins     = skinList;
  _skinCount = count;
  _curSkin   = 0;
  if (_skins && _skinCount > 0 && _skins[0]) {
    uiSetLayout(*_skins[0]);
  }
}

uint8_t uiSetSkin(uint8_t idx) {
  if (!_skins || _skinCount == 0) return 0;
  _curSkin = idx % _skinCount;
  if (_skins[_curSkin]) uiSetLayout(*_skins[_curSkin]);
  return _curSkin;
}

uint8_t uiNextSkin(bool forward) {
  if (millis() - _lastSkinChangeMs < 350) return _curSkin;
  _lastSkinChangeMs = millis();

  if (!_skins || _skinCount == 0) return 0;
  if (forward) _curSkin = (_curSkin + 1) % _skinCount;
  else         _curSkin = (_curSkin + _skinCount - 1) % _skinCount;
  if (_skins[_curSkin]) uiSetLayout(*_skins[_curSkin]);
  return _curSkin;
}


uint8_t uiGetSkin() {
  return _curSkin;
}
