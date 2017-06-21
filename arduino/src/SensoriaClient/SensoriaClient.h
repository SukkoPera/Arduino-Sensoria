#ifndef _SENSORIACLIENT_H_INCLUDED
#define _SENSORIACLIENT_H_INCLUDED

#include <Sensoria.h>
#include "SensoriaIterator.h"
#include "ServerProxy.h"
#include "TransducerProxy.h"

class SensoriaClient {
private:
	SensoriaCommunicator *comm;

	ServerProxy *servers[MAX_SERVERS];

	int nServers;

	// If 0, autodiscovery is disabled
	unsigned long lastDiscoveryTime;

	boolean parseHloReply (char *reply, char*& serverName, char** transducerList);

	ServerProxy* realizeServer (SensoriaAddress* addr, char*& serverName, char** transducerList, uint16_t crc);

	friend class SensoriaIterator;

	void delServer (const char* name);

public:
	SensoriaClient ();

	void begin (SensoriaCommunicator& comm, const boolean autodiscover = true);

	void discover ();

	boolean registerNode (SensoriaAddress* addr);

	ServerProxy *getServer (const char *name);

	TransducerProxy *getTransducer (const char *name);

	SensorProxy *getSensor (const char *name);

	ActuatorProxy *getActuator (const char *name);

	SensoriaIterator getIterator ();

	void loop ();
};

#endif
