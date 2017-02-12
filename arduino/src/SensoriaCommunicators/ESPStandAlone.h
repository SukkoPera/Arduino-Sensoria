
#ifdef ARDUINO_ARCH_ESP8266

#include <Sensoria.h>
#include <SensoriaCore/Communicator.h>
#include <SensoriaCore/common.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SensoriaCore/debug.h>

#define HOST_NAME   "0"
#define IN_BUF_SIZE 512

class ESPCommunicator: public SensoriaCommunicator {
private:
	WiFiUDP udpMain;
	WiFiUDP udpNot;

	uint8_t buffer[IN_BUF_SIZE];

public:
	ESPCommunicator () {
	}

	boolean begin (const char *_ssid, const char *_password, int channels = CC_SERVER) {
		DPRINT ("Connecting to SSID: ");
		DPRINT (_ssid);
		WiFi.mode (WIFI_STA);
		WiFi.begin (_ssid, _password);
		while (WiFi.status () != WL_CONNECTED) {
			DPRINT (".");
			delay (500);
		}
		DPRINTLN (" Connected!");

		DPRINT ("IP address: ");
		DPRINTLN (WiFi.localIP ());

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

	boolean receiveGeneric (WiFiUDP& udp, char **str, IPAddress *senderAddr, uint16_t *senderPort) {
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

	//~ boolean stop () {
		//~ if (!WiFi.unregisterUDP ()) {
			//~ DPRINT ("unregister udp err\r\n");
			//~ return false;
		//~ } else {
			//~ return true;
		//~ }
	//~ }
};

#endif    // PLATFORM_ESP8266
