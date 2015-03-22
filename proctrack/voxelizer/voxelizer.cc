
#include<voxelizer.h>

ClassImp(voxelizer)


//==========================================================================
voxelizer::voxelizer(gate::VLEVEL vl, std::string label) :
	IAlgo(vl, "voxelizer", 0, label) {
}



//==========================================================================
voxelizer::voxelizer(const gate::ParamStore& gs, gate::VLEVEL vl, std::string label) :
	IAlgo(gs, vl, "voxelizer", 0, label) {

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
//	_m.message("Voxel size: (%f, %f, %f) mm" %(_voxelSizeX, _voxelSizeY, _voxelSizeZ),
//	           gate::NORMAL);

	_minEnergy  = gs.fetch_dstore("minEnergy");
//	_m.message("Track Minimum Energy: %.1f KeV" %(_minEnergy/gate::keV),
//	           gate::NORMAL);
}



//==========================================================================
bool voxelizer::initialize() {
	_m.message(this->getAlgoLabel(), "::initialize()", gate::CONCISE);
	
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

	_m.message(this->getAlgoLabel(), "::Execute()", gate::NORMAL);
	
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
	_m.message(this->getAlgoLabel(), "::Number of Paolina Voxels:", numVoxels, gate::NORMAL);

	/// Getting Paolina Tracks
	std::vector<paolina::Track*> pTracks = _trackBuilder->IdentifyTracks(pVoxels);
	int numTracks = pTracks.size();
	gate::Centella::instance()->hman()->fill(this->alabel("NumTracks"), numTracks);
	_m.message(this->getAlgoLabel(), "::Number of Paolina Tracks:", numTracks, gate::NORMAL);

  /// Converting Paolina Tracks to GATE Tracks
	int numInvTracks = 0;
  for (int t=0; t<pTracks.size(); t++) {
    paolina::Track* pTrack = pTracks[t];
		double trkEdep = pTrack->GetEDep();
		//Discarding Invisible Paolina Tracks
    if (trkEdep < _minEnergy) {
    	numInvTracks += 1;
      _m.message(this->getAlgoLabel(), "::Invisible RTrack of ", trkEdep, "MeV", gate::NORMAL);
		}
    // Processing Visible PTracks
    else {
      _m.message(this->getAlgoLabel(), "::New rTrack of", trkEdep, "MeV", gate::NORMAL);
      // Translating Paolina Voxels & Tracks to Gate Tracks & Hits
      gate::Track* gTrack = new gate::Track();
      gTrack->SetID(t);
     	gTrack->SetLength(pTrack->GetLength());
     	// Adding Hits
      for (int v=0; v<pTrack->NVoxels(); v++) {
				const paolina::Voxel* pVoxel = pTrack->GetVoxel(v);
      	gate::Hit* gHit = new gate::Hit();
      	gHit->SetID(v);
      	gHit->SetPosition(pVoxel->GetPosition().x(),
      	                  pVoxel->GetPosition().y(),
      	                  pVoxel->GetPosition().z());
      	gHit->SetAmplitude(pVoxel->GetEDep());
      	gTrack->AddHit(gHit);
      }

    	// Adding Ordered Hits
      for (int v=0; v<pTrack->NMainPathVoxels(); v++) {
        const paolina::Voxel* pVoxel = pTrack->GetMainPathVoxel(v);
        for (auto hit: gTrack->GetHits()) {
          if ( (hit->GetAmplitude() == pVoxel->GetEDep()) and
               (hit->GetPosition().x() == pVoxel->GetPosition().x()) and
               (hit->GetPosition().y() == pVoxel->GetPosition().y()) and
               (hit->GetPosition().z() == pVoxel->GetPosition().z()) ) {
            //gTrack->AddOrdHit(hit);
            break;
          }
        } 
      }

    	// Setting Extremes
      std::pair <int, int> pExtremes = pTrack->GetExtremes();
      //gate::Hit* hit1 = gTrack->GetHit(pExtremes.first);
      //gate::Hit* hit2 = gTrack->GetHit(pExtremes.second);
      //gTrack->SetExtreme1(hit1);
      //gTrack->SetExtreme2(hit2);
      gTrack->store("Extreme1ID", pExtremes.first);
      gTrack->store("Extreme2ID", pExtremes.second);

    	// Setting Spatial Resolution
      gTrack->store("SpatialResX", _voxelSizeX);
      gTrack->store("SpatialResY", _voxelSizeZ);
      gTrack->store("SpatialResZ", _voxelSizeY);
    	
    	// Assigning TTrackIDs
    	std::vector<gate::MCTrack*> mcTracks = evt.GetMCTracks();
    	std::vector<int> mcTrackIDs;
   	  for (auto mcTrack: mcTracks) {
   	  	bool connected = false;
       	//Checking distances between first hit of mcTrack to all from the gTrack
       	gate::Point3D mcHitPos = mcTrack->GetHits()[0]->GetPosition();
       	for (auto rHit: gTrack->GetHits()) {
         	gate::Point3D rHitPos = rHit->GetPosition();
         	if ( (abs(mcHitPos.x()-rHitPos.x()) < _voxelSizeX) &&
           	   (abs(mcHitPos.y()-rHitPos.y()) < _voxelSizeY) &&
             	 (abs(mcHitPos.z()-rHitPos.z()) < _voxelSizeZ) ) {
 	          connected = true;
 	        	mcTrackIDs.push_back(mcTrack->GetID());
   	        //rTrack->AddTTrackID(mcTrack->GetID());
   	        _m.message(this->getAlgoLabel(), "::mcTrack", mcTrack->GetID(),
   	                   "connected to Track", gTrack->GetID(), gate::WARNING);
         	  break;
         	}
       	}
      }
      
      gTrack->store("mcTrackIDs", mcTrackIDs);

      // Verbosing
	    _m.message(this->getAlgoLabel(), "::Track", gTrack->GetID(), gTrack, gate::NORMAL);

    	// Adding the gTrack to the event
    	evt.AddTrack(gate::SIPM, gTrack);
		}

	}

	// Filling Histo of Invisible Tracks
	if (numInvTracks>0)
		gate::Centella::instance()->hman()->fill(this->alabel("NumInvTracks"), numInvTracks);

	return true;

}



//==========================================================================
bool voxelizer::finalize() {
	_m.message(this->getAlgoLabel(), "::finalize()", gate::CONCISE);
	
	delete _voxelBuilder;
	delete _trackBuilder;
	
	return true;
}
