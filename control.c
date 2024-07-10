#include "control.h"

double FX_max(void) { return 1000000000 / (SPR_X * RPI_X * SPS_23) * 60; }

double FY_max(void) { return 1000000000 / (SPR_Y * RPI_Y * SPS_23) * 60; }

double FZ_max(void) { return 1000000000 / (SPR_Z * RPI_Z * SPS_23) * 60; }

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

  double F = A * l + FMIN;

  if (l / L > 0.5) { F = A * (L - l) + FMIN; }

  if (F > Fmax) { F = Fmax; }

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
  
  //.. compiling x-axis steps
  (*S) = Sx;

  double x = 0;
  double y = 0;
  double z = 0;
  double ll = 0;
  double l = 0;
  double L = sqrt(dx * dx + dy * dy + dz * dz);
  double F;
  
  bool duplicate;
  unsigned long long t = 0;
  
  for (unsigned long long s = 0 ; s < Sx ; ++s) {

    x += dxs;
    y = x * fabs(dy / dx);
    z = x * fabs(dz / dx);

    l = sqrt(x * x + y * y + z * z);
    F = F_accel(l, L, FAR, action -> F);

    t += (unsigned long long) round((l - ll) / F * 60 * 1000000000);
    
    (*times)[s] = t;
    (*mx)[s]    = PUL_X;
    (*nx)[s]    = dx > 0;

    ll = l;
  }

  //.. compiling y-axis steps
  t  = 0;
  y  = 0;
  ll = 0;
  
  for (unsigned long long s = 0 ; s < Sy ; ++s) {

    y += dys;
    x = y * fabs(dx / dy);
    z = y * fabs(dz / dy);

    l = sqrt(x * x + y * y + z * z);
    F = F_accel(l, L, FAR, action -> F);

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
  t  = 0;
  z  = 0;
  ll = 0;

  for (unsigned long long s = 0 ; s < Sz ; ++s) {

    z += dzs;
    x = z * fabs(dx / dz);
    y = z * fabs(dy / dz);

    l = sqrt(x * x + y * y + z * z);
    F = F_accel(l, L, FAR, action -> F);

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

void compile_curve(Action * action, double X, double Y, double Z, unsigned int ** mx, unsigned int ** my, unsigned int ** mz, unsigned int ** nx, unsigned int ** ny, unsigned int ** nz, unsigned long long ** times, int * S) {

  //.. angles
  double T1 = atan2(Y - action -> Y0, X - action -> X0) + 2 * M_PI * ((Y - action -> Y0) < 0);
  double T2 = atan2(action -> Y - action -> Y0, action -> X - action -> X0) + 2 * M_PI * ((action -> Y - action -> Y0) < 0);

  if (T1 > M_PI && lfeq(T2, 0)) { T2 += 2 * M_PI; }
  if (T2 > M_PI && lfeq(T1, 0)) { T1 += 2 * M_PI; }
  
  if (lfeq(T1, T2) || lfeq(fabs(T1 - T2), 2 * M_PI)) { T2 += 2 * M_PI; } //.. complete circle
  
  //.. radius
  double R = sqrt(pow(X - action -> X0, 2) + pow(Y - action -> Y0, 2));

  //.. steps  
  double dxs = 1 / (RPI_X * SPR_X);
  double dys = 1 / (RPI_Y * SPR_Y);
  double dzs = 1 / (RPI_Z * SPR_Z);
  
  //.. allocating data structures
  unsigned long long zsteps = fabs(Z - action -> Z) / dzs;
  
  (*S) = round(5 * R / dxs) + round(5 * R / dys) + zsteps;

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
  
  if (lfeq(R, 0)) { return; }
  
  //.. compiling x-y steps  
  double dmin = dxs;
  if (dys < dmin) { dmin = dys; }

  double dT = dmin / R / 100;
  dT = (T2 - T1) / round(fabs(T2 - T1) / dT);

  int ix, iy;

  int ixl = (int) round(R * cos(T1) / dxs);
  int iyl = (int) round(R * sin(T1) / dys);

  double dl;
  double F;
  double l;
  double L = fabs(T2 - T1) * R;
  double Tc;

  unsigned long long time = 0;
  
  for (double T = T1 ; dT > 0 && T <= T2 || dT < 0 && T >= T2 ; T += dT) {

    ix = (int) round(R * cos(T) / dxs);
    iy = (int) round(R * sin(T) / dys);
    
    if (ix != ixl || iy != iyl) {
 
      dl = sqrt(pow((double) (ix - ixl) * dxs, 2) + pow((double) (iy - iyl) * dys, 2));
      l  = fabs(Tc - T1) * R;
      
      Tc = atan2((double) iy * dys, (double) ix * dxs) + 2 * M_PI * (((double) iy * dys) < 0);
     
      F = F_accel(l, L, FAR, action -> F); 
      
      time += (unsigned long long) round(dl / F * 60 * 1000000000);

      (*times)[(*S)] = time;
      (*mx)[(*S)]    = PUL_X * (ix != ixl);
      (*my)[(*S)]    = PUL_Y * (iy != iyl);
      
      switch(quadrant(Tc)) {

      case 1:
	(*nx)[(*S)] = dT > 0;
	(*ny)[(*S)] = dT < 0;
	break;
      case 2:
	(*nx)[(*S)] = dT > 0;
	(*ny)[(*S)] = dT > 0;
	break;
      case 3:
	(*nx)[(*S)] = dT < 0;
	(*ny)[(*S)] = dT > 0;
	break;
      case 4:
	(*nx)[(*S)] = dT < 0;
	(*ny)[(*S)] = dT < 0;
	break;
      default: break;
      }
      
      (*S)++;
	
      ixl = ix;
      iyl = iy;
    }
  }

  //.. compiling z-steps
  unsigned long long s2;

  unsigned long long dt = 0;
  if (zsteps > 0) { dt = time / zsteps; }
  
  time = 0;

  bool duplicate;
  
  for (unsigned long long s = 1 ; s <= zsteps ; s++) {

    time += dt;
    
    duplicate = false;

    for (int s2 = 0 ; s2 < *S ; ++s2) {

      if ((*times)[s2] == time) {

        (*mz)[s2] = PUL_Z;
        (*nz)[s2] = (Z - action -> Z) < 0;
        
        duplicate = true;
        break;
      }
    }

    if (!duplicate) {

      (*times)[*S] = time;
      (*mz)[*S]    = PUL_Z;
      (*nz)[*S]    = (Z - action -> Z) < 0;
      
      (*S)++;
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
    cnc -> X += (double) (mx[s] > 0) * dx * (nx[s] ? -1 :  1);
    cnc -> Y += (double) (my[s] > 0) * dy * (ny[s] ? -1 :  1);
    cnc -> Z += (double) (mz[s] > 0) * dz * (nz[s] ?  1 : -1);
    
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
	compile_linear(targ -> action, cnc -> X, cnc -> Y, cnc -> Z, &mx, &my, &mz, &nx, &ny, &nz, &times, &S);
	break;

      case Curve:
	compile_curve(targ -> action, cnc -> X, cnc -> Y, cnc -> Z, &mx, &my, &mz, &nx, &ny, &nz, &times, &S);
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
