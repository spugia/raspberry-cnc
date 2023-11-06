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

void read_config(CNC * cnc) {

  cnc -> unit = IN;

  FILE * file = fopen(CPATH, "ab+");

  int dex = 0;
  
  char line[CMDLEN];

  while (fgets(line, sizeof(line), file)) {

    switch (dex++) {

    case 0: cnc -> unit = atoi(line);
    default: break;
    }
  }

  fclose(file);
}

void read_position(CNC * cnc) {

  //.. creating directory
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
  printf("X: %.6f %s\n", r2v(cnc -> X, cnc -> unit), unit_name(cnc -> unit));
  printf("Y: %.6f %s\n", r2v(cnc -> Y, cnc -> unit), unit_name(cnc -> unit));
  printf("Z: %.6f %s\n", r2v(cnc -> Z, cnc -> unit), unit_name(cnc -> unit));
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
  printf(" Translation Rate: %.6f %s/min\n",   r2v(cnc -> mat.Fxy, cnc -> unit), unit_name(cnc -> unit));
  printf("      Plunge Rate: %.6f %s/min\n",   r2v(cnc -> mat.Fz, cnc -> unit), unit_name(cnc -> unit));
  printf("       Plunge Inc: %.6f %s\n",       r2v(cnc -> mat.dz, cnc -> unit), unit_name(cnc -> unit));
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
  printf("        Type: %s\n",      tool_name(cnc -> tool.type));
  printf("    Diameter: %.6f %s\n", r2v(cnc -> tool.d, cnc -> unit), unit_name(cnc -> unit));
  printf("Flute Length: %.6f %s\n", r2v(cnc -> tool.Lf, cnc -> unit), unit_name(cnc -> unit));
  printf("Total Length: %.6f %s\n", r2v(cnc -> tool.Lt, cnc -> unit), unit_name(cnc -> unit));
  printf("\n");
}

int parse_unit(char * str) {

  if (seq(str, "in")) { return IN; }
  if (seq(str, "mm")) { return MM; }

  return -1;
}

int parse_tool(char * str) {

  if (seq(str, "empty"))   { return NONE;    }
  if (seq(str, "endmill")) { return ENDMILL; }
}

char * tool_name(enum ToolType type) {

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
    
    if (seq(segs[1], "X"))           { printf("> X = %.6f %s\n", r2v(cnc -> X, cnc -> unit), unit_name(cnc -> unit));                     return true; }
    if (seq(segs[1], "Y"))           { printf("> Y = %.6f %s\n", r2v(cnc -> Y, cnc -> unit), unit_name(cnc -> unit));                     return true; }
    if (seq(segs[1], "Z"))           { printf("> Z = %.6f %s\n", r2v(cnc -> Z, cnc -> unit), unit_name(cnc -> unit));                     return true; }
    if (seq(segs[1], "P"))           { printf("> P = (%.6f, %.6f, %.6f) %s\n", r2v(cnc -> X, cnc -> unit),
					                                       r2v(cnc -> Y, cnc -> unit),
					                                       r2v(cnc -> Z, cnc -> unit),
					                                       unit_name(cnc -> unit));                                   return true; }
    if (seq(segs[1], "UnitType"))    { printf("> Unit Type = %s\n", unit_name(cnc -> unit));                                              return true; }
    if (seq(segs[1], "ToolType"))    { printf("> Tool Type = %s\n", tool_name(cnc -> tool.type));                                         return true; }
    if (seq(segs[1], "ToolSize"))    { printf("> Tool Size = %.6f %s\n", r2v(cnc -> tool.d, cnc -> unit), unit_name(cnc -> unit));        return true; }
    if (seq(segs[1], "FluteLength")) { printf("> Flute Length = %.6f %s\n", r2v(cnc -> tool.Lf, cnc -> unit), unit_name(cnc -> unit));    return true; }
    if (seq(segs[1], "ToolLength"))  { printf("> Tool Length = %.6f %s\n", r2v(cnc -> tool.Lt, cnc -> unit), unit_name(cnc -> unit));     return true; }
    if (seq(segs[1], "FeedRate"))    { printf("> Feed Rate = %.4f %s/min\n", r2v(cnc -> mat.Fxy, cnc -> unit), unit_name(cnc -> unit));   return true; }
    if (seq(segs[1], "PlungeRate"))  { printf("> Plunge Rate = %.4f %s/min\n", r2v(cnc -> mat.Fz, cnc -> unit), unit_name(cnc -> unit));  return true; }
    if (seq(segs[1], "PlungeInc"))   { printf("> Plunge Increment = %.6f %s\n", r2v(cnc -> mat.dz, cnc -> unit), unit_name(cnc -> unit)); return true; }
    if (seq(segs[1], "TransInc"))    { printf("> Translational Increment = %.2f%% of bit\n", cnc -> mat.dr * 100);                        return true; }
  }
  
  return false;
}

