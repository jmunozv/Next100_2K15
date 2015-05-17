#ifndef _blobsCharacterizer__
#define _blobsCharacterizer__

#include <GATE/Centella.h>

class blobsCharacterizer : public gate::IAlgo {

public:
  
  //! default contructor
  blobsCharacterizer(gate::VLEVEL=gate::NORMAL,
	       std::string label="blobsCharacterizerInstance");
  
  //! constructor with store with input parameters 
  blobsCharacterizer(const gate::ParamStore& gs,
	       gate::VLEVEL=gate::NORMAL,
	       std::string label="blobsCharacterizerInstance");
  
  //! destructor
  virtual ~blobsCharacterizer(){};
  
  //! initialize algorithm
  bool initialize();        
  
  //! execute algorithm: process current event
  bool execute(gate::Event& evt);  
  
  //! finalize algorithm
  bool finalize();          
  
private:

  // Blob Radius
  double _blobRadius;

  // Method that calculates the distance between 2 hits, along the track
  double inTrackDistance(gate::BTrack* track, gate::BHit* hit1, gate::BHit* hit2);
 
  ClassDef(blobsCharacterizer,0)
    
};

#endif
