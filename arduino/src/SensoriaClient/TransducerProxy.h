#ifndef _SENSORPROXY_H_INCLUDED
#define _SENSORPROXY_H_INCLUDED

#include "ServerProxy.h"

#define SAVE_RAM

enum TransducerType {
  TYPE_SENSOR,
  TYPE_ACTUATOR,
  N_TYPES
};


class TransducerProxy {
protected:
  ServerProxy *srvpx;

public:
  char name[MAX_TRANSDUCER_NAME];
#ifndef SAVE_RAM
  char description[MAX_TRANSDUCER_DESC];
  char version[MAX_TRANSDUCER_VER];
#endif
  TransducerType type;

  TransducerProxy (ServerProxy* _srvpx, const char *name, TransducerType _type, const char *stereotype, const char *description, const char *version);
};

class SensorProxy: public TransducerProxy {
public:
  SensorProxy (ServerProxy* _srvpx, const char *name, const char *stereotype, const char *description, const char *version = NULL);

  boolean read (char*& reply);
};

class ActuatorProxy: public TransducerProxy {
public:
  ActuatorProxy (ServerProxy* _srvpx, const char *name, const char *stereotype, const char *description, const char *version = NULL);
};

#endif
