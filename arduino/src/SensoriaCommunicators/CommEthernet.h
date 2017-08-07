#if 1

#include <Sensoria.h>
#include <SensoriaCore/Communicator.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SensoriaCore/debug.h>

#define IN_BUF_SIZE 192

class SensoriaEthernetCommunicator: public SensoriaCommunicator {
private:
	uint8_t buffer[IN_BUF_SIZE];

public:
	EthernetUDP udpMain;
	EthernetUDP udpNot;

	boolean begin (byte mac[], int channels = CC_SERVER) {
		// Check for the presence of ESP
		if (!Ethernet.begin (mac)) {
			DPRINTLN (F("Ethernet Shield not found"));
			return false;
		}

		DPRINT (F("IP address: "));
		DPRINTLN (Ethernet.localIP ());

		/* Clients don't really need this, but how can we make an output-only
		 * socket?
		 */
		if (channels & CC_SERVER) {
			if (!udpMain.begin (DEFAULT_PORT)) {
				DPRINTLN (F("Cannot setup main listening socket"));
				return false;
			}
		}

		/* This can be enabled at will if you want to be able to RECEIVE
		 * notifications
		 */
		if (channels & CC_NOTIFICATIONS) {
			if (!udpNot.begin (DEFAULT_NOTIFICATION_PORT)) {
				DPRINTLN (F("Cannot setup notification listening socket"));
				return false;
			} else {
				DPRINT (F("Notification listening socket setup on port "));
				DPRINTLN (DEFAULT_NOTIFICATION_PORT);
			}
		}

		return true;
	}

	boolean send (const char *str, IPAddress& dest, uint16_t port) override {
		udpMain.beginPacket (dest, port);
		udpMain.write ((const uint8_t *) str, strlen (str));
		udpMain.endPacket ();

		// FIXME
		return true;
	}

  boolean broadcast (const char *str, uint16_t port) {
    // FIXME
  }

	boolean receiveGeneric (UDP& udp, char **str, IPAddress *senderAddr, uint16_t *senderPort) {
		// Assume we'll receive nothing
		boolean ret = false;

		int packetSize = udp.parsePacket ();
		if (packetSize) {
			*senderAddr = udp.remoteIP ();
			*senderPort = udp.remotePort ();

			// read the packet into packetBufffer
			int len = udp.read (buffer, IN_BUF_SIZE - 1);
			if (len > 0) {
				buffer[len] = '\0';  // Ensure command is a valid string
				*str = reinterpret_cast<char *> (buffer);
			}
#if 0
			DPRINT (F("Received packet of size "));
			DPRINT (packetSize);
			DPRINT (F(" from "));
			DPRINT (*senderAddr);
			DPRINT (F(", port "));
			DPRINT (*senderPort);
			DPRINT (F(": \""));
			DPRINT (*str);
			DPRINTLN (F("\""));
#endif
			ret = true;
		}

		return ret;
	}

	boolean receiveString (char **str, IPAddress *senderAddr, uint16_t *senderPort, SensoriaChannel channel) override {
		boolean ret = false;
		switch (channel) {
			case CC_SERVER:
				ret = receiveGeneric (udpMain, str, senderAddr, senderPort);
				break;
			case CC_NOTIFICATIONS:
				ret = receiveGeneric (udpNot, str, senderAddr, senderPort);
				break;
			default:
				break;
		}

		return ret;
	}
};

#endif
