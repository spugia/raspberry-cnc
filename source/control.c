#include "../include/control.h"

double FX_max(void) { return 1000000000 / (SPR_X * RPI_X * 2 * SPS_23) * 60; }

double FY_max(void) { return 1000000000 / (SPR_Y * RPI_Y * 2 * SPS_23) * 60; }

double FZ_max(void) { return 1000000000 / (SPR_Z * RPI_Z * 2 * SPS_23) * 60; }

double FXY_max(void) {

  if (FX_max() < FY_max()) { return FX_max(); }

  return FY_max();
}

double F_max(double dx, double dy, double dz) {

  if (isnan(dx)) { dx = 0; }
  if (isnan(dy)) { dy = 0; }
  if (isnan(dz)) { dz = 0; }
  
  double tx = fabs(dx) / FX_max();
  double ty = fabs(dy) / FY_max();
  double tz = fabs(dz) / FZ_max();

  if (lfeq(dx, 0)) { tx = 0; }
  if (lfeq(dy, 0)) { ty = 0; }
  if (lfeq(dz, 0)) { tz = 0; }
  
  if (!lfeq(dx, 0) && tx >= ty && tx >= tz) { return FX_max(); }
  if (!lfeq(dy, 0) && ty >= tx && ty >= tz) { return FY_max(); }
  if (!lfeq(dz, 0) && tz >= tx && tz >= ty) { return FZ_max(); }

  return FZ_max();
}

double F_accel(double l, double L, double A, double Fmax) {

  double F = A * l + FMIN;

  if (l / L > 0.5) { F = A * (L - l) + FMIN; }

  if (F > Fmax) { F = Fmax; }

  return F;
}

Action * create_linear(double X, double Y, double Z, double F) {

  Action * linear = malloc(sizeof(struct Action));

  linear -> type = Linear;
  linear -> dir  = DIR_None;
  
  linear -> X = X;
  linear -> Y = Y;
  linear -> Z = Z;

  linear -> X0 = 0;
  linear -> Y0 = 0;
  
  linear -> F = F;

  return linear;
}

Action * create_curve(double X, double Y, double Z, double X0, double Y0, enum DirType dir, double F) {

  Action * curve = malloc(sizeof(struct Action));

  curve -> type = Curve;
  curve -> dir  = dir;
  
  curve -> X = X;
  curve -> Y = Y;
  curve -> Z = Z;

  curve -> X0 = X0;
  curve -> Y0 = Y0;
  
  curve -> F = F;

  return curve;
}

