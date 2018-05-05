#ifndef _UDP_ADDRESS_H_
#define _UDP_ADDRESS_H_

#include <Arduino.h>
#include <SensoriaCore/utils.h>

class UdpAddress: public SensoriaAddress {
public:
	IPAddress ip;
	uint16_t port;
	boolean inUse;

	char* toString (char* buf, byte size) const override {
		char tmp[6];    // Max length of a 16-bit integer + 1

		// Clear output string
		buf[0] = '\0';

		// Stringize IP
		for (int i = 0; i < 4; i++) {
			 	utoa (ip[i], tmp, 10);
				strncat (buf, tmp, size - 1);
				strncat (buf, ".", size - 1);
		}

		// Replace last dot with a colon
		buf[strlen (buf) - 1] = ':';

		// Append port
		utoa (port, tmp, 10);
		strncat (buf, tmp, size - 1);

		return buf;
	}

protected:
	virtual bool equalTo (const SensoriaAddress& otherBase) const override {
		const UdpAddress& other = static_cast<const UdpAddress&> (otherBase);
		return ip == other.ip && port == other.port;
	}

	virtual void clone (const SensoriaAddress& otherBase) override {
		const UdpAddress& other = static_cast<const UdpAddress&> (otherBase);
		ip = other.ip;
		port = other.port;
	}

	// Default copy/assignment operators should be fine
};

#endif
