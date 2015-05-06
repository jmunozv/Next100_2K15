
#include<blobsCharacterizer.h>

ClassImp(blobsCharacterizer)

//==========================================================================
blobsCharacterizer::blobsCharacterizer(gate::VLEVEL vl, std::string label) : 
IAlgo(vl,"blobsCharacterizer",0,label) {

  _m.message("Constructor()", gate::NORMAL);
}


//==========================================================================
blobsCharacterizer::blobsCharacterizer(const gate::ParamStore& gs,
                                       gate::VLEVEL vl, std::string label) :
  IAlgo(gs,vl,"blobsCharacterizer",0,label) {

  _m.message("Constructor()", gate::NORMAL);

  // Getting the parameters
  _blobRadius  = gs.fetch_dstore("blobRadius");
  _m.message("Blob Radius:", _blobRadius/gate::mm, "mm", gate::NORMAL);
}



//==========================================================================
bool blobsCharacterizer::initialize() {

  _m.message("Initialize()", gate::NORMAL);

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


  return true;
}



//==========================================================================
bool blobsCharacterizer::execute(gate::Event& evt) {

  _m.message("Execute()", gate::DETAILED);

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

  // Filling Histograms
  gate::Centella::instance()->hman()->fill(this->alabel("Blob1E"), blob1E);
  gate::Centella::instance()->hman()->fill(this->alabel("Blob2E"), blob2E);
  gate::Centella::instance()->hman()->fill2d(this->alabel("Blob1E_Blob2E"), blob1E, blob2E);

  // Verbosing
  _m.message("Blob1 Energy:", blob1E, gate::DETAILED);
  _m.message("Blob2 Energy:", blob2E, gate::DETAILED);

  return true;
}



//==========================================================================
bool blobsCharacterizer::finalize() {

  _m.message("Finalising algorithm",this->getAlgoLabel(),gate::NORMAL);
  
  return true;
}
