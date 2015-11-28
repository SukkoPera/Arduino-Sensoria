#include "SensoriaServer.h"

SensoriaServer::SensoriaServer (const char *_serverName, PGM_P _serverVersion): nTransducers (0), serverVersion (reinterpret_cast <PGM_P> (_serverVersion)) {
	clearBuffer ();
	strlcpy (serverName, _serverName, MAX_SERVER_NAME);
}

boolean SensoriaServer::begin () {
	return true;
}

boolean SensoriaServer::stop () {
	return true;
}

int SensoriaServer::addTransducer (Transducer& transducer) {
	if (nTransducers < MAX_SENSORS) {
		transducers[nTransducers++] = &transducer;
		return nTransducers - 1;
	} else {
		return -1;
	}
}

Transducer *SensoriaServer::getTransducer (char *name) const {
	strupr (name);
	for (int i = 0; i < nTransducers; ++i) {
		if (strcmp_P (name, reinterpret_cast<PGM_P> (transducers[i] -> name)) == 0)
			return transducers[i];
	}

	return NULL;
}

// Called automatically by SensoriaServer before read()
void SensoriaServer::clearSensorBuffer () {
	for (int i = 0; i < SENSOR_BUF_SIZE; i++)
		sensorBuf[i] = '\0';
}

boolean SensoriaServer::send_srv (const char *str, boolean cr) {
	if (str) {
		// Append to buffer
		// TODO: Check for buffer overflow
		strcat (buf, str);
	}

	if (cr) {
		// Send!
		boolean ok = false;
		strcat_P (buf, PSTR ("\r"));   // Line terminator
		ok = send (buf);

		clearBuffer ();

		return ok;
	} else {
		return true;
	}
}

boolean SensoriaServer::send_srv (const __FlashStringHelper *str, boolean cr) {
	if (str) {
		// Append to buffer
		// TODO: Check for buffer overflow
		strcat_P (buf, reinterpret_cast <PGM_P> (str));
	}

	if (cr) {
		// Send!
		boolean ok = false;
		strcat_P (buf, PSTR ("\r"));   // Line terminator
		ok = send (buf);

		clearBuffer ();

		return ok;
	} else {
		return true;
	}
}
