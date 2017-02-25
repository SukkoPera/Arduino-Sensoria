#include "AllStereotypes.h"

static WeatherData wd;
static RelayData rs;
static ControlledRelayData cr;
static MotionData md;

Stereotype* stereotypes[N_STEREOTYPES] = {
  &wd,
  &rs,
  &cr,
  &md
};
