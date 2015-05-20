#ifndef _extremesCharacterizer__
#define _extremesCharacterizer__

#include <GATE/Centella.h>

class extremesCharacterizer : public gate::IAlgo {

 public:
  
  //! default contructor
  extremesCharacterizer(gate::VLEVEL=gate::NORMAL,
	       std::string label="extremesCharacterizerInstance");
  
  //! constructor with store with input parameters 
  extremesCharacterizer(const gate::ParamStore& gs,
	       gate::VLEVEL=gate::NORMAL,
	       std::string label="extremesCharacterizerInstance");
  
  //! destructor
  virtual ~extremesCharacterizer(){};
  
  //! initialize algorithm
  bool initialize();        
  
  //! execute algorithm: process current event
  bool execute(gate::Event& evt);  
  
  //! finalize algorithm
  bool finalize();          
  
 private:

  // Method that calculates the distance between 2 hits, along the track
  // double inTrackDistance(gate::BTrack* track, gate::BHit* hit1, gate::BHit* hit2);

  
  ClassDef(extremesCharacterizer,0)
    
};

#endif
