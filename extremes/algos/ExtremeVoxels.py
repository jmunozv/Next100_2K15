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
This algorithm studies the possibility of consider the number of voxels arround the extremes
as a new filter.
"""


class ExtremeVoxels(AAlgo):

  ############################################################
  def __init__(self, param=False, level=1, label="", **kargs):

    """
    Energy Smearing Algorithm
    """
    #self.m.log(1, 'Constructor()')

    ### GENERAL STUFF
    self.name = 'ExtremeVoxels'
    #self.level = level
    AAlgo.__init__(self, param, level, self.name, 0, label, kargs)


    ### PARAMETERS
    # Blob Radius
    try:
      self.blobRadius = self.ints['blobRadius']
      self.m.log(1, "Blob Radius: %i mm" %(self.blobRadius))
    except KeyError:
      self.m.log(1, "WARNING!! Parameter: 'blobRadius' not defined.")
      exit(0)
    
    # Minimum Number of Voxels
    try:
      self.numVoxels = self.ints['numVoxels']
      self.m.log(1, "Minimum Number of Voxels: %i" %(self.numVoxels))
    except KeyError:
      self.m.log(1, "WARNING!! Parameter: 'numVoxels' not defined.")
      exit(0)
    


  ############################################################    
  def initialize(self):

    self.m.log(1, 'Initialize()')
    
    ### Defining histos
    # Histogram of BlobRad vs Blob1 Voxels
    histo_name = self.alabel("BlobRad_Blob1Voxels")
    histo_desc = "Blobs Radius vs Blob1 Voxels"
    self.m.log(2, "Booking ", histo_name)
    self.m.log(3, "   Description: ", histo_desc)
    self.hman.h2(histo_name, histo_desc, 20, 0, 20, 20, 0, 20)
    self.hman.fetch(histo_name).SetOption("COLZ")

    # Histogram of BlobRad vs Blob2 Voxels
    histo_name = self.alabel("BlobRad_Blob2Voxels")
    histo_desc = "Blobs Radius vs Blob2 Voxels"
    self.m.log(2, "Booking ", histo_name)
    self.m.log(3, "   Description: ", histo_desc)
    self.hman.h2(histo_name, histo_desc, 20, 0, 20, 20, 0, 20)
    self.hman.fetch(histo_name).SetOption("COLZ")

    # Histogram of Blob1 Voxels for the radius set by parameter
    histo_name = self.alabel("Blob1Voxels")
    histo_desc = "Number of Blob1 Voxels"
    self.m.log(2, "Booking ", histo_name)
    self.m.log(3, "   Description: ", histo_desc)
    self.hman.h1(histo_name, histo_desc, 30, 0, 30)

    # Histogram of Blob2 Voxels for the radius set by parameter
    histo_name = self.alabel("Blob2Voxels")
    histo_desc = "Number of Blob2 Voxels"
    self.m.log(2, "Booking ", histo_name)
    self.m.log(3, "   Description: ", histo_desc)
    self.hman.h1(histo_name, histo_desc, 30, 0, 30)

    # Distance between True & Recons. extremes for events passing the filter
    histo_name = self.alabel("DistTrueRec")
    histo_desc = "Distance True-Rec extremes"
    self.m.log(2, "Booking ", histo_name)
    self.m.log(3, "   Description: ", histo_desc)
    self.hman.h1(histo_name, histo_desc, 40, 0, 40) 


    ### Counters:
    self.numInputEvents = 0
    self.numOutputEvents = 0
    self.numEvtsSwapedBlobs = 0

    return



  ############################################################
  def execute(self, event=""):

    self.m.log(2, 'Execute()')    

    self.numInputEvents += 1   

    # Getting with the Hottest Track
    hTrack = 0
    maxEdep = 0
    for track in self.event.GetTracks():
      trackEdep = track.GetEnergy()
      if (trackEdep > maxEdep):
        maxEdep = trackEdep
        hTrack = track

    # Getting the extremes positions
    ext1Pos = hTrack.GetExtremes().first.GetPosition()
    ext2Pos = hTrack.GetExtremes().second.GetPosition()

    ### Study of Blob Radius vs Blob2E
    numRads = 20
    voxelsBlob1 = []
    voxelsBlob2 = []
    eBlob1 = []
    eBlob2 = []
    for i in range(numRads):
      voxelsBlob1.append(0)
      voxelsBlob2.append(0)
      eBlob1.append(0.)
      eBlob2.append(0.)

    for hit in event.GetHits():
      hPos = hit.GetPosition()
      dist1 = gate.distance(ext1Pos, hPos)
      dist2 = gate.distance(ext2Pos, hPos)

      for rad in range(numRads):
        if (dist1 < rad):
          voxelsBlob1[rad] += 1
          eBlob1[rad] += hit.GetAmplitude()

        if (dist2 < rad):
          voxelsBlob2[rad] += 1
          eBlob2[rad] += hit.GetAmplitude()

    # Filling Histograms
    for rad in range(numRads):
      voxels1 = voxelsBlob1[rad]
      voxels2 = voxelsBlob2[rad]
      if (voxels1 < voxels2):
        self.hman.fill(self.alabel("BlobRad_Blob2Voxels"), rad+1, voxels1)
        self.hman.fill(self.alabel("BlobRad_Blob1Voxels"), rad+1, voxels2)
      else:
        self.hman.fill(self.alabel("BlobRad_Blob2Voxels"), rad+1, voxels2)
        self.hman.fill(self.alabel("BlobRad_Blob1Voxels"), rad+1, voxels1)
 
    # Verbosity
    self.m.log(3, 'Extreme A - Voxels distribution:', voxelsBlob1)
    self.m.log(3, 'Extreme B - Voxels distribution:', voxelsBlob2)


    ### Study of Number of Voxels given a Blob Radius
    voxels1 = voxelsBlob1[self.blobRadius]
    voxels2 = voxelsBlob2[self.blobRadius]

    if (voxels1 < voxels2):
      self.hman.fill(self.alabel("Blob2Voxels"), voxels1)
      self.hman.fill(self.alabel("Blob1Voxels"), voxels2)
      self.m.log(2, 'Extreme 1 Num Voxels (rad=%i): %i' %(self.blobRadius, voxels2))
      self.m.log(2, 'Extreme 2 Num Voxels (rad=%i): %i' %(self.blobRadius, voxels1))
    else:
      self.hman.fill(self.alabel("Blob2Voxels"), voxels2)
      self.hman.fill(self.alabel("Blob1Voxels"), voxels1)
      self.m.log(2, 'Extreme 1 Num Voxels (rad=%i): %i' %(self.blobRadius, voxels1))
      self.m.log(2, 'Extreme 2 Num Voxels (rad=%i): %i' %(self.blobRadius, voxels2))


    filtResult = True
    ### Filter Implementation
    if ((voxels1 > self.numVoxels) and (voxels2 > self.numVoxels)):
      self.m.log(2, 'Filter Passed')
      self.numOutputEvents += 1
      filtResult = True
    else:
      self.m.log(2, 'Filter Failed')
      filtResult = False

    ### Only for events passing the filter ...
    ### Computing the error in extremes identification.

    # Getting the MC extremes positions
    ext1MCPos = ext2MCPos = 0
    MCTracks = event.GetMCTracks()

    # If Signal, True extremes are the second extremes of "primary" tracks
    if (event.GetMCEventType() == gate.BB0NU):
      numPrimaries = 0
      for MCTrack in MCTracks:
        if (MCTrack.GetParticle().IsPrimary()):
          numPrimaries += 1
          if (numPrimaries ==1): ext1MCPos = MCTrack.GetExtremes().second.GetPosition()
          if (numPrimaries ==2):
            ext2MCPos = MCTrack.GetExtremes().second.GetPosition()
            break

    # If Background, True extremes are the extremes of the Hottest TTracks
    else:
      # Getting the hottest True Track from the Hottest rTrack
      MCTrackIDs = hTrack.fetch_ivstore("mcTrackIDs")
      maxEdep = 0
      hMCTrack = 0
      for MCTrack in MCTracks:
        if MCTrack.GetID() in MCTrackIDs : 
          eDep = MCTrack.GetEnergy()
          if (eDep > maxEdep):
            maxEdep = eDep
            hMCTrack = MCTrack

      ext1MCPos = hMCTrack.GetExtremes().first.GetPosition()
      ext2MCPos = hMCTrack.GetExtremes().second.GetPosition()


    self.m.log(3, 'Extreme 1 Rec. Position:', ext1Pos)
    self.m.log(3, 'Extreme 2 Rec. Position:', ext2Pos)

    self.m.log(3, 'Extreme 1 MC Position:', ext1MCPos)
    self.m.log(3, 'Extreme 2 MC Position:', ext2MCPos)


    # // Matching Rec & True extremes by euclidean distance
    d11 = gate.distance(ext1Pos, ext1MCPos)
    d12 = gate.distance(ext1Pos, ext2MCPos)
    if (d11 < d12):
      dist1 = d11;
      dist2 = gate.distance(ext2Pos, ext2MCPos)

      # In this case, Rec1 corresponds to MC1
      # If energy arround Rec2 is lower than Rec1, then blobs are swaped
      if (eBlob1[10] > eBlob2[10]):
        self.numEvtsSwapedBlobs += 1

    else:
      dist1 = d12
      dist2 = gate.distance(ext2Pos, ext1MCPos)

      # In this case, Rec2 corresponds to MC1
      #If energy arround Rec2 is higher than Rec1, then blobs are swaped
      if (eBlob1[10] < eBlob2[10]):
        self.numEvtsSwapedBlobs += 1


    self.m.log(2, 'Distance 1:', dist1)
    self.m.log(2, 'Distance 2:', dist2)

    self.hman.fill(self.alabel("DistTrueRec"), dist1)
    self.hman.fill(self.alabel("DistTrueRec"), dist2)


    return filtResult

    

  ############################################################
  def finalize(self):

    self.m.log(1, 'Finalize()')

    self.m.log(1, 'Input  Events: ', self.numInputEvents)
    self.m.log(1, 'Output Events: ', self.numOutputEvents)
    self.m.log(1, 'SwapedBlob Events: ', self.numEvtsSwapedBlobs)

    self.logman["USER"].ints[self.alabel("InputEvents")] = self.numInputEvents
    self.logman["USER"].ints[self.alabel("OutputEvents")] = self.numOutputEvents
    self.logman["USER"].ints[self.alabel("SwapedBlobEvents")] = self.numEvtsSwapedBlobs

    return
