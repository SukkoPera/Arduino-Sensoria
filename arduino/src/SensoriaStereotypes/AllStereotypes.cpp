#include "AllStereotypes.h"

static WeatherData wd;
static RelayData rs;
static ControlledRelayData cr;

Stereotype* stereotypes[N_STEREOTYPES] = {
  &wd,
  &rs,
  &cr
};
