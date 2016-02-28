#include "SensoriaClient.h"
#include "ServerProxy.h"
#include "TransducerProxy.h"
#include "SensoriaIterator.h"

SensoriaIterator::SensoriaIterator (SensoriaClient *_client): client (_client), srv (-1), t (-1) {
}

ServerProxy* SensoriaIterator::nextNode () {
  ServerProxy* ret = NULL;

  if (srv < client -> nServers - 1)
    ret = client -> servers[++srv];

  return ret;
}

TransducerProxy* SensoriaIterator::nextTransducer () {
  TransducerProxy* ret = NULL;

  if (srv >= 0 || nextNode ()) {
    ServerProxy *srvpx = client -> servers[srv];
    if (t < srvpx -> nTransducers - 1) {
      ret = srvpx -> transducers[++t];
    } else {
      // Next server
      if (nextNode ()) {
        t = -1;
        ret = nextTransducer ();
      }
    }
  }

  return ret;
}

SensorProxy* SensoriaIterator::nextSensor () {
  TransducerProxy* ret;

  do {
    ret = nextTransducer ();
  } while (ret && ret -> type != TYPE_SENSOR);

  return static_cast<SensorProxy *> (ret);
}

ActuatorProxy* SensoriaIterator::nextActuator () {
  TransducerProxy* ret;

  do {
    ret = nextTransducer ();
  } while (ret && ret -> type != TYPE_ACTUATOR);

  return static_cast<ActuatorProxy *> (ret);
}
