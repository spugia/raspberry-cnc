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

void                read_config(CNC *);
void              read_position(CNC *);
void              read_material(CNC *);
void                  read_tool(CNC *);

char *                tool_name(enum ToolType);

int                  parse_tool(char *);
int                  parse_unit(char *);

bool                 parse_hold(char **, int);
bool                 parse_cont(char **, int);
bool                 parse_stop(char **, int);
bool                 parse_skip(char **, int);
bool                 parse_exit(char **, int);

bool                  parse_get(CNC *, char **, int);
bool                  parse_set(CNC *, char **, int);
bool              parse_comment(CNC *, char **, int);

int               parse_spindle(CNC *, char **, int);
char **               parse_exe(CNC *, char **, int, int *);

Action **            parse_goto(CNC *, char **, int);
Action **           parse_delta(CNC *, char **, int);
Action **            parse_face(CNC *, char **, int, int *);
Action **   parse_square_pocket(CNC *, char **, int, int *);
Action ** parse_circular_pocket(CNC *, char **, int, int *);
Action **          parse_cutout(CNC *, char **, int, int *);
