
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

  // Getting the parameters
  _blobRadius = gs.fetch_dstore("blobRadius");
  _m.message("Blob Radius:", _blobRadius, gate::NORMAL);
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

  // Histogram: Distance from Blob2 vs Energy Deposited from Blob2
  gate::Centella::instance()->hman()->h2(this->alabel("DistBlob2_EdepBlob2"),
                                         "Distance from Blob2 vs Edep from Blob2",
                                         50, 0, 250, 100, 0.0, 2.5);
  gate::Centella::instance()->hman()->fetch(this->alabel("DistBlob2_EdepBlob2"))->SetOption("COLZ");
  gate::Centella::instance()->hman()->fetch(this->alabel("DistBlob2_EdepBlob2"))->GetXaxis()->SetTitle("Distance from Blob2 [mm]");
  gate::Centella::instance()->hman()->fetch(this->alabel("DistBlob2_EdepBlob2"))->GetYaxis()->SetTitle("Edep from Blob2 [MeV]");


  // Histogram: InTrack Length
  gate::Centella::instance()->hman()->h1(this->alabel("InTrackLength"),
                                         "Length along the track", 60, 0, 300);

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
  std::vector <double> distExtFirst  = hTrack->fetch_dvstore("DistExtFirst");
  std::vector <double> distExtSecond = hTrack->fetch_dvstore("DistExtSecond");
  
  // Identifyng Blob2
  const std::pair <gate::BHit*, gate::BHit*> extremes = hTrack->GetExtremes();
  const gate::Point3D blob1Pos = extremes.first->GetPosition();
  const gate::Point3D blob2Pos = extremes.second->GetPosition();

  double blob1E = 0;
  double blob2E = 0;

  for (auto hit: hTrack->GetHits()) {
    double dist1 = distExtFirst[hit->GetID()];
    if (dist1 < _blobRadius) blob1E += hit->GetAmplitude();

    double dist2 = distExtSecond[hit->GetID()];
    if (dist2 < _blobRadius) blob2E += hit->GetAmplitude();
  }

  gate::BHit* blob2;
  std::vector <double> distBegining;
  if (blob1E < blob2E) {
    blob2 = extremes.first;
    distBegining = distExtFirst;
  }
  else {
    blob2 = extremes.second;
    distBegining = distExtSecond;
  }

  // Filling Histograms
  for (auto hit: hTrack->GetHits()) {
    double dist = distBegining[hit->GetID()];
    double hitE = hit->GetAmplitude();
    if (hitE > 0.001)
      gate::Centella::instance()->hman()->fill2d(this->alabel("DistBlob2_HitE"), dist, hitE);

    double EfromBlob2 = 0.;
    for (auto hitt: hTrack->GetHits()) {
      double d = distBegining[hitt->GetID()];
      if (d < dist) EfromBlob2 += hitt->GetAmplitude();
    }
    if (hitE > 0.001)
      gate::Centella::instance()->hman()->fill2d(this->alabel("EdepBlob2_HitE"), EfromBlob2, hitE);

    if (dist > _blobRadius)
      gate::Centella::instance()->hman()->fill2d(this->alabel("DistBlob2_EdepBlob2"), dist, EfromBlob2);
  }

  /// Length issues
  double length = distExtFirst[extremes.second->GetID()];
  gate::Centella::instance()->hman()->fill(this->alabel("InTrackLength"), length);




  return true;
}



//==========================================================================
bool trackEProfiler::finalize() {

  _m.message("Finalize()", gate::NORMAL);

  return true;
}