void compile_linear(Action * action, double X, double Y, double Z, int32_t rox, int32_t roy, int32_t roz, int32_t ROX, int32_t ROY, int32_t ROZ, unsigned int ** mx, unsigned int ** my, unsigned int ** mz, unsigned int ** nx, unsigned int ** ny, unsigned int ** nz, unsigned long long ** times, int * S) {

  //.. distance along each axis
  double dx = 0;
  double dy = 0;
  double dz = 0;

  if (!isnan(action -> X)) { dx = X - action -> X; }
  if (!isnan(action -> Y)) { dy = Y - action -> Y; }
  if (!isnan(action -> Z)) { dz = Z - action -> Z; }

  //.. feed rate
  double Fact = action -> F;
  double Fmax = F_max(dx, dy, dz);
  
  if (isnan(Fact)) { Fact = Fmax; }

  //.. steps along each axis
  int Sx = (int) round(fabs(dx) * RPI_X * SPR_X);
  int Sy = (int) round(fabs(dy) * RPI_Y * SPR_Y);
  int Sz = (int) round(fabs(dz) * RPI_Z * SPR_Z);

  int32_t Rx, Ry, Rz;
  
  //.. step size in each axis
  double dxs = 1 / (RPI_X * SPR_X);
  double dys = 1 / (RPI_Y * SPR_Y);
  double dzs = 1 / (RPI_Z * SPR_Z);
  
  //.. allocating memory
  if (dx > 0) { Rx = rox;       }
  else        { Rx = ROX - rox; }

  if (dy > 0) { Ry = roy;       }
  else        { Ry = ROY - roy; }

  if (dz > 0) { Rz = roz;       }
  else        { Rz = ROZ - roz; }
  
  if (lfeq(dx, 0)) { Rx = 0; }
  if (lfeq(dy, 0)) { Ry = 0; }
  if (lfeq(dz, 0)) { Rz = 0; }

  (*S) = Sx + Sy + Sz + Rx + Ry + Rz;
  
  *mx     = malloc(sizeof(unsigned int)       * (*S));
  *my     = malloc(sizeof(unsigned int)       * (*S));
  *mz     = malloc(sizeof(unsigned int)       * (*S));
  *nx     = malloc(sizeof(unsigned int)       * (*S));
  *ny     = malloc(sizeof(unsigned int)       * (*S));
  *nz     = malloc(sizeof(unsigned int)       * (*S));
  *times  = malloc(sizeof(unsigned long long) * (*S));

  *mx = (unsigned int *) memset(*mx, 0, sizeof(unsigned int) * (*S));
  *my = (unsigned int *) memset(*my, 0, sizeof(unsigned int) * (*S));
  *mz = (unsigned int *) memset(*mz, 0, sizeof(unsigned int) * (*S));

  *nx = (unsigned int *) memset(*nx, 0, sizeof(unsigned int) * (*S));
  *ny = (unsigned int *) memset(*ny, 0, sizeof(unsigned int) * (*S));
  *nz = (unsigned int *) memset(*nz, 0, sizeof(unsigned int) * (*S));

  (*S) = 0;

  double x  = 0;
  double y  = 0;
  double z  = 0;
  double ll = 0;
  double l  = 0;
  double L  = sqrt(dx * dx + dy * dy + dz * dz);
  double F;
  
  bool duplicate;
  unsigned long long t;
  unsigned long long tr = 0;
  
  //.. x run-out  
  for (int32_t s = 0 ; s < Rx ; s++) {

    tr += (unsigned long long) round(dxs / FRUN * 60 * 1000000000);

    (*times)[*S] = tr;
    (*mx)[*S]    = PUL_X;
    (*nx)[*S]    = dx > 0;

    (*S)++;
  }
  
  //.. y run-out
  
  for (int32_t s = 0 ; s < Ry ; s++) {

    tr += (unsigned long long) round(dys / FRUN * 60 * 1000000000);
    
    (*times)[*S] = tr;
    (*my)[*S]    = PUL_Y;
    (*ny)[*S]    = dy > 0;

    (*S)++;
  }

  //.. z run-out  
  for (int32_t s = 0 ; s < Rz ; s++) {

    tr += (unsigned long long) round(dzs / FRUN * 60 * 1000000000);
    
    (*times)[*S] = tr;
    (*mz)[*S]    = PUL_Z;
    (*nz)[*S]    = dz > 0;

    (*S)++;
  }

  //.. compiling x-axis steps
  t = tr;
  
  for (unsigned long long s = 0 ; s < Sx ; s++) {

    x += dxs;
    y = x * fabs(dy / dx);
    z = x * fabs(dz / dx);

    l = sqrt(x * x + y * y + z * z);
    F = F_accel(l, L, FAR, Fact);

    t += (unsigned long long) round((l - ll) / F * 60 * 1000000000);
    
    (*times)[*S] = t;
    (*mx)[*S]    = PUL_X;
    (*nx)[*S]    = dx > 0;

    (*S)++;
    
    ll = l;
  }

  //.. compiling y-axis steps
  t  = tr;
  y  = 0;
  ll = 0;
  
  for (unsigned long long s = 0 ; s < Sy ; ++s) {

    y += dys;
    x = y * fabs(dx / dy);
    z = y * fabs(dz / dy);

    l = sqrt(x * x + y * y + z * z);
    F = F_accel(l, L, FAR, Fact);

    t += (unsigned long long) round((l - ll) / F * 60 * 1000000000);
    
    ll = l;
    
    duplicate = false;
    
    for (int s2 = 0 ; s2 < *S ; ++s2) {

      if ((*times)[s2] == t) {

        (*my)[s2] = PUL_Y;
        (*ny)[s2] = dy > 0;

        duplicate = true;
        break;
      }
    }

    if (!duplicate) {

      (*times)[*S] = t;
      (*my)[*S]    = PUL_Y;
      (*ny)[*S]    = dy > 0;
      
      (*S)++;
    }
  }

  //.. compiling z-axis steps
  t  = tr;
  z  = 0;
  ll = 0;

  for (unsigned long long s = 0 ; s < Sz ; ++s) {

    z += dzs;
    x = z * fabs(dx / dz);
    y = z * fabs(dy / dz);

    l = sqrt(x * x + y * y + z * z);
    F = F_accel(l, L, FAR, Fact);

    t += (unsigned long long) round((l - ll) / F * 60 * 1000000000);
    
    ll = l;

    duplicate = false;
    
    for (int s2 = 0 ; s2 < *S ; ++s2) {

      if ((*times)[s2] == t) {

        (*mz)[s2] = PUL_Z;
        (*nz)[s2] = dz < 0;
        
        duplicate = true;
        break;
      }
    }

    if (!duplicate) {

      (*times)[*S] = t;
      (*mz)[*S]    = PUL_Z;
      (*nz)[*S]    = dz < 0;
      
      (*S)++;
    }
  }
  
  //.. resizing data
  *times = (unsigned long long *) realloc(*times, sizeof(unsigned long long) * (*S));
  *mx    =       (unsigned int *) realloc(*mx,    sizeof(unsigned int)       * (*S));
  *my    =       (unsigned int *) realloc(*my,    sizeof(unsigned int)       * (*S));
  *mz    =       (unsigned int *) realloc(*mz,    sizeof(unsigned int)       * (*S));
  *nx    =       (unsigned int *) realloc(*nx,    sizeof(unsigned int)       * (*S));
  *ny    =       (unsigned int *) realloc(*ny,    sizeof(unsigned int)       * (*S));
  *nz    =       (unsigned int *) realloc(*nz,    sizeof(unsigned int)       * (*S));
}

