
#include<extremesCharacterizer.h>

ClassImp(extremesCharacterizer)

//==========================================================================
extremesCharacterizer::extremesCharacterizer(gate::VLEVEL vl, std::string label) : 
IAlgo(vl,"extremesCharacterizer",0,label) {

  _m.message("Constructor()", gate::NORMAL);
}



//==========================================================================
extremesCharacterizer::extremesCharacterizer(const gate::ParamStore& gs, 
			   gate::VLEVEL vl, std::string label) :
  IAlgo(gs,vl,"extremesCharacterizer",0,label) {

  _m.message("Constructor()", gate::NORMAL);
}



//==========================================================================
bool extremesCharacterizer::initialize() {

  _m.message("Initialize()", gate::NORMAL);

  /// HISTOGRAMS
  // Histogram: Distance between True & Recons. extremes
  gate::Centella::instance()->hman()->h1(this->alabel("DistTrueRec"),
                                         "Distance True-Rec extremes",
                                         40, 0, 40);
 
  // Histogram: Blobs Radius vs Blob1 Energy
  gate::Centella::instance()->hman()->h2(this->alabel("BlobRad_Blob1E_1"),
                                         "Blobs Radius vs Blob1 Energy (HotTrack Hit)",
                                         20, 0, 20, 50, 0.0, 1.0);
  gate::Centella::instance()->hman()->fetch(this->alabel("BlobRad_Blob1E_1"))->SetOption("COLZ");

  gate::Centella::instance()->hman()->h2(this->alabel("BlobRad_Blob1E_2"),
                                         "Blobs Radius vs Blob1 Energy (Every hit)",
                                         20, 0, 20, 50, 0.0, 1.0);
  gate::Centella::instance()->hman()->fetch(this->alabel("BlobRad_Blob1E_2"))->SetOption("COLZ");

  // Histogram: Blobs Radius vs Blob2 Energy
  gate::Centella::instance()->hman()->h2(this->alabel("BlobRad_Blob2E_1"),
                                         "Blobs Radius vs Blob2 Energy (HotTrack Hit)",
                                         20, 0, 20, 50, 0.0, 1.0);
  gate::Centella::instance()->hman()->fetch(this->alabel("BlobRad_Blob2E_1"))->SetOption("COLZ");

  gate::Centella::instance()->hman()->h2(this->alabel("BlobRad_Blob2E_2"),
                                         "Blobs Radius vs Blob2 Energy (Every hit)",
                                         20, 0, 20, 50, 0.0, 1.0);
  gate::Centella::instance()->hman()->fetch(this->alabel("BlobRad_Blob2E_2"))->SetOption("COLZ");

  return true;
}



