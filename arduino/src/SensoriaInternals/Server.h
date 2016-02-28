#ifndef _SENSORIA_H_INCLUDED
#define _SENSORIA_H_INCLUDED

<<<<<<< HEAD
#include <Arduino.h>
#include <IPAddress.h>
#include "Transducer.h"
#include "Sensor.h"
#include "Actuator.h"
#include "Communicator.h"
=======
#include "Transducer.h"
#include "Sensor.h"
#include "Actuator.h"
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
#include "utils.h"
#include "common.h"
#include "debug.h"

#define OUT_BUF_SIZE 192
#define SENSOR_BUF_SIZE 32

class SensoriaServer {
private:
<<<<<<< HEAD
  SensoriaCommunicator *comm;

	byte nTransducers;

	Transducer *transducers[MAX_TRANSDUCERS];
=======
	static const byte MAX_SENSORS = 8;
	byte nTransducers;

	Transducer *transducers[MAX_SENSORS];
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9

	char buf[OUT_BUF_SIZE];
	char sensorBuf[SENSOR_BUF_SIZE];

  FlashString serverName;
  FlashString serverVersion;

	uint32_t hash;

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

<<<<<<< HEAD
  // Temp?
  IPAddress remoteAddress;
  uint16_t remotePort;

protected:
	void process_cmd (char *buffer, IPAddress senderAddr, uint16_t senderPort);
=======
protected:
	void process_cmd (char *buffer);

	// Override to implement actual sending of data
	virtual boolean send (const char *str) = 0;
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9

public:
	SensoriaServer ();

<<<<<<< HEAD
	boolean begin (FlashString _serverName, SensoriaCommunicator& _comm);

  void loop ();
=======
	// Override if needed, but always call super
	virtual boolean begin (FlashString _serverName, FlashString _serverVersion);

	// Override if needed
	virtual boolean stop ();
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9

	int addTransducer (Transducer& transducer);

	Transducer *getTransducer (char *name) const;
};

#endif
