#include "../include/cam.h"

Action ** square_pocket(CNC * cnc, double Lx, double Ly, double h, int * A) {

  double X0 = cnc -> X0;
  double Y0 = cnc -> Y0;
  double Z0 = cnc -> Z0;

  double b  = cnc -> tool.Dc;
  
  double Fxy = cnc -> mat.Fxy;
  double Fz  = cnc -> mat.Fz;
  double dr  = cnc -> mat.dr;
  double dz  = cnc -> mat.dz;
  
  double xmin = X0 - Lx / 2 + b / 2;
  double xmax = X0 + Lx / 2 - b / 2;
  double ymin = Y0 - Ly / 2 + b / 2;
  double ymax = Y0 + Ly / 2 - b / 2;
  
  *A = 3;
  Action ** s = malloc(sizeof(Action *) * (*A));

  //.. go to start position
  s[0] = create_linear(NAN, NAN, Z_OFF, NAN);
  s[1] = create_linear(xmin, ymin, NAN, NAN);
  s[2] = create_linear(NAN, NAN, cnc -> Z0, NAN);
  
  //.. cut pocket
  bool min;
  bool xstp;

  double x;
  double y;
  double z = Z0 - dz;

  if (z < Z0 - h) { z = Z0 - h; }

  if (Lx > Ly) { xstp = true;  }
  else         { xstp = false; }
  
  while (z >= Z0 - h) {

    s = (Action **) realloc(s, sizeof(Action *) * (*A + 6));

    s[(*A)++] = create_linear(xmin, ymin, NAN, NAN);
    s[(*A)++] = create_linear(NAN, NAN, z, Fz);
    s[(*A)++] = create_linear(xmin, ymax, NAN, Fxy);
    s[(*A)++] = create_linear(xmax, ymax, NAN, Fxy);
    s[(*A)++] = create_linear(xmax, ymin, NAN, Fxy);
    s[(*A)++] = create_linear(xmin, ymin, NAN, Fxy);

    min = true;

    if (xstp) {

      y = ymin + b / 2;

      while (y <= ymax - b / 2) {

	s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));

	if (!(min = !min)) { x = xmax - b / 2; }
	else               { x = xmin + b / 2; }

	s[(*A)++] = create_linear(NAN, y, NAN, Fxy);
	s[(*A)++] = create_linear(x, NAN, NAN, Fxy);
      
	if (!lfeq(y, ymax - b / 2) && (y + dr * b) > ymax - b / 2) { y = ymax - b / 2; }
	else                                                       { y += dr * b;      }
      }
      
    } else {
      
      x = xmin + b / 2;

      while (x <= xmax - b / 2) {

	s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));
      
	if (!(min = !min)) { y = ymax - b / 2; }
	else               { y = ymin + b / 2; }

	s[(*A)++] = create_linear(x, NAN, NAN, Fxy);
	s[(*A)++] = create_linear(NAN, y, NAN, Fxy);
      
	if (!lfeq(x, xmax - b / 2) && (x + dr * b) > xmax - b / 2) { x = xmax - b / 2; }
	else                                                       { x += dr * b;      }
      }
    }
    
    if (!lfeq(z, -h + Z0) && (z - dz) < (-h + Z0)) { z = -h + Z0; }
    else                                           { z -= dz;     }    
  }

  //.. return to safe point
  s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));

  s[(*A)++] = create_linear(NAN, NAN, Z_OFF, NAN);
  s[(*A)++] = create_linear(X0, Y0, NAN, NAN);
  
  return s;
}

