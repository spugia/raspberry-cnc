#include "cio.h"

char * read_line(void) {

  int c;

  size_t lenmax = CMDLEN, len = lenmax;
 
  char * line = malloc(lenmax), * linep = line;

  while (true) {

    c = fgetc(stdin);

    if (c == EOF)  break;

    if (--len == 0) {

      len = lenmax;
      char * linen = realloc(linep, lenmax *= 2);

      if (linen == NULL) {

	free(linep);
	return NULL;
      }

      line = linen + (line - linep);
      linep = linen;
    }

    if ((*line++ = c) == '\n') break;
  }

  *line = '\0';

  return linep;
}

char ** split_line(char * line, char del, int * L) {

  //.. pruning unecessary characters
  int l2 = 0;
  char * line2 = malloc(sizeof(char) * (strlen(line) + 1));

  for (int l = 0 ; l < strlen(line) ; ++l) {

    if ((int) line[l] != 0 && (int) line[l] != 10) {

      line2[l2++] = line[l];
    }
  }
  
  line2[l2++] = 0;

  //.. splitting pruned line
  *L = 0;
  char ** segs = malloc(sizeof(char *));

  int ie = 0;
  int il = 0;

  for (int i = 0 ; i < strlen(line2) ; ++i) {

    ie = (i == (strlen(line2) - 1) && line2[i] != del);
    
    if (line2[i] == del || ie) {

      segs = realloc(segs, sizeof(char *) * ++(*L));
      segs[*L - 1] = malloc(sizeof(char) * (i - il + 1 + ie));
	
      for (int i2 = il ; i2 < i + ie ; ++i2) {

	segs[*L - 1][i2 - il] = line2[i2];
      }

      segs[*L - 1][i - il + ie] = 0;
      
      il = i + 1;
    }
  }

  //.. freeing memory
  free(line2);
  
  return segs;
}

void read_position(CNC * cnc) {

  //.. creating directory
  char * directory = "cnc-state";

  struct stat dir = {0};

  if (stat(DPATH, &dir) == -1) { mkdir(DPATH, 0755); }
  
  cnc -> X = 0;
  cnc -> Y = 0;
  cnc -> Z = 0;

  FILE * file = fopen(PPATH, "ab+");

  int dex = 0;

  char line[CMDLEN];

  while (fgets(line, sizeof(line), file)) {

    switch (dex++) {

    case 0: cnc -> X = atof(line);
    case 1: cnc -> Y = atof(line);
    case 2: cnc -> Z = atof(line);
    default: break;
    }
  }

  printf("------------ POSITION ------------\n");
  printf("X: %.6f\n", cnc -> X);
  printf("Y: %.6f\n", cnc -> Y);
  printf("Z: %.6f\n", cnc -> Z);
  printf("\n");
    
  fclose(file);
}

void read_material(CNC * cnc) {

  cnc -> mat.Fxy = 0;
  cnc -> mat.Fz  = 0;
  cnc -> mat.dz  = 0;
  cnc -> mat.dr  = 0;

  FILE * file = fopen(MPATH, "ab+");

  int dex = 0;

  char line[CMDLEN];

  while (fgets(line, sizeof(line), file)) {

    switch(dex++) {

    case 0: cnc -> mat.Fxy = atof(line);
    case 1: cnc -> mat.Fz  = atof(line);
    case 2: cnc -> mat.dz  = atof(line);
    case 3: cnc -> mat.dr  = atof(line);
    default: break;
    }
  }

  fclose(file);

  printf("------------ MATERIAL ------------\n");
  printf(" Translation Rate: %.6f in/min\n", cnc -> mat.Fxy);
  printf("      Plunge Rate: %.6f in/min\n", cnc -> mat.Fz);
  printf("       Plunge Inc: %.6f in\n", cnc -> mat.dz);
  printf("Translational Inc: %.2f%% of bit\n", cnc -> mat.dr * 100);
  printf("\n");
}

