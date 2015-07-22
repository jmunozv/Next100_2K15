#ifndef _trueBlobFilter__
#define _trueBlobFilter__

#include <GATE/Centella.h>

class trueBlobFilter : public gate::IAlgo {

 public:
  
  //! default contructor
  trueBlobFilter(gate::VLEVEL=gate::NORMAL,
	       std::string label="trueBlobFilterInstance");
  
  //! constructor with store with input parameters 
  trueBlobFilter(const gate::ParamStore& gs,
	       gate::VLEVEL=gate::NORMAL,
	       std::string label="trueBlobFilterInstance");
  
  //! destructor
  virtual ~trueBlobFilter(){};
  
  //! initialize algorithm
  bool initialize();        
  
  //! execute algorithm: process current event
  bool execute(gate::Event& evt);  
  
  //! finalize algorithm
  bool finalize();          
  
 private:
  
  ClassDef(trueBlobFilter,0)
    
};

#endif