bool parse_set(CNC * cnc, char ** segs, int L) {

  double v1, v2, v3;
  
  if (seq(segs[0], "set")) {
    
    if (seq(segs[1], "X") && L == 3 && parse_lf(segs[2], &v1)) {

      cnc -> X = v2r(v1, cnc -> unit);
      printf("> X = %.6f %s\n", r2v(cnc -> X, cnc -> unit), unit_name(cnc -> unit));
    
      return true;
    }

    if (seq(segs[1], "Y") && L == 3 && parse_lf(segs[2], &v1)) {

      cnc -> Y = v2r(v1, cnc -> unit);
      printf("> Y = %.6f %s\n", r2v(cnc -> Y, cnc -> unit), unit_name(cnc -> unit));
    
      return true;
    }
    
    if (seq(segs[1], "Z") && L == 3 && parse_lf(segs[2], &v1)) {

      cnc -> Z = v2r(v1, cnc -> unit);
      printf("> Z = %.6f %s\n", r2v(cnc -> Z, cnc -> unit), unit_name(cnc -> unit));
    
      return true;
    }

    if (seq(segs[1], "P") && L == 5 && parse_lf(segs[2], &v1) && parse_lf(segs[3], &v2) && parse_lf(segs[4], &v3)) {

      cnc -> X = v2r(v1, cnc -> unit);
      cnc -> Y = v2r(v2, cnc -> unit);
      cnc -> Z = v2r(v3, cnc -> unit);

      printf("> P = (%.6f %.6f %.6f) %s\n", r2v(cnc -> X, cnc -> unit),
	                                    r2v(cnc -> Y, cnc -> unit),
	                                    r2v(cnc -> Z, cnc -> unit),
	                                    unit_name(cnc -> unit));

      return true;
    }

    if (seq(segs[1], "UnitType") && L == 3) {

      v1 = parse_unit(segs[2]);
      
      if (v1 < 0) { return false; }

      cnc -> unit = (int) v1;
      printf("> Unit Type = %s\n", unit_name(cnc -> unit));

      return true;
    }
    
    if (seq(segs[1], "ToolType") && L == 3) {

      v1 = parse_tool(segs[2]);
      
      if (v1 < 0) { return false; }

      cnc -> tool.type = (int) v1;
      printf("> Tool Type = %s\n", tool_name(cnc -> tool.type));

      return true;
    }
    
    if (seq(segs[1], "ToolSize") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> tool.d = v2r(v1, cnc -> unit);
      printf("> Tool Size = %.6f %s\n", r2v(cnc -> tool.d, cnc -> unit), unit_name(cnc -> unit));

      return true;
    }
    
    if (seq(segs[1], "FluteLength") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> tool.Lf = v2r(v1, cnc -> unit);
      printf("> Flute Length = %.6f %s\n", r2v(cnc -> tool.Lf, cnc -> unit), unit_name(cnc -> unit));

      return true;
    }
    
    if (seq(segs[1], "ToolLength") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> tool.Lt = v2r(v1, cnc -> unit);
      printf("> Tool Length = %.6f %s\n", r2v(cnc -> tool.Lt, cnc -> unit), unit_name(cnc -> unit));

      return true;
    }

    if (seq(segs[1], "FeedRate") && L == 3 && parse_lf(segs[2], &v1)) {

      v1 = v2r(v1, cnc -> unit);
      
      if (v1 <= 0 || v1 > FXY_max()) { return false; }
      
      cnc -> mat.Fxy = v1;
      printf("> Feed Rate = %.4f %s/min\n", r2v(cnc -> mat.Fxy, cnc -> unit), unit_name(cnc -> unit));

      return true;
    }

    if (seq(segs[1], "PlungeRate") && L == 3 && parse_lf(segs[2], &v1)) {

      v1 = v2r(v1, cnc -> unit);
      
      if (v1 <= 0 || v1 > FZ_max()) { return false; }
      
      cnc -> mat.Fz = v1;
      printf("> Plunge Rate = %.4f %s/min\n", r2v(cnc -> mat.Fz, cnc -> unit), unit_name(cnc -> unit));

      return true;
    }

    if (seq(segs[1], "PlungeInc") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> mat.dz = v2r(v1, cnc -> unit);
      printf("> Plunge Increment = %.6f %s\n", r2v(cnc -> mat.dz, cnc -> unit), unit_name(cnc -> unit));

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

int parse_spindle(CNC * cnc, char ** segs, int L) {

  if (L != 2 || !seq(segs[0], "spindle")) { return -1; }

  if (seq(segs[1],  "on")) { printf("> spindle on\n");  return 0; }
  if (seq(segs[1], "off")) { printf("> spindle off\n"); return 1; }

  return -1; 
}

Action ** parse_goto(CNC * cnc, char ** segs, int L) {

  if (L < 4 || L > 5 || !seq(segs[0], "goto")) { return NULL; }
  
  double X, Y, Z, F;
  
  if (!parse_lf(segs[1], &X) || !parse_lf(segs[2], &Y) || !parse_lf(segs[3], &Z)) { return false; }

  X = v2r(X, cnc -> unit);
  Y = v2r(Y, cnc -> unit);
  Z = v2r(Z, cnc -> unit);
  
  if (L == 5) {

    if (!parse_lf(segs[4], &F)) { return false; }
    F = v2r(F, cnc -> unit);

  } else if (L == 4) { F = F_max(fabs(X - cnc -> X), fabs(Y - cnc -> Y), fabs(Z - cnc -> Z)); }
  
  Action ** s = malloc(sizeof(Action *));
  s[0] = create_linear(X, Y, Z, F);
  
  return s;
}

Action ** parse_delta(CNC * cnc, char ** segs, int L) {

  if (L < 4 || L > 5 || !seq(segs[0], "delta")) { return NULL; }

  double dx, dy, dz, F;

  if (!parse_lf(segs[1], &dx) || !parse_lf(segs[2], &dy) || !parse_lf(segs[3], &dz)) { return false; }

  dx = v2r(dx, cnc -> unit);
  dy = v2r(dy, cnc -> unit);
  dz = v2r(dz, cnc -> unit);
  
  if (L == 5) {

    if (!parse_lf(segs[4], &F)) { return false; }
    F = v2r(F, cnc -> unit);

  } else if (L == 4) { F = F_max(dx, dy, dz); }

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

  x0 = v2r(x0, cnc -> unit);
  y0 = v2r(y0, cnc -> unit);
  z0 = v2r(z0, cnc -> unit);
  lx = v2r(lx, cnc -> unit);
  ly = v2r(ly, cnc -> unit);
  h  = v2r(h,  cnc -> unit);
  
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

  x0 = v2r(x0, cnc -> unit);
  y0 = v2r(y0, cnc -> unit);
  z0 = v2r(z0, cnc -> unit);
  lx = v2r(lx, cnc -> unit);
  ly = v2r(ly, cnc -> unit);
  h  = v2r(h,  cnc -> unit);
  
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

  x0 = v2r(x0, cnc -> unit);
  y0 = v2r(y0, cnc -> unit);
  z0 = v2r(z0, cnc -> unit);
  ri = v2r(ri, cnc -> unit);
  ro = v2r(ro, cnc -> unit);
  h  = v2r(h,  cnc -> unit);
  
  if (ri < 0 || ro <= 0)         { return NULL; }
  if ((ro - ri) < cnc -> tool.d) { return NULL; }

  if (lfeq(ri, 0)) { ri = -cnc -> tool.d / 2; }
  
  return circular_pocket(cnc -> mat.Fxy, cnc -> mat.Fz, cnc -> tool.d, x0, y0, z0, ri, ro, cnc -> mat.dr, h, cnc -> mat.dz, A);
}

Action ** parse_cutout(CNC * cnc, char ** segs, int L, int * A) {
  
  if (L != 11 || !seq(segs[0], "cutout")) { return NULL; }

  double z0, xmin, ymin, xmax, ymax, rtl, rtr, rbr, rbl, h;
  
  if (!parse_lf(segs[1], &z0) ||
      !parse_lf(segs[2], &xmin) ||
      !parse_lf(segs[3], &ymin) ||
      !parse_lf(segs[4], &xmax) ||
      !parse_lf(segs[5], &ymax) ||
      !parse_lf(segs[6], &rtl) ||
      !parse_lf(segs[7], &rtr) ||
      !parse_lf(segs[8], &rbr) ||
      !parse_lf(segs[9], &rbl) ||
      !parse_lf(segs[10], &h)) { return NULL; }

  z0   = v2r(z0, cnc -> unit);
  xmin = v2r(xmin, cnc -> unit);
  ymin = v2r(ymin, cnc -> unit);
  xmax = v2r(xmax, cnc -> unit);
  ymax = v2r(ymax, cnc -> unit);
  rtl  = v2r(rtl, cnc -> unit);
  rtr  = v2r(rtr, cnc -> unit);
  rbr  = v2r(rbr, cnc -> unit);
  rbl  = v2r(rbl, cnc -> unit);
  h    = v2r(h, cnc -> unit);

  if ((xmax - xmin) < rbl + rbr)     { return NULL; }
  if ((xmax - xmin) < rtl + rtr)     { return NULL; }
  if ((ymax - ymin) < rbl + rtl)     { return NULL; }
  if ((ymax - ymin) < rbr + rtr)     { return NULL; }
  if ((xmax - xmin) < cnc -> tool.d) { return NULL; }
  if ((ymax - ymin) < cnc -> tool.d) { return NULL; }
  if (rtl < 0)                       { return NULL; }
  if (rtr < 0)                       { return NULL; }
  if (rbr < 0)                       { return NULL; }
  if (rbl < 0)                       { return NULL; }
  
  return cutout(cnc -> mat.Fxy, cnc -> mat.Fz, cnc -> tool.d, z0, xmin, ymin, xmax, ymax, rtl, rtr, rbr, rbl, h, cnc -> mat.dz, A);
}

