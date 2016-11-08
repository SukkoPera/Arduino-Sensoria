#if 1

#include "Sensoria.h"
#include "SensoriaCore/common.h"
#include "SensoriaCore/debug.h"
#include "SensoriaCore/utils.h"
#include "ServerProxy.h"
#include "TransducerProxy.h"

//~ #define DEBUG_COMMS
#define CMD_LEN 3
//~ #define min(x, y) (x <= y ? x : y)


ServerProxy::ServerProxy (SensoriaCommunicator* _comm, IPAddress& _address, uint16_t _port): comm (_comm), address (_address), port (_port), nTransducers (0) {
	name[0] = '\0';
}

// args is input, reply is output
boolean ServerProxy::sendcmd (const char *args, char*& reply) {
	//~ char cmd[CMD_LEN + 1];
	//~ char *p = strchr (args, ' ');
	//~ if (p == NULL) {
		//~ strlcpy (cmd, args, min (strlen (args), CMD_LEN) + 1);
	//~ } else {
		//~ strlcpy (cmd, args, min (CMD_LEN, p - args) + 1);
	//~ }

	//~ DPRINT ("CMD = ");
	//~ DPRINTLN (cmd);

#ifdef DEBUG_COMMS
	DPRINT (F("<-- "));
	DPRINTLN (args);
#endif

	reply = NULL;

	boolean ret = comm -> send (args, address, port);
	if (!ret) {
		DPRINTLN (F("Cannot send command"));
	} else {
		IPAddress addr;
		uint16_t port;
		if ((ret = comm -> receiveStringWithTimeout (&reply, &addr, &port, CLIENT_TIMEOUT))) {
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
				ret = false;
			} else if (strncmp (rc[0], args, CMD_LEN) != 0) {
				DPRINT (F("Unexpected reply: "));
				DPRINTLN (reply);
				ret = false;
			}   // else command succeeded!
		} else {
			DPRINTLN (F("No reply received"));
		}
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

#endif
