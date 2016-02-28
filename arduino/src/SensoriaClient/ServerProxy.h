#ifndef _SERVERPROXY_H_INCLUDED
#define _SERVERPROXY_H_INCLUDED

#include <Sensoria.h>
#include <SensoriaInternals/Communicator.h>
//~ #include <SensoriaInternals/Server.h>

class TransducerProxy;
class SensorProxy;
class ActuatorProxy;


class ServerProxy {
public:
  char name[MAX_SERVER_NAME];

  ServerProxy (SensoriaCommunicator* _comm, IPAddress& address, uint16_t port = DEFAULT_PORT);

  boolean sendcmd (const char *cmd, char*& reply);

  boolean addTransducer (TransducerProxy *tpx);

  TransducerProxy* getTransducer (const char *name) const;

  SensorProxy* getSensor (const char *name) const;

private:
  SensoriaCommunicator *comm;
  IPAddress address;
  uint16_t port;

  byte nTransducers;

  TransducerProxy *transducers[MAX_TRANSDUCERS];

  friend class SensoriaIterator;
  friend class SensoriaClient;
};

#endif
