#ifndef _SENSORIAITERATOR_H_INCLUDED
#define _SENSORIAITERATOR_H_INCLUDED

#include <Arduino.h>

class ServerProxy;
class TransducerProxy;
class SensorProxy;
class ActuatorProxy;
class SensoriaClient;

class SensoriaIterator {
private:
  SensoriaClient* client;
  int8_t srv;
  int8_t t;

public:
  SensoriaIterator (SensoriaClient *_client);

  ServerProxy* nextNode ();

  TransducerProxy* nextTransducer ();

  SensorProxy* nextSensor ();

  ActuatorProxy* nextActuator ();
};

#endif
