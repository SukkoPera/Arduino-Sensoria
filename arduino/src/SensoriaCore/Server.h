#ifndef _SENSORIA_H_INCLUDED
#define _SENSORIA_H_INCLUDED

#include <Arduino.h>
#include <IPAddress.h>
#include "Transducer.h"
#include "Sensor.h"
#include "Actuator.h"
#include "Communicator.h"
#include "Stereotype.h"
#include "utils.h"
#include "common.h"
#include "debug.h"

#define OUT_BUF_SIZE 192
#define SENSOR_BUF_SIZE 32

class SensoriaServer {
private:
	SensoriaCommunicator *comm;

	byte nTransducers;

	Transducer *transducers[MAX_TRANSDUCERS];

	char buf[OUT_BUF_SIZE];
	char sensorBuf[SENSOR_BUF_SIZE];

	FlashString serverName;
	FlashString serverVersion;

	uint32_t hash;

	Stereotype* getStereotype (FlashString s);

	void clearBuffer ();

	void clearSensorBuffer ();

	void cmd_qry (char *args);

	void cmd_ver (const char *args);

	void cmd_rea (char *args);

	void cmd_wri (char *args);

	boolean send_srv (const char *str, boolean cr = false);

#ifdef ENABLE_FLASH_STRINGS
	boolean send_srv (const __FlashStringHelper *str, boolean cr = false);
#endif

	// Commodity method to flush data to server
	boolean send_srv ();

	// Temp?
	IPAddress remoteAddress;
	uint16_t remotePort;

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
