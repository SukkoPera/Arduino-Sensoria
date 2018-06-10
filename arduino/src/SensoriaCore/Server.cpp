#include <Sensoria.h>
#include <SensoriaStereotypes/AllStereotypes.h>
#include "Server.h"
#include "common.h"


#define OK_STR "OK"
static const char _OK[] PROGMEM = OK_STR;
#define OK_FSTR PSTR_TO_F(_OK)
#define ERR_STR "ERR"
static const char _ERR[] PROGMEM = ERR_STR;
#define ERR_FSTR PSTR_TO_F(_ERR)

#define ENABLE_DETAILED_ERRORS

// Note that these do NOT put CMD in front of message
#ifdef ENABLE_DETAILED_ERRORS
#define ERR_MSG(msg) (F(ERR_STR " " msg))
#else
#define ERR_MSG(msg) (F(ERR_STR))
#endif

#define OK_MSG(msg) (F(OK_STR " " msg))


SensoriaServer::SensoriaServer (): comm (NULL), outBuf (outBufRaw, OUT_BUF_SIZE) {
}

boolean SensoriaServer::begin (FlashString _serverName, SensoriaCommunicator& _comm) {
	serverName = _serverName;
	comm = &_comm;
	nTransducers = 0;

#ifdef ENABLE_NOTIFICATIONS
	nNotificationReqs = 0;
#endif

#ifdef ENABLE_CMD_DIE
	running = true;
#endif

	return strlen_P (F_TO_PSTR (_serverName)) > 0;
}

