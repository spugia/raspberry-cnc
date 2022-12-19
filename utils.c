#include "utils.h"

double sign(double val) {
  
  if (val < 0) { return -1.0; }
  
  return 1.0;
}

bool lfeq(double f1, double f2) {

  return fabs(f1 - f2) < TOLF;
}

bool seq(char * str1, char * str2) {
  
  if (str1 == NULL || str2 == NULL) { return false; }
  
  if (strlen(str1) != strlen(str2)) { return false; }

  for (int c = 0 ; c < strlen(str2) ; ++c) {

    if (tolower(str1[c]) != tolower(str2[c])) { return false; }
  }

  return true;
}

bool parse_lf(char * str, double * val) {

  char * endptr = 0;
  strtod(str, &endptr);

  if (*endptr != '\0' || endptr == str) { return false; }

  (*val) = atof(str);
  
  return true;
}

double asin2(double x) {

  double y = asin(x);

  if (y < 0) { y = fabs(y) + M_PI; }

  return y;
}

double acos2(double x) {

  double y = acos(x);

  if (y < 0) { y = fabs(y) + M_PI; }

  return y;
}
