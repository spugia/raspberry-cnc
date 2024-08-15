#include "../include/cam.h"

Action ** face(double Fxy, double Fz, double b, double X0, double Y0, double Z0, double Lx, double Ly, double dr, double h, double dz, int * A) {
  
  double xmin = X0 - Lx / 2 + b / 2;
  double xmax = X0 + Lx / 2 - b / 2;
  double ymin = Y0 - Ly / 2 + b / 2;
  double ymax = Y0 + Ly / 2 - b / 2;
  
  *A = 3;
  Action ** s = malloc(sizeof(Action *) * (*A));
  
  //.. go to start position
  s[0] = create_linear(NAN, NAN, Z_OFF, NAN);
  s[1] = create_linear(xmin, ymin, NAN, NAN);
  s[2] = create_linear(NAN, NAN, Z0, NAN);
  
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

    s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));
    
    s[(*A)++] = create_linear(xmin, ymin, NAN, NAN);
    s[(*A)++] = create_linear(NAN, NAN, z, Fz);

    min = true;
    
    if (xstp) {
	
      y = ymin;
    
      while (y <= ymax) {

	s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));
      
	if (!(min = !min)) { x = xmax; }
	else               { x = xmin; }

	s[(*A)++] = create_linear(x, NAN, NAN, Fxy);
	s[(*A)++] = create_linear(NAN, y, NAN, Fxy);
      
	if (!lfeq(y, ymax) && (y + dr * b) > ymax) { y = ymax;    }
	else                                       { y += dr * b; }
      }

    } else {

      x = xmin;

      while (x <= xmax) {

	s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));

	if (!(min = !min)) { y = ymax; }
	else               { y = ymin; }

	s[(*A)++] = create_linear(NAN, y, NAN, Fxy);
	s[(*A)++] = create_linear(x, NAN, NAN, Fxy);

	if (!lfeq(x, xmax) && (x + dr * b) > xmax) { x = xmax;    }
	else                                       { x += dr * b; }
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

Action ** square_pocket(double Fxy, double Fz, double b, double X0, double Y0, double Z0, double Lx, double Ly, double dr, double h, double dz, int * A) {

  double xmin = X0 - Lx / 2 + b / 2;
  double xmax = X0 + Lx / 2 - b / 2;
  double ymin = Y0 - Ly / 2 + b / 2;
  double ymax = Y0 + Ly / 2 - b / 2;
  
  *A = 3;
  Action ** s = malloc(sizeof(Action *) * (*A));

  //.. go to start position
  s[0] = create_linear(NAN, NAN, Z0, NAN);
  s[1] = create_linear(xmin, ymin, NAN, NAN);
  s[2] = create_linear(NAN, NAN, Z0, NAN);
  
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

      y = ymin;

      while (y <= ymax) {

	s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));

	if (!(min = !min)) { x = xmax; }
	else               { x = xmin; }

	s[(*A)++] = create_linear(NAN, y, NAN, Fxy);
	s[(*A)++] = create_linear(x, NAN, NAN, Fxy);
      
	if (!lfeq(y, ymax) && (y + dr * b) > ymax) { y = ymax;    }
	else                                       { y += dr * b; }
      }
      
    } else {
      
      x = xmin;

      while (x <= xmax) {

	s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));
      
	if (!(min = !min)) { y = ymax; }
	else               { y = ymin; }

	s[(*A)++] = create_linear(x, NAN, NAN, Fxy);
	s[(*A)++] = create_linear(NAN, y, NAN, Fxy);
      
	if (!lfeq(x, xmax) && (x + dr * b) > xmax) { x = xmax;    }
	else                                       { x += dr * b; }
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

