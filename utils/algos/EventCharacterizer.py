from Centella.AAlgo import AAlgo
from Centella.physical_constants import *

from ROOT import gSystem
gSystem.Load("$GATE_DIR/lib/libGATE")
gSystem.Load("$GATE_DIR/lib/libGATEIO")
gSystem.Load("$GATE_DIR/lib/libGATEUtils")
from ROOT import gate

"""
This algorithm characterizes Background events that have passed all filters
Parameters:
	blobRadius
	ROIMinE -> ROI Minimum  Energy
	ROIMaxE -> ROI Maximum  Energy
"""


class EventCharacterizer(AAlgo):

	############################################################
	def __init__(self, param=False, level=1, label="", **kargs):

		"""
		Event Characterizer Algorithm
		"""
		#self.m.log(1, 'Constructor()')

		### GENERAL STUFF
		self.name = 'EventCharacterizer'
		#self.level = level
		AAlgo.__init__(self, param, level, self.name, 0, label, kargs)

		### PARAMETERS
		# Blob Radius
		try:
			self.blobRadius = self.doubles['blobRadius']
			self.m.log(1, "Blob Radius: %.1f mm" %(self.blobRadius))
		except KeyError:
			self.m.log(1, "WARNING!! Parameter: 'blobRadius' not defined.")
			exit(0)

		# ROI Minimum Energy
		try:
			self.roiMinE = self.doubles['roiMinE']
			self.m.log(1, "ROI Minimum Energy: %.3f MeV." %(self.roiMinE/MeV))
		except KeyError:
			self.m.log(1, "WARNING!! Parameter: 'roiMinE' not defined.")
			exit(0)

		# ROI Maximum Energy
		try:
			self.roiMaxE = self.doubles['roiMaxE']
			self.m.log(1, "ROI Maximum Energy: %.3f MeV." %(self.roiMaxE/MeV))
		except KeyError:
			self.m.log(1, "WARNING!! Parameter: 'roiMaxE' not defined.")
			exit(0)



	############################################################		
	def initialize(self):

		self.m.log(1, 'Initialize()')
		
		### Defining histos
		# Event Energy Histogram 
		histo_name = self.alabel("EventEnergy")
		histo_desc = "Event Energy"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc,
								 int(self.roiMaxE/KeV - self.roiMinE/KeV),
								 self.roiMinE, self.roiMaxE)
		self.hman.fetch(self.alabel("EventEnergy")).GetXaxis().SetTitle("Event Energy [MeV]")

		# Histogram of Blob1 Energy for the radius set by parameter
		histo_name = self.alabel("Blob1E")
		histo_desc = "Blob1 Energy"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 80, 0, 0.8)

		# Histogram of Blob2 Energy for the radius set by parameter
		histo_name = self.alabel("Blob2E")
		histo_desc = "Blob2 Energy"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 80, 0, 0.8)

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

		# Distance between True & Recons. extreme 1 for events passing the filter
		histo_name = self.alabel("DistTrueRec1")
		histo_desc = "Distance True-Rec extreme 1"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 40, 0, 40) 

		# Distance between True & Recons. extreme 2 for events passing the filter
		histo_name = self.alabel("DistTrueRec2")
		histo_desc = "Distance True-Rec extreme 2"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 40, 0, 40) 

		# Distance between True & Recons. extremes for events passing the filter
		histo_name = self.alabel("DistTrueRec")
		histo_desc = "Distance True-Rec extremes"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 40, 0, 40) 

		# Number of TrueTracks in RecTrack
		histo_name = self.alabel("NumTrueTracks")
		histo_desc = "Number of True Tracks"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 20, 0, 20) 

		# Relation between MainPath Hits over Total Hits
		histo_name = self.alabel("PercMPHits")
		histo_desc = "Perc. of MPHits / Hits"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 30, 20, 80) 

		# Relation between MainPath Hits (in BB zone) over Total Hits (in BB zone)
		histo_name = self.alabel("PercMPHitsBB")
		histo_desc = "Perc. of MPHits / Hits  in BB area"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 30, 20, 80) 

		# Creator process of hottest True Track from RecTrack
		histo_name = self.alabel("hMCTrack_process")
		histo_desc = "Hottest MC Track creator process"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 5, 0, 5)
		self.hman.fetch(histo_name).GetXaxis().SetBinLabel(1, 'Photoelectric') 
		self.hman.fetch(histo_name).GetXaxis().SetBinLabel(2, 'Compton') 
		self.hman.fetch(histo_name).GetXaxis().SetBinLabel(3, 'Ionization') 
		self.hman.fetch(histo_name).GetXaxis().SetBinLabel(4, 'PairProduction') 
		self.hman.fetch(histo_name).GetXaxis().SetBinLabel(5, 'Other') 


		### Counters:
		self.numInputEvents = 0
		self.numSwappedEvents = 0
		self.numEventsConnectedBlobs = 0

		return



	############################################################
	def execute(self, event=""):

		self.m.log(2, 'Execute()')		
	
		self.numInputEvents += 1

		### Event Energy
		evtEdep = event.GetEnergy()
		self.hman.fill(self.alabel("EventEnergy"), evtEdep)
		self.m.log(2, "Event Edep: %.3f MeV" %(evtEdep/MeV))


		### Extremes stuff
		# Getting the Hottest Track
		hTrack = 0
		maxEdep = 0
		for track in self.event.GetTracks():
			trackEdep = track.GetEnergy()
			if (trackEdep > maxEdep):
				maxEdep = trackEdep
				hTrack = track

		distExtFirst  = hTrack.fetch_dvstore("DistExtFirst")
		distExtSecond = hTrack.fetch_dvstore("DistExtSecond")

		# MPHits - Hits stuff
		numHits = hTrack.GetHits().size()
		numMPHits = hTrack.fetch_ivstore("MainPathHits").size()
		self.m.log(2, 'Total Hits: %i   MainPath Hits: %i' % (numHits, numMPHits))		
		self.hman.fill(self.alabel("PercMPHits"), numMPHits*100./numHits)


		# Getting Recons extremes
		energyStart = -1
		ext1Pos = ext2Pos = 0
		blob1E = blob2E = 0
		blob1Vxl = blob2Vxl = 0
		
		# Getting the energies
		connectedBlobs = False
		for hit in hTrack.GetHits():
			dist1 = distExtFirst[hit.GetID()]
			if (dist1 < self.blobRadius):
				blob1E += hit.GetAmplitude()
				blob1Vxl += 1

			dist2 = distExtSecond[hit.GetID()]
			if (dist2 < self.blobRadius):
				blob2E += hit.GetAmplitude()
				blob2Vxl += 1

			if ((dist1 < self.blobRadius) and (dist2 < self.blobRadius)): connectedBlobs = True

		# Connected Blobs Event counter
		if (connectedBlobs):
			self.m.log(2, 'Event with Blobs connected.')
			self.numEventsConnectedBlobs += 1

		# BB MPHits - BB Hits stuff
		numBBHits = numHits - blob1Vxl - blob2Vxl
		numBBMPHits = numMPHits
		for hit in hTrack.GetHits():
			if (hit.GetID() in hTrack.fetch_ivstore("MainPathHits")):
				dist1 = distExtFirst[hit.GetID()]
				dist2 = distExtSecond[hit.GetID()]
				if ((dist1 < self.blobRadius) or (dist2 < self.blobRadius)): numBBMPHits -= 1
		self.m.log(2, 'Total Hits in BB: %i   MainPath Hits in BB: %i' % (numBBHits, numBBMPHits))		
		self.hman.fill(self.alabel("PercMPHitsBB"), numBBMPHits*100./numBBHits)


		# Ordering & Filling Histos
		if (blob1E < blob2E):
			energyStart = hTrack.GetExtremes().first.GetID()
			ext1Pos = hTrack.GetExtremes().first.GetPosition()
			ext2Pos = hTrack.GetExtremes().second.GetPosition()
			self.hman.fill(self.alabel("Blob1E"), blob2E)
			self.hman.fill(self.alabel("Blob1Voxels"), blob2Vxl)
			self.m.log(2, 'Blob1  Energy: %.3f   Voxels: %i' % (blob2E, blob2Vxl))		
			self.hman.fill(self.alabel("Blob2E"), blob1E)
			self.hman.fill(self.alabel("Blob2Voxels"), blob1Vxl)
			self.m.log(2, 'Blob2  Energy: %.3f   Voxels: %i' % (blob1E, blob1Vxl))		

		else:
			energyStart = hTrack.GetExtremes().second.GetID()
			ext2Pos = hTrack.GetExtremes().first.GetPosition()
			ext1Pos = hTrack.GetExtremes().second.GetPosition()
			self.hman.fill(self.alabel("Blob1E"), blob1E)
			self.hman.fill(self.alabel("Blob1Voxels"), blob1Vxl)
			self.m.log(2, 'Blob1  Energy: %.3f   Voxels: %i' % (blob1E, blob1Vxl))		
			self.hman.fill(self.alabel("Blob2E"), blob2E)
			self.hman.fill(self.alabel("Blob2Voxels"), blob2Vxl)
			self.m.log(2, 'Blob2  Energy: %.3f   Voxels: %i' % (blob2E, blob2Vxl))		

		# Getting the MC extremes
		ext1MCPos = ext2MCPos = 0
		MCTracks = event.GetMCTracks()
		numMCTracks = MCTracks.size()
		self.m.log(2, 'Total MC Tracks: %i' % (numMCTracks))		
		self.hman.fill(self.alabel("NumTrueTracks"), numMCTracks)

		# If Signal, True extremes are the second extremes of "primary" tracks
		if (event.GetMCEventType() == gate.BB0NU):
			numPrimaries = 0
			for MCTrack in MCTracks:
				if (MCTrack.GetParticle().IsPrimary()):
					numPrimaries += 1
					if (numPrimaries == 1): ext1MCPos = MCTrack.GetExtremes().second.GetPosition()
					if (numPrimaries == 2):
						ext2MCPos = MCTrack.GetExtremes().second.GetPosition()
						break

		# If Background, True extremes are the extremes of the Hottest TTrack
		else:
			# Getting the hottest True Track from the Hottest Track
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

			# Creator process of hottest True Part 
			process = hMCTrack.GetParticle().GetCreatorProc()
			self.m.log(2, 'Hottest MC Track Creator process: %s' %(process))
			if (process == 'phot'): self.hman.fill(self.alabel("hMCTrack_process"), 0)
			elif (process == 'compt'): self.hman.fill(self.alabel("hMCTrack_process"), 1)
			elif (process == 'eIoni'): self.hman.fill(self.alabel("hMCTrack_process"), 2)
			elif (process == 'conv'): self.hman.fill(self.alabel("hMCTrack_process"), 3)
			else:
				self.hman.fill(self.alabel("hMCTrack_process"), 4)
				print proc

		# Verbosing
		self.m.log(3, 'Extreme 1 MC Position: (%f, %f, %f)' %(ext1MCPos.x(), ext1MCPos.y(), ext1MCPos.z()))
		self.m.log(3, 'Extreme 2 MC Position: (%f, %f, %f)' %(ext2MCPos.x(), ext2MCPos.y(), ext2MCPos.z()))

		self.m.log(3, 'Extreme 1 Rec. Position: (%f, %f, %f)' %(ext1Pos.x(), ext1Pos.y(), ext1Pos.z()))
		self.m.log(3, 'Extreme 2 Rec. Position: (%f, %f, %f)' %(ext2Pos.x(), ext2Pos.y(), ext2Pos.z()))


		# Matching Rec & True extremes by euclidean distance
		dist1 = dist2 = 0
		d11 = gate.distance(ext1Pos, ext1MCPos)
		d12 = gate.distance(ext1Pos, ext2MCPos)

		# Swapped extremes ??
		if (d12 < d11):
			self.m.log(2, 'Extremes Swapped')
			self.numSwappedEvents += 1
			dist1 = d12
			dist2 = gate.distance(ext2Pos, ext1MCPos)
		# Correctly matched extrem
		else:
			dist1 = d11
			dist2 = gate.distance(ext2Pos, ext2MCPos)

		self.m.log(2, 'Distance 1:', dist1)
		self.m.log(2, 'Distance 2:', dist2)

		# Filling Histograms
		self.hman.fill(self.alabel("DistTrueRec"), dist1)
		self.hman.fill(self.alabel("DistTrueRec"), dist2)

		self.hman.fill(self.alabel("DistTrueRec1"), dist1)
		self.hman.fill(self.alabel("DistTrueRec2"), dist2)

		return True

		

	############################################################
	def finalize(self):

		self.m.log(1, 'Finalize()')

		self.m.log(1, 'Input  Events: ', self.numInputEvents)
		self.m.log(1, 'Swapped Events: ', self.numSwappedEvents)
		self.m.log(1, 'Connected Blobs Events: ', self.numEventsConnectedBlobs)

		self.logman["USER"].ints[self.alabel("InputEvents")] = self.numInputEvents
		self.logman["USER"].ints[self.alabel("SwappedEvents")] = self.numSwappedEvents
		self.logman["USER"].ints[self.alabel("ConnectedBlobsEvents")] = self.numEventsConnectedBlobs

		return
