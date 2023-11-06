// ENUMERATORS

enum ToolType { NONE = 0, ENDMILL = 1 };
enum UnitType { IN   = 0, MM      = 1 };

// STRUCTURES

typedef struct TOOL {

  enum ToolType type;
  
  double d;  //.. tool diameter [in]
  double Lf; //.. tool flute length
  double Lt; //.. total tool length
  
} TOOL;

typedef struct MAT {

  double Fxy; //.. translation feed rate [in/min]
  double Fz;  //.. plunge feed rate [in/min]

  double dz;  //.. cutting depth
  double dr;  //.. radial step size

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

void     initialize(CNC *); 
void        cleanup(CNC *);

void write_position(CNC *);
void write_material(CNC *);
void     write_tool(CNC *);
void   write_config(CNC *);
