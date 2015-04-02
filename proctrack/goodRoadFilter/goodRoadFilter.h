#ifndef _goodRoadFilter__
#define _goodRoadFilter__

#include <GATE/Centella.h>

class goodRoadFilter : public gate::IAlgo {

 public:
  
  //! default contructor
  goodRoadFilter(gate::VLEVEL=gate::NORMAL,
	       std::string label="goodRoadFilterInstance");
  
  //! constructor with store with input parameters 
  goodRoadFilter(const gate::ParamStore& gs,
	       gate::VLEVEL=gate::NORMAL,
	       std::string label="goodRoadFilterInstance");
  
  //! destructor
  virtual ~goodRoadFilter(){};
  
  //! initialize algorithm
  bool initialize();        
  
  //! execute algorithm: process current event
  bool execute(gate::Event& evt);  
  
  //! finalize algorithm
  bool finalize();          
  
 private:
  
  ClassDef(goodRoadFilter,0)
    
};

#endif
