#ifndef _simpleBlobFilter__
#define _simpleBlobFilter__

#include <GATE/Centella.h>

class simpleBlobFilter : public gate::IAlgo {

 public:
  
  //! default contructor
  simpleBlobFilter(gate::VLEVEL=gate::NORMAL,
	       std::string label="simpleBlobFilterInstance");
  
  //! constructor with store with input parameters 
  simpleBlobFilter(const gate::ParamStore& gs,
	       gate::VLEVEL=gate::NORMAL,
	       std::string label="simpleBlobFilterInstance");
  
  //! destructor
  virtual ~simpleBlobFilter(){};
  
  //! initialize algorithm
  bool initialize();        
  
  //! execute algorithm: process current event
  bool execute(gate::Event& evt);  
  
  //! finalize algorithm
  bool finalize();          
  
 private:
  
  ClassDef(simpleBlobFilter,0)
    
};

#endif