void compile_curve(Action * action, double X, double Y, double Z, int32_t rox, int32_t roy, int32_t roz, int32_t ROX, int32_t ROY, int32_t ROZ, unsigned int ** mx, unsigned int ** my, unsigned int ** mz, unsigned int ** nx, unsigned int ** ny, unsigned int ** nz, unsigned long long ** times, int * S) {

  //.. step sizes
  double dxs = 1 / (RPI_X * SPR_X);
  double dys = 1 / (RPI_Y * SPR_Y);
  double dzs = 1 / (RPI_Z * SPR_Z);

  double dxymin = dxs;
  if (dys < dxs) { dxymin = dys; }

  //.. near position check
  if (fabs(X - action -> X) < dxs) { X = action -> X; }
  if (fabs(Y - action -> Y) < dys) { Y = action -> Y; }
  
  //.. angles
  double T1 = atan3(          X - action -> X0, Y           - action -> Y0);
  double T2 = atan3(action -> X - action -> X0, action -> Y - action -> Y0);
  
  //.. radius
  double R = sqrt(pow(X - action -> X0, 2) + pow(Y - action -> Y0, 2));

  //.. arc length
  double L;

  if (lfeq(T1, T2)) { L = 2 * M_PI * R;                         }
  else              { L = arc_length(R, T1, T2, action -> dir); }
  
  //.. feed rate
  double F_act = action -> F;

  if (isnan(F_act)) { F_act = F_max(L, L, Z - action -> Z); }

  //.. render curve
  unsigned long long P;
  int ** points = render_curve(R, dxs, dys, &P);
  
  order_points(&points, &P, T1, T2, action -> dir);

  //.. allocating data structures
  int32_t Rz;
  double dz = 0;

  if (!isnan(action -> Z)) { dz = Z - action -> Z; }
  
  if (dz > 0) { Rz = roz;       }
  else        { Rz = ROZ - roz; }
  
  if (lfeq(dz, 0)) { Rz = 0; }

  int zsteps = (int) round(fabs(dz) / dzs);
  
  (*S) = 2 * P + zsteps + Rz + 2 * ROX + 2 * ROY + ROZ;
  
  if (*S == 0) { (*S) = 1; R = 0; }
  
  *times = malloc(sizeof(unsigned long long) * (*S));
  *mx    = malloc(sizeof(unsigned int)       * (*S));
  *my    = malloc(sizeof(unsigned int)       * (*S));
  *mz    = malloc(sizeof(unsigned int)       * (*S));
  *nx    = malloc(sizeof(unsigned int)       * (*S));
  *ny    = malloc(sizeof(unsigned int)       * (*S));
  *nz    = malloc(sizeof(unsigned int)       * (*S));
  
  *mx = (unsigned int *) memset(*mx, 0, sizeof(unsigned int) * (*S));
  *my = (unsigned int *) memset(*my, 0, sizeof(unsigned int) * (*S));
  *mz = (unsigned int *) memset(*mz, 0, sizeof(unsigned int) * (*S));
  *nx = (unsigned int *) memset(*nx, 0, sizeof(unsigned int) * (*S));
  *ny = (unsigned int *) memset(*ny, 0, sizeof(unsigned int) * (*S));
  *nz = (unsigned int *) memset(*nz, 0, sizeof(unsigned int) * (*S));

  (*S) = 0;
  
  if (R < dxymin) { return; }

  //.. z run-out
  unsigned long long time = 0;
  
  for (int32_t s = 0 ; s < Rz ; s++) {

    time += (unsigned long long) round(dzs / FRUN * 60 * 1000000000);
    
    (*times)[*S] = time;
    (*mz)[*S]    = PUL_Z;
    (*nz)[*S]    = dz > 0;

    (*S)++;
  }
  
  //.. compiling x-y steps
  int sx, sy;

  int ix, iy, iz;
  int ixl = points[0][0];
  int iyl = points[0][1];
  int izl = 0;
  
  double t1, t, F, dl, l = 0;

  t1 = atan3(points[0][0], points[0][1]);
  
  for (int p = 1 ; p < P ; p++) {

    ix = points[p][0];
    iy = points[p][1];

    if (ix != ixl || iy != iyl) {

      sx = ix - ixl;
      sy = iy - iyl;

      //.. runout - x
      if (sx > 0) {

	for (; rox <= ROX ; rox++) {

	  time += (unsigned long long) round(dxs / FRUN * 60 * 1000000000);
	  
	  (*times)[(*S)] = time;
	  (*mx)[(*S)]    = PUL_X;
	  (*nx)[(*S)]    = sx < 0;

	  (*S)++;
	}
	
      } else if (sx < 0) {

	for (; rox >= 0 ; rox--) {

	  time += (unsigned long long) round(dxs / FRUN * 60 * 1000000000);

	  (*times)[(*S)] = time;
	  (*mx)[(*S)]    = PUL_X;
	  (*nx)[(*S)]    = sx < 0;

	  (*S)++;
	}
      }

      //.. runout - y
      if (sy > 0) {

	for (; roy <= ROY ; roy++) {

	  time += (unsigned long long) round(dys / FRUN * 60 * 1000000000);
	  
	  (*times)[(*S)] = time;
	  (*my)[(*S)]    = PUL_Y;
	  (*ny)[(*S)]    = sy < 0;

	  (*S)++;
	}
	
      } else if (sy < 0) {

	for (; roy >= 0 ; roy--) {

	  time += (unsigned long long) round(dys / FRUN * 60 * 1000000000);

	  (*times)[(*S)] = time;
	  (*my)[(*S)]    = PUL_Y;
	  (*ny)[(*S)]    = sy < 0;

	  (*S)++;
	}
      }

      //.. x/y step
      t = atan3(points[p][0], points[p][1]);
      l = arc_length(R, t1, t, action -> dir);
      
      dl = sqrt(pow((double) (ix - ixl) * dxs, 2) + pow((double) (iy - iyl) * dys, 2));
      
      F = F_accel(l, L, FAR, action -> F);

      time += (unsigned long long) round(dl / F * 60 * 1000000000);

      (*times)[(*S)] = time;
      (*mx)[(*S)]    = PUL_X * (ix != ixl);
      (*my)[(*S)]    = PUL_Y * (iy != iyl);

      (*nx)[(*S)] = sx < 0;
      (*ny)[(*S)] = sy < 0;
      
      (*S)++;

      //.. z step
      iz = (int) round(l / L * (double) zsteps);

      if (iz != izl) {

	(*times)[(*S)] = time;
	(*mz)[(*S)]    = PUL_Z;
	(*nz)[(*S)]    = dz < 0;
      }
      
      //.. last posotion
      ixl = ix;
      iyl = iy;
      izl = iz;
    }
    
    free(points[p]);
  }
  
  free(points);

  //.. resizing data structures
  *times = (unsigned long long *) realloc(*times, sizeof(unsigned long long) * (*S));
  *mx    =       (unsigned int *) realloc(*mx,    sizeof(unsigned int)       * (*S));
  *my    =       (unsigned int *) realloc(*my,    sizeof(unsigned int)       * (*S));
  *mz    =       (unsigned int *) realloc(*mz,    sizeof(unsigned int)       * (*S));  
  *nx    =       (unsigned int *) realloc(*nx,    sizeof(unsigned int)       * (*S));
  *ny    =       (unsigned int *) realloc(*ny,    sizeof(unsigned int)       * (*S));
  *nz    =       (unsigned int *) realloc(*nz,    sizeof(unsigned int)       * (*S));
}

