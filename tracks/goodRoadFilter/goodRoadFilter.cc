
#include<goodRoadFilter.h>

ClassImp(goodRoadFilter)

//==========================================================================
goodRoadFilter::goodRoadFilter(gate::VLEVEL vl, std::string label) : 
IAlgo(vl,"goodRoadFilter",0,label) {
  _m.message("Constructor()", gate::NORMAL);
}



//==========================================================================
goodRoadFilter::goodRoadFilter(const gate::ParamStore& gs, 
			   gate::VLEVEL vl, std::string label) :
  IAlgo(gs,vl,"goodRoadFilter",0,label) {

  _m.message("Constructor()", gate::NORMAL);

  // Getting the parameters
  _minEnergy = gs.fetch_dstore("minEnergy");
  _m.message("Minimum Energy of Hottest Track:", _minEnergy/gate::MeV, "MeV", gate::NORMAL);
}



//==========================================================================
bool goodRoadFilter::initialize() {

  _m.message("Initialize()", gate::NORMAL);
  
  /// Histograms
  // Histograms with Event Energy After Filter
  gate::Centella::instance()->hman()->h1(this->alabel("evtEdepAfter"),
                                         "Event Energy Dep. After Filter", 100, 2.4, 2.5);


  /// Counters
  _numInputEvents  = 0;
  _numOutputEvents = 0;

  return true;
}



//==========================================================================
bool goodRoadFilter::execute(gate::Event& evt) {

  _m.message("Execute()", gate::DETAILED);

  _numInputEvents += 1;

  // Getting Tracks
  std::vector<gate::Track*> tracks = evt.GetTracks();
  
  // Getting the Hottest Track
  double maxEdep = 0.;
  gate::Track* hTrack;
  for (auto track: tracks) {
    double eDep = track->GetEnergy();
    if (eDep > maxEdep) {
      maxEdep = eDep;
      hTrack = track;
    }
  }
  int hTrackID = hTrack->GetID();

  if (maxEdep >= _minEnergy) {
    _m.message("Filter Passed. Track ID:", hTrackID, " with Edep:", maxEdep, gate::DETAILED);
    _numOutputEvents += 1;
    gate::Centella::instance()->hman()->fill(this->alabel("evtEdepAfter"), evt.GetEnergy());
    return true;      
  }

  else {
    _m.message("Filter Failed. Track ID:", hTrackID, " with Edep:", maxEdep, gate::DETAILED);
    return false;      
  }

  return true;
}



//==========================================================================
bool goodRoadFilter::finalize() {

  _m.message("Finalize()", gate::NORMAL);

  _m.message("Input  Events:", _numInputEvents, gate::NORMAL);
  _m.message("Output Events:", _numOutputEvents, gate::NORMAL);
  
  gate::Centella::instance()->logman().getLogs("USER").store(this->alabel("InputEvents"), _numInputEvents);
  gate::Centella::instance()->logman().getLogs("USER").store(this->alabel("OutputEvents"), _numOutputEvents);

  return true;
}
