
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

  // Calculating the blobs energy
  const std::pair <gate::BHit*, gate::BHit*> extremes = hTrack->GetExtremes();
  const gate::Point3D blob1Pos = extremes.first->GetPosition();
  const gate::Point3D blob2Pos = extremes.second->GetPosition();

  double blob1E = 0;
  double blob2E = 0;

  for (auto hit: evt.GetHits()) {
    const gate::Point3D hitPos = hit->GetPosition();

    double dist1 = gate::distance(hitPos, blob1Pos);
    if (dist1 < _blobRadius) blob1E += hit->GetAmplitude();

    double dist2 = gate::distance(hitPos, blob2Pos);
    if (dist2 < _blobRadius) blob2E += hit->GetAmplitude();
  }

  // Ordering the energies (Blob1 is the hottest)
  if (blob2E > blob1E) {
    double auxE = blob1E;
    blob1E = blob2E;
    blob2E = auxE;
  }

  // Both Blobs must be over the energy threshold
  if ((blob1E > _blobMinE) && (blob2E > _blobMinE)) {
    gate::Centella::instance()->hman()->fill(this->alabel("evtEdepAfter"), evt.GetEnergy());
    _m.message("Filter Passed. Blob1E:", blob1E, " Blob2E:", blob2E, gate::DETAILED);
    _numOutputEvents += 1;
    return true;
  }
  else {
    _m.message("Filter Failed. Blob1E:", blob1E, " Blob2E:", blob2E, gate::DETAILED);
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