void read_tool(CNC * cnc) {

  cnc -> tool.type = NONE;

  FILE * file = fopen(TPATH, "ab+");

  int dex = 0;

  char line[CMDLEN];

  while (fgets(line, sizeof(line), file)) {

    switch (dex++) {

    case 0: cnc -> tool.type = atoi(line);
    case 1: cnc -> tool.d    = atof(line);
    case 2: cnc -> tool.Lf   = atof(line);
    case 3: cnc -> tool.Lt   = atof(line);
    default: break;
    }
  }

  fclose(file);

  printf("-------------- TOOL --------------\n");
  printf("        Type: %s\n", tooltype_name(cnc -> tool.type));
  printf("    Diameter: %.6f in\n", cnc -> tool.d);
  printf("Flute Length: %.6f in\n", cnc -> tool.Lf);
  printf("Total Length: %.6f in\n", cnc -> tool.Lt);
  printf("\n");
}

char * tooltype_name(enum ToolType type) {

  switch (type) {

  case NONE:    return "empty";
  case ENDMILL: return "endmill";
  default: break;
  }

  return "invalid type";
}

bool parse_exit(char ** segs, int L) {

  if (L == 1 && seq(segs[0], "exit")) { return true; }
  
  return false;
}

bool parse_hold(char ** segs, int L) {

  if (L == 1 && seq(segs[0], "hold")) { printf("> program holding\n"); return true; }

  return false;
}

bool parse_cont(char ** segs, int L) {

  if (L == 1 && seq(segs[0], "continue")) { printf("> program continuing\n"); return true; }

  return false;
}

bool parse_stop(char ** segs, int L) {

  if (L == 1 && seq(segs[0], "stop")) { printf("> program stopped\n"); return true; }

  return false;
}

bool parse_skip(char ** segs, int L) {

  if (L == 1 && seq(segs[0], "skip")) { printf("> skipping command. . .\n"); return true; }

  return false;
}

bool parse_get(CNC * cnc, char ** segs, int L) {
  
  if (L == 2 && seq(segs[0], "get")) {
    
    if (seq(segs[1], "X"))           { printf("> X = %.6f in\n", cnc -> X);                                        return true; }
    if (seq(segs[1], "Y"))           { printf("> Y = %.6f in\n", cnc -> Y);                                        return true; }
    if (seq(segs[1], "Z"))           { printf("> Z = %.6f in\n", cnc -> Z);                                        return true; }
    if (seq(segs[1], "P"))           { printf("> P = (%.6f, %.6f, %.6f) in\n", cnc -> X, cnc -> Y, cnc -> Z);      return true; }
    if (seq(segs[1], "ToolType"))    { printf("> Tool Type = %s\n", tooltype_name(cnc -> tool.type));              return true; }
    if (seq(segs[1], "ToolSize"))    { printf("> Tool Size = %.6f in\n", cnc -> tool.d);                           return true; }
    if (seq(segs[1], "FluteLength")) { printf("> Flute Length = %.6f in\n", cnc -> tool.Lf);                       return true; }
    if (seq(segs[1], "ToolLength"))  { printf("> Tool Length = %.6f in\n", cnc -> tool.Lt);                        return true; }
    if (seq(segs[1], "FeedRate"))    { printf("> Feed Rate = %.4f in/min\n", cnc -> mat.Fxy);                      return true; }
    if (seq(segs[1], "PlungeRate"))  { printf("> Plunge Rate = %.4f in/min\n", cnc -> mat.Fz);                     return true; }
    if (seq(segs[1], "PlungeInc"))   { printf("> Plunge Increment = %.6f in\n", cnc -> mat.dz);                    return true; }
    if (seq(segs[1], "TransInc"))    { printf("> Translational Increment = %.2f%% of bit\n", cnc -> mat.dr * 100); return true; }
  }
  return false;
}

