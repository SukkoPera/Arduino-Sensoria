#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <Sensoria.h>
#include <SensoriaCore/common.h>
#include <SensoriaCore/debug.h>
#include <SensoriaClient/TransducerProxy.h>
#include <SensoriaStereotypes/AllStereotypes.h>

class GenericNotificationReceiver {
public:
	boolean registered;
	TransducerProxy* transducer;
	NotificationType type;
	word period;
	unsigned long lastRegAttemptTime;

	virtual boolean onGenericNotification (Stereotype *st) = 0;
};

template <typename T>
class NotificationReceiver: public GenericNotificationReceiver {
public:
	boolean onGenericNotification (Stereotype *st) override {
		T& data = *static_cast<T*> (st);
		return onNotification (data);
	}

protected:
	virtual boolean onNotification (T& data) = 0;
};

class NotificationManager {
public:
	static const int MAX_RECEIVERS = 4;

private:
	SensoriaCommunicator *comm;
	GenericNotificationReceiver* receivers[MAX_RECEIVERS];
	int nRec = 0;

	void checkRegistrations () {
		for (byte i = 0; i < nRec; i++) {
			GenericNotificationReceiver& rec = *receivers[i];
			if (!rec.registered && (rec.lastRegAttemptTime == 0 || millis () - rec.lastRegAttemptTime >= 30000UL)) {
				// Try to register
				rec.registered = rec.transducer -> requestNotification (rec.type, rec.period);
				if (rec.registered) {
					DPRINT (F("Notification request for "));
					DPRINT (rec.transducer -> name);
					DPRINTLN (F(" succeeded"));
				} else {
					DPRINTLN (F("Notification request failed"));
				}

				rec.lastRegAttemptTime = millis ();
			}
		}
	}


public:
	void begin (SensoriaCommunicator& _comm) {
		comm = &_comm;
		nRec = 0;
	}

	int registerReceiver (GenericNotificationReceiver& rec, TransducerProxy& t, const NotificationType nType, const word period = 0) {
		int ret = -1;
		if (nType != NT_PRD || period != 0) {
			// Parameters are good
			if (nRec < MAX_RECEIVERS) {
				receivers[nRec] = &rec;
				rec.registered = false;
				rec.transducer = &t;
				rec.type = nType;
				rec.period = period;
				rec.lastRegAttemptTime = 0;

				ret = nRec++;;
			}
		}

		return ret;
	}

	// NOT TT T:12.34
	void processNotification (char* buffer, IPAddress senderAddr, uint16_t senderPort) {
		// Unused for the moment
		(void) senderAddr;
		(void) senderPort;

		char *p[3];
		if (splitString (buffer, p, 3) != 3) {
			DPRINT (F("Error parsing notification: \""));
			DPRINT (buffer);
			DPRINTLN ("\"");
		} else if (strcmp_P (p[0], PSTR ("NOT")) != 0) {
			DPRINT (F("Received non-notification on notification channel: \""));
			DPRINT (buffer);
			DPRINTLN ("\"");
		} else {
			int interested = 0;
			for (byte i = 0; i < nRec; i++) {
				GenericNotificationReceiver& rec = *receivers[i];
				if (strcmp (p[1], rec.transducer -> name) == 0) {
					DPRINT (F("Found interested NotificationReceiver: "));
					DPRINTLN (i);

					rec.transducer -> stereotype -> clear ();
					if (rec.transducer -> stereotype -> unmarshal (p[2])) {
						rec.onGenericNotification (rec.transducer -> stereotype);
					} else {
						DPRINT (F("Unmarshaling failed, cannot deliver notification to interested NotificationReceiver"));
					}
					interested++;
				}
			}

			if (interested == 0) {
				DPRINTLN (F("No interested NotificationReceivers found"));
			}
		}
	}

	void loop () {
		char *buffer;
		IPAddress addr;
		uint16_t port;

		if (comm -> receiveNotification (&buffer)) {
			strstrip (buffer);
			DPRINT (F("Received on notifications socket: "));
			DPRINTLN (buffer);

			processNotification (buffer, addr, port);
		}

		checkRegistrations ();
	}

	boolean isRegistered (int regId) {
		return regId < nRec && receivers[regId] -> registered;
	}
};

#endif