int ** render_curve(double R, double dxs, double dys, unsigned long long * P) {

  //.. normalize parameters
  int Rn = (int) round(R  / dys);

  double dxn = 1;
  double dyn = dys / dxs;
  
  //.. memory allocation
  *P = 0;
  int ** points = (int **) malloc(sizeof(int *));

  //.. render initial points
  int x = 0;
  int y = Rn;
  int d = 1 - Rn;

  render_points(x, y, dxn, dyn, &points, P);
  
  //.. render remaining points
  while (x < y) {

    if (d < 0) { d += 2 * x + 3;            }
    else       { d += 2 * (x - y) + 5; y--; }

    x++;
    render_points(x, y, dxn, dyn, &points, P);
  }

  return points;
}

void render_points(int X, int Y, double dx, double dy, int *** points, unsigned long long * P) {

  int candidates[8][2] = {

    { + round((double) X * dx), + round((double) Y * dy) },
    { - round((double) X * dx), + round((double) Y * dy) },
    { + round((double) X * dx), - round((double) Y * dy) },
    { - round((double) X * dx), - round((double) Y * dy) },
    { + round((double) Y * dx), + round((double) X * dy) },
    { - round((double) Y * dx), + round((double) X * dy) },
    { + round((double) Y * dx), - round((double) X * dy) },
    { - round((double) Y * dx), - round((double) X * dy) }
  };

  *points = realloc(*points, sizeof(int *) * (*P + 8));
  
  for (unsigned int i = 0 ; i < 8 ; i++) {

    (*points)[*P] = malloc(sizeof(int) * 2);
      
    (*points)[*P][0] = candidates[i][0];
    (*points)[*P][1] = candidates[i][1];

    (*P)++;
  }
}

