
#include<voxelizer.h>

ClassImp(voxelizer)


//==========================================================================
voxelizer::voxelizer(gate::VLEVEL vl, std::string label) :
	IAlgo(vl, "voxelizer", 0, label) {
  _m.message("Constructor()", gate::NORMAL);
}



//==========================================================================
voxelizer::voxelizer(const gate::ParamStore& gs, gate::VLEVEL vl, std::string label) :
	IAlgo(gs, vl, "voxelizer", 0, label) {

  _m.message("Constructor()", gate::NORMAL);

	// Setting NEXT100 dimensions
	std::pair<double, double>	detSizeX;
	detSizeX.first = -550.;
	detSizeX.second = +550.;
	std::pair<double, double>	detSizeY;
	detSizeY.first = -550.;
	detSizeY.second = +550.;
	std::pair<double, double>	detSizeZ;
	detSizeZ.first = -700.;
	detSizeZ.second = +700.;

	_detSize.push_back(detSizeX);
	_detSize.push_back(detSizeY);
	_detSize.push_back(detSizeZ);

	// Getting the parameters
	_voxelSizeX = gs.fetch_dstore("voxelSizeX");
	_voxelSizeY = gs.fetch_dstore("voxelSizeY");	
	_voxelSizeZ = gs.fetch_dstore("voxelSizeZ");
	_m.message("Voxel size: (", _voxelSizeX, ",", _voxelSizeY, ",",
	           _voxelSizeZ, ") mm", gate::NORMAL);

	_minEnergy  = gs.fetch_dstore("minEnergy");
	_m.message("Track Minimum Energy:", _minEnergy/gate::keV, "KeV", gate::NORMAL);
}



//==========================================================================
bool voxelizer::initialize() {

	_m.message("Initialize()", gate::NORMAL);
	
	// Creating the Voxel Builder
	std::vector<double> voxelSize;
	voxelSize.push_back(_voxelSizeX);
	voxelSize.push_back(_voxelSizeY);
	voxelSize.push_back(_voxelSizeZ);
	_voxelBuilder = new paolina::VoxelBuilder(voxelSize, _detSize);

	// Creating the Track Builder
	_trackBuilder = new paolina::TrackBuilder();


	/// HISTOGRAMS
	// Histogram with number of voxels
	gate::Centella::instance()->hman()->h1(this->alabel("NumVoxels"),
	                                       "Number of Paolina Voxels",
	                                       40, 0.0, 200.0);
	// Histogram with number of tracks
	gate::Centella::instance()->hman()->h1(this->alabel("NumTracks"),
	                                       "Number of Paolina Tracks",
	                                       15, 0.0, 15.0);
	// Histogram with number of invisible tracks
	gate::Centella::instance()->hman()->h1(this->alabel("NumInvTracks"),
	                                       "Number of Invisible Paolina Tracks",
	                                       5, 0.0, 5.0);

	return true;
}



