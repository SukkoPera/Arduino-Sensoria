#include <Sensoria.h>
#include <SensoriaStereotypes/AllStereotypes.h>
#include <SensoriaCore/utils.h>
#include <SensoriaCore/debug.h>
#include "TransducerProxy.h"

TransducerProxy::TransducerProxy (ServerProxy* _srvpx, const char *_name,
    TransducerType _type, Stereotype *_stereotype, const char *_description,
    const char *_version): srvpx (_srvpx), type (_type), stereotype (_stereotype) {

  strlcpy (name, _name, MAX_TRANSDUCER_NAME);
#ifndef SAVE_RAM
  strlcpy (description, _description, MAX_TRANSDUCER_DESC);
  if (_version != NULL)
    strlcpy (version, _version, MAX_TRANSDUCER_VER);
#else
  // Make GCC happy
  (void) _description;
  (void) _version;
#endif
}

Stereotype *TransducerProxy::parseReply (char*& reply) {
  Stereotype *ret = NULL;

  if (stereotype) {
    stereotype -> clear ();
    if (stereotype -> unmarshal (reply)) {
      ret = stereotype;
    }
  }

  return ret;
}

boolean TransducerProxy::read (char*& reply) {
  boolean ret;
  char buf[8] = {0}, *r;
  strcat (buf, "REA ");
  strcat (buf, name);
  strcat (buf, "\n");

  if ((ret = srvpx -> sendcmd (buf, r))) {
    char *p[2];
    if (splitString (r, p, 2) != 2) {
      DPRINT (F("AAA:"));
      DPRINTLN (r);
      ret = false;
    } else if (strcmp (p[0], name) != 0) {
      DPRINT (F("AAAAAA: "));
      DPRINTLN (r);
      ret = false;
    } else {
      reply = p[1];
    }
  }

  return ret;
}

Stereotype *TransducerProxy::read () {
  Stereotype *ret = NULL;
  char *reply;
  if (read (reply)) {
    ret = parseReply (reply);
  }

  return ret;
}

#define SZ_NRQ 24

boolean TransducerProxy::requestNotification (NotificationType type, word period) {
  boolean ret = false;

  if (type == NT_CHA || period > 0) {
    char buf[8] = {0}, *r;
    strncat_P (buf, PSTR ("NRQ "), SZ_NRQ);
    strncat (buf, name, SZ_NRQ);
    switch (type) {
      case NT_CHA:
        strncat_P (buf, PSTR (" CHA"), SZ_NRQ);
        break;
      case NT_PRD:
        strncat_P (buf, PSTR (" PRD "), SZ_NRQ);
        utoa (period, buf + 11, 10);
        break;
    }
    strncat (buf, "\n", SZ_NRQ);

    if ((ret = srvpx -> sendcmd (buf, r))) {
      char *p[3];
      if (splitString (r, p, 2) != 2) {
        DPRINT (F("AAA:"));
        DPRINTLN (r);
        ret = false;
      } else if (strcmp (p[0], name) != 0) {
        DPRINT (F("AAAAAA: "));
        DPRINTLN (r);
        ret = false;
      } else if (strcmp_P (p[1], PSTR ("OK")) == 0) {
        ret = true;
      }
    }
  }

  return ret;
}

/******************************************************************************/


SensorProxy::SensorProxy (ServerProxy* _srvpx, const char *_name,
    Stereotype *_stereotype, const char *_description, const char *_version):
  TransducerProxy (_srvpx, _name, TransducerType::TYPE_SENSOR, _stereotype,
    _description, _version) {
}

/******************************************************************************/


ActuatorProxy::ActuatorProxy (ServerProxy* _srvpx, const char *_name, Stereotype *_stereotype, const char *_description, const char *_version):
  TransducerProxy (_srvpx, _name, TransducerType::TYPE_ACTUATOR, _stereotype, _description, _version) {
}

#define SZ 32

boolean ActuatorProxy::write (char* what, char*& reply) {
  boolean ret;

  char buf[SZ];
  buf[0] = '\0';
  strncat_P (buf, PSTR ("WRI "), SZ);
  strncat (buf, name, SZ);
  strncat_P (buf, PSTR (" "), SZ);
  strncat (buf, what, SZ);
  strncat_P (buf, PSTR ("\n"), SZ);

  ret = srvpx -> sendcmd (buf, reply);
  return ret;
}

boolean ActuatorProxy::write (Stereotype& st) {
  char *tmp, buf[16];
  tmp = st.marshal (buf, 16);
  return tmp && write (buf, tmp);
}
