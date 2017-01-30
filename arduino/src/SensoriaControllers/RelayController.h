#include <Sensoria.h>
#include <SensoriaCore/common.h>
#include <SensoriaCore/debug.h>
//~ #include <SensoriaClient/TransducerProxy.h>
#include <SensoriaCore/Actuator.h>
#include <SensoriaClient/NotificationManager.h>
#include <SensoriaStereotypes/AllStereotypes.h>

template <typename T>
class RelayController: public NotificationReceiver<T> {
private:
	Actuator *r;

public:
	virtual void begin (Actuator& _r) {
		r = &_r;
	}

protected:
	boolean onNotification (T& data) override {
		ControlledRelayData rdata;
		if (r -> read (&rdata)) {
			if (rdata.controller == ControlledRelayData::CTRL_AUTO) {
				// R is under our control
				ControlledRelayData::State newState = checkEnable (data) ? ControlledRelayData::STATE_ON : ControlledRelayData::STATE_OFF;
				if (newState != rdata.state) {
					rdata.state = newState;
					r -> write (&rdata);
				}
			} else {
				DPRINTLN (F("Ignoring notification as relay is under manual control"));
			}
		} else {
			DPRINTLN (F("Ignoring notification as relay actuator cannot be read"));
		}

		return true;
	}

	virtual boolean checkEnable (T& data) {
		return mustEnable (data);
	}

	virtual boolean toManual () {
		return true;
	}

	virtual boolean toAuto () {
		return true;
	}

	virtual boolean mustEnable (T& data) = 0;
};

template <typename T, unsigned long TIME_SEC>
class DelayedRelayController: public RelayController<T> {
private:
	boolean enabled;
	unsigned long startTime;

public:
	void begin (Actuator& _r) override {
		RelayController<T>::begin (_r);
		enabled = false;
		startTime = 0;
	}

protected:
	boolean checkEnable (T& data) override {
		if (this -> mustEnable (data) != enabled) {
			if (startTime == 0) {
				// First over thres
				startTime = millis ();
			} else if (millis () - startTime >= TIME_SEC * 1000UL) {
				enabled = !enabled;
				startTime = 0;
			}
		} else {
			startTime = 0;
		}

		return enabled;
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
class LowLightController: public DelayedRelayController<WeatherData, 5> {
protected:
	boolean mustEnable (WeatherData& data) override {
		return data.light10bit <= LUX_THRES;
	}
};
