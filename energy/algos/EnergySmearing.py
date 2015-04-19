from Centella.AAlgo import AAlgo
from Centella.physical_constants import *
from math import sqrt
from ROOT import TRandom3

"""
This algorithm smears event energy in a per Track basis
Parameters:
   FWHMqbb -> FWHM at Qbb
"""


class EnergySmearing(AAlgo):

  ############################################################
  def __init__(self, param=False, level=1, label="", **kargs):

    """
    Energy Smearing Algorithm
    """
    #self.m.log(1, 'Constructor()')

    ### GENERAL STUFF
    self.name = 'EnergySmearing'
    #self.level = level
    AAlgo.__init__(self, param, level, self.name, 0, label, kargs)


    ### NEEDED CONSTANTS and RAND GENERATOR
    self.Qbb = 2.45783
    self.FWHM2Sigma = 2.3548
    self.Rand = TRandom3()

    ### PARAMETERS
    # FWHM at Qbb
    try:
      self.FWHMqbb = self.doubles['FWHMqbb']
      self.m.log(1, "FWHM at Qbb: %.1f %%" %(self.FWHMqbb))
    except KeyError:
      self.m.log(1, "WARNING!! Parameter: 'FWHMqbb' not defined.")
      exit(0)
    


  ############################################################    
  def initialize(self):

    self.m.log(1, 'Initialize()')
    
    ### Defining histos
    # Histogram of True Event Edep
    histo_name = self.alabel("TrueEvtEdep")
    histo_desc = "True Event Energy Dep."
    self.m.log(2, "Booking ", histo_name)
    self.m.log(3, "   Description: ", histo_desc)
    self.hman.h1(histo_name, histo_desc, 80, 2.3, 2.7)
    
    # Histogram of Smeared Event Edep
    histo_name = self.alabel("SmEvtEdep")
    histo_desc = "Smeared Event Energy Dep."
    self.m.log(2, "Booking ", histo_name)
    self.m.log(3, "   Description: ", histo_desc)
    self.hman.h1(histo_name, histo_desc, 60, 2.35, 2.65)

    return



  ############################################################
  def execute(self, event=""):

    self.m.log(2, 'Execute()')    
  
    for track in self.event.GetTracks():
      # True Edeps
      trueTrackEdep = track.GetEnergy()

      # Smeared Edeps
      track.store("EnergyRes", self.FWHMqbb)
      # Scaling FWHM at Qbb to Sigma at True Energy
      sigmaE = ((self.FWHMqbb/100.) * sqrt(self.Qbb) * sqrt(trueTrackEdep)) / self.FWHM2Sigma
      smTrackEdep = trueTrackEdep + self.Rand.Gaus(0., 1.) * sigmaE
      track.SetEnergy(smTrackEdep)
      self.m.log(3, 'Track %i  ->  True Edep: %.3f  Sm. Edep: %.3f' 
                 %(track.GetID(), trueTrackEdep, smTrackEdep))

      # Updating Energy of Hits from Tracks
      hitFactor = smTrackEdep / trueTrackEdep
      for hit in track.GetHits() :
        hit.SetAmplitude(hit.GetAmplitude() * hitFactor);

    # Updating Event Energy
    self.event.SetEnergy(event.GetTracksEnergy(1))

    # Filling Histograms
    self.hman.fill(self.alabel("TrueEvtEdep"), event.GetMCEnergy())
    self.hman.fill(self.alabel("SmEvtEdep"), event.GetEnergy())


    # Verbosity
    self.m.log(2, 'Event True Edep: %.3f  Sm. Edep: %.3f'
               %(event.GetMCEnergy(), event.GetEnergy()))

    return True

    

  ############################################################
  def finalize(self):

    self.m.log(1, 'Finalize()')

    return