void order_points(int *** points, unsigned long long * P, double T1, double T2, enum DirType dir) {
  
  double T;
  unsigned int i, j;

  //.. compute points properties
  double * angles = malloc(sizeof(double) * (*P));
  bool * betweens = malloc(sizeof(bool)   * (*P));
  
  for (i = 0 ; i < *P ; i++) {

    T = atan3((*points)[i][0], (*points)[i][1]);
    
    betweens[i] = between_angles(T1, T2, T, dir);
    angles[i]   = arc_length(1, T1, T, dir);
  }

  //.. resize arrays
  unsigned long long P2 = 0;
  
  double * angles2 = malloc(sizeof(double) * (*P));
  int ** points2   = malloc(sizeof(int *)  * (*P));

  for (i = 0 ; i < *P ; i++) {

    if (betweens[i]) {
      
      angles2[P2] =    angles[i];
      points2[P2] = (*points)[i];

      P2++;
      
    } else { free((*points)[i]); }
  }

  free(betweens);
  free(angles);
  free(*points);

  angles  = NULL;
  *points = NULL;
  
  *P = P2;
  angles  = angles2;
  *points = points2;

  //.. order angles
  int x, y;
  
  for (i = 0 ; i < *P ; i++) {
    
    for (j = 0 ; j < *P - i - 1 ; j++) {

      if (angles[j] > angles[j + 1]) {
	  
	//.. swap points
	x = (*points)[j][0];
	y = (*points)[j][1];
        
	(*points)[j][0] = (*points)[j + 1][0];
	(*points)[j][1] = (*points)[j + 1][1];
	
	(*points)[j + 1][0] = x;
	(*points)[j + 1][1] = y;

	//.. swap angles
	T = angles[j];
	    
	angles[j] = angles[j + 1];
	angles[j + 1] = T;
      }
    }
  }

  //.. add extra point
  if (lfeq(fabs(T2 - T1), 2 * M_PI) || lfeq(T1, T2)) {

    (*points) = (int **) realloc(*points, sizeof(int *) * (++(*P)));

    (*points)[*P - 1] = malloc(sizeof(int) * 2);
    (*points)[*P - 1][0] = (*points)[0][0];
    (*points)[*P - 1][1] = (*points)[0][1];
  }
  
  //.. cleanup
  free(angles);
}

