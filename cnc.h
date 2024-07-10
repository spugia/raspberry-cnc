// ENUMERATORS

enum ToolType { NONE = 0, FLATMILL = 1, BALLMILL = 2, CHAMFER = 3, DRILL = 4 };
enum UnitType { IN = 0, MM = 1 };

// STRUCTURES

typedef struct TOOL {

  enum ToolType type;
  
  double Dc; //.. mill diameter [in]
  double Ds; //.. shank diameter [in]
  double Lc; //.. tool cutting length [in]
  double Lt; //.. total tool length [in]
  
} TOOL;

typedef struct MAT {

  double Fxy; //.. translation feed rate [in/min]
  double Fz;  //.. plunge feed rate [in/min]

  double dz;  //.. step down
  double dr;  //.. step over

} MAT;

typedef struct CNC {

  FILE * fpos;      //.. position file
  FILE * ftool;     //.. tool file
  
  double X;         //.. spindle X position [in]
  double Y;         //.. spindle Y position [in]
  double Z;         //.. spindle Z position [in]

  enum UnitType unit;
  
  struct MAT mat;   //.. material settings
  struct TOOL tool; //.. loaded tool settings
  
} CNC;

// FUNCTIONS
void       initialize(CNC *); 
void          cleanup(CNC *);

void   write_position(CNC *);
void   write_material(CNC *);
void       write_tool(CNC *);
void     write_config(CNC *);
