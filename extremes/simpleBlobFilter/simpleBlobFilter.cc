
#include<simpleBlobFilter.h>

ClassImp(simpleBlobFilter)


//==========================================================================
simpleBlobFilter::simpleBlobFilter(gate::VLEVEL vl, std::string label) : 
IAlgo(vl,"simpleBlobFilter",0,label) {

  _m.message("Constructor()", gate::NORMAL);
}



//==========================================================================
simpleBlobFilter::simpleBlobFilter(const gate::ParamStore& gs,
                                   gate::VLEVEL vl, std::string label) :
  IAlgo(gs,vl,"simpleBlobFilter",0,label) {

  _m.message("Constructor()", gate::NORMAL);

  // Getting the parameters
  _blobRadius = gs.fetch_dstore("blobRadius");
  _m.message("Blob Radius:", _blobRadius, gate::NORMAL);

  _blobMinE = gs.fetch_dstore("blobMinE");
  _m.message("Blob Minimum Energy:", _blobMinE, gate::NORMAL);

  _minVoxels = gs.fetch_istore("minVoxels");
  _m.message("Minimum Voxels in Blob:", _minVoxels, gate::NORMAL);

}



//==========================================================================
bool simpleBlobFilter::initialize() {

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
bool simpleBlobFilter::execute(gate::Event& evt) {

  _m.message("Execute()", gate::DETAILED);

  _numInputEvents += 1;

  std::vector<gate::Track*> rTracks = evt.GetTracks();
  int numTracks = rTracks.size();

  // Getting the Hottest Track
  gate::Track* hTrack;
  if (numTracks == 1) hTrack = rTracks[0];
  else {
    double maxEdep = 0.;
    for (auto track: rTracks) {
      double eDep = track->GetEnergy();
      if (eDep > maxEdep) {
        maxEdep = eDep;
        hTrack = track;
      }
    }
  }

  // Calculating the blobs energy & voxels
  const std::pair <gate::BHit*, gate::BHit*> extremes = hTrack->GetExtremes();
  const gate::Point3D blob1Pos = extremes.first->GetPosition();
  const gate::Point3D blob2Pos = extremes.second->GetPosition();

  double blob1E = 0;
  double blob2E = 0;

  int blob1Voxels = 0;
  int blob2Voxels = 0;

  for (auto hit: evt.GetHits()) {
    const gate::Point3D hitPos = hit->GetPosition();

    //double dist1 = gate::distance(hitPos, blob1Pos);
    double dist1 = inTrackDistance(hTrack, hit, extremes.first);
    if (dist1 < _blobRadius) {
      blob1E += hit->GetAmplitude();
      blob1Voxels += 1;
    }

    //double dist2 = gate::distance(hitPos, blob2Pos);
    double dist2 = inTrackDistance(hTrack, hit, extremes.second);
    if (dist2 < _blobRadius) {
      blob2E += hit->GetAmplitude();
      blob2Voxels += 1;
    }
  }

  // Ordering the energies & voxels (Blob1 is the hottest in terms of energy)
  if (blob2E > blob1E) {
    double auxE = blob1E;
    int auxVoxels  = blob1Voxels;

    blob1E = blob2E;
    blob2E = auxE;

    blob1Voxels = blob2Voxels;
    blob2Voxels = auxVoxels;
  }

  // Both Blobs must be over the energy threshold and over the Number of Voxels
  if ((blob1E > _blobMinE) && (blob2E > _blobMinE) &&
      (blob1Voxels > _minVoxels) && (blob2Voxels > _minVoxels)) {
    gate::Centella::instance()->hman()->fill(this->alabel("evtEdepAfter"), evt.GetEnergy());
    _m.message("Filter Passed. Blob1E:", blob1E, " Blob2E:", blob2E, gate::DETAILED);
    _m.message("               Blob1Voxels:", blob1Voxels, " Blob2Voxels:", blob2Voxels, gate::DETAILED);
    _numOutputEvents += 1;
    return true;
  }
  else {
    _m.message("Filter Failed. Blob1E:", blob1E, " Blob2E:", blob2E, gate::DETAILED);
    _m.message("               Blob1Voxels:", blob1Voxels, " Blob2Voxels:", blob2Voxels, gate::DETAILED);
    return false;
  }

  return false;
}



//==========================================================================
bool simpleBlobFilter::finalize() {

  _m.message("Finalize()", gate::NORMAL);

  _m.message("Input  Events:", _numInputEvents, gate::NORMAL);
  _m.message("Output Events:", _numOutputEvents, gate::NORMAL);
  
  gate::Centella::instance()->logman().getLogs("USER").store(this->alabel("InputEvents"), _numInputEvents);
  gate::Centella::instance()->logman().getLogs("USER").store(this->alabel("OutputEvents"), _numOutputEvents);

  return true;
}



//==========================================================================
double simpleBlobFilter::inTrackDistance(gate::BTrack* track, gate::BHit* hit1, gate::BHit* hit2) {
  double distance = 0.;
  std::vector <gate::BHit*> hits = track->GetHits();
  std::vector <int> mpHitsIDs = track->fetch_ivstore("MainPathHits");
  std::vector <gate::BHit*> mpHits;
  for (auto mpHitID: mpHitsIDs) mpHits.push_back(hits[mpHitID]);

  const gate::Point3D hit1Pos = hit1->GetPosition();
  const gate::Point3D hit2Pos = hit2->GetPosition();

  // First, calculate distance from hits to MainPath
  double dist_h1_mp = 1000;
  double dist_h2_mp = 1000;
  gate::BHit* mpHit1;
  gate::BHit* mpHit2;
  for (auto mpHit: mpHits) {
    double dist1 = gate::distance(hit1Pos, mpHit->GetPosition());
    double dist2 = gate::distance(hit2Pos, mpHit->GetPosition());
    if (dist1 < dist_h1_mp) {
      dist_h1_mp = dist1;
      mpHit1 = mpHit;
    }
    if (dist2 < dist_h2_mp) {
      dist_h2_mp = dist2;
      mpHit2 = mpHit;
    }
  }

 // Calculate distance between mpHits along the track main path
  double dist = 0.;
  bool firstFound  = false;
  bool secondFound = false;

  if (mpHit1 == mpHit2) {
    dist = 0;
  }
  else {
    for (int i=0; i<mpHits.size(); i++) {
      if ((mpHits[i]->GetID() == mpHit1->GetID()) || (mpHits[i]->GetID() == mpHit2->GetID())) {
        if (not firstFound) firstFound = true;
        else secondFound = true;
      }
      if (firstFound && (not secondFound)) dist += gate::distance(mpHits[i], mpHits[i+1]);
      if (firstFound && secondFound) break;
    }
  }

  distance = dist_h1_mp + dist_h2_mp + dist;

  return distance;
}

