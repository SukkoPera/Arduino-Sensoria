#if 0

THIS NEEDS TO BE FIXED AFTER THE SWITCH TO COMMUNICATOR CLASS!

#include <IPAddress.h>
#include <SensoriaCore/Communicator.h>
#include <ESP8266.h>

#define HOST_NAME   "0"
#define IN_BUF_SIZE 64
#define TIMEOUT 500

class SensoriaWifiServer: public SensoriaServer {
private:
	SoftwareSerial swSerial;
	ESP8266 *wifi;

	const char *ssid;
	const char *password;

	uint8_t buffer[IN_BUF_SIZE];

public:
	SensoriaWifiServer (): wifi (NULL) {
	}

	boolean begin (Stream& _serial, const char *_ssid, const char *_password) {
    SensoriaServer::begin (_serverName, F("20160202"));
    ssid = _ssid;
    password = _password;

    delay (2000);

		DPRINT (F("FW Version:"));
		DPRINTLN (wifi.getVersion ().c_str ());

		if (!wifi.setOprToStation ()) {
			DPRINTLN (F("to station err\r\n"));
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

	//~ boolean send (const char *str) {
		//~ return wifi.send ((const uint8_t *) str, strlen (str));
	//~ }

  // FIXME
	boolean send (const char *str, IPAddress& dest, uint16_t port) override {
    /* FIXME: Find a better way to convert IPAddress to string. This
     * uses ~2 kB of flash, because of the String class, which isn't
     * used anywhere else!
     */
    String addr = String(dest[0]) + "." +  String(dest[1]) + "." + \
      String(dest[2]) + "." + String(dest[3]);
    DPRINT (F("Opening UDP socket to "));
    DPRINT (addr);
    DPRINT (F(":"));
    DPRINTLN (port);

    if (!wifi.registerUDP (addr.c_str (), port)) {
      DPRINT (F("Cannot open UDP socket to "));
      DPRINT (addr);
      DPRINT (F(":"));
      DPRINTLN (port);
    }

    boolean ret = wifi.send ((const uint8_t *) str, strlen (str));

    //~ if (!wifi.unregisterUDP()) {
      //~ DPRINTLN(F("Cannot close UDP socket"));
    //~ }

    return ret;
	}

  boolean receiveString (char **str, IPAddress *senderAddr, uint16_t *senderPort) override {
		uint32_t len = wifi.recv (buffer, IN_BUF_SIZE - 1, TIMEOUT);
		if (len > 0) {
			buffer[len] = '\0';  // Ensure command is a valid string
			char *charbuf = reinterpret_cast<char *> (buffer);

			DPRINT (F("Received:["));
			//~ for (uint32_t i = 0; i < len; i++) {
				//~ DPRINT ((char) buffer[i]);
			//~ }
			DPRINT (charbuf);
			DPRINT (F("]\r\n"));
		} else {
			DPRINTLN (F("Timeout"));
		}

		return (len > 0);
	}

  char *receiveString () {
    char *ret;

		uint32_t len = wifi.recv (buffer, IN_BUF_SIZE - 1, TIMEOUT);
		if (len > 0) {
			buffer[len] = '\0';  // Ensure command is a valid string
			char *charbuf = reinterpret_cast<char *> (buffer);

			DPRINT (F("Received:["));
			//~ for (uint32_t i = 0; i < len; i++) {
				//~ DPRINT ((char) buffer[i]);
			//~ }
			DPRINT (charbuf);
			DPRINT (F("]\r\n"));

			ret = charbuf;
		} else {
			DPRINTLN (F("TIMEOUT"));
      ret = NULL;
		}

		return ret;
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

#endif