boolean SensoriaServer::end () {
#ifdef ENABLE_CMD_DIE
	running = false;
#endif

	return true;
}

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
			transducers[nTransducers] = &transducer;

			return nTransducers++;
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

		// Trim off leading spaces
		while (*args && isspace (*args))
			++args;
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
#ifdef ENABLE_CMD_DIE
	} else if (strcmp_P (cmd, PSTR ("DIE")) == 0) {
		cmd_die (senderAddr, args);
#endif
#ifdef ENABLE_CMD_RST
	} else if (strcmp_P (cmd, PSTR ("RST")) == 0) {
		cmd_rst (senderAddr, args);
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

void SensoriaServer::deleteNotification (byte i) {
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
						outBuf.print (' ');
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
#ifdef ENABLE_CMD_DIE
	if (running) {
#endif
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

#ifdef ENABLE_CMD_DIE
	}
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
		outBuf.print (t.type == Transducer::SENSOR ? 'S' : 'A');
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

	if (args != NULL) {
		Transducer *t = getTransducer (args);
		if (t) {
			Stereotype *st = getStereotype (t -> stereotype);   // Can't be NULL by now!
			st -> clear ();
			if (t -> readGeneric (st)) {
				// Try to marshal
				clearSensorBuffer ();
				char *buf = st -> marshal (sensorBuf, SENSOR_BUF_SIZE);
				if (buf) {
					outBuf.print (OK_MSG(""));
					outBuf.print (buf);
				} else {
					outBuf.print (ERR_MSG("Marshaling failed"));
				}
			} else {
				outBuf.print (ERR_MSG("Read failed"));
			}
		} else {
			DPRINT (F("ERR No such transducer: "));
			DPRINTLN (args);

			outBuf.print (ERR_MSG("No such transducer"));
		}
	} else {
		DPRINTLN (F("ERR Bad request"));
		outBuf.print (ERR_MSG("Bad request"));
	}

	// Send reply
	outBuf.print ('\n');
	comm -> reply ((const char *) outBuf, clientAddr);
}

void SensoriaServer::cmd_wri (const SensoriaAddress* clientAddr, char *args) {
	outBuf.begin ();
	outBuf.print (F("WRI "));

	if (args != NULL) {
		char *p[2];
		int n = splitString (args, p, 2);
		if (n == 2) {
			char* tName = p[0];
			char* data = p[1];

			Transducer *t = getTransducer (tName);
			if (t) {
				if (t -> type == Transducer::ACTUATOR) {
					// Do the unmrshaling, baby!
					Stereotype *st = getStereotype (t -> stereotype);
					st -> clear ();
					if (st -> unmarshal (data)) {
						if (t -> writeGeneric (st)) {
							outBuf.print (OK_FSTR);
							// Write succeeded, try to read back new status
							st -> clear ();
							if (t -> readGeneric (st)) {
								// Read ok, append result to reply
								clearSensorBuffer ();
								char *buf = st -> marshal (sensorBuf, SENSOR_BUF_SIZE);
								if (buf) {
									outBuf.print (' ');
									outBuf.print (buf);
								} else {
									/* Write succeded but marshaling the new
									 * status failed, report success anyway, the
									 * client can always try a new Read.
									 */
								}
							} else {
								/* Write succeded but subsequent Read failed,
								 * WTF??? Report success and leave it to the
								 * client to sort out the situation.
								 */
							}
						} else {
							// Write failed
							outBuf.print (ERR_MSG("Write failed"));
						}
					} else {
						DPRINT (F("Unmarshaling with "));
						DPRINT (st -> tag);
						DPRINT (F(" failed for: "));
						DPRINTLN (data);
						outBuf.print (ERR_MSG("Unmarshaling failed"));
					}
				} else {
					outBuf.print (ERR_MSG("Transducer is not an actuator"));
				}
			} else {
				DPRINT (F("ERR No such transducer: "));
				DPRINTLN (args);

				outBuf.print (ERR_MSG("No such transducer"));
			}
		} else {
			DPRINTLN (F("ERR Bad request"));
			outBuf.print (ERR_MSG("Bad request"));
		}
	} else {
		DPRINTLN (F("ERR Bad request"));
		outBuf.print (ERR_MSG("Bad request"));
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
	outBuf.print (F("NRQ "));

	// Get first arg
	if (args != NULL) {
		char *p[3];
		int n = splitString (args, p, 3);
		if (n >= 2) {
			char* tName = p[0];
			char* nTypeStr = p[1];

			const NotificationType type = parseNotificationTypeStr (nTypeStr);
			if (type != NT_UNK) {
				bool added = false;
				SensoriaAddress* notAddr = comm -> getNotificationAddress (clientAddr);
				if (notAddr) {
					const int i = findNotification (notAddr, type, tName);
					if (i >= 0) {
						// Request already exists, assume client lost track of it and return success
						DPRINTLN (F("NRQ already exists"));
						outBuf.print (OK_FSTR);
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

										outBuf.print (OK_FSTR);
										break;
									case NT_PRD:
										if (n < 3) {
											nNotificationReqs--;
											DPRINTLN (F("NRQ ERR No interval specified"));
											outBuf.print (ERR_MSG("No interval specified"));
										} else {
											word intv = atoi (p[2]);

											{
												char addrBuf[32];

												DPRINT (F("Notifying "));
												DPRINT (req.destAddr -> toString (addrBuf, sizeof (addrBuf)));
												DPRINT (F(" of values of "));
												DPRINT (t -> name);
												DPRINT (F(" every "));
												DPRINT (intv);
												DPRINTLN (F(" second(s)"));
											}

											req.type = NT_PRD;
											req.period = intv * 1000UL;

											outBuf.print (OK_FSTR);
										}
										break;
									default:
										break;
								}
							} else {
								DPRINTLN (F("NRQ ERR Max notification requests reached"));

								outBuf.print (ERR_MSG("Max notification requests reached"));
							}
						} else {
							DPRINT (F("NRQ ERR No such transducer: "));
							DPRINTLN (args);

							outBuf.print (ERR_MSG("No such transducer"));
						}
					}

					if (!added)
						comm -> releaseAddress (notAddr);
				} else {
					DPRINTLN (F("NRQ ERR No address available"));

					outBuf.print (ERR_MSG("No address available"));
				}
			} else {
				DPRINT (F("NRQ ERR Bad notification request type: "));
				DPRINTLN (nTypeStr);

				outBuf.print (ERR_MSG("Bad notification request type"));
			}
		} else {
			DPRINTLN (F("NRQ ERR Bad request"));
			outBuf.print (ERR_MSG("Bad request"));
		}
	} else {
		DPRINTLN (F("NRQ ERR Bad request"));
		outBuf.print (ERR_MSG("Bad request"));
	}

	// Send reply
	outBuf.print ('\n');
	comm -> reply ((const char *) outBuf, clientAddr);
}

