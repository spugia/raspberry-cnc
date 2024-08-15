#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <pigpio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <ncurses.h>

#include "../include/cnc.h"

#include "utils.c"
#include "units.c"
#include "control.c"
#include "cam.c"
#include "cio.c"

int main() {
  
  //.. initialize console
  initscr();
  echo();
  scrollok(stdscr, true);
  keypad(stdscr, true);

  refresh();
  
  //.. initial conditions
  CNC * cnc = malloc(sizeof(struct CNC));

  //.. initializing machine
  initialize(cnc);

  //.. initialize driver thread
  pthread_t thread;

  TARG targ;
  targ.pause  = false;
  targ.stop   = false;
  targ.exit   = false;
  targ.action = NULL;
  targ.cnc    = cnc;
  
  pthread_create(&thread, NULL, control_thread, (void *) (&targ));
  
  //.. command loop
  bool hold = false;
  bool exe  = false;
  bool skip = false;
  
  int l, L, n, dn, v, N, A, T;

  Action ** s;

  char * unitcmd;
    
  char ** lines;
  char ** tlines;
  
  for (;;) {
    
    //.. recieving line
    char * line;

    if (hold || !exe)      { line = read_line(); }
    else if (exe && n < N) { line = lines[n++];  }
    else if (exe) {

      printw("> program complete\n");
      refresh();
      
      free(lines);
      lines = NULL;

      exe = false;
      
      continue;
    }

    if (exe && skip) { skip = false; continue; }
    
    //.. parsing line
    char ** segs = split_line(line, ' ', &L);
    
    if (!exe && !hold && parse_exit(segs, L)) {

      //.. exit
      free(segs);
      free(line);
      
      break;

    } else if (!hold && exe && parse_hold(segs, L)) {

      //.. hold
      hold = true;
            
    } else if (hold && parse_stop(segs, L)) {

      //.. stop
      free(lines);
      lines = NULL;

      exe = false;
      hold = false;
      
    } else if (hold && parse_cont(segs, L)) {

      //.. continue
      hold = false;

    } else if (exe && parse_skip(segs, L)) {

      //.. skip
      hold = false;
      skip = true;

    } else if (exe && (tlines = parse_toolchange(cnc, segs, L, &T)) != NULL) {
      
      //.. tool change
      unitcmd = malloc(sizeof(char) * 256);
      sprintf(unitcmd, "set unittype %s", unittype_name(cnc -> unit));

      lines = realloc(lines, sizeof(char *) * (N + T + 1));

      //.. shift commands
      for (dn = N - 1 ; dn >= n ; dn--) {

	lines[dn + T + 1] = lines[dn];
      }

      //.. insert tool change
      for (dn = 0 ; dn < T ; dn++) {

        lines[n + dn] = tlines[dn];
      }

      //.. reset units
      lines[n + T] = unitcmd;

      N += (T + 1);

      free(tlines);
      tlines = NULL;
      
    } else if (parse_get(cnc, segs, L)) {
    } else if (parse_set(cnc, segs, L)) {

      //.. set
      write_config(cnc);
      write_position(cnc);
      write_material(cnc);
      write_tool(cnc);
    }
    else if (exe && parse_comment(cnc, segs, L))                             { }
    else if ((v = parse_spindle(cnc, segs, L)) != -1)                        { gpioWrite(SPND, v); }
    else if ((s = parse_goto(cnc, segs, L)) != NULL)                         { if (execute_sequence(&targ, s, 1) && exe) { hold = true; } }
    else if ((s = parse_delta(cnc, segs, L)) != NULL)                        { if (execute_sequence(&targ, s, 1) && exe) { hold = true; } }
    else if (!hold && (s = parse_face(cnc, segs, L, &A)) != NULL)            { if (execute_sequence(&targ, s, A) && exe) { hold = true; } }
    else if (!hold && (s = parse_square_pocket(cnc, segs, L, &A)) != NULL)   { if (execute_sequence(&targ, s, A) && exe) { hold = true; } }
    else if (!hold && (s = parse_circular_pocket(cnc, segs, L, &A)) != NULL) { if (execute_sequence(&targ, s, A) && exe) { hold = true; } }
    else if (!hold && (s = parse_cutout(cnc, segs, L, &A)) != NULL)          { if (execute_sequence(&targ, s, A) && exe) { hold = true; } }
    else if (!hold && (s = parse_drill(cnc, segs, L, &A)) != NULL)           { if (execute_sequence(&targ, s, A) && exe) { hold = true; } }
    else if (!hold && (s = parse_bore(cnc, segs, L, &A)) != NULL)            { if (execute_sequence(&targ, s, A) && exe) { hold = true; } }
    else if (!hold && (s = parse_fillet(cnc, segs, L, &A)) != NULL)          { if (execute_sequence(&targ, s, A) && exe) { hold = true; } }
    else if (!hold && (s = parse_groove(cnc, segs, L, &A)) != NULL)          { if (execute_sequence(&targ, s, A) && exe) { hold = true; } }
    else if (!hold && (s = parse_engrave(cnc, segs, L, &A)) != NULL)         { if (execute_sequence(&targ, s, A) && exe) { hold = true; } }
    else if (!exe && (lines = parse_exe(cnc, segs, L, &N)) != NULL) {
      
      exe = true;
      n = 0;
      
    } else {

      //.. invalid
      printw("> invalid command or arguments\n");
      refresh();
      
      if (exe && !hold) {
	
	free(lines);
	lines = NULL;

	exe = false;
      }
    }

    free(segs);
    free(line);
  }

  //.. stop thread
  targ.pause = false;
  targ.stop  = true;
  targ.exit  = true;

  pthread_join(thread, NULL);
  
  //.. cleanup
  cleanup(cnc);
  
  return 0;
}

