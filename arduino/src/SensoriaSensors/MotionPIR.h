#include <SensoriaCore/Sensor.h>
#include <SensoriaStereotypes/MotionData.h>

class MotionPIR: public Sensor<MotionData> {
private:
	byte pin;

public:
	bool begin (FlashString name, FlashString description, byte _pin) {
		pin = _pin;
		pinMode (pin, INPUT);

		return Sensor::begin (name, F("MD"), description, F("20170224"));
	}

	boolean read (MotionData& md) override {
		md.motionDetected = digitalRead (pin) == HIGH;
#if 0
		DPRINT (F("Motion detected: "));
		DPRINTLN (md.motionDetected ? F("YES") : F("NO"));
#endif
		return true;
	}
};
