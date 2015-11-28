#ifndef _TRANSDUCER_H_INCLUDED
#define _TRANSDUCER_H_INCLUDED

#include <Arduino.h>
#include "internals/utils.h"
#include "internals/debug.h"

class Transducer {
public:
	enum Type {
		SENSOR,
		ACTUATOR,
		N_TYPES
	};

	Type type;
	const __FlashStringHelper *name;
	const __FlashStringHelper *description;
	const __FlashStringHelper *version;

	Transducer (Type _type): type (_type) {
		name = NULL;
		description = NULL;
		version = NULL;
	}

	virtual bool begin (const __FlashStringHelper *_name, const __FlashStringHelper *_description, const __FlashStringHelper *_version) {
		if (_name != NULL && _description != NULL && strlen_P (reinterpret_cast<PGM_P> (_name)) == 2) {
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

	//~ const char *get_type_string () const {
		//~ if (type < N_TYPES)
			//~ return type_strings[type];
		//~ else
			//~ return NULL;
	//~ }
};

#endif
