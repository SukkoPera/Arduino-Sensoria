#include <SoftwareSerial.h>
#include <ESP8266.h>

#include "SensoriaServer.h"

#define HOST_NAME   "0"
#define INPUT_PORT 9999
#define IN_BUF_SIZE 64

class SensoriaWifiServer: public SensoriaServer {
private:
	SoftwareSerial swSerial;
	ESP8266 wifi;

	const char *ssid;
	const char *password;

	uint8_t buffer[IN_BUF_SIZE];

public:
	SensoriaWifiServer (const byte rx, const byte tx): swSerial (rx, tx), wifi (swSerial) {
	}

	boolean begin (FlashString _serverName, const char *_ssid, const char *_password) {
    SensoriaServer::begin (_serverName, F("20160202"));
    ssid = _ssid;
    password = _password;

		DPRINT (F("FW Version:"));
		DPRINTLN (wifi.getVersion ().c_str ());

		if (!wifi.setOprToStation ()) {
			DPRINTLN (F("to station + softap err\r\n"));
			return false;
		}

		if (wifi.joinAP (ssid, password)) {
			DPRINTLN (F("Join AP success"));
			DPRINT (F("IP: "));
			DPRINTLN (wifi.getLocalIP().c_str());
		} else {
			DPRINTLN (F("Join AP failure"));
			return false;
		}

		// 0 will have us choose a random port for outgoing messages
		if (!wifi.registerUDP (HOST_NAME, 0, INPUT_PORT, 2)) {
			DPRINTLN (F("register udp err"));
			return false;
		}

		return true;
	}

	boolean send (const char *str) {
		return wifi.send ((const uint8_t *) str, strlen (str));
	}

	boolean receive () {
		uint32_t len = wifi.recv (buffer, IN_BUF_SIZE - 1, 10000);
		if (len > 0) {
			buffer[len] = '\0';  // Ensure command is a valid string
			char *charbuf = reinterpret_cast<char *> (buffer);

			DPRINT (F("Received:["));
			//~ for (uint32_t i = 0; i < len; i++) {
				//~ DPRINT ((char) buffer[i]);
			//~ }
			DPRINT (charbuf);
			DPRINT (F("]\r\n"));

			process_cmd (charbuf);
		} else {
			DPRINTLN (F("No commands received in 10 secs"));
		}

		return (len > 0);
	}

	boolean stop () {
		if (!wifi.unregisterUDP ()) {
			DPRINT ("unregister udp err\r\n");
			return false;
		} else {
			return true;
		}
	}
};
