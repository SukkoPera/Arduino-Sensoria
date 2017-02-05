#ifndef _SENSOR_H_INCLUDED
#define _SENSOR_H_INCLUDED

#include "Transducer.h"
#include "common.h"

template <typename ST>
class Sensor: public TransducerT<ST> {
protected:
	Sensor (): TransducerT<ST> (Transducer::SENSOR) {
	}

	virtual void configure (const char *name _UNUSED, char *value _UNUSED) {
	}

	// Sensors are never writable
	boolean writeGeneric (Stereotype* st) override {
		(void) st;
		return false;
	}
};

#endif
