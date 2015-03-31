#ifndef _roadCharaterizer__
#define _roadCharaterizer__

#include <GATE/Centella.h>

class roadCharaterizer : public gate::IAlgo {

 public:
  
  //! default contructor
  roadCharaterizer(gate::VLEVEL=gate::NORMAL,
	       std::string label="roadCharaterizerInstance");
  
  //! constructor with store with input parameters 
  roadCharaterizer(const gate::ParamStore& gs,
	       gate::VLEVEL=gate::NORMAL,
	       std::string label="roadCharaterizerInstance");
  
  //! destructor
  virtual ~roadCharaterizer(){};
  
  //! initialize algorithm
  bool initialize();        
  
  //! execute algorithm: process current event
  bool execute(gate::Event& evt);  
  
  //! finalize algorithm
  bool finalize();          
  
 private:
  
  ClassDef(roadCharaterizer,0)
    
};

#endif
