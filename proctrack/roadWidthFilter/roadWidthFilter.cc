
#include<roadWidthFilter.h>

#include <TVector.h>
#include <TMatrix.h>

ClassImp(roadWidthFilter)


//==========================================================================
roadWidthFilter::roadWidthFilter(gate::VLEVEL vl, std::string label) : 
IAlgo(vl,"roadWidthFilter",0,label) {
  _m.message("Constructor()", gate::NORMAL);
}



//====================================================================================
roadWidthFilter::roadWidthFilter(const gate::ParamStore& gs, gate::VLEVEL vl, std::string label) :
  IAlgo(gs,vl,"roadWidthFilter",0,label) {

  _m.message("Constructor()", gate::NORMAL);

  // Getting the parameters
  _roadWidth  = gs.fetch_dstore("roadWidth");
  _m.message("Road Width:", _roadWidth/gate::mm, "mm", gate::NORMAL);
}



//==========================================================================
bool roadWidthFilter::initialize() {

  _m.message("Initialize()", gate::NORMAL);

  // Histograms with number of roads
  gate::Centella::instance()->hman()->h1(this->alabel("NumRoads"),
                                         "Number of Roads", 4, 0, 4);
  gate::Centella::instance()->hman()->fetch(this->alabel("NumRoads"))->GetXaxis()->SetBinLabel(2, "One");
  gate::Centella::instance()->hman()->fetch(this->alabel("NumRoads"))->GetXaxis()->SetBinLabel(3, "More than One");

  /// Counters
  _numInputEvents  = 0;
  _numOutputEvents = 0;

  return true;
}



//==========================================================================
bool roadWidthFilter::execute(gate::Event& evt) {

  _m.message("Execute()", gate::DETAILED);
  
  _numInputEvents += 1;

  // Getting Tracks
  std::vector<gate::Track*> tracks = evt.GetTracks();
  int numTracks = tracks.size();
  _m.message("Number of Tracks:", numTracks, gate::DETAILED);

  // If only one Track, No computing needed
  if (numTracks == 1) {
    _m.message("Filter Passed -> Just One RTrack", gate::DETAILED);
    _numOutputEvents += 1;
    gate::Centella::instance()->hman()->fill(this->alabel("NumRoads"), 1);
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

  // First Check: Some RTrack Too Far From the Rest ??
  for (int i=0; i<numTracks; i++) {
    if (minDistVector(i) > _roadWidth) {
      _m.message("Filter Failed -> Track", i, "too far from the rest", gate::DETAILED);
      gate::Centella::instance()->hman()->fill(this->alabel("NumRoads"), 2);
      return false;
    }
  }

  // At this point, If there are only 2 RTracks, they are connected
  if (numTracks == 2) {
    _m.message("Filter Passed -> Both Tracks are connected.", gate::DETAILED);
    _numOutputEvents +=1 ;
    gate::Centella::instance()->hman()->fill(this->alabel("NumRoads"), 1);
    return true;
  }

  // All RTracks connected ??
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
        if (dist < _roadWidth) {
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
  if (notConnected.size() == 0) {
    _m.message("Filter Passed -> All RTracks Connected Among Them", gate::DETAILED);
    _numOutputEvents +=1 ;
    gate::Centella::instance()->hman()->fill(this->alabel("NumRoads"), 1);
    return true;
  }
  else {
    _m.message("Filter Failed -> Not Connected Tracks:", gate::vector_to_string(notConnected), gate::DETAILED);
    gate::Centella::instance()->hman()->fill(this->alabel("NumRoads"), 2);
    return false;
  }

  gate::Centella::instance()->hman()->fill(this->alabel("NumRoads"), 2);
  return false;
}



//==========================================================================
bool roadWidthFilter::finalize() {

  _m.message("Finalize()", gate::NORMAL);

  _m.message("Input  Events:", _numInputEvents, gate::NORMAL);
  _m.message("Output Events:", _numOutputEvents, gate::NORMAL);

  gate::Centella::instance()->logman().getLogs("USER").store(this->alabel("InputEvents"), _numInputEvents);
  gate::Centella::instance()->logman().getLogs("USER").store(this->alabel("OutputEvents"), _numOutputEvents);

  return true;
}
