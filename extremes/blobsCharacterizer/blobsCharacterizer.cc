
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


  // Histogram: Blob1 Voxels
  gate::Centella::instance()->hman()->h1(this->alabel("Blob1Voxels"),
                                         "Blob1 Voxels", 30, 0, 30);
  // Histogram: Blob2 Voxels
  gate::Centella::instance()->hman()->h1(this->alabel("Blob2Voxels"),
                                         "Blob2 Voxels", 30, 0, 30);

  // Histogram: Blob1 vs Blob2 Voxels
  gate::Centella::instance()->hman()->h2(this->alabel("Blob1Voxels_Blob2Voxels"),
                                         "Blob1 vs. Blob2 Voxels",
                                         30, 0, 30, 30, 0, 30);
  gate::Centella::instance()->hman()->fetch(this->alabel("Blob1Voxels_Blob2Voxels"))->SetOption("COLZ");
  gate::Centella::instance()->hman()->fetch(this->alabel("Blob1Voxels_Blob2Voxels"))->GetXaxis()->SetTitle("Blob1 Voxels");
  gate::Centella::instance()->hman()->fetch(this->alabel("Blob1Voxels_Blob2Voxels"))->GetYaxis()->SetTitle("Blob2 Voxels");

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
  std::vector <double> distExtFirst  = hTrack->fetch_dvstore("DistExtFirst");
  std::vector <double> distExtSecond = hTrack->fetch_dvstore("DistExtSecond");

  // Calculating the blobs energy
  const std::pair <gate::BHit*, gate::BHit*> extremes = hTrack->GetExtremes();

  double blob1E = 0;
  double blob2E = 0;
  int blob1Voxels = 0;
  int blob2Voxels = 0;

  for (auto hit: evt.GetHits()) {
    //double dist1 = inTrackDistance(hTrack, hit, extremes.first);
    double dist1 = distExtFirst[hit->GetID()];
    if (dist1 < _blobRadius) {
      blob1E += hit->GetAmplitude();
      blob1Voxels += 1;
    }
    //double dist2 = inTrackDistance(hTrack, hit, extremes.second);
    double dist2 = distExtSecond[hit->GetID()];
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

  // Filling Histograms
  gate::Centella::instance()->hman()->fill(this->alabel("Blob1E"), blob1E);
  gate::Centella::instance()->hman()->fill(this->alabel("Blob2E"), blob2E);
  gate::Centella::instance()->hman()->fill2d(this->alabel("Blob1E_Blob2E"), blob1E, blob2E);

  gate::Centella::instance()->hman()->fill(this->alabel("Blob1Voxels"), blob1Voxels);
  gate::Centella::instance()->hman()->fill(this->alabel("Blob2Voxels"), blob2Voxels);
  gate::Centella::instance()->hman()->fill2d(this->alabel("Blob1Voxels_Blob2Voxels"), blob1Voxels, blob2Voxels);

  // Verbosing
  _m.message("Blob1 Energy:", blob1E, gate::DETAILED);
  _m.message("Blob2 Energy:", blob2E, gate::DETAILED);

  _m.message("Blob1 Voxels:", blob1Voxels, gate::DETAILED);
  _m.message("Blob2 Voxels:", blob2Voxels, gate::DETAILED);

  return true;
}



//==========================================================================
bool blobsCharacterizer::finalize() {

  _m.message("Finalising algorithm",this->getAlgoLabel(),gate::NORMAL);
  
  return true;
}



