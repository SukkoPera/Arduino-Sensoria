#ifndef _SENSORIA_H_INCLUDED
#define _SENSORIA_H_INCLUDED

#include <avr/pgmspace.h>
#include "Transducer.h"
#include "Sensor.h"
#include "Actuator.h"
#include "internals/utils.h"
#include "internals/debug.h"


#define OUT_BUF_SIZE 128
#define SENSOR_BUF_SIZE 32

class SensoriaServer {
public:
	static const byte MAX_SERVER_NAME = 32;

private:
	static const byte MAX_SENSORS = 8;
	byte nTransducers;

	Transducer *transducers[MAX_SENSORS];

	char buf[OUT_BUF_SIZE];
	char sensorBuf[SENSOR_BUF_SIZE];

	char serverName[MAX_SERVER_NAME];
	PGM_P serverVersion;

	inline void clearBuffer () {
		buf[0] = '\0';
	}

	void clearSensorBuffer ();

	void cmd_qry (char *args) {
		if (args != NULL) {
			// Get first arg
			char *space = strchr (args, ' ');
			if (space)
				*space = '\0';

			Transducer *t = getTransducer (args);
			if (t) {
				send_srv ("QRY ");
				send_srv (t -> name);
				send_srv ("|");
				send_srv (t -> type == Transducer::SENSOR ? "S" : "A");
				send_srv ("|");
				send_srv (t -> description);
				send_srv ("|");
				send_srv (t -> version, true);
			} else {
				DPRINT (F("ERR No such transducer: "));
				DPRINTLN (args);

				send_srv (F("ERR No such transducer: "));
				send_srv (args, true);
			}
		} else {
			// List sensors
			send_srv (F("QRY "));

			for (byte i = 0; i < nTransducers; i++) {
				Transducer *t = transducers[i];
				send_srv (t -> name);
				send_srv (" ");
				send_srv (t -> type == Transducer::SENSOR ? "S" : "A");
				send_srv (" ");
				send_srv (t -> description);

				if (i < nTransducers - 1)
					send_srv (F("|"));
			}

			// Send reply
			send_srv ((char *) NULL, true);
		}
	}

	void cmd_ver (const char *args _UNUSED) {
		//~ send_srv (F("VER SukkoMeteoStation 0.1 20151101"), true);
		send_srv (F("VER "));
		send_srv (serverName);

		if (serverVersion) {
			send_srv (F(" "));
			send_srv (reinterpret_cast<const __FlashStringHelper *> (serverVersion));
		}

		send_srv ((char *) NULL, true);
	}

	void cmd_rea (char *args) {
		// Get first arg
		if (args != NULL) {
			char *space = strchr (args, ' ');
			if (space)
				*space = '\0';

			Transducer *t = getTransducer (args);
			if (t) {
				if (t -> type == Transducer::SENSOR) {
					Sensor *s = (Sensor *) t;
					clearSensorBuffer ();
					send_srv (F("REA "));
					send_srv (s -> name);
					send_srv (" ");
					send_srv (s -> read (sensorBuf, SENSOR_BUF_SIZE), true);
				} else {
					send_srv (F("ERR Transducer is not a sensor"), true);
				}
			} else {
				DPRINT (F("ERR No such transducer: "));
				DPRINTLN (args);

				send_srv (F("ERR No such transducer: "));
				send_srv (args, true);
			}
		} else {
			DPRINTLN (F("ERR Missing transducer name"));
			send_srv (F("ERR Missing transducer name"), true);
		}
	}

	void cmd_wri (char *args) {
		// Get first arg
		if (args != NULL) {
			char *space = strchr (args, ' ');
			char *rest = NULL;
			if (space) {
				*space = '\0';
				rest = space + 1;
			}

			Transducer *t = getTransducer (args);
			if (t) {
				if (t -> type == Transducer::ACTUATOR) {
					Actuator *a = (Actuator *) t;
					send_srv (F("WRI "));
					send_srv (a -> name);
					send_srv (" ");
					send_srv (a -> write (rest) ? "OK" : "ERR", true);
				} else {
					send_srv (F("ERR Transducer is not an actuator"), true);
				}
			} else {
				DPRINT (F("ERR No such transducer: "));
				DPRINTLN (args);

				send_srv (F("ERR No such transducer: "));
				send_srv (args, true);
			}
		} else {
			DPRINTLN (F("ERR Missing transducer name"));
			send_srv (F("ERR Missing transducer name"), true);
		}
	}


	boolean send_srv (const char *str, boolean cr = false);
	boolean send_srv (const __FlashStringHelper *str, boolean cr = false);

protected:
	void process_cmd (char *buffer) {
		char *cmd, *args;

		// Separate command and arguments
		//char *buffer = reinterpret_cast<char *> (buffer8);
		strstrip (buffer);
		char *space = strchr (buffer, ' ');
		if (space) {
			*space = '\0';
			args = space + 1;
		} else {
			// Command with no args
			args = NULL;
		}

		cmd = buffer;
		strupr (cmd);	// Done in place

		DPRINT (F("Processing command: \""));
		DPRINT (cmd);
		DPRINTLN (F("\""));

		if (strcmp_P (cmd, PSTR ("QRY")) == 0) {
			cmd_qry (args);
		} else if (strcmp_P (cmd, PSTR ("REA")) == 0) {
			cmd_rea (args);
		} else if (strcmp_P (cmd, PSTR ("VER")) == 0) {
			cmd_ver (args);
		} else if (strcmp_P (cmd, PSTR ("WRI")) == 0) {
			cmd_wri (args);
		} else {
			DPRINT (F("Unsupported command: \""));
			DPRINT (cmd);
			DPRINTLN (("\""));

			send_srv (F("ERR Unsupported command: \""));
			send_srv (cmd);
			send_srv (F("\""), true);
		}
	}

	// Override to implement actual sending of data
	virtual boolean send (const char *str) = 0;

public:
	SensoriaServer (const char *_serverName, PGM_P _serverVersion = NULL);

	// Override if needed
	virtual boolean begin ();

	// Override if needed
	virtual boolean stop ();

	int addTransducer (Transducer& transducer);

	Transducer *getTransducer (char *name) const;
};

#endif
