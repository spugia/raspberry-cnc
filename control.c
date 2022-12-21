#include "control.h"

double FX_max(void) { return 1000000000 / (SPR_X * RPI_X * 2 * SPS_23) * 60; }

double FY_max(void) { return 1000000000 / (SPR_Y * RPI_Y * 2 * SPS_23) * 60; }

double FZ_max(void) { return 1000000000 / (SPR_Z * RPI_Z * 2 * SPS_23) * 60; }

double FXY_max(void) {

  double FX = FX_max();
  double FY = FY_max();

  return (FX < FY) * FX + (FX >= FY) * FY;
}

double FXYZ_max(void) {

  double FXY = FXY_max();
  double FZ  = FZ_max();

  return (FXY < FZ) * FXY + (FXY >= FZ) * FZ;
}

double F_max(double dx, double dy, double dz) {

  if (dx != 0 && dy == 0 && dz == 0) { return FX_max();  }
  if (dx == 0 && dy != 0 && dz == 0) { return FY_max();  }
  if (dx == 0 && dy == 0 && dz != 0) { return FZ_max();  }
  if (dx != 0 && dy != 0 && dz == 0) { return FXY_max(); }

  return FXYZ_max();  
}

double F_accel(double l, double L, double A, double Fmax) {

  double F = A * l;

  if (F > Fmax) {

    F = (L - l) * A;

    if (F < Fmax) { return F; }
    
    return Fmax;
  }

  return F;
}

Action * create_linear(double X, double Y, double Z, double F) {

  Action * linear = malloc(sizeof(struct Action));

  linear -> type = Linear;

  linear -> X = X;
  linear -> Y = Y;
  linear -> Z = Z;

  linear -> X0 = 0;
  linear -> Y0 = 0;
  
  linear -> F = F;

  return linear;
}

Action * create_curve(double X, double Y, double Z, double X0, double Y0, double F) {

  Action * curve = malloc(sizeof(struct Action));

  curve -> type = Curve;

  curve -> X = X;
  curve -> Y = Y;
  curve -> Z = Z;

  curve -> X0 = X0;
  curve -> Y0 = Y0;
  
  curve -> F = F;

  return curve;
}

