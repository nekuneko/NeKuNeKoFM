#pragma once
#include <Arduino.h>

static constexpr const char* PTYeurope[32] = {
    "Sin definir",      // 0: None / No programme type
    "Noticias",         // 1: News
    "Actualidad",       // 2: Current Affairs
    "Información",      // 3: Information
    "Deportes",         // 4: Sport
    "Educación",        // 5: Education
    "Drama",            // 6: Drama
    "Cultura",          // 7: Culture
    "Ciencia",          // 8: Science
    "Variado",          // 9: Varied
    "Música Pop",       // 10: Pop Music
    "Música Rock",      // 11: Rock Music
    "Música Ligera",    // 12: Easy Listening
    "Clásica Ligera",   // 13: Light Classical
    "Música Clásica",   // 14: Serious Classical
    "Otra Música",      // 15: Other Music
    "Tiempo",           // 16: Weather
    "Finanzas",         // 17: Finance
    "Infantil",         // 18: Children's programmes
    "Asuntos Sociales", // 19: Social Affairs
    "Religión",         // 20: Religion
    "Llamadas",         // 21: Phone In
    "Viajes",           // 22: Travel
    "Ocio",             // 23: Leisure
    "Música Jazz",      // 24: Jazz Music
    "Música Country",   // 25: Country Music
    "Música Nacional",  // 26: National Music
    "Clásicos Pop",     // 27: Oldies Music
    "Música Folk",      // 28: Folk Music
    "Documental",       // 29: Documentary
    "Prueba Alarma",    // 30: Alarm Test
    "Alarma"            // 31: Alarm
};

typedef void (*RDSServiceCallback)(const char* psName);
typedef void (*RDSTextCallback)(const char* radioText);
typedef void (*RDSTimeCallback)(uint8_t hour, uint8_t minute);
typedef void (*RDSPtyCallback)(uint8_t ptyCode);

class NKFM_RDSParser {
  private:
    char     _rdsPS[9];
    char     _rdsPS_buf[9];
    uint8_t  _psMask = 0;

    char     _rdsRT[65];
    uint8_t  _rdsLastAB = 255;
    uint16_t _rtMask    = 0;
    uint16_t _rtEndMask = 0xFFFF;
    bool     _rtReady   = false;

    RDSServiceCallback _onPS   = nullptr;
    RDSTextCallback    _onRT   = nullptr;
    RDSTimeCallback    _onTime = nullptr;
    RDSPtyCallback     _onPTY  = nullptr;

    static char rdsCharToGFX(uint8_t c);

  public:
    NKFM_RDSParser();

    void reset();
    void setCallbacks(RDSServiceCallback onPS, RDSTextCallback onRT, RDSTimeCallback onTime, RDSPtyCallback onPTY);
    void feedGroup(uint16_t b1, uint16_t b2, uint16_t b3, uint16_t b4,
                   uint8_t errB = 0, uint8_t errC = 0, uint8_t errD = 0);

    const char* getPS() const { return _rdsPS; }
    const char* getRT() const { return _rdsRT; }

    static const char* getPTYName(uint8_t pty) {
      return (pty < 32) ? PTYeurope[pty] : PTYeurope[0];
    }
};