/* Delete single notification request
 * --> NDL OT PRD [10]/NDL OT CHA
 * <-- NDL OK
 *
 * --> NDL ALL
 * <-- NDL OK
 */
void SensoriaServer::cmd_ndl (const SensoriaAddress* clientAddr, char *args) {
	outBuf.begin ();
	outBuf.print (F("NDL "));

	// Get first arg
	if (args != NULL) {
		char *p[3];
		int n = splitString (args, p, 3);
		if (n == 1) {
			// NDL ALL
			if (strcmp_P (p[0], PSTR("ALL")) == 0) {
				SensoriaAddress* notAddr = comm -> getNotificationAddress (clientAddr);
				if (notAddr) {
					/* Since deleting a NRQ results in all NRQs to its right
					 * shiting down one place, iterate on the array in reverse
					 * order.
					 */
					for (int i = nNotificationReqs - 1; i >= 0; --i) {
						NotificationRequest& req = notificationReqs[i];
						if (*req.destAddr == *clientAddr) {
							deleteNotification (i);
						}
					}

					outBuf.print (OK_FSTR);
				} else {
					DPRINTLN (F("NDL ERR No address available"));
					outBuf.print (ERR_MSG("No address available"));
				}
			} else {
				DPRINTLN (F("NDL ERR Bad request"));
				outBuf.print (ERR_MSG("Bad request"));
			}
		} else if (n >= 2) {
			// NDL <transducer> <type>
			char* tName = p[0];
			char* nTypeStr = p[1];

			NotificationType type = parseNotificationTypeStr (nTypeStr);
			if (type != NT_UNK) {
				SensoriaAddress* notAddr = comm -> getNotificationAddress (clientAddr);
				if (notAddr) {
					int i = findNotification (notAddr, type, tName);
					if (i >= 0) {
						//  Notification found
						deleteNotification (i);
						outBuf.print (OK_FSTR);
					} else {
						DPRINTLN (F("NDL ERR No such NRQ"));
						outBuf.print (ERR_MSG("No such notification request"));
					}

					comm -> releaseAddress (notAddr);
				} else {
					DPRINTLN (F("NDL ERR No address available"));
					outBuf.print (ERR_MSG("No address available"));
				}
			} else {
				DPRINT (F("NDL ERR Bad notification request type: "));
				DPRINTLN (nTypeStr);

				outBuf.print (ERR_MSG("Bad notification request type"));
			}
		} else {
			DPRINTLN (F("NDL ERR Bad request"));
			outBuf.print (ERR_MSG("Bad request"));
		}
	} else {
		DPRINTLN (F("NDL ERR Bad request"));
		outBuf.print (ERR_MSG("Bad request"));
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
	outBuf.print (F("NCL " OK_STR "\n"));
	comm -> reply ((const char *) outBuf, clientAddr);
}

#endif    // ENABLE_NOTIFICATIONS

#ifdef ENABLE_CMD_DIE
void SensoriaServer::cmd_die (const SensoriaAddress* clientAddr, char *args) {
	(void) *args;

	end ();

	// This can't really fail
	outBuf.begin ();
	outBuf.print (F("DIE " OK_STR "\n"));
	comm -> reply ((const char *) outBuf, clientAddr);
}
#endif

#ifdef ENABLE_CMD_RST

#include <avr/io.h>
#include <avr/wdt.h>

#define softReset() do {wdt_enable (WDTO_30MS); while(1) {}} while (0)

void SensoriaServer::cmd_rst (const SensoriaAddress* clientAddr, char *args) {
	(void) *args;

	// This can't really fail
	outBuf.begin ();
	outBuf.print (F("RST " OK_STR "\n"));
	comm -> reply ((const char *) outBuf, clientAddr);

	delay (500);
	softReset ();
}
#endif
