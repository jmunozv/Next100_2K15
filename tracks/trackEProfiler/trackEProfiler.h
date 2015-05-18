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

  // Method that calculates the distance between 2 hits, along the track
  double inTrackDistance(gate::BTrack* track, gate::BHit* hit1, gate::BHit* hit2);
  
  ClassDef(trackEProfiler,0)
    
};

#endif
