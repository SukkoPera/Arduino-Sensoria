#include "AllStereotypes.h"

static WeatherData wd;
static RelayData rs;
static ControlledRelayData cr;
static MotionData md;
static TimeControlData tc;

const char* TimeControlData::DAY_ABBREVS[NDAYS] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};


Stereotype* stereotypes[N_STEREOTYPES] = {
  &wd,
  &rs,
  &cr,
  &md,
  &tc
};
