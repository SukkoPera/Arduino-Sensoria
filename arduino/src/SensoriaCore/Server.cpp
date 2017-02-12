#include <Sensoria.h>
#include <SensoriaStereotypes/AllStereotypes.h>
//~ #include <RokkitHash.h>
#include "Server.h"
#include "common.h"

SensoriaServer::SensoriaServer (): comm (NULL) {
}

boolean SensoriaServer::begin (FlashString _serverName, SensoriaCommunicator& _comm) {
	serverName = _serverName;
	serverVersion = F("20170130");
	comm = &_comm;
	nTransducers = 0;
	clearBuffer ();

#ifdef ENABLE_NOTIFICATIONS
	nNotificationReqs = 0;
#endif

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

boolean SensoriaServer::send_srv (const char *str, boolean cr, IPAddress* destAddr, word* destPort) {
	if (str) {
		// Append to buffer (keep space for trailing \n and \0)
		if (strlen (buf) < OUT_BUF_SIZE - 2)
			strcat (buf, str);
	}

	if (cr) {
		// Send!
		strcat_P (buf, PSTR ("\r"));   // Line terminator
		boolean ok = comm -> send (buf,
		                           destAddr ? *destAddr : remoteAddress,
		                           destPort ? *destPort : remotePort);

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
#ifdef ENABLE_NOTIFICATIONS
	} else if (strcmp_P (cmd, PSTR ("NRQ")) == 0) {
		cmd_nrq (args);
	} else if (strcmp_P (cmd, PSTR ("NDL")) == 0) {
		cmd_ndl (args);
	} else if (strcmp_P (cmd, PSTR ("NCL")) == 0) {
		cmd_ncl (args);
#endif
	} else {
		DPRINT (F("Unsupported command: \""));
		DPRINT (cmd);
		DPRINTLN (("\""));

		send_srv (F("ERR Unsupported command: \""));
		send_srv (cmd);
		send_srv (F("\""), true);
	}
}

#ifdef ENABLE_NOTIFICATIONS

/**
 * NOTE: This can modify tName
 */
int SensoriaServer::findNotification (IPAddress& addr, word port, NotificationType type, char* tName) {
  int ret = -1;

  strupr (tName);
  for (byte i = 0; i < nNotificationReqs; i++) {
    NotificationRequest& req = notificationReqs[i];

    if (req.destAddr == addr &&
        req.destPort == port &&
        req.type == type &&
        strcmp_P (tName, F_TO_PSTR (req.transducer -> name)) == 0) {

      // Notification found!
      ret = i;
      break;
    }
  }

  return ret;
}

/**
 * NOTE: This can modify nTypeStr
 */
NotificationType SensoriaServer::parseNotificationTypeStr (char *nTypeStr) {
  NotificationType type = NT_UNK;

  strupr (nTypeStr);
  if (strcmp_P (nTypeStr, PSTR ("CHA")) == 0) {
    type = NT_CHA;
  } else if (strcmp_P (nTypeStr, PSTR ("PRD")) == 0) {
    type = NT_PRD;
  }

  return type;
}
#endif

void SensoriaServer::handleNotificationReqs () {
#ifdef ENABLE_NOTIFICATIONS
	// This assumes NRQs 0...nNotificationReqs-1 are always valid
	for (byte i = 0; i < nNotificationReqs; i++) {
		NotificationRequest& req = notificationReqs[i];
		if (millis () - req.timeLastSent >= req.period) {
			Stereotype* st = getStereotype (req.transducer -> stereotype);
			st -> clear ();
			if (req.transducer -> readGeneric (st)) {
				if (req.type == NT_PRD || !(*st == req.transducer -> getLastReading ())) {
					clearSensorBuffer ();
					char *buf = st -> marshal (sensorBuf, SENSOR_BUF_SIZE);
					if (buf) {
						DPRINT (F("Sending notification for "));
						DPRINTLN (req.transducer -> name);

						send_srv (F("NOT "));
						send_srv (req.transducer -> name);
						send_srv (" ");   // No F() here saves flash and wastes no RAM
						send_srv (buf, true, &req.destAddr, &req.destPort);

						req.transducer -> setLastReading (*st);
						req.timeLastSent = millis ();
					} else {
						DPRINTLN (F("ERR Notification marshaling failed"));
					}
				}
			} else {
				DPRINTLN (F("ERR Transducer read for notification failed"));
			}
		}
	}
#endif
}

void SensoriaServer::loop () {
	char *cmd;
	IPAddress addr;
	uint16_t port;

	if (comm -> receiveString (&cmd, &addr, &port, CC_SERVER)) {
		process_cmd (cmd, addr, port);
	}

	handleNotificationReqs ();
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
			send_srv (t -> type == Transducer::SENSOR ? ("S") : ("A"));
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
			send_srv (" ");		   // No F() here saves flash and wastes no RAM
			send_srv (t -> type == Transducer::SENSOR ? ("S") : ("A"));
			send_srv (" ");
			send_srv (t -> stereotype);
			send_srv (" ");

			send_srv (t -> description);

			if (i < nTransducers - 1)
				send_srv (F("|"));
		}

		// Send reply
		send_srv ();
	}
}

