
#include<roadWidthStudy.h>

#include <TVector.h>
#include <TMatrix.h>


ClassImp(roadWidthStudy)

//==========================================================================
roadWidthStudy::roadWidthStudy(gate::VLEVEL vl, std::string label) : 
IAlgo(vl,"roadWidthStudy",0,label) {
  _m.message("Constructor()", gate::NORMAL);
}



//==========================================================================
roadWidthStudy::roadWidthStudy(const gate::ParamStore& gs, 
                      			   gate::VLEVEL vl, std::string label) :
  IAlgo(gs,vl,"roadWidthStudy",0,label) {

  _m.message("Constructor()", gate::NORMAL);

  // Getting the parameters
  _maxWidth  = gs.fetch_dstore("maxWidth");
  _m.message("Maximum Width Studied:", _maxWidth/gate::mm, "mm", gate::NORMAL);
}



//==========================================================================
bool roadWidthStudy::initialize() {

  _m.message("Initialize()", gate::NORMAL);

  /// HISTOGRAMS
  // Histogram of Events with 1 road
  gate::Centella::instance()->hman()->h1(this->alabel("Events1Road"),
                                         "Number of Events with 1 road",
                                         _maxWidth, 1, _maxWidth+1);

  // Done because I need to manage this Histo in a way not aforded by gate::HistoManager
  _myHisto = gate::Centella::instance()->hman()->fetch(this->alabel("Events1Road"));

  return true;
}



//==========================================================================
bool roadWidthStudy::execute(gate::Event& evt) {

  _m.message("Execute()", gate::DETAILED);

  // Getting Tracks
  std::vector<gate::Track*> tracks = evt.GetTracks();
  int numTracks = tracks.size();
  _m.message("Number of Tracks:", numTracks, gate::DETAILED);

  // If only one Track, No computing needed
  if (numTracks == 1) {
    for (int width=1; width<_maxWidth+1; width++)
      _myHisto->AddBinContent(width);
    return true;
  }

  // Generating the Distance Matrix
  TMatrix distMatrix = TMatrix(numTracks, numTracks);
  for (int i=0; i<numTracks-1; i++) {
    for (int j=i+1; j<numTracks; j++) {
      //double trkDist = getTracksDist(tracks[i], tracks[j]);
      double trkDist = gate::distance(tracks[i], tracks[j]);
      distMatrix(i,j) = distMatrix(j,i) = trkDist;
      _m.message("Distance (", i, ",", j, "):", trkDist, gate::DETAILED);
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
    _m.message("Minimum Distance ", i, ":", minDist, gate::DETAILED);
  }

  // Starting the study for every Width
  for (int width=1; width<_maxWidth+1; width++) {
    _m.message("Study of Width:", width, gate::VERBOSE);

    // First Check: Some RTrack Too Far From the Rest ??
    bool firstCheck = true;
    for (int i=0; i<numTracks; i++) {
      if (minDistVector(i) > width) {
        firstCheck = false;
        _m.message("First Check False. Track", i, "too far from the rest", gate::VERBOSE);
        break;
      }
    }

    // Second Check: All RTracks connected ??
    if (firstCheck == true) {
      // If there are only 2, they are connected
      if (numTracks == 2) {
        _myHisto->AddBinContent(width);
        _m.message("Both Tracks are connected.", gate::VERBOSE);
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
                _m.message("  Connection of ", notConnected[i], "to", connected[j], gate::VERBOSE);
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
          _myHisto->AddBinContent(width);
          _m.message("All RTracks Connected Among Them", gate::VERBOSE);
        }
        else {
          _m.message("Not Connected Tracks:", gate::vector_to_string(notConnected), gate::VERBOSE);
        }
      }
    }
  } // Widths

  return true;
}



//==========================================================================
bool roadWidthStudy::finalize() {

  _m.message("Finalize()", gate::NORMAL);

  _m.message("Number of events with just One Road:", gate::NORMAL);

  std::vector<int> numEvts1road;
  for (int width=1; width<_maxWidth+1; width++) {
    _m.message("       Width:", width, " -> ", _myHisto->GetBinContent(width), gate::NORMAL);
    numEvts1road.push_back(_myHisto->GetBinContent(width));
  }
  gate::Centella::instance()->logman().getLogs("USER").store(this->alabel("NumEventsWithOneRoad"), numEvts1road);

  //_myHisto->Print("range");

  return true;
}

