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
	void begin (Actuator& _r) {
		r = &_r;
	}

protected:
	boolean onNotification (T& data) {
		ControlledRelayData rdata;
		if (r -> read (&rdata)) {
			if (rdata.controller == ControlledRelayData::CTRL_AUTO) {
				// R is under our control
				ControlledRelayData::State newState = mustEnable (data) ? ControlledRelayData::STATE_ON : ControlledRelayData::STATE_OFF;
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
	unsigned long timeFirstOverThreshold;

protected:
	boolean checkEnable (T& data) {
		boolean ret = false;

		if (mustEnable (data)) {
			if (timeFirstOverThreshold == 0) {
				// First over thres
				timeFirstOverThreshold = millis ();
			} else if (millis () - timeFirstOverThreshold >= TIME_SEC * 1000) {
				ret = true;
			}
		} else {
			timeFirstOverThreshold = 0;
		}

		return ret;
	}
};

class WeatherDataRelayController: public RelayController<WeatherData> {
protected:
	boolean mustEnable (WeatherData& data) {
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

		return data.temperature >= 25;
	}
};

// 'float' is not a valid type for a template non-type parameter
template <int TEMP>
class OverTempController: public RelayController<WeatherData> {
protected:
	boolean mustEnable (WeatherData& data) {
		return data.temperature >= TEMP;
	}
};

template <int LUX_THRES>
class UnderLuxController: public DelayedRelayController<WeatherData, 10> {
protected:
	boolean mustEnable (WeatherData& data) {
		return data.lightLux <= LUX_THRES;
	}
};

class DarkController: public UnderLuxController<0> {
};