void SensoriaServer::cmd_ver (const char *args _UNUSED) {
	send_srv (F("VER "));
	send_srv (serverName);

	if (serverVersion) {
		send_srv (" ");
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
			if (t -> readGeneric (st)) {
				// Try to marshal
				clearSensorBuffer ();
				char *buf = st -> marshal (sensorBuf, SENSOR_BUF_SIZE);
				if (buf) {
					send_srv (F("REA "));
					send_srv (t -> name);
					send_srv (" ");   // No F() here saves flash and wastes no RAM
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
				if (rest) {
					// Do the unmrshaling, baby!
					Stereotype *st = getStereotype (t -> stereotype);
					st -> clear ();
					if (st -> unmarshal (rest)) {
						send_srv (F("WRI "));
						send_srv (t -> name);
						send_srv (" ");   // No F() here saves flash and wastes no RAM
						send_srv (t -> writeGeneric (st) ? F("OK") : F("ERR"), true);
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

#ifdef ENABLE_NOTIFICATIONS
/* --> NRQ OT PRD 10/NRQ OT CHA
 * <-- NRQ OT PRD OK/NRQ OT CHA OK
 */
void SensoriaServer::cmd_nrq (char *args) {
	// Get first arg
	if (args != NULL) {
		char *p[3];
		int n = splitString (args, p, 3);
		if (n >= 2) {
			char* tName = p[0];
			char* nTypeStr = p[1];

      NotificationType type = parseNotificationTypeStr (nTypeStr);
      if (type != NT_UNK) {
        int i = findNotification (remoteAddress, DEFAULT_NOTIFICATION_PORT, type, tName);
        if (i >= 0) {
          // Request already exists, assume client lost track of it and return success
          DPRINTLN (F("NRQ already exists"));
          send_srv (F("NRQ OK"), true);
        } else {
          Transducer *t = getTransducer (tName);
          if (t) {
            if (nNotificationReqs < MAX_NOTIFICATION_REQS) {
              NotificationRequest& req = notificationReqs[nNotificationReqs++];
              req.destAddr = remoteAddress;
              req.destPort = DEFAULT_NOTIFICATION_PORT;		// FIXME
              req.type = type;
              req.transducer = t;
              req.timeLastSent = 0;
              req.lastReading[0] = '\0';

              switch (type) {
                case NT_CHA:
                  DPRINT (F("Notifying on change of "));
                  DPRINTLN (t -> name);

                  req.period = NOTIFICATION_POLL_INTERVAL;

                  send_srv (F("NRQ OK"), true);
                  break;
                case NT_PRD:
                  if (n < 3) {
                    nNotificationReqs--;
                    DPRINTLN (F("ERR No interval specified"));
                    send_srv (F("MRQ ERR"), true);
                  } else {
                    word intv = atoi (p[2]);

                    DPRINT (F("Notifying values of "));
                    DPRINT (t -> name);
                    DPRINT (F(" every "));
                    DPRINT (intv);
                    DPRINTLN (F(" second(s)"));

                    req.type = NT_PRD;
                    req.period = intv * 1000UL;

                    send_srv (F("NRQ OK"), true);
                  }
                  break;
                default:
                  break;
              }
            } else {
              DPRINTLN (F("ERR Max notification requests reached"));

              send_srv (F("NRQ ERR"), true);
            }
          } else {
            DPRINT (F("ERR No such transducer: "));
            DPRINTLN (args);

            send_srv (F("NRQ ERR"), true);
          }
        }
      } else {
        DPRINT (F("ERR Bad notification request type: "));
        DPRINTLN (nTypeStr);

        send_srv (F("NRQ ERR"), true);
      }
		} else {
			DPRINTLN (F("ERR Bad request"));
			send_srv (F("ERR Bad request"), true);
		}
	} else {
		DPRINTLN (F("ERR Missing transducer name"));
		send_srv (F("ERR Missing transducer name"), true);
	}
}

/* Delete single notification request
 * --> NDL OT PRD [10]/NDL OT CHA
 * <-- NDL OT PRD OK/NDL OT CHA OK
 */
void SensoriaServer::cmd_ndl (char *args) {
	// Get first arg
	if (args != NULL) {
		char *p[3];
		int n = splitString (args, p, 3);
		if (n >= 2) {
			char* tName = p[0];
			char* nTypeStr = p[1];

      NotificationType type = parseNotificationTypeStr (nTypeStr);
      if (type != NT_UNK) {
        int i = findNotification (remoteAddress, DEFAULT_NOTIFICATION_PORT, type, tName);
        if (i >= 0) {
          //  Notification found
          DPRINT (F("Deleting notification request "));
          DPRINTLN (i);

          /* NRQs 0...nNotificationReqs-1 are always valid, so deleting a
           * NRQ means we have to shift all subsequent NRQs down by one
           * place
           */
          for (byte j = 0; i + j + 1 < nNotificationReqs; j++) {
            notificationReqs[i + j] = notificationReqs[i + j + 1];
          }

          nNotificationReqs--;

          send_srv (F("NDL OK"), true);
        } else {
          DPRINT (F("ERR No such NRQ"));
          send_srv (F("NDL ERR"), true);
        }
      } else {
        DPRINT (F("ERR Bad notification request type: "));
        DPRINTLN (nTypeStr);

        send_srv (F("NDL ERR"), true);
      }
		} else {
			DPRINTLN (F("ERR Bad request"));
			send_srv (F("ERR Bad request"), true);
		}
	} else {
		DPRINTLN (F("ERR Missing transducer name"));
		send_srv (F("ERR Missing transducer name"), true);
	}
}

// Clear (= Delete) all notification requests
void SensoriaServer::cmd_ncl (char *args) {
	(void) *args;

	nNotificationReqs = 0;
	DPRINTLN (F("Cleared all notification requests"));

	// This can't really fail
	send_srv (F("NCL OK"), true);
}

#endif    // ENABLE_NOTIFICATIONS
