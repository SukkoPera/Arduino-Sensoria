#ifndef _SENSOR_H_INCLUDED
#define _SENSOR_H_INCLUDED

#include "Transducer.h"


class Sensor: public Transducer {
private:
  //~ static const char *type_strings[];

public:
  Sensor (): Transducer (Transducer::SENSOR) {
  }



//  virtual ~Sensor () {
//  }

  //~ const char *get_type_string () const {
    //~ if (type < N_TYPES)
      //~ return type_strings[type];
    //~ else
      //~ return NULL;
  //~ }

  // Called automatically by server before read()
  //~ void clearBuffer () {
    //~ for (int i = 0; i < BUF_SIZE; i++)
      //~ buf[i] = '\0';
  //~ }

  /* Override to implement the actual sensor reading and reporting.
   * A buffer that can be used to contain the result is provided, of
   * the given size, initialized to all-zeros. Feel free to use it or
   * provide your own. Just return whatever buffer you used.
   */
  virtual char *read (char *buf, const byte size) = 0;

  virtual void configure (const char *name _UNUSED, char *value _UNUSED) {
  }
};

#endif
