#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "NeKuNeKoFM.h"

#define TFT_SWAP_RB 0

struct UITheme {
  uint16_t bg, card, border, accent, accent2, gold, text, muted, dark, warn, good;
};

constexpr UITheme THEMES[] = {
  { 0x0000, 0x0863, 0x3A50, 0x3E7F, 0x05BF,  0xFFE0, 0xFFFF, 0xBDF7, 0x18E5, 0xFD20, 0x07FF }, // 0: OKABE-ITO
  { 0x2104, 0x0000, 0xDEFB, 0xFFE0, 0x7FEF,  0xFFFF, 0xFFFF, 0xBDF7, 0x4208, 0x37FF, 0xFD4A }, // 1: PURE INK B&W
  { 0x0000, 0x0842, 0x39E9, 0x05FF, 0x07FF,  0xFFE0, 0xFFFF, 0xCE79, 0x18E4, 0xFD20, 0xFFFF }, // 2: NEKU VISION PRO
  { 0x0086, 0x0000, 0xD6FF, 0xFFE0, 0x07FF,  0xFFFF, 0xFFFF, 0xAD7D, 0x1129, 0xF800, 0x3E7F }, // 3: PURE INK BLUE
  { 0xDEFB, 0xFFFF, 0x2104, 0x001F, 0x8010,  0x0000, 0x0000, 0x4208, 0xBDF7, 0xC800, 0x02B5 }, // 4: PURE PAPER B&W
  { 0x0000, 0x0000, 0xFFFF, 0x05BF, 0xFFE0,  0xFFFF, 0xFFFF, 0xCE79, 0x2945, 0xFD20, 0x07FF }, // 5: HIGH CONTRAST
  { 0x0146, 0x0249, 0x04D1, 0x2FF9, 0xFFE0,  0xFFFF, 0xE7FE, 0x7E18, 0x032C, 0xFD20, 0x07FF }, // 6: OCEAN TEAL
  { 0x08A6, 0x1149, 0x32B1, 0xFB8C, 0x3E7F,  0xFFFF, 0xFFFF, 0x9538, 0x21EC, 0xFBE0, 0x07FF }, // 7: CORAL & MIDNIGHT
  { 0x0000, 0x2125, 0x52AA, 0xFB40, 0x07FF,  0xFFFF, 0xFFFF, 0xBDF7, 0x3186, 0xF980, 0xFFE0 }  // 8: OBSIDIAN OLED
};

constexpr uint8_t THEME_COUNT = sizeof(THEMES) / sizeof(*THEMES);
extern uint8_t curThemeIdx;

#if TFT_SWAP_RB
  #define FIX_COLOR(c) ((((c) & 0x001F) << 11) | ((c) & 0x07E0) | (((c) & 0xF800) >> 11))
#else
  #define FIX_COLOR(c) (c)
#endif

#define C_BG      FIX_COLOR(THEMES[curThemeIdx].bg)
#define C_CARD    FIX_COLOR(THEMES[curThemeIdx].card)
#define C_BORDER  FIX_COLOR(THEMES[curThemeIdx].border)
#define C_ACCENT  FIX_COLOR(THEMES[curThemeIdx].accent)
#define C_GOLD    FIX_COLOR(THEMES[curThemeIdx].gold)
#define C_TEXT    FIX_COLOR(THEMES[curThemeIdx].text)
#define C_MUTED   FIX_COLOR(THEMES[curThemeIdx].muted)
#define C_DARK    FIX_COLOR(THEMES[curThemeIdx].dark)
#define C_WARN    FIX_COLOR(THEMES[curThemeIdx].warn)
#define C_ACCENT2 FIX_COLOR(THEMES[curThemeIdx].accent2)
#define C_GOOD    FIX_COLOR(THEMES[curThemeIdx].good)

enum DirtyMask : uint8_t {
  D_FULL    = 0x01,
  D_FREQ    = 0x02,
  D_SIGNAL  = 0x04,
  D_NAME    = 0x08,
  D_PTY     = 0x10,
  D_VOL     = 0x20,
  D_TIME    = 0x40,
  D_BATTERY = 0x80,
  D_ALL     = 0xFE
};

