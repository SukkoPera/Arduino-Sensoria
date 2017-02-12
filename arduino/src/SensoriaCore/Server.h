#ifndef _SENSORIA_H_INCLUDED
#define _SENSORIA_H_INCLUDED

#include <Arduino.h>
#include <IPAddress.h>
#include "Transducer.h"
#include "Sensor.h"
#include "Actuator.h"
#include "Communicator.h"
#include "Stereotype.h"
#include "NotificationRequest.h"
#include "utils.h"
#include "common.h"
#include "debug.h"

#define OUT_BUF_SIZE 192

class SensoriaServer {
private:
	SensoriaCommunicator* comm;

	byte nTransducers;

	Transducer* transducers[MAX_TRANSDUCERS];

#ifdef ENABLE_NOTIFICATIONS
	byte nNotificationReqs;

	NotificationRequest notificationReqs[MAX_NOTIFICATION_REQS];
#endif

	char buf[OUT_BUF_SIZE];
	char sensorBuf[SENSOR_BUF_SIZE];

	FlashString serverName;
	FlashString serverVersion;

	uint32_t hash;

	Stereotype* getStereotype (FlashString s);

	void clearBuffer ();

	void clearSensorBuffer ();

#ifdef ENABLE_NOTIFICATIONS
  int findNotification (IPAddress& addr, word port, NotificationType type, char* tName);

  NotificationType parseNotificationTypeStr (char *nTypeStr);

	void handleNotificationReqs ();
#endif

	void cmd_qry (char *args);

	void cmd_ver (const char *args);

	void cmd_rea (char *args);

	void cmd_wri (char *args);

	void cmd_nrq (char *args);

#ifdef ENABLE_NOTIFICATIONS
	void cmd_ndl (char *args);

	void cmd_ncl (char *args);
#endif

	boolean send_srv (const char *str, boolean cr = false, IPAddress* destAddr = NULL, word* destPort = NULL);

#ifdef ENABLE_FLASH_STRINGS
	boolean send_srv (const __FlashStringHelper *str, boolean cr = false);
#endif

	// Commodity method to flush data to server
	boolean send_srv ();

	// Temp?
	IPAddress remoteAddress;
	word remotePort;

protected:
	void process_cmd (char *buffer, IPAddress senderAddr, uint16_t senderPort);

public:
	SensoriaServer ();

	boolean begin (FlashString _serverName, SensoriaCommunicator& _comm);

	void loop ();

	int addTransducer (Transducer& transducer);

	Transducer *getTransducer (char *name) const;
};

#endif
