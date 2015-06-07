
#include<roadCharacterizer.h>

ClassImp(roadCharacterizer)

//==========================================================================
roadCharacterizer::roadCharacterizer(gate::VLEVEL vl, std::string label) : 
IAlgo(vl,"roadCharacterizer",0,label) {

  _m.message("Constructor()", gate::NORMAL);
}



//==========================================================================
roadCharacterizer::roadCharacterizer(const gate::ParamStore& gs, 
			   gate::VLEVEL vl, std::string label) :
  IAlgo(gs,vl,"roadCharacterizer",0,label) {

  _m.message("Constructor()", gate::NORMAL);

}



//==========================================================================
bool roadCharacterizer::initialize() {

  _m.message("Initialize()", gate::NORMAL);

  /// HISTOGRAMS
  // Histogram: Number of Tracks
  gate::Centella::instance()->hman()->h1(this->alabel("NumTracks"),
                                         "Number of Tracks", 10, 0, 10);

  // Histogram: Energy of Hottest Track
  gate::Centella::instance()->hman()->h1(this->alabel("EnergyHotTrk"),
                                         "Energy of Hottest Track", 60, 2.2, 2.5);

  // Histogram: Energy of Non Hottest Tracks
  gate::Centella::instance()->hman()->h1(this->alabel("EnergyNonHotTrk"),
                                         "Energy of Non Hottest Tracks", 50, 0.0, 0.05);

  // Histogram: Distance from Non Hottest Tracks to Hottest Track
  gate::Centella::instance()->hman()->h1(this->alabel("Dist_NonHotTrk_HotTrk"),
                                         "Distance from NHTrks to HTrk", 50, 0.0, 100.);

  // Histogram: Energy of 2nd Hottest Track, when there are only 2
  gate::Centella::instance()->hman()->h1(this->alabel("Energy2ndHotTrk"),
                                         "Energy of 2nd Hottest Track (when trere are only 2)", 50, 0.0, 0.05);

  // Histogram: Distance from 2nd Hottest Track to Hottest Track
  gate::Centella::instance()->hman()->h1(this->alabel("Dist_2ndHotTrk_HotTrk"),
                                         "Distance from 2ndHTrk to HTrk", 50, 0.0, 100.);

  // Histogram: Minimum Distance from 2nd Hottest Track to Hottest Track Extremes
  gate::Centella::instance()->hman()->h1(this->alabel("MinDist_2ndHotTrk_HotTrkExt"),
                                         "Minimum Distance from 2ndHTrk to HTrkExt", 50, 0.0, 100.);

  // Histogram2D: Distance from 2nd Hottest Track to Hottest Track vs Energy of 2nd Hottest Track
  gate::Centella::instance()->hman()->h2(this->alabel("Dist2ndHotTrk_vs_Energy2ndHotTrk"),
                                         "Distance of 2ndHTrk vs Energy of 2ndHTrk",
                                         50, 0.0, 100., 50, 0.0, 0.05);
  gate::Centella::instance()->hman()->fetch(this->alabel("Dist2ndHotTrk_vs_Energy2ndHotTrk"))->SetOption("COLZ");
  gate::Centella::instance()->hman()->fetch(this->alabel("Dist2ndHotTrk_vs_Energy2ndHotTrk"))->GetXaxis()->SetTitle("Dist 2ndHT to HT [mm]");
  gate::Centella::instance()->hman()->fetch(this->alabel("Dist2ndHotTrk_vs_Energy2ndHotTrk"))->GetYaxis()->SetTitle("Energy 2ndHT [MeV]");

  // Histogram2D: Minimum Distance from 2nd Hottest Track to Hottest Track Extremes vs Energy of 2nd Hottest Track
  gate::Centella::instance()->hman()->h2(this->alabel("MinDist2ndHotTrkExt_vs_Energy2ndHotTrk"),
                                         "Min Distance of 2ndHTrk to HTExt vs Energy of 2ndHTrk",
                                         50, 0.0, 100., 50, 0.0, 0.05);
  gate::Centella::instance()->hman()->fetch(this->alabel("MinDist2ndHotTrkExt_vs_Energy2ndHotTrk"))->SetOption("COLZ");
  gate::Centella::instance()->hman()->fetch(this->alabel("MinDist2ndHotTrkExt_vs_Energy2ndHotTrk"))->GetXaxis()->SetTitle("Min Dist 2ndHT to HTExt [mm]");
  gate::Centella::instance()->hman()->fetch(this->alabel("MinDist2ndHotTrkExt_vs_Energy2ndHotTrk"))->GetYaxis()->SetTitle("Energy 2ndHT [MeV]");

  return true;
}