bool parse_set(CNC * cnc, char ** segs, int L) {

  double v1, v2, v3;
  
  if (seq(segs[0], "set")) {
    
    if (seq(segs[1], "X") && L == 3 && parse_lf(segs[2], &v1)) {

      cnc -> X = v1;
      printf("> X = %.6f in\n", cnc -> X);
    
      return true;
    }

    if (seq(segs[1], "Y") && L == 3 && parse_lf(segs[2], &v1)) {

      cnc -> Y = v1;
      printf("> Y = %.6f in\n", cnc -> Y);
    
      return true;
    }
    
    if (seq(segs[1], "Z") && L == 3 && parse_lf(segs[2], &v1)) {

      cnc -> Z = v1;
      printf("> Z = %.6f in\n", cnc -> Z);
    
      return true;
    }

    if (seq(segs[1], "P") && L == 5 && parse_lf(segs[2], &v1) && parse_lf(segs[3], &v2) && parse_lf(segs[4], &v3)) {

      cnc -> X = v1;
      cnc -> Y = v2;
      cnc -> Z = v3;

      printf("> P = (%.6f %.6f %.6f) in\n", cnc -> X , cnc -> Y, cnc -> Z);

      return true;
    }
    
    if (seq(segs[1], "ToolType") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 < 0) { return false; }
      
      cnc -> tool.type = (int) v1;
      printf("> Tool Type = %s\n", tooltype_name(cnc -> tool.type));

      return true;
    }
    
    if (seq(segs[1], "ToolSize") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> tool.d = v1;
      printf("> Tool Size = %.6f in\n", cnc -> tool.d);

      return true;
    }
    
    if (seq(segs[1], "FluteLength") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> tool.Lf = v1;
      printf("> Flute Length = %.6f in\n", cnc -> tool.Lf);

      return true;
    }
    
    if (seq(segs[1], "ToolLength") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> tool.Lt = v1;
      printf("> Tool Length = %.6f in\n", cnc -> tool.Lt);

      return true;
    }

    if (seq(segs[1], "FeedRate") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0 || v1 > FXY_max()) { return false; }
      
      cnc -> mat.Fxy = v1;
      printf("> Feed Rate = %.4f in/min\n", cnc -> mat.Fxy);

      return true;
    }

    if (seq(segs[1], "PlungeRate") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0 || v1 > FZ_max()) { return false; }
      
      cnc -> mat.Fz = v1;
      printf("> Plunge Rate = %.4f in/min\n", cnc -> mat.Fz);

      return true;
    }

    if (seq(segs[1], "PlungeInc") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> mat.dz = v1;
      printf("> Plunge Increment = %.6f in\n", cnc -> mat.dz);

      return true;
    }

    if (seq(segs[1], "TransInc") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> mat.dr = v1;
      printf("> Translational Increment = %.2f%% of bit\n", cnc -> mat.dr * 100);

      return true;
    }
  }
  
  return false;
}

bool parse_comment(CNC * cnc, char ** segs, int L) {

  if (L < 2 || !seq(segs[0], "#")) { return false; }

  printf(">");

  for (int l = 1 ; l < L ; ++l) {

    printf(" %s", segs[l]);
  }

  printf("\n");
  
  return true;
}

char ** parse_exe(CNC * cnc, char ** segs, int L, int * A) {
  
  if (L != 2 || !seq(segs[0], "exe")) { return NULL; }

  //.. opening file
  FILE * file = fopen(segs[1], "r");
  
  if (file == NULL) { return NULL; }
  
  //.. reading lines
  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  rewind(file);

  char * line = malloc(fsize + 1);
  fread(line, fsize, 1, file);
  line[fsize] = 0;

  fclose(file);

  return split_line(line, ';', A);
}

