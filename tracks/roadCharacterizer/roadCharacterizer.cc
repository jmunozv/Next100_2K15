
#include<roadCharacterizer.h>

ClassImp(roadCharacterizer)

//==========================================================================
roadCharacterizer::roadCharacterizer(gate::VLEVEL vl, std::string label) : 
IAlgo(vl,"roadCharacterizer",0,label) {

  _m.message("Constructor()", gate::NORMAL);
}



//==========================================================================
roadCharacterizer::roadCharacterizer(const gate::ParamStore& gs, 
			   gate::VLEVEL vl, std::string label) :
  IAlgo(gs,vl,"roadCharacterizer",0,label) {

  _m.message("Constructor()", gate::NORMAL);

}



//==========================================================================
bool roadCharacterizer::initialize() {

  _m.message("Initialize()", gate::NORMAL);

  /// HISTOGRAMS
  // Histogram: Number of Tracks
  gate::Centella::instance()->hman()->h1(this->alabel("NumTracks"),
                                         "Number of Tracks", 10, 0, 10);

  // Histogram: Energy of Hottest Track
  gate::Centella::instance()->hman()->h1(this->alabel("EnergyHotTrk"),
                                         "Energy of Hottest Track", 75, 2.35, 2.5);

  // Histogram: Energy of Hottest Track
  gate::Centella::instance()->hman()->h1(this->alabel("EnergyNonHotTrk"),
                                         "Energy of Tracks (Non Hottest)", 30, 0.01, 0.04);

  // // Histogram: Main Path Voxels of Hottest Track
  // gate::Centella::instance()->hman()->h1(this->alabel("NumMPVoxelsHotTrk"),
  //                                        "Number of Main Path Voxels of Hottest Track",
  //                                        60, 0, 120);

  return true;
}



//==========================================================================
bool roadCharacterizer::execute(gate::Event& evt) {

  _m.message("Execute()", gate::DETAILED);

  // Getting Tracks
  std::vector<gate::Track*> tracks = evt.GetTracks();
  int numTracks = tracks.size();
  gate::Centella::instance()->hman()->fill(this->alabel("NumTracks"), numTracks);
  _m.message("Number of Tracks:", numTracks, gate::DETAILED);
  
  // Dealing with the Hottest Track
  double maxEdep = 0.;
  gate::Track* hTrack;
  for (auto track: tracks) {
    double eDep = track->GetEnergy();
    _m.message("Track", track->GetID(), " ->  Edep:", eDep, gate::VERBOSE);
    if (eDep > maxEdep) {
      maxEdep = eDep;
      hTrack = track;
    }
  }
  int hTrackID = hTrack->GetID();
  gate::Centella::instance()->hman()->fill(this->alabel("EnergyHotTrk"), maxEdep);
  _m.message("Hottest Track ID:", hTrackID, " -> Edep:", maxEdep, gate::DETAILED);

  // int numOrdHits = hTrack->fetch_ivstore("MainPathHits").size();
  // gate::Centella::instance()->hman()->fill(this->alabel("NumMPVoxelsHotTrk"), numOrdHits);
  // _m.message("Hottest Track ID:", hTrackID, " -> Num Hits in the Main Path:", numOrdHits, gate::DETAILED);


  // Dealing with the Non Hottest Tracks
  if (numTracks > 1) {
    for (auto track: tracks) {
      double eDep = track->GetEnergy();
      if (eDep != maxEdep) {
        gate::Centella::instance()->hman()->fill(this->alabel("EnergyNonHotTrk"), eDep);
        _m.message("Non Hottest Track ID:", track->GetID(), " -> Edep:", eDep, gate::DETAILED);
      }
    }
  }

  return true;
}



//==========================================================================
bool roadCharacterizer::finalize(){

  _m.message("Finalize()", gate::NORMAL);
  
  return true;
}
