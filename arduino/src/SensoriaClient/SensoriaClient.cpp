#include <Arduino.h>
#include <Sensoria.h>
#include <SensoriaCore/common.h>
#include <SensoriaCore/debug.h>
#include <SensoriaCore/utils.h>
#include <SensoriaStereotypes/AllStereotypes.h>
#include "SensoriaClient.h"

//~ #define DEBUG_COMMS

SensoriaClient::SensoriaClient (): comm (NULL), nServers (0) {
}

void SensoriaClient::begin (SensoriaCommunicator& _comm, const boolean autodiscover) {
	comm = &_comm;

	if (autodiscover) {
		discover ();
		lastDiscoveryTime = millis ();
	} else {
		// Disable autodiscovery
		lastDiscoveryTime = 0;
	}
}

void SensoriaClient::discover () {
	DPRINTLN (F("Discovering nodes..."));

	char* reply;
	SensoriaAddress* sender;
	SensoriaCommunicator::SendResult res = comm -> broadcast ("HLO");
	if (res == SensoriaCommunicator::SEND_OK) {
#ifdef DEBUG_COMMS
		DPRINTLN (F("Waiting for broadcast replies"));
#endif

		while (comm -> receiveBroadcastReply (reply, sender, DISCOVERY_TIMEOUT)) {
			// Remove trailing whitespace
			strstrip (reply);

#ifdef DEBUG_COMMS
			char buf[32];

			DPRINT (F("(discovery) "));
			DPRINT (sender -> toString (buf, sizeof (buf)));
			DPRINT (F(" --> "));
			DPRINTLN (reply);
#endif

			boolean add = false;
			char *p[4];
			int n = splitString (reply, p, 4);
			if (n != 4) {
				DPRINT (F("Unexpected HLO reply: "));
				DPRINTLN (reply);
			} else {
				strupr (p[0]);		// p[0] should be "HLO"
				if (strcmp_P (p[0], PSTR ("ERR")) == 0) {
					DPRINTLN (F("Node does not support HLO"));
				} else if (strcmp_P (p[0], PSTR ("HLO")) != 0) {
					DPRINT (F("Unexpected HLO reply: "));
					DPRINTLN (reply);
				} else {
					const char* serverName = p[1];
					const byte protoVer = atoi (p[2]);
					char* transducerListStr = p[3];

					// transducerListStr will be modified by the splitString() below, so do this now
					uint16_t crc = crc16_update_str (0, transducerListStr);

					char* transducerList[MAX_TRANSDUCERS + 1];
					int n = splitString (transducerListStr, transducerList, MAX_TRANSDUCERS, '|');
					transducerList[n] = NULL;	// Terminate list, n is definitely n < MAX_TRANSDUCERS + 1

					ServerProxy* srvpx = getServer (serverName);
					if (srvpx) {
						DPRINT (F("Server is already known: "));
						DPRINTLN (serverName);

						if (*sender != *(srvpx -> address) || crc != srvpx -> checksum) {
							DPRINTLN (F("Server has changed"));
							DPRINTLN (*sender != *(srvpx -> address));
							DPRINTLN (crc != srvpx -> checksum);
							delServer (serverName);
							delete srvpx;
							add = true;
						} else {
							DPRINTLN (F("Server is unchanged"));
						}
					} else {
						DPRINT (F("Server is new: "));
						DPRINTLN (serverName);
						add = true;
					}

					if (add) {
						if (nServers < MAX_SERVERS) {
							ServerProxy* srvpx = new ServerProxy (comm, sender);
							realizeServer (srvpx, serverName, protoVer, transducerList, crc);
							servers[nServers++] = srvpx;
						} else {
							DPRINTLN (F("Too many servers registered, skipping"));
							add = false;
						}
					}
				}
			}

			// Prepare for next iteration
			if (!add) {
				comm -> releaseAddress (sender);
			}
		}
	} else {
		DPRINTLN (F("Discovery failed"));
	}
}

void SensoriaClient::realizeServer (ServerProxy* srvpx, const char*& serverName,
	const byte protocolVersion, char** transducerList, uint16_t crc) {

	strlcpy (srvpx -> name, serverName, MAX_SERVER_NAME);
	srvpx -> checksum = crc;
	srvpx -> protocolVersion = protocolVersion;

	for (int i = 0; i < MAX_TRANSDUCERS && transducerList[i]; i++) {
		strstrip (transducerList[i]);
		char *t[4];
		int m = splitString (transducerList[i], t, 4);
		if (m < 3) {
			DPRINT (F("Cannot parse transducer info: '"));
			DPRINT (transducerList[i]);
			DPRINTLN ('\'');
		} else {
			const char* tName = t[0];
			const char tType = t[1][0];
			const char* tStereo = t[2];
			const char* tDesc = NULL;
			if (m == 4)
				tDesc = t[3];

			Stereotype *st = NULL;
			for (int j = 0; !st && j < N_STEREOTYPES; j++) {
				if (strcmp (stereotypes[j] -> tag, t[2]) == 0)
					st = stereotypes[j];
			}

			DPRINT (F("- "));
			DPRINT (tType == 'S' ? F("Sensor ") : F("Actuator "));
			DPRINT (tName);
			DPRINT (F(" using stereotype "));
			DPRINT (tStereo);
			DPRINTLN (st ? F(" (Available)") : F(" (Not available)"));

			if (t[1][0] == 'S') {
				// We got a sensor
				// FIXME: Remove dynamic allocation
				SensorProxy *spx = new SensorProxy (srvpx, t[0], st, tDesc);
				if (!srvpx -> addTransducer (spx)) {
					DPRINT (F("Cannot register sensor: "));
					DPRINTLN (t[0]);
				}
			} else if (t[1][0] == 'A') {
				// Actuator
				ActuatorProxy *apx = new ActuatorProxy (srvpx, t[0], st, tDesc);
				if (!srvpx -> addTransducer (apx)) {
					DPRINT (F("Cannot register actuator: "));
					DPRINTLN (t[0]);
				}
			} else {
				DPRINT (F("Unsupported transducer type: "));
				DPRINTLN (t[1][0]);
			}
		}
	}
}

