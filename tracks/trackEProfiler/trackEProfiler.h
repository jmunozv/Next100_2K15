#ifndef _trackEProfiler__
#define _trackEProfiler__

#include <GATE/Centella.h>

class trackEProfiler : public gate::IAlgo {

 public:
  
  //! default contructor
  trackEProfiler(gate::VLEVEL=gate::NORMAL,
	       std::string label="trackEProfilerInstance");
  
  //! constructor with store with input parameters 
  trackEProfiler(const gate::ParamStore& gs,
	       gate::VLEVEL=gate::NORMAL,
	       std::string label="trackEProfilerInstance");
  
  //! destructor
  virtual ~trackEProfiler(){};
  
  //! initialize algorithm
  bool initialize();        
  
  //! execute algorithm: process current event
  bool execute(gate::Event& evt);  
  
  //! finalize algorithm
  bool finalize();          
  
 private:

  double _blobRadius;

  ClassDef(trackEProfiler,0)
    
};

#endif