bool between_angles(double T1, double T2, double T, enum DirType dir) {

  //.. complete circle
  if (lfeq(T1, T2) || lfeq(fabs(T2 - T1), 2 * M_PI)) { return true; }

  //.. incomplete circle
  if (dir == DIR_CCW) {

    //.. counter-clockwise
    if (T1 < T2) { return T1 <= T && T <= T2; }
    else         { return T >= T1 || T <= T2; }
    
  } else if (dir == DIR_CW) {

    //.. clockwise
    if (T2 < T1) { return T2 <= T && T <= T1; }
    else         { return T >= T2 || T <= T1; }
  }
  
  return false;
}

double arc_length(double R, double T1, double T2, enum DirType dir) {

  double theta = 0;

  if (dir == DIR_CW) {

    if (T2 > T1) { theta = 2 * M_PI - (T2 - T1); }
    else         { theta = T1 - T2;              }

  } else {

    if (T2 > T1) { theta = T2 - T1;              }
    else         { theta = 2 * M_PI - (T1 - T2); }
  }
  
  return R * theta;
}

void sort_action(unsigned long long * time, unsigned int * mx, unsigned int * my, unsigned int * mz, unsigned int * nx, unsigned int * ny, unsigned int * nz, int S) {

  int mindex;

  unsigned int tmpnx;
  unsigned int tmpny;
  unsigned int tmpnz;
  
  unsigned int tmpmx;
  unsigned int tmpmy;
  unsigned int tmpmz;
  
  unsigned long long tmptime;
  
  for (int s = 0 ; s < S - 1 ; ++s) {

    mindex  = s;
    tmptime = time[mindex];
    
    for (int s2 = s + 1 ; s2 < S ; ++s2) {

      if (time[s2] < tmptime) {

	tmptime = time[s2];
	mindex  = s2;
      }
    }

    if (mindex != s) {

      tmpmx   =   mx[s];
      tmpmy   =   my[s];
      tmpmz   =   mz[s];
      tmpnx   =   nx[s];
      tmpny   =   ny[s];
      tmpnz   =   nz[s];
      tmptime = time[s];

      mx[s]   =   mx[mindex];
      my[s]   =   my[mindex];
      mz[s]   =   mz[mindex];
      nx[s]   =   nx[mindex];
      ny[s]   =   ny[mindex];
      nz[s]   =   nz[mindex];
      time[s] = time[mindex];

      mx[mindex]   = tmpmx;
      my[mindex]   = tmpmy;
      mz[mindex]   = tmpmz;
      nx[mindex]   = tmpnx;
      ny[mindex]   = tmpny;
      nz[mindex]   = tmpnz;
      time[mindex] = tmptime;
    }
  }
}

