#include "../include/cio.h"

char * read_line(void) {

  char * line = malloc(sizeof(char) * CMDLEN);
  getstr(line);

  return line;
}

char ** split_line(char * line, char del, int * L) {

  //.. pruning unecessary characters
  bool lastspace = false;

  int chr;
  int l2 = 0;
  char * line2 = malloc(sizeof(char) * (strlen(line) + 1));

  for (int l = 0 ; l < strlen(line) ; ++l) {

    chr = (int) line[l];
    
    if (chr != 0 && chr != 10 && !(lastspace && chr == 32)) {

      line2[l2++] = line[l];
    }

    if (chr == 32) { lastspace = true;  }
    else           { lastspace = false; }
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
  
  cnc -> rox = 0;
  cnc -> roy = 0;
  cnc -> roz = 0;

  cnc -> X0 = 0;
  cnc -> Y0 = 0;
  cnc -> Z0 = 0;
  
  FILE * file = fopen(PPATH, "ab+");

  int dex = 0;

  char line[CMDLEN];

  while (fgets(line, sizeof(line), file)) {

    switch (dex++) {

    case 0: cnc -> X   = atof(line);
    case 1: cnc -> Y   = atof(line);
    case 2: cnc -> Z   = atof(line);
    case 3: cnc -> rox = atoi(line);
    case 4: cnc -> roy = atoi(line);
    case 5: cnc -> roz = atoi(line);
    case 6: cnc -> X0  = atof(line);
    case 7: cnc -> Y0  = atof(line);
    case 8: cnc -> Z0  = atof(line);
      
    default: break;
    }
  }

  printw("------------ POSITION ------------\n");
  printw(" P: (%.6f %.6f %.6f) %s\n", r2v(cnc -> X, cnc -> unit),  r2v(cnc -> Y, cnc -> unit),  r2v(cnc -> Z,  cnc -> unit), unit_name(cnc -> unit));
  printw("P0: (%.6f %.6f %.6f) %s\n", r2v(cnc -> X0, cnc -> unit), r2v(cnc -> Y0, cnc -> unit), r2v(cnc -> Z0, cnc -> unit), unit_name(cnc -> unit));
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
  printw("            Type: %s\n",      tooltype_name(cnc -> tool.type));
  printw("Cutting Diameter: %.6f %s\n", r2v(cnc -> tool.Dc, cnc -> unit), unit_name(cnc -> unit));
  printw("  Shank Diameter: %.6f %s\n", r2v(cnc -> tool.Ds, cnc -> unit), unit_name(cnc -> unit));
  printw("  Cutting Length: %.6f %s\n", r2v(cnc -> tool.Lc, cnc -> unit), unit_name(cnc -> unit));
  printw("    Total Length: %.6f %s\n", r2v(cnc -> tool.Lt, cnc -> unit), unit_name(cnc -> unit));
  printw("\n");
  refresh();
}

bool parse_lf(char * str, char ** vn, double * vv, int V, double * val) {

  //.. copy string
  char * str2 = cstr(str);
  
  //.. checking sign
  double sign = 0;
    
  switch (str2[0]) {

  case '-': sign = -1; break;
  case '+': sign =  1; break;
  default: break;
  }

  //.. truncate sign
  if (!lfeq(sign, 0)) {

    for (int c = 1 ; c < strlen(str2) ; c++) {

      str2[c - 1] = str2[c];
    }

    str2[strlen(str2) - 1] = '\0';
    
  } else { sign = 1; }
  
  //.. check for variables
  for (int v = 0 ; v < V; v++) {

    if (seq(str2, vn[v])) { *val = sign * vv[v]; return true; }
  }

  free(str2);
  
  //.. parse original string
  char * endptr = 0;
  strtod(str, &endptr);

  if (*endptr != '\0' || endptr == str) { return false; }

  (*val) = atof(str);

  return true;
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

char ** parse_toolchange(char ** segs, int L, int * A) {

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

  if (L == 1 && seq(segs[0], "hold")) { printw("> program holding\n"); refresh(); return true; }

  return false;
}

bool parse_cont(char ** segs, int L) {

  if (L == 1 && seq(segs[0], "continue")) { printw("> program continuing\n"); refresh(); return true; }

  return false;
}

bool parse_stop(char ** segs, int L) {

  if (L == 1 && seq(segs[0], "stop")) { printw("> program stopped\n"); refresh(); return true; }

  return false;
}

uint32_t parse_skip(char ** segs, int L, char ** vn, double * vv, int V) {
  
  double s = 0;

  if ((L != 1 && L != 2) || !seq(segs[0], "skip"))             { return 0; }
  if (L == 2 && (!parse_lf(segs[1], vn, vv, V, &s) || s <= 0)) { return 0; }
  if (L == 1)                                                  { s = 1;    }
  
  printw("> skipping %u command(s). . .\n", (uint32_t) s); refresh();

  return (uint32_t) s;
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
    if (seq(segs[1], "X0"))              { printw("> X0 = %.6f %s\n", r2v(cnc -> X0, cnc -> unit), unit_name(cnc -> unit));                    refresh(); return true; }
    if (seq(segs[1], "Y0"))              { printw("> Y0 = %.6f %s\n", r2v(cnc -> Y0, cnc -> unit), unit_name(cnc -> unit));                    refresh(); return true; }
    if (seq(segs[1], "Z0"))              { printw("> Z0 = %.6f %s\n", r2v(cnc -> Z0, cnc -> unit), unit_name(cnc -> unit));                    refresh(); return true; }
    if (seq(segs[1], "P0"))              { printw("> P0 = (%.6f, %.6f, %.6f) %s\n", r2v(cnc -> X0, cnc -> unit),
					                                           r2v(cnc -> Y0, cnc -> unit),
					                                           r2v(cnc -> Z0, cnc -> unit),
					                                           unit_name(cnc -> unit));                                    refresh(); return true; }
    if (seq(segs[1], "Runout"))          { printw("> Runout = (%d, %d, %d) steps\n", cnc -> rox,
					                                             cnc -> roy,
					                                             cnc -> roz);                                              refresh(); return true; }
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

bool parse_set(CNC * cnc, char ** segs, int L, char ** vn, double * vv, int V) {

  double v1, v2, v3;
  
  if (seq(segs[0], "set")) {
    
    if (seq(segs[1], "X") && L == 3 && parse_lf(segs[2], vn, vv, V, &v1)) {

      cnc -> X = v2r(v1, cnc -> unit);
      printw("> X = %.6f %s\n", r2v(cnc -> X, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }

    if (seq(segs[1], "Y") && L == 3 && parse_lf(segs[2], vn, vv, V, &v1)) {

      cnc -> Y = v2r(v1, cnc -> unit);
      printw("> Y = %.6f %s\n", r2v(cnc -> Y, cnc -> unit), unit_name(cnc -> unit));
      refresh();
    
      return true;
    }
    
    if (seq(segs[1], "Z") && L == 3 && parse_lf(segs[2], vn, vv, V, &v1)) {

      cnc -> Z = v2r(v1, cnc -> unit);
      printw("> Z = %.6f %s\n", r2v(cnc -> Z, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }

    if (seq(segs[1], "P") && L == 5 && parse_lf(segs[2], vn, vv, V, &v1) && parse_lf(segs[3], vn, vv, V, &v2) && parse_lf(segs[4], vn, vv, V, &v3)) {

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


    if (seq(segs[1], "X0") && L == 3 && parse_lf(segs[2], vn, vv, V, &v1)) {

      cnc -> X0 = v2r(v1, cnc -> unit);
      printw("> X0 = %.6f %s\n", r2v(cnc -> X0, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }

    if (seq(segs[1], "Y0") && L == 3 && parse_lf(segs[2], vn, vv, V, &v1)) {

      cnc -> Y0 = v2r(v1, cnc -> unit);
      printw("> Y0 = %.6f %s\n", r2v(cnc -> Y0, cnc -> unit), unit_name(cnc -> unit));
      refresh();
    
      return true;
    }
    
    if (seq(segs[1], "Z0") && L == 3 && parse_lf(segs[2], vn, vv, V, &v1)) {

      cnc -> Z0 = v2r(v1, cnc -> unit);
      printw("> Z0 = %.6f %s\n", r2v(cnc -> Z0, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }

    if (seq(segs[1], "P0") && L == 5 && parse_lf(segs[2], vn, vv, V, &v1) && parse_lf(segs[3], vn, vv, V, &v2) && parse_lf(segs[4], vn, vv, V, &v3)) {

      cnc -> X0 = v2r(v1, cnc -> unit);
      cnc -> Y0 = v2r(v2, cnc -> unit);
      cnc -> Z0 = v2r(v3, cnc -> unit);

      printw("> P0 = (%.6f %.6f %.6f) %s\n", r2v(cnc -> X0, cnc -> unit),
	                                     r2v(cnc -> Y0, cnc -> unit),
	                                     r2v(cnc -> Z0, cnc -> unit),
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
    
    if (seq(segs[1], "CuttingDiameter") && L == 3 && parse_lf(segs[2], vn, vv, V, &v1)) {

      if (v1 < 0) { return false; }
      
      cnc -> tool.Dc = v2r(v1, cnc -> unit);
      printw("> Cutting Diameter = %.6f %s\n", r2v(cnc -> tool.Dc, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }
    
    if (seq(segs[1], "ShankDiameter") && L == 3 && parse_lf(segs[2], vn, vv, V, &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> tool.Ds = v2r(v1, cnc -> unit);
      printw("> Shank Diameter = %.6f %s\n", r2v(cnc -> tool.Ds, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }
    
    if (seq(segs[1], "CuttingLength") && L == 3 && parse_lf(segs[2], vn, vv, V, &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> tool.Lc = v2r(v1, cnc -> unit);
      printw("> Cutting Length = %.6f %s\n", r2v(cnc -> tool.Lc, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }
    
    if (seq(segs[1], "ToolLength") && L == 3 && parse_lf(segs[2], vn, vv, V, &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> tool.Lt = v2r(v1, cnc -> unit);
      printw("> Tool Length = %.6f %s\n", r2v(cnc -> tool.Lt, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }
    
    if (seq(segs[1], "FeedRate") && L == 3 && parse_lf(segs[2], vn, vv, V, &v1)) {

      v1 = v2r(v1, cnc -> unit);
      
      if (v1 <= 0 || v1 > FXY_max()) { return false; }
      
      cnc -> mat.Fxy = v1;
      printw("> Feed Rate = %.4f %s/min\n", r2v(cnc -> mat.Fxy, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }

    if (seq(segs[1], "PlungeRate") && L == 3 && parse_lf(segs[2], vn, vv, V, &v1)) {

      v1 = v2r(v1, cnc -> unit);
      
      if (v1 <= 0 || v1 > FZ_max()) { return false; }
      
      cnc -> mat.Fz = v1;
      printw("> Plunge Rate = %.4f %s/min\n", r2v(cnc -> mat.Fz, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }

    if (seq(segs[1], "StepDown") && L == 3 && parse_lf(segs[2], vn, vv, V, &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> mat.dz = v2r(v1, cnc -> unit);
      printw("> Step Down = %.6f %s\n", r2v(cnc -> mat.dz, cnc -> unit), unit_name(cnc -> unit));
      refresh();
      
      return true;
    }

    if (seq(segs[1], "StepOver") && L == 3 && parse_lf(segs[2], vn, vv, V, &v1)) {

      if (v1 <= 0) { return false; }
      
      cnc -> mat.dr = v1 / 100;
      printw("> Step Over = %.2f%% of bit\n", cnc -> mat.dr * 100);
      refresh();
      
      return true;
    }

    if (seq(segs[1], "Runout") && L == 5 && parse_lf(segs[2], vn, vv, V, &v1) && parse_lf(segs[3], vn, vv, V, &v2) && parse_lf(segs[4], vn, vv, V, &v3)) {

      cnc -> rox = (int) v1;
      cnc -> roy = (int) v2;
      cnc -> roz = (int) v3;

      if (cnc -> rox < 0) { cnc -> rox = 0; }
      if (cnc -> roy < 0) { cnc -> roy = 0; }
      if (cnc -> roz < 0) { cnc -> roz = 0; }

      if (cnc -> rox > cnc -> ROX) { cnc -> rox = cnc -> ROX; }
      if (cnc -> roy > cnc -> ROY) { cnc -> roy = cnc -> ROY; }
      if (cnc -> roz > cnc -> ROZ) { cnc -> roz = cnc -> ROZ; }
      
      printw("> Runout = (%d, %d, %d) steps\n", cnc -> rox, cnc -> roy, cnc -> roz);
      
      refresh();
      
      return true;
    }
  }
  
  return false;
}

bool parse_comment(char ** segs, int L) {

  if (L < 2 || !seq(segs[0], "#")) { return false; }

  printw(">");

  for (int l = 1 ; l < L ; ++l) {

    printw(" %s", segs[l]);
  }

  printw("\n");

  refresh();
  
  return true;
}

bool parse_var(char *** vn, double ** vv, int * V, char ** segs, int L) {

  if (L < 2 || !seq(segs[0], "var")) { return false; }

  //.. parse value
  double sum = 0;
  double val;
  
  for (int v = 2 ; v < L ; v++) {

    if (!parse_lf(segs[v], *vn, *vv, *V, &val)) { return false; }

    sum += val;
  }
  
  //.. existing variable
  for (int v = 0 ; v < *V ; v++) {

    if (seq((*vn)[v], segs[1])) {

      //.. set variable
      if (L > 2) {

	(*vv)[v] = sum;
      }
      
      //.. readout variable
      printw("> %s = %lf\n", (*vn)[v], (*vv)[v]);
      refresh();
      
      return true;
    }
  }

  if (L == 2) { return false; }

  //.. check for valid name
  for (int c = 0 ; c < strlen(segs[1]) ; c++) {

    if (segs[1][c] == '-' || segs[1][c] == '+') { return false; }
  }
  
  //.. new variable
 (*V)++;

  *vn = (char **)  realloc(*vn, sizeof(char *) * (*V));
  *vv = (double *) realloc(*vv, sizeof(double) * (*V));

  (*vn)[(*V) - 1] = cstr(segs[1]);
  (*vv)[(*V) - 1] = sum;

  printw("> %s = %lf\n", (*vn)[(*V) - 1], (*vv)[(*V) - 1]);
  refresh();
      
  return true;
}

bool parse_div(char ** vn, double * vv, int  V, char ** segs, int L) {
  
  if (L != 3 || !seq(segs[0], "div")) { return false; }

  //.. parse value
  double val;
  
  if (!parse_lf(segs[2], vn, vv, V, &val)) { return false; }

  if (lfeq(val, 0)) { return false; }

  //.. find existing variable
  for (int v = 0 ; v < V ; v++) {

    if (seq(segs[1], vn[v])) {

      vv[v] /= val;

      printw("> %s = %lf\n", vn[v], vv[v]);
      refresh();
      
      return true;
    }
  }

  return false;
}

bool parse_mult(char ** vn, double * vv, int  V, char ** segs, int L) {

  if (L != 3 || !seq(segs[0], "mult")) { return false; }

  //.. parse value
  double val;
  
  if (!parse_lf(segs[2], vn, vv, V, &val)) { return false; }
  
  //.. find existing variable
  for (int v = 0 ; v < V ; v++) {

    if (seq(segs[1], vn[v])) {

      vv[v] *= val;

      printw("> %s = %lf\n", vn[v], vv[v]);
      refresh();
      
      return true;
    }
  }

  return false;  
}

bool parse_clear_var(char *** vn, double ** vv, int * V, char ** segs, int L) {

  if (L != 1 || !seq(segs[0], "clear_vars")) { return false; }
  
  for (int v = 0 ; v < *V ; v++) {

    free((*vn)[v]);
  }

  *V = 0;
  *vn = realloc(*vn, sizeof(char *));
  *vv = realloc(*vv, sizeof(double));
  
  return true;
}

char ** parse_exe(char ** segs, int L, int * A) {
  
  if (L != 2 || !seq(segs[0], "exe")) { return NULL; }

  //.. check for file
  struct stat f_stat;

  if (stat(segs[1], &f_stat) != 0) { return NULL; }
  if (!S_ISREG(f_stat.st_mode))    { return NULL; }
  
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

Action ** parse_goto(CNC * cnc, char ** segs, int L, char ** vn, double * vv, int V) {

  if (L < 4 || L > 5 || !seq(segs[0], "goto")) { return NULL; }
  
  double X, Y, Z, F;
  
  if (!parse_lf(segs[1], vn, vv, V, &X) || !parse_lf(segs[2], vn, vv, V, &Y) || !parse_lf(segs[3], vn, vv, V, &Z)) { return false; }

  X = v2r(X, cnc -> unit);
  Y = v2r(Y, cnc -> unit);
  Z = v2r(Z, cnc -> unit);
  
  if (L == 5) {

    if (!parse_lf(segs[4], vn, vv, V, &F)) { return false; }
    if (lfeq(F, 0))                        { return false; }
      
    F = v2r(F, cnc -> unit);
    
  } else if (L == 4) { F = F_max(fabs(X - cnc -> X), fabs(Y - cnc -> Y), fabs(Z - cnc -> Z)); }
  
  Action ** s = malloc(sizeof(Action *));
  s[0] = create_linear(X, Y, Z, F);
  
  return s;
}

Action ** parse_delta(CNC * cnc, char ** segs, int L, char ** vn, double * vv, int V) {

  if (L < 4 || L > 5 || !seq(segs[0], "delta")) { return NULL; }

  double dx, dy, dz, F;

  if (!parse_lf(segs[1], vn, vv, V, &dx) || !parse_lf(segs[2], vn, vv, V, &dy) || !parse_lf(segs[3], vn, vv, V, &dz)) { return false; }

  dx = v2r(dx, cnc -> unit);
  dy = v2r(dy, cnc -> unit);
  dz = v2r(dz, cnc -> unit);
  
  if (L == 5) {

    if (!parse_lf(segs[4], vn, vv, V, &F)) { return false; }
    if (lfeq(F, 0))                        { return false; }

    F = v2r(F, cnc -> unit);

  } else if (L == 4) { F = F_max(dx, dy, dz); }

  Action ** s = malloc(sizeof(Action *));
  s[0] = create_linear(cnc -> X + dx, cnc -> Y + dy, cnc -> Z + dz, F);

  return s;
}

Action ** parse_curve(CNC * cnc, char ** segs, int L, char ** vn, double * vv, int V) {

  if ((L != 7 && L != 8) || !seq(segs[0], "curve")) { return NULL; }

  double x, y, z, x0, y0, F;

  enum DirType dir = DIR_None;

  if      (seq(segs[1], "cw"))  { dir = DIR_CW;  }
  else if (seq(segs[1], "ccw")) { dir = DIR_CCW; }
  else                          { return NULL;   }

  if (!parse_lf(segs[2], vn, vv, V, &x)  ||
      !parse_lf(segs[3], vn, vv, V, &y)  ||
      !parse_lf(segs[4], vn, vv, V, &z)  ||
      !parse_lf(segs[5], vn, vv, V, &x0) ||
      !parse_lf(segs[6], vn, vv, V, &y0)) { return NULL; }
  
  x  = v2r(x, cnc -> unit);
  y  = v2r(y, cnc -> unit);
  z  = v2r(z, cnc -> unit);
  x0 = v2r(x0, cnc -> unit);
  y0 = v2r(y0, cnc -> unit);

  double dx = fabs(x - cnc -> X);
  double dy = fabs(y - cnc -> Y);
  double dz = fabs(z - cnc -> Z);
  
  if (L == 8 && parse_lf(segs[7], vn, vv, V, &F)) {
  
    F = v2r(F, cnc -> unit);  
  }
  else if (L == 7) { F = F_max(dx, dy, dz); }
  else             { return NULL; }
  
  Action ** s = malloc(sizeof(Action *));
  s[0] = create_curve(x, y, z, x0, y0, dir, F);

  return s;
}

Action ** parse_square_pocket(CNC * cnc, char ** segs, int L, char ** vn, double * vv, int V, int * A) {

  if ((L != 4 && L != 6) || !seq(segs[0], "square_pocket")) { return NULL; }

  double lxi, lyi, lxo, lyo, h;

  lxi = lyi = 0;
  
  if ((L == 6 && !parse_lf(segs[1], vn, vv, V, &lxi)) ||
      (L == 6 && !parse_lf(segs[2], vn, vv, V, &lyi)) ||
      !parse_lf(segs[L-3], vn, vv, V, &lxo) ||
      !parse_lf(segs[L-2], vn, vv, V, &lyo) ||
      !parse_lf(segs[L-1], vn, vv, V, &h)) { return NULL; }

  lxi = v2r(lxi, cnc -> unit);
  lyi = v2r(lyi, cnc -> unit);
  lxo = v2r(lxo, cnc -> unit);
  lyo = v2r(lyo, cnc -> unit);
  h   = v2r(h,   cnc -> unit);
  
  if (lxo < 0 || lyo < 0 || lxi < 0 || lyi < 0 || h < 0)                    { return NULL; }
  if ((lxo - lxi) / 2 < cnc -> tool.Dc || (lyo - lyi) / 2 < cnc -> tool.Dc) { return NULL; } 

  Action ** s;
  
  if (L == 4) { s =        square_pocket(cnc,           lxo, lyo, h, A); }
  if (L == 6) { s = nested_square_pocket(cnc, lxi, lyi, lxo, lyo, h, A); }

  return s;
}

Action ** parse_drill(CNC * cnc, char ** segs, int L, char ** vn, double * vv, int V, int * A) {

  if (L != 2 || !seq(segs[0], "drill")) { return NULL; }

  double h;

  if (!parse_lf(segs[1], vn, vv, V, &h)) { return NULL; }

  h = v2r(h, cnc -> unit);

  if (h <= 0) { return NULL; }
  
  return drill(cnc, h, A);
}

Action ** parse_bore(CNC * cnc, char ** segs, int L, char ** vn, double * vv, int V, int * A) {

  if (L != 3 || !seq(segs[0], "bore")) { return NULL; }

  double r, h;
  
  if (!parse_lf(segs[1], vn, vv, V, &r) ||
      !parse_lf(segs[2], vn, vv, V, &h)) { return NULL; }

   r = v2r(r, cnc -> unit);
   h = v2r(h, cnc -> unit);

   if (h <= 0)                  { return NULL; }
   if (r <= cnc -> tool.Dc / 2) { return NULL; }
   
   return bore(cnc, r, h, A);
}

Action ** parse_circular_pocket(CNC * cnc, char ** segs, int L, char ** vn, double * vv, int V, int * A) {

  if (L != 4 || !seq(segs[0], "circular_pocket")) { return NULL; }

  double ri, ro, h;

  if (!parse_lf(segs[1], vn, vv, V, &ri) ||
      !parse_lf(segs[2], vn, vv, V, &ro) ||
      !parse_lf(segs[3], vn, vv, V, &h)) { return NULL; }

  ri = v2r(ri, cnc -> unit);
  ro = v2r(ro, cnc -> unit);
  h  = v2r(h,  cnc -> unit);

  if (ri < 0 || ro <= 0)          { return NULL; }
  if (lfeq(ri, 0))                { ri = -cnc -> tool.Dc / 2; }
  if ((ro - ri) < cnc -> tool.Dc) { return NULL; }

  return circular_pocket(cnc, ri, ro, h, A);
}

Action ** parse_engrave(CNC * cnc, char ** segs, int L, char ** vn, double * vv, int V, int * A) {

  if (L != 4 || !seq(segs[0], "engrave")) { return NULL; }

  double h, dz, a;
  
  if (!parse_lf(segs[1], vn, vv, V, &h) ||
      !parse_lf(segs[2], vn, vv, V, &dz) ||
      !parse_lf(segs[3], vn, vv, V, &a)) { return NULL; }

   h  = v2r(h, cnc -> unit);
   a  = a * M_PI / 180;

   if (h <= 0) { return NULL; }
   
   char * text = malloc(sizeof(char) * strlen(segs[1]));
   strcpy(text, segs[1]);
   
   return engrave_text(cnc, text, h, a, A);
}

Action ** parse_radial_contour(CNC * cnc, char ** segs, int L, char ** vn, double * vv, int V, int * A) {

  //.. check parameters
  if (L != 4 || !seq(segs[0], "radial_contour")) { return NULL; }

  double off, h;

  if (!parse_lf(segs[2], vn, vv, V, &off) ||
      !parse_lf(segs[3], vn, vv, V, &h)) { return NULL; }

  off = v2r(off, cnc -> unit);
  h   = v2r(h,   cnc -> unit);

  if (h   <= 0) { return NULL; }
  if (off <  0) { return NULL; }

  //.. read profile
  double * x, * y, * x0, * y0;
  enum DirType * d;
  int P;

  if (!parse_sub(segs[1], &x, &y, &x0, &y0, &d, &P, cnc -> unit)) { return NULL; }
  
  //.. generate cam
  Action ** S = radial_contour(cnc, x, y, x0, y0, d, off, h, P, A);

  free(x);
  free(y);
  free(x0);
  free(y0);
  free(d);

  return S;
}

Action ** parse_side_contour(CNC * cnc, char ** segs, int L, char ** vn, double * vv, int V, int * A) {

  //.. check parameters
  if (L != 5 || !seq(segs[0], "side_contour")) { return NULL; }

  double ox, oy, h;

  if (!parse_lf(segs[2], vn, vv, V, &ox) ||
      !parse_lf(segs[3], vn, vv, V, &oy) ||
      !parse_lf(segs[4], vn, vv, V, &h)) { return NULL; }

  ox = v2r(ox, cnc -> unit);
  oy = v2r(oy, cnc -> unit);
  h  = v2r(h,  cnc -> unit);

  if (h <= 0) { return NULL; }

  //.. read profile
  double * x, * y, * x0, * y0;
  enum DirType * d;
  int P;

  if (!parse_sub(segs[1], &x, &y, &x0, &y0, &d, &P, cnc -> unit)) { return NULL; }
  
  //.. generate cam
  Action ** S = side_contour(cnc, x, y, x0, y0, d, ox, oy, h, P, A);

  free(x);
  free(y);
  free(x0);
  free(y0);
  free(d);

  return S;
}

bool parse_sub(char * path, double ** X, double ** Y, double ** X0, double ** Y0, enum DirType ** D, int * P, enum UnitType unit) {

  //.. check for file
  struct stat f_stat;

  if (stat(path, &f_stat) != 0) { return false; }
  if (!S_ISREG(f_stat.st_mode)) { return false; }

  //.. opening file
  FILE * file = fopen(path, "r");
  
  if (file == NULL) { return false; }

  //.. reading lines
  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  rewind(file);

  char * text = malloc(fsize + 1);
  fread(text, fsize, 1, file);
  text[fsize] = 0;

  fclose(file);

  char ** lines = split_line(text, ';', P);

  *X  = malloc(sizeof(double) * (*P));
  *Y  = malloc(sizeof(double) * (*P));
  *X0 = malloc(sizeof(double) * (*P));
  *Y0 = malloc(sizeof(double) * (*P));
  *D  = malloc(sizeof(enum DirType) * (*P));
  
  //.. parse lines
  int A;

  double x, y, x0, y0;
 
  for (int l = 0 ; l < *P ; l++) {

    char ** segs = split_line(lines[l], ' ', &A);

    if (A == 3 && seq(segs[0], "|") && parse_lf(segs[1], NULL, NULL, 0, &x) && parse_lf(segs[2], NULL, NULL, 0, &y)) {

      (*X)[l]  = v2r(x, unit);
      (*Y)[l]  = v2r(y, unit);
      (*X0)[l] = 0;
      (*Y0)[l] = 0;
      (*D)[l]  = DIR_None;
      
    } else if (A == 6 && seq(segs[0], "o") && l != 0 && parse_lf(segs[1], NULL, NULL, 0, &x) && parse_lf(segs[2], NULL, NULL, 0, &y) && parse_lf(segs[3], NULL, NULL, 0, &x0) && parse_lf(segs[4], NULL, NULL, 0, &y0) && (seq(segs[5], "cw") || seq(segs[5], "ccw"))) {
      
      (*X)[l]  = v2r(x, unit);
      (*Y)[l]  = v2r(y, unit);
      (*X0)[l] = v2r(x0, unit);
      (*Y0)[l] = v2r(y0, unit);

      if (seq(segs[5], "cw")) { (*D)[l] = DIR_CW;  }
      else                    { (*D)[l] = DIR_CCW; }

    } else {

      free(*X);
      free(*Y);
      free(*X0);
      free(*Y0);
      free(*D);

      return false;
    }
  }
  
  //.. check for close loop
  if (*P < 3 || !lfeq((*X)[0], (*X)[*P - 1]) || !lfeq((*Y)[0], (*Y)[*P - 1])) {

    free(*X);
    free(*Y);
    free(*X0);
    free(*Y0);
    free(*D);

    return false;
  }
  
  return true;
}
