#if 1

#include <Arduino.h>
#include <Sensoria.h>
#include <SensoriaCore/common.h>
#include <SensoriaCore/debug.h>
#include <SensoriaCore/utils.h>
#include <SensoriaStereotypes/AllStereotypes.h>
#include "TransducerProxy.h"
#include "ServerProxy.h"

//~ #define DEBUG_COMMS
#define CMD_LEN 3

// FIXME: Not sure this works on AVRs
static FlashString OK_STR PROGMEM = "OK";

ServerProxy::ServerProxy (SensoriaCommunicator* _comm, SensoriaAddress* _address):
	protocolVersion (0), nFailures (0), comm (_comm), address (_address),
	nTransducers (0) {

	name[0] = '\0';
}

// args is input, reply is output
SensoriaCommunicator::SendResult ServerProxy::sendcmd (const char *args, char*& reply) {
#ifdef DEBUG_COMMS
	DPRINT (F("<-- "));
	DPRINT (args);   // Trailing CR is embedded in string, so no need for DPRINTLN
#endif

	reply = NULL;
	SensoriaCommunicator::SendResult ret = comm -> sendCmd (args, address, reply);
	if (ret > 0) {
#ifdef DEBUG_COMMS
		DPRINT (F("--> "));
		DPRINTLN (reply);
#endif

		// Remove trailing whitespace
		strstrip (reply);

		char *rc[2];
		if (splitString (reply, rc, 2) == 2)
			reply = rc[1];
		if (strcmp_P (rc[0], PSTR ("ERR")) == 0) {
			DPRINT (F("Command failed: "));
			DPRINTLN (rc[1]);
			ret = SensoriaCommunicator::SEND_ERR;
		} else if (strncmp (rc[0], args, CMD_LEN) != 0) {
			// Reply doesn't start with the command we sent
			DPRINT (F("Unexpected reply: "));
			DPRINTLN (reply);
			ret = SensoriaCommunicator::SEND_UNEXP_ERR;
		} else {
			// Command succeeded!
			ret = SensoriaCommunicator::SEND_OK;
		}
	} else {
		DPRINTLN (F("Cannot send command or no reply received"));
	}

	return ret;
}

boolean ServerProxy::addTransducer (TransducerProxy *tpx) {
	if (nTransducers < MAX_TRANSDUCERS) {
		transducers[nTransducers++] = tpx;
		return true;
	} else {
		return false;
	}
}

TransducerProxy* ServerProxy::getTransducer (const char *name) const {
	TransducerProxy *tpx = NULL;

	char tmp[MAX_TRANSDUCER_NAME];
	strlcpy (tmp, name, MAX_TRANSDUCER_NAME);
	strupr (tmp);
	for (int i = 0; !tpx && i < nTransducers; i++) {
		if (strcmp (tmp, transducers[i] -> name) == 0) {
			tpx = transducers[i];
		}
	}

	return tpx;
}

SensorProxy* ServerProxy::getSensor (const char *name) const {
	SensorProxy *spx = NULL;

	char tmp[MAX_TRANSDUCER_NAME];
	strlcpy (tmp, name, MAX_TRANSDUCER_NAME);
	strupr (tmp);
	for (int i = 0; !spx && i < nTransducers; i++) {
		if (strcmp (tmp, transducers[i] -> name) == 0 && transducers[i] -> type == TYPE_SENSOR) {
			spx = static_cast<SensorProxy *> (transducers[i]);
		}
	}

	return spx;
}

boolean ServerProxy::read (TransducerProxy& t) {
	boolean ret = false;
	char buf[8] = {0}, *r;
	strcat (buf, "REA ");
	strcat (buf, t.name);
	strcat (buf, "\n");

	SensoriaCommunicator::SendResult res = sendcmd (buf, r);
	if (res > 0) {
		char *p[2];
		if (splitString (r, p, 2) != 2) {
			DPRINT (F("Unexpected REA reply: "));
			DPRINTLN (r);
		} else if (strcmp_P (p[0], OK_STR) != 0) {
			DPRINT (F("Read failed"));
		} else {
			char *reply = p[1];
			t.stereotype -> clear ();
			ret = t.stereotype -> unmarshal (reply);
		}
	} else if (res == SensoriaCommunicator::SEND_TIMEOUT) {
		DPRINTLN (F("Read timeout"));
		nFailures++;
	}

	return ret;
}

#define SZ 32

// UNTESTED!
boolean ServerProxy::write (ActuatorProxy& a, Stereotype& st) {
	boolean ret = false;

	char buf[SZ] = {'\0'};
	strncat_P (buf, PSTR ("WRI "), SZ);   // 4
	strncat (buf, a.name, SZ);            // +2=6
	strncat_P (buf, PSTR (" "), SZ - 9);  // +1=7
	char *tmp = st.marshal (buf + 7, SZ - 9);
	strncat_P (buf, PSTR ("\n"), SZ);

	if (tmp) {
		SensoriaCommunicator::SendResult res = sendcmd (buf, tmp);
		if (res > 0) {
			char *p[2];
			if (splitString (tmp, p, 2) < 1) {
				DPRINT (F("Unexpected WRI reply: "));
				DPRINTLN (tmp);
			} else if (strcmp_P (p[0], OK_STR) != 0) {
				DPRINT (F("Write failed"));
			}
		} else if (res == SensoriaCommunicator::SEND_TIMEOUT) {
			DPRINTLN (F("Write timeout"));
			nFailures++;
		}
	} else {
		DPRINTLN (F("Marshalling failed"));
	}

	return ret;
}

#endif
