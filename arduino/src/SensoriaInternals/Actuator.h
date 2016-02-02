#ifndef _ACTUATOR_H_INCLUDED
#define _ACTUATOR_H_INCLUDED

#include "Transducer.h"
//~ #include <SensoriaInternals/utils.h>


class Actuator: public Transducer {
private:

public:
	Actuator (): Transducer (Transducer::ACTUATOR) {
	}

	/* Override to implement the actual Actuator writing and reporting.
	 */
	virtual bool write (char *buf) = 0;

	virtual void configure (const char *name _UNUSED, char *value _UNUSED) {
	}
};

#endif
