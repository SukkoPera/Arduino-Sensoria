#ifndef MOTIONDATA_H_INCLUDED
#define MOTIONDATA_H_INCLUDED

#include <SensoriaCore/Stereotype.h>
#include <SensoriaCore/utils.h>
#include <SensoriaCore/debug.h>


class MotionData: public Stereotype {
public:
	boolean motionDetected;

	MotionData (): Stereotype ("MD") {
	}

  MotionData& operator= (MotionData& other) {
    motionDetected = other.motionDetected;

    return *this;
  }

  bool operator== (Stereotype const& genericOther) override {
    MotionData const& other = static_cast<MotionData const&> (genericOther);
    return motionDetected == other.motionDetected;
  }

	virtual void clear () override {
		motionDetected = false;
	}

	boolean unmarshal (char *s) override {
		strupr (s);
		if (strcmp_P (s, PSTR ("MOTION")) == 0) {
			motionDetected = true;
    } else {
      motionDetected = false;
		}

		return true;
	}

	char *marshal (char *buf, unsigned int size) override {
		if (size >= 10) {
			buf[0] = '\0';

      if (motionDetected) {
					strncat_P (buf, PSTR ("MOTION"), size);
      } else {
					strncat_P (buf, PSTR ("NO_MOTION"), size);
			}
		} else {
			buf = NULL;
		}

		return buf;
	}
};

#endif
