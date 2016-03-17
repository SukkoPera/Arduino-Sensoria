#include <SensoriaCore/Actuator.h>

class Relay: public Actuator {
private:
  static const byte NO_PIN = 255;
  byte pin;

public:
  enum State {
    STATE_OFF = 0,
    STATE_ON
  };

  State state;

  boolean inverted;

  Relay (): pin (NO_PIN) {
  }

  bool begin (FlashString name, FlashString description, byte _pin, boolean invertedLogic = false, State initialState = STATE_OFF) {
    if (Actuator::begin (name, description, F("20160317")) && _pin != NO_PIN) {
      pin = _pin;
      inverted = invertedLogic;
      state = initialState;

      pinMode (pin, OUTPUT);
      digitalWrite (pin, !!state ^ inverted);   // Nice hack, huh? ;)

      return true;
    } else {
      return false;
    }
  }

  bool write (char *buf) {
    if (pin != NO_PIN) {
      strupr (buf);
      if (strcmp_P (buf, PSTR("ON")) == 0) {
        state = STATE_ON;
      } else if (strcmp_P (buf, PSTR("OFF")) == 0) {
        state = STATE_OFF;
      } else {
        return false;
      }

      digitalWrite (pin, !!state ^ inverted);

      return true;
    } else {
      return false;
    }
  }
};