//==========================================================================
bool voxelizer::execute(gate::Event& evt) {

	_m.message("Execute()", gate::DETAILED);

	/// Getting MC Hits
	std::vector<gate::MCHit*> mcHits = evt.GetMCHits();

	/// Translating MC Hits to paolina Hits
	std::vector< std::pair< TVector3, double> > pHits;
	for (auto mcHit: mcHits) {
		std::pair< TVector3, double> pHit;
		const gate::Point3D& pos = mcHit->GetPosition();
		pHit.first = TVector3(pos.x(), pos.y(), pos.z());
		pHit.second = mcHit->GetAmplitude();
		pHits.push_back(pHit);
	}

	/// Getting Paolina Voxels
	std::vector<paolina::Voxel*> pVoxels = _voxelBuilder->FillVoxels(pHits);
	int numVoxels = pVoxels.size();
	gate::Centella::instance()->hman()->fill(this->alabel("NumVoxels"), numVoxels);
	_m.message("Number of Paolina Voxels:", numVoxels, gate::DETAILED);

	/// Getting Paolina Tracks
	std::vector<paolina::Track*> pTracks = _trackBuilder->IdentifyTracks(pVoxels);
	int numTracks = pTracks.size();
	gate::Centella::instance()->hman()->fill(this->alabel("NumTracks"), numTracks);
	_m.message("Number of Paolina Tracks:", numTracks, gate::DETAILED);

  /// Converting Paolina Tracks to GATE Tracks
	int numInvTracks = 0;
  for (int t=0; t<pTracks.size(); t++) {
    paolina::Track* pTrack = pTracks[t];
		double trkEdep = pTrack->GetEDep();
		//Discarding Invisible Paolina Tracks
    if (trkEdep < _minEnergy) {
    	numInvTracks += 1;
      _m.message("Invisible Track of ", trkEdep, "MeV", gate::DETAILED);
		}
    // Processing Visible PTracks
    else {
      // Translating Paolina Voxels & Tracks to Gate Tracks & Hits
      gate::Track* gTrack = new gate::Track();
      gTrack->SetID(t);
      _m.message("New Track with ID:", t, gate::DETAILED);
      double trackLength = pTrack->GetLength();
     	gTrack->SetLength(trackLength);
      _m.message("    Length:", trackLength, "mm", gate::DETAILED);
     	// Adding Hits
     	int numHits = pTrack->NVoxels();
      _m.message("    Adding", numHits, "Hits", gate::DETAILED);     	
      for (int v=0; v<numHits; v++) {
				const paolina::Voxel* pVoxel = pTrack->GetVoxel(v);
      	gate::Hit* gHit = new gate::Hit();
      	gHit->SetID(v);
      	gHit->SetPosition(pVoxel->GetPosition().x(),
      	                  pVoxel->GetPosition().y(),
      	                  pVoxel->GetPosition().z());
      	gHit->SetAmplitude(pVoxel->GetEDep());
      	gTrack->AddHit(gHit);
      	evt.AddHit(gate::SIPM, gHit);
      }

      // Setting Track Energy
      gTrack->SetEnergy(gTrack->GetHitsEnergy());
      _m.message("    Energy Dep:", gTrack->GetEnergy(), "MeV", gate::DETAILED);

    	// Adding Ordered Hits.
    	// Adding their "global" position to a vector of integers
    	// And given to the Track as a property 
    	std::vector<int> mpHits;
      for (int v=0; v<pTrack->NMainPathVoxels(); v++) {
        const paolina::Voxel* pVoxel = pTrack->GetMainPathVoxel(v);
        mpHits.push_back(pVoxel->GetVectorID());
      }
      _m.message("    Main Path Num Hits:", mpHits.size(), gate::DETAILED);       
      _m.message("    Main Path Hits IDs:", gate::vector_to_string(mpHits), gate::VERBOSE);       
      gTrack->store("MainPathHits", mpHits);

    	// Setting Extremes
      std::pair <int, int> pExtremes = pTrack->GetExtremes();
      gTrack->SetExtremes(pExtremes.first, pExtremes.second);
      _m.message("    Extremes Hits:", pExtremes.first, pExtremes.second, gate::DETAILED);       

    	// Setting Spatial Resolution
      gTrack->store("SpatialResX", _voxelSizeX);
      gTrack->store("SpatialResY", _voxelSizeZ);
      gTrack->store("SpatialResZ", _voxelSizeY);
      _m.message("    Spatial Resolution: (", _voxelSizeX, ",", _voxelSizeY, ",",
                 _voxelSizeZ, ")", gate::DETAILED);     	
    	
    	// Adding Mirror Tracks
    	std::vector<int> mcTrackIDs;
   	  for (auto mcTrack: evt.GetMCTracks()) {
        bool connected = false;
       	//Checking distances between mcTrack Hits to gTrack Hits
        for (auto mcHit: mcTrack->GetHits()) {
          gate::Point3D mcHitPos = mcHit->GetPosition();
          for (auto rHit: gTrack->GetHits()) {
            gate::Point3D rHitPos = rHit->GetPosition();
            //std::cout << mcHitPos << rHitPos << std::endl;
            if ( (abs(mcHitPos.x()-rHitPos.x()) < _voxelSizeX/2.) &&
                 (abs(mcHitPos.y()-rHitPos.y()) < _voxelSizeY/2.) &&
                 (abs(mcHitPos.z()-rHitPos.z()) < _voxelSizeZ/2.) ) {
              connected = true;
              mcTrackIDs.push_back(mcTrack->GetID());
              gTrack->AddMirrorTrack(mcTrack);
              _m.message("       mcTrack", mcTrack->GetID(),
                         "connected to Track", gTrack->GetID(), gate::VERBOSE);
              break;
            }
          }
          if (connected) break;
        } 
      }
      gTrack->store("mcTrackIDs", mcTrackIDs);
      if (mcTrackIDs.size() > 0)
      	_m.message("    Origin MC Track IDs:", gate::vector_to_string(mcTrackIDs), gate::DETAILED);
      else
      	_m.message("    Origin MC Track IDs:", gate::DETAILED);

      // Adding the gTrack to the event
      evt.AddTrack(gate::SIPM, gTrack);

      // Verbosing
	    _m.message("Track", gTrack->GetID(), gate::DUMP);
      if (_m.level() >= gate::DUMP) gTrack->Info();
		}
	}

	// Setting Event Energy
	evt.SetEnergy(evt.GetTracksEnergy(gate::SIPM));
  _m.message("Event Energy Dep:", evt.GetEnergy(), "MeV", gate::DETAILED);


	// Filling Histo of Invisible Tracks
	if (numInvTracks>0)
		gate::Centella::instance()->hman()->fill(this->alabel("NumInvTracks"), numInvTracks);


	// Releasing memory related with Paolina stuff
  for (size_t i=0; i < pVoxels.size(); ++i) delete pVoxels[i];
	pVoxels.clear();
  for (size_t i=0; i < pTracks.size(); ++i) delete pTracks[i];
	pTracks.clear();

	return true;
}



//==========================================================================
bool voxelizer::finalize() {

	_m.message("Finalize()", gate::NORMAL);
	
	delete _voxelBuilder;
	delete _trackBuilder;
	
	return true;
}
