#ifndef _SERVERPROXY_H_INCLUDED
#define _SERVERPROXY_H_INCLUDED

#include <Sensoria.h>
#include <SensoriaCore/Communicator.h>
//~ #include <SensoriaCore/Server.h>

class TransducerProxy;
class SensorProxy;
class ActuatorProxy;
class Stereotype;


class ServerProxy {
public:
	char name[MAX_SERVER_NAME];

	byte nFailures;

	ServerProxy (SensoriaCommunicator* _comm, SensoriaAddress* address);

	SensoriaCommunicator::SendResult sendcmd (const char *cmd, char*& reply);

	boolean addTransducer (TransducerProxy *tpx);

	TransducerProxy* getTransducer (const char *name) const;

	SensorProxy* getSensor (const char *name) const;

	boolean read (TransducerProxy& t);

	boolean write (ActuatorProxy& a, Stereotype& st);

private:
	SensoriaCommunicator *comm;
	SensoriaAddress *address;

	byte nTransducers;

	uint16_t checksum;

	TransducerProxy *transducers[MAX_TRANSDUCERS];

	friend class SensoriaIterator;
	friend class SensoriaClient;
};

#endif
