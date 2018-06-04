#include <SensoriaCore/Actuator.h>
#include <SensoriaStereotypes/TimeControlData.h>

class Schedule: public Actuator<TimeControlData> {
private:

public:
	static const byte DAYS_PER_WEEK = 7;
	static const byte HOURS_PER_DAY = 24;
	static const byte MAX_ENTRY_VAL = 3;
	static const byte H_PER_ENTRY = 4;
	static const byte N_ENTRIES = HOURS_PER_DAY / H_PER_ENTRY;

	struct Entry {
		byte h0: 2;
		byte h1: 2;
		byte h2: 2;
		byte h3: 2;
	};

	Entry vals[DAYS_PER_WEEK][N_ENTRIES];

	Schedule () {
		reset ();
	}

	void reset (byte val = 0) {
		if (val > MAX_ENTRY_VAL)
			val = 0;

		for (byte d = 0; d < DAYS_PER_WEEK; d++)
			for (byte h = 0; h < HOURS_PER_DAY; h++)
				setValue (d, h, val);
	}

	// day is day of week, use timeDayOfWeek_t (Sunday is day 1)
	byte getValue (byte day, byte hour) const {
		byte q = hour / H_PER_ENTRY;
		byte r = hour % H_PER_ENTRY;

		byte ret = -1;
		switch (r) {
			case 0:
				ret = vals[day][q].h0;
				break;
			case 1:
				ret = vals[day][q].h1;
				break;
			case 2:
				ret = vals[day][q].h2;
				break;
			case 3:
				ret = vals[day][q].h3;
				break;
			default:
				break;
		}

		return ret;
	}

	// day is day of week, use timeDayOfWeek_t (Sunday is day 1)
	boolean setValue (const byte day, const byte hour, byte v) {
		boolean ret = true;

		byte q = hour / H_PER_ENTRY;
		byte r = hour % H_PER_ENTRY;

		if (day < DAYS_PER_WEEK && q < N_ENTRIES) {
			if (v > MAX_ENTRY_VAL) {
				DPRINT (F("Value exceeds max, capping to "));
				DPRINTLN (MAX_ENTRY_VAL);
				v = MAX_ENTRY_VAL;
			}

			switch (r) {
				case 0:
					vals[day][q].h0 = v;
					break;
				case 1:
					vals[day][q].h1 = v;
					break;
				case 2:
					vals[day][q].h2 = v;
					break;
				case 3:
					vals[day][q].h3 = v;
					break;
				default:
					ret = false;
					break;
			}
		} else {
			ret = false;
		}

		return ret;
	}

	boolean begin (FlashString name, FlashString description) {
		if (Actuator::begin (name, F("TC"), description)) {
			return true;
		} else {
			return false;
		}
	}

	boolean write (TimeControlData& tc) override {
		boolean ret = true;

		for (byte d = 0; d < DAYS_PER_WEEK; ++d) {
			for (byte h = 0; h < HOURS_PER_DAY; ++h) {
				setValue (d, h, tc.data[d][h]);
			}
		}

		return ret;
	}

	boolean read (TimeControlData& rd) override {
		boolean ret = true;

		for (byte d = 0; d < DAYS_PER_WEEK; ++d) {
			for (byte h = 0; h < HOURS_PER_DAY; ++h) {
				rd.data[d][h] = getValue (d, h);
			}
		}

		return ret;
	}
};
