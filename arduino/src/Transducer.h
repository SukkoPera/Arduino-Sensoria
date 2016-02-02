#ifndef _TRANSDUCER_H_INCLUDED
#define _TRANSDUCER_H_INCLUDED

#include <Arduino.h>
#include "internals/utils.h"
#include "internals/common.h"
#include "internals/debug.h"

class Transducer {
public:
	enum Type {
		SENSOR,
		ACTUATOR,
		N_TYPES
	};

	Type type;
	FlashString name;
	FlashString description;
	FlashString version;

	Transducer (Type _type): type (_type) {
		name = NULL;
		description = NULL;
		version = NULL;
	}

	virtual bool begin (FlashString _name, FlashString _description, FlashString _version) {
		if (_name != NULL && _description != NULL && strlen_P (F_TO_PSTR (_name)) == 2) {
			name = _name;
			description = _description;
			version = _version;

			DPRINT ("New transducer: ");
			DPRINTLN (name);
			return true;
		} else {
			return false;
		}
	}
};

#endif