//==========================================================================
bool roadCharacterizer::execute(gate::Event& evt) {

  _m.message("Execute()", gate::DETAILED);

  // Getting Tracks
  std::vector<gate::Track*> tracks = evt.GetTracks();
  int numTracks = tracks.size();
  gate::Centella::instance()->hman()->fill(this->alabel("NumTracks"), numTracks);
  _m.message("Number of Tracks:", numTracks, gate::DETAILED);
  
  // Dealing with the Hottest Track
  double maxEdep = 0.;
  gate::Track* hTrack;
  for (auto track: tracks) {
    double eDep = track->GetEnergy();
    _m.message("Track", track->GetID(), " ->  Edep:", eDep, gate::VERBOSE);
    if (eDep > maxEdep) {
      maxEdep = eDep;
      hTrack = track;
    }
  }
  int hTrackID = hTrack->GetID();
  std::pair<gate::BHit*,gate::BHit*> hTrackExtremes = hTrack->GetExtremes();

  gate::Centella::instance()->hman()->fill(this->alabel("EnergyHotTrk"), maxEdep);
  _m.message("Hottest Track ID:", hTrackID, " -> Edep:", maxEdep, gate::DETAILED);


  // Dealing with the Non Hottest Tracks
  if (numTracks > 1) {
    for (auto track: tracks) {
      double eDep = track->GetEnergy();
      if (eDep != maxEdep) {
        gate::Centella::instance()->hman()->fill(this->alabel("EnergyNonHotTrk"), eDep);
        _m.message("Non Hottest Track ID:", track->GetID(), " -> Edep:", eDep, gate::DETAILED);

        double dist = gate::distance(track, hTrack);
        gate::Centella::instance()->hman()->fill(this->alabel("Dist_NonHotTrk_HotTrk"), dist);
        _m.message("Non Hottest Track ID:", track->GetID(), " -> Distance to HTrk:", dist, gate::DETAILED);

        // Dealing with the 2nd Hottest Track, when there are only 2
        if (numTracks == 2) {
          gate::Centella::instance()->hman()->fill(this->alabel("Energy2ndHotTrk"), eDep);
          _m.message("2nd Hottest Track ID:", track->GetID(), " -> Edep:", eDep, gate::DETAILED);

          gate::Centella::instance()->hman()->fill(this->alabel("Dist_2ndHotTrk_HotTrk"), dist);
          _m.message("2nd Hottest Track ID:", track->GetID(), " -> Distance to HTrk:", dist, gate::DETAILED);

          gate::Centella::instance()->hman()->fill2d(this->alabel("Dist2ndHotTrk_vs_Energy2ndHotTrk"), dist, eDep);

          // Getting the minimum distnce from 2ndHotTrack to HotTrk extremes
          double minDist = 1000.;
          for (auto hit: track->GetHits()) {
            double dist1 = gate::distance(hit, hTrackExtremes.first);
            if (dist1 < minDist) minDist = dist1;
            double dist2 = gate::distance(hit, hTrackExtremes.second);
            if (dist2 < minDist) minDist = dist2;
          }
          gate::Centella::instance()->hman()->fill(this->alabel("MinDist_2ndHotTrk_HotTrkExt"), minDist);
          gate::Centella::instance()->hman()->fill2d(this->alabel("MinDist2ndHotTrkExt_vs_Energy2ndHotTrk"), minDist, eDep);
        }
      }
    }
  }

  return true;
}



//==========================================================================
bool roadCharacterizer::finalize(){

  _m.message("Finalize()", gate::NORMAL);
  
  return true;
}
