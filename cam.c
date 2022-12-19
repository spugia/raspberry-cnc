#include "cam.h"

Action ** face(double Fxy, double Fz, double b, double X0, double Y0, double Z0, double Lx, double Ly, double dr, double h, double dz, int * A) {

  double xmin = X0 - Lx / 2 + b / 2;
  double xmax = X0 + Lx / 2 - b / 2;
  double ymin = Y0 - Ly / 2 + b / 2;
  double ymax = Y0 + Ly / 2 - b / 2;
  
  *A = 3;
  Action ** s = malloc(sizeof(Action *) * (*A));
  
  //.. go to start position
  s[0] = create_linear(NAN, NAN, Z0 + 0.25, FZ_max());
  s[1] = create_linear(xmin, ymin, NAN, FXY_max());
  s[2] = create_linear(NAN, NAN, Z0, FZ_max());
  
  //.. cut pocket
  bool min;
  
  double x;
  double y;
  double z = Z0 - dz;

  if (z < Z0 - h) { z = Z0 - h; }
  
  while (z >= Z0 - h) {

    s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));
    
    s[(*A)++] = create_linear(xmin, ymin, NAN, FX_max());
    s[(*A)++] = create_linear(NAN, NAN, z, Fz);

    x = xmin;
    min = true;
    
    while (x <= xmax) {

      s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));
      
      if (!(min = !min)) { y = ymax; }
      else               { y = ymin; }

      s[(*A)++] = create_linear(x, NAN, NAN, Fxy);
      s[(*A)++] = create_linear(NAN, y, NAN, Fxy);
      
      if (!lfeq(x, xmax) && (x + dr * b) > xmax) { x = xmax;    }
      else                                       { x += dr * b; }
    }
    
    if (!lfeq(z, -h + Z0) && (z - dz) < (-h + Z0)) { z = -h + Z0; }
    else                                           { z -= dz;     }
  }

  //.. return to safe point
  s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));

  s[(*A)++] = create_linear(NAN, NAN, Z0 + 0.25, FZ_max());
  s[(*A)++] = create_linear(X0, Y0, NAN, FXY_max());
  
  return s;
}

Action ** square_pocket(double Fxy, double Fz, double b, double X0, double Y0, double Z0, double Lx, double Ly, double dr, double h, double dz, int * A) {

  double xmin = X0 - Lx / 2 + b / 2;
  double xmax = X0 + Lx / 2 - b / 2;
  double ymin = Y0 - Ly / 2 + b / 2;
  double ymax = Y0 + Ly / 2 - b / 2;
  
  *A = 3;
  Action ** s = malloc(sizeof(Action *) * (*A));

  //.. go to start position
  s[0] = create_linear(NAN, NAN, Z0 + 0.25, FZ_max());
  s[1] = create_linear(xmin, ymin, NAN, FXY_max());
  s[2] = create_linear(NAN, NAN, Z0, FZ_max());

  //.. cut pocket
  bool min;

  double x;
  double y;
  double z = Z0 - dz;

  if (z < Z0 - h) { z = Z0 - h; }
  
  while (z >= Z0 - h) {

    s = (Action **) realloc(s, sizeof(Action *) * (*A + 6));

    s[(*A)++] = create_linear(xmin, ymin, NAN, FXY_max());
    s[(*A)++] = create_linear(NAN, NAN, z, Fz);
    s[(*A)++] = create_linear(xmin, ymax, NAN, Fxy);
    s[(*A)++] = create_linear(xmax, ymax, NAN, Fxy);
    s[(*A)++] = create_linear(xmax, ymin, NAN, Fxy);
    s[(*A)++] = create_linear(xmin, ymin, NAN, Fxy);
    
    x = xmin;
    min = true;

    while (x <= xmax) {

      s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));
      
      if (!(min = !min)) { y = ymax; }
      else               { y = ymin; }

      s[(*A)++] = create_linear(x, NAN, NAN, Fxy);
      s[(*A)++] = create_linear(NAN, y, NAN, Fxy);
      
      if (!lfeq(x, xmax) && (x + dr * b) > xmax) { x = xmax;    }
      else                                       { x += dr * b; }
    }
    
    if (!lfeq(z, -h + Z0) && (z - dz) < (-h + Z0)) { z = -h + Z0; }
    else                                           { z -= dz;     }    
  }

  //.. return to safe point
  s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));

  s[(*A)++] = create_linear(NAN, NAN, Z0 + 0.25, FZ_max());
  s[(*A)++] = create_linear(X0, Y0, NAN, FXY_max());
  
  return s;
}

Action ** circular_pocket(double Fxy, double Fz, double b, double X0, double Y0, double Z0, double Ri, double Ro, double dr, double h, double dz, int * A) {

  *A = 3;
  Action ** s = malloc(sizeof(Action *) * (*A));

  //.. go to start position
  s[0] = create_linear(NAN, NAN, Z0 + 0.25, FZ_max());
  s[1] = create_linear(X0, Y0 + Ro - b / 2, NAN, FXY_max());
  s[2] = create_linear(NAN, NAN, Z0, FZ_max());

  //.. cut pocket
  double z = Z0 - dz;

  if (z < Z0 - h) { z = Z0 - h; }
  
  while (z >= Z0 - h) {

    //.. go to new height
    s = (Action **) realloc(s, sizeof(Action *) * (*A + 1));
    s[(*A)++] = create_linear(NAN, NAN, z, Fz);

    //.. cut circles
    double y = Y0 + Ro - b / 2;

    while (y >= Y0 + Ri + b / 2) {

      s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));

      s[(*A)++] = create_linear(NAN, y, z, Fxy);
      s[(*A)++] = create_curve(X0, y, z, X0, Y0, Fxy);

      if (!lfeq(y, Y0 + Ri + b / 2) && (y - b * dr) < Y0 + Ri + b / 2) { y = Y0 + Ri + b / 2; }
      else                                                             { y -= b * dr;         }
    }

    s = (Action **) realloc(s, sizeof(Action *) * (*A + 1));
    s[(*A)++] = create_linear(NAN, Y0 + Ro - b / 2, z, Fxy);

    if (!lfeq(z, -h + Z0) && (z - dz) < (-h + Z0)) { z = -h + Z0; }
    else                                           { z -= dz;     }
  }

  //.. return to safe point
  s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));

  s[(*A)++] = create_linear(NAN, NAN, Z0 + 0.25, FZ_max());
  s[(*A)++] = create_linear(X0, Y0, NAN, FXY_max());
  
  return s;
}
