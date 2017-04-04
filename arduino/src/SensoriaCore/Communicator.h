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

class SensoriaCommunicator {
public:

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
};

#endif
