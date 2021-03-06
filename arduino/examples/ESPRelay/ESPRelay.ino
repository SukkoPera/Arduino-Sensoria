#include <Sensoria.h>

#include <SensoriaCommunicators/ESPStandAlone.h>
ESPCommunicator comm;

#include <SensoriaCore/Server.h>
SensoriaServer srv;

#include <SensoriaActuators/Relay.h>

/* On my board:
 * D0 is GPIO16 and controls the onboard led (with an inverted logic)
 * D3 is GPIO0 and is pulled to Vcc
 * D4 is GPIO2, controls the led on the ESP (inverted logic) module and is pulled to Vcc
 * D8 is GPIO15 and is pulled to GND
 * D9/D10 is serial RX/TX
 *
 * So let's just avoid all of those pins and we're left with 5 free GPIOs. If
 * you need more, have a look at this:
 * http://www.forward.com.au/pfod/ESP8266/GPIOpins/index.html
 */
#define N_RELAY 6
Relay relays[N_RELAY];
byte relayPins[N_RELAY] = {D0, D1, D2, D5, D6, D7};
const char* relayNames[N_RELAY] = {F("R0"), F("R1"), F("R2"), F("R3"), F("R4"), F("R5")};
#define INVERTED true

// Use the ESP led so we can use D0 for a relay
#define LED_PIN D4

// Wi-Fi parameters
#define WIFI_SSID        "ssid"
#define WIFI_PASSWORD    "password"


#define FLASH_EVERY (60 * 1000L)  // ms
#define FLASH_LEN 15  // ms
unsigned long lastFlashTime = 0;


void mypanic (int interval) {
  pinMode (LED_PIN, OUTPUT);
  while (42) {
    digitalWrite (LED_PIN, HIGH);
    delay (interval);
    digitalWrite (LED_PIN, LOW);
    delay (interval);
  }
}

void setup (void) {
  DSTART ();

  if (!comm.begin (WIFI_SSID, WIFI_PASSWORD)) {
    mypanic (100);
  }

  if (!srv.begin (F("ESPDallas-1"), comm)) {
    mypanic (500);
  }

  for (int i = 0; i < N_RELAY; i++) {
    if (relays[i].begin (relayNames[i], F("Relay"), relayPins[i], INVERTED)) {
      if (srv.addTransducer (relays[i]) >= 0) {
        DPRINT (F("Actuator registered: "));
        DPRINTLN (relays[i].name);
      } else {
        mypanic (1000);
      }
    } else {
      DPRINTLN (F("Actuator failed begin()"));
    }
  }

  // Signal we're ready!
  //Serial.println (F("GO!"));
  pinMode (LED_PIN, OUTPUT);
  for (int i = 0; i < 3; i++) {
    digitalWrite (LED_PIN, LOW);
    delay (100);
    digitalWrite (LED_PIN, HIGH);
    delay (100);
  }
}

void loop (void) {
  if (millis () - lastFlashTime >= FLASH_EVERY) {
    digitalWrite (LED_PIN, LOW);
    delay (FLASH_LEN);
    digitalWrite (LED_PIN, HIGH);
    lastFlashTime = millis ();
  }

  srv.loop ();
}
