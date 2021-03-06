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

  // Counters
  int _numInputEvents;
  int _numInterEvents;
  int _numOutputEvents;

  // Blobs stuff
  double _blobRadius;
  double _blobMinE;
  int    _minVoxels;
    
  ClassDef(simpleBlobFilter,0)
    
};

#endif
