// DEFINITIONS

#define DPATH "cnc-state"
#define PPATH "cnc-state/state.position"
#define TPATH "cnc-state/state.tool"
#define MPATH "cnc-state/state.material"
#define CPATH "cnc-state/state.config"

#define CMDLEN 1024

// FUNCTIONS

char *                read_line(void);
char **              split_line(char *, char, int *);

//.. config read files
void                read_config(CNC *);
void              read_position(CNC *);
void              read_material(CNC *);
void                  read_tool(CNC *);

//.. data types
bool                   parse_lf(char *, char **, double *, int, double *);

//.. enums
char *            tooltype_name(enum ToolType);
char *            unittype_name(enum UnitType);

int              parse_tooltype(char *);
int                  parse_unit(char *);

//.. execution
bool              parse_comment(char **, int);
bool                  parse_var(char ***, double **, int *, char **, int);
bool                  parse_div(char **, double *, int, char **, int);
bool                 parse_mult(char **, double *, int, char **, int);
bool            parse_clear_var(char ***, double **, int *, char **, int);

bool                 parse_hold(char **, int);
bool                 parse_cont(char **, int);
bool                 parse_stop(char **, int);
uint32_t             parse_skip(char **, int, char **, double *, int);
bool                 parse_exit(char **, int);
char **               parse_exe(char **, int, int *);

bool                  parse_get(CNC *, char **, int);
bool                  parse_set(CNC *, char **, int, char **, double *, int);

//.. machine control
char **        parse_toolchange(char **, int, int *);

//.. level 0 cam
Action **            parse_goto(CNC *, char **, int, char **, double *, int);
Action **           parse_delta(CNC *, char **, int, char **, double *, int);
Action **           parse_curve(CNC *, char **, int, char **, double *, int);

//.. level 1 cam
Action **   parse_square_pocket(CNC *, char **, int, char **, double *, int, int *);
Action **           parse_drill(CNC *, char **, int, char **, double *, int, int *);
Action **            parse_bore(CNC *, char **, int, char **, double *, int, int *);
Action ** parse_circular_pocket(CNC *, char **, int, char **, double *, int, int *);
Action **         parse_engrave(CNC *, char **, int, char **, double *, int, int *);

//.. level 2 cam
Action **  parse_radial_contour(CNC *, char **, int, char **, double *, int, int *);
Action **    parse_side_contour(CNC *, char **, int, char **, double *, int, int *);
bool                  parse_sub(char *, double **, double **, double **, double **, enum DirType **, int *, enum UnitType);
