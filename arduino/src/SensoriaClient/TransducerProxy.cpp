#include "TransducerProxy.h"
#include <SensoriaCore/utils.h>
#include <SensoriaCore/debug.h>

TransducerProxy::TransducerProxy (ServerProxy* _srvpx, const char *_name, TransducerType _type, const char *_stereotype, const char *_description, const char *_version): srvpx (_srvpx), type (_type) {
  strlcpy (name, _name, MAX_TRANSDUCER_NAME);
#ifndef SAVE_RAM
  strlcpy (description, _description, MAX_TRANSDUCER_DESC);
  if (_version != NULL)
    strlcpy (version, _version, MAX_TRANSDUCER_VER);
#endif
}

SensorProxy::SensorProxy (ServerProxy* _srvpx, const char *_name, const char *_stereotype, const char *_description, const char *_version):
  TransducerProxy (_srvpx, _name, TransducerType::TYPE_SENSOR, _stereotype, _description, _version) {
}

boolean SensorProxy::read (char*& reply) {
  boolean ret;
  char buf[8] = {0}, *r;
  strcat (buf, "REA ");
  strcat (buf, name);
  strcat (buf, "\n");

  if ((ret = srvpx -> sendcmd (buf, r))) {
    char *p[2];
    if (splitString (r, p, 2) != 2) {
      DPRINTLN (F("AAAA"));
      ret = false;
    } else if (strcmp (p[0], name) != 0) {
      DPRINTLN (F("AAAAAA"));
      ret = false;
    } else {
      reply = p[1];
    }
  }

  return ret;
}

ActuatorProxy::ActuatorProxy (ServerProxy* _srvpx, const char *_name, const char *_stereotype, const char *_description, const char *_version):
  TransducerProxy (_srvpx, _name, TransducerType::TYPE_ACTUATOR, _stereotype, _description, _version) {
}
