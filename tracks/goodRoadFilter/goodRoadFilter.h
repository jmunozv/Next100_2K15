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

  // Counters
  int _numInputEvents;
  int _numOutputEvents;

  // Maximum Number of Tracks
  int _maxNumTracks;

  // Minimum & Maximum Energy of Hottest Track
  double _minEnergy = 0;
  double _maxEnergy = 0;

  // Method that fills & store hits distances to track extremes
  void fillHitsDistances(gate::BTrack* track);

  // Method that calculates the distance between 2 hits, along the track
  // DEPRECATED !!
  //double inTrackDistance(gate::BTrack* track, gate::BHit* hit1, gate::BHit* hit2);

  ClassDef(goodRoadFilter,0)
    
};

#endif
