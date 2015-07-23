
#include<trueBlobFilter.h>

ClassImp(trueBlobFilter)

//==========================================================================
trueBlobFilter::trueBlobFilter(gate::VLEVEL vl, std::string label) : 
IAlgo(vl,"trueBlobFilter",0,label) {

  _m.message("Constructor()", gate::NORMAL);
}



//==========================================================================
trueBlobFilter::trueBlobFilter(const gate::ParamStore& gs, 
			   gate::VLEVEL vl, std::string label) :
  IAlgo(gs,vl,"trueBlobFilter",0,label) {

  _m.message("Constructor()", gate::NORMAL);

  // Getting the parameters
  _blobRadius = gs.fetch_dstore("blobRadius");
  _m.message("Blob Radius:", _blobRadius, gate::NORMAL);

  _blobMinE = gs.fetch_dstore("blobMinE");
  _m.message("Blob Minimum Energy:", _blobMinE, gate::NORMAL);

}



//==========================================================================
bool trueBlobFilter::initialize() {

  _m.message("Initialize()", gate::NORMAL);
  
  /// Histograms
  // Histograms with Event Energy After Filter
  gate::Centella::instance()->hman()->h1(this->alabel("evtEdepAfter"),
                                         "Event Energy Dep. After Filter", 100, 2.4, 2.5);

  /// HISTOGRAMS
  // Histogram: Blob1 Energy
  gate::Centella::instance()->hman()->h1(this->alabel("Blob1E"),
                                         "Blob1 Energy", 80, 0, 0.8);
  // Histogram: Blob2 Energy
  gate::Centella::instance()->hman()->h1(this->alabel("Blob2E"),
                                         "Blob2 Energy", 80, 0, 0.8);

  // Histogram: Blob1 vs Blob2 Energy
  gate::Centella::instance()->hman()->h2(this->alabel("Blob1E_Blob2E"),
                                         "Blob1 vs. Blob2 Energy",
                                         80, 0, 0.8, 80, 0, 0.8);
  gate::Centella::instance()->hman()->fetch(this->alabel("Blob1E_Blob2E"))->SetOption("COLZ");
  gate::Centella::instance()->hman()->fetch(this->alabel("Blob1E_Blob2E"))->GetXaxis()->SetTitle("Blob1 Energy [MeV]");
  gate::Centella::instance()->hman()->fetch(this->alabel("Blob1E_Blob2E"))->GetYaxis()->SetTitle("Blob2 Energy [MeV]");


  /// Counters
  _numInputEvents  = 0;
  _numOutputEvents = 0;

  return true;

}