Action ** nested_square_pocket(CNC * cnc, double Lxi, double Lyi, double Lxo, double Lyo, double h, int * A) {

  double X0 = cnc -> X0;
  double Y0 = cnc -> Y0;
  double Z0 = cnc -> Z0;

  double b  = cnc -> tool.Dc;
  
  double Fxy = cnc -> mat.Fxy;
  double Fz  = cnc -> mat.Fz;
  double dr  = cnc -> mat.dr;
  double dz  = cnc -> mat.dz;
  
  double xi = Lxi / 2 + b / 2;
  double yi = Lyi / 2 + b / 2;
  double xo = Lxo / 2 - b / 2;
  double yo = Lyo / 2 - b / 2;

  double xdiv = ceil((Lxo - Lxi) / 2 / (b * dr));
  double ydiv = ceil((Lyo - Lyi) / 2 / (b * dr));

  double rdiv = xdiv;
  if (ydiv > xdiv) { rdiv = ydiv; }

  double dx = (xo - xi) / (double) rdiv;
  double dy = (yo - yi) / (double) rdiv;
  
  *A = 3;
  Action ** s = malloc(sizeof(Action *) * (*A));

  //.. go to start position
  s[0] = create_linear(NAN, NAN, Z_OFF, NAN);
  s[1] = create_linear(-xi + X0, -yi + Y0, NAN, NAN);
  s[2] = create_linear(NAN, NAN, Z0, NAN);
  
  //.. cut pocket
  double x;
  double y;
  double z = Z0 - dz;

  if (z < Z0 - h) { z = Z0 - h; }

  while (z >= Z0 - h) {

    s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));
    
    //.. go to new height
    s[(*A)++] = create_linear(-xi + X0, -yi + Y0, NAN, NAN);
    s[(*A)++] = create_linear(NAN, NAN, z, Fz);

    //.. concentric cuts
    for (int r = 0 ; r <= rdiv ; r++) {

      x = xi + dx * (double) r;
      y = yi + dy * (double) r;

      s = (Action **) realloc(s, sizeof(Action *) * (*A + 5));    
            
      s[(*A)++] = create_linear(-x + X0, -y + Y0, NAN, Fxy);
      s[(*A)++] = create_linear(-x + X0,  y + Y0, NAN, Fxy);
      s[(*A)++] = create_linear( x + X0,  y + Y0, NAN, Fxy);
      s[(*A)++] = create_linear( x + X0, -y + Y0, NAN, Fxy);
      s[(*A)++] = create_linear(-x + X0, -y + Y0, NAN, Fxy);
    }
    
    if (!lfeq(z, -h + Z0) && (z - dz) < (-h + Z0)) { z = -h + Z0; }
    else                                           { z -= dz;     }    
  }  

  //.. return to safe point
  s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));

  s[(*A)++] = create_linear(NAN, NAN, Z_OFF, NAN);
  s[(*A)++] = create_linear(X0, Y0, NAN, NAN);
  
  return s;
}

Action ** drill(CNC * cnc, double h, int * A) {

  double X0 = cnc -> X0;
  double Y0 = cnc -> Y0;
  double Z0 = cnc -> Z0;

  double b  = cnc -> tool.Dc;
  
  double Fz = cnc -> mat.Fz;
  double dz = cnc -> mat.dr * b;
  
  *A = 4 + 3 * (ceil(h / dz) + 1);
  Action ** s = malloc(sizeof(Action *) * (*A));

  (*A) = 0;
  
  //.. go to start position
  s[(*A)++] = create_linear(NAN, NAN, Z_OFF, FZ_max());
  s[(*A)++] = create_linear(X0, Y0, NAN, NAN);
  s[(*A)++] = create_linear(NAN, NAN, Z0, FZ_max());

  //.. peck drilling
  double z = Z0;
  
  while (z >= Z0 - h) {

    s[(*A)++] = create_linear(NAN, NAN, z, Fz);
    
    if (!lfeq(z, -h + Z0) && (z - dz) < (-h + Z0)) { z = -h + Z0; }
    else {

      s[(*A)++] = create_linear(NAN, NAN, Z0, FZ_max());
      s[(*A)++] = create_linear(NAN, NAN, z,  FZ_max());

      z -= dz;
    }    
  }
  
  //.. return to safe
  s[(*A)++] = create_linear(NAN, NAN, Z_OFF, FZ_max());

  //.. realloc
  s = (Action **) realloc(s, sizeof(Action *) * (*A));
  
  return s;
}

