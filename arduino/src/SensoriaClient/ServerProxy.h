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
	enum CommandResult {
		SEND_OK = 1,          // All good, reply is valid
		SEND_ERR = -1,        // Command sent but got an error reply
		SEND_UNEXP_ERR = -1,  // Command sent but got an unexpected reply
		SEND_TIMEOUT = -99    // Send timed out
	};

	char name[MAX_SERVER_NAME];

	byte nFailures;

	ServerProxy (SensoriaCommunicator* _comm, IPAddress& address, uint16_t port = DEFAULT_PORT);

	CommandResult sendcmd (const char *cmd, char*& reply);

	boolean addTransducer (TransducerProxy *tpx);

	TransducerProxy* getTransducer (const char *name) const;

	SensorProxy* getSensor (const char *name) const;

	boolean read (TransducerProxy& t);

	boolean write (ActuatorProxy& a, Stereotype& st);

private:
	SensoriaCommunicator *comm;
	IPAddress address;
	uint16_t port;

	byte nTransducers;

	uint16_t checksum;

	TransducerProxy *transducers[MAX_TRANSDUCERS];

	friend class SensoriaIterator;
	friend class SensoriaClient;
};

#endif
