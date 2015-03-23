#ifndef _roadWidthStudy__
#define _roadWidthStudy__

#include <GATE/Centella.h>

class roadWidthStudy : public gate::IAlgo {

 public:
  
  //! default contructor
  roadWidthStudy(gate::VLEVEL=gate::NORMAL,
	       std::string label="roadWidthStudyInstance");
  
  //! constructor with store with input parameters 
  roadWidthStudy(const gate::ParamStore& gs,
	       gate::VLEVEL=gate::NORMAL,
	       std::string label="roadWidthStudyInstance");
  
  //! destructor
  virtual ~roadWidthStudy(){};
  
  //! initialize algorithm
  bool initialize();        
  
  //! execute algorithm: process current event
  bool execute(gate::Event& evt);  
  
  //! finalize algorithm
  bool finalize();          
  
 private:
  
  ClassDef(roadWidthStudy,0)
    
};

#endif