Action ** bore(CNC * cnc, double R, double h, int * A) {

  double X0 = cnc -> X0;
  double Y0 = cnc -> Y0;
  double Z0 = cnc -> Z0;

  double b  = cnc -> tool.Dc;
  
  double Fxy = cnc -> mat.Fxy;
  double Fz  = cnc -> mat.Fz;
  
  *A = 3;
  Action ** s = malloc(sizeof(Action *) * (*A));

  double r = R - b / 2;
  
  //.. go to start position
  s[0] = create_linear(NAN, NAN, Z_OFF, NAN);
  s[1] = create_linear(X0, Y0 + r, NAN, NAN);
  s[2] = create_linear(NAN, NAN, Z0, NAN);

  //.. bore
  double dz = 2 * M_PI * fabs(r) / Fxy * Fz;
  double z  = Z0 - dz;

  while (z >= Z0 - h) {

    s = (Action **) realloc(s, sizeof(Action *) * (*A + 1));
  
    s[(*A)++] = create_curve(X0, Y0 + r, z, X0, Y0, DIR_CW, Fxy);
      
    //.. set new height
    if (!lfeq(z, -h + Z0) && (z - dz) < (-h + Z0)) { z = -h + Z0; }
    else                                           { z -= dz;     }
  }
  
  //.. finish and return to safe point
  s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));
  
  s[(*A)++] = create_curve(X0, Y0 + r, Z0 - h, X0, Y0, DIR_CW, Fxy);
  s[(*A)++] = create_linear(NAN, NAN, Z_OFF, NAN);
  
  return s;
}

Action ** circular_pocket(CNC * cnc, double Ri, double Ro, double h, int * A) {

  double X0 = cnc -> X0;
  double Y0 = cnc -> Y0;
  double Z0 = cnc -> Z0;

  double b  = cnc -> tool.Dc;
  
  double Fxy = cnc -> mat.Fxy;
  double Fz  = cnc -> mat.Fz;
  double dr  = cnc -> mat.dr;
  double dz  = cnc -> mat.dz;
  
  *A = 3;
  Action ** s = malloc(sizeof(Action *) * (*A));

  //.. go to start position
  s[0] = create_linear(NAN, NAN, Z_OFF, NAN);
  s[1] = create_linear(X0, Y0 + Ro - b / 2, NAN, NAN);
  s[2] = create_linear(NAN, NAN, Z0, NAN);

  //.. cut pocket
  double y, z = Z0 - dz;

  if (z < Z0 - h) { z = Z0 - h; }
  
  while (z >= Z0 - h) {

    //.. go to new height
    s = (Action **) realloc(s, sizeof(Action *) * (*A + 1));
    s[(*A)++] = create_linear(NAN, NAN, z, Fz);

    //.. cut circles
    y = Y0 + Ro - b / 2;

    while (y >= Y0 + Ri + b / 2) {

      s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));

      s[(*A)++] = create_linear(X0, y, z, Fxy);
      s[(*A)++] = create_curve(X0, y, z, X0, Y0, DIR_CW, Fxy);

      if (!lfeq(y, Y0 + Ri + b / 2) && (y - b * dr) < Y0 + Ri + b / 2) { y = Y0 + Ri + b / 2; }
      else                                                             { y -= b * dr;         }
    }

    s = (Action **) realloc(s, sizeof(Action *) * (*A + 1));
    s[(*A)++] = create_linear(X0, Y0 + Ro - b / 2, z, Fxy);

    //.. set new height
    if (!lfeq(z, -h + Z0) && (z - dz) < (-h + Z0)) { z = -h + Z0; }
    else                                           { z -= dz;     }
  }

  //.. return to safe point
  s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));

  s[(*A)++] = create_linear(NAN, NAN, Z_OFF, NAN);
  s[(*A)++] = create_linear(X0, Y0, NAN, NAN);
  
  return s;
}

