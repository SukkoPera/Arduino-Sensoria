#ifdef ARDUINO_ARCH_ESP8266

#include <Sensoria.h>
#include <SensoriaCore/Communicator.h>
#include <SensoriaCore/common.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SensoriaCore/debug.h>

#define HOST_NAME   "0"
#define IN_BUF_SIZE 512

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

/******************************************************************************/

#define BROADCAST_ADDRESS 192,168,1,255

class ESPCommunicator: public SensoriaCommunicator {
private:
	static const byte N_ADDRESSES = 16;
	UdpAddress addressPool[N_ADDRESSES];

	static const uint16_t DEFAULT_NOTIFICATION_PORT = 9998;

	WiFiUDP udpMain;
	WiFiUDP udpNot;

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

	boolean sendGeneric (const char *str, IPAddress& dest, uint16_t port) {
		int ret = udpMain.beginPacket (dest, port);
		if (ret) {
			udpMain.write (reinterpret_cast<const uint8_t *> (str), strlen (str));
			ret = udpMain.endPacket ();
		}

		return true;
	}

public:
	//~ ESPCommunicator () {
	//~ }

	SensoriaAddress* getAddress () override {
		SensoriaAddress* ret = NULL;

#if 0
		byte cnt = 0;
		for (byte i = 0; i < N_ADDRESSES && !ret; i++) {
			if (!addressPool[i].inUse)
				++cnt;
		}
		DPRINT (F("Addresses not in use: "));
		DPRINTLN (cnt);
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

	virtual SensoriaAddress* getNotificationAddress (const SensoriaAddress* client) override {
		UdpAddress* addr = reinterpret_cast<UdpAddress*> (getAddress ());
		if (addr) {
			const UdpAddress& clientUdpAddr = *reinterpret_cast<const UdpAddress*> (client);
			addr -> ip = clientUdpAddr.ip;
			addr -> port = DEFAULT_NOTIFICATION_PORT;
		}

		return addr;
	}

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

		// Notifications use a fixed port, not the one we have in addr
		return sendGeneric (notification, addr.ip, DEFAULT_NOTIFICATION_PORT) ? SEND_OK : SEND_ERR;
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
    bcAddr.ip = IPAddress (BROADCAST_ADDRESS);
    bcAddr.port = DEFAULT_BROADCAST_PORT;
    SendResult ret = this -> reply (cmd, &bcAddr);
    if (ret == SEND_OK) {
      lastBroadcastTime = millis ();
    }

    return ret;
  }

  boolean receiveBroadcastReply (char*& reply, SensoriaAddress*& sender, unsigned int timeout) override {
    boolean ret = false;

    while (!ret && millis () - lastBroadcastTime < timeout) {
      UdpAddress addr;
      ret = receiveGeneric (udpMain, reply, addr.ip, addr.port);
      if (ret) {
        // Got something
        UdpAddress* senderUdp = reinterpret_cast<UdpAddress*> (getAddress ());
        if (senderUdp) {
          *senderUdp = addr;
          sender = senderUdp;
        } else {
          DPRINTLN (F("Cannot allocate address for broadcast reply"));
          ret = false;
        }
      }
    }

		return ret;
	}

	boolean receiveNotification (char*& notification) override {
		return false;
	}
};

#endif    // PLATFORM_ESP8266