void splice_action(unsigned long long * times, struct timespec ** steps, int S) {

  (*steps) = malloc(sizeof(struct timespec) * S);

  double delay;
  double last = 0;

  for (int s = 0 ; s < S ; ++s) {
    
    delay = times[s] - last - 2 * SPS_23;
    last  = times[s];

    (*steps)[s].tv_sec  = (int) ((double) delay / (double) 1000000000);
    (*steps)[s].tv_nsec = delay;
  }
}

void execute_action(CNC * cnc, struct timespec * steps, unsigned int * mx, unsigned int * my, unsigned int * mz, unsigned int * nx, unsigned int * ny, unsigned int * nz, int S, bool * pause, bool * stop) {

  struct timespec delay;

  delay.tv_sec  = 0;
  delay.tv_nsec = SPS_23;

  double dx = 1 / (RPI_X * SPR_X);
  double dy = 1 / (RPI_Y * SPR_Y);
  double dz = 1 / (RPI_Z * SPR_Z);
  
  for (int s = 0 ; s < S ; ++s) {
    
    //.. setting direction
    if (s == 0 || nx[s] != nx[s-1]) { gpioWrite(DIR_X, nx[s]); }
    if (s == 0 || ny[s] != ny[s-1]) { gpioWrite(DIR_Y, ny[s]); }
    if (s == 0 || nz[s] != nz[s-1]) { gpioWrite(DIR_Z, nz[s]); }
    nanosleep(&delay, NULL);
    
    //.. pulsing
    if (mx[s] > 0) { gpioWrite(mx[s], 1); }
    if (my[s] > 0) { gpioWrite(my[s], 1); }
    if (mz[s] > 0) { gpioWrite(mz[s], 1); }
    nanosleep(&delay, NULL);

    if (mx[s] > 0) { gpioWrite(mx[s], 0); }
    if (my[s] > 0) { gpioWrite(my[s], 0); }
    if (mz[s] > 0) { gpioWrite(mz[s], 0); }
    nanosleep(&(steps[s]), NULL);

    //.. updating position
    if (mx[s] > 0) {

      cnc -> rox += (nx[s] != 0 ? -1 : 1);
      
      if      (cnc -> rox < 0)          { cnc -> rox = 0;          cnc -> X -= dx; }
      else if (cnc -> rox > cnc -> ROX) { cnc -> rox = cnc -> ROX; cnc -> X += dx; } 
    }

    if (my[s] > 0) {

      cnc -> roy += (ny[s] != 0 ? -1 : 1);  
    
      if      (cnc -> roy < 0)          { cnc -> roy = 0;          cnc -> Y -= dy; }
      else if (cnc -> roy > cnc -> ROY) { cnc -> roy = cnc -> ROY; cnc -> Y += dy; }
    }

    if (mz[s] > 0) {

      cnc -> roz += (nz[s] != 0 ? 1 : -1);
      
      if      (cnc -> roz < 0)          { cnc -> roz = 0;          cnc -> Z -= dz; }
      else if (cnc -> roz > cnc -> ROZ) { cnc -> roz = cnc -> ROZ; cnc -> Z += dz; }
    }
      
    //.. hold
    if (*stop)     {              return; }
    while (*pause) { if (*stop) { return; } }
  }  
}