Action ** radial_contour(CNC * cnc, double * xs, double * ys, double * x0s, double * y0s, enum DirType * ds, double offset, double h, int P, int * A) {

  double X0 = cnc -> X0;
  double Y0 = cnc -> Y0;
  double Z0 = cnc -> Z0;

  double b = cnc -> tool.Dc;
  
  double Fxy = cnc -> mat.Fxy;
  double Fz  = cnc -> mat.Fz;
  double dz  = cnc -> mat.dz;
  double dr  = cnc -> mat.dr;

  double xstr, ystr;

  double * xo = malloc(sizeof(double) * P);
  double * yo = malloc(sizeof(double) * P);
  
  //.. go to zero position
  offset_profile(xs, ys, xo, yo, P, offset);

  xstr = xo[0] + X0;
  ystr = yo[0] + Y0;
  
  *A = 3;
  Action ** s = malloc(sizeof(Action *) * (*A));
  s[0] = create_linear(NAN,   NAN, Z_OFF, NAN);
  s[1] = create_linear(xstr, ystr,   NAN, NAN);
  s[2] = create_linear(NAN,   NAN,    Z0, NAN);
  
  //.. cutting layers
  double z    = Z0 - dz;
  double doff = b * dr;
  double off;

  if (lfeq(doff, 0)) { doff = dr; }
  if (z < Z0 - h)    { z = Z0 - h; }
  
  while (z >= Z0 - h) {

    off = offset;

    //.. go to start
    s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));
    s[(*A)++] = create_linear(xstr, ystr, NAN, NAN);
    s[(*A)++] = create_linear(NAN, NAN, z, Fz);

    //.. radial increments
    while (off >= 0) {

      //.. calculate new profile
      offset_profile(xs, ys, xo, yo, P, off);
      
      s = (Action **) realloc(s, sizeof(Action *) * (*A + P));
      
      for (int p = 0 ; p < P ; p++) {
	
	if (ds[p] == DIR_None) { s[(*A)++] = create_linear(xo[p] + X0, yo[p] + Y0, NAN,                                  Fxy); }
	else                   { s[(*A)++] =  create_curve(xo[p] + X0, yo[p] + Y0, NAN, x0s[p] + X0, y0s[p] + Y0, ds[p], Fxy); }
      }
      
      if (!lfeq(off, 0) && (off - doff) < 0) { off  = 0;    }
      else                                   { off -= doff; }
    }

    //.. set new height
    if (!lfeq(z, -h + Z0) && (z - dz) < (-h + Z0)) { z = -h + Z0; }
    else                                           { z -= dz;     }
  }

  //.. return to safe point
  s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));
  s[(*A)++] = create_linear(xstr, ystr,   NAN, NAN);
  s[(*A)++] = create_linear(NAN,   NAN, Z_OFF, NAN);
  
  free(xo);
  free(yo);
  
  return s;
}

void offset_profile(double * x, double * y, double * X, double * Y, int P, double offset) {

  //.. zero offset case
  if (lfeq(offset, 0)) {

    for (int p = 0 ; p < P ; p++) {

      X[p] = x[p];
      Y[p] = y[p];
    }

    return;
  }
  
  //.. calculating scale factor
  double xcm, ycm;
  profile_cm(x, y, P, &xcm, &ycm);

  double x1 = x[0] - xcm;
  double y1 = y[0] - ycm;
  double x2 = x[1] - xcm;
  double y2 = y[1] - ycm;

  double S = 1 + offset * sqrt(pow(y2 - y1, 2) + pow(x2 - x1, 2)) / (x2 * y1 - x1 * y2);
  
  //.. apply offset
  for (int p = 0 ; p < P ; p++) {

    X[p] = x[p] * S;
    Y[p] = y[p] * S;
  }

  //.. adjusting center
  double xcm2, ycm2;
  profile_cm(X, Y, P, &xcm2, &ycm2);

  for (int p = 0 ; p < P ; p++) {

    X[p] += -xcm2 + xcm;
    Y[p] += -ycm2 + ycm;
  }
}

void profile_cm(double * xs, double * ys, int P, double * xcm, double * ycm) {

  *xcm = 0;
  *ycm = 0;
  
  for (int p = 1 ; p < P ; p++) {

    *xcm += xs[p];
    *ycm += ys[p];
  }

  *xcm /= (double) P;
  *ycm /= (double) P;
}

Action ** side_contour(CNC * cnc, double * xs, double * ys, double * x0s, double * y0s, enum DirType * ds, double ox, double oy, double h, int P, int * A) {

  
  
  return NULL;
}

Action ** engrave_text(CNC * cnc, char * text, double h, double a, int * A) {

  double X0 = cnc -> X0;
  double Y0 = cnc -> Y0;
  double Z0 = cnc -> Z0;

  double b  = cnc -> tool.Dc;
  
  double Fxy = cnc -> mat.Fxy;
  double Fz  = cnc -> mat.Fz;
  double dz  = cnc -> mat.dz;
  
  *A = 2;
  Action ** s = malloc(sizeof(Action *) * (*A));

  //.. go to zero position
  s[0] = create_linear(NAN, NAN, Z_OFF, NAN);
  s[1] = create_linear(X0, Y0, NAN, NAN);
  
  //.. text
  int C = strlen(text);

  double x, w = h / 3;
  
  for (int c = 0 ; c < C ; c++) {

    x = ((double) c + 0.5 - (double) C / 2) * 1.1 * w + X0;
          
    //.. cam by letter
    switch (toupper(text[c])) {
      
    case 'A': engrave_A(x, Y0, Z0, w, h, dz, Fxy, Fz, &s, A); break;
    case 'B': engrave_B(x, Y0, Z0, w, h, dz, Fxy, Fz, &s, A); break;
    case 'C': engrave_C(x, Y0, Z0, w, h, dz, Fxy, Fz, &s, A); break;
    default: break;
    }
  }

  // underline / box?

  //.. transform engraving
  
  //.. return to safe point
  s = (Action **) realloc(s, sizeof(Action *) * (*A + 1));
  
  s[(*A)++] = create_linear(NAN, NAN, Z_OFF, NAN);

  //.. free text
  free(text);
  
  return s;
}

