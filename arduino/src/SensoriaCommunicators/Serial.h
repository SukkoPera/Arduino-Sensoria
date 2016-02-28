#include <Sensoria.h>
#include <SensoriaInternals/Communicator.h>
#include <SensoriaInternals/common.h>

class SensoriaSerialCommunicator: public SensoriaCommunicator {
private:
  Stream *serial;

  char *readSerialString () {
    static char buf[100];
    static int i = 0;

    char *ret = NULL;

    while (serial -> available ()) {
        char c = serial -> read ();
        switch (c) {
          case '\r':
            // Ignore
            break;
          case '\n':
            // End of string, process
            buf[i] = '\0';
            ret = buf;
            i = 0;
            break;
          case -1:
            // No char available to read
            break;
          default:
            // Got new char, append
            buf[i++] = c;
            break;
      }
    }

    return ret;
  }

public:
  SensoriaSerialCommunicator () {
  }

  void begin (Stream& _serial) {
    serial = &_serial;
  }

  // dest & port are ignored, of course
  boolean send (const char *str, IPAddress& dest _UNUSED, uint16_t port _UNUSED) override {
    return serial -> println (str) > 0;
  }

  boolean receiveString (char **str, IPAddress *senderAddr _UNUSED, uint16_t *senderPort _UNUSED) override {
    if ((*str = readSerialString ())) {
      //~ Serial.print ("Received: \"");
      //~ Serial.print (str);
      //~ Serial.println ("\"");
    }

    return *str != NULL;
  }
};
