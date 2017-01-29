#ifndef _SENSORPROXY_H_INCLUDED
#define _SENSORPROXY_H_INCLUDED

#include <SensoriaCore/Stereotype.h>
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

  Stereotype* stereotype;

  Stereotype* parseReply (char*& reply);

  boolean read (char*& reply);

  Stereotype* read ();

  // Period is in seconds and is mandatory if type is periodic
  boolean requestNotification (NotificationType type, word period = 0);

  TransducerProxy (ServerProxy* _srvpx, const char *name, TransducerType _type, Stereotype *stereotype, const char *description, const char *version);
};

class SensorProxy: public TransducerProxy {
public:
  SensorProxy (ServerProxy* _srvpx, const char *name, Stereotype *stereotype, const char *description, const char *version = NULL);
};

class ActuatorProxy: public TransducerProxy {
public:
  ActuatorProxy (ServerProxy* _srvpx, const char *name, Stereotype *stereotype, const char *description, const char *version = NULL);

  boolean write (char* buf, char*& reply);

  boolean write (Stereotype& st);
};

#endif
