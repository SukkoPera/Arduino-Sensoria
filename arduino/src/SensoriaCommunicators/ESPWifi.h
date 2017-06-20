#include <Sensoria.h>
#include <SensoriaCore/Communicator.h>
#include <SensoriaCore/common.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>
#include <SensoriaCore/debug.h>

#define IN_BUF_SIZE 192

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
				strncat (buf, tmp, size);
				strncat (buf, ".", size);
		}

		// Replace last dot with a colon
		buf[strlen (buf) - 1] = ':';

		// Append port
		utoa (port, tmp, 10);
		strncat (buf, tmp, size);

		return buf;
	}

	// Default copy operator should be fine
};

/* This uses Bruno Portaluri's WiFiEsp library:
 * https://github.com/bportaluri/WiFiEsp
 */
class SensoriaEsp8266Communicator: public SensoriaCommunicator {
private:
	static const byte N_ADDRESSES = 4;
	UdpAddress addressPool[N_ADDRESSES];

	uint8_t buffer[IN_BUF_SIZE];

	/* 255.255.255.255 does not seem to work, see:
	 * https://github.com/bportaluri/WiFiEsp/issues/95
	 */
#define BROADCAST_ADDRESS 192, 168, 1, 255

	boolean receiveGeneric (WiFiEspUDP& udp, char*& str, IPAddress& senderAddr, uint16_t& senderPort) {
		// Assume we'll receive nothing
		boolean ret = false;

		int packetSize = udp.parsePacket ();
		if (packetSize) {
			senderAddr = udp.remoteIP ();
			senderPort = udp.remotePort ();

			// read the packet into packetBufffer
			int len = udp.read (buffer, IN_BUF_SIZE - 1);
			if (len > 0) {
				buffer[len] = '\0';  // Ensure command is a valid string
				str = reinterpret_cast<char *> (buffer);
			}
#if 0
			DPRINT (F("Received packet of size "));
			DPRINT (packetSize);
			DPRINT (F(" from "));
			DPRINT (senderAddr);
			DPRINT (F(", port "));
			DPRINT (senderPort);
			DPRINT (F(": \""));
			DPRINT (str);
			DPRINTLN (F("\""));
#endif
			ret = true;
		}

		return ret;
	}

public:
	WiFiEspUDP udpMain;
#ifdef ENABLE_NOTIFICATIONS
	WiFiEspUDP udpNot;
#endif

	SensoriaAddress* getAddress () override {
		SensoriaAddress* ret = NULL;

		for (byte i = 0; i < N_ADDRESSES && !ret; i++) {
			if (!addressPool[i].inUse) {
				addressPool[i].inUse = true;
				ret = &(addressPool[i]);
			}
		}

		return ret;
	}

	void releaseAddress (SensoriaAddress* addr) override {
		for (byte i = 0; i < N_ADDRESSES; i++) {
			if (&(addressPool[i]) == addr) {
				addressPool[i].inUse = false;
			}
		}
	}

	boolean begin (Stream& _serial, const char *_ssid, const char *_password, int channels = CC_SERVER) {
		for (byte i = 0; i < N_ADDRESSES; i++) {
			addressPool[i].inUse = false;
		}

		WiFi.init (&_serial);

		// Check for the presence of ESP
		if (WiFi.status() == WL_NO_SHIELD) {
			DPRINTLN (F("ESP8266 not found"));
			return false;
		}

		DPRINT (F("FW Version:"));
		DPRINTLN (WiFi.firmwareVersion ());

		// FIXME: Add max attempts
		int status;
		do {
			DPRINT (F("Connecting to AP: "));
			DPRINTLN (_ssid);
			status = WiFi.begin (const_cast<char *> (_ssid), _password);
		} while (status != WL_CONNECTED);
		DPRINTLN (F("Joined AP"));

		DPRINT (F("IP address: "));
		DPRINTLN (WiFi.localIP ());

		// Clients don't need this
		if (channels & CC_SERVER) {
			if (!udpMain.begin (DEFAULT_PORT)) {
				DPRINTLN (F("Cannot setup main listening socket"));
				return false;
			}
		}

		/* This can be enabled at will if you want to be able to RECEIVE
		 * notifications
		 */
#ifdef ENABLE_NOTIFICATIONS
		if (channels & CC_NOTIFICATIONS) {
			if (!udpNot.begin (DEFAULT_NOTIFICATION_PORT)) {
				DPRINTLN (F("Cannot setup notification listening socket"));
				return false;
			} else {
				DPRINT (F("Notification listening socket setup on port "));
				DPRINTLN (DEFAULT_NOTIFICATION_PORT);
			}
		}
#endif

		return true;
	}

	boolean receiveCmd (char*& cmd, SensoriaAddress* client) override {
		UdpAddress& addr = *reinterpret_cast<UdpAddress*> (client);
		return receiveGeneric (udpMain, cmd, addr.ip, addr.port);
	}

	SendResult reply (const char* reply, const SensoriaAddress* client) override {
		const UdpAddress& addr = *reinterpret_cast<const UdpAddress*> (client);

		int ret = udpMain.beginPacket (addr.ip, addr.port);
		if (ret) {
			udpMain.write (reinterpret_cast<const uint8_t *> (reply), strlen (reply));
			ret = udpMain.endPacket ();
		}

		return ret ? SEND_OK : SEND_ERR;
	}

	boolean notify (const char* notification, const SensoriaAddress* client) override {
		return false;
	}

	SendResult sendCmd (const char* cmd, const SensoriaAddress& server, char*& reply) override {
		return SEND_ERR;
	}

	SendResult broadcast (const char* cmd, char*& reply, unsigned int replyTimeout) override {
		//~ unsigned long startTime = millis ();
		//~ while (millis () - startTime < DISCOVERY_TIMEOUT) {
		return SEND_ERR;
	}

	boolean receiveNotification (char*& notification) override {
		return false;
	}

#if 0
	boolean broadcast (const char *str, uint16_t port) override {
		IPAddress broadcastIp (BROADCAST_ADDRESS);
		return send (str, broadcastIp, port);
	}

	boolean receiveString (char **str, IPAddress *senderAddr, uint16_t *senderPort, SensoriaChannel channel) override {
		boolean ret = false;
		switch (channel) {
			case CC_SERVER:
				ret = receiveGeneric (udpMain, str, senderAddr, senderPort);
				break;
#ifdef ENABLE_NOTIFICATIONS
			case CC_NOTIFICATIONS:
				ret = receiveGeneric (udpNot, str, senderAddr, senderPort);
				break;
#endif
			default:
				break;
		}

		return ret;
	}

	//~ boolean stop () override {
		//~ if (!wifi.unregisterUDP ()) {
			//~ DPRINT ("unregister udp err\r\n");
			//~ return false;
		//~ } else {
			//~ return true;
		//~ }
	//~ }
#endif
};
