#ifndef _roadWidthFilter__
#define _roadWidthFilter__

#include <GATE/Centella.h>

class roadWidthFilter : public gate::IAlgo {

 public:
  
  //! default contructor
  roadWidthFilter(gate::VLEVEL=gate::NORMAL,
	       std::string label="roadWidthFilterInstance");
  
  //! constructor with store with input parameters 
  roadWidthFilter(const gate::ParamStore& gs,
	       gate::VLEVEL=gate::NORMAL,
	       std::string label="roadWidthFilterInstance");
  
  //! destructor
  virtual ~roadWidthFilter(){};
  
  //! initialize algorithm
  bool initialize();        
  
  //! execute algorithm: process current event
  bool execute(gate::Event& evt);  
  
  //! finalize algorithm
  bool finalize();          
  
 private:

  // Counters
  int _numInputEvents;
  int _numOutputEvents;

  // Road Width
  double _roadWidth;

  ClassDef(roadWidthFilter,0)
    
};

#endif
