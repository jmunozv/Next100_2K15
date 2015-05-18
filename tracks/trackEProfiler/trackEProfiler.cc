
#include<trackEProfiler.h>

ClassImp(trackEProfiler)

//==========================================================================
trackEProfiler::trackEProfiler(gate::VLEVEL vl, std::string label) : 
IAlgo(vl,"trackEProfiler",0,label) {

  _m.message("Constructor()", gate::NORMAL);
}



//==========================================================================
trackEProfiler::trackEProfiler(const gate::ParamStore& gs, gate::VLEVEL vl, std::string label) :
  IAlgo(gs,vl,"trackEProfiler",0,label) {

  _m.message("Constructor()", gate::NORMAL);
}



//==========================================================================
bool trackEProfiler::initialize() {

  _m.message("Initialize()", gate::NORMAL);

  /// HISTOGRAMS
  // Histogram: Distance from Blob2 vs Hit Energy
  gate::Centella::instance()->hman()->h2(this->alabel("DistBlob2_HitE"),
                                         "Distance from Blob2 vs Hit Energy",
                                         50, 0, 250, 25, 0.0, 0.05);
  gate::Centella::instance()->hman()->fetch(this->alabel("DistBlob2_HitE"))->SetOption("COLZ");
  gate::Centella::instance()->hman()->fetch(this->alabel("DistBlob2_HitE"))->GetXaxis()->SetTitle("Distance from Blob2 [mm]");
  gate::Centella::instance()->hman()->fetch(this->alabel("DistBlob2_HitE"))->GetYaxis()->SetTitle("Hit Energy [MeV]");

  // Histogram: Energy Deposited from Blob2 vs Hit Energy
  gate::Centella::instance()->hman()->h2(this->alabel("EdepBlob2_HitE"),
                                         "Edep from Blob2 vs Hit Energy",
                                         100, 0., 2.5, 25, 0.0, 0.05);
  gate::Centella::instance()->hman()->fetch(this->alabel("EdepBlob2_HitE"))->SetOption("COLZ");
  gate::Centella::instance()->hman()->fetch(this->alabel("EdepBlob2_HitE"))->GetXaxis()->SetTitle("Edep from Blob2 [MeV]");
  gate::Centella::instance()->hman()->fetch(this->alabel("EdepBlob2_HitE"))->GetYaxis()->SetTitle("Hit Energy [MeV]");


  // Histogram: InTrack Length
  gate::Centella::instance()->hman()->h1(this->alabel("InTrackLength"),
                                         "Length along the track", 60, 0, 300);

  // Histogram: InTrack Length (without blobs)
  gate::Centella::instance()->hman()->h1(this->alabel("InTrackLengthNB"),
                                         "Length along the track (No Blobs)", 60, 0, 300);

  return true;
}



//==========================================================================
bool trackEProfiler::execute(gate::Event& evt) {

  _m.message("Execute()", gate::DETAILED);

  // Dealing with the Hottest Reconstructed Track
  std::vector<gate::Track*> Tracks = evt.GetTracks();
  double maxEdep = 0.;
  gate::Track* hTrack;
  for (auto track: Tracks) {
    double eDep = track->GetEnergy();
    if (eDep > maxEdep) {
      maxEdep = eDep;
      hTrack = track;
    }
  }
  
  // Identifyng Blob2
  const std::pair <gate::BHit*, gate::BHit*> extremes = hTrack->GetExtremes();
  const gate::Point3D blob1Pos = extremes.first->GetPosition();
  const gate::Point3D blob2Pos = extremes.second->GetPosition();

  double blob1E = 0;
  double blob2E = 0;

  for (auto hit: hTrack->GetHits()) {
    //const gate::Point3D hitPos = hit->GetPosition();

    //double dist1 = gate::distance(hitPos, blob1Pos);
    double dist1 = inTrackDistance(hTrack, hit, extremes.first);
    if (dist1 < 12) blob1E += hit->GetAmplitude();

    //double dist2 = gate::distance(hitPos, blob2Pos);
    double dist2 = inTrackDistance(hTrack, hit, extremes.second);
    if (dist2 < 12) blob2E += hit->GetAmplitude();
  }

  gate::BHit* blob2;
  if (blob1E < blob2E) blob2 = extremes.first;
  else blob2 = extremes.second;


  // Filling Histograms
  for (auto hit: hTrack->GetHits()) {
    double dist = inTrackDistance(hTrack, hit, blob2);
    double hitE = hit->GetAmplitude();
    if (hitE > 0.001)
      gate::Centella::instance()->hman()->fill2d(this->alabel("DistBlob2_HitE"), dist, hitE);

    double EfromBlob2 = 0.;
    for (auto hitt: hTrack->GetHits()) {
      double d = inTrackDistance(hTrack, hitt, blob2);
      if (d < dist) EfromBlob2 += hitt->GetAmplitude();
    }
    if (hitE > 0.001)
      gate::Centella::instance()->hman()->fill2d(this->alabel("EdepBlob2_HitE"), EfromBlob2, hitE);
  }

  /// Length issues
  double length = inTrackDistance(hTrack, extremes.first, extremes.second);
  gate::Centella::instance()->hman()->fill(this->alabel("InTrackLength"), length);


  return true;
}



//==========================================================================
bool trackEProfiler::finalize() {

  _m.message("Finalize()", gate::NORMAL);

  return true;
}



//==========================================================================
double trackEProfiler::inTrackDistance(gate::BTrack* track, gate::BHit* hit1, gate::BHit* hit2) {
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

