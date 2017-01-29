#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <Sensoria.h>
#include <SensoriaCore/common.h>
#include <SensoriaCore/debug.h>
#include <SensoriaClient/TransducerProxy.h>
#include <SensoriaStereotypes/AllStereotypes.h>

class GenericNotificationReceiver {
public:
  TransducerProxy* transducer;
  NotificationType type;

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

public:
  void begin (SensoriaCommunicator& _comm) {
    comm = &_comm;
    nRec = 0;
  }

  boolean registerReceiver (GenericNotificationReceiver& rec, TransducerProxy& t, const NotificationType nType, const word period = 0) {
    boolean ret = false;
    if (nType != NT_PRD || period != 0) {
      // Parameters are good
      if (nRec < MAX_RECEIVERS) {
        if (t.requestNotification (nType, period)) {
          receivers[nRec++] = &rec;

          rec.transducer = &t;
          rec.type = nType;

          DPRINT (F("Notification request for "));
          DPRINT (t.name);
          DPRINTLN (F(" succeeded"));
          ret = true;
        } else {
          DPRINTLN (F("Notification request failed"));
        }
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
      for (int i = 0; i < nRec; i++) {
        GenericNotificationReceiver& rec = *receivers[i];
        if (strcmp (p[1], rec.transducer -> name) == 0) {
          DPRINT (F("Found interested NotificationReceiver: "));
          DPRINTLN (i);

          Stereotype* st = rec.transducer -> parseReply (p[2]);
          rec.onGenericNotification (st);
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

    if (comm -> receiveString (&buffer, &addr, &port, CC_NOTIFICATIONS)) {
      strstrip (buffer);
      DPRINT (F("Received on notifications socket: "));
      DPRINTLN (buffer);

      processNotification (buffer, addr, port);
    }
  }
};

#endif