//==========================================================================
bool trueBlobFilter::execute(gate::Event& evt) {

  _m.message("Execute()", gate::DETAILED);

  _numInputEvents += 1;

  std::vector<gate::Track*> rTracks = evt.GetTracks();
  int numTracks = rTracks.size();

 
  /// Getting the Hottest Rec Track
  gate::Track* hRTrack;
  if (numTracks == 1) hRTrack = rTracks[0];
  else {
    double maxEdep = 0.;
    for (auto track: rTracks) {
      double eDep = track->GetEnergy();
      if (eDep > maxEdep) {
        maxEdep = eDep;
        hRTrack = track;
      }
    }
  }
  std::vector <double> distExtFirst  = hRTrack->fetch_dvstore("DistExtFirst");
  std::vector <double> distExtSecond = hRTrack->fetch_dvstore("DistExtSecond");
  //std::cout << gate::vector_to_string(distExtFirst);

  /// True Extremes
  gate::Point3D tPos1, tPos2;
  std::vector<gate::MCTrack*> tTracks = evt.GetMCTracks();

  // If Signal, True extremes are the second extremes of "primary" tracks
  if (evt.GetMCEventType() == gate::BB0NU) {
    int numPrimaries = 0;
    for (auto tTrack: tTracks) {
      const gate::MCParticle tPart = tTrack->GetParticle();
      if (tPart.IsPrimary()) {
        numPrimaries += 1;
        if (numPrimaries == 1)
          tPos1 = tTrack->GetExtremes().second->GetPosition();
        if (numPrimaries == 2) {
          tPos2 = tTrack->GetExtremes().second->GetPosition();
          break;
        }
      }
    }
  }

  // If Background, True extremes are the extremes of the Hottest TTracks
  else {
    std::vector<int> tTrackIDs = hRTrack->fetch_ivstore("mcTrackIDs");
    // Getting the hottest True Track from the Hottest rTrack
    double maxEdep = 0.;
    gate::MCTrack* hTTrack;
    for (auto tTrack: tTracks) {
      int tTrackID = tTrack->GetID();
      bool inList = false;
      for (int i=0; i<tTrackIDs.size(); i++) {
        if (tTrackID == tTrackIDs[i]) {
          inList = true;
          break;
        }
      }
      if (inList) {
        double eDep = tTrack->GetEnergy();
        if (eDep > maxEdep) {
          maxEdep = eDep;
          hTTrack = tTrack;
        }
      }
    }
    tPos1 = hTTrack->GetExtremes().first->GetPosition();
    tPos2 = hTTrack->GetExtremes().second->GetPosition();
  }


  /// Identifying the Main Path Rec Hits closest to True Extremes

  // Main Path Hits
  std::vector <gate::BHit*> hits = hRTrack->GetHits();
  std::vector <int> mpHitsIDs = hRTrack->fetch_ivstore("MainPathHits");
  std::vector <gate::BHit*> mpHits;
  for (auto mpHitID: mpHitsIDs) mpHits.push_back(hits[mpHitID]);

  // Identifying the closest ones
  double minDist1 = 10000.;
  double minDist2 = 10000.;
  gate::BHit* closestHit1;
  gate::BHit* closestHit2;
  for (auto hit: mpHits) {
    gate::Point3D pos = hit->GetPosition();
    double dist1 = gate::distance(pos, tPos1);
    double dist2 = gate::distance(pos, tPos2);
    if (dist1 < minDist1) {
      minDist1 = dist1;
      closestHit1 = hit;
    }
    if (dist2 < minDist2) {
      minDist2 = dist2;
      closestHit2 = hit;
    }
  }
  
  int closestHitID1 = closestHit1->GetID();
  int closestHitID2 = closestHit2->GetID();

  double origin1 = distExtFirst[closestHitID1];
  double origin2 = distExtFirst[closestHitID2];

  //std::cout << minDist1 << ' ' << closestHitID1 << ' ' << origin1 << std::endl;
  //std::cout << minDist2 << ' ' << closestHitID2 << ' ' << origin2 << std::endl;


  /// Computing blobs energy
  double blob1E = 0;
  double blob2E = 0;

  for (auto hit: hRTrack->GetHits()) {
    double dist1 = abs (distExtFirst[hit->GetID()] - origin1);
    if (dist1 < _blobRadius) {
      blob1E += hit->GetAmplitude();
      //std::cout << "Adding hitID: " << hit->GetID() << " to blob1" << std::endl;
    }

    double dist2 = abs (distExtFirst[hit->GetID()] - origin2);
    if (dist2 < _blobRadius) {
      blob2E += hit->GetAmplitude();
      //std::cout << "Adding hitID: " << hit->GetID() << " to blob2" << std::endl;
    }
  }

  // Ordering the energies (Blob1 is the hottest in terms of energy)
  if (blob2E > blob1E) {
    double auxE = blob1E;
    blob1E = blob2E;
    blob2E = auxE;
  }

  // Filling Histograms
  gate::Centella::instance()->hman()->fill(this->alabel("Blob1E"), blob1E);
  gate::Centella::instance()->hman()->fill(this->alabel("Blob2E"), blob2E);
  gate::Centella::instance()->hman()->fill2d(this->alabel("Blob1E_Blob2E"), blob1E, blob2E);
  //std::cout << blob1E << ' ' << blob2E << std::endl << std::endl;



  if ((blob1E > _blobMinE) && (blob2E > _blobMinE)) {
    _numOutputEvents += 1;
    return true;
  }

  return false;

}



//==========================================================================
bool trueBlobFilter::finalize() {

  _m.message("Finalize()", gate::NORMAL);

  _m.message("Input  Events:", _numInputEvents, gate::NORMAL);
  _m.message("Output Events:", _numOutputEvents, gate::NORMAL);
  
  gate::Centella::instance()->logman().getLogs("USER").store(this->alabel("InputEvents"), _numInputEvents);
  gate::Centella::instance()->logman().getLogs("USER").store(this->alabel("OutputEvents"), _numOutputEvents);
  
  return true;

}
