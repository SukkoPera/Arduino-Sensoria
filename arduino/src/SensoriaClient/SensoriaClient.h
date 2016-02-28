#ifndef _SENSORIACLIENT_H_INCLUDED
#define _SENSORIACLIENT_H_INCLUDED

#include <Sensoria.h>
#include "SensoriaIterator.h"
#include "ServerProxy.h"
#include "TransducerProxy.h"

class SensoriaClient {
private:
  SensoriaCommunicator *comm;

  ServerProxy *servers[MAX_SERVERS];

  int nServers;

  friend class SensoriaIterator;

public:
  SensoriaClient ();

  void begin (SensoriaCommunicator& comm);

  boolean registerNode (IPAddress& addr, uint16_t port = DEFAULT_PORT);

  boolean discoverSensors (ServerProxy& srvpx);

  ServerProxy *getServer (const char *name);

  TransducerProxy *getTransducer (const char *name);

  SensorProxy *getSensor (const char *name);

  ActuatorProxy *getActuator (const char *name);

  SensoriaIterator getIterator ();
};

#endif
