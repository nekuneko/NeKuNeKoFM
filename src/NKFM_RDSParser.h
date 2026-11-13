#pragma once
#include <Arduino.h>

typedef void (*NKFM_RDSServiceNameCb)(const char* name);
typedef void (*NKFM_RDSTextCb)(const char* text);
typedef void (*NKFM_RDSTimeCb)(uint8_t hour, uint8_t minute);
typedef void (*NKFM_RDSPtyCb)(uint8_t pty);

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

class NKFM_RDSParser {
  private:
    char _psName1[10];
    char _psName2[10];
    char _programServiceName[10];

    char     _rdstext[66];
    char     _lastSentRT[66];
    uint8_t  _lastTextIDX;
    uint8_t  _textAB;
    uint8_t  _lastTextAB;
    uint16_t _rtMask;
    uint8_t  _rtEndSeg;

    uint16_t _lastMinutes;
    uint8_t  _lastPTY;

    NKFM_RDSServiceNameCb _sendServiceName = nullptr;
    NKFM_RDSTextCb        _sendText        = nullptr;
    NKFM_RDSTimeCb        _sendTime        = nullptr;
    NKFM_RDSPtyCb         _sendPTY         = nullptr;

    static char decodeCP437(uint8_t c);

  public:
    NKFM_RDSParser();

    void reset();
    void feedGroup(uint16_t b1, uint16_t b2, uint16_t b3, uint16_t b4,
                   uint8_t errB = 0, uint8_t errC = 0, uint8_t errD = 0);

    void attachServiceNameCallback(NKFM_RDSServiceNameCb cb) { _sendServiceName = cb; }
    void attachTextCallback(NKFM_RDSTextCb cb)               { _sendText = cb; }
    void attachTimeCallback(NKFM_RDSTimeCb cb)               { _sendTime = cb; }
    void attachPTYCallback(NKFM_RDSPtyCb cb)                 { _sendPTY = cb; }

    const char* getServiceName() const { return _programServiceName; }
    const char* getRadioText() const   { return _rdstext; }
    uint8_t     getPTY() const         { return _lastPTY; }

    static const char* getPTYName(uint8_t pty) {
      return (pty < 32) ? PTYeurope[pty] : PTYeurope[0];
    }

    static const char* getPTYNameEurope(uint8_t pty) {
      return getPTYName(pty);
    }


};
