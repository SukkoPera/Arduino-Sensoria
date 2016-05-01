#if 1

#include <Sensoria.h>
#include <SensoriaCore/Communicator.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>
#include <SensoriaCore/debug.h>

#define IN_BUF_SIZE 192

/* This uses Bruno Portaluri's WiFiEsp library:
 * https://github.com/bportaluri/WiFiEsp
 */
class SensoriaEsp8266Communicator: public SensoriaCommunicator {
private:
	//~ Stream *serial;

	//~ const char *ssid;
	//~ const char *password;

	uint8_t buffer[IN_BUF_SIZE];

public:
	WiFiEspUDP udp;

	boolean begin (Stream& _serial, const char *_ssid, const char *_password) {
    //~ serial = &_serial;
    //~ ssid = _ssid;
    //~ password = _password;

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
      DPRINTLN (ssid);
      status = WiFi.begin (const_cast<char *> (_ssid), _password);
    } while (status != WL_CONNECTED);

    //~ if (wifi.joinAP (ssid, password)) {
      DPRINTLN (F("Joined AP"));
      //~ DPRINT (F("IP: "));
      //~ DPRINTLN (wifi.getLocalIP().c_str());
    //~ } else {
      //~ DPRINTLN (F("Join AP failure"));
      //~ return false;
    //~ }

    // 0 will have us choose a random port for outgoing messages
    //~ if (!wifi.registerUDP (HOST_NAME, 0, INPUT_PORT, 2)) {
      //~ DPRINTLN (F("register udp err"));
      //~ return false;
    //~ }

    DPRINT (F("IP address: "));
    DPRINTLN (WiFi.localIP ());

    // FIXME: Make this separate, we don't need it for clients
    if (!udp.begin (DEFAULT_PORT)) {
      DPRINTLN (F("Cannot setup listening socket"));
      return false;
    }

    return true;
  }

	//~ boolean send (const char *str) override {
    //~ udp.beginPacket (udp.remoteIP (), udp.remotePort ());
    //~ udp.write ((const uint8_t *) str, strlen (str));
    //~ udp.endPacket ();

    //~ // FIXME
    //~ return true;
	//~ }

  boolean send (const char *str, IPAddress& dest, uint16_t port) override {
    udp.beginPacket (dest, port);
    udp.write ((const uint8_t *) str, strlen (str));
    udp.endPacket ();

    // FIXME
    return true;
  }

  boolean receiveString (char **str, IPAddress *senderAddr, uint16_t *senderPort) override {
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

	//~ boolean stop () override {
		//~ if (!wifi.unregisterUDP ()) {
			//~ DPRINT ("unregister udp err\r\n");
			//~ return false;
		//~ } else {
			//~ return true;
		//~ }
	//~ }
};

#endif
