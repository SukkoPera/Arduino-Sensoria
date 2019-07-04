/*
 * Time_NTP.pde
 * Example showing time sync to NTP time source
 *
 * This sketch uses the Ethernet library
 */

#include <TimeLib.h>
#include <Sensoria.h>
#include <SensoriaClient/SensoriaClient.h>
#include <SensoriaStereotypes/DateTimeData.h>
#include <SensoriaCommunicators/ESPStandAlone.h>

const int timeZone = 1;     // Central European Time
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)

// Wi-Fi parameters
#define WIFI_SSID        "ssid"
#define WIFI_PASSWORD    "password"

ESPCommunicator comm;
SensoriaClient client;


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

void setup()
{
	Serial.begin(9600);
	while (!Serial); // Needed for Leonardo only
	delay(250);
	Serial.println("SensoriaTime Example");

	if (!comm.begin (WIFI_SSID, WIFI_PASSWORD)) {
		mypanic (100);
	}

	client.begin (comm, true);

	//~ UdpAddress* outdoor1 = comm.getAddress (192, 168, 1, 152, 9999);
	//~ while (!client.registerNode (outdoor1)) {
		//~ Serial.println (F("Cannot register outdoor1 node"));
			//~ delay (1000);
	//~ }
	//~ Serial.println (F("Node outdoor1 registered"));

	Serial.println("Waiting for sync...");
	setSyncProvider(getSensoriaTime);
}

time_t prevDisplay = 0; // when the digital clock was displayed

void loop()
{
	if (timeStatus() != timeNotSet) {
		if (now() != prevDisplay) { //update the display only if time has changed
			prevDisplay = now();
			digitalClockDisplay();
		}
	}

	client.loop ();
}

void digitalClockDisplay(){
	// digital clock display of the time
	Serial.print(hour());
	printDigits(minute());
	printDigits(second());
	Serial.print(" ");
	Serial.print(day());
	Serial.print(" ");
	Serial.print(month());
	Serial.print(" ");
	Serial.print(year());
	Serial.println();
}

void printDigits(int digits){
	// utility for digital clock display: prints preceding colon and leading 0
	Serial.print(":");
	if(digits < 10)
		Serial.print('0');
	Serial.print(digits);
}

time_t getSensoriaTime () {
	time_t ret = 0;

	SensorProxy *sensor = client.getSensor ("$T");
	if (sensor) {
		Stereotype *st;
		if (!(sensor -> read (st))) {
			Serial.println (F("Read FAILED"));
		} else {
			DateTimeData& data = *static_cast<DateTimeData *> (st);
			if (data.unixtime > 0) {
				ret = data.unixtime;
			}
		}
	} else {
		Serial.println (F("Clock sensor not found"));
	}

	return ret;
}

