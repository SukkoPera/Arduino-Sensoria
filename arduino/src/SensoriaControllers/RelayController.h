#include <Sensoria.h>
#include <SensoriaCore/common.h>
#include <SensoriaCore/debug.h>
//~ #include <SensoriaClient/TransducerProxy.h>
#include <SensoriaCore/Actuator.h>
#include <SensoriaClient/NotificationManager.h>
#include <SensoriaStereotypes/AllStereotypes.h>

template <typename T>
class RelayController: public NotificationReceiver<T> {
protected:
	ControlledRelay *r;
	TransducerProxy *t;

	boolean onNotification (T& data) override {
		enableRelay (mustEnable (data));

		return true;
	}

	void enableRelay (boolean enabled) {
		ControlledRelayData rdata;
		if (this -> r -> read (rdata)) {
			if (rdata.controller == ControlledRelayData::CTRL_AUTO) {
				// R is under our control
				ControlledRelayData::State newState = enabled ? ControlledRelayData::STATE_ON : ControlledRelayData::STATE_OFF;
				if (newState != rdata.state) {
					rdata.state = newState;
					if (!this -> r -> write (rdata)) {
						DPRINTLN (F("Failed write to relay actuator"));
					}
				}
			//~ } else {
				//~ DPRINTLN (F("Relay is under manual control"));
			}
		} else {
			DPRINTLN (F("Relay actuator cannot be read"));
		}
	}

	virtual boolean toManual () {
		return true;
	}

	virtual boolean toAuto () {
		return true;
	}

	virtual boolean mustEnable (T& data) = 0;

public:
	virtual void begin (ControlledRelay& _r, TransducerProxy& _t) {
		r = &_r;
		t = &_t;

		// Set initial state
		T& data = *static_cast<T *> (this -> t -> read ());
		enableRelay (mustEnable (data));
	}
};

template <typename T, unsigned long TIME_SEC>
class DelayedRelayController: public RelayController<T> {
private:
	boolean enabled;
	unsigned long startTime;

public:
	void begin (ControlledRelay& _r, TransducerProxy& _t) override {
		this -> r = &_r;
		this -> t = &_t;
		startTime = 0;

		// Set initial state
		T& data = *static_cast<T *> (this -> t -> read ());
		enabled = this -> mustEnable (data);
	}

	void loop () {
		if (startTime != 0 && millis () - startTime >= TIME_SEC * 1000UL) {
			enabled = !enabled;
			startTime = 0;
		}

		this -> enableRelay (enabled);
	}

protected:
	boolean onNotification (T& data) override {
		if (this -> mustEnable (data) != enabled) {
			if (startTime == 0) {
				// Instant when we first detect a status change
				startTime = millis ();
			}
		} else {
			startTime = 0;
		}

		return true;
	}
};

// 'float' is not a valid type for a template non-type parameter
template <int TEMP>
class OverTempController: public RelayController<WeatherData> {
protected:
	boolean mustEnable (WeatherData& data) override {
		Serial.println (F("Received WeatherData notification:"));
		if (data.temperature != WeatherData::UNDEFINED) {
			Serial.print (F("- Temp = "));
			Serial.println (data.temperature);
		}
		if (data.humidity != WeatherData::UNDEFINED) {
			Serial.print (F("- Humidity = "));
			Serial.println (data.humidity);
		}
		if (data.localPressure != WeatherData::UNDEFINED) {
			Serial.print (F("- Local Pressure = "));
			Serial.println (data.localPressure);
		}

		return data.temperature >= TEMP;
	}
};

template <int LUX_THRES>
class LowLightController: public DelayedRelayController<WeatherData, 10> {
protected:
	boolean mustEnable (WeatherData& data) override {
		//~ if (data.light10bit != WeatherData::UNDEFINED) {
			//~ Serial.print (F("- Light 10bit = "));
			//~ Serial.println (data.light10bit);
		//~ }
		return data.light10bit <= LUX_THRES;
	}
};
