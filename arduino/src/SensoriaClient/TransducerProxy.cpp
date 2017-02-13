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

boolean TransducerProxy::read (Stereotype*& st) {
  boolean ret = srvpx -> read (*this);
  if (ret) {
    st = stereotype;
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
			default:
				break;
		}
		strncat (buf, "\n", SZ_NRQ);

		if ((ret = srvpx -> sendcmd (buf, r))) {
			ret = strcmp_P (r, PSTR ("OK")) == 0;
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

boolean ActuatorProxy::write (Stereotype& st) {
  return srvpx -> write (*this, st);
}
