#include <Sensoria.h>
#include <SensoriaStereotypes/AllStereotypes.h>
#include "Server.h"
#include "common.h"

SensoriaServer::SensoriaServer (): comm (NULL), outBuf (outBufRaw, OUT_BUF_SIZE) {
}

boolean SensoriaServer::begin (FlashString _serverName, SensoriaCommunicator& _comm) {
	serverName = _serverName;
	comm = &_comm;
	nTransducers = 0;

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

// Called automatically by SensoriaServer before read()
void SensoriaServer::clearSensorBuffer () {
	for (int i = 0; i < SENSOR_BUF_SIZE; i++)
		sensorBuf[i] = '\0';
}

void SensoriaServer::process_cmd (char *buffer, const SensoriaAddress* senderAddr) {
	char *cmd, *args;

	{
		char addrbuf[32];
		DPRINT (F("Processing command: \""));
		DPRINT (buffer);
		DPRINT (F("\" from "));
		DPRINTLN (senderAddr -> toString (addrbuf, sizeof (addrbuf)));
	}

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
	if (strcmp_P (cmd, PSTR ("HLO")) == 0) {
		cmd_hlo (senderAddr, args);
	} else if (strcmp_P (cmd, PSTR ("REA")) == 0) {
		cmd_rea (senderAddr, args);
	} else if (strcmp_P (cmd, PSTR ("WRI")) == 0) {
		cmd_wri (senderAddr, args);
#ifdef ENABLE_NOTIFICATIONS
	} else if (strcmp_P (cmd, PSTR ("NRQ")) == 0) {
		cmd_nrq (senderAddr, args);
	} else if (strcmp_P (cmd, PSTR ("NDL")) == 0) {
		cmd_ndl (senderAddr, args);
	} else if (strcmp_P (cmd, PSTR ("NCL")) == 0) {
		cmd_ncl (senderAddr, args);
#endif
	} else {
		DPRINT (F("Unsupported command: \""));
		DPRINT (cmd);
		DPRINTLN (("\""));

		outBuf.begin ();
		outBuf.print (F("ERR Unsupported command: \""));
		outBuf.print (cmd);
		outBuf.print (F("\"\n"));
		comm -> reply ((const char *) outBuf, senderAddr);
	}
}

#ifdef ENABLE_NOTIFICATIONS

/**
 * NOTE: This can modify tName
 */
int SensoriaServer::findNotification (const SensoriaAddress* clientAddr, NotificationType type, char* tName) {
	int ret = -1;

	strupr (tName);
	for (byte i = 0; i < nNotificationReqs; i++) {
		NotificationRequest& req = notificationReqs[i];

		if (*req.destAddr == *clientAddr &&
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

void SensoriaServer::handleNotificationReqs () {
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

						outBuf.begin ();
						outBuf.print (F("NOT "));
						outBuf.print (req.transducer -> name);
						outBuf.print (" ");   // No F() here saves flash and wastes no RAM
						outBuf.print (buf);
						outBuf.print ('\n');
						comm -> notify ((const char *) outBuf, req.destAddr);

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
}
#endif

void SensoriaServer::loop () {
	SensoriaAddress* addr = comm -> getAddress ();
	if (!addr) {
		DPRINTLN (F("No address available"));
	} else {
		char *cmd;

		if (comm -> receiveCmd (cmd, addr)) {
#if 0
			char buf[24];
			DPRINT (F("Received command from "));
			DPRINT (addr -> toString (buf, sizeof (buf)));
			DPRINT (F(": \""));
			DPRINT (cmd);
			DPRINTLN (F("\""));
#endif
			process_cmd (cmd, addr);
		}

		comm -> releaseAddress (addr);
	}

#ifdef ENABLE_NOTIFICATIONS
	handleNotificationReqs ();
#endif
}

/*******************************************************************************
 * COMMANDS
 *******************************************************************************
 */

void SensoriaServer::cmd_hlo (const SensoriaAddress* clientAddr, char *args) {
	(void) args;

	outBuf.begin ();
	outBuf.print (F("HLO "));
	outBuf.print (serverName);
	outBuf.print (' ');
	outBuf.print (SensoriaServer::PROTOCOL_VERSION);
	outBuf.print (' ');

	for (byte i = 0; i < nTransducers; i++) {
		Transducer& t = *transducers[i];
		outBuf.print (t.name);
		outBuf.print (' ');
		outBuf.print (t.type == Transducer::SENSOR ? ("S") : ("A"));
		outBuf.print (' ');
		outBuf.print (t.stereotype);
		outBuf.print (' ');
		outBuf.print (t.description);

		if (i < nTransducers - 1)
			outBuf.print ('|');
	}

	// Send reply
	outBuf.print ('\n');
	comm -> reply ((const char *) outBuf, clientAddr);
}

void SensoriaServer::cmd_rea (const SensoriaAddress* clientAddr, char *args) {
	outBuf.begin ();
	outBuf.print (F("REA "));

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
					outBuf.print (F("OK "));
					outBuf.print (buf);
				} else {
					outBuf.print (F("ERR Marshaling failed"));
				}
			} else {
				outBuf.print (F("ERR"));
			}
		} else {
			DPRINT (F("REA ERR No such transducer: "));
			DPRINTLN (args);

			outBuf.print (F("ERR No such transducer: "));
			outBuf.print (args);
		}
	} else {
		DPRINTLN (F("REA ERR Missing transducer name"));
		outBuf.print (F("ERR Missing transducer name"));
	}

	// Send reply
	outBuf.print ('\n');
	comm -> reply ((const char *) outBuf, clientAddr);
}

