from Centella.AAlgo import AAlgo
from Centella.physical_constants import *
from math import sqrt
from ROOT import TRandom3

from ROOT import gSystem
gSystem.Load("$GATE_DIR/lib/libGATE")
gSystem.Load("$GATE_DIR/lib/libGATEIO")
gSystem.Load("$GATE_DIR/lib/libGATEUtils")
from ROOT import gate

"""
This algorithm characterizes voxels created by Paolina in terms of energy and spatial resolution
"""


class voxelCharacterizer(AAlgo):

  ############################################################
  def __init__(self, param=False, level=1, label="", **kargs):

    """
    Voxel Characterizer Algorithm
    """

    ### GENERAL STUFF
    self.name = 'voxelCharacterizer'
    #self.level = level
    AAlgo.__init__(self, param, level, self.name, 0, label, kargs)



  ############################################################    
  def initialize(self):

    self.m.log(1, 'Initialize()')
    
    ### Defining histos
    # Voxels Energy
    histo_name = self.alabel("VoxelEnergy")
    histo_desc = "Voxels Energy"
    self.m.log(2, "Booking ", histo_name)
    self.m.log(3, "   Description: ", histo_desc)
    self.hman.h1(histo_name, histo_desc, 100, 0, 0.05)

    # Voxels Spatial error
    histo_name = self.alabel("VoxelError")
    histo_desc = "Voxels Spatial Error"
    self.m.log(2, "Booking ", histo_name)
    self.m.log(3, "   Description: ", histo_desc)
    self.hman.h1(histo_name, histo_desc, 20, 0, 2.)



    return



  ############################################################
  def execute(self, event=""):

    self.m.log(2, 'Execute()')    

    mcHits  = self.event.GetMCHits()
    hits = self.event.GetHits()

    for hit in hits:
      # Getting the energy
      voxelE = hit.GetAmplitude()
      self.m.log(2, 'Voxel Energy: %.3f MeV' %(voxelE))
      self.hman.fill(self.alabel("VoxelEnergy"), voxelE)

      # Getting the minimum distance to MC Hits
      min_dist = 100
      for mcHit in mcHits:
        dist = gate.distance(hit, mcHit)
        if (dist<min_dist): min_dist = dist
      self.m.log(2, 'Voxel Spatial Error: %.1f mm' %(min_dist))
      self.hman.fill(self.alabel("VoxelError"), min_dist)



    return True

    

  ############################################################
  def finalize(self):

    self.m.log(1, 'Finalize()')

    return