Action ** parse_goto(CNC * cnc, char ** segs, int L) {

  if (!seq(segs[0], "goto")) { return NULL; }
  
  double X, Y, Z, F;
  
  if (!parse_lf(segs[1], &X) || !parse_lf(segs[2], &Y) || !parse_lf(segs[3], &Z)) { return false; }
  
  if (L == 5 && !parse_lf(segs[4], &F)) { return false; }
  else if (L == 4) { F = F_max(fabs(X - cnc -> X), fabs(Y - cnc -> Y), fabs(Z - cnc -> Z)); }
  
  Action ** s = malloc(sizeof(Action *));
  s[0] = create_linear(X, Y, Z, F);
  
  return s;
}

Action ** parse_delta(CNC * cnc, char ** segs, int L) {

  if (!seq(segs[0], "delta")) { return NULL; }

  double dx, dy, dz, F;

  if (!parse_lf(segs[1], &dx) || !parse_lf(segs[2], &dy) || !parse_lf(segs[3], &dz)) { return false; }

  if (L == 5 && !parse_lf(segs[4], &F)) { return false;          }
  else if (L == 4)                      { F = F_max(dx, dy, dz); }

  Action ** s = malloc(sizeof(Action *));
  s[0] = create_linear(cnc -> X + dx, cnc -> Y + dy, cnc -> Z + dz, F);

  return s;
}

Action ** parse_face(CNC * cnc, char ** segs, int L, int * A) {

  if (L != 7 || !seq(segs[0], "face")) { return NULL; }

  double x0, y0, z0, lx, ly, h;

  if (!parse_lf(segs[1], &x0) ||
      !parse_lf(segs[2], &y0) ||
      !parse_lf(segs[3], &z0) ||
      !parse_lf(segs[4], &lx) ||
      !parse_lf(segs[5], &ly) ||
      !parse_lf(segs[6], &h)) { return NULL; }

  if (lx < 0 || ly < 0 || h < 0)                { return NULL; }
  if (lx < cnc -> tool.d || ly < cnc -> tool.d) { return NULL; } 
  
  return face(cnc -> mat.Fxy, cnc -> mat.Fz, cnc -> tool.d, x0, y0, z0, lx, ly, cnc -> mat.dr, h, cnc -> mat.dz, A);
}

Action ** parse_square_pocket(CNC * cnc, char ** segs, int L, int * A) {

  if (L != 7 || !seq(segs[0], "square_pocket")) { return NULL; }

  double x0, y0, z0, lx, ly, h;

  if (!parse_lf(segs[1], &x0) ||
      !parse_lf(segs[2], &y0) ||
      !parse_lf(segs[3], &z0) ||
      !parse_lf(segs[4], &lx) ||
      !parse_lf(segs[5], &ly) ||
      !parse_lf(segs[6], &h)) { return NULL; }

  if (lx < 0 || ly < 0 || h < 0)                { return NULL; }
  if (lx < cnc -> tool.d || ly < cnc -> tool.d) { return NULL; } 
    
  return square_pocket(cnc -> mat.Fxy, cnc -> mat.Fz, cnc -> tool.d, x0, y0, z0, lx, ly, cnc -> mat.dr, h, cnc -> mat.dz, A);    
}

Action ** parse_circular_pocket(CNC * cnc, char ** segs, int L, int * A) {

  if (L != 7 || !seq(segs[0], "circular_pocket")) { return NULL; }

  double x0, y0, z0, ri, ro, h;

  if (!parse_lf(segs[1], &x0) ||
      !parse_lf(segs[2], &y0) ||
      !parse_lf(segs[3], &z0) ||
      !parse_lf(segs[4], &ri) ||
      !parse_lf(segs[5], &ro) ||
      !parse_lf(segs[6], &h)) { return NULL; }

  if (ri < 0 || ro <= 0)         { return NULL; }
  if ((ro - ri) < cnc -> tool.d) { return NULL; }

  if (lfeq(ri, 0)) { ri = -cnc -> tool.d / 2; }
  
  return circular_pocket(cnc -> mat.Fxy, cnc -> mat.Fz, cnc -> tool.d, x0, y0, z0, ri, ro, cnc -> mat.dr, h, cnc -> mat.dz, A);
}
