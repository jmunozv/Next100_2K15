#ifndef _roadCharacterizer__
#define _roadCharacterizer__

#include <GATE/Centella.h>

class roadCharacterizer : public gate::IAlgo {

 public:
  
  //! default contructor
  roadCharacterizer(gate::VLEVEL=gate::NORMAL,
	       std::string label="roadCharacterizerInstance");
  
  //! constructor with store with input parameters 
  roadCharacterizer(const gate::ParamStore& gs,
	       gate::VLEVEL=gate::NORMAL,
	       std::string label="roadCharacterizerInstance");
  
  //! destructor
  virtual ~roadCharacterizer(){};
  
  //! initialize algorithm
  bool initialize();        
  
  //! execute algorithm: process current event
  bool execute(gate::Event& evt);  
  
  //! finalize algorithm
  bool finalize();          
  
 private:
  
  ClassDef(roadCharacterizer,0)
    
};

#endif