Action ** nested_square_pocket(double Fxy, double Fz, double b, double X0, double Y0, double Z0, double Lxi, double Lyi, double Lxo, double Lyo, double dr, double h, double dz, int * A) {

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

Action ** circular_pocket(double Fxy, double Fz, double b, double X0, double Y0, double Z0, double Ri, double Ro, double dr, double h, double dz, int * A) {

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

Action ** cutout(double Fxy, double Fz, double b, double Z0, double xmin, double ymin, double xmax, double ymax, double rtl, double rtr, double rbr, double rbl, double h, double dz, int * A) {

  int a = (rtl > 0) + (rtr > 0) + (rbr > 0) + (rbl > 0);
  
  *A = 3;
  Action ** s = malloc(sizeof(Action *) * (*A));

  //.. go to start position
  s[0] = create_linear(NAN, NAN, Z_OFF, NAN);
  s[1] = create_linear(xmin, ymin + rbl, NAN, NAN);
  s[2] = create_linear(NAN, NAN, Z0, NAN);

  //.. cut pocket
  double z = Z0 - dz;

  if (z < Z0 - h) { z = Z0 - h; }
  
  while (z >= Z0 - h) {

    s = (Action **) realloc(s, sizeof(Action *) * (*A + 5 + a));
    
    //.. go to new height
    s[(*A)++] = create_linear(NAN, NAN, z, Fz);

    //.. cutting groove
    s[(*A)++] = create_linear(NAN, ymax - rtl, NAN, Fxy);
    if (rtl > 0) { s[(*A)++] = create_curve(xmin + rtl, ymax, NAN, xmin + rtl, ymax - rtl, DIR_CW, Fxy); }

    s[(*A)++] = create_linear(xmax - rtr, NAN, NAN, Fxy);
    if (rtr > 0) { s[(*A)++] = create_curve(xmax, ymax - rtr, NAN, xmax - rtr, ymax - rtr, DIR_CW, Fxy); }
    
    s[(*A)++] = create_linear(NAN, ymin + rbr, NAN, Fxy);
    if (rbr > 0) { s[(*A)++] = create_curve(xmax - rbr, ymin, NAN, xmax - rbr, ymin + rbr, DIR_CW, Fxy); }
    
    s[(*A)++] = create_linear(xmin + rbl, NAN, NAN, Fxy);
    if (rbl > 0) { s[(*A)++] = create_curve(xmin, ymin + rbl, NAN, xmin + rbl, ymin + rbl, DIR_CW, Fxy); }
    
    //.. set new height
    if (!lfeq(z, -h + Z0) && (z - dz) < (-h + Z0)) { z = -h + Z0; }
    else                                           { z -= dz;     }
  }

  //.. return to safe point
  s = (Action **) realloc(s, sizeof(Action *) * (*A + 1));
  
  s[(*A)++] = create_linear(NAN, NAN, Z_OFF, NAN);
  
  return s;  
}

Action ** bore(double Fxy, double Fz, double b, double X0, double Y0, double Z0, double R, double h, int * A) {

  *A = 3;
  Action ** s = malloc(sizeof(Action *) * (*A));

  double r = R - b;
  
  //.. go to start position
  s[0] = create_linear(NAN, NAN, Z_OFF, NAN);
  s[1] = create_linear(X0, Y0 + r, NAN, NAN);
  s[2] = create_linear(NAN, NAN, Z0, NAN);

  //.. bore
  double dz = 2 * M_PI * r / Fxy * Fz;
  double z = Z0 - dz;
  
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

Action ** drill(double Fz, double X0, double Y0, double Z0, double h, int * A) {

  *A = 5;
  Action ** s = malloc(sizeof(Action *) * (*A));

  //.. go to start position
  s[0] = create_linear(NAN, NAN, Z_OFF, NAN);
  s[1] = create_linear(X0, Y0, NAN, NAN);
  s[2] = create_linear(NAN, NAN, Z0, NAN);

  //.. drill
  s[3] = create_linear(NAN, NAN, Z0 - h, Fz);
  
  //.. return to safe
  s[4] = create_linear(NAN, NAN, Z_OFF, NAN);

  return s;
}

Action ** fillet(double Fxy, double Fz, double b, double X0, double Y0, double Z0, double R, double a, double h, double dz, int * A) {
  
  *A = 3;
  Action ** s = malloc(sizeof(Action *) * (*A));

  //.. start and end points
  double r  = R + b / 2;
  double x1 = r * cos(a) + X0;
  double y1 = r * sin(a) + Y0;
  double x2 = r * cos(a + M_PI / 2) + X0;
  double y2 = r * sin(a + M_PI / 2) + Y0;

  //.. go to start position
  s[0] = create_linear(NAN, NAN, Z_OFF, NAN);
  s[1] = create_linear(x1, y1, NAN, NAN);
  s[2] = create_linear(NAN, NAN, Z0, NAN);

  //.. cut pocket
  bool dir = true;
  
  double x, y;
  double z = Z0 - dz;

  if (z < Z0 - h) { z = Z0 - h; }
  
  while (z >= Z0 - h) {

    s = (Action **) realloc(s, sizeof(Action *) * (*A + 2));
    
    //.. go to new height
    s[(*A)++] = create_linear(NAN, NAN, z, Fz);

    //.. cutting groove
    x = x1;
    y = y1;
    
    if (dir) {
      x = x2;
      y = y2;
    }

    dir = !dir;

    s[(*A)++] = create_curve(x, y, NAN, X0, Y0, DIR_CW, Fxy);
    
    //.. set new height
    if (!lfeq(z, -h + Z0) && (z - dz) < (-h + Z0)) { z = -h + Z0; }
    else                                           { z -= dz;     }
  }

  //.. return to safe point
  s = (Action **) realloc(s, sizeof(Action *) * (*A + 1));
  
  s[(*A)++] = create_linear(NAN, NAN, Z_OFF, NAN);
  
  return s;  
}

Action ** groove(double Fxy, double Fz, double X0, double Y0, double Z0, double * Xs, double * Ys, int P, double h, double dz, int * A) {

  //.. offset positions
  for (int p = 0 ; p < P ; p++) {

    Xs[p] += X0;
    Ys[p] += Y0;
  }
  
  //.. go to start position
  *A = 3;
  Action ** s = malloc(sizeof(Action *) * (*A));

  s[0] = create_linear(NAN, NAN, Z_OFF, NAN);
  s[1] = create_linear(Xs[0], Ys[0], NAN, NAN);
  s[2] = create_linear(NAN, NAN, Z0, NAN);

  //.. cut groove
  bool forward = true;
  
  double z = Z0 - dz;

  if (z < Z0 - h) { z = Z0 - h; }
  
  while (z >= Z0 - h) {

    s = (Action **) realloc(s, sizeof(Action *) * (*A + P));
    
    //.. go to new height
    s[(*A)++] = create_linear(NAN, NAN, z, Fz);

    //.. cutting groove
    if (forward) {

      for (int p = 1 ; p < P ; p++) { s[(*A)++] = create_linear(Xs[p], Ys[p], NAN, Fxy); }
      
    } else {

      for (int p = P - 2 ; p >= 0 ; p--) { s[(*A)++] = create_linear(Xs[p], Ys[p], NAN, Fxy); }
    }

    forward = !forward;
    
    //.. set new height
    if (!lfeq(z, -h + Z0) && (z - dz) < (-h + Z0)) { z = -h + Z0; }
    else                                           { z -= dz;     }
  }

  //.. return to safe point
  s = (Action **) realloc(s, sizeof(Action *) * (*A + 1));
  
  s[(*A)++] = create_linear(NAN, NAN, Z_OFF, NAN);

  free(Xs);
  free(Ys);
  
  return s;   
}

Action ** engrave_text(double Fxy, double Fz, char * text, double X0, double Y0, double Z0, double h, double dz, double a, int * A) {

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
    case 'D': break;
    case 'E': break;
    case 'F': break;
    case 'G': break;
    case 'H': break;
    case 'I': break;
    case 'J': break;
    case 'K': break;
    case 'L': break;
    case 'M': break;
    case 'N': break;
    case 'O': break;
    case 'P': break;
    case 'Q': break;
    case 'R': break;
    case 'S': break;
    case 'T': break;
    case 'U': break;
    case 'V': break;
    case 'W': break;
    case 'X': break;
    case 'Y': break;
    case 'Z': break;
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
