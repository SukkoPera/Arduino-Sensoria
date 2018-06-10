#ifndef _SENSORIA_H_INCLUDED
#define _SENSORIA_H_INCLUDED

#include <Arduino.h>
#include <IPAddress.h>
#include <PString.h>
#include "Transducer.h"
#include "Sensor.h"
#include "Actuator.h"
#include "Communicator.h"
#include "Stereotype.h"
#include "NotificationRequest.h"
#include "utils.h"
#include "common.h"
#include "debug.h"

//~ #define ENABLE_CMD_DIE
//~ #define ENABLE_CMD_RST

class SensoriaServer {
public:
	static const byte PROTOCOL_VERSION = 1;

private:
#ifdef ARDUINO_ARCH_STM32F1
	static const word OUT_BUF_SIZE = 512;
#else
	static const byte OUT_BUF_SIZE = 192;
#endif

	SensoriaCommunicator* comm;

	byte nTransducers;

	Transducer* transducers[MAX_TRANSDUCERS];

#ifdef ENABLE_NOTIFICATIONS
	byte nNotificationReqs;

	NotificationRequest notificationReqs[MAX_NOTIFICATION_REQS];
#endif

#ifdef ENABLE_CMD_DIE
	boolean running;
#endif

	char outBufRaw[OUT_BUF_SIZE];
	PString outBuf;
	char sensorBuf[SENSOR_BUF_SIZE];

	FlashString serverName;

	Stereotype* getStereotype (FlashString s);

	void clearBuffer ();

	void clearSensorBuffer ();

#ifdef ENABLE_NOTIFICATIONS
	int findNotification (const SensoriaAddress* clientAddr, NotificationType type, char* tName);

	NotificationType parseNotificationTypeStr (char *nTypeStr);

	void handleNotificationReqs ();
#endif

	void cmd_hlo (const SensoriaAddress* clientAddr, char *args);

	void cmd_rea (const SensoriaAddress* clientAddr, char *args);

	void cmd_wri (const SensoriaAddress* clientAddr, char *args);

	void cmd_nrq (const SensoriaAddress* clientAddr, char *args);

#ifdef ENABLE_NOTIFICATIONS
	void cmd_ndl (const SensoriaAddress* clientAddr, char *args);

	void cmd_ncl (const SensoriaAddress* clientAddr, char *args);
#endif

#ifdef ENABLE_CMD_DIE
	void cmd_die (const SensoriaAddress* clientAddr, char *args);
#endif

#ifdef ENABLE_CMD_RST
	void cmd_rst (const SensoriaAddress* clientAddr, char *args);
#endif

protected:
	void process_cmd (char *buffer, const SensoriaAddress* senderAddr);

public:
	SensoriaServer ();

	boolean begin (FlashString _serverName, SensoriaCommunicator& _comm);

	boolean end ();

	void loop ();

	int addTransducer (Transducer& transducer);

	Transducer *getTransducer (char *name) const;
};

#endif
