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
 
  // Blob Energy range to be studied
  double _blobMinE;
  double _blobMaxE;

  // Blob Energy Threshold before looking number of voxels
  double _voxelsEth;

  ClassDef(blobsCharacterizer,0)
    
};

#endif
