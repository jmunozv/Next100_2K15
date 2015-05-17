
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
  gate::Centella::instance()->hman()->h2(this->alabel("BlobRad_Blob1E"),
                                         "Blobs Radius vs Blob1 Energy",
                                         20, 0, 20, 50, 0.0, 1.0);
  gate::Centella::instance()->hman()->fetch(this->alabel("BlobRad_Blob1E"))->SetOption("COLZ");

  // Histogram: Blobs Radius vs Blob2 Energy
  gate::Centella::instance()->hman()->h2(this->alabel("BlobRad_Blob2E"),
                                         "Blobs Radius vs Blob2 Energy",
                                         20, 0, 20, 50, 0.0, 1.0);
  gate::Centella::instance()->hman()->fetch(this->alabel("BlobRad_Blob2E"))->SetOption("COLZ");


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


  ///// Study of Blob Radius vs Blob1E & Blob2E
  TH1* myHisto1 = gate::Centella::instance()->hman()->fetch(this->alabel("BlobRad_Blob1E"));
  TH1* myHisto2 = gate::Centella::instance()->hman()->fetch(this->alabel("BlobRad_Blob2E"));
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
    //dist1 = gate::distance(rPos1, hPos);
    dist1 = inTrackDistance(hRTrack, rExtremes.first, rHit);
    //dist2 = gate::distance(rPos2, hPos);
    dist2 = inTrackDistance(hRTrack, rExtremes.second, rHit);
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

  _m.message("Energy at extreme A:", gate::vector_to_string(eBlob1), gate::DETAILED);       
  _m.message("Energy at extreme B:", gate::vector_to_string(eBlob2), gate::DETAILED);       


  return true;
}



//==========================================================================
bool extremesCharacterizer::finalize() {

  _m.message("Finalize()", gate::NORMAL);

  return true;
}



//==========================================================================
double extremesCharacterizer::inTrackDistance(gate::BTrack* track, gate::BHit* hit1, gate::BHit* hit2) {
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

