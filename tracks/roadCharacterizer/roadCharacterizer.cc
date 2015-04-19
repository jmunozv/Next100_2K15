
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
                                         "Energy of Hottest Track", 50, 2.0, 2.5);
  // Histogram: Main Path Voxels of Hottest Track
  gate::Centella::instance()->hman()->h1(this->alabel("NumMPVoxelsHotTrk"),
                                         "Number of Main Path Voxels of Hottest Track",
                                         60, 0, 120);
  // Histogram: Energy of Second Hottest Track
  gate::Centella::instance()->hman()->h1(this->alabel("Energy2HotTrk"),
                                         "Energy of 2nd Hottest Track", 60, 0.0, 0.6);
  // Histogram: Energies of Hottest Track vs Second Hottest Track
  gate::Centella::instance()->hman()->h2(this->alabel("EnergyHotVS2Hot"),
                                         "Energy of Hottest Track vs 2nd Hottest Track",
                                         50, 2.0, 2.5, 60, 0.0, 0.6);
  gate::Centella::instance()->hman()->fetch(this->alabel("EnergyHotVS2Hot"))->SetOption("COLZ");

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

  int numOrdHits = hTrack->fetch_ivstore("MainPathHits").size();
  gate::Centella::instance()->hman()->fill(this->alabel("NumMPVoxelsHotTrk"), numOrdHits);
  _m.message("Hottest Track ID:", hTrackID, " -> Num Hits in the Main Path:", numOrdHits, gate::DETAILED);


  // Dealing with the 2nd Hottest Track
  if (numTracks > 1) {
    double maxEdep2 = 0.;
    gate::Track* hTrack2;
    for (auto track: tracks) {
      double eDep = track->GetEnergy();
      if ((eDep > maxEdep2) && (eDep != maxEdep)) {
        maxEdep2 = eDep;
        hTrack2 = track;
      }
    }
    int hTrackID2 = hTrack2->GetID();
    gate::Centella::instance()->hman()->fill(this->alabel("Energy2HotTrk"), maxEdep2);
    _m.message("2nd Hottest Track ID:", hTrackID2, " -> Edep:", maxEdep2, gate::DETAILED);

    gate::Centella::instance()->hman()->fill2d(this->alabel("EnergyHotVS2Hot"), maxEdep, maxEdep2);

  }

  return true;
}



//==========================================================================
bool roadCharacterizer::finalize(){

  _m.message("Finalize()", gate::NORMAL);
  
  return true;
}