void initialize(CNC * cnc) {
  
  //.. setting up GPIO
  gpioInitialise();

  //.. setting pin modes
  gpioSetMode(PUL_X, PI_OUTPUT);
  gpioSetMode(PUL_Y, PI_OUTPUT);
  gpioSetMode(PUL_Z, PI_OUTPUT);
  
  gpioSetMode(DIR_X, PI_OUTPUT);
  gpioSetMode(DIR_Y, PI_OUTPUT);
  gpioSetMode(DIR_Z, PI_OUTPUT);

  gpioSetMode(STP_X, PI_INPUT);
  gpioSetMode(STP_Y, PI_INPUT);
  gpioSetMode(STP_Z, PI_INPUT);
  
  gpioSetMode(SPND, PI_OUTPUT);
  
  //.. setting pin states
  gpioWrite(DIR_X, 0);
  gpioWrite(DIR_Y, 0);
  gpioWrite(DIR_Z, 0);

  gpioWrite(SPND, 1);
  
  //.. loading machine state
    read_config(cnc);
  read_position(cnc);
  read_material(cnc);
      read_tool(cnc);

    write_config(cnc);
  write_position(cnc);
  write_material(cnc);
      write_tool(cnc);      
}

void cleanup(CNC * cnc) {
  
  //.. turning off motors
  gpioWrite(PUL_X, 0);
  gpioWrite(PUL_Y, 0);
  gpioWrite(PUL_Z, 0);

  gpioWrite(DIR_X, 0);
  gpioWrite(DIR_Y, 0);
  gpioWrite(DIR_Z, 0);

  gpioWrite(SPND, 1);

  //.. freeing allocated memory
  if (cnc != NULL) { free(cnc); }

  //.. restore console
  endwin();
}

void write_config(CNC * cnc) {

  FILE * file = fopen(CPATH, "w+");
  fprintf(file, "%d\n", cnc -> unit);
  fclose(file);
}

void write_tool(CNC * cnc) {

  FILE * file = fopen(TPATH, "w+");
  fprintf(file, "%d\n%.8f\n%.8f\n%.8f\n%.8f\n", cnc -> tool.type, cnc -> tool.Dc, cnc -> tool.Ds, cnc -> tool.Lc, cnc -> tool.Lt);
  fclose(file);
}

void write_material(CNC * cnc) {

  FILE * file = fopen(MPATH, "w+");
  fprintf(file, "%.8f\n%.8f\n%.8f\n%.8f\n", cnc -> mat.Fxy, cnc -> mat.Fz, cnc -> mat.dz, cnc -> mat.dr);
  fclose(file);
}

void write_position(CNC * cnc) {

  FILE * file = fopen(PPATH, "w+");
  fprintf(file, "%.8f\n%.8f\n%.8f\n", cnc -> X, cnc -> Y, cnc -> Z);
  fclose(file);
}
