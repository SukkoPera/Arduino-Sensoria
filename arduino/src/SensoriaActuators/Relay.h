#include <SensoriaCore/Actuator.h>
#include <SensoriaStereotypes/RelayData.h>

class Relay: public Actuator<RelayData> {
private:
  static const byte NO_PIN = ~0;
  byte pin;

public:
  typedef RelayData::State State;

  State state;

  boolean inverted;

  Relay (): pin (NO_PIN) {
  }

  boolean begin (FlashString name, FlashString description, byte _pin, boolean invertedLogic = false, State initialState = RelayData::STATE_OFF) {
    if (Actuator::begin (name, F("RS"), description) && _pin != NO_PIN) {
      pin = _pin;
      inverted = invertedLogic;
      state = initialState;

      pinMode (pin, OUTPUT);
      doSwitch ();

      return true;
    } else {
      return false;
    }
  }

  boolean write (RelayData& rd) override {
    if (pin != NO_PIN) {
      if (rd.state != RelayData::STATE_UNKNOWN) {
        state = rd.state;
        doSwitch ();
        return true;
      } else {
        DPRINTLN (F("Cannot set relay to unknown state"));
      }
    }

    return false;
  }

  boolean read (RelayData& rd) override {
    rd.state = state;
    return true;
  }

private:
  // Makes the relay reflect the internal state
  void doSwitch () {
    digitalWrite (pin, !!state ^ inverted);   // Nice hack, huh? ;)
  }
};
