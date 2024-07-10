#include "cio.h"

void printl(const char * text) {

  printw("%s\n", text);
  refresh();
}

char * read_line(void) {

  char * line = malloc(sizeof(char) * CMDLEN);
  getstr(line);

  return line;
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

  printw("------------ POSITION ------------\n");
  printw("X: %.6f %s\n", r2v(cnc -> X, cnc -> unit), unit_name(cnc -> unit));
  printw("Y: %.6f %s\n", r2v(cnc -> Y, cnc -> unit), unit_name(cnc -> unit));
  printw("Z: %.6f %s\n", r2v(cnc -> Z, cnc -> unit), unit_name(cnc -> unit));
  printw("\n");
  refresh();
  
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

  printw("------------ MATERIAL ------------\n");
  printw("Translation Rate: %.6f %s/min\n",   r2v(cnc -> mat.Fxy, cnc -> unit), unit_name(cnc -> unit));
  printw("     Plunge Rate: %.6f %s/min\n",   r2v(cnc -> mat.Fz, cnc -> unit), unit_name(cnc -> unit));
  printw("       Step Down: %.6f %s\n",       r2v(cnc -> mat.dz, cnc -> unit), unit_name(cnc -> unit));
  printw("       Step Over: %.2f%% of bit\n", cnc -> mat.dr * 100);
  printw("\n");
  refresh();
}

void read_tool(CNC * cnc) {

  cnc -> tool.type = NONE;

  FILE * file = fopen(TPATH, "ab+");

  int dex = 0;

  char line[CMDLEN];

  while (fgets(line, sizeof(line), file)) {

    switch (dex++) {

    case 0: cnc -> tool.type = atoi(line);
    case 1: cnc -> tool.Dc   = atof(line);
    case 2: cnc -> tool.Ds   = atof(line);
    case 3: cnc -> tool.Lc   = atof(line);
    case 4: cnc -> tool.Lt   = atof(line);
    default: break;
    }
  }

  fclose(file);

  printw("-------------- TOOL --------------\n");
  printw("            Type: %s\n",       tooltype_name(cnc -> tool.type));
  printw("Cutting Diameter: %.6f %s\n",  r2v(cnc -> tool.Dc, cnc -> unit), unit_name(cnc -> unit));
  printw("  Shank Diameter: %.6f %s\n",  r2v(cnc -> tool.Ds, cnc -> unit), unit_name(cnc -> unit));
  printw("  Cutting Length: %.6f %s\n",  r2v(cnc -> tool.Lc, cnc -> unit), unit_name(cnc -> unit));
  printw("    Total Length: %.6f %s\n",  r2v(cnc -> tool.Lt, cnc -> unit), unit_name(cnc -> unit));
  printw("\n");
  refresh();
}

int parse_unit(char * str) {

  if (seq(str, "in")) { return IN; }
  if (seq(str, "mm")) { return MM; }

  return -1;
}

int parse_tooltype(char * str) {

  if (seq(str, "empty"))    { return NONE;     }
  if (seq(str, "flatmill")) { return FLATMILL; }
  if (seq(str, "ballmill")) { return BALLMILL; }
  if (seq(str, "chamfer"))  { return CHAMFER;  }
  if (seq(str, "drill"))    { return DRILL;    }
}

char * tooltype_name(enum ToolType type) {

  switch (type) {

  case NONE:     return "empty";
  case FLATMILL: return "flatmill";
  case BALLMILL: return "ballmill";
  case CHAMFER:  return "chamfer";
  case DRILL:    return "drill";
  default: break;
  }

  return "invalid type";
}

char * unittype_name(enum UnitType type) {

  switch (type) {

  case MM: return "mm";
  case IN: return "in";
  default: break;
  }

  return "invalid type";
}

