#include "../include/units.h"

double r2v(double v, enum UnitType unit) {

  switch (unit) {

  case IN: return v;
  case MM: return v * 25.4;
  default: break;
  }

  return v;
}

double v2r(double r, enum UnitType unit) {

  switch (unit) {

  case IN: return r;
  case MM: return r / 25.4;
  default: break;
  }

  return r;
}

char * unit_name(enum UnitType unit) {

  switch(unit) {

  case IN: return "in";
  case MM: return "mm";
  default: break;
  }

  return "invalid type";
}
