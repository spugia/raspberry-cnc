// DEFINITIONS

//.. pinout
#define PUL_X 25
#define PUL_Y 23
#define PUL_Z 15

#define DIR_X 24
#define DIR_Y 18
#define DIR_Z 14

#define STP_X 4
#define STP_Y 27
#define STP_Z 22

#define RELAY 26

//.. runout [in]
#define SLOP_X 0.0150F
#define SLOP_Y 0.0240F
#define SLOP_Z 0.0000F

//.. screw size [thread / in]
#define RPI_X 10.0F
#define RPI_Y 10.0F
#define RPI_Z 5.08F

//.. steps-per-revolution
#define SPR_X 800.0F
#define SPR_Y 800.0F
#define SPR_Z 800.0F

//.. feed acceleration [in/min/in]
#define FAR  300.0F
#define FMIN 1.0F
#define FRUN 2.0F

//.. NEMA23 pulsing frequency (limited by pi)
#define SPS_23 250000

// ENUMERATORS

enum ActionType { ACT_None = 0, Linear = 1, Curve = 2 };
enum    DirType { DIR_None = 0, DIR_CW = 1, DIR_CCW = 2 };

// STRUCTURES

typedef struct Action {

  enum ActionType type;
  enum    DirType dir;
  
  double X;  //.. [in]
  double Y;  //.. [in]
  double Z;  //.. [in]
  
  double X0; //.. [in]
  double Y0; //.. [in]
  
  double F;  //.. [in/min]

} Action;

typedef struct TARG {

  bool pause;
  bool stop;
  bool exit;

  CNC * cnc;

  double X;
  double Y;
  double Z;
  
  Action * action;

} TARG;

// FUNCTIONS

double                FX_max(void);
double                FY_max(void);
double                FZ_max(void);
double               FXY_max(void);
double                 F_max(double, double, double);
double               F_accel(double, double, double, double);

Action *       create_linear(double, double, double, double);
Action *        create_curve(double, double, double, double, double, enum DirType, double);

void          compile_linear(Action *, double, double, double, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, unsigned int **, unsigned int **, unsigned int **, unsigned int **, unsigned int **, unsigned int **, unsigned long long **, int *);
void           compile_curve(Action *, double, double, double, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, unsigned int **, unsigned int **, unsigned int **, unsigned int **, unsigned int **, unsigned int **, unsigned long long **, int *);
int **          render_curve(double, double, double, unsigned long long *);
void           render_points(int, int, double, double, int ***, unsigned long long *);
void            order_points(int ***, unsigned long long *, double, double, enum DirType);
bool          between_angles(double, double, double, enum DirType);
double            arc_length(double, double, double, enum DirType);

void             sort_action(unsigned long long *, unsigned int *, unsigned int *, unsigned int *, unsigned int *, unsigned int *, unsigned int *, int);
void           splice_action(unsigned long long *, struct timespec **, int);
void          execute_action(CNC *, struct timespec *, unsigned int *, unsigned int *, unsigned int *, unsigned int *, unsigned int *, unsigned int *, int, bool *, bool *);
void *        control_thread(void *);
bool        execute_sequence(TARG *, Action **, int);
