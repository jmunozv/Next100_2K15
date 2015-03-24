
#include<roadWidthStudy.h>

#include <TVector.h>
#include <TMatrix.h>


ClassImp(roadWidthStudy)

//==========================================================================
roadWidthStudy::roadWidthStudy(gate::VLEVEL vl, std::string label) : 
IAlgo(vl,"roadWidthStudy",0,label) {
  _m.message("Constructor()", gate::CONCISE);
}



//==========================================================================
roadWidthStudy::roadWidthStudy(const gate::ParamStore& gs, 
                      			   gate::VLEVEL vl, std::string label) :
  IAlgo(gs,vl,"roadWidthStudy",0,label) {

  _m.message("Constructor()", gate::CONCISE);

  // Getting the parameters
  _minWidth  = gs.fetch_dstore("minWidth");
  _maxWidth  = gs.fetch_dstore("maxWidth");
  _m.message("Minimum Width Studied:", _minWidth/gate::mm, "mm", gate::CONCISE);
  _m.message("Maximum Width Studied:", _maxWidth/gate::mm, "mm", gate::CONCISE);
}



//==========================================================================
bool roadWidthStudy::initialize() {

  _m.message("Initialize()", gate::CONCISE);

  /// HISTOGRAMS
  // Histogram of Events with 1 road
  gate::Centella::instance()->hman()->h1(this->alabel("Events1Road"),
                                         "Number of Events with 1 road",
                                         (_maxWidth-_minWidth), _minWidth, _maxWidth);

  // Done because I need to manage this Histo in a way not aforded by gate::HistoManager
  _myHisto = gate::Centella::instance()->hman()->fetch(this->alabel("Events1Road"));

  return true;
}



//==========================================================================
bool roadWidthStudy::execute(gate::Event& evt) {

  _m.message("Execute()", gate::NORMAL);

  // Getting Tracks
  std::vector<gate::Track*> tracks = evt.GetTracks();
  int numTracks = tracks.size();
  _m.message("Number of Tracks:", numTracks, gate::NORMAL);

  // If only one Track, No computing needed
  if (numTracks == 1) {
    for (int width=_minWidth; width<_maxWidth+1; width++)
      _myHisto->AddBinContent(width+1);
    return true;
  }

  // Generating the Distance Matrix
  TMatrix distMatrix = TMatrix(numTracks, numTracks);
  for (int i=0; i<numTracks-1; i++) {
    for (int j=i+1; j<numTracks; j++) {
      double trkDist = getTracksDist(tracks[i], tracks[j]);
      distMatrix(i,j) = distMatrix(j,i) = trkDist;
      _m.message("Distance (", i, ",", j, "):", trkDist, gate::NORMAL);
    }
  }

  // Generating the Minimum Distance Vector
  TVector minDistVector(numTracks);
  for (int i=0; i<numTracks; i++) {
    double minDist = 1000.;
    for (int j=0; j<numTracks; j++) {
      if (i != j) 
        if (distMatrix(i,j) < minDist) minDist = distMatrix(i,j);
    }
    minDistVector(i) = minDist;
    _m.message("Minimum Distance ", i, ":", minDist, gate::NORMAL);
  }





  // Starting the study for every Width
  for (int width=_minWidth; width<_maxWidth+1; width++) {
    _m.message("Study of Width:", width, gate::WARNING);

    // First Check: Some RTrack Too Far From the Rest ??
    bool firstCheck = true;
    for (int i=0; i<numTracks; i++) {
      if (minDistVector(i) > width) {
        firstCheck = false;
        _m.message("First Check False. Track", i, "too far from the rest", gate::WARNING);
        break;
      }
    }

    // Second Check: All RTracks connected ??
    if (firstCheck == true) {
      // If there are only 2, they are connected
      if (numTracks == 2) {
        _myHisto->AddBinContent(width+1);
        _m.message("Both Tracks are connected.", gate::WARNING);
      }

      else {
        // Initializing vectors
        std::vector<int> connected;
        connected.push_back(0);
        std::vector<int> notConnected;
        for (int i=1; i<numTracks; i++) notConnected.push_back(i);

        // Checking connections
        bool gotConnection;
        do {
          gotConnection = false;
          for (int i=0; i<notConnected.size(); i++) {
            for (int j=0; j<connected.size(); j++) {
              double dist = distMatrix(notConnected[i], connected[j]);
              if (dist < width) {
                gotConnection = true;
                _m.message("  Connection of ", notConnected[i], gate::WARNING);
                connected.push_back(notConnected[i]);
                notConnected.erase(notConnected.begin()+i);
                break;
              }
            }
            if (gotConnection) break;
          }
        } while (gotConnection and (notConnected.size()>0));

        // If every RTrack is connected -> Evt OK
        int nons = notConnected.size();
        if (nons==0) {
          _myHisto->AddBinContent(width+1);
          _m.message("All RTracks Connected Among Them", gate::WARNING);
        }
        //else {
        //  std::cout << "RTracks not Connected With The Others: ";
        //  for (int i=0; i<nons; i++) std::cout << notConnected[i] << ", ";
        //  std::cout << std::endl;
        //}

      }
    }
  } // Widths





  return true;
}



//==========================================================================
bool roadWidthStudy::finalize() {

  _m.message("Finalize()", gate::CONCISE);

  //gate::Centella::instance()->hman()->fetch(this->alabel("Events1Road"))->Print();
  //_myHisto->Print("range");

  _m.message("Number of events with just One Road:", gate::CONCISE);
  for (int width=_minWidth; width<_maxWidth+1; width++)
    _m.message("       Width:", width, " -> ", _myHisto->GetBinContent(width+1), gate::CONCISE);

  return true;
}



//==========================================================================
double roadWidthStudy::getTracksDist(gate::BTrack* track1, gate::BTrack* track2) {
  double tracksDist = 10000.;
  for (auto hit1 : track1->GetHits()) {
    for (auto hit2 : track2->GetHits()) {
      double hitsDist = getHitsDist(hit1, hit2);
      if (hitsDist < tracksDist) tracksDist = hitsDist;
    }
  }

  return tracksDist;
}



//==========================================================================
double roadWidthStudy::getHitsDist(gate::BHit* hit1, gate::BHit* hit2) {
  gate::Point3D pos1 = hit1->GetPosition();
  gate::Point3D pos2 = hit2->GetPosition();
  double distx = pos1.x() - pos2.x();
  double disty = pos1.y() - pos2.y();
  double distz = pos1.z() - pos2.z();
  return sqrt(distx*distx + disty*disty + distz*distz);
}

