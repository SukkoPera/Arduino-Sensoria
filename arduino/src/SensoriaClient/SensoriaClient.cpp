#include <Sensoria.h>
#include <SensoriaCore/common.h>
#include <SensoriaCore/debug.h>
#include <SensoriaCore/utils.h>
#include <SensoriaStereotypes/AllStereotypes.h>
#include "SensoriaClient.h"

SensoriaClient::SensoriaClient (): comm (NULL), nServers (0) {
}

void SensoriaClient::begin (SensoriaCommunicator& _comm) {
  comm = &_comm;
}

//~ int SensoriaClient::discover () {

//~ }

boolean SensoriaClient::registerNode (IPAddress& addr, uint16_t port) {
  boolean ret = false;

  if (comm && nServers < MAX_SERVERS - 1) {
    char *reply;

    ServerProxy* srvpx = new ServerProxy (comm, addr, port);
    if (!srvpx -> sendcmd ("VER", reply)) {
      DPRINTLN (F("Cannot retrieve node info"));
    } else {
      //~ DPRINT ("Parsing: '");
      //~ DPRINT (reply);
      //~ DPRINTLN ("'");

      char *p[2];
      int n = splitString (reply, p, 2);
      if (n == 0) {
        DPRINTLN (F("Cannot parse node info"));
      } else {
        DPRINT (F("Registering node '"));
        DPRINT (p[0]);
        DPRINTLN (F("'"));
        strlcpy (srvpx -> name, p[0], MAX_SERVER_NAME);

        // Discover sensors
        if (!srvpx -> sendcmd ("QRY", reply)) {
          DPRINTLN (F("Cannot retrieve sensors list"));
        } else {
          //~ DPRINT ("Parsing: '");
          //~ DPRINT (reply);
          //~ DPRINTLN ("'");

          char *p[MAX_TRANSDUCERS + 1];
          int n = splitString (reply, p, MAX_TRANSDUCERS + 1, '|');
          DPRINT (F("Found "));
          DPRINT (n);
          DPRINTLN (F(" transducer(s):"));

          for (int i = 0; i < n; i++) {
            char *t[4];
            int m = splitString (p[i], t, 4);
            if (m != 4) {
              DPRINTLN (F("Cannot parse sensor info"));
            } else {
              Stereotype *st = NULL;
              for (int i = 0; !st && i < N_STEREOTYPES; i++) {
                if (strcmp (stereotypes[i] -> tag, t[2]) == 0)
                  st = stereotypes[i];
              }

              DPRINT (F("- Found "));
              DPRINT (t[1][0] == 'S' ? F("sensor ") : F("actuator "));
              DPRINT (t[0]);
              DPRINT (F(" ("));
              DPRINT (t[3]);
              DPRINT (F(") using stereotype "));
              DPRINT (t[2]);
              DPRINTLN (st ? F(" (Available)") : F(" (Not available)"));

              if (t[1][0] == 'S') {
                // We got a sensor
                // FIXME: Remove dynamic allocation
                SensorProxy *spx = new SensorProxy (srvpx, t[0], st, t[3]);
                if (!srvpx -> addTransducer (spx)) {
                  DPRINT (F("Cannot register sensor: "));
                  DPRINTLN (t[0]);
                }
              } else if (t[1][0] == 'A') {
                // Actuator
                ActuatorProxy *apx = new ActuatorProxy (srvpx, t[0], st, t[3]);
                if (!srvpx -> addTransducer (apx)) {
                  DPRINT (F("Cannot register actuator: "));
                  DPRINTLN (t[0]);
                }
              } else {
                DPRINT (F("Unsupported transducer type: "));
                DPRINTLN (t[1][0]);
              }
            }
          }

          // Register server
          if (nServers < MAX_SERVERS - 1) {
            servers[nServers++] = srvpx;
          }

          ret = true;
        }
      }
    }
  }

  return ret;
}

TransducerProxy *SensoriaClient::getTransducer (const char *name) {
  TransducerProxy *ret = NULL;

  for (int i = 0; !ret && i < nServers; i++) {
    ServerProxy& srvpx = *servers[i];
    for (int j = 0; j < srvpx.nTransducers; j++) {
      TransducerProxy& tpx = *srvpx.transducers[j];
      if (strcmp (tpx.name, name) == 0)
        ret = &tpx;
    }
  }

  return ret;
}

SensorProxy *SensoriaClient::getSensor (const char *name) {
  SensorProxy *ret = NULL;

  for (int i = 0; !ret && i < nServers; i++) {
    ServerProxy& srvpx = *servers[i];
    for (int j = 0; j < srvpx.nTransducers; j++) {
      TransducerProxy& tpx = *srvpx.transducers[j];
      if (tpx.type == TYPE_SENSOR && strcmp (tpx.name, name) == 0)
        ret = (SensorProxy *) &tpx;
    }
  }

  return ret;
}

ActuatorProxy *SensoriaClient::getActuator (const char *name) {
  ActuatorProxy *ret = NULL;

  for (int i = 0; !ret && i < nServers; i++) {
    ServerProxy& srvpx = *servers[i];
    for (int j = 0; j < srvpx.nTransducers; j++) {
      TransducerProxy& tpx = *srvpx.transducers[j];
      if (tpx.type == TYPE_ACTUATOR && strcmp (tpx.name, name) == 0)
        ret = (ActuatorProxy *) &tpx;
    }
  }

  return ret;
}

SensoriaIterator SensoriaClient::getIterator () {
  return SensoriaIterator (this);
}
