#include <Sensoria.h>
#include <SensoriaCore/common.h>
#include <SensoriaCore/debug.h>
#include <SensoriaCore/utils.h>
#include <SensoriaStereotypes/AllStereotypes.h>
#include "SensoriaClient.h"

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
	if (!comm -> broadcast ("HLO", DEFAULT_PORT)) {
		DPRINTLN (F("Cannot broadcast discovery command"));
	} else {
		unsigned long startTime = millis ();
		while (millis () - startTime < DISCOVERY_TIMEOUT) {
			char* reply;
			IPAddress addr;
			uint16_t port;
			if ((comm -> receiveStringWithTimeout (&reply, &addr, &port, CC_SERVER, DISCOVERY_TIMEOUT))) {
				// Remove trailing whitespace
				strstrip (reply);

#ifdef DEBUG_COMMS
				DPRINT (F("(discovery) "));
				DPRINT (addr);
				DPRINT (F(" --> "));
				DPRINTLN (reply);
#endif

				char *p[3];
				int n = splitString (reply, p, 3);
				if (n < 2) {
					DPRINT (F("Unexpected HLO reply: "));
					DPRINTLN (reply);
				} else {
					strupr (p[0]);
					if (strcmp_P (p[0], PSTR ("ERR")) == 0) {
						DPRINTLN (F("Node does not support HLO"));
					} else if (strcmp_P (p[0], PSTR ("HLO")) != 0) {
						DPRINT (F("Unexpected HLO reply: "));
						DPRINTLN (reply);
					} else {
						char* serverName = p[1];

						// p[2] will be modified by the splitString() below, so do this now
						uint16_t crc = crc16_update_str (0, p[2]);

						char* transducerList[MAX_TRANSDUCERS];
						int n = splitString (p[2], transducerList, MAX_TRANSDUCERS, '|');
						if (n < MAX_TRANSDUCERS) {
							// Terminate list
							transducerList[n] = NULL;
						}

						boolean add = false;

						ServerProxy* srvpx = getServer (serverName);
						if (srvpx) {
							DPRINT (F("Server is already known: "));
							DPRINTLN (serverName);

							if (addr != srvpx -> address || port != srvpx -> port
									|| crc != srvpx -> checksum) {

								DPRINTLN (F("Server has changed"));
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
							if (nServers < MAX_SERVERS - 1) {
								ServerProxy* srvpx = realizeServer (addr, port, serverName, transducerList, crc);
								servers[nServers++] = srvpx;
							} else {
								DPRINTLN (F("Too many servers registered, skipping"));
							}
						}
					}
				}
			}
		}
	}
}

ServerProxy* SensoriaClient::realizeServer (IPAddress& addr, uint16_t port, char*& serverName, char** transducerList, uint16_t crc) {
	DPRINT (F("Found node '"));
	DPRINT (serverName);
	DPRINTLN (F("' with:"));

	ServerProxy* srvpx = new ServerProxy (comm, addr, port);
	strlcpy (srvpx -> name, serverName, MAX_SERVER_NAME);
	srvpx -> checksum = crc;

	for (int i = 0; i < MAX_TRANSDUCERS && transducerList[i]; i++) {
		char *t[4];
		int m = splitString (transducerList[i], t, 4);
		if (m != 4) {
			DPRINTLN (F("Cannot parse transducer info"));
		} else {
			Stereotype *st = NULL;
			for (int j = 0; !st && j < N_STEREOTYPES; j++) {
				if (strcmp (stereotypes[j] -> tag, t[2]) == 0)
					st = stereotypes[j];
			}

			DPRINT (F("- Found "));
			DPRINT (t[1][0] == 'S' ? F("sensor ") : F("actuator "));
			DPRINT (t[0]);
			DPRINT (F(" ("));
			DPRINT (t[3]);
			DPRINT (F(") using stereotype "));
			DPRINT (t[2]);
			DPRINTLN (st ? F(" (Available)") : F(" (Not available)"));

			if (t[1][0] == 'S') {
				// We got a sensor
				// FIXME: Remove dynamic allocation
				SensorProxy *spx = new SensorProxy (srvpx, t[0], st, t[3]);
				if (!srvpx -> addTransducer (spx)) {
					DPRINT (F("Cannot register sensor: "));
					DPRINTLN (t[0]);
				}
			} else if (t[1][0] == 'A') {
				// Actuator
				ActuatorProxy *apx = new ActuatorProxy (srvpx, t[0], st, t[3]);
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

	return srvpx;
}

boolean SensoriaClient::registerNode (IPAddress& addr, uint16_t port) {
	boolean ret = false;

	if (comm && nServers < MAX_SERVERS - 1) {
		char *reply;

		// FIXME: Remove dynamic allocation
		ServerProxy* srvpx = new ServerProxy (comm, addr, port);
		ServerProxy::CommandResult res = srvpx -> sendcmd ("HLO", reply);
		if (res > 0) {
			//~ DPRINT ("Parsing: '");
			//~ DPRINT (reply);
			//~ DPRINTLN ("'");

			char *p[2];
			int n = splitString (reply, p, 2);
			if (n == 0) {
				DPRINT (F("Unexpected HLO reply: "));
				DPRINTLN (reply);
			} else {
				DPRINT (F("Registering node '"));
				DPRINT (p[0]);
				DPRINTLN (F("'"));
				strlcpy (srvpx -> name, p[0], MAX_SERVER_NAME);

				char *q[MAX_TRANSDUCERS + 1];
				int n = splitString (p[1], q, MAX_TRANSDUCERS + 1, '|');
				DPRINT (F("Found "));
				DPRINT (n);
				DPRINTLN (F(" transducer(s):"));

				for (int i = 0; i < n; i++) {
					char *t[4];
					int m = splitString (q[i], t, 4);
					if (m != 4) {
						DPRINTLN (F("Cannot parse transducer info"));
					} else {
						Stereotype *st = NULL;
						for (int j = 0; !st && j < N_STEREOTYPES; j++) {
							if (strcmp (stereotypes[j] -> tag, t[2]) == 0)
								st = stereotypes[j];
						}

						DPRINT (F("- Found "));
						DPRINT (t[1][0] == 'S' ? F("sensor ") : F("actuator "));
						DPRINT (t[0]);
						DPRINT (F(" ("));
						DPRINT (t[3]);
						DPRINT (F(") using stereotype "));
						DPRINT (t[2]);
						DPRINTLN (st ? F(" (Available)") : F(" (Not available)"));

						if (t[1][0] == 'S') {
							// We got a sensor
							// FIXME: Remove dynamic allocation
							SensorProxy *spx = new SensorProxy (srvpx, t[0], st, t[3]);
							if (!srvpx -> addTransducer (spx)) {
								DPRINT (F("Cannot register sensor: "));
								DPRINTLN (t[0]);
							}
						} else if (t[1][0] == 'A') {
							// Actuator
							ActuatorProxy *apx = new ActuatorProxy (srvpx, t[0], st, t[3]);
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

				// Register server
				servers[nServers++] = srvpx;

				ret = true;
			}
		} else if (res == ServerProxy::SEND_TIMEOUT) {
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
