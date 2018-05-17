#ifndef THRESHOLDER_H_INCLUDED
#define THRESHOLDER_H_INCLUDED

#include <SensoriaCore/Actuator.h>
#include <SensoriaStereotypes/ValueSetData.h>

template <byte N>
class ThresholdHolder: public Actuator<ValueSetData> {
public:
	static const byte N_THRES = N;

	signed int thresholds[N_THRES];

	boolean begin (FlashString name, FlashString description) {
		return Actuator::begin (name, F("VS"), description, F("20180511"));
	}

	boolean write (ValueSetData& vs) override {
		boolean ret = true;

		for (byte i = 0; i < min (vs.nData, N_THRES); ++i) {
			int tmp;
			if (!vs.getDataInt (i, tmp)) {
				DPRINT (F("Invalid threshold for V"));
				DPRINT (i + 1);
				DPRINT (F(": "));
				DPRINTLN (vs.data[i]);

				ret = false;
			} else {
				thresholds[i] = tmp;
			}
		}

		return ret;
	}

	boolean read (ValueSetData& vs) override {
		bool ret = true;

		for (byte i = 0; ret && i < N_THRES; ++i) {
			ret = ret && vs.append (thresholds[i]);
		}

		return ret;
	}
};

#endif
