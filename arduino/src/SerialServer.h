#include "SensoriaServer.h"

class SensoriaSerialServer: public SensoriaServer {
private:
  char *readSerialString () {
    static char buf[100];
    static int i = 0;

    char *ret = NULL;

    while (Serial.available ()) {
        char c = Serial.read ();
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
  SensoriaSerialServer () {
  }

  boolean begin (FlashString _serverName) {
    Serial.begin (9600);
    return SensoriaServer::begin (_serverName, F("20160202"));
  }

  boolean send (const char *str) {
    return Serial.println (str) > 0;
  }

  boolean receive () {
    char *str;
    if ((str = readSerialString ())) {
      Serial.print ("Received: \"");
      Serial.print (str);
      Serial.println ("\"");
      process_cmd (str);
    }

    return (str != NULL);
  }
};