struct UIState {
  char       name[12]   = "";
  char       rt[68]     = "";
  uint8_t    hour       = 0;
  uint8_t    min        = 0;
  RADIO_FREQ freq       = 9740;
  uint8_t    pty        = 0;
  uint8_t    vol        = 8;
  bool       muted      = false;
  bool       synced     = false;
  uint8_t    rssi       = 0;
  uint8_t    snr        = 0;
  bool       stereo     = false;
  float      vbat       = 0.0f;
  uint8_t    batPct     = 0;
  bool       hasBat     = false;
  bool       bassBoost  = false;
  int16_t    scrollX    = 230;
  uint16_t   rtPixelW   = 0;

  const NKFM_Preset* presets = nullptr;
  uint8_t presetCount        = 0;
};

class NKFM_UILayout {
  public:
    virtual ~NKFM_UILayout() {}
    virtual uint16_t rawWidth() const = 0;
    virtual uint16_t rawHeight() const = 0;
    virtual void drawStatic(Adafruit_ST7789* tft) = 0;
    virtual void render(Adafruit_ST7789* tft, const UIState& st, uint8_t dirty) = 0;
    virtual void tickScroll(Adafruit_ST7789* tft, UIState& st) = 0;
    virtual void clearRTBox(Adafruit_ST7789* tft) = 0;
};

namespace UIGfx {
  int8_t findPresetIndex(const UIState& st, RADIO_FREQ f);
  void drawSpeakerIcon(Adafruit_ST7789* tft, int16_t x, int16_t y, bool muted, uint16_t col);
  void drawBrushIcon(Adafruit_ST7789* tft, int16_t x, int16_t y, uint16_t handleCol, uint16_t tipCol);
  void drawStereoIcon(Adafruit_ST7789* tft, int16_t x, int16_t y, bool stereo);
  void drawMusicNoteIcon(Adafruit_ST7789* tft, int16_t x, int16_t y, uint16_t col);
  void drawFittedText(Adafruit_ST7789* tft, const char *str, int16_t x, int16_t y, int16_t maxW, int16_t boxH, uint8_t maxSize, uint16_t color);
}

void uiInit(Adafruit_ST7789 &display, NKFM_UILayout &layout, uint8_t rotation, int8_t backlightPin = -1, bool blActiveLow = true);
void uiSetLayout(NKFM_UILayout &layout);
void uiSetPresets(const NKFM_Preset* presetList, uint8_t count);
void uiSetTimezoneOffset(int8_t hoursOffset);

uint8_t uiSetTheme(uint8_t idx);
uint8_t uiNextTheme(bool forward = true);

void uiSetSkins(NKFM_UILayout* const* skinList, uint8_t count);
uint8_t uiSetSkin(uint8_t idx);
uint8_t uiNextSkin(bool forward = true);
uint8_t uiGetSkin();

void uiSetRotation(uint8_t rotation);
void uiMarkDirty(uint8_t flags);
void uiToggleDisplay();
bool uiIsDisplayOn();

void uiSetFrequency(RADIO_FREQ freq);
void uiSetSignal(uint8_t rssi, uint8_t snr, bool stereo);
void uiSetServiceName(const char *name);
void uiSetRadioText(const char *text);
void uiClearRDS();
void uiSetPTY(uint8_t pty);
void uiSetVolume(uint8_t vol, bool muted);
void uiSetTime(uint8_t h, uint8_t m, uint8_t s, bool synced);
void uiSetBattery(float volts, uint8_t percent);
void uiSetBassBoost(bool active);

void uiInitClock(uint8_t h, uint8_t m, uint8_t s);
void uiTickClock(unsigned long now);
void uiTickScroll(unsigned long now);
void uiRender();

void DisplayServiceName(const char *name);
void DisplayRT(const char *text);
void DisplayPTY(uint8_t pty);
void DisplayTime(uint8_t h, uint8_t m);