void compile_linear(Action * action, double X, double Y, double Z, unsigned int ** mx, unsigned int ** my, unsigned int ** mz, unsigned int ** nx, unsigned int ** ny, unsigned int ** nz, unsigned long long ** times, int * S) {

  //.. distance along each axis
  double dx = 0;
  double dy = 0;
  double dz = 0;

  if (!isnan(action -> X)) { dx = X - action -> X; }
  if (!isnan(action -> Y)) { dy = Y - action -> Y; }
  if (!isnan(action -> Z)) { dz = Z - action -> Z; }

  //.. steps along each axis
  int Sx = (int) round(fabs(dx) * RPI_X * SPR_X);
  int Sy = (int) round(fabs(dy) * RPI_Y * SPR_Y);
  int Sz = (int) round(fabs(dz) * RPI_Z * SPR_Z);

  double dxs = 1 / (RPI_X * SPR_X);
  double dys = 1 / (RPI_Y * SPR_Y);
  double dzs = 1 / (RPI_Z * SPR_Z);
  
  //.. allocating memory
  (*S) = Sx + Sy + Sz;
  
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
  
  //.. feed rates
  double L  = sqrt(dx * dx  + dy * dy + dz * dz);
  
  double Fx = action -> F * fabs(dx) / L;
  double Fy = action -> F * fabs(dy) / L;
  double Fz = action -> F * fabs(dz) / L;
  
  //.. compiling steps
  double l = 0;
  double t = 0;
  double F;
  
  for (double s = 1 ; s <= Sx ; ++s) {

    l += dxs;
    t += dxs / F * 60;
        
    F = F_accel(l, fabs(dx), FAR_X, Fx);

    (*times)[*S] = (unsigned long long) round(t * 1000000000);
    (*mx)[*S]    = PUL_X;
    (*nx)[*S]    = dx > 0;

    (*S)++;
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

void compile_curve(Action * action, double X, double Y, unsigned int ** mx, unsigned int ** my, unsigned int ** mz, unsigned int ** nx, unsigned int ** ny, unsigned int ** nz, unsigned long long ** times, int * S) {

  //.. angles
  double T1 = atan2(Y - action -> Y0, X - action -> X0) + 2 * M_PI * ((Y - action -> Y0) < 0);
  double T2 = atan2(action -> Y - action -> Y0, action -> X - action -> X0) + 2 * M_PI * ((action -> Y - action -> Y0) < 0);

  T2 += (T1 >= T2) * 2 * M_PI;
  
  //.. radius
  double dxs = 1 / (RPI_X * SPR_X);
  double dys = 1 / (RPI_Y * SPR_Y);

  double dmin = dxs;
  if (dys < dmin) { dmin = dys; }
  
  double R = sqrt(pow(X - action -> X0, 2) + pow(Y - action -> Y0, 2));

  //.. allocating data structures  
  (*S) = round(5 * R / dxs) + round(5 * R / dys);

  *times  = malloc(sizeof(unsigned long long) * (*S));
  *mx     = malloc(sizeof(unsigned int)       * (*S));
  *my     = malloc(sizeof(unsigned int)       * (*S));
  *mz     = malloc(sizeof(unsigned int)       * (*S));
  *nx     = malloc(sizeof(unsigned int)       * (*S));
  *ny     = malloc(sizeof(unsigned int)       * (*S));
  *nz     = malloc(sizeof(unsigned int)       * (*S));
  
  *mx = (unsigned int *) memset(*mx, 0, sizeof(unsigned int) * (*S));
  *my = (unsigned int *) memset(*my, 0, sizeof(unsigned int) * (*S));
  *mz = (unsigned int *) memset(*mz, 0, sizeof(unsigned int) * (*S));
  *nx = (unsigned int *) memset(*nx, 0, sizeof(unsigned int) * (*S));
  *ny = (unsigned int *) memset(*ny, 0, sizeof(unsigned int) * (*S));
  *nz = (unsigned int *) memset(*nz, 0, sizeof(unsigned int) * (*S));

  (*S) = 0;

  if (lfeq(R, 0)) { return; }
  
  //.. compiling steps
  bool incx, incy;
  
  int xl = 0;
  int yl = 0;
  int x, y;

  double Nx = 1;
  double Ny = 1;
  double Tx = T1;
  double Ty = T1;
  double T  = T1;
  double dT = dmin / R / 25;

  while ((T += dT) <= T2) {

    incx = false;
    incy = false;
    
    x = round((R * cos(T) + action -> X0 - X) / dxs);
    y = round((R * sin(T) + action -> Y0 - Y) / dys);

    incx = (x != xl);
    incy = (y != yl);

    if (!incx) { Tx += T; Nx++; }
    else {

      (*times)[(*S)] = (unsigned long long) round(fabs(Tx / Nx - T1) * R / action -> F * 60 * 1000000000);
      (*mx)[(*S)]    = PUL_X;
      (*nx)[(*S)]    = x < xl;

      (*S)++;
      
      Tx = T;
      Nx = 1;
      xl = x;
    }
    
    if (!incy) { Ty += T; Ny++; }
    else {

      if (!incx) { (*times)[(*S)++] = (unsigned long long) round(fabs(Ty / Ny - T1) * R / action -> F * 60 * 1000000000); }

      (*my)[(*S) - 1] = PUL_Y;
      (*ny)[(*S) - 1] = y < yl;

      Ty = T;
      Ny = 1;
      yl = y;
    }
  }
  
  //.. resizing data structures
  *times = (unsigned long long *) realloc(*times, sizeof(unsigned long long) * (*S));
  *mx    =       (unsigned int *) realloc(*mx,    sizeof(unsigned int)       * (*S));
  *my    =       (unsigned int *) realloc(*my,    sizeof(unsigned int)       * (*S));
  *mz    =       (unsigned int *) realloc(*mz,    sizeof(unsigned int)       * (*S));  
  *nx    =       (unsigned int *) realloc(*nx,    sizeof(unsigned int)       * (*S));
  *ny    =       (unsigned int *) realloc(*ny,    sizeof(unsigned int)       * (*S));
  *nz    =       (unsigned int *) realloc(*nz,    sizeof(unsigned int)       * (*S));    
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
    
    delay = times[s] - last - SPS_23;
    last  = times[s];

    (*steps)[s].tv_sec  = (int) ((double) delay / (double) 1000000000);
    (*steps)[s].tv_nsec = delay;
  }
}

void execute_action(struct timespec * steps, unsigned int * mx, unsigned int * my, unsigned int * mz, unsigned int * nx, unsigned int * ny, unsigned int * nz, int S) {

  struct timespec delay;

  delay.tv_sec  = 0;
  delay.tv_nsec = SPS_23;
  
  for (int s = 0 ; s < S ; ++s) {

    //.. setting direction
    if (s == 0 || nx[s] != nx[s-1]) { gpioWrite(DIR_X, nx[s]); }
    if (s == 0 || ny[s] != ny[s-1]) { gpioWrite(DIR_Y, ny[s]); }
    if (s == 0 || nz[s] != nz[s-1]) { gpioWrite(DIR_Z, nz[s]); }
    
    //.. pulsing
    if (mx[s] > 0) { gpioWrite(mx[s], 1); }
    if (my[s] > 0) { gpioWrite(my[s], 1); }
    if (mz[s] > 0) { gpioWrite(mz[s], 1); }
    nanosleep(&delay, NULL);

    if (mx[s] > 0) { gpioWrite(mx[s], 0); }
    if (my[s] > 0) { gpioWrite(my[s], 0); }
    if (mz[s] > 0) { gpioWrite(mz[s], 0); }
    nanosleep(&(steps[s]), NULL);
  }  
}

void execute_sequence(CNC * cnc, Action ** actions, int A) {

  int S;

  unsigned int       * mx;
  unsigned int       * my;
  unsigned int       * mz;
  unsigned int       * nx;
  unsigned int       * ny;
  unsigned int       * nz;
  unsigned long long * times;
  struct timespec    * steps;
  
  for (int a = 0 ; a < A ; ++a) {

    mx     = NULL;
    my     = NULL;
    mz     = NULL;
    nx     = NULL;
    ny     = NULL;
    nz     = NULL;
    steps  = NULL;
    times  = NULL;

    //.. executing action
    switch (actions[a] -> type) {

    case Linear:
      
      compile_linear(actions[a], cnc -> X, cnc -> Y, cnc -> Z, &mx, &my, &mz, &nx, &ny, &nz, &times, &S);
      printf("> %.2f%% - P: (%.6f, %.6f, %.6f) in X F%.4f in/min\n", (double) (a + 1) / (double) A * 100, actions[a] -> X, actions[a] -> Y, actions[a] -> Z, actions[a] -> F);

      break;

    case Curve:

      compile_curve(actions[a], cnc -> X, cnc -> Y, &mx, &my, &mz, &nx, &ny, &nz, &times, &S);
      printf("> %.2f%% o P0: (%.6f, %.6f) in P: (%.6f, %.6f) in X F%.4f in/min\n", (double) (a + 1) / (double) A * 100, actions[a] -> X0, actions[a] -> Y0, actions[a] -> X, actions[a] -> Y, actions[a] -> F);

      break;

    default: break;
    }

    sort_action(times, mx, my, mz, nx, ny, nz, S);
    splice_action(times, &steps, S);
    execute_action(steps, mx, my, mz, nx, ny, nz, S);

    //.. updating postion
    if (!isnan(actions[a] -> X)) { cnc -> X = actions[a] -> X; }
    if (!isnan(actions[a] -> Y)) { cnc -> Y = actions[a] -> Y; }
    if (!isnan(actions[a] -> Z)) { cnc -> Z = actions[a] -> Z; }

    write_position(cnc);
    
    //.. freeing memory
    if (mx != NULL)         { free(mx);         mx = NULL;         }
    if (my != NULL)         { free(my);         my = NULL;         }
    if (mz != NULL)         { free(mz);         mz = NULL;         }
    if (nx != NULL)         { free(nx);         nx = NULL;         }
    if (ny != NULL)         { free(ny);         ny = NULL;         }
    if (nz != NULL)         { free(nz);         nz = NULL;         }
    if (times != NULL)      { free(times);      times = NULL;      }
    if (steps != NULL)      { free(steps);      steps = NULL;      }
    if (actions[a] != NULL) { free(actions[a]); actions[a] = NULL; }
  }

  //.. cleanup
  free(actions);
}