void SensoriaServer::cmd_wri (const SensoriaAddress* clientAddr, char *args) {
	outBuf.begin ();
	outBuf.print (F("WRI "));

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
						outBuf.print (t -> writeGeneric (st) ? F("OK") : F("ERR"));
					} else {
						DPRINT (F("Unmarshaling with "));
						DPRINT (st -> tag);
						DPRINT (F(" failed for: "));
						DPRINTLN (rest);
						outBuf.print (F("ERR Unmarshaling failed"));
					}
				} else {
					outBuf.print (F("ERR Nothing to write"));
				}
			} else {
				outBuf.print (F("ERR Transducer is not an actuator"));
			}
		} else {
			DPRINT (F("ERR No such transducer: "));
			DPRINTLN (args);

			outBuf.print (F("ERR No such transducer: "));
			outBuf.print (args);
		}
	} else {
		DPRINTLN (F("ERR Missing transducer name"));
		outBuf.print (F("ERR Missing transducer name"));
	}

	// Send reply
	outBuf.print ('\n');
	comm -> reply ((const char *) outBuf, clientAddr);
}

#ifdef ENABLE_NOTIFICATIONS
/* --> NRQ OT PRD 10/NRQ OT CHA
 * <-- NRQ OT OK/NRQ OT OK
 */
void SensoriaServer::cmd_nrq (const SensoriaAddress* clientAddr, char *args) {
	outBuf.begin ();

	// Get first arg
	if (args != NULL) {
		char *p[3];
		int n = splitString (args, p, 3);
		if (n >= 2) {
			char* tName = p[0];
			char* nTypeStr = p[1];

			NotificationType type = parseNotificationTypeStr (nTypeStr);
			if (type != NT_UNK) {
				bool added = false;
				SensoriaAddress* notAddr = comm -> getNotificationAddress (clientAddr);
				if (notAddr) {
					int i = findNotification (notAddr, type, tName);
					if (i >= 0) {
						// Request already exists, assume client lost track of it and return success
						DPRINTLN (F("NRQ already exists"));
						outBuf.print (F("NRQ OK"));
					} else {
						Transducer *t = getTransducer (tName);
						if (t) {
							if (nNotificationReqs < MAX_NOTIFICATION_REQS) {
								NotificationRequest& req = notificationReqs[nNotificationReqs++];
								req.destAddr = notAddr;
								req.type = type;
								req.transducer = t;
								req.timeLastSent = 0;
								added = true;

								switch (type) {
									case NT_CHA:
										{
											char addrBuf[32];

											DPRINT (F("Notifying "));
											DPRINT (req.destAddr -> toString (addrBuf, sizeof (addrBuf)));
											DPRINT (F(" on change of "));
											DPRINTLN (t -> name);
										}

										req.period = NOTIFICATION_POLL_INTERVAL;

										outBuf.print (F("NRQ OK"));
										break;
									case NT_PRD:
										if (n < 3) {
											nNotificationReqs--;
											DPRINTLN (F("ERR No interval specified"));
											outBuf.print (F("NRQ ERR"));
										} else {
											word intv = atoi (p[2]);

											{
												char addrBuf[32];

												DPRINT (F("Notifying "));
												DPRINT (req.destAddr -> toString (addrBuf, sizeof (addrBuf)));
												DPRINT (F("of values of "));
												DPRINT (t -> name);
												DPRINT (F(" every "));
												DPRINT (intv);
												DPRINTLN (F(" second(s)"));
											}

											req.type = NT_PRD;
											req.period = intv * 1000UL;

											outBuf.print (F("NRQ OK"));
										}
										break;
									default:
										break;
								}
							} else {
								DPRINTLN (F("ERR Max notification requests reached"));

								outBuf.print (F("NRQ ERR"));
							}
						} else {
							DPRINT (F("ERR No such transducer: "));
							DPRINTLN (args);

							outBuf.print (F("NRQ ERR"));
						}
					}

					if (!added)
						comm -> releaseAddress (notAddr);
				} else {
					DPRINTLN (F("ERR No address available"));

					outBuf.print (F("NRQ ERR"));
				}
			} else {
				DPRINT (F("ERR Bad notification request type: "));
				DPRINTLN (nTypeStr);

				outBuf.print (F("NRQ ERR"));
			}
		} else {
			DPRINTLN (F("ERR Bad request"));
			outBuf.print (F("ERR Bad request"));
		}
	} else {
		DPRINTLN (F("ERR Missing transducer name"));
		outBuf.print (F("ERR Missing transducer name"));
	}

	// Send reply
	outBuf.print ('\n');
	comm -> reply ((const char *) outBuf, clientAddr);
}

