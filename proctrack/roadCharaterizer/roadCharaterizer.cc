
#include<roadCharaterizer.h>

ClassImp(roadCharaterizer)

//==========================================================================
roadCharaterizer::roadCharaterizer(gate::VLEVEL vl, std::string label) : 
IAlgo(vl,"roadCharaterizer",0,label){
//==========================================================================


}

//==========================================================================
roadCharaterizer::roadCharaterizer(const gate::ParamStore& gs, 
			   gate::VLEVEL vl, std::string label) :
  IAlgo(gs,vl,"roadCharaterizer",0,label){
//==========================================================================


}

//==========================================================================
bool roadCharaterizer::initialize(){
//==========================================================================

  _m.message("Intializing algorithm",this->getAlgoLabel(),gate::NORMAL);
  
  gate::Centella::instance()
    ->hman()->h1(this->alabel("EvtID"),"EvtID",10,0,100);

  return true;

}

//==========================================================================
bool roadCharaterizer::execute(gate::Event& evt){
//==========================================================================

  _m.message("Executing algorithm",this->getAlgoLabel(),gate::VERBOSE);
  
  _m.message("Event number:",evt.GetEventID(),gate::VERBOSE);
  
  gate::Centella::instance()
    ->hman()->fill(this->alabel("EvtID"),evt.GetEventID());
  
  return true;

}

//==========================================================================
bool roadCharaterizer::finalize(){
//==========================================================================

  _m.message("Finalising algorithm",this->getAlgoLabel(),gate::NORMAL);
  
  return true;

}
