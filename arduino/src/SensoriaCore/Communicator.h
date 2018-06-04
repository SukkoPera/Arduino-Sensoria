#ifndef _COMMUNICATOR_H_INCLUDED
#define _COMMUNICATOR_H_INCLUDED

#include <Arduino.h>
#include <IPAddress.h>

enum SensoriaChannel {
	CC_SERVER        = 1 << 0,
	CC_NOTIFICATIONS = 1 << 1,

	// We still need to enable the UDP socket for output
	CC_CLIENT        = CC_SERVER
};

class SensoriaAddress {
public:
	virtual char* toString (char* buf, byte size) const = 0;

	SensoriaAddress& operator=(const SensoriaAddress& other) {
		clone (other);
		return *this;
	}

	bool operator== (const SensoriaAddress& other) const {
			return equalTo (other);
	}

	bool operator!= (const SensoriaAddress& other) const {
		return !(*this == other);
	}

protected:
	virtual bool equalTo (const SensoriaAddress& other) const = 0;

	virtual void clone (const SensoriaAddress& otherBase) = 0;
};

class SensoriaCommunicator {
public:
	enum SendResult {
		SEND_OK = 1,          // All good, reply is valid
		SEND_ERR = -1,        // Command sent but got an error reply
		SEND_UNEXP_ERR = -1,  // Command sent but got an unexpected reply
		SEND_TIMEOUT = -99    // Send timed out
	};

	virtual SensoriaAddress* getAddress () = 0;

	virtual void releaseAddress (SensoriaAddress* addr) = 0;

#ifdef ENABLE_NOTIFICATIONS
	virtual SensoriaAddress* getNotificationAddress (const SensoriaAddress* client) = 0;
#endif

	// Functions for servers
	virtual boolean receiveCmd (char*& cmd, SensoriaAddress* client) = 0;

	virtual SendResult reply (const char* reply, const SensoriaAddress* client) = 0;

	virtual boolean notify (const char* notification, const SensoriaAddress* client) = 0;

	// Function for clients
	virtual SendResult sendCmd (const char* cmd, const SensoriaAddress* server, char*& reply) = 0;

	virtual SendResult broadcast (const char* cmd) = 0;

	virtual boolean receiveBroadcastReply (char*& reply, SensoriaAddress*& sender, unsigned int timeout) = 0;

#ifdef ENABLE_NOTIFICATIONS
	virtual boolean receiveNotification (char*& notification) = 0;
#endif




#if 0
	// Override if needed, but always call super
	//~ virtual boolean begin () = 0;

	// Override if needed
	//~ virtual boolean stop ();

		// Override to implement actual sending of data
	// Mmmmh... Deprecate?
	//~ virtual boolean send (const char *str) = 0;

	// Override to implement actual sending of data
	virtual boolean send (const char *str, IPAddress& dest, uint16_t port) = 0;

	virtual boolean broadcast (const char *str, uint16_t port) = 0;

	// Override to implement actual receiving of data
	virtual boolean receiveString (char **str, IPAddress *senderAddr, uint16_t *senderPort, SensoriaChannel channel) = 0;

	boolean receiveStringWithTimeout (char **str, IPAddress *senderAddr, uint16_t *senderPort, SensoriaChannel channel, unsigned int timeout_ms) {;
		boolean ret;
		unsigned long start = millis ();
		do {
			ret = receiveString (str, senderAddr, senderPort, channel);
			if (ret)
				break;
		} while (millis () - start < timeout_ms);

		return ret;
	}
#endif
};

#endif