/* Delete single notification request
 * --> NDL OT PRD [10]/NDL OT CHA
 * <-- NDL OT PRD OK/NDL OT CHA OK
 */
void SensoriaServer::cmd_ndl (const SensoriaAddress* clientAddr, char *args) {
	outBuf.begin ();

	// Get first arg
	if (args != NULL) {
		char *p[3];
		int n = splitString (args, p, 3);
		if (n >= 2) {
			char* tName = p[0];
			char* nTypeStr = p[1];

			NotificationType type = parseNotificationTypeStr (nTypeStr);
			if (type != NT_UNK) {
				SensoriaAddress* notAddr = comm -> getNotificationAddress (clientAddr);
				if (notAddr) {
					int i = findNotification (notAddr, type, tName);
					if (i >= 0) {
						//  Notification found
						DPRINT (F("Deleting notification request "));
						DPRINTLN (i);

						NotificationRequest& req = notificationReqs[i];
						comm -> releaseAddress (req.destAddr);

						/* NRQs 0...nNotificationReqs-1 are always valid, so deleting a
						 * NRQ means we have to shift all subsequent NRQs down by one
						 * place
						 */
						for (byte j = 0; i + j + 1 < nNotificationReqs; j++) {
							notificationReqs[i + j] = notificationReqs[i + j + 1];
						}

						nNotificationReqs--;

						outBuf.print (F("NDL OK"));
					} else {
						DPRINTLN (F("ERR No such NRQ"));
						outBuf.print (F("NDL ERR"));
					}

					comm -> releaseAddress (notAddr);
				} else {
					DPRINTLN (F("ERR No address available"));
					outBuf.print (F("NDL ERR"));
				}
			} else {
				DPRINT (F("ERR Bad notification request type: "));
				DPRINTLN (nTypeStr);

				outBuf.print (F("NDL ERR"));
			}
		} else {
			DPRINTLN (F("ERR Bad request"));
			outBuf.print (F("ERR Bad request"));
		}
	} else {
		DPRINTLN (F("ERR Missing transducer name"));
		outBuf.print (F("ERR Missing transducer name"));
	}

	// Send reply
	outBuf.print ('\n');
	comm -> reply ((const char *) outBuf, clientAddr);
}

// Clear (= Delete) all notification requests
void SensoriaServer::cmd_ncl (const SensoriaAddress* clientAddr, char *args) {
	(void) *args;

	for (byte i = 0; i < nNotificationReqs; i++) {
		NotificationRequest& req = notificationReqs[i];
		comm -> releaseAddress (req.destAddr);
	}

	nNotificationReqs = 0;
	DPRINTLN (F("Cleared all notification requests"));

	// This can't really fail
	outBuf.begin ();
	outBuf.print (F("NCL OK\n"));
	comm -> reply ((const char *) outBuf, clientAddr);
}

#endif    // ENABLE_NOTIFICATIONS
