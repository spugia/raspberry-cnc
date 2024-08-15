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
bool                   parse_lf(char *, double *);

//.. enums
char *            tooltype_name(enum ToolType);
char *            unittype_name(enum UnitType);

int              parse_tooltype(char *);
int                  parse_unit(char *);

//.. execution
bool              parse_comment(CNC *, char **, int);

bool                 parse_hold(char **, int);
bool                 parse_cont(char **, int);
bool                 parse_stop(char **, int);
bool                 parse_skip(char **, int);
bool                 parse_exit(char **, int);
char **               parse_exe(CNC *, char **, int, int *);

bool                  parse_get(CNC *, char **, int);
bool                  parse_set(CNC *, char **, int);

//.. machine control
int               parse_spindle(CNC *, char **, int);
char **        parse_toolchange(CNC *, char **, int, int *);

//.. cam
Action **            parse_goto(CNC *, char **, int);
Action **           parse_delta(CNC *, char **, int);
Action **            parse_face(CNC *, char **, int, int *);
Action **   parse_square_pocket(CNC *, char **, int, int *);
Action ** parse_circular_pocket(CNC *, char **, int, int *);
Action **          parse_cutout(CNC *, char **, int, int *);
Action **           parse_drill(CNC *, char **, int, int *);
Action **            parse_bore(CNC *, char **, int, int *);
Action **          parse_fillet(CNC *, char **, int, int *);
Action **           parse_grove(CNC *, char **, int, int *);
Action **         parse_engrave(CNC *, char **, int, int *);
