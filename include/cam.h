// DEFINITIONS

#define Z_OFF 0.25 //.. [in]

// FUNCTIONS

//.. Level 1
Action **        square_pocket(CNC *, double, double, double, int *);
Action ** nested_square_pocket(CNC *, double, double, double, double, double, int *);
Action **                drill(CNC *, double, int *);
Action **                 bore(CNC *, double, double, int *);
Action **      circular_pocket(CNC *, double, double, double, int *);

Action **         engrave_text(CNC *, char *, double, double, int *);
void                 engrave_A(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_B(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_C(double, double, double, double, double, double, double, double, Action ***, int *);

//.. Level 2
Action **       radial_contour(CNC *, double *, double *, double *, double *, enum DirType *, double, double, int, int *);
void            offset_profile(double *, double *, double *, double *, int, double);
void                profile_cm(double *, double *, int P, double *, double *);
Action **         side_contour(CNC *, double *, double *, double *, double *, enum DirType *, double, double, double, int, int *);
