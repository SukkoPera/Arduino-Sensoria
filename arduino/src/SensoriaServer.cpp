//~ #include <RokkitHash.h>
#include "SensoriaServer.h"
#include <internals/common.h>


SensoriaServer::SensoriaServer (): nTransducers (0), hash (42) {
	clearBuffer ();
}

boolean SensoriaServer::begin (FlashString _serverName, FlashString _serverVersion) {
//~ #ifdef ENABLE_FLASH_STRINGS
  serverName = _serverName;
  serverVersion = _serverVersion;

  return strlen_P (F_TO_PSTR (_serverName)) > 0;
//~ #else
	//~ strlcpy (serverName, _serverName, MAX_SERVER_NAME);

  //~ if (_serverVersion != NULL)
    //~ strlcpy (serverVersion, _serverVersion, MAX_SERVER_NAME);
  //~ else
    //~ serverVersion = NULL;

  //~ return strlen (_serverName) > 0;
//~ #endif
}

boolean SensoriaServer::stop () {
	return true;
}

int SensoriaServer::addTransducer (Transducer& transducer) {
	if (nTransducers < MAX_SENSORS) {
		transducers[nTransducers++] = &transducer;

		// Update hash
		//~ hash = rokkit (transducer.name, strlen_P (reinterpret_cast<PGM_P> (transducer.name)), hash);
		//~ hash = rokkit (transducer.type == Transducer::SENSOR ? F("S") : F("A"), 1, hash);
		//~ hash = rokkit (transducer.description, strlen_P (reinterpret_cast<PGM_P> (transducer.description)), hash);
		//~ hash = rokkit (transducer.version, strlen_P (reinterpret_cast<PGM_P> (transducer.version)), hash);

		return nTransducers - 1;
	} else {
		return -1;
	}
}

Transducer *SensoriaServer::getTransducer (char *name) const {
	strupr (name);
	for (int i = 0; i < nTransducers; ++i) {
		if (strcmp_P (name, reinterpret_cast<PGM_P> (transducers[i] -> name)) == 0)
			return transducers[i];
	}

	return NULL;
}

void SensoriaServer::clearBuffer () {
  buf[0] = '\0';
}

// Called automatically by SensoriaServer before read()
void SensoriaServer::clearSensorBuffer () {
	for (int i = 0; i < SENSOR_BUF_SIZE; i++)
		sensorBuf[i] = '\0';
}

boolean SensoriaServer::send_srv (const char *str, boolean cr) {
	if (str) {
		// Append to buffer (keep space for trailing \n and \0)
		if (strlen (buf) < OUT_BUF_SIZE - 2)
      strcat (buf, str);
	}

	if (cr) {
		// Send!
		strcat_P (buf, PSTR ("\r"));   // Line terminator
		boolean ok = send (buf);

		clearBuffer ();

		return ok;
	} else {
		return true;
	}
}

#ifdef ENABLE_FLASH_STRINGS
boolean SensoriaServer::send_srv (const __FlashStringHelper *str, boolean cr) {
	if (str) {
		// Append to buffer (keep space for trailing \n and \0)
    if (strlen (buf) < OUT_BUF_SIZE - 2)
      strcat_P (buf, F_TO_PSTR (str));
	}

	if (cr) {
		// Send!
		strcat_P (buf, PSTR ("\r"));   // Line terminator
		boolean ok = send (buf);

		clearBuffer ();

		return ok;
	} else {
		return true;
	}
}
#endif

boolean SensoriaServer::send_srv () {
  return send_srv ((char *) NULL, true);
}

void SensoriaServer::process_cmd (char *buffer) {
  char *cmd, *args;

  // Separate command and arguments
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


/*******************************************************************************
 * COMMANDS
 *******************************************************************************
 */

void SensoriaServer::cmd_qry (char *args) {
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
      send_srv ("|WD|");		// FIXME!
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
      send_srv (" WD ");		// FIXME!
      send_srv (t -> description);

      if (i < nTransducers - 1)
        send_srv (F("|"));
    }

    // Send reply
    send_srv ((char *) NULL, true);
  }
}

void SensoriaServer::cmd_ver (const char *args _UNUSED) {
  send_srv (F("VER "));
  send_srv (serverName);

  if (serverVersion) {
    send_srv (F(" "));
    send_srv (serverVersion);
  }

  send_srv ();
}

void SensoriaServer::cmd_rea (char *args) {
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
        char *buf = s -> read (sensorBuf, SENSOR_BUF_SIZE);
        if (buf) {
          send_srv (F("REA "));
          send_srv (s -> name);
          send_srv (" ");
          send_srv (buf, true);
        } else {
          send_srv (F("ERR Read failed"), true);
        }
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

void SensoriaServer::cmd_wri (char *args) {
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
