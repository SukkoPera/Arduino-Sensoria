#include <Sensoria.h>
#include <SensoriaStereotypes/AllStereotypes.h>
//~ #include <RokkitHash.h>
#include "Server.h"
#include "common.h"


SensoriaServer::SensoriaServer (): comm (NULL), nTransducers (0), hash (42) {
	clearBuffer ();
}

boolean SensoriaServer::begin (FlashString _serverName, SensoriaCommunicator& _comm) {
  serverName = _serverName;
  serverVersion = F("20160320");
  comm = &_comm;

  return strlen_P (F_TO_PSTR (_serverName)) > 0;
}

//~ boolean SensoriaServer::stop () {
	//~ return true;
//~ }

Stereotype* SensoriaServer::getStereotype (FlashString s) {
  Stereotype *st = NULL;
  for (int i = 0; !st && i < N_STEREOTYPES; i++) {
    //~ DPRINT ("-> ");
    //~ DPRINT (stereotypes[i] -> tag);
    //~ DPRINT ("-");
    //~ DPRINTLN (s);

    if (strcmp_P (stereotypes[i] -> tag, F_TO_PSTR (s)) == 0)
      st = stereotypes[i];
  }

  return st;
}

int SensoriaServer::addTransducer (Transducer& transducer) {
	if (nTransducers < MAX_TRANSDUCERS) {
    // Look up stereotype
    if (getStereotype (transducer.stereotype)) {
      transducers[nTransducers++] = &transducer;

      // Update hash
      //~ hash = rokkit (transducer.name, strlen_P (reinterpret_cast<PGM_P> (transducer.name)), hash);
      //~ hash = rokkit (transducer.type == Transducer::SENSOR ? F("S") : F("A"), 1, hash);
      //~ hash = rokkit (transducer.description, strlen_P (reinterpret_cast<PGM_P> (transducer.description)), hash);
      //~ hash = rokkit (transducer.version, strlen_P (reinterpret_cast<PGM_P> (transducer.version)), hash);

      return nTransducers - 1;
    } else {
      DPRINT (F("Unknown stereotype: "));
      DPRINTLN (transducer.stereotype);

      return -10;
    }
	} else {
		return -1;
	}
}

Transducer *SensoriaServer::getTransducer (char *name) const {
	strupr (name);
	for (int i = 0; i < nTransducers; ++i) {
		if (strcmp_P (name, F_TO_PSTR (transducers[i] -> name)) == 0)
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
		boolean ok = comm -> send (buf, remoteAddress, remotePort);

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
		boolean ok = comm -> send (buf, remoteAddress, remotePort);

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

void SensoriaServer::process_cmd (char *buffer, IPAddress senderAddr, uint16_t senderPort) {
  char *cmd, *args;

  remoteAddress = senderAddr;
  remotePort = senderPort;

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


void SensoriaServer::loop () {
  char *cmd;
  IPAddress addr;
  uint16_t port;

  if (comm -> receiveString (&cmd, &addr, &port)) {
    process_cmd (cmd, addr, port);
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
      send_srv (F("QRY "));
      send_srv (t -> name);
      send_srv (F("|"));
      send_srv (t -> type == Transducer::SENSOR ? F("S") : F("A"));
      send_srv (F("|"));
      send_srv (t -> stereotype);
      send_srv (F("|"));
      send_srv (t -> description);
      send_srv (F("|"));
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
      send_srv (F(" "));
      send_srv (t -> type == Transducer::SENSOR ? ("S") : ("A"));
      send_srv (F(" "));
      send_srv (t -> stereotype);
      send_srv (F(" "));

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
      Stereotype *st = getStereotype (t -> stereotype);   // Can't be NULL by now!
      st -> clear ();
      if (t -> read (st)) {
        // Try to marshal
        clearSensorBuffer ();
        char *buf = st -> marshal (sensorBuf, SENSOR_BUF_SIZE);
        if (buf) {
          send_srv (F("REA "));
          send_srv (t -> name);
          send_srv ((" "));
          send_srv (buf, true);
        } else {
          send_srv (F("ERR Marshaling failed"), true);
        }
      } else {
        send_srv (F("ERR Read failed"), true);
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

        if (rest) {
          // Do the unmrshaling, baby!
          Stereotype *st = getStereotype (a -> stereotype);
          st -> clear ();
          if (st -> unmarshal (rest)) {
            send_srv (F("WRI "));
            send_srv (a -> name);
            send_srv ((" "));
            send_srv (a -> write (st) ? F("OK") : F("ERR"), true);
          } else {
            DPRINT (F("Unmarshaling with "));
            DPRINT (st -> tag);
            DPRINT (F(" failed for: "));
            DPRINTLN (buf);
            send_srv (F("ERR Unmarshaling failed"), true);
          }
        } else {
          send_srv (F("ERR Nothing to write"), true);
        }
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