void engrave_A(double X0, double Y0, double Z0, double w, double h, double dz, double Fxy, double Fz, Action *** s, int * A) {
  
  //.. resize action list
  (*s) = (Action **) realloc(*s, sizeof(Action *) * (*A + 8));

  //.. goto starting position
  (*s)[(*A)++] = create_linear(        NAN,         NAN, Z0 + Z_OFF, NAN);
  (*s)[(*A)++] = create_linear(-w / 2 + X0, -h / 2 + Y0,        NAN, NAN);
  (*s)[(*A)++] = create_linear(        NAN,         NAN,         Z0, NAN);
  (*s)[(*A)++] = create_linear(        NAN,         NAN,    Z0 - dz, Fz);

  //.. cut letter
  double m  = -2 * h / w;
  double xb = (h / 2 - h) / m;
    
  (*s)[(*A)++] = create_linear(         X0,  h / 2 + Y0, NAN, Fxy);
  (*s)[(*A)++] = create_linear( w / 2 + X0, -h / 2 + Y0, NAN, Fxy);
  (*s)[(*A)++] = create_linear( xb    + X0,          Y0, NAN, Fxy);
  (*s)[(*A)++] = create_linear(-xb    + X0,         NAN, NAN, Fxy);
}

void engrave_B(double X0, double Y0, double Z0, double w, double h, double dz, double Fxy, double Fz, Action *** s, int * A) {

  //.. resize action list
  (*s) = (Action **) realloc(*s, sizeof(Action *) * (*A + 11));

  //.. goto starting position
  (*s)[(*A)++] = create_linear(        NAN,         NAN, Z0 + Z_OFF, NAN);
  (*s)[(*A)++] = create_linear(-w / 2 + X0, -h / 2 + Y0,        NAN, NAN);
  (*s)[(*A)++] = create_linear(        NAN,         NAN,         Z0, NAN);
  (*s)[(*A)++] = create_linear(        NAN,         NAN,    Z0 - dz, Fz);
  
  //.. cut letter
  (*s)[(*A)++] = create_linear(NAN,          h / 2 + Y0, NAN, Fxy);
  (*s)[(*A)++] = create_linear( w / 2 + X0,         NAN, NAN, Fxy);
  (*s)[(*A)++] = create_linear(NAN,                  Y0, NAN, Fxy);
  (*s)[(*A)++] = create_linear(-w / 2 + X0,         NAN, NAN, Fxy);
  (*s)[(*A)++] = create_linear( w / 2 + X0,         NAN, NAN, Fxy);
  (*s)[(*A)++] = create_linear(NAN,         -h / 2 + Y0, NAN, Fxy);
  (*s)[(*A)++] = create_linear(-w / 2 + X0,         NAN, NAN, Fxy);
}

void engrave_C(double X0, double Y0, double Z0, double w, double h, double dz, double Fxy, double Fz, Action *** s, int * A) {

  //.. resize action list
  (*s) = (Action **) realloc(*s, sizeof(Action *) * (*A + 7));

  //.. goto starting position
  (*s)[(*A)++] = create_linear(       NAN,        NAN, Z0 + Z_OFF, NAN);
  (*s)[(*A)++] = create_linear(w / 2 + X0, h / 2 + Y0,        NAN, NAN);
  (*s)[(*A)++] = create_linear(       NAN,        NAN,         Z0, NAN);
  (*s)[(*A)++] = create_linear(       NAN,        NAN,    Z0 - dz, Fz);

  //.. cut letter
  (*s)[(*A)++] = create_linear(-w / 2 + X0,         NAN, NAN, Fxy);
  (*s)[(*A)++] = create_linear(NAN,         -h / 2 + Y0, NAN, Fxy);
  (*s)[(*A)++] = create_linear( w / 2 + X0,         NAN, NAN, Fxy);
}
