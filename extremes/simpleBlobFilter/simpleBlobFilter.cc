
#include<simpleBlobFilter.h>

ClassImp(simpleBlobFilter)

//==========================================================================
simpleBlobFilter::simpleBlobFilter(gate::VLEVEL vl, std::string label) : 
IAlgo(vl,"simpleBlobFilter",0,label){
//==========================================================================


}

//==========================================================================
simpleBlobFilter::simpleBlobFilter(const gate::ParamStore& gs, 
			   gate::VLEVEL vl, std::string label) :
  IAlgo(gs,vl,"simpleBlobFilter",0,label){
//==========================================================================


}

//==========================================================================
bool simpleBlobFilter::initialize(){
//==========================================================================

  _m.message("Intializing algorithm",this->getAlgoLabel(),gate::NORMAL);
  
  gate::Centella::instance()
    ->hman()->h1(this->alabel("EvtID"),"EvtID",10,0,100);

  return true;

}

//==========================================================================
bool simpleBlobFilter::execute(gate::Event& evt){
//==========================================================================

  _m.message("Executing algorithm",this->getAlgoLabel(),gate::VERBOSE);
  
  _m.message("Event number:",evt.GetEventID(),gate::VERBOSE);
  
  gate::Centella::instance()
    ->hman()->fill(this->alabel("EvtID"),evt.GetEventID());
  
  return true;

}

//==========================================================================
bool simpleBlobFilter::finalize(){
//==========================================================================

  _m.message("Finalising algorithm",this->getAlgoLabel(),gate::NORMAL);
  
  return true;

}
