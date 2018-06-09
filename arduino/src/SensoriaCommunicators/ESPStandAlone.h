#ifdef ARDUINO_ARCH_ESP8266

#include <Sensoria.h>
#include <SensoriaCore/Communicator.h>
#include <SensoriaCore/common.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SensoriaCommunicators/UdpAddress.h>
#include <SensoriaCore/debug.h>

#define HOST_NAME   "0"
#define IN_BUF_SIZE 512


class ESPCommunicator: public SensoriaCommunicator {
private:
	static const byte N_ADDRESSES = 16;
	UdpAddress addressPool[N_ADDRESSES];

	static const uint16_t DEFAULT_PORT = 9999;

	static const uint16_t DEFAULT_BROADCAST_PORT = DEFAULT_PORT;

	static const uint16_t DEFAULT_NOTIFICATION_PORT = 9998;

	WiFiUDP udpMain;

#ifdef ENABLE_NOTIFICATIONS
	WiFiUDP udpNot;
#endif

	uint8_t buffer[IN_BUF_SIZE];

	unsigned long lastBroadcastTime;

	boolean receiveGeneric (WiFiUDP& udp, char*& str, IPAddress& senderAddr, uint16_t& senderPort) {
		// Assume we'll receive nothing
		boolean ret = false;

		int packetSize = udp.parsePacket ();
		if (packetSize) {
			senderAddr = udp.remoteIP ();
			senderPort = udp.remotePort ();

			// Read the packet into packetBufffer
			int len = udp.read (buffer, IN_BUF_SIZE - 1);
			if (len > 0) {
				buffer[len] = '\0';  // Ensure command is a valid string
				str = reinterpret_cast<char *> (buffer);
			}
#ifdef DEBUG_COMMUNICATOR
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

	boolean sendGeneric (const char *str, IPAddress& dest, uint16_t port) {
		int ret = udpMain.beginPacket (dest, port);
		if (ret) {
			udpMain.write (reinterpret_cast<const uint8_t *> (str), strlen (str));
			ret = udpMain.endPacket ();
		}

		return ret;
	}

public:
	//~ ESPCommunicator () {
	//~ }

	SensoriaAddress* getAddress () override {
		SensoriaAddress* ret = NULL;

#ifdef DEBUG_COMMUNICATOR
		static unsigned long last = 0;
		if (last == 0 || millis () - last >= 5000) {
			byte cnt = 0;
			for (byte i = 0; i < N_ADDRESSES && !ret; i++) {
				if (!addressPool[i].inUse)
					++cnt;
			}
			DPRINT (F("Addresses not in use: "));
			DPRINTLN (cnt);

			last = millis ();
		}
#endif

		for (byte i = 0; i < N_ADDRESSES && !ret; i++) {
			if (!addressPool[i].inUse) {
				addressPool[i].inUse = true;
				ret = &(addressPool[i]);
			}
		}

		return ret;
	}

	UdpAddress* getAddress (byte ip1, byte ip2, byte ip3, byte ip4, uint16_t port) {
		UdpAddress* addr = reinterpret_cast<UdpAddress*> (getAddress ());
		if (addr) {
			addr -> ip = IPAddress (ip1, ip2, ip3, ip4);
			addr -> port = port;
		}

		return addr;
	}

	void releaseAddress (SensoriaAddress* addr) override {
		for (byte i = 0; i < N_ADDRESSES; i++) {
			if (&(addressPool[i]) == addr) {
				addressPool[i].inUse = false;
			}
		}
	}

#ifdef ENABLE_NOTIFICATIONS
	virtual SensoriaAddress* getNotificationAddress (const SensoriaAddress* client) override {
		UdpAddress* addr = reinterpret_cast<UdpAddress*> (getAddress ());
		if (addr) {
			const UdpAddress& clientUdpAddr = *reinterpret_cast<const UdpAddress*> (client);
			addr -> ip = clientUdpAddr.ip;
			addr -> port = DEFAULT_NOTIFICATION_PORT;
		}

		return addr;
	}
#endif

	/*****/

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
		UdpAddress& addr = *const_cast<UdpAddress*> (reinterpret_cast<const UdpAddress*> (client));
		return sendGeneric (reply, addr.ip, addr.port) ? SEND_OK : SEND_ERR;
	}

	boolean notify (const char* notification, const SensoriaAddress* client) override {
		UdpAddress& addr = *const_cast<UdpAddress*> (reinterpret_cast<const UdpAddress*> (client));
		return sendGeneric (notification, addr.ip, addr.port) ? SEND_OK : SEND_ERR;
	}

	SendResult sendCmd (const char* cmd, const SensoriaAddress* server, char*& reply) override {
		UdpAddress& srvUdpAddr = *const_cast<UdpAddress*> (reinterpret_cast<const UdpAddress*> (server));
		SendResult res = sendGeneric (cmd, srvUdpAddr.ip, srvUdpAddr.port) ? SEND_OK : SEND_ERR;
		if (res > 0) {
			unsigned long start = millis ();

			while (millis () - start < CLIENT_TIMEOUT) {
				UdpAddress addr;    // Dummy address
				if (receiveGeneric (udpMain, reply, addr.ip, addr.port)) {
					// Got something
					break;
				}
			}

			if (millis () - start >= CLIENT_TIMEOUT)
				res = SEND_TIMEOUT;
		}

		return res;
	}

	SendResult broadcast (const char* cmd) override {
		UdpAddress bcAddr;
		bcAddr.ip = IPAddress (((uint32_t) WiFi.localIP ()) | (~(uint32_t) WiFi.subnetMask ()));
		bcAddr.port = DEFAULT_BROADCAST_PORT;

#ifdef DEBUG_COMMUNICATOR
		char buf[32];
		bcAddr.toString (buf, sizeof (buf));
		DPRINT (F("Broadcast address is: "));
		DPRINTLN (buf);
#endif

		SendResult ret = this -> reply (cmd, &bcAddr);
		if (ret == SEND_OK) {
			lastBroadcastTime = millis ();
		}

		return ret;
	}

	boolean receiveBroadcastReply (char*& reply, SensoriaAddress*& sender, unsigned int timeout) override {
		boolean ret = false;

		while (!ret && millis () - lastBroadcastTime < timeout) {
			IPAddress ip;
			uint16_t port;
			ret = receiveGeneric (udpMain, reply, ip, port);
			if (ret) {
				// Got something
				UdpAddress* senderUdp = reinterpret_cast<UdpAddress*> (getAddress ());
				if (senderUdp) {
					senderUdp -> ip = ip;
					senderUdp -> port = port;			// Don't touch the inUse flag!
					sender = senderUdp;
				} else {
					DPRINTLN (F("Cannot allocate address for broadcast reply"));
					ret = false;
				}
			}
		}

		return ret;
	}

#ifdef ENABLE_NOTIFICATIONS
	boolean receiveNotification (char*& notification) override {
		IPAddress ip;
		uint16_t port;
		return receiveGeneric (udpNot, notification, ip, port);
	}
#endif
};

#endif    // PLATFORM_ESP8266