bool execute_sequence(TARG * targ, Action ** actions, int A) {

  //.. lock console
  noecho();
  nodelay(stdscr, true);
  refresh();
  
  targ -> pause = false;
  targ -> stop  = false;

  int a;

  for (a = 0 ; a < A ; a++) {

    //.. printing to console
    switch (actions[a] -> type) {

    case Linear:

      printw("> %.2f%% - P: (%.6f, %.6f, %.6f) %s X F%.4f %s/min\n", (double) (a + 1) / (double) A * 100,
                                                                     r2v(actions[a] -> X, targ -> cnc -> unit),
                                                                     r2v(actions[a] -> Y, targ -> cnc -> unit),
                                                                     r2v(actions[a] -> Z, targ -> cnc -> unit),
                                                                     unit_name(targ -> cnc -> unit),
                                                                     r2v(actions[a] -> F, targ -> cnc -> unit),
                                                                     unit_name(targ -> cnc -> unit));
      refresh();
      
      break;

    case Curve:

      printw("> %.2f%% o P0: (%.6f, %.6f) %s P: (%.6f, %.6f, %.6f) %s X F%.4f %s/min\n", (double) (a + 1) / (double) A * 100,
                                                                                         r2v(actions[a] -> X0, targ -> cnc -> unit),
                                                                                         r2v(actions[a] -> Y0, targ -> cnc -> unit),
                                                                                         unit_name(targ -> cnc -> unit),
                                                                                         r2v(actions[a] -> X, targ -> cnc -> unit),
                                                                                         r2v(actions[a] -> Y, targ -> cnc -> unit),
                                                                                         r2v(actions[a] -> Z, targ -> cnc -> unit),
                                                                                         unit_name(targ -> cnc -> unit),
                                                                                         r2v(actions[a] -> F, targ -> cnc -> unit),
                                                                                         unit_name(targ -> cnc -> unit));
      refresh();
      
      break;

    default: break;
    }

    //.. execute action
    targ -> action = actions[a];

    do {

      switch (getch()) {
      case 27:
	targ -> stop = true;
	break;

      case 32:
	targ -> pause = !(targ -> pause);
	if (targ -> pause) { printw("> paused\n");  refresh(); }
	else               { printw("> resumed\n"); refresh(); }
	break;
	
      default: break;
      }

    } while (targ -> action != NULL && !(targ -> stop));
    
    if (targ -> stop) {

      printw("> sequence interrupted\n");
      refresh();

      break;
    }
  }

  //.. cleanup
  for (a++ ; a < A ; a++) { free(actions[a]); }
  free(actions);

  //.. reset console
  echo();
  nodelay(stdscr, false);
  refresh();

  return targ -> stop;
}

void * control_thread(void * args) {

  TARG * targ = (TARG *) args;

  CNC * cnc  = targ -> cnc;
  
  unsigned int * mx, * my, * mz, * nx, * ny, * nz;
  unsigned long long * times;
  struct timespec * steps;

  int S;
  
  mx = my = mz = nx = ny = nz = NULL;
  times = NULL;
  steps = NULL;

  do {

    if (targ -> action != NULL) {

      //.. compiling actions
      switch (targ -> action -> type) {

      case Linear:
	compile_linear(targ -> action, cnc -> X, cnc -> Y, cnc -> Z, cnc -> rox, cnc -> roy, cnc -> roz, cnc -> ROX, cnc -> ROY, cnc -> ROZ, &mx, &my, &mz, &nx, &ny, &nz, &times, &S);
	break;

      case Curve:
	compile_curve(targ -> action, cnc -> X, cnc -> Y, cnc -> Z, cnc -> rox, cnc -> roy, cnc -> roz, cnc -> ROX, cnc -> ROY, cnc -> ROZ, &mx, &my, &mz, &nx, &ny, &nz, &times, &S);
      	break;
	
      default: break;
      }
      
      sort_action(times, mx, my, mz, nx, ny, nz, S);
      splice_action(times, &steps, S);

      //.. execute action
      execute_action(targ -> cnc, steps, mx, my, mz, nx, ny, nz, S, &(targ -> pause), &(targ -> stop));

      //.. updating postion
      write_position(targ -> cnc);
      
      //.. freeing memory
      if (mx != NULL)             { free(mx);             mx = NULL;             }
      if (my != NULL)             { free(my);             my = NULL;             }
      if (mz != NULL)             { free(mz);             mz = NULL;             }
      if (nx != NULL)             { free(nx);             nx = NULL;             }
      if (ny != NULL)             { free(ny);             ny = NULL;             }
      if (nz != NULL)             { free(nz);             nz = NULL;             }
      if (times != NULL)          { free(times);          times = NULL;          }
      if (steps != NULL)          { free(steps);          steps = NULL;          }
      if (targ -> action != NULL) { free(targ -> action); targ -> action = NULL; }
    }
    
  } while (!(targ -> exit));
  
  return NULL;
}
