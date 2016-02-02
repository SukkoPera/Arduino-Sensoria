#ifndef _SENSORIA_H_INCLUDED
#define _SENSORIA_H_INCLUDED

#include "Transducer.h"
#include "Sensor.h"
#include "Actuator.h"
#include "internals/utils.h"
#include "internals/common.h"
#include "internals/debug.h"


#define OUT_BUF_SIZE 192
#define SENSOR_BUF_SIZE 32

class SensoriaServer {
private:
	static const byte MAX_SENSORS = 8;
	byte nTransducers;

	Transducer *transducers[MAX_SENSORS];

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

protected:
	void process_cmd (char *buffer);

	// Override to implement actual sending of data
	virtual boolean send (const char *str) = 0;

public:
	SensoriaServer ();

	// Override if needed, but always call super
	virtual boolean begin (FlashString _serverName, FlashString _serverVersion);

	// Override if needed
	virtual boolean stop ();

	int addTransducer (Transducer& transducer);

	Transducer *getTransducer (char *name) const;
};

#endif
