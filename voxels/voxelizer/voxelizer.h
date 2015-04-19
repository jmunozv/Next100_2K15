#ifndef _voxelizer__
#define _voxelizer__

#include <GATE/Centella.h>
#include <GATE/system_of_units.h>

#include <paolina/VoxelBuilder.h>
#include <paolina/TrackBuilder.h>
#include <paolina/Track.h>

class voxelizer : public gate::IAlgo {

  public:
  
    //! default contructor
    voxelizer(gate::VLEVEL=gate::NORMAL,
	            std::string label="voxelizerInstance");
  
    //! constructor with store with input parameters 
    voxelizer(const gate::ParamStore& gs,
	            gate::VLEVEL=gate::NORMAL,
	            std::string label="voxelizerInstance");
  
    //! destructor
    virtual ~voxelizer(){};
  
    //! initialize algorithm
    bool initialize();        
  
    //! execute algorithm: process current event
    bool execute(gate::Event& evt);  
  
    //! finalize algorithm
    bool finalize();          
  
  private:

    // Voxel Size
    double _voxelSizeX;
    double _voxelSizeY;
    double _voxelSizeZ;

    // Detector Size
    std::vector< std::pair<double,double> > _detSize;

    // Paolina Builders
    paolina::VoxelBuilder* _voxelBuilder;
    paolina::TrackBuilder* _trackBuilder;

    // Minimum Track Energy to be visible
    double _minEnergy;


    ClassDef(voxelizer, 0)
    
};

#endif
