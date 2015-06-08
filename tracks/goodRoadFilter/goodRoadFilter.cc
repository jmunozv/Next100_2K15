
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
  _maxNumTracks = gs.fetch_istore("maxNumTracks");
  _m.message("Maximum Number of Tracks:", _maxNumTracks, gate::NORMAL);

  _minEnergy = gs.fetch_dstore("minEnergy");
  _m.message("Minimum Energy of Hottest Track:", _minEnergy/gate::MeV, "MeV", gate::NORMAL);

  _maxEnergy = gs.fetch_dstore("maxEnergy");
  _m.message("Maximum Energy of Hottest Track:", _maxEnergy/gate::MeV, "MeV", gate::NORMAL);
}



//==========================================================================
bool goodRoadFilter::initialize() {

  _m.message("Initialize()", gate::NORMAL);
  
  /// Histograms
  // Histograms Before Filter
  gate::Centella::instance()->hman()->h1(this->alabel("evtEdepBefore"),
                                         "Event Energy Dep. Before Filter", 100, 2.3, 2.6);

  gate::Centella::instance()->hman()->h1(this->alabel("NumTracksBefore"),
                                         "Number of Tracks After Filter", 10, 0, 10);

  // Histograms After Filter
  gate::Centella::instance()->hman()->h1(this->alabel("HTrkEdepAfter"),
                                         "Hottest Track Energy Dep. After Filter", 100, 2.3, 2.6);

  gate::Centella::instance()->hman()->h1(this->alabel("evtEdepAfter"),
                                         "Event Energy Dep. After Filter", 100, 2.3, 2.6);

  gate::Centella::instance()->hman()->h1(this->alabel("NumTracksAfter"),
                                         "Number of Tracks After Filter", 10, 0, 10);


  /// Counters
  _numInputEvents  = 0;
  _numOutputEvents = 0;

  return true;
}



//==========================================================================
bool goodRoadFilter::execute(gate::Event& evt) {

  _m.message("Execute()", gate::DETAILED);

  _numInputEvents += 1;

  // Event Energy
  double evtEnergy = evt.GetEnergy();

  // Getting Tracks
  std::vector<gate::Track*> tracks = evt.GetTracks();
  int numTracks = tracks.size();

  // Filling Before Histos
  gate::Centella::instance()->hman()->fill(this->alabel("evtEdepBefore"), evtEnergy);
  gate::Centella::instance()->hman()->fill(this->alabel("NumTracksBefore"), tracks.size());      

  if (numTracks <= _maxNumTracks) {

    // Getting the Hottest Track
    double hTrkEdep = 0.;
    gate::Track* hTrack;
    for (auto track: tracks) {
      double eDep = track->GetEnergy();
      if (eDep > hTrkEdep) {
        hTrkEdep = eDep;
        hTrack = track;
      }
    }
    int hTrackID = hTrack->GetID();

    if ((hTrkEdep <= _maxEnergy) && (hTrkEdep >= _minEnergy)) {
      _m.message("Filter Passed. Num Tracks:", numTracks, gate::DETAILED);
      _m.message("               Hottest Track ID:", hTrackID, " with Edep:", hTrkEdep, gate::DETAILED);

      _numOutputEvents += 1;
      
      gate::Centella::instance()->hman()->fill(this->alabel("HTrkEdepAfter"), hTrkEdep);
      gate::Centella::instance()->hman()->fill(this->alabel("evtEdepAfter"), evtEnergy);
      gate::Centella::instance()->hman()->fill(this->alabel("NumTracksAfter"), numTracks);

      // Calculate & store the distance to extremes of every hit
      for (auto track: tracks) fillHitsDistances(track);

      return true;      
    }

    else {
      _m.message("Filter Failed. Hottest Track ID:", hTrackID, " with Edep:", hTrkEdep, gate::DETAILED);
      return false;      
    }
  }

  else {
      _m.message("Filter Failed. Num Tracks: ", numTracks, gate::DETAILED);
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



//==========================================================================
void goodRoadFilter::fillHitsDistances(gate::BTrack* track) {
  // Hits
  std::vector <gate::BHit*> hits = track->GetHits();
  
  // Main Path Hits
  std::vector <int> mpHitsIDs = track->fetch_ivstore("MainPathHits");
  std::vector <gate::BHit*> mpHits;
  for (auto mpHitID: mpHitsIDs) mpHits.push_back(hits[mpHitID]);

  // Vectors of distances
  std::vector <double> distExtFirst;
  std::vector <double> distExtSecond;
  for (auto hit: hits) {
    distExtFirst.push_back(0.);
    distExtSecond.push_back(0.);
  }

  // Main Path Hits. Distances from First Extreme
  double distance = 0;
  for (unsigned int i=1; i < mpHits.size(); i++) {
    double dist = gate::distance(mpHits[i-1], mpHits[i]);
    distance += dist;
    distExtFirst[mpHits[i]->GetID()] = distance;
  }

  // Main Path Hits. Distances from Second Extreme
  distance = 0;
  for (unsigned int i = mpHits.size()-1; i>0; i--) {
    double dist = gate::distance(mpHits[i-1], mpHits[i]);
    distance += dist;
    distExtSecond[mpHits[i-1]->GetID()] = distance;
  }
  
  // Non Main Path Hits
  for (auto hit: hits) {
    distance = 0.;
    if (distExtFirst[hit->GetID()] == 0.) {
      // Identifying the closest Main Path Hit (euclidean)
      double dist = 1000.;
      gate::BHit* closestMPHit;
      for (auto mpHit: mpHits) {
        double d = gate::distance(hit, mpHit);
        if (d < dist) {
          dist = d;
          closestMPHit = mpHit;
        }
      }
      distExtFirst[hit->GetID()] = dist + distExtFirst[closestMPHit->GetID()];
      distExtSecond[hit->GetID()] = dist + distExtSecond[closestMPHit->GetID()];
    }
  }

  // Adding Vectors of distances to the track
  track->store("DistExtFirst", distExtFirst);
  track->store("DistExtSecond", distExtSecond);
}