//==========================================================================
bool extremesCharacterizer::execute(gate::Event& evt) {

  _m.message("Execute()", gate::DETAILED);

  // Dealing with the Hottest Reconstructed Track
  std::vector<gate::Track*> rTracks = evt.GetTracks();
  double maxEdep = 0.;
  gate::Track* hRTrack;
  for (auto track: rTracks) {
    double eDep = track->GetEnergy();
    if (eDep > maxEdep) {
      maxEdep = eDep;
      hRTrack = track;
    }
  }

  //// Distance from Rec to True Extremes
  gate::Point3D rPos1, rPos2;
  gate::Point3D tPos1, tPos2;
  double dist1, dist2;

  // Rec Extremes
  std::pair<gate::BHit*, gate::BHit*> rExtremes = hRTrack->GetExtremes();
  rPos1 = rExtremes.first->GetPosition();
  rPos2 = rExtremes.second->GetPosition();

  // True Extremes
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

  _m.message("Rec. Position 1: (", rPos1.x(), ",", rPos1.y(), ",", rPos1.z(), ")", gate::VERBOSE);       
  _m.message("Rec. Position 2: (", rPos2.x(), ",", rPos2.y(), ",", rPos2.z(), ")", gate::VERBOSE);       
  _m.message("True Position 1: (", tPos1.x(), ",", tPos1.y(), ",", tPos1.z(), ")", gate::VERBOSE);       
  _m.message("True Position 2: (", tPos2.x(), ",", tPos2.y(), ",", tPos2.z(), ")", gate::VERBOSE);       

  // Matching Rec & True extremes
  double d11 = gate::distance(rPos1, tPos1);
  double d12 = gate::distance(rPos1, tPos2);
  if (d11 < d12) {
    dist1 = d11;
    dist2 = gate::distance(rPos2, tPos2);
  }
  else {
    dist1 = d12;
    dist2 = gate::distance(rPos2, tPos1);
  }
  _m.message("Distance 1:", dist1, gate::DETAILED);       
  _m.message("Distance 2:", dist2, gate::DETAILED);       

  gate::Centella::instance()->hman()->fill(this->alabel("DistTrueRec"), dist1);
  gate::Centella::instance()->hman()->fill(this->alabel("DistTrueRec"), dist2);


  ///// Study of Blob Radius vs Blob1E & Blob2E (Considering hits from the Hottest Track)
  TH1* myHisto1 = gate::Centella::instance()->hman()->fetch(this->alabel("BlobRad_Blob1E_1"));
  TH1* myHisto2 = gate::Centella::instance()->hman()->fetch(this->alabel("BlobRad_Blob2E_1"));
  int numRads = myHisto1->GetXaxis()->GetNbins();
  std::vector <double> eBlob1;
  std::vector <double> eBlob2;
  for (int rad=0; rad<numRads; rad++) {
    eBlob1.push_back(0.);
    eBlob2.push_back(0.);
  }

  for (auto rHit: hRTrack->GetHits()) {
    gate::Point3D hPos = rHit->GetPosition();
    double hitE = rHit->GetAmplitude();
    dist1 = gate::distance(rPos1, hPos);
    dist2 = gate::distance(rPos2, hPos);
    for (int rad=0; rad<numRads; rad++) {
      if (dist1 < rad) eBlob1[rad] += hitE;
      if (dist2 < rad) eBlob2[rad] += hitE;
    }
  }

  for (int rad=0; rad<numRads; rad++) {
    double e1 = eBlob1[rad];
    double e2 = eBlob2[rad];
    if (e1 < e2) {
      myHisto2->Fill(rad+1, e1);
      myHisto1->Fill(rad+1, e2);
    }
    else {
      myHisto2->Fill(rad+1, e2);
      myHisto1->Fill(rad+1, e1);
    }
  }

  _m.message("Energy at extreme A (Hot Trk):", gate::vector_to_string(eBlob1), gate::DETAILED);       
  _m.message("Energy at extreme B (Hot Trk):", gate::vector_to_string(eBlob2), gate::DETAILED);       


  ///// Study of Blob Radius vs Blob2E (Considering every hit in the event)
  myHisto1 = gate::Centella::instance()->hman()->fetch(this->alabel("BlobRad_Blob1E_2"));
  myHisto2 = gate::Centella::instance()->hman()->fetch(this->alabel("BlobRad_Blob2E_2"));
  numRads = myHisto1->GetXaxis()->GetNbins();
  eBlob1.clear();
  eBlob2.clear();
  for (int rad=0; rad<numRads; rad++) {
    eBlob1.push_back(0.);
    eBlob2.push_back(0.);
  }

  for (auto rHit: evt.GetHits()) {
    gate::Point3D hPos = rHit->GetPosition();
    double hitE = rHit->GetAmplitude();
    dist1 = gate::distance(rPos1, hPos);
    dist2 = gate::distance(rPos2, hPos);
    for (int rad=0; rad<numRads; rad++) {
      if (dist1 < rad) eBlob1[rad] += hitE;
      if (dist2 < rad) eBlob2[rad] += hitE;
    }
  }

  for (int rad=0; rad<numRads; rad++) {
    double e1 = eBlob1[rad];
    double e2 = eBlob2[rad];
    if (e1 < e2) {
      myHisto2->Fill(rad+1, e1);
      myHisto1->Fill(rad+1, e2);
    }
    else {
      myHisto2->Fill(rad+1, e2);
      myHisto1->Fill(rad+1, e1);
    }
  }

  _m.message("Energy at extreme A (Event):", gate::vector_to_string(eBlob1), gate::DETAILED);       
  _m.message("Energy at extreme B (Event):", gate::vector_to_string(eBlob2), gate::DETAILED);


  return true;
}



//==========================================================================
bool extremesCharacterizer::finalize() {

  _m.message("Finalize()", gate::NORMAL);

  return true;
}

