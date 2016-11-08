#ifndef _TRANSDUCER_H_INCLUDED
#define _TRANSDUCER_H_INCLUDED

#include "Stereotype.h"
#include "utils.h"
#include "common.h"
#include "debug.h"

class Transducer {
public:
	enum Type {
		SENSOR,
		ACTUATOR,
		N_TYPES
	};

	Type type;
	FlashString name;
	FlashString stereotype;
	FlashString description;
	FlashString version;

	Transducer (Type _type): type (_type) {
		name = NULL;
		description = NULL;
		version = NULL;
		stereotype = NULL;
	}

	virtual boolean begin (FlashString _name, FlashString _stereotype, FlashString _description, FlashString _version) {
		if (_name != NULL && _description != NULL && strlen_P (F_TO_PSTR (_name)) == 2) {
			name = _name;
			stereotype = _stereotype;
			description = _description;
			version = _version;

			DPRINT (F("New transducer: "));
			DPRINT (name);
			DPRINT (F(" using stereotype "));
			DPRINTLN (stereotype);
			return true;
		} else {
			return false;
		}
	}

	/* Override to implement the actual sensor reading and reporting.
	 *
	 * An actuator might also have a state or some parameters to read, so you can
	 * override this method for that.
	 */
	virtual boolean read (Stereotype *st _UNUSED) = 0;
};

#endif
