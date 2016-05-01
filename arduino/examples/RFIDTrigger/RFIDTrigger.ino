#include <SPI.h>
#include <MFRC522.h>

// RC522 RFID Reader Pin Definition
#define RST_PIN         9
#define SS_PIN          10

MFRC522 mfrc522 (SS_PIN, RST_PIN);

// Size of "good" key
#define KEY_SIZE 4

// Good key
byte GOOD_KEY[KEY_SIZE] = {0x85, 0xB1, 0x15, 0x53};


#include <SoftwareSerial.h>
#include <Sensoria.h>
#include <SensoriaClient/SensoriaClient.h>
#include <SensoriaStereotypes/RelayData.h>
#include <SensoriaCommunicators/ESPWifiAlt.h>

IPAddress relayNodeAddr (192, 168, 1, 173);

// Wi-Fi parameters
#define WIFI_SSID        "ssid"
#define WIFI_PASSWORD    "password"

#define RELAY_ACTNAME "R1"

SoftwareSerial swSerial (7, 8);
SensoriaEsp8266Communicator comm;
SensoriaClient client;


ActuatorProxy *relayActuator = NULL;
boolean relayState;


#define LED_PIN LED_BUILTIN

void mypanic (int interval) {
  pinMode (LED_PIN, OUTPUT);
  while (42) {
    digitalWrite (LED_PIN, HIGH);
    delay (interval);
    digitalWrite (LED_PIN, LOW);
    delay (interval);
  }
}

void setup () {
  Serial.begin (9600);

  //~ Serial3.begin (9600);
  swSerial.begin (9600);
	if (!comm.begin (swSerial, WIFI_SSID, WIFI_PASSWORD)) {
		Serial.println (F("WiFi error"));
		mypanic (100);
	}

  client.begin (comm);
	while (!client.registerNode (relayNodeAddr)) {
		Serial.println (F("Cannot register Relay Node"));
    delay (1000);
	}
  Serial.println (F("Relay Node registered"));

	relayActuator = client.getActuator (RELAY_ACTNAME);
	if (!relayActuator) {
		mypanic (2000);
	}
  Stereotype *st;
  if (!(st = relayActuator -> read ())) {
		Serial.println (F("Relay state read failed"));
	} else {
		RelayData &data = *static_cast<RelayData *> (st);
    relayState = data.state == RelayData::STATE_ON;
    Serial.print (F("Initial relay state = "));
    Serial.println (relayState ? F("ON") : F("OFF"));
	}

  // Init RFID reader
  SPI.begin ();			// Init SPI bus
	mfrc522.PCD_Init ();		// Init MFRC522
	mfrc522.PCD_DumpVersionToSerial ();
  Serial.println (F("RFID Reader inited"));
}

void loop () {
  static MFRC522::Uid lastUid = {0, {0}, 0};
  static unsigned long lastCardDetected = 0;

	// Look for new cards
	if (!mfrc522.PICC_IsNewCardPresent ()) {
    if (millis () - lastCardDetected >= 1000) {
      lastUid.size = 0;     // Enought to consider the next card new
    }
		return;
	}

	// Select one of the cards
	if (!mfrc522.PICC_ReadCardSerial ()) {
		return;
	}

	// Dump debug info about the card; PICC_HaltA() is automatically called
	//mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

  if (mfrc522.uid.size == KEY_SIZE) {
    if (!compareUid (lastUid, mfrc522.uid)) {
      // New key presented!
      if (compareUid (mfrc522.uid, GOOD_KEY, KEY_SIZE)) {
        Serial.println ("GOOD!");
        toggleRelay ();
      } else {
        Serial.println ("Bad!");
      }

      lastUid = mfrc522.uid;
    }

    lastCardDetected = millis ();
  }
}


void toggleRelay () {
  // Toggle remote relay
  RelayData rd;
  if (relayState) {
    rd.state = RelayData::STATE_OFF;
  } else {
    rd.state = RelayData::STATE_ON;
  }
  relayActuator -> write (rd);
  relayState = !relayState;
  Serial.print (F("Relay is now: "));
  Serial.println (relayState ? F("ON") : F("OFF"));
}



boolean compareUid (byte *k1, byte *k2, byte size) {
  boolean match = true;
  for (int i = 0; match && i < size; i++) {
    if (k1[i] != k2[i])
      match = false;
  }

  return match;
}

boolean compareUid (MFRC522::Uid& k1, MFRC522::Uid& k2) {
  return k1.size == k2.size && compareUid (k1.uidByte, k2.uidByte, k1.size);
}

boolean compareUid (MFRC522::Uid& k1, byte *k2, byte size) {
  return k1.size == size && compareUid (k1.uidByte, k2, k1.size);
}

