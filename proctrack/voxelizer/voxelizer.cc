
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
	_m.message(this->getAlgoLabel(), "::initialize()", gate::NORMAL);
	_m.message("Intializing algorithm", this->getAlgoLabel(), gate::NORMAL);
	
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

	_m.message(this->getAlgoLabel(), "::Execute()", gate::VERBOSE);
	
	// Getting MC Hits
	std::vector<gate::MCHit*> mcHits = evt.GetMCHits();

	// Translating MC Hits to paolina Hits
	std::vector< std::pair< TVector3, double> > pHits;
	for (auto mcHit: mcHits) {
		std::pair< TVector3, double> pHit;
		const gate::Point3D& pos = mcHit->GetPosition();
		pHit.first = TVector3(pos.x(), pos.y(), pos.z());
		pHit.second = mcHit->GetAmplitude();
		pHits.push_back(pHit);
	}

	// Getting Paolina Voxels
	std::vector<paolina::Voxel*> pVoxels = _voxelBuilder->FillVoxels(pHits);
	int numVoxels = pVoxels.size();
	gate::Centella::instance()->hman()->fill(this->alabel("NumVoxels"), numVoxels);
	_m.message(this->getAlgoLabel(), "::Number of Voxels: ", numVoxels, gate::VERBOSE);

	// Getting Paolina Tracks
	std::vector<paolina::Track*> pTracks = _trackBuilder->IdentifyTracks(pVoxels);
	int numTracks = pTracks.size();
	gate::Centella::instance()->hman()->fill(this->alabel("NumTracks"), numTracks);
	_m.message(this->getAlgoLabel(), "::Number of Tracks: ", numTracks, gate::VERBOSE);

  // Converting Paolina Tracks to GATE Tracks



	return true;

}



//==========================================================================
bool voxelizer::finalize() {

	_m.message("Finalising algorithm", this->getAlgoLabel(), gate::NORMAL);
	
	delete _voxelBuilder;
	delete _trackBuilder;
	
	return true;

}
