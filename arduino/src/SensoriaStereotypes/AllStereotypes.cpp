#include "AllStereotypes.h"

static WeatherData wd;
static RelayData rs;
static ControlledRelayData cr;
static MotionData md;
static TimeControlData tc;
static ValueSetData vs;
static DateTimeData dt;

const char* TimeControlData::DAY_ABBREVS[TimeControlData::NDAYS] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};


Stereotype* stereotypes[N_STEREOTYPES] = {
	&wd,
	&rs,
	&cr,
	&md,
	&tc,
	&vs,
	&dt
};
