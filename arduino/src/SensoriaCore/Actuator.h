#ifndef _ACTUATOR_H_INCLUDED
#define _ACTUATOR_H_INCLUDED

#include "Transducer.h"
//~ #include <SensoriaCore/utils.h>

template <typename ST>
class Actuator: public TransducerT<ST> {
private:
	virtual boolean writeGeneric (Stereotype* st) override {
		ST* tst = static_cast<ST*> (st);
		return write (*tst);
	}

protected:
	Actuator (): TransducerT<ST> (Transducer::ACTUATOR) {
	}

	/* Override to implement the actual Actuator writing.
	 */
	virtual boolean write (ST& st) = 0;

	/* Since not all actuators might be interested in this, provide a do-nothing
	 * default, but keep this virtual.
	 */
	virtual boolean read (ST& st) override {
		(void) st;
		return false;
	}

	virtual void configure (const char *name _UNUSED, char *value _UNUSED) {
	}
};

#endif
