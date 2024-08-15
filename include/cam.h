// DEFINITIONS

#define Z_OFF 0.25 //.. [in]

// FUNCTIONS

Action **                 face(double, double, double, double, double, double, double, double, double, double, double, int *);
Action **        square_pocket(double, double, double, double, double, double, double, double, double, double, double, int *);
Action ** nested_square_pocket(double, double, double, double, double, double, double, double, double, double, double, double, double, int *);
Action **      circular_pocket(double, double, double, double, double, double, double, double, double, double, double, int *);
Action **               cutout(double, double, double, double, double, double, double, double, double, double, double, double, double, double, int *);
Action **                 bore(double, double, double, double, double, double, double, double, int *);
Action **                drill(double, double, double, double, double, int *);
Action **               fillet(double, double, double, double, double, double, double, double, double, double, int *);
Action **               groove(double, double, double, double, double, double *, double *, int, double, double, int *);

Action **         engrave_text(double, double, char *, double, double, double, double, double, double, int *);
void                 engrave_A(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_B(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_C(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_D(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_E(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_F(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_G(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_H(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_I(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_J(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_K(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_L(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_M(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_N(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_O(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_P(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_Q(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_R(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_S(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_T(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_U(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_V(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_W(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_X(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_Y(double, double, double, double, double, double, double, double, Action ***, int *);
void                 engrave_Z(double, double, double, double, double, double, double, double, Action ***, int *);