char ** parse_toolchange(CNC * cnc, char ** segs, int L, int * A) {

  if (L != 2 || !seq(segs[0], "toolchange")) { return NULL; }

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

bool parse_exit(char ** segs, int L) {

  if (L == 1 && seq(segs[0], "exit")) { return true; }
  
  return false;
}

bool parse_hold(char ** segs, int L) {

  if (L == 1 && seq(segs[0], "hold")) { printl("> program holding"); return true; }

  return false;
}

bool parse_cont(char ** segs, int L) {

  if (L == 1 && seq(segs[0], "continue")) { printl("> program continuing"); return true; }

  return false;
}

bool parse_stop(char ** segs, int L) {

  if (L == 1 && seq(segs[0], "stop")) { printl("> program stopped"); return true; }

  return false;
}

bool parse_skip(char ** segs, int L) {

  if (L == 1 && seq(segs[0], "skip")) { printl("> skipping command. . ."); return true; }

  return false;
}

bool parse_get(CNC * cnc, char ** segs, int L) {
  
  if (L == 2 && seq(segs[0], "get")) {
    
    if (seq(segs[1], "X"))               { printw("> X = %.6f %s\n", r2v(cnc -> X, cnc -> unit), unit_name(cnc -> unit));                      refresh(); return true; }
    if (seq(segs[1], "Y"))               { printw("> Y = %.6f %s\n", r2v(cnc -> Y, cnc -> unit), unit_name(cnc -> unit));                      refresh(); return true; }
    if (seq(segs[1], "Z"))               { printw("> Z = %.6f %s\n", r2v(cnc -> Z, cnc -> unit), unit_name(cnc -> unit));                      refresh(); return true; }
    if (seq(segs[1], "P"))               { printw("> P = (%.6f, %.6f, %.6f) %s\n", r2v(cnc -> X, cnc -> unit),
					                                           r2v(cnc -> Y, cnc -> unit),
					                                           r2v(cnc -> Z, cnc -> unit),
					                                           unit_name(cnc -> unit));                                    refresh(); return true; }
    if (seq(segs[1], "UnitType"))        { printw("> Unit Type = %s\n", unit_name(cnc -> unit));                                               refresh(); return true; }
    if (seq(segs[1], "ToolType"))        { printw("> Tool Type = %s\n", tooltype_name(cnc -> tool.type));                                      refresh(); return true; }
    if (seq(segs[1], "CuttingDiameter")) { printw("> Cutting Diameter = %.6f %s\n", r2v(cnc -> tool.Dc, cnc -> unit), unit_name(cnc -> unit)); refresh(); return true; }
    if (seq(segs[1], "ShankDiameter"))   { printw("> Shank Diameter = %.6f %s\n", r2v(cnc -> tool.Ds, cnc -> unit), unit_name(cnc -> unit));   refresh(); return true; }
    if (seq(segs[1], "CuttingLength"))   { printw("> Cutting Length = %.6f %s\n", r2v(cnc -> tool.Lc, cnc -> unit), unit_name(cnc -> unit));   refresh(); return true; }
    if (seq(segs[1], "ToolLength"))      { printw("> Tool Length = %.6f %s\n", r2v(cnc -> tool.Lt, cnc -> unit), unit_name(cnc -> unit));      refresh(); return true; }
    if (seq(segs[1], "FeedRate"))        { printw("> Feed Rate = %.4f %s/min\n", r2v(cnc -> mat.Fxy, cnc -> unit), unit_name(cnc -> unit));    refresh(); return true; }
    if (seq(segs[1], "PlungeRate"))      { printw("> Plunge Rate = %.4f %s/min\n", r2v(cnc -> mat.Fz, cnc -> unit), unit_name(cnc -> unit));   refresh(); return true; }
    if (seq(segs[1], "StepDown"))        { printw("> Step Down = %.6f %s\n", r2v(cnc -> mat.dz, cnc -> unit), unit_name(cnc -> unit));         refresh(); return true; }
    if (seq(segs[1], "StepOver"))        { printw("> Step Over = %.2f%% of bit\n", cnc -> mat.dr * 100);                                       refresh(); return true; }
  }
  
  return false;
}

bool parse_set(CNC * cnc, char ** segs, int L) {

  double v1, v2, v3;
  
  if (seq(segs[0], "set")) {
    
    if (seq(segs[1], "X") && L == 3 && parse_lf(segs[2], &v1)) {

      cnc -> X = v2r(v1, cnc -> unit);
      printw("> X = %.6f %s\n", r2v(cnc -> X, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }

    if (seq(segs[1], "Y") && L == 3 && parse_lf(segs[2], &v1)) {

      cnc -> Y = v2r(v1, cnc -> unit);
      printw("> Y = %.6f %s\n", r2v(cnc -> Y, cnc -> unit), unit_name(cnc -> unit));
      refresh();
    
      return true;
    }
    
    if (seq(segs[1], "Z") && L == 3 && parse_lf(segs[2], &v1)) {

      cnc -> Z = v2r(v1, cnc -> unit);
      printw("> Z = %.6f %s\n", r2v(cnc -> Z, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }

    if (seq(segs[1], "P") && L == 5 && parse_lf(segs[2], &v1) && parse_lf(segs[3], &v2) && parse_lf(segs[4], &v3)) {

      cnc -> X = v2r(v1, cnc -> unit);
      cnc -> Y = v2r(v2, cnc -> unit);
      cnc -> Z = v2r(v3, cnc -> unit);

      printw("> P = (%.6f %.6f %.6f) %s\n", r2v(cnc -> X, cnc -> unit),
	                                    r2v(cnc -> Y, cnc -> unit),
	                                    r2v(cnc -> Z, cnc -> unit),
	                                    unit_name(cnc -> unit));
      refresh();
      
      return true;
    }

    if (seq(segs[1], "UnitType") && L == 3) {

      v1 = parse_unit(segs[2]);
      
      if (v1 < 0) { return false; }

      cnc -> unit = (int) v1;
      printw("> Unit Type = %s\n", unit_name(cnc -> unit));
      refresh();
      
      return true;
    }
    
    if (seq(segs[1], "ToolType") && L == 3) {

      v1 = parse_tooltype(segs[2]);
      
      if (v1 < 0) { return false; }

      cnc -> tool.type = (int) v1;
      printw("> Tool Type = %s\n", tooltype_name(cnc -> tool.type));
      refresh();
      
      return true;
    }
    
    if (seq(segs[1], "CuttingDiameter") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 < 0) { return false; }
      
      cnc -> tool.Dc = v2r(v1, cnc -> unit);
      printw("> Cutting Diameter = %.6f %s\n", r2v(cnc -> tool.Dc, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }
    
    if (seq(segs[1], "ShankDiameter") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> tool.Ds = v2r(v1, cnc -> unit);
      printw("> Shank Diameter = %.6f %s\n", r2v(cnc -> tool.Ds, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }
    
    if (seq(segs[1], "CuttingLength") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> tool.Lc = v2r(v1, cnc -> unit);
      printw("> Cutting Length = %.6f %s\n", r2v(cnc -> tool.Lc, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }
    
    if (seq(segs[1], "ToolLength") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> tool.Lt = v2r(v1, cnc -> unit);
      printw("> Tool Length = %.6f %s\n", r2v(cnc -> tool.Lt, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }
    
    if (seq(segs[1], "FeedRate") && L == 3 && parse_lf(segs[2], &v1)) {

      v1 = v2r(v1, cnc -> unit);
      
      if (v1 <= 0 || v1 > FXY_max()) { return false; }
      
      cnc -> mat.Fxy = v1;
      printw("> Feed Rate = %.4f %s/min\n", r2v(cnc -> mat.Fxy, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }

    if (seq(segs[1], "PlungeRate") && L == 3 && parse_lf(segs[2], &v1)) {

      v1 = v2r(v1, cnc -> unit);
      
      if (v1 <= 0 || v1 > FZ_max()) { return false; }
      
      cnc -> mat.Fz = v1;
      printw("> Plunge Rate = %.4f %s/min\n", r2v(cnc -> mat.Fz, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }

    if (seq(segs[1], "StepDown") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> mat.dz = v2r(v1, cnc -> unit);
      printw("> Plunge Increment = %.6f %s\n", r2v(cnc -> mat.dz, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }

    if (seq(segs[1], "StepOver") && L == 3 && parse_lf(segs[2], &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> mat.dr = v1 / 100;
      printw("> Translational Increment = %.2f%% of bit\n", cnc -> mat.dr * 100);
      refresh();
      
      return true;
    }
  }
  
  return false;
}

bool parse_comment(CNC * cnc, char ** segs, int L) {

  if (L < 2 || !seq(segs[0], "#")) { return false; }

  printw(">");

  for (int l = 1 ; l < L ; ++l) {

    printw(" %s", segs[l]);
  }

  printw("\n");

  refresh();
  
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

  if (seq(segs[1],  "on")) { printl("> spindle on");  return 0; }
  if (seq(segs[1], "off")) { printl("> spindle off"); return 1; }

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
  
  if (lx < 0 || ly < 0 || h < 0)                  { return NULL; }
  if (lx < cnc -> tool.Dc || ly < cnc -> tool.Dc) { return NULL; } 
  
  return face(cnc -> mat.Fxy, cnc -> mat.Fz, cnc -> tool.Dc, x0, y0, z0, lx, ly, cnc -> mat.dr, h, cnc -> mat.dz, A);
}

Action ** parse_square_pocket(CNC * cnc, char ** segs, int L, int * A) {

  if ((L != 7 && L != 9) || !seq(segs[0], "square_pocket")) { return NULL; }

  double x0, y0, z0, lxi, lyi, lxo, lyo, h;

  lxi = lyi = 0;
  
  if (!parse_lf(segs[1],   &x0) ||
      !parse_lf(segs[2],   &y0) ||
      !parse_lf(segs[3],   &z0) ||
      (L == 9 && !parse_lf(segs[4], &lxi)) ||
      (L == 9 && !parse_lf(segs[5], &lyi)) ||
      !parse_lf(segs[L-3], &lxo) ||
      !parse_lf(segs[L-2], &lyo) ||
      !parse_lf(segs[L-1], &h)) { return NULL; }

  x0 = v2r(x0, cnc -> unit);
  y0 = v2r(y0, cnc -> unit);
  z0 = v2r(z0, cnc -> unit);
  lxi = v2r(lxi, cnc -> unit);
  lyi = v2r(lyi, cnc -> unit);
  lxo = v2r(lxo, cnc -> unit);
  lyo = v2r(lyo, cnc -> unit);
  h  = v2r(h,  cnc -> unit);
  
  if (lxo < 0 || lyo < 0 || lxi < 0 || lyi < 0 || h < 0)                    { return NULL; }
  if ((lxo - lxi) / 2 < cnc -> tool.Dc || (lyo - lyi) / 2 < cnc -> tool.Dc) { return NULL; } 

  Action ** s;
  
  if (L == 7) { s = square_pocket(cnc -> mat.Fxy, cnc -> mat.Fz, cnc -> tool.Dc, x0, y0, z0, lxo, lyo, cnc -> mat.dr, h, cnc -> mat.dz, A); }
  if (L == 9) { s = nested_square_pocket(cnc -> mat.Fxy, cnc -> mat.Fz, cnc -> tool.Dc, x0, y0, z0, lxi, lyi, lxo, lyo, cnc -> mat.dr, h, cnc -> mat.dz, A); }

  return s;
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
  
  if (ri < 0 || ro <= 0)          { return NULL; }
  if ((ro - ri) < cnc -> tool.Dc) { return NULL; }

  if (lfeq(ri, 0)) { ri = -cnc -> tool.Dc / 2; }
  
  return circular_pocket(cnc -> mat.Fxy, cnc -> mat.Fz, cnc -> tool.Dc, x0, y0, z0, ri, ro, cnc -> mat.dr, h, cnc -> mat.dz, A);
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

  if ((xmax - xmin) < rbl + rbr)      { return NULL; }
  if ((xmax - xmin) < rtl + rtr)      { return NULL; }
  if ((ymax - ymin) < rbl + rtl)      { return NULL; }
  if ((ymax - ymin) < rbr + rtr)      { return NULL; }
  if ((xmax - xmin) < cnc -> tool.Dc) { return NULL; }
  if ((ymax - ymin) < cnc -> tool.Dc) { return NULL; }
  if (rtl < 0)                        { return NULL; }
  if (rtr < 0)                        { return NULL; }
  if (rbr < 0)                        { return NULL; }
  if (rbl < 0)                        { return NULL; }
  
  return cutout(cnc -> mat.Fxy, cnc -> mat.Fz, cnc -> tool.Dc, z0, xmin, ymin, xmax, ymax, rtl, rtr, rbr, rbl, h, cnc -> mat.dz, A);
}

Action ** parse_drill(CNC * cnc, char ** segs, int L, int * A) {

  if (L != 5 || !seq(segs[0], "drill")) { return NULL; }

  double x0, y0, z0, h;
  
  if (!parse_lf(segs[2], &x0) ||
      !parse_lf(segs[3], &y0) ||
      !parse_lf(segs[4], &z0) ||
      !parse_lf(segs[6], &h)) { return NULL; }

   x0 = v2r(x0, cnc -> unit);
   y0 = v2r(y0, cnc -> unit);
   z0 = v2r(z0, cnc -> unit);
   h  = v2r(h, cnc -> unit);

   if (h <= 0) { return NULL; }
   
   return drill(cnc -> mat.Fz, x0, y0, z0, h, A);
}

Action ** parse_bore(CNC * cnc, char ** segs, int L, int * A) {

  if (L != 6 || !seq(segs[0], "bore")) { return NULL; }

  double x0, y0, z0, r, h;
  
  if (!parse_lf(segs[1], &x0) ||
      !parse_lf(segs[2], &y0) ||
      !parse_lf(segs[3], &z0) ||
      !parse_lf(segs[4], &r) ||
      !parse_lf(segs[5], &h)) { return NULL; }

   x0 = v2r(x0, cnc -> unit);
   y0 = v2r(y0, cnc -> unit);
   z0 = v2r(z0, cnc -> unit);
   r  = v2r(r, cnc -> unit);
   h  = v2r(h, cnc -> unit);

   if (h <= 0)                                        { return NULL; }
   if (r <= cnc -> tool.Dc || r > 2 * cnc -> tool.Dc) { return NULL; }
   
   return bore(cnc -> mat.Fxy, cnc -> mat.Fz, cnc -> tool.Dc, x0, y0, z0, r, h, A);
}

Action ** parse_fillet(CNC * cnc, char ** segs, int L, int * A) {

  if (L != 7 || ! seq(segs[0], "fillet")) { return NULL; }

  double x0, y0, z0, r, a, h;

  if (!parse_lf(segs[1], &x0) ||
      !parse_lf(segs[2], &y0) ||
      !parse_lf(segs[3], &z0) ||
      !parse_lf(segs[4], &r) ||
      !parse_lf(segs[5], &a) ||
      !parse_lf(segs[6], &h)) { return NULL; }

  x0 = v2r(x0, cnc -> unit);
  y0 = v2r(y0, cnc -> unit);
  z0 = v2r(z0, cnc -> unit);
  r  = v2r(r,  cnc -> unit);
  h  = v2r(h,  cnc -> unit);

  if (h <= 0) { return NULL; }
  if (r <= 0) { return NULL; }
  if (a <  0) { return NULL; }

  while (a >= 360) { a -= 360; }

  a *= M_PI / 180;

  return fillet(cnc -> mat.Fxy, cnc -> mat.Fz, cnc -> tool.Dc, x0, y0, z0, r, a, h, cnc -> mat.dz, A);
}

Action ** parse_engrave(CNC * cnc, char ** segs, int L, int * A) {

  if (L != 8 || !seq(segs[0], "engrave")) { return NULL; }

  double x0, y0, z0, w, h, a;
  
  if (!parse_lf(segs[2], &x0) ||
      !parse_lf(segs[3], &y0) ||
      !parse_lf(segs[4], &z0) ||
      !parse_lf(segs[5], &w) ||
      !parse_lf(segs[6], &h) ||
      !parse_lf(segs[7], &a)) { return NULL; }

   x0 = v2r(x0, cnc -> unit);
   y0 = v2r(y0, cnc -> unit);
   z0 = v2r(z0, cnc -> unit);
   w  = v2r(w, cnc -> unit);
   h  = v2r(h, cnc -> unit);
   a  = a * M_PI / 180;

   if (w <= 0) { return NULL; }
   if (h <= 0) { return NULL; }

   char * text = malloc(sizeof(char) * strlen(segs[1]));
   strcpy(text, segs[1]);
   
   return engrave_text(cnc -> mat.Fxy, cnc -> mat.Fz, text, x0, y0, z0, w, h, a, A);
}
