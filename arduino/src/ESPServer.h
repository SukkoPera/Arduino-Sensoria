#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "SensoriaServer.h"

#define HOST_NAME   "0"
#define INPUT_PORT 9999
#define IN_BUF_SIZE 64

class ESPServer: public SensoriaServer {
private:
	WiFiUDP udp;

	const char *ssid;
	const char *password;

	uint8_t buffer[IN_BUF_SIZE];

public:
	ESPServer () {
	}

	boolean begin (FlashString _serverName, const char *_ssid, const char *_password) {
    if (SensoriaServer::begin (_serverName, F("20160102"))) {
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

      udp.begin (INPUT_PORT);

      return true;
    } else {
      return false;
    }
	}

	boolean send (const char *str) {
    udp.beginPacket (udp.remoteIP (), udp.remotePort ());
    udp.write ((const uint8_t *) str, strlen (str));
    udp.endPacket ();
	}

	boolean receive () {
    int packetSize = udp.parsePacket ();
    if (packetSize) {
      DPRINT ("Received packet of size ");
      DPRINT (packetSize);
      DPRINT (" from ");
      IPAddress remoteIp = udp.remoteIP ();
      DPRINT (remoteIp);
      DPRINT (", port ");
      DPRINTLN (udp.remotePort());

      // read the packet into packetBufffer
      int len = udp.read (buffer, IN_BUF_SIZE - 1);
      if (len > 0) {
        buffer[len] = '\0';  // Ensure command is a valid string
        char *charbuf = reinterpret_cast<char *> (buffer);

        DPRINT (F("Received:["));
        DPRINT (charbuf);
        DPRINT (F("]\r\n"));

        process_cmd (charbuf);
      }
		}

		return (packetSize);
	}

	boolean stop () {
		//~ if (!WiFi.unregisterUDP ()) {
			//~ DPRINT ("unregister udp err\r\n");
			//~ return false;
		//~ } else {
			//~ return true;
		//~ }
	}
};
