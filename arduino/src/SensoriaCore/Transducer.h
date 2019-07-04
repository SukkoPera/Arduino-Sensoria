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

	Transducer (Type _type): type (_type) {
		name = NULL;
		description = NULL;
		stereotype = NULL;
	}

	virtual boolean begin (FlashString _name, FlashString _stereotype, FlashString _description) {
		if (_name != NULL && _description != NULL && strlen_P (F_TO_PSTR (_name)) == 2) {
			name = _name;
			stereotype = _stereotype;
			description = _description;

			DPRINT (F("New transducer: "));
			DPRINT (name);
			DPRINT (F(" using stereotype "));
			DPRINTLN (stereotype);
			return true;
		} else {
			return false;
		}
	}

#ifdef ENABLE_NOTIFICATIONS
	virtual Stereotype& getLastReading () = 0;

	virtual void setLastReading (Stereotype& reading) = 0;
#endif

	/* Override to implement the actual sensor reading and reporting.
	 *
	 * An actuator might also have a state or some parameters to read, so you can
	 * override this method for that.
	 */
	virtual boolean readGeneric (Stereotype* st) = 0;

	/* Override to implement the actual Actuator writing.
	 */
	virtual boolean writeGeneric (Stereotype* st) = 0;
};

template <typename ST>
class TransducerT: public Transducer {
private:
#ifdef ENABLE_NOTIFICATIONS
	ST lastReading;

	Stereotype& getLastReading () {
		return lastReading;
	}

	void setLastReading (Stereotype& st) {
		ST& tst = static_cast<ST&> (st);
		lastReading = tst;
	}
#endif

	virtual boolean readGeneric (Stereotype* st) override {
		ST* tst = static_cast<ST*> (st);
		return read (*tst);
	}

protected:
	TransducerT (Type _type): Transducer (_type) {
	}

	/* Override to implement the actual sensor reading and reporting.
	 *
	 * An actuator might also have a state or some parameters to read, so you can
	 * override this method for that.
	 */
	virtual boolean read (ST& st) = 0;
};

#endif
