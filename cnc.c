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

#include "cnc.h"
#include "utils.c"
#include "units.c"
#include "control.c"
#include "cam.c"
#include "cio.c"

int main() {

  //.. initial conditions
  CNC * cnc = malloc(sizeof(struct CNC));

  //.. initializing machine
  initialize(cnc);
  
  //.. command loop
  bool hold = false;
  bool exe  = false;
  bool skip = false;
  
  int l, L, n, v, N, A;

  Action ** s;
  
  char ** lines;
  
  for (;;) {

    //.. recieving line
    char * line;

    if (hold || !exe)      { line = read_line(); }
    else if (exe && n < N) { line = lines[n++];  }
    else if (exe) {

      printf("> program complete\n");

      free(lines);
      lines = NULL;

      exe = false;
      
      continue;
    }

    if (exe && skip) { skip = false; continue; }
    
    //.. parsing line
    char ** segs = split_line(line, ' ', &L);
    
    if (!exe && !hold && parse_exit(segs, L)) {

      free(segs);
      free(line);

      break;

    } else if (!hold && exe && parse_hold(segs, L)) {

      hold = true;
            
    } else if (hold && parse_stop(segs, L)) {

      free(lines);
      lines = NULL;

      exe = false;
      hold = false;
      
    } else if (hold && parse_cont(segs, L)) {

      hold = false;

    } else if (exe && parse_skip(segs, L)) {

      hold = false;
      skip = true;
      
    } else if (parse_get(cnc, segs, L)) {
    } else if (parse_set(cnc, segs, L)) {

      write_config(cnc);
      write_position(cnc);
      write_material(cnc);
      write_tool(cnc);

    }
    else if (exe && parse_comment(cnc, segs, L))                             { }
    else if ((v = parse_spindle(cnc, segs, L)) != -1)                        { gpioWrite(SPND, v);          }
    else if (!hold && (s = parse_goto(cnc, segs, L)) != NULL)                { execute_sequence(cnc, s, 1); }
    else if (!hold && (s = parse_delta(cnc, segs, L)) != NULL)               { execute_sequence(cnc, s, 1); }
    else if (!hold && (s = parse_face(cnc, segs, L, &A)) != NULL)            { execute_sequence(cnc, s, A); }
    else if (!hold && (s = parse_square_pocket(cnc, segs, L, &A)) != NULL)   { execute_sequence(cnc, s, A); }
    else if (!hold && (s = parse_circular_pocket(cnc, segs, L, &A)) != NULL) { execute_sequence(cnc, s, A); }
    else if (!hold && (s = parse_cutout(cnc, segs, L, &A)) != NULL)          { execute_sequence(cnc, s, A); }
    else if (!exe && (lines = parse_exe(cnc, segs, L, &N)) != NULL) {
      
      exe = true;
      n = 0;
      
    } else {
      
      printf("> invalid command or arguments\n");

      if (exe) {
	
	free(lines);
	lines = NULL;

	exe = false;
      }
    }

    free(segs);
    free(line);
  }

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
  gpioWrite(PUL_X, 1);
  gpioWrite(PUL_Y, 0);
  gpioWrite(PUL_Z, 0);
  
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
}

void write_config(CNC * cnc) {

  FILE * file = fopen(CPATH, "w+");
  fprintf(file, "%d\n", cnc -> unit);
  fclose(file);
}

void write_tool(CNC * cnc) {

  FILE * file = fopen(TPATH, "w+");
  fprintf(file, "%d\n%.8f\n%.8f\n%.8f\n", cnc -> tool.type, cnc -> tool.d, cnc -> tool.Lf, cnc -> tool.Lt);
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