boolean SensoriaClient::registerNode (SensoriaAddress* addr) {
	boolean ret = false;

	if (comm && nServers < MAX_SERVERS) {
		char *reply;

		// FIXME: Remove dynamic allocation
		ServerProxy* srvpx = new ServerProxy (comm, addr);
		SensoriaCommunicator::SendResult res = srvpx -> sendcmd ("HLO", reply);
		if (res > 0) {
#ifdef DEBUG_COMMS
			DPRINT ("Parsing: '");
			DPRINT (reply);
			DPRINTLN ("'");
#endif

			char *p[3];
			int n = splitString (reply, p, 3);
			if (n == 0) {
				DPRINT (F("Unexpected HLO reply: "));
				DPRINTLN (reply);
			} else {
				const char* serverName = p[0];
				const byte protoVer = atoi (p[1]);
				char* transducerListStr = p[2];

				// transducerListStr will be modified by the splitString() below, so do this now
				uint16_t crc = crc16_update_str (0, transducerListStr);

				DPRINT (F("Registering node '"));
				DPRINT (serverName);
				DPRINTLN ('\'');
				strlcpy (srvpx -> name, serverName, MAX_SERVER_NAME);

				char *transducerList[MAX_TRANSDUCERS + 1];
				int n = splitString (transducerListStr, transducerList, MAX_TRANSDUCERS, '|');
				transducerList[n] = NULL;	// Terminate list, n is definitely n < MAX_TRANSDUCERS + 1

				realizeServer (srvpx, serverName, protoVer, transducerList, crc);
				servers[nServers++] = srvpx;

				ret = true;
			}
		} else if (res == SensoriaCommunicator::SEND_TIMEOUT) {
			// HLO failed
			srvpx -> nFailures++;
		}
	}

	return ret;
}

ServerProxy *SensoriaClient::getServer (const char *name) {
	ServerProxy *ret = NULL;

	for (int i = 0; !ret && i < nServers; i++) {
		ServerProxy& srvpx = *servers[i];
		if (strcmp (srvpx.name, name) == 0)
			ret = &srvpx;
	}

	return ret;
}

TransducerProxy *SensoriaClient::getTransducer (const char *name) {
	TransducerProxy *ret = NULL;

	for (int i = 0; !ret && i < nServers; i++) {
		ServerProxy& srvpx = *servers[i];
		for (int j = 0; j < srvpx.nTransducers; j++) {
			TransducerProxy& tpx = *srvpx.transducers[j];
			if (strcmp (tpx.name, name) == 0)
				ret = &tpx;
		}
	}

	return ret;
}

SensorProxy *SensoriaClient::getSensor (const char *name) {
	SensorProxy *ret = NULL;

	for (int i = 0; !ret && i < nServers; i++) {
		ServerProxy& srvpx = *servers[i];
		for (int j = 0; j < srvpx.nTransducers; j++) {
			TransducerProxy& tpx = *srvpx.transducers[j];
			if (tpx.type == TYPE_SENSOR && strcmp (tpx.name, name) == 0)
				ret = (SensorProxy *) &tpx;
		}
	}

	return ret;
}

ActuatorProxy *SensoriaClient::getActuator (const char *name) {
	ActuatorProxy *ret = NULL;

	for (int i = 0; !ret && i < nServers; i++) {
		ServerProxy& srvpx = *servers[i];
		for (int j = 0; j < srvpx.nTransducers; j++) {
			TransducerProxy& tpx = *srvpx.transducers[j];
			if (tpx.type == TYPE_ACTUATOR && strcmp (tpx.name, name) == 0)
				ret = (ActuatorProxy *) &tpx;
		}
	}

	return ret;
}

SensoriaIterator SensoriaClient::getIterator () {
	return SensoriaIterator (this);
}

void SensoriaClient::delServer (const char* name) {
	int idx = -1;
	for (int i = 0; idx < 0 && i < nServers; i++) {
		ServerProxy& srvpx = *servers[i];
		if (strcmp (srvpx.name, name) == 0)
			idx = i;
	}

	if (idx >= 0) {
		for (byte j = 0; idx + j + 1 < nServers; j++) {
			servers[idx + j] = servers[idx + j + 1];
		}

		nServers--;
	}
}

void SensoriaClient::loop () {
	// Autodiscovery
	if (DISCOVERY_INTERVAL > 0 && lastDiscoveryTime > 0 &&
			millis () - lastDiscoveryTime >= DISCOVERY_INTERVAL) {

		discover ();
		lastDiscoveryTime = millis ();
	}

	for (byte i = 0; i < nServers; i++) {
		ServerProxy& srvpx = *servers[i];

		if (srvpx.nFailures >= MAX_FAILURES) {
			DPRINT (F("Deleting server "));
			DPRINT (srvpx.name);
			DPRINTLN (F(" because of excessive failures"));
			delServer (srvpx.name);
			delete &srvpx;
		}
	}
}
