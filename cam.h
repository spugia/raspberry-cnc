// FUNCTIONS

Action **                 face(double, double, double, double, double, double, double, double, double, double, double, int *);
Action **        square_pocket(double, double, double, double, double, double, double, double, double, double, double, int *);
Action ** nested_square_pocket(double, double, double, double, double, double, double, double, double, double, double, double, double, int *);
Action **      circular_pocket(double, double, double, double, double, double, double, double, double, double, double, int *);
Action **               cutout(double, double, double, double, double, double, double, double, double, double, double, double, double, double, int *);
Action **                 bore(double, double, double, double, double, double, double, double, int *);
Action **                drill(double, double, double, double, double, int *);
Action **               fillet(double, double, double, double, double, double, double, double, double, double, int *);
Action **         engrave_text(double, double, char *, double, double, double, double, double, double, int *);
void                 engrave_A(double, double, Action ***, int *);
