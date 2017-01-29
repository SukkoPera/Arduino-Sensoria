#include <SensoriaCore/Actuator.h>
#include <SensoriaStereotypes/ControlledRelayData.h>

class ControlledRelay: public Actuator {
private:
  static const byte NO_PIN = ~0;
  byte pin;

public:
  typedef ControlledRelayData::State State;
  typedef ControlledRelayData::Controller Controller;

  State state;
  Controller controller;

  boolean inverted;

  ControlledRelay (): pin (NO_PIN) {
  }

  boolean begin (FlashString name, FlashString description, byte _pin, boolean invertedLogic = false,
                 State initialState = ControlledRelayData::STATE_OFF,
                 Controller initialController = ControlledRelayData::CTRL_AUTO) {
    if (Actuator::begin (name, F("CR"), description, F("20170128")) && _pin != NO_PIN) {
      pin = _pin;
      inverted = invertedLogic;
      state = initialState;
      controller = initialController;

      pinMode (pin, OUTPUT);
      doSwitch ();

      return true;
    } else {
      return false;
    }
  }

  boolean write (Stereotype *st) override {
    if (pin != NO_PIN) {
      ControlledRelayData& rd = *static_cast<ControlledRelayData *> (st);
      if (rd.state != ControlledRelayData::STATE_UNKNOWN &&
          rd.controller != ControlledRelayData::CTRL_UNKNOWN) {
        state = rd.state;
        controller = rd.controller;
        doSwitch ();
        return true;
      } else {
        DPRINTLN (F("Cannot set relay to unknown state/controller"));
      }
    }

    return false;
  }

  boolean read (Stereotype *st) override {
    ControlledRelayData& rd = *static_cast<ControlledRelayData *> (st);
    rd.state = state;
    rd.controller = controller;
    return true;
  }

private:
  // Makes the relay reflect the internal state
  void doSwitch () {
    digitalWrite (pin, !!state ^ inverted);   // Nice hack, huh? ;)
  }
};
